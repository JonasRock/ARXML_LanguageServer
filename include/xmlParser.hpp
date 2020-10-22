#ifndef XMLPARSER_H
#define XMLPARSER_H

#include <vector>
#include <cstdint>

#include "boost/iostreams/device/mapped_file.hpp"

#include "types.hpp"
#include "shortnameStorage.hpp"

using namespace boost;

class xmlParser
{
public:
    xmlParser(std::string filepath);
    ~xmlParser();

    void parse();
    lsp::LocationLink getDefinition(lsp::TextDocumentPositionParams posParams);
    std::vector<lsp::LocationLink> getReferences(lsp::TextDocumentPositionParams posParams);

    lsp::Position getPositionFromOffset(uint32_t offset);
    uint32_t getOffsetFromPosition(lsp::Position pos);

private:
    void parseShortnames();
    void parseReferences();
    void parseNewlines();

    iostreams::mapped_file mmap;

    //This tree contains the shortname path structure,
    //with the shortnames as keys and character offsets as values
    shortnameStorage storage;

    std::vector<uint32_t> newLineOffsets;
};

#endif /* XMLPARSER_H */
