/**
 * @file config.hpp
 * @author Jonas Rock
 * @brief Contains global configuration variables configurable in extension settings
 * @version 0.1
 * @date 2020-11-05
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <cstdint>

namespace lsp
{
    namespace config
    {
        extern uint32_t maxOpenFiles;
        extern bool precalculateOnOpeningFiles;
        extern bool shutdown;
    }
}

#endif /* CONFIG_H */