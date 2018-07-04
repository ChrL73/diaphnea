/* --------------------------------------------------------------------
 *
 * Copyright (C) 2018
 *
 * This file is part of the Diaphnea project.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License
 * version 3 as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * -------------------------------------------------------------------- */

#pragma once

#include <vector>

#include "Request.h"

namespace map_server
{
    class CommonData;

    class ElementsInfoRequest : public Request
    {
    private:
        CommonData * const _commonData;
        const std::vector<const char *> _elementIds;

        void execute(void);

    public:
        ElementsInfoRequest(CommonData *commonData, const char *socketId, const char *requestId, const std::vector<const char *>& elementIds) :
            Request(socketId, requestId), _commonData(commonData), _elementIds(elementIds) {}
    };
}

