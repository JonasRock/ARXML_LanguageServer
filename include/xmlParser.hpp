#ifndef XMLPARSER_H
#define XMLPARSER_H

#include <list>
#include <cstdint>

#include "boost/iostreams/device/mapped_file.hpp"

#include "types.hpp"
#include "shortnameStorage.hpp"

using namespace boost;

struct storageElement
{
    std::string uri;
    std::shared_ptr<shortnameStorage> storage;
    uint32_t lastUsedID;
};

/**
 * @brief contains all functionality to parse shortnames and references
 * 
 */
class xmlParser
{
public:
    xmlParser();
    ~xmlParser();

    lsp::LocationLink getDefinition(const lsp::TextDocumentPositionParams &posParams);
    std::vector<lsp::Location> getReferences(const lsp::ReferenceParams &posParams);
    void preParseFile(const lsp::DocumentUri);

private:
    std::shared_ptr<shortnameStorage> parse(const lsp::DocumentUri uri);
    void parseShortnames(iostreams::mapped_file &mmap, std::shared_ptr<shortnameStorage> storage);
    void parseReferences(iostreams::mapped_file &mmap, std::shared_ptr<shortnameStorage> storage);
    void parseNewlines(iostreams::mapped_file &mmap, std::shared_ptr<shortnameStorage> storage);

    std::list<storageElement> storages;
};

#endif /* XMLPARSER_H */