#include "config.hpp"

uint32_t lsp::config::maxOpenFiles = 5;
bool lsp::config::precalculateOnOpeningFiles = true;
bool lsp::config::shutdown = false;
bool lsp::config::referenceLinkToParentShortname = true;