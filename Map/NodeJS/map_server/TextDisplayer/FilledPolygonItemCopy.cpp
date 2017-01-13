#include "FilledPolygonItemCopy.h"
#include "Point.h"

#include <cmath>

namespace map_server
{
    FilledPolygonItemCopy::~FilledPolygonItemCopy()
    {
        int i, n = _pointVector.size();
        for (i = 0; i < n; ++i) delete _pointVector[i];

        for (i = 0; i < _height; ++i) delete _intersections[i];
        delete[] _intersections;
    }

    void FilledPolygonItemCopy::addPoint(double x, double y)
    {
        // Avoid the case where y is an integer (adding 0.01 pixel to y will not have any visible impact)
        double a;
        if (modf(y, &a) == 0.0) y += 0.01;

        _pointVector.push_back(new Point(x, y));
    }

    void FilledPolygonItemCopy::setIntersections(double height)
    {
        if (_intersections != 0) return;

        _yMin = height - 1.0;
        _yMax = 0.0;

        _height = static_cast<int>(floor(height));
        _intersections = new std::multiset<double> *[_height];
        int i;
        for (i = 0; i < _height; ++i) _intersections[i] = 0;

        int n = _pointVector.size();
        for (i = 0; i < n - 1; ++i)
        {
            const Point *point1 = _pointVector[i];
            const Point *point2 = _pointVector[i];
            double x1 = point1->getX();
            double y1 = point1->getY();
            double x2 = point2->getX();
            double y2 = point2->getY();

            if (y1 < _yMin) _yMin = y1;
            if (y1 > _yMax) _yMax = y1;

            double yMin, yMax;
            if (y1 > y2)
            {
                yMin = ceil(y1);
                yMax = floor(y2);
            }
            else
            {
                yMin = ceil(y2);
                yMax = floor(y1);
            }

            if (yMin <= yMax && yMax >= 0.0 && yMin <= height - 1.0)
            {
                if (yMin < 0.0 ) yMin = 0.0;
                if (yMax > _height - 1.0) yMax = _height - 1.0;

                double y;
                double a = (x2 - x1) / (y2- y1);
                for (y = yMin; y <= yMax; ++y)
                {
                    double x = x1 + a * (y - y1);
                    int yI = static_cast<int>(floor(y));
                    if (_intersections[yI] == 0) _intersections[yI] = new std::multiset<double>;
                    _intersections[yI]->insert(x);
                }
            }
        }

        if (_yMin < 0.0) _yMin = 0.0;
        if (_yMax > height - 1.0) _yMax = height - 1.0;
    }
}
