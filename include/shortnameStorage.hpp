#ifndef SHORTNAMESTORAGE_H
#define SHORTNAMESTORAGE_H

#include <string>
#include <cstdint>
#include <deque>
#include "boost/multi_index_container.hpp"
#include "boost/multi_index/ordered_index.hpp"
#include "boost/multi_index/mem_fun.hpp"
#include "boost/multi_index/member.hpp"

using namespace boost;

struct referenceRange
{
    std::pair<uint32_t, uint32_t> refOffsetRange;
    std::pair<uint32_t, uint32_t> targetOffsetRange;
};

class shortnameElement
{
public:
    //the stringpath through the shortname hierarchy, without leading or trailing '/'
    std::string path;
    //the shortname string itself
    std::string name;
    //the offset in the file to the first character of the shortname
    uint32_t fileOffset;
    //vector of the start positions of the references in the file
    //std::list<uint32_t> referenceOffsets;

    const std::pair<uint32_t, uint32_t> getOffsetRange() const
    {
        return std::make_pair(fileOffset, fileOffset + name.size());
    }

    //Returns the full path of the shortname including itself
    std::string getFullPath () const
    {
        if(path.size())
        {
            return path + "/" + name;
        }
        else
        {
            return name;
        }
    }
};

//Only used as tags for the multi-index container
struct fullPathIndex_t {};
struct offsetIndex_t {};

typedef multi_index_container<
        shortnameElement,
        multi_index::indexed_by<
            multi_index::ordered_unique<
                multi_index::tag<fullPathIndex_t>,
                multi_index::const_mem_fun<shortnameElement, std::string, &shortnameElement::getFullPath>
            >,
            multi_index::ordered_unique<
                multi_index::tag<offsetIndex_t>,
                multi_index::member<shortnameElement, uint32_t, &shortnameElement::fileOffset>
            >
        >
    > shortnameContainer_t;



class shortnameStorage
{
public:
    void addShortname(const shortnameElement &elem);
    void addReference(const referenceRange &ref);
    //Throws std::elementNotFoundException
    const shortnameElement &getByFullPath(const std::string &searchPath) const;
    //Throws std::elementNotFoundException
    const shortnameElement &getByOffset(const uint32_t searchOffset) const;
    const referenceRange &getReferenceByOffset(const uint32_t searchOffset) const;
    std::deque<referenceRange> references;


    shortnameStorage()
        : shortnames(), references(),
          fullPathIndex{shortnames.get<fullPathIndex_t>()},
          offsetIndex{shortnames.get<offsetIndex_t>()}
    {}
    
private:
    shortnameContainer_t shortnames;
    shortnameContainer_t::index<fullPathIndex_t>::type &fullPathIndex;
    shortnameContainer_t::index<offsetIndex_t>::type &offsetIndex;
};

#endif /* SHORTNAMESTORAGE_H */