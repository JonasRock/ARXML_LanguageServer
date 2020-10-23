#include <algorithm>
#include <optional>

#include "shortnameStorage.hpp"
#include "lspExceptions.hpp"

void shortnameStorage::add(const shortnameElement &elem)
{
    shortnames.push_back(elem);
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
        return (*ret);
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
        return (*ret);
    }
    else throw lsp::elementNotFoundException();;
}

const shortnameTreeNode* shortnameStorage::resolvePath(const std::string &path)
{
    return resolvePath(path, &shortnames);
}

const shortnameTreeNode* shortnameStorage::resolvePath(const std::string &path, const shortnameTreeNode* searchRoot)
{
    if (path == "")
        return searchRoot;
    else
    {
        (*searchRoot)
    }
}