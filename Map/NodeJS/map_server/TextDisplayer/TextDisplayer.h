#pragma once

#include "Potential.h"
#include "TextPotential.h"

#include <vector>
#include <map>
#include <string>
#include <mutex>

namespace map_server
{
    class ItemCopy;
    class PointItemCopy;
    class LineItemCopy;
    class FilledPolygonItemCopy;
	class TextDisplayerParameters;
	class TextInfo;
	class SvgCreator;

    class TextDisplayer
    {
    private:
		static std::mutex _mutex;
		static int _counter;
		static std::map<std::string, int *> _clientActiveDisplayerMap;
		int _id;
		int *_clientActiveDisplayerId;
		bool isDisplayerActive(void);

		static std::mutex *_coutMutexPtr;

        const TextDisplayerParameters * const _parameters;
		const std::string _socketId;
        const char * const _requestId;

        const double _width;
        const double _height;
        const double _xFocus;
        const double _yFocus;
        const double _scale;
        const bool _createPotentialImage;
        SvgCreator * const _svgCreator;


        std::vector<ItemCopy *> _itemVector;

        bool displayText(ItemCopy *item, TextInfo *textInfo);
        bool displayPointText(PointItemCopy *item, TextInfo *textInfo);
        bool displayLineText(LineItemCopy *item, TextInfo *textInfo);
        bool displayFilledPolygonText(FilledPolygonItemCopy *item, TextInfo *textInfo);

        void sendResponse(ItemCopy *item, TextInfo *textInfo);

        TextPotential getTextPotential(ItemCopy *item, TextInfo *textInfo, double x, double y, bool selfRepulsion);
        Potential getPotential(double x, double y, ItemCopy *selfItem);
        Potential getElementaryPotential(ItemCopy *item, double x, double y);

        void hsvToRgb(double h, double s, double v, double& r, double& g, double& b);

    public:
        static void setCoutMutex(std::mutex *coutMutexPtr) { _coutMutexPtr = coutMutexPtr; }
		static void clearClientMap(void);

        TextDisplayer(const TextDisplayerParameters *parameters, const std::string& socketId, const char *requestId,
            double width, double height, double xFocus, double yFocus, double scale, bool createPotentialImage, SvgCreator *svgCreator);
		~TextDisplayer();

        void addItem(ItemCopy *item) { _itemVector.push_back(item); }
        bool start(void);
    };
}
