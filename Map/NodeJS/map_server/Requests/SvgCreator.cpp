#include "SvgCreator.h"
#include "SvgItemInfo.h"
#include "SvgTextInfo.h"
#include "SvgLineInfo.h"
#include "MapData.h"
#include "FilledPolygonItem.h"
#include "LineItem.h"
#include "PointItem.h"
#include "ItemLook.h"
#include "Point.h"
#include "BezierInfo.h"
#include "TextInfo.h"

#include <sstream>
#include <fstream>

namespace map_server
{
    SvgCreator::~SvgCreator()
    {
        std::multimap<int, SvgInfo *>::iterator it = _infoMap.begin();
        for (; it != _infoMap.end(); ++it) delete (*it).second;
    }

    void SvgCreator::execute(void)
    {
        std::stringstream content;

        content << "<?xml version=\"1.0\" encoding=\"utf-8\"?>" << std::endl
                << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" width=\"" << _widthInPixels
                << "\" height=\"" << _heightInPixels << "\">" <<std::endl;

        MapData::lock();

        std::multimap<int, SvgInfo *>::iterator it = _infoMap.begin();
        for (; it != _infoMap.end(); ++it)
        {
            SvgItemInfo *svgItemInfo = dynamic_cast<SvgItemInfo *>((*it).second);
            if (svgItemInfo != 0)
            {
                const MapItem *item = svgItemInfo->getItem();
                const ItemLook *look = svgItemInfo->getLook();
                int resolutionIndex = svgItemInfo->getResolutionIndex();

                const FilledPolygonItem *filledPolygonItem = dynamic_cast<const FilledPolygonItem *>(item);
                if (filledPolygonItem != 0)
                {
                    content << "<path style=\"fill:" << look->getHexColor()
                            << ";fill-opacity:" << static_cast<double>(look->getAlpha()) / 255.0
                            << ";stroke:none\" d=\"";

                    int i, n = filledPolygonItem->getPointVector(resolutionIndex).size();
                    for (i = 0; i < n; ++i)
                    {
                        const Point *point = filledPolygonItem->getPointVector(resolutionIndex)[i];
                        double x = (point->getX() - _xFocus) * _scale + 0.5 * _widthInPixels;
                        double y = (point->getY() - _yFocus) * _scale + 0.5 * _heightInPixels;

                        if (i == 0)
                        {
                            content << "M " << x << "," << y << " ";
                        }
                        else
                        {
                            double x1 = (point->getBezierInfo()->getX1() - _xFocus) * _scale + 0.5 * _widthInPixels;
                            double y1 = (point->getBezierInfo()->getY1() - _yFocus) * _scale + 0.5 * _heightInPixels;
                            double x2 = (point->getBezierInfo()->getX2() - _xFocus) * _scale + 0.5 * _widthInPixels;
                            double y2 = (point->getBezierInfo()->getY2() - _yFocus) * _scale + 0.5 * _heightInPixels;
                            content << "C " << x1 << " " << y1 << "," << x2 << " " << y2 << "," << x << " " << y << " ";
                        }
                    }

                    content << "\"></path>" << std::endl;
                }
                else
                {
                    const LineItem *lineItem = dynamic_cast<const LineItem *>(item);
                    if (lineItem != 0)
                    {
                        content << "<path style=\"fill:none;stroke:" << look->getHexColor()
                                << ";stroke-opacity:" << static_cast<double>(look->getAlpha()) / 255.0
                                << ";stroke-width:" << look->getSize() * _scale * _sizeFactor
                                << ";stroke-linecap:round;stroke-linejoin:round\" d=\"";

                        bool lastIn = false;
                        std::stringstream lastMove;
                        int i, n = lineItem->getPointVector(resolutionIndex).size();
                        for (i = 0; i < n; ++i)
                        {
                            const Point *point = lineItem->getPointVector(resolutionIndex)[i];
                            double x = (point->getX() - _xFocus) * _scale + 0.5 * _widthInPixels;
                            double y = (point->getY() - _yFocus) * _scale + 0.5 * _heightInPixels;
                            bool in = (x > -10.0 && x < _widthInPixels + 10.0 && y > -10.0 && y < _heightInPixels + 10.0);

                            if (i == 0 || (!in && !lastIn))
                            {
                                lastMove.str("");
                                lastMove << "M " << x << "," << y << " ";
                            }
                            else
                            {
                                double x1 = (point->getBezierInfo()->getX1() - _xFocus) * _scale + 0.5 * _widthInPixels;
                                double y1 = (point->getBezierInfo()->getY1() - _yFocus) * _scale + 0.5 * _heightInPixels;
                                double x2 = (point->getBezierInfo()->getX2() - _xFocus) * _scale + 0.5 * _widthInPixels;
                                double y2 = (point->getBezierInfo()->getY2() - _yFocus) * _scale + 0.5 * _heightInPixels;
                                content << lastMove.str() << "C " << x1 << " " << y1 << "," << x2 << " " << y2 << "," << x << " " << y << " ";
                                lastMove.str("");
                            }

                            lastIn = in;
                        }

                        content << "\"></path>" << std::endl;
                    }
                    else
                    {
                        const PointItem *pointItem = dynamic_cast<const PointItem *>(item);
                        if (pointItem != 0)
                        {
                            double x = (pointItem->getPoint()->getX() - _xFocus) * _scale + 0.5 * _widthInPixels;
                            double y = (pointItem->getPoint()->getY() - _yFocus) * _scale + 0.5 * _heightInPixels;

                            content << "<circle cx=\"" << x << "\" cy=\"" << y << "\" r=\"" << 0.5 * look->getSize() * _scale * _sizeFactor
                                    << "\" stroke=\"black\" stroke-width=\"" << _scale * _sizeFactor
                                    << "\" fill=\"" << look->getHexColor()
                                    << "\" fill-opacity=\"" << static_cast<double>(look->getAlpha()) / 255.0 << "\"></circle>" << std::endl;
                        }
                    }
                }
            }
            else
            {
                SvgTextInfo *svgTextInfo = dynamic_cast<SvgTextInfo *>((*it).second);
                if (svgTextInfo != 0)
                {
                    int i, n = svgTextInfo->getLineVector().size();
                    for (i = 0; i < n; ++i)
                    {
                        const SvgLineInfo *lineInfo = svgTextInfo->getLineVector()[i];

                        double x = (lineInfo->getX() - _xFocus) * _scale + 0.5 * _widthInPixels;
                        double y = (lineInfo->getY() - _yFocus) * _scale + 0.5 * _heightInPixels;

                        content << "<text x=\"" << x << "\" y=\"" << y << "\" font-size=\"" << svgTextInfo->getTextInfo()->getFontSize()
                                << "\" font-family=\"arial\" fill=\"" << svgTextInfo->getTextInfo()->getHexColor()
                                << "\" fill-opacity=\"" << static_cast<double>(svgTextInfo->getTextInfo()->getAlpha()) / 255.0
                                << "\">" << lineInfo->getText() << "</text>" << std::endl;
                    }
                }
            }
        }

        MapData::unlock();

        content << "</svg>";

        // Todo: Prevent different threads to write simultaneously the same file
		std::string fileName;
		if (_socketId.size() > 2) fileName = "map_" + _socketId.substr(2, _socketId.size() - 2) + ".svg";
		else fileName = "map_" + _socketId + ".svg";

        std::ofstream file(fileName);
        file << content.str();
    }
}
