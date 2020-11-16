#include "arxmlStorage.hpp"

#include <algorithm>

#include "lspExceptions.hpp"

lsp::ArxmlStorage::ArxmlStorage()
    : shortnames_(), references_(),
      shortnamesFullPathIndex_{shortnames_.get<tag_fullPathIndex>()},
      shortnamesOffsetIndex_{shortnames_.get<tag_offsetIndex>()}
    {}

const lsp::ShortnameElement &lsp::ArxmlStorage::getShortnameByFullPath(const std::string &fullPath) const
{
    auto res = shortnamesFullPathIndex_.find(fullPath);
    if (res != shortnamesFullPathIndex_.end())
    {
        return *res;
    }
    else  throw lsp::elementNotFoundException();
}

const lsp::ShortnameElement &lsp::ArxmlStorage::getShortnameByOffset(const uint32_t &offset) const
{
    //Get the element with that has a higher offset that we look for
    auto res = shortnamesOffsetIndex_.upper_bound(offset);

    //First element is already higher than we look for -> not found
    if(res == shortnamesOffsetIndex_.begin())
    {
        throw lsp::elementNotFoundException();
    }
    //Now we can look if the previous element matches
    --res;
    if (offset >= (*res).charOffset && offset <= ((*res).charOffset + (*res).name.length()))
    {
        return *res;
    }
    //Doesn't match, smaller than what we look for -> not found
    throw lsp::elementNotFoundException();
}

const lsp::ReferenceElement &lsp::ArxmlStorage::getReferenceByOffset(const uint32_t &offset) const
{
    auto res = std::find_if(references_.begin(), references_.end(),
    [offset](const ReferenceElement &elem)
    {
        if (offset >= elem.charOffset && offset <= (elem.charOffset + elem.targetPath.length()))
        {
            return true;
        }
        return false;
    });
    if (res != references_.end())
    {
        return *res;
    }
    throw lsp::elementNotFoundException();
}

std::vector<const lsp::ReferenceElement*> lsp::ArxmlStorage::getReferencesByShortname(const ShortnameElement &elem) const
{
    std::vector<const lsp::ReferenceElement*> results;
    for(auto &ref : references_)
    {
        if(ref.targetPath == elem.getFullPath())
        {
            results.push_back(&ref);
        }
    }
    return results;
}

std::vector<const lsp::ShortnameElement*> lsp::ArxmlStorage::getShortnamesByPathOnly(const std::string &path) const
{
    std::vector<const lsp::ShortnameElement*> results;
    for (auto &elem: shortnames_)
    {
        if(elem.path == path)
            results.push_back(&elem);
    }
    return results;
}

const lsp::ShortnameElement* lsp::ArxmlStorage::addShortname(const lsp::ShortnameElement &elem)
{
    auto it = shortnamesOffsetIndex_.emplace_hint(shortnamesOffsetIndex_.end(), elem);
    return &(*it);
}

const lsp::ReferenceElement* const lsp::ArxmlStorage::addReference(const lsp::ReferenceElement &elem)
{
    references_.push_back(elem);
    return &(references_.back());
}

void lsp::ArxmlStorage::addNewlineOffset(const uint32_t newlineOffset)
{
    newlineOffsets_.push_back(newlineOffset);
}

void lsp::ArxmlStorage::reserveNewlineOffsets(const uint32_t numNewlineOffsets)
{
    newlineOffsets_.reserve(numNewlineOffsets);
}

const uint32_t lsp::ArxmlStorage::getOffsetFromPosition(const lsp::types::Position &position) const
{
    return newlineOffsets_[position.line] + position.character;
}

const lsp::types::Position lsp::ArxmlStorage::getPositionFromOffset(const uint32_t offset) const
{
    lsp::types::Position ret;
    ret.line = std::lower_bound(newlineOffsets_.begin(), newlineOffsets_.end(), offset) - newlineOffsets_.begin() - 1;
    ret.character = offset - newlineOffsets_[ret.line];
    return ret;
}

std::string lsp::ShortnameElement::getFullPath() const
{
    if(path.length())
    {
        return path + "/" + name;
    }
    else
    {
        return name;
    }
}