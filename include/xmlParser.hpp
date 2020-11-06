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

#include "boost/iostreams/device/mapped_file.hpp"

#include "types.hpp"
#include "shortnameStorage.hpp"

using namespace boost;

namespace lsp
{


struct StorageElement
{
    std::string uri;
    std::shared_ptr<ShortnameStorage> storage;
    uint32_t lastUsedID;
};

/**
 * @brief Parser to precess arxml files and manage its data
 * 
 */
class XmlParser
{
public:
    /**
     * @brief Get link locations for a given position
     * 
     * @param posParams start position
     * @return lsp::types::LocationLink Ranges for links
     */
    lsp::types::LocationLink getDefinition(const lsp::types::TextDocumentPositionParams &posParams);

    /**
     * @brief Get all reference locations for a given position
     * 
     * @param posParams start position
     * @return std::vector<lsp::types::Location> locations for all corresponding references
     */
    std::vector<lsp::types::Location> getReferences(const lsp::types::ReferenceParams &posParams);

    /**
     * @brief parse the file into a shortnameStorage before the first request
     * 
     * @param uri document to preparse
     * 
     */
    void preParseFile(const lsp::types::DocumentUri uri);

private:
    std::shared_ptr<ShortnameStorage> parse(const lsp::types::DocumentUri uri);
    void parseShortnames(iostreams::mapped_file &mmap, std::shared_ptr<ShortnameStorage> storage);
    void parseReferences(iostreams::mapped_file &mmap, std::shared_ptr<ShortnameStorage> storage);
    void parseNewlines(iostreams::mapped_file &mmap, std::shared_ptr<ShortnameStorage> storage);

    std::list<StorageElement> storages;
};


}

#endif /* XMLPARSER_H */