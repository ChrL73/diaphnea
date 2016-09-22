﻿using MongoDB.Bson;
using MongoDB.Driver;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MapDataProcessing
{
    class PolygonLinePart
    {
        private static Dictionary<KmlFileData, PolygonLinePart> _partDictionary = new Dictionary<KmlFileData, PolygonLinePart>();

        internal static PolygonLinePart getPart(KmlFileData lineData, KmlFileData pointData1, KmlFileData pointData2)
        {
            PolygonLinePart part;
            if (!_partDictionary.TryGetValue(lineData, out part))
            {
                part = new PolygonLinePart(lineData, pointData1, pointData2);
                _partDictionary.Add(lineData, part);
            }
            else
            {
                if (pointData1 != part._pointData1)
                {
                    MessageLogger.addMessage(XmlLogLevelEnum.ERROR, String.Format("Different point1s ('{0}' and '{1}') for line polygon part '{2}'",
                                             pointData1, part._pointData1, lineData));
                    return null;
                }
                else if (pointData2 != part._pointData2)
                {
                    MessageLogger.addMessage(XmlLogLevelEnum.ERROR, String.Format("Different point2s ('{0}' and '{1}') for line polygon part '{2}'",
                                             pointData2, part._pointData2, lineData));
                    return null;
                }
            }

            return part;
        }

        private readonly KmlFileData _lineData;
        private readonly KmlFileData _pointData1;
        private readonly KmlFileData _pointData2;
        private readonly Dictionary<XmlResolution, DatabasePointList> _smoothedLineDictionary = new Dictionary<XmlResolution, DatabasePointList>();

        private PolygonLinePart(KmlFileData lineData, KmlFileData pointData1, KmlFileData pointData2)
        {
            _lineData = lineData;
            _pointData1 = pointData1;
            _pointData2 = pointData2;
        }

        internal KmlFileData Line { get { return _lineData; } }
        internal KmlFileData Point1 { get { return _pointData1; } }
        internal KmlFileData Point2 { get { return _pointData2; } }

        internal List<GeoPoint> getPointList(XmlResolution resolution)
        {
            DatabasePointList list;
            if (_smoothedLineDictionary.TryGetValue(resolution, out list)) return list.PointList;
            return null;
        }

        internal static int smoothAll(MapData mapData)
        {
            foreach (PolygonLinePart part in _partDictionary.Values)
            {
                List<GeoPoint> line = new List<GeoPoint>();
                line.Add(part.Point1.PointList[0]);
                line.AddRange(part.Line.PointList);
                line.Add(part.Point2.PointList[0]);

                foreach (XmlResolution resolution in mapData.XmlMapData.resolutionList)
                {
                    List<GeoPoint> smoothedLine = Smoother.smoothLine(line, resolution, part.Line.Path);
                    if (smoothedLine == null) return -1;
                    part._smoothedLineDictionary.Add(resolution, new DatabasePointList(smoothedLine));
                    if (KmlWriter.write(smoothedLine, KmlFileTypeEnum.LINE, "Polygons_Lines", Path.GetFileName(part.Line.Path), resolution) != 0) return -1;
                }
            }

            return 0;
        }

        internal static int fillDatabase(IMongoDatabase database, MapData mapData)
        {
            IMongoCollection<BsonDocument> pointListCollection = database.GetCollection<BsonDocument>("point_lists");

            foreach (PolygonLinePart part in _partDictionary.Values)
            {
                foreach (KeyValuePair<XmlResolution, DatabasePointList> pair in part._smoothedLineDictionary)
                {
                    DatabasePointList list = pair.Value;
                    double resolution = pair.Key.sampleLength1 * Double.Parse(pair.Key.sampleRatio);

                    BsonArray pointArray = new BsonArray();
                    foreach (GeoPoint point in list.PointList)
                    {
                        pointArray.Add(point.getBsonDocument());
                    }

                    BsonDocument pointListDocument = new BsonDocument()
                    {
                        { "map", mapData.XmlMapData.parameters.mapId },
                        { "item", Path.GetFileNameWithoutExtension(part.Line.Path) },
                        { "resolution", resolution },
                        { "count", list.PointList.Count },
                        { "points", pointArray }
                    };

                    pointListCollection.InsertOne(pointListDocument);
                    list.Id = pointListDocument.GetValue("_id");
                }
            }

            return 0;
        }
    }
}