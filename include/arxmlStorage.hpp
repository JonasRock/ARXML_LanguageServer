#ifndef SHORTNAMESTORAGE_H
#define SHORTNAMESTORAGE_H

#include <string>
#include <vector>
#include <deque>

#include "boost/multi_index_container.hpp"
#include "boost/multi_index/ordered_index.hpp"
#include "boost/multi_index/mem_fun.hpp"
#include "boost/multi_index/member.hpp"

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
    std::vector<ShortnameElement*> children;
    std::vector<ReferenceElement*> references;
    ShortnameElement* parent;
    std::string getFullPath() const;
};

struct ReferenceElement
{
    std::string name;
    uint32_t charOffset;
    ShortnameElement* target;
};

// Typedef for the multi index map for the shortnames
// For random_access_indices Validity of iterators and references to elements is preserved in all operations, regardless of the capacity status.
struct tag_fullPathIndex {};
struct tag_offsetIndex {};
typedef boost::multi_index_container<
    ShortnameElement,
    boost::multi_index::indexed_by<
        boost::multi_index::ordered_unique<
            boost::multi_index::tag<tag_fullPathIndex>,
            boost::multi_index::const_mem_fun<ShortnameElement, std::string, &ShortnameElement::getFullPath>
        >,
        boost::multi_index::ordered_unique<
            boost::multi_index::tag<tag_offsetIndex>,
            boost::multi_index::member<ShortnameElement, uint32_t, &ShortnameElement::charOffset>
        >
    >
> shortnameContainer_t;

class ArxmlStorage
{
public:
    const ShortnameElement &getShortnameByFullPath(const std::string &fullPath) const;
    const ShortnameElement &getShortnameByOffset(const uint32_t &offset) const;
    const ReferenceElement &getReferenceByOffset(const uint32_t &offset) const;

    auto addShortname(const ShortnameElement &elem);
    const lsp::ReferenceElement* const addReference(const ReferenceElement &elem);
    bool addChildToShortname;
    bool addParentToShortname;
    bool addReferenceToShortname;
    void addNewlineOffset(const uint32_t newlineOffset);
    void reserveNewlineOffsets(const uint32_t numNewlineOffsets);
    const uint32_t getOffsetFromPosition(const lsp::types::Position &position) const;
    const lsp::types::Position getPositionFromOffset(const uint32_t offset) const;

private:
    shortnameContainer_t shortnames_;
    shortnameContainer_t::index<tag_fullPathIndex>::type &shortnamesFullPathIndex_;
    shortnameContainer_t::index<tag_offsetIndex>::type &shortnamesOffsetIndex_;
    std::deque<ReferenceElement> references_;
    std::vector<uint32_t> newlineOffsets_;
};


} /* namespace lsp */

#endif /* SHORTNAMESTORAGE_H */