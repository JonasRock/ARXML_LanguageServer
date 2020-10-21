#ifndef XMLPARSER_H
#define XMLPARSER_H

#include <vector>
#include <cstdint>

#include "boost/iostreams/device/mapped_file.hpp"
#include "boost/property_tree/ptree.hpp"

#include "types.hpp"

using namespace boost;

struct shortnameProps
{
    uint32_t offset;
    std::vector<std::pair<std::uint32_t, std::uint32_t>> referenceOffsetRange;
};

class xmlParser
{
public:
    void parseShortnames();
    void parseReferences();
    void parseNewlines();
    position getPositionFromOffset(uint32_t offset);
    uint32_t getOffsetFromPosition(position pos);
    xmlParser(std::string filepath);
    ~xmlParser();

private:
    //This tree contains the shortname path structure,
    //with the shortnames as keys and character offsets as values
    property_tree::basic_ptree<std::string, shortnameProps> shortnameTree;

    std::vector<uint32_t> newLineOffsets;
    iostreams::mapped_file mmap;
};

#endif /* XMLPARSER_H */
