/**
 * @file xmlParser.hpp
 * @author Jonas Rock
 * @brief Parser to process arxml files and manage its data
 * @version 0.1
 * @date 2020-11-05
 */

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
 * @brief Parser to precess arxml files and manage its data
 * 
 */
class xmlParser
{
public:
    /**
     * @brief Get link locations for a given position
     * 
     * @param posParams start position
     * @return lsp::LocationLink Ranges for links
     */
    lsp::LocationLink getDefinition(const lsp::TextDocumentPositionParams &posParams);

    /**
     * @brief Get all reference locations for a given position
     * 
     * @param posParams start position
     * @return std::vector<lsp::Location> locations for all corresponding references
     */
    std::vector<lsp::Location> getReferences(const lsp::ReferenceParams &posParams);

    /**
     * @brief parse the file into a shortnameStorage before the first request
     * 
     * @param uri document to preparse
     * 
     */
    void preParseFile(const lsp::DocumentUri uri);

private:
    std::shared_ptr<shortnameStorage> parse(const lsp::DocumentUri uri);
    void parseShortnames(iostreams::mapped_file &mmap, std::shared_ptr<shortnameStorage> storage);
    void parseReferences(iostreams::mapped_file &mmap, std::shared_ptr<shortnameStorage> storage);
    void parseNewlines(iostreams::mapped_file &mmap, std::shared_ptr<shortnameStorage> storage);

    std::list<storageElement> storages;
};

#endif /* XMLPARSER_H */