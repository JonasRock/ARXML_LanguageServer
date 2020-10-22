#include "shortnameStorage.hpp"
#include <algorithm>
#include "lspExceptions.hpp"

void shortnameStorage::add(const shortnameElement &elem)
{
    shortnames.push_back(elem);
}

shortnameElement &shortnameStorage::getByName(const std::string &searchString)
{
    auto ret = std::find_if(shortnames.begin(), shortnames.end(),
        [&searchString] (const shortnameElement &elem)
        {
            return !(elem.name.compare(searchString));
        }
    );
    if(ret != shortnames.end())
    {
        return *ret;
    }
    else throw lsp::elementNotFoundException();
}

shortnameElement &shortnameStorage::getByFullPath(const std::string &searchString)
{
    auto ret = std::find_if(shortnames.begin(), shortnames.end(),
        [&searchString] (const shortnameElement &elem)
        {
            return !(elem.getFullPath().compare(searchString));
        }
    );
    if(ret != shortnames.end())
    {
        return *ret;
    }
    else throw lsp::elementNotFoundException();
}

shortnameElement &shortnameStorage::getByOffset(const uint32_t searchOffset)
{
    auto ret = std::find_if(shortnames.begin(), shortnames.end(),
        [&searchOffset] (const shortnameElement &elem)
        {
            auto offsetRange = elem.getOffsetRange();
            return (searchOffset >= offsetRange.first && searchOffset <= offsetRange.second);
        }
    );
    if(ret != shortnames.end())
    {
        return *ret;
    }
    else throw lsp::elementNotFoundException();
}