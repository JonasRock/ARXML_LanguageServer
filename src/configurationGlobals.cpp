/**
 * @file configurationGlobals.cpp
 * @author Jonas Rock
 * @brief global state, will probably be moved
 * @version 0.1
 * @date 2020-10-27
 * 
 * 
 */

#include <memory>
#include "xmlParser.hpp"

namespace configurationGlobals
{
    bool receivedShutdownRequest = false;
    bool shutdownReady = false;
}