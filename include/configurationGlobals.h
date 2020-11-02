#ifndef CONFIGURATIONGLOBALS_H
#define CONFIGURATIONGLOBALS_H

/**
 * @file configurationGlobals.h
 * @author Jonas Rock
 * @brief globals declarations
 * @version 0.1
 * @date 2020-10-27
 * 
 * 
 */

#include "xmlParser.hpp"

namespace configurationGlobals
{
    const uint32_t maxOpenFiles = 5;
    extern bool receivedShutdownRequest;
    extern bool shutdownReady;
}

#endif /* CONFIGURATIONGLOBALS_H */
