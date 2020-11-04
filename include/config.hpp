#ifndef CONFIG_H
#define CONFIG_H

#include <cstdint>

namespace lsp
{
    namespace config
    {
        extern uint32_t maxOpenFiles;
        extern bool precalculateOnOpeningFiles;
    }
}

#endif /* CONFIG_H */