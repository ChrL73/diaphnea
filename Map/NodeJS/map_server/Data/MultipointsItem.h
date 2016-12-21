#pragma once

#include "MapItem.h"

#include <vector>
#include <map>
#include <sstream>

namespace map_server
{
    class Point;

    class MultipointsItem : public MapItem
    {
    private:
        std::map<double, std::vector<const Point *> > _pointVectorMap;
        std::vector<std::vector<const Point *> > _pointVectorVector;
        bool hasResolution(void) const { return true; }

    protected:
        MultipointsItem(int id, int resolutionCount) : MapItem(id, resolutionCount), _resolutionCount(resolutionCount) {}

        const int _resolutionCount;
        void addPointArray(std::stringstream& s, int resolutionIndex);

    public:
		virtual ~MultipointsItem();

		const std::vector<const Point *>& getPointVector(int resolutionIndex) const { return _pointVectorVector[resolutionIndex]; }
        void addPoint(unsigned int resolutionIndex, double samplingLength, Point *point);
        virtual void setInfoJsonVector(void) = 0;
    };
}