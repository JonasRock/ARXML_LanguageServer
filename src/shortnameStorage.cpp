#include <algorithm>
#include <optional>

#include "shortnameStorage.hpp"
#include "lspExceptions.hpp"

const shortnameElement &shortnameStorage::getByFullPath(const std::string &searchString) const
{
    auto res = fullPathIndex.find(searchString);
    if (res != fullPathIndex.end())
    {
        return *res;
    }
    else throw lsp::elementNotFoundException();
}

const shortnameElement &shortnameStorage::getByOffset(const uint32_t searchOffset) const
{
    auto res = offsetIndex.upper_bound(searchOffset);

    //res now is a element with offset higher than searched, so we can see if the next lower element is correct
    if( res == offsetIndex.begin())
    {   
        //lowest element is already too big
        throw lsp::elementNotFoundException();
    }
    --res;
    if( searchOffset >= (*res).getOffsetRange().first && searchOffset <= (*res).getOffsetRange().second)
    {
        return *res;
    }
    else
    {
        throw lsp::elementNotFoundException();
    }
}

void shortnameStorage::addShortname(const shortnameElement &elem /*, const auto hint */)
{
    if(elem.name.size())
    {
        offsetIndex.emplace_hint(offsetIndex.end(), elem);
    }
    else
    {
        throw lsp::malformedElementInsertionException();
    }
}

void shortnameStorage::addReference(const referenceRange &ref)
{
    if(ref.refOffsetRange.first <= ref.refOffsetRange.second
        && ref.targetOffsetRange.first <= ref.targetOffsetRange.second)
    {
        references.push_back(ref);
    }
    else
    {
        throw lsp::malformedElementInsertionException();
    }
    
}

const referenceRange &shortnameStorage::getReferenceByOffset(const uint32_t searchOffset) const
{
    auto res = std::find_if(references.begin(), references.end(),
    [searchOffset](const referenceRange &range)
    {
        if (range.refOffsetRange.first <= searchOffset && range.refOffsetRange.second >= searchOffset)
        {
            return true;
        }
        else
        {
            return false;
        }
    });
    if( res != references.end())
    {
        return *res;
    }
    else throw lsp::elementNotFoundException();
}

const std::pair<uint32_t, uint32_t> shortnameElement::getOffsetRange() const
{
    return std::make_pair(fileOffset, fileOffset + name.size() - 1);
}

std::string shortnameElement::getFullPath () const
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

lsp::Position shortnameStorage::getPositionFromOffset(const uint32_t offset)
{
    lsp::Position ret;
    uint32_t index = std::lower_bound(newlineOffsets.begin(), newlineOffsets.end(), offset) - newlineOffsets.begin() - 1;
    ret.character = offset - newlineOffsets[index];
    ret.line = index;
    return ret;
}

uint32_t shortnameStorage::getOffsetFromPosition(const lsp::Position &pos)
{
    return newlineOffsets[pos.line] + pos.character;
}

void shortnameStorage::addNewlineOffset(uint32_t offset)
{
    newlineOffsets.push_back(offset);
}

void shortnameStorage::reserveNewlines(const uint32_t numNewlines)
{
    newlineOffsets.reserve(numNewlines);
}