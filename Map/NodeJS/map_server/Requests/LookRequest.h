#pragma once

#include "Request.h"

namespace map_server
{
    class LookRequest : public Request
    {
    private:
        const char * const _mapId;
        const int _lookId;

        void execute(void);

    public:
        LookRequest(const char *socketId, const char *requestId, const char *mapId, int lookId, bool sendResponse) :
            Request(socketId, requestId, sendResponse), _mapId(mapId), _lookId(lookId) {}
    };
}
