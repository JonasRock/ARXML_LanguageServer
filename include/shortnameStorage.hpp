#ifndef SHORTNAMESTORAGE_H
#define SHORTNAMESTORAGE_H

/**
 * @file shortnameStorage.hpp
 * @author Jonas Rock
 * @brief contains the data structures for storage of parsed data
 * @version 0.1
 * @date 2020-10-27
 * 
 * 
 */

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


/**
 * @brief contains all important info for a given shortname
 * 
 */
class shortnameElement
{
public:
    //the stringpath through the shortname hierarchy, without leading or trailing '/'
    std::string path;
    //the shortname string itself
    std::string name;
    //the offset in the file to the first character of the shortname
    uint32_t fileOffset;

    //Returns the start and end offsets of the shortname
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

/** @cond
 * 
 *  @brief only used as tags for the multi_index_container indices
 */
struct fullPathIndex_t {};
struct offsetIndex_t {};
/// @endcond

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

/**
 * @brief class that managaes the parsed reference and shortname data
 * 
 */
class shortnameStorage
{
public:
    /**
     * @brief adds a given shortname element to the storage
     * 
     * @param elem element to be inserted
     */
    void addShortname(const shortnameElement &elem);

    /**
     * @brief adds a given reference element to the storage
     * 
     * @param ref reference to be inserted
     */
    void addReference(const referenceRange &ref);

    /**
     * @brief Get a shortname from storage by its full path, including its name
     * 
     * @param searchPath the pathString to search for
     * @return const shortnameElement& 
     * @exception lsp::elementNotFoundException When no result can be found using the searchPath
     */
    const shortnameElement &getByFullPath(const std::string &searchPath) const;

    /**
     * @brief Get a shortname from storage by a offset
     * 
     * @param searchOffset the offset to search for 
     * @return const shortnameElement& 
     * @exception lsp::elementNotFoundException When no result can be found
     */
    const shortnameElement &getByOffset(const uint32_t searchOffset) const;

    /**
     * @brief Get a reference by its offset
     * 
     * @param searchOffset the offset to search fo
     * @return const referenceRange& 
     * @exception lsp::elementNotFoundException When no result can be found
     */
    const referenceRange &getReferenceByOffset(const uint32_t searchOffset) const;

    /**
     * @brief The container for all references
     * 
     */
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