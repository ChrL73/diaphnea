#include "Map.h"
#include "PointElement.h"
#include "PolygonElement.h"
#include "LineElement.h"
#include "PointLook.h"
#include "LineLook.h"
#include "PolygonLook.h"
#include "LineItem.h"
#include "FilledPolygonItem.h"
#include "PointItem.h"
#include "Point.h"
#include "ItemLook.h"
#include "DatabaseError.h"

namespace map_server
{
    Map::~Map()
    {
        std::map<std::string, MapElement *>::iterator elementIt = _elementMap.begin();
        for (; elementIt != _elementMap.end(); ++elementIt) delete (*elementIt).second;

        std::map<int, MapItem *>::iterator itemIt = _itemMap.begin();
        for (; itemIt != _itemMap.end(); ++itemIt) delete (*itemIt).second;

        std::map<int, const Look *>::iterator lookIt = _lookMap.begin();
        for (; lookIt != _lookMap.end(); ++lookIt) delete (*lookIt).second;
    }

    MapElement *Map::getElement(const std::string& id)
    {
        std::map<std::string, MapElement *>::iterator elementIt = _elementMap.find(id);
        if (elementIt == _elementMap.end()) return 0;

        MapElement *element = (*elementIt).second;
		if (element->error()) return 0;
		if (!element->isLoaded())
		{
			element->load();
			if (element->error()) return 0;
		}
        return element;
    }

    LineItem *Map::getLineItem(const mongo::OID& mongoId)
    {
        std::string mongoIdStr = mongoId.toString();
        std::map<std::string, LineItem *>::iterator itemIt = _lineItemMap.find(mongoIdStr);

        if (itemIt == _lineItemMap.end())
        {
			bool lineOk = false;
			std::unique_ptr<mongo::DBClientCursor> cursor = _connectionPtr->query("diaphnea.items", MONGO_QUERY("_id" << mongoId), 1);
            if (cursor->more())
            {
                mongo::BSONObj dbItem = cursor->next();

                int itemId = dbItem.getIntField("item_id");
                int cap1Round = dbItem.getIntField("cap1_round");
                int cap2Round = dbItem.getIntField("cap2_round");

				if (itemId >= 0 && itemId <= _maxIntDbValue && (cap1Round == 0 || cap1Round == 1) && (cap2Round == 0 || cap2Round == 1))
				{
					LineItem *item = new LineItem(itemId, _sampleLengthVector.size(), cap1Round != 0, cap2Round != 0);
					if (addPointLists(item, dbItem))
					{
                        lineOk = true;
                        itemIt = _lineItemMap.insert(std::pair<std::string, LineItem *>(mongoIdStr, item)).first;
                        _itemMap.insert(std::pair<int, MapItem *>(itemId, item));
					}
					else
					{
                        delete item;
					}
				}
            }

            if (!lineOk)
            {
                itemIt = _lineItemMap.insert(std::pair<std::string, LineItem *>(mongoIdStr, 0)).first;
				_errorVector.push_back(new DatabaseError(__FILE__, __func__, __LINE__));
            }
        }

        return (*itemIt).second;
    }

    FilledPolygonItem *Map::getFilledPolygonItem(const mongo::OID& mongoId)
    {
        std::string mongoIdStr = mongoId.toString();
        std::map<std::string, FilledPolygonItem *>::iterator itemIt = _filledPolygonItemMap.find(mongoIdStr);

        if (itemIt == _filledPolygonItemMap.end())
        {
            bool polygonOk = false;
			std::unique_ptr<mongo::DBClientCursor> cursor = _connectionPtr->query("diaphnea.items", MONGO_QUERY("_id" << mongoId), 1);
            if (cursor->more())
            {
                mongo::BSONObj dbItem = cursor->next();

                int itemId = dbItem.getIntField("item_id");
                if (itemId >= 0 && itemId <= _maxIntDbValue)
				{
					FilledPolygonItem *item = new FilledPolygonItem(itemId, _sampleLengthVector.size());
                    if (addPointLists(item, dbItem))
                    {
                        polygonOk = true;
                        itemIt = _filledPolygonItemMap.insert(std::pair<std::string, FilledPolygonItem *>(mongoIdStr, item)).first;
                        _itemMap.insert(std::pair<int, MapItem *>(itemId, item));
                    }
                    else
                    {
                        delete item;
                    }
                }
            }

            if (!polygonOk)
            {
                itemIt = _filledPolygonItemMap.insert(std::pair<std::string, FilledPolygonItem *>(mongoIdStr, 0)).first;
                _errorVector.push_back(new DatabaseError(__FILE__, __func__, __LINE__));
            }
        }

        return (*itemIt).second;
    }

    bool Map::addPointLists(MultipointsItem *item, mongo::BSONObj& dbItem)
    {
        mongo::BSONElement pointListsElt = dbItem.getField("point_lists");
        if (pointListsElt.type() != mongo::Array)
        {
            _errorVector.push_back(new DatabaseError(__FILE__, __func__, __LINE__));
            return false;
        }
        std::vector<mongo::BSONElement> pointListIdVector = pointListsElt.Array();
        if (pointListIdVector.empty())
        {
            _errorVector.push_back(new DatabaseError(__FILE__, __func__, __LINE__));
            return false;
        }

        int i, n = pointListIdVector.size();
        for (i = 0; i < n; ++i)
        {
			std::unique_ptr<mongo::DBClientCursor> cursor = _connectionPtr->query("diaphnea.point_lists", MONGO_QUERY("_id" << pointListIdVector[i].OID()), 1);
            if (cursor->more())
            {
                mongo::BSONObj pointList = cursor->next();

                mongo::BSONElement pointsElt = pointList.getField("points");
                if (pointsElt.type() != mongo::Array)
                {
                    _errorVector.push_back(new DatabaseError(__FILE__, __func__, __LINE__));
                    return false;
                }
                std::vector<mongo::BSONElement> pointVector = pointsElt.Array();
                int m = pointVector.size();
                if (m < 2)
                {
                    _errorVector.push_back(new DatabaseError(__FILE__, __func__, __LINE__));
                    return false;
                }

                int j;
                for (j = 0; j < m; ++j)
                {
                    mongo::BSONObj dbPoint = pointVector[j].Obj();

                    mongo::BSONElement xElt = dbPoint.getField("x");
                    mongo::BSONElement yElt = dbPoint.getField("y");
                    if (xElt.type() != mongo::NumberDouble || yElt.type() != mongo::NumberDouble)
                    {
                        _errorVector.push_back(new DatabaseError(__FILE__, __func__, __LINE__));
                        return false;
                    }

                    double x = xElt.Double();
                    double y = yElt.Double();

                    Point *point;
                    if (j == 0)
                    {
                        point = new Point(x, -y);
                    }
                    else
                    {
                        mongo::BSONElement x1Elt = dbPoint.getField("x1");
                        mongo::BSONElement y1Elt = dbPoint.getField("y1");
                        mongo::BSONElement x2Elt = dbPoint.getField("x2");
                        mongo::BSONElement y2Elt = dbPoint.getField("y2");

                        if (x1Elt.type() != mongo::NumberDouble || y1Elt.type() != mongo::NumberDouble ||
                            x2Elt.type() != mongo::NumberDouble || y2Elt.type() != mongo::NumberDouble)
                        {
                            _errorVector.push_back(new DatabaseError(__FILE__, __func__, __LINE__));
                            return false;
                        }

                        double x1 = x1Elt.Double();
                        double y1 = y1Elt.Double();
                        double x2 = x2Elt.Double();
                        double y2 = y2Elt.Double();

                        point = new Point(x1, -y1, x2, -y2, x, -y);
                    }

                    item->addPoint(i, _sampleLengthVector[i], point);
                }
            }
            else
            {
                _errorVector.push_back(new DatabaseError(__FILE__, __func__, __LINE__));
                return false;
            }
        }

        item->setInfoJsonVector();

        return true;
    }

    void Map::addPointItem(PointItem *pointItem)
    {
        _itemMap.insert(std::pair<int, MapItem *>(pointItem->getId(), pointItem));
    }

    MapItem *Map::getItem(int itemId)
    {
        std::map<int, MapItem *>::iterator it1 = _itemMap.find(itemId);
        if (it1 != _itemMap.end()) return (*it1).second;

        if (!_itemToElement0MapLoaded) loadItemToElement0Map();

        std::map<int, std::string>::iterator it2 = _itemToElement0Map.find(itemId);
        if (it2 != _itemToElement0Map.end())
        {
            MapElement *element0 = getElement((*it2).second);
            if (element0 != 0)
            {
                it1 = _itemMap.find(itemId);
                if (it1 != _itemMap.end()) return (*it1).second;
            }
        }

        return 0;
    }

    void Map::loadItemToElement0Map(void)
    {
        _itemToElement0MapLoaded = true;

        mongo::BSONObj projection = BSON("item_id" << 1 << "element0" << 1);

        bool found = false;
        std::unique_ptr<mongo::DBClientCursor> cursor = _connectionPtr->query("diaphnea.items", MONGO_QUERY("map" << _id), 0, 0, &projection);
        while (cursor->more())
        {
            found = true;
            mongo::BSONObj dbItem = cursor->next();
            std::string element0 = dbItem.getStringField("element0");
            int id = dbItem.getIntField("item_id");

            if (id >= 0 && id <= _maxIntDbValue && !element0.empty()) _itemToElement0Map.insert(std::pair<int, std::string>(id, element0));
            else _errorVector.push_back(new DatabaseError(__FILE__, __func__, __LINE__));
        }

        if (!found) _errorVector.push_back(new DatabaseError(__FILE__, __func__, __LINE__));
    }

    const Look *Map::getLook(int lookId) const
    {
        std::map<int, const Look *>::const_iterator it = _lookMap.find(lookId);
        if (it != _lookMap.end()) return (*it).second;
        return 0;
    }

    void Map::addItemLook(const ItemLook *look)
    {
        _itemLookMap.insert(std::pair<int, const ItemLook *>(look->getId(), look));
    }

    const ItemLook *Map::getItemLook(int lookId) const
    {
        std::map<int, const ItemLook *>::const_iterator it = _itemLookMap.find(lookId);
        if (it != _itemLookMap.end()) return (*it).second;
        return 0;
    }

    int Map::getResolutionIndex(double scale)
    {
        double wantedLength = _resolutionThreshold / scale;
        int i, n = _sampleLengthVector.size();
        for (i = n - 1; i > 0; --i)
        {
            if (wantedLength > _sampleLengthVector[i]) break;
        }

        return i;
    }

    void Map::load(void)
    {
        _loaded = true;
        std::string elementIdsJson, languagesJson, namesJson, categoriesJson;

        if (!loadElements(elementIdsJson))
        {
            _error = true;
            return;
        }

        std::unique_ptr<mongo::DBClientCursor> cursor = _connectionPtr->query("diaphnea.maps", MONGO_QUERY("_id" << _mongoId), 1);
        if (cursor->more())
        {
            mongo::BSONObj dbMap = cursor->next();

            mongo::BSONElement zoomMinDistanceElt = dbMap.getField("zoom_min_distance");
            mongo::BSONElement zoomMaxDistanceElt = dbMap.getField("zoom_max_distance");
            mongo::BSONElement resolutionThresholdElt = dbMap.getField("resolution_threshold");
            mongo::BSONElement sizeParameter1Elt = dbMap.getField("size_parameter1");
            mongo::BSONElement sizeParameter2Elt = dbMap.getField("size_parameter2");

            if (zoomMinDistanceElt.type() != mongo::NumberDouble || zoomMaxDistanceElt.type() != mongo::NumberDouble ||
                resolutionThresholdElt.type() != mongo::NumberDouble || sizeParameter1Elt.type() != mongo::NumberDouble || sizeParameter2Elt.type() != mongo::NumberDouble)
            {
                _error = true;
                _errorVector.push_back(new DatabaseError(__FILE__, __func__, __LINE__));
				return;
            }

            _zoomMinDistance = zoomMinDistanceElt.Double();
            _zoomMaxDistance = zoomMaxDistanceElt.Double();
            _resolutionThreshold = resolutionThresholdElt.Double();
            _sizeParameter1 = sizeParameter1Elt.Double();
            _sizeParameter2 = sizeParameter2Elt.Double();

            if (!loadNameAndLanguages(dbMap, languagesJson, namesJson))
            {
                _error = true;
                return;
            }

            if (!loadResolutions(dbMap))
            {
                _error = true;
                return;
            }

            if (!loadLooks(dbMap))
            {
                _error = true;
                return;
            }

            if (!loadCategories(dbMap, categoriesJson))
            {
                _error = true;
                return;
            }

            if (elementIdsJson.empty()) elementIdsJson = "null";
            if (languagesJson.empty()) languagesJson = "null";
            if (namesJson.empty()) namesJson = "null";
            if (categoriesJson.empty()) categoriesJson = "null";

            std::stringstream jsonStream;
            jsonStream << "{\"elementIds\":" << elementIdsJson << ",\"languages\":" << languagesJson
                       << ",\"names\":" << namesJson << ",\"categories\":" << categoriesJson
                       << ",\"zoomMinDistance\":" << _zoomMinDistance << ",\"zoomMaxDistance\":" << _zoomMaxDistance
                       << ",\"sizeParameter1\":" << _sizeParameter1 << ",\"sizeParameter2\":" << _sizeParameter2 << "}";
            _infoJson = jsonStream.str();
        }
    }

    bool Map::loadElements(std::string& elementIdsJson)
    {
        mongo::BSONObj projection = BSON("id" << 1 << "item_id" << 1);

		std::unique_ptr<mongo::DBClientCursor> cursor = _connectionPtr->query("diaphnea.point_elements", MONGO_QUERY("map" << _id), 0, 0, &projection);
        while (cursor->more())
        {
            mongo::BSONObj dbElement = cursor->next();
            std::string id = dbElement.getStringField("id");
            int itemId = dbElement.getIntField("item_id");
            mongo::BSONElement mongoIdElt = dbElement.getField("_id");

            if (itemId >= 0 && itemId <= _maxIntDbValue && !id.empty() && mongoIdElt.type() == mongo::jstOID)
            {
                PointElement *element = new PointElement(mongoIdElt.OID(), id, this);
                _elementMap.insert(std::pair<std::string, MapElement *>(element->getId(), element));

                if (elementIdsJson.empty()) elementIdsJson = "[\"";
                else elementIdsJson += "\",\"";
                elementIdsJson += element->getId();

                _itemToElement0Map.insert(std::pair<int, std::string>(itemId, id));
            }
            else
            {
                _errorVector.push_back(new DatabaseError(__FILE__, __func__, __LINE__));
                return false;
            }
        }

        cursor = _connectionPtr->query("diaphnea.polygon_elements", MONGO_QUERY("map" << _id), 0, 0, &projection);
        while (cursor->more())
        {
            mongo::BSONObj dbElement = cursor->next();
            std::string id = dbElement.getStringField("id");
            mongo::BSONElement mongoIdElt = dbElement.getField("_id");

            if (!id.empty() && mongoIdElt.type() == mongo::jstOID)
            {
                PolygonElement *element = new PolygonElement(mongoIdElt.OID(), id, this);
                _elementMap.insert(std::pair<std::string, MapElement *>(element->getId(), element));

                if (elementIdsJson.empty()) elementIdsJson = "[\"";
                else elementIdsJson += "\",\"";
                elementIdsJson += element->getId();
            }
            else
            {
                _errorVector.push_back(new DatabaseError(__FILE__, __func__, __LINE__));
                return false;
            }
        }

        cursor = _connectionPtr->query("diaphnea.line_elements", MONGO_QUERY("map" << _id), 0, 0, &projection);
        while (cursor->more())
        {
            mongo::BSONObj dbElement = cursor->next();
            std::string id = dbElement.getStringField("id");
            mongo::BSONElement mongoIdElt = dbElement.getField("_id");

            if (!id.empty() && mongoIdElt.type() == mongo::jstOID)
            {
                LineElement *element = new LineElement(mongoIdElt.OID(), id, this);
                _elementMap.insert(std::pair<std::string, MapElement *>(element->getId(), element));

                if (elementIdsJson.empty()) elementIdsJson = "[\"";
                else elementIdsJson += "\",\"";
                elementIdsJson += element->getId();
            }
            else
            {
                _errorVector.push_back(new DatabaseError(__FILE__, __func__, __LINE__));
                return false;
            }
        }

        if (!elementIdsJson.empty())
        {
            elementIdsJson += "\"]";
        }
        else
        {
            _errorVector.push_back(new DatabaseError(__FILE__, __func__, __LINE__));
            return false;
        }

        return true;
    }

    bool Map::loadNameAndLanguages(mongo::BSONObj dbMap, std::string& languagesJson, std::string& namesJson)
    {
        mongo::BSONElement nameElt = dbMap.getField("name");
        if (nameElt.type() != mongo::Object)
        {
            _errorVector.push_back(new DatabaseError(__FILE__, __func__, __LINE__));
            return false;
        }
        mongo::BSONObj dbName = nameElt.Obj();

        mongo::BSONElement languagesElt = dbMap.getField("languages");
        if (languagesElt.type() != mongo::Array)
        {
            _errorVector.push_back(new DatabaseError(__FILE__, __func__, __LINE__));
            return false;
        }
        std::vector<mongo::BSONElement> dbLanguageVector = languagesElt.Array();
        if (dbLanguageVector.empty())
        {
            _errorVector.push_back(new DatabaseError(__FILE__, __func__, __LINE__));
            return false;
        }

        int i, n = dbLanguageVector.size();
        if (n == 0)
        {
            _errorVector.push_back(new DatabaseError(__FILE__, __func__, __LINE__));
            return false;
        }

        for (i = 0; i < n; ++i)
        {
            mongo::BSONElement languageElt = dbLanguageVector[i];
            if (languageElt.type() != mongo::Object)
            {
                _errorVector.push_back(new DatabaseError(__FILE__, __func__, __LINE__));
                return false;
            }
            mongo::BSONObj dbLanguage = languageElt.Obj();

            std::string languageId = dbLanguage.getStringField("id");
            std::string languageName = dbLanguage.getStringField("name");
            if (languageId.empty() || languageName.empty())
            {
                _errorVector.push_back(new DatabaseError(__FILE__, __func__, __LINE__));
                return false;
            }

            _languageNameMap.insert(std::pair<std::string, std::string>(languageId, languageName));
            _languageIdVector.push_back(languageId);
            _languageIdSet.insert(languageId);

            const char *mapName = dbName.getStringField(languageId);
            _nameMap.insert(std::pair<std::string, std::string>(languageId, mapName));

            if (languagesJson.empty()) languagesJson = "[{\"id\":\"";
            else languagesJson += "\"},{\"id\":\"";
            languagesJson += std::string(languageId) + "\",\"name\":\"" + languageName;

            if (namesJson.empty()) namesJson = "{\"";
            else namesJson += "\",\"";
            namesJson += std::string(languageId) + "\":\"" + mapName;
        }
        if (!languagesJson.empty()) languagesJson += "\"}]";
        if (!namesJson.empty()) namesJson += "\"}";

        return true;
    }

    bool Map::loadResolutions(mongo::BSONObj dbMap)
    {
        mongo::BSONElement resolutionsElt = dbMap.getField("resolutions");
        if (resolutionsElt.type() != mongo::Array)
        {
            _errorVector.push_back(new DatabaseError(__FILE__, __func__, __LINE__));
            return false;
        }
        std::vector<mongo::BSONElement> dbResolutionVector = resolutionsElt.Array();

        _sampleLengthVector.resize(dbResolutionVector.size());
        int i, n = dbResolutionVector.size();
        for (i = 0; i < n; ++i)
        {
            mongo::BSONElement resolutionElt = dbResolutionVector[i];
            if (resolutionElt.type() != mongo::Object)
            {
                _errorVector.push_back(new DatabaseError(__FILE__, __func__, __LINE__));
                return false;
            }
            mongo::BSONObj dbResolution = resolutionElt.Obj();

            unsigned int index = dbResolution.getIntField("index");
            mongo::BSONElement sampleLengthElt = dbResolution.getField("sample_length");
            if (sampleLengthElt.type() != mongo::NumberDouble || index >= _sampleLengthVector.size())
            {
                _errorVector.push_back(new DatabaseError(__FILE__, __func__, __LINE__));
                return false;
            }

            _sampleLengthVector[index] = sampleLengthElt.Double();
        }

        return true;
    }

    bool Map::loadLooks(mongo::BSONObj dbMap)
    {
        mongo::BSONElement looksElt = dbMap.getField("looks");
        if (looksElt.type() != mongo::Array)
        {
            _errorVector.push_back(new DatabaseError(__FILE__, __func__, __LINE__));
            return false;
        }
        std::vector<mongo::BSONElement> dbLookVector = looksElt.Array();

        int i, n = dbLookVector.size();
        for (i = 0; i < n; ++i)
        {
            mongo::BSONElement lookElt = dbLookVector[i];
            if (lookElt.type() != mongo::Object)
            {
                _errorVector.push_back(new DatabaseError(__FILE__, __func__, __LINE__));
                return false;
            }
            mongo::BSONObj dbLook = lookElt.Obj();

            int id = dbLook.getIntField("id");
            std::string type = dbLook.getStringField("type");
            int textAlpha = dbLook.getIntField("text_alpha");
            int textRed = dbLook.getIntField("text_red");
            int textGreen = dbLook.getIntField("text_green");
            int textBlue = dbLook.getIntField("text_blue");
            mongo::BSONElement textSizeElt = dbLook.getField("text_size");

            if (id < 0 || id > _maxIntDbValue || type.empty() || textAlpha < 0 || textAlpha > 255 || textRed < 0 || textRed > 255 ||
                textGreen < 0 || textGreen > 255 || textBlue < 0 || textBlue > 255 || textSizeElt.type() != mongo::NumberDouble)
            {
                _errorVector.push_back(new DatabaseError(__FILE__, __func__, __LINE__));
                return false;
            }

            std::string textNameJson;
            if (!getLookNameJson(dbLook, "text_name", textNameJson)) return false;

            if (type == "point")
            {
                int pointZIndex = dbLook.getIntField("point_z_index");
                int pointAlpha = dbLook.getIntField("point_alpha");
                int pointRed = dbLook.getIntField("point_red");
                int pointGreen = dbLook.getIntField("point_green");
                int pointBlue = dbLook.getIntField("point_blue");
                mongo::BSONElement pointSizeElt = dbLook.getField("point_size");

                if (pointZIndex < -_maxIntDbValue || pointZIndex > _maxIntDbValue || pointAlpha < 0 || pointAlpha > 255 || pointRed < 0 || pointRed > 255 ||
                    pointGreen < 0 || pointGreen > 255 || pointBlue < 0 || pointBlue > 255 || pointSizeElt.type() != mongo::NumberDouble)
                {
                    _errorVector.push_back(new DatabaseError(__FILE__, __func__, __LINE__));
                    return false;
                }

                std::string pointNameJson;
                if (!getLookNameJson(dbLook, "point_name", pointNameJson)) return false;

                PointLook *look = new PointLook(id, textAlpha, textRed, textGreen, textBlue, textSizeElt.Double(), textNameJson, pointZIndex, pointAlpha,
                                                pointRed, pointGreen, pointBlue, pointSizeElt.Double(), pointNameJson, this);
                _lookMap.insert(std::pair<int, const Look *> (id, look));
            }
            else if (type == "line")
            {
                int lineZIndex = dbLook.getIntField("line_z_index");
                int lineAlpha = dbLook.getIntField("line_alpha");
                int lineRed = dbLook.getIntField("line_red");
                int lineGreen = dbLook.getIntField("line_green");
                int lineBlue = dbLook.getIntField("line_blue");
                mongo::BSONElement lineSizeElt = dbLook.getField("line_size");

                if (lineZIndex < -_maxIntDbValue || lineZIndex > _maxIntDbValue || lineAlpha < 0 || lineAlpha > 255 || lineRed < 0 || lineRed > 255 ||
                    lineGreen < 0 || lineGreen > 255 || lineBlue < 0 || lineBlue > 255 || lineSizeElt.type() != mongo::NumberDouble)
                {
                    _errorVector.push_back(new DatabaseError(__FILE__, __func__, __LINE__));
                    return false;
                }

                std::string lineNameJson;
                if (!getLookNameJson(dbLook, "line_name", lineNameJson)) return false;

                LineLook *look = new LineLook(id, textAlpha, textRed, textGreen, textBlue, textSizeElt.Double(), textNameJson, lineZIndex, lineAlpha,
                                              lineRed, lineGreen, lineBlue, lineSizeElt.Double(), lineNameJson, this);
                _lookMap.insert(std::pair<int, const Look *> (id, look));
            }
            else if (type == "polygon")
            {
                int contourZIndex = dbLook.getIntField("contour_z_index");
                int contourAlpha = dbLook.getIntField("contour_alpha");
                int contourRed = dbLook.getIntField("contour_red");
                int contourGreen = dbLook.getIntField("contour_green");
                int contourBlue = dbLook.getIntField("contour_blue");
                mongo::BSONElement contourSizeElt = dbLook.getField("contour_size");
                int fillZIndex = dbLook.getIntField("fill_z_index");
                int fillAlpha = dbLook.getIntField("fill_alpha");
                int fillRed = dbLook.getIntField("fill_red");
                int fillGreen = dbLook.getIntField("fill_green");
                int fillBlue = dbLook.getIntField("fill_blue");

                if (contourZIndex < -_maxIntDbValue || contourZIndex > _maxIntDbValue || contourAlpha < 0 || contourAlpha > 255 || contourRed < 0 || contourRed > 255 ||
                    contourGreen < 0 || contourGreen > 255 || contourBlue < 0 || contourBlue > 255 || contourSizeElt.type() != mongo::NumberDouble ||
                    fillZIndex < -_maxIntDbValue || fillZIndex > _maxIntDbValue || fillAlpha < 0 || fillAlpha > 255 || fillRed < 0 || fillRed > 255 ||
                    fillGreen < 0 || fillGreen > 255 || fillBlue < 0 || fillBlue > 255)
                {
                    _errorVector.push_back(new DatabaseError(__FILE__, __func__, __LINE__));
                    return false;
                }

                std::string contourNameJson;
                if (!getLookNameJson(dbLook, "contour_name", contourNameJson)) return false;

                std::string fillNameJson;
                if (!getLookNameJson(dbLook, "fill_name", fillNameJson)) return false;

                PolygonLook *look = new PolygonLook(id, textAlpha, textRed, textGreen, textBlue, textSizeElt.Double(), textNameJson,
                                                    contourZIndex, contourAlpha, contourRed, contourGreen, contourBlue, contourSizeElt.Double(), contourNameJson,
                                                    fillZIndex, fillAlpha, fillRed, fillGreen, fillBlue, fillNameJson, this);
                _lookMap.insert(std::pair<int, const Look *> (id, look));
            }
        }

        return true;
    }

    bool Map::getLookNameJson(mongo::BSONObj dbLook, const std::string& fieldName, std::string& json)
    {
        mongo::BSONElement nameElt = dbLook.getField(fieldName);
        if (nameElt.type() != mongo::Object)
        {
            _errorVector.push_back(new DatabaseError(__FILE__, __func__, __LINE__));
            return false;
        }
        mongo::BSONObj dbName = nameElt.Obj();

        int j, m = _languageIdVector.size();
        for (j = 0; j < m; ++j)
        {
            std::string lookName = dbName.getStringField(_languageIdVector[j]);

            if (lookName.empty())
            {
                _errorVector.push_back(new DatabaseError(__FILE__, __func__, __LINE__));
                return false;
            }
            else
            {
                if (j == 0) json += "{\"";
                else json += "\",\"";
                json += std::string(_languageIdVector[j]) + "\":\"" + lookName;
            }
        }
        json += "\"}";

        return true;
    }

    bool Map::loadCategories(mongo::BSONObj dbMap, std::string& categoriesJson)
    {
        mongo::BSONElement categoriesElt = dbMap.getField("categories");
        if (categoriesElt.type() != mongo::Array)
        {
            _errorVector.push_back(new DatabaseError(__FILE__, __func__, __LINE__));
            return false;
        }
        std::vector<mongo::BSONElement> dbCategoryVector = categoriesElt.Array();

        int i, n = dbCategoryVector.size();
        if (n == 0)
        {
            _errorVector.push_back(new DatabaseError(__FILE__, __func__, __LINE__));
            return false;
        }

        for (i = 0; i < n; ++i)
        {
            mongo::BSONElement categoryElt = dbCategoryVector[i];
            if (categoryElt.type() != mongo::Object)
            {
                _errorVector.push_back(new DatabaseError(__FILE__, __func__, __LINE__));
                return false;
            }
            mongo::BSONObj dbCategory = categoryElt.Obj();

            int id = dbCategory.getIntField("id");
            if (id < 0 || id > _maxIntDbValue )
            {
                _errorVector.push_back(new DatabaseError(__FILE__, __func__, __LINE__));
                return false;
            }

            mongo::BSONElement nameElt = dbCategory.getField("name");
            if (nameElt.type() != mongo::Object)
            {
                _errorVector.push_back(new DatabaseError(__FILE__, __func__, __LINE__));
                return false;
            }
            mongo::BSONObj dbName = nameElt.Obj();

            if (i == 0) categoriesJson += "[";
            else categoriesJson += ",";

            int j, m = _languageIdVector.size();
            for (j = 0; j < m; ++j)
            {
                std::string categoryName = dbName.getStringField(_languageIdVector[j]);

                if (categoryName.empty())
                {
                    _errorVector.push_back(new DatabaseError(__FILE__, __func__, __LINE__));
                    return false;
                }
                else
                {
                    if (j == 0) categoriesJson += "{\"";
                    else categoriesJson += "\",\"";
                    categoriesJson += std::string(_languageIdVector[j]) + "\":\"" + categoryName;
                }
            }
            categoriesJson += "\"}";
        }
        categoriesJson += "]";

        return true;
    }
}
