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
    if( res == offsetIndex.end() || res == offsetIndex.begin())
    {
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
    //emplace_hint for improved performance?
    offsetIndex.emplace_hint(offsetIndex.end(), elem);
}

void shortnameStorage::addReference(const referenceRange &ref)
{
    references.push_back(ref);
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