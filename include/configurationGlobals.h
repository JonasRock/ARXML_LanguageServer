#ifndef CONFIGURATIONGLOBALS_H
#define CONFIGURATIONGLOBALS_H

#include "xmlParser.hpp"

namespace configurationGlobals
{
    extern bool receivedShutdownRequest;
    extern bool shutdownReady;
    extern std::shared_ptr<xmlParser> xParsePtr;
}

#endif /* CONFIGURATIONGLOBALS_H */
