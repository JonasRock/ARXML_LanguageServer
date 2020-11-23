#ifndef SHORTNAMESTORAGE_H
#define SHORTNAMESTORAGE_H

#include <string>
#include <vector>
#include <deque>
#include <functional>

#include "boost/multi_index_container.hpp"
#include "boost/multi_index/ordered_index.hpp"
#include "boost/multi_index/mem_fun.hpp"
#include "boost/multi_index/member.hpp"
#include "boost/multi_index/composite_key.hpp"

#include "types.hpp"

namespace lsp
{

struct ReferenceElement;
// struct ShortnameElement;

struct ShortnameElement
{
    std::string name;
    std::string path;
    uint32_t charOffset;
    uint32_t fileIndex;
    mutable std::vector<const ShortnameElement*> children;
    mutable std::vector<const ReferenceElement*> references;
    const ShortnameElement* parent;
    std::string getFullPath() const;
};

struct ReferenceElement
{
    std::string name;
    uint32_t charOffset;
    std::string targetPath;
    const ShortnameElement* owner;
    uint32_t fileIndex;
};

// Typedef for the multi index map for the shortnames
struct tag_fullPathIndex {};
struct tag_offsetIndex {};
typedef boost::multi_index_container<
    ShortnameElement,
    boost::multi_index::indexed_by<
        boost::multi_index::ordered_unique<
            boost::multi_index::tag<tag_fullPathIndex>,
            boost::multi_index::composite_key<
                ShortnameElement,
                //This is ordered with fileIndex as the second key, so we can search for elements by full path without specifying an fileIndex
                //This is especially important for references where we don't necessarily know where the element is
                boost::multi_index::const_mem_fun<ShortnameElement, std::string, &ShortnameElement::getFullPath>,
                boost::multi_index::member<ShortnameElement, uint32_t, &ShortnameElement::fileIndex>
            >
        >,
        boost::multi_index::ordered_unique<
            boost::multi_index::tag<tag_offsetIndex>,
            boost::multi_index::composite_key<
                ShortnameElement,
                //This is ordered with fileIndex as first key as it doesnt make much sense to search of an offset without saying what file
                //This also allows us to use emplace hint effectively, as we can always append at the end, improving performance
                boost::multi_index::member<ShortnameElement, uint32_t, &ShortnameElement::fileIndex>,
                boost::multi_index::member<ShortnameElement, uint32_t, &ShortnameElement::charOffset>
            >
        >
    >
> shortnameContainer_t;

class ArxmlStorage
{
public:
    const ShortnameElement &getShortnameByOffset(const uint32_t &offset, const uint32_t fileIndex) const;
    const ReferenceElement &getReferenceByOffset(const uint32_t &offset, const uint32_t fileIndex) const;

    const ShortnameElement &getShortnameByFullPath(const std::string &fullPath, const uint32_t fileIndex) const;
    std::vector<const lsp::ShortnameElement*> getShortnamesByFullPath(const std::string &fullPath) const;

    std::vector<const ReferenceElement*> getReferencesByShortname(const ShortnameElement &elem) const;
    std::vector<const lsp::ShortnameElement*> getShortnamesByPathOnly(const std::string &path) const;

    const lsp::ShortnameElement* addShortname(const ShortnameElement &elem);
    const lsp::ReferenceElement* const addReference(const ReferenceElement &elem);
    void addFileIndex(std::string uri);
    uint32_t getFileIndex(std::string uri);
    std::string getUriFromFileIndex(uint32_t fileIndex);
    bool containsFile(std::string uri);;

    void addNewlineOffset(const uint32_t newlineOffset, const uint32_t fileIndex);
    void reserveNewlineOffsets(const uint32_t numNewlineOffsets, const uint32_t fileIndex);

    const uint32_t getOffsetFromPosition(const lsp::types::Position &position, const uint32_t fileIndex) const;
    const lsp::types::Position getPositionFromOffset(const uint32_t offset, const uint32_t fileIndex) const;

    ArxmlStorage();

private:
    shortnameContainer_t shortnames_;
    shortnameContainer_t::index<tag_fullPathIndex>::type &shortnamesFullPathIndex_;
    shortnameContainer_t::index<tag_offsetIndex>::type &shortnamesOffsetIndex_;
    std::deque<ReferenceElement> references_;
    std::vector<std::vector<uint32_t>> newlineOffsets_;
    //sanitizedFilePath_[fileIndex] = corresponding file path
    std::vector<std::string> URIs_;
};


} /* namespace lsp */

#endif /* SHORTNAMESTORAGE_H */