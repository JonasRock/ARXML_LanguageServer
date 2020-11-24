#include "arxmlStorage.hpp"

#include <algorithm>

#include "lspExceptions.hpp"
#include "boost/tuple/tuple.hpp"

lsp::ArxmlStorage::ArxmlStorage()
    : shortnames_(),
      shortnamesFullPathIndex_{shortnames_.get<tag_fullPathIndex>()},
      shortnamesOffsetIndex_{shortnames_.get<tag_offsetIndex>()}
    {}

const lsp::ShortnameElement &lsp::ArxmlStorage::getShortnameByFullPath(const std::string &fullPath, const uint32_t fileIndex) const
{
    auto res = shortnamesFullPathIndex_.find(boost::make_tuple(fullPath, fileIndex));
    if (res != shortnamesFullPathIndex_.end())
    {
        return *res;
    }
    else  throw lsp::elementNotFoundException();
}

std::vector<const lsp::ShortnameElement*> lsp::ArxmlStorage::getShortnamesByFullPath(const std::string &fullPath) const
{
    std::vector<const lsp::ShortnameElement*> results;
    for( auto itPair = shortnamesFullPathIndex_.equal_range(fullPath);
        itPair.first != itPair.second; itPair.first++)
    {
        results.push_back(&(*itPair.first));
    }
    return results;
}

const lsp::ShortnameElement &lsp::ArxmlStorage::getShortnameByOffset(const uint32_t &offset, const uint32_t fileIndex) const
{
    //Get the element with that has a higher offset that we look for
    auto res = shortnamesOffsetIndex_.upper_bound(boost::make_tuple(fileIndex, offset));

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

const lsp::ReferenceElement &lsp::ArxmlStorage::getReferenceByOffset(const uint32_t &offset, const uint32_t fileIndex) const
{
    auto res = std::find_if(references_.begin(), references_.end(),
    [offset, fileIndex](const ReferenceElement &elem)
    {
        if (offset >= elem.charOffset && offset <= (elem.charOffset + elem.targetPath.length()) && elem.fileIndex == fileIndex)
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

std::vector<const lsp::ShortnameElement*> lsp::ArxmlStorage::getUniqueShortnamesByPathOnly(const std::string &path) const
{
    std::vector<const lsp::ShortnameElement*> results;
    for (auto &elem: shortnames_)
    {
        if(elem.path == path)
        {
            bool duplicate = false;
            for (auto &result : results)
            {
                if(result->name == elem.name)
                    duplicate = true;
            }
            if(!duplicate)
                results.push_back(&elem);
        }
    }
    return results;
}

const lsp::ShortnameElement* lsp::ArxmlStorage::addShortname(const lsp::ShortnameElement &elem)
{
    auto it = shortnamesOffsetIndex_.emplace_hint(shortnamesOffsetIndex_.end(), elem);
    return &(*it);
}

const lsp::ReferenceElement* lsp::ArxmlStorage::addReference(const lsp::ReferenceElement &elem)
{
    references_.push_back(elem);
    return &(references_.back());
}

void lsp::ArxmlStorage::addNewlineOffset(const uint32_t newlineOffset, const uint32_t fileIndex)
{
    newlineOffsets_[fileIndex].push_back(newlineOffset);
}

void lsp::ArxmlStorage::reserveNewlineOffsets(const uint32_t numNewlineOffsets, const uint32_t fileIndex)
{
    newlineOffsets_.push_back(std::vector<uint32_t>());
    newlineOffsets_[fileIndex].reserve(numNewlineOffsets);
}

uint32_t lsp::ArxmlStorage::getOffsetFromPosition(const lsp::types::Position &position, const uint32_t fileIndex) const
{
    return newlineOffsets_[fileIndex][position.line] + position.character;
}

const lsp::types::Position lsp::ArxmlStorage::getPositionFromOffset(const uint32_t offset, const uint32_t fileIndex) const
{
    lsp::types::Position ret;
    ret.line = std::lower_bound(
        newlineOffsets_[fileIndex].begin(), newlineOffsets_[fileIndex].end(), offset
    ) - newlineOffsets_[fileIndex].begin() - 1;
    ret.character = offset - newlineOffsets_[fileIndex][ret.line];
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

uint32_t lsp::ArxmlStorage::getFileIndex(std::string uri)
{
    for(uint32_t i = 0; i < URIs_.size(); i++)
    {
        if(!URIs_[i].compare(uri))
        {
            return i;
        }
    }
    throw lsp::elementNotFoundException();
}

void lsp::ArxmlStorage::addFileIndex(std::string uri)
{
    URIs_.push_back(uri);
}

bool lsp::ArxmlStorage::containsFile(std::string uri)
{
    auto pos = std::find(std::begin(URIs_), std::end(URIs_), uri);
    if (pos != std::end(URIs_)) {
        return true;
    }
    return false;
}

std::string lsp::ArxmlStorage::getUriFromFileIndex(uint32_t fileIndex)
{
    return URIs_[fileIndex];
}