#pragma once

#include "ItemCopy.h"

namespace map_server
{
    class LineItemCopy : public ItemCopy
    {
    public:
        LineItemCopy(const std::string& elementId, double importance) : ItemCopy(elementId, importance) {}
    };
}
