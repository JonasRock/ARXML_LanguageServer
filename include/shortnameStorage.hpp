/**
 * @file shortnameStorage.hpp
 * @author Jonas Rock
 * @brief Data structure for the parsed arxml data
 * @version 0.1
 * @date 2020-11-05
 */

#ifndef SHORTNAMESTORAGE_H
#define SHORTNAMESTORAGE_H

#include <string>
#include <deque>

#include "boost/multi_index_container.hpp"
#include "boost/multi_index/ordered_index.hpp"
#include "boost/multi_index/mem_fun.hpp"
#include "boost/multi_index/member.hpp"

#include "types.hpp"

using namespace boost;

/**
 * @brief Represents link ranges for a reference
 * 
 */
struct referenceRange
{
    std::pair<uint32_t, uint32_t> refOffsetRange;
    std::pair<uint32_t, uint32_t> targetOffsetRange;
};


/**
 * @brief Represents a shortname element with all its required info
 * 
 */
class shortnameElement
{
public:
    /**
     * @brief The path to the element through the shortname tree. Without leading or trailing '/'
     * 
     */
    std::string path;

    /**
     * @brief Shortname itself
     * 
     */
    std::string name;

    /**
     * @brief Char offset from file start of first character of the shortname
     * 
     */
    uint32_t fileOffset;

    /**
     * @brief Returns the char offsets from file start to first and last char of the shortname
     * 
     * @return const std::pair<uint32_t, uint32_t> first is start, second is end
     */
    const std::pair<uint32_t, uint32_t> getOffsetRange() const;

    /**
     * @brief Returns the full path including the shortname itself e.g. "path/name" without leading or trailing '/'
     * 
     * @return std::string 
     */
    std::string getFullPath () const;
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
 * @brief Data structure for the parsed arxml data
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
     * @brief Convert from char offset to lineNr/charNr position
     * 
     * @param offset Char offset from file start
     * @return lsp::Position position in lineNr/charNr representation
     */
    lsp::Position getPositionFromOffset(const uint32_t offset);

    /**
     * @brief Convert from lineNr/charNr to char offset
     * 
     * @param pos Position in lineNr/charNr representation
     * @return uint32_t Char offset from file start
     */
    uint32_t getOffsetFromPosition(const lsp::Position &pos);

    /** @cond
     * 
     *  @brief only used during parsing
     */
    void addNewlineOffset(const uint32_t offset);
    void reserveNewlines(const uint32_t numNewlines);
    /// @endcond

    /**
     * @brief Contains all found references
     * 
     */
    std::deque<referenceRange> references;
    
    /**
     * @brief Construct a new shortname Storage object
     * 
     */
    shortnameStorage()
        : shortnames(), references(),
          fullPathIndex{shortnames.get<fullPathIndex_t>()},
          offsetIndex{shortnames.get<offsetIndex_t>()}
    {}
    
private:
    shortnameContainer_t shortnames;
    std::vector<uint32_t> newlineOffsets;
    shortnameContainer_t::index<fullPathIndex_t>::type &fullPathIndex;
    shortnameContainer_t::index<offsetIndex_t>::type &offsetIndex;
    
};

#endif /* SHORTNAMESTORAGE_H */