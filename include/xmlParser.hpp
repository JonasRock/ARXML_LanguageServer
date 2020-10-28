#ifndef XMLPARSER_H
#define XMLPARSER_H

/**
 * @file xmlParser.hpp
 * @author Jonas rock
 * @brief contains the xmlParser used to process arxml files
 * @version 0.1
 * @date 2020-10-27
 * 
 * 
 */

#include <map>
#include <vector>
#include <cstdint>

#include "boost/iostreams/device/mapped_file.hpp"

#include "types.hpp"
#include "shortnameStorage.hpp"

using namespace boost;

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

    lsp::Position getPositionFromOffset(const uint32_t offset);
    uint32_t getOffsetFromPosition(const lsp::Position &pos);

private:
    std::shared_ptr<shortnameStorage> parse(const lsp::DocumentUri uri);
    void parseShortnames(iostreams::mapped_file &mmap, std::shared_ptr<shortnameStorage> storage);
    void parseReferences(iostreams::mapped_file &mmap, std::shared_ptr<shortnameStorage> storage);
    void parseNewlines(iostreams::mapped_file &mmap);

    //This tree contains the shortname path structure,
    //with the shortnames as keys and character offsets as values
    std::map<std::string, std::shared_ptr<shortnameStorage>> storages;

    std::vector<uint32_t> newLineOffsets;
};

#endif /* XMLPARSER_H */
