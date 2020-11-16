#include "xmlParser.hpp"
#include <iostream>

#include "lspExceptions.hpp"
#include "config.hpp"
#include <chrono>

const lsp::ShortnameElement &helper_getShortnameFromInnerPath(std::shared_ptr<lsp::ArxmlStorage> storage, lsp::ReferenceElement &reference, const uint32_t offset)
{
    uint32_t cursorDistance = offset - reference.charOffset;
    const lsp::ShortnameElement* shortname = &(storage->getShortnameByFullPath(reference.targetPath));
    std::string fullPath = shortname->getFullPath();
    uint32_t num = std::count(fullPath.begin() + cursorDistance, fullPath.end(), '/');
    for(int i = 0; i < num; i++)
    {
        if(shortname->parent == nullptr)
        {
            throw lsp::elementNotFoundException();
        }
        shortname = shortname->parent;
    }
    return (*shortname);
}

const lsp::types::Hover lsp::XmlParser::getHover(const lsp::types::TextDocumentPositionParams &params)
{
    auto storage = parseFull(params.textDocument.uri);
    uint32_t offset = storage->getOffsetFromPosition(params.position) + 2;
    ShortnameElement shortname;
    //Is it a shortname?
    try
    {
        shortname = storage->getShortnameByOffset(offset);
    }
    //Maybe a reference, in that case get the references of whatever name the user pointed to
    catch (const lsp::elementNotFoundException &e)
    {
        ReferenceElement reference = storage->getReferenceByOffset(offset);
        shortname = helper_getShortnameFromInnerPath(storage, reference, offset);
    }
    lsp::types::Hover result;
    result.contents += "**Full path:** " + shortname.getFullPath() + "\n";
    for (int i = 0; i < shortname.references.size() && i < 10; ++i)
    {
        result.contents += "- **" + shortname.references[i]->name + ":** " + shortname.references[i]->targetPath + "\n";
    }
    if(shortname.references.size() > 10)
        result.contents += "- ... (" + std::to_string(shortname.references.size()) + " reference element)";
    result.range.start = params.position;
    result.range.end = params.position;
    return result;
}

const lsp::types::LocationLink lsp::XmlParser::getDefinition(const lsp::types::TextDocumentPositionParams &params)
{
    auto storage = parseFull(params.textDocument.uri);
    uint32_t offset = storage->getOffsetFromPosition(params.position) + 2;
    ReferenceElement reference = storage->getReferenceByOffset(offset);
    ShortnameElement shortname = helper_getShortnameFromInnerPath(storage, reference, offset);
    
    //The number of '/' between where the user clicked and where the name ends is the number of times we need to get the parent
    lsp::types::LocationLink result;
    result.originSelectionRange.start = storage->getPositionFromOffset(reference.charOffset);
    result.originSelectionRange.start = storage->getPositionFromOffset(reference.charOffset + reference.targetPath.length());
    result.targetUri = params.textDocument.uri;
    result.targetRange.start = storage->getPositionFromOffset(shortname.charOffset);
    result.targetRange.end = storage->getPositionFromOffset(shortname.charOffset + shortname.name.length());
    result.targetSelectionRange = result.targetRange;

    return result;
}

std::vector<lsp::types::Location> lsp::XmlParser::getReferences(const lsp::types::ReferenceParams &params)
{
    std::vector<lsp::types::Location> results;
    
    auto storage = parseFull(params.textDocument.uri);
    uint32_t offset = storage->getOffsetFromPosition(params.position) + 2;
    lsp::ShortnameElement elem;
    try
    {
        elem = storage->getShortnameByOffset(offset);
    }
    catch(const lsp::elementNotFoundException& e)
    {
        auto reference = storage->getReferenceByOffset(offset);
        elem = helper_getShortnameFromInnerPath(storage, reference, offset);
    }

    for(auto &ref: storage->getReferencesByShortname(elem))
    {
        lsp::types::Location res;
        res.uri = params.textDocument.uri;
        res.range.start = storage->getPositionFromOffset(ref->charOffset);
        res.range.end = storage->getPositionFromOffset(ref->charOffset + ref->targetPath.length());
        results.push_back(res);
    }
    if(results.size() && params.context.includeDeclaration)
    {
        lsp::types::Location res;
        res.uri = params.textDocument.uri;
        res.range.start = storage->getPositionFromOffset(elem.charOffset);
        res.range.end = storage->getPositionFromOffset(elem.charOffset + elem.name.length());
    }
    return results;
}

std::vector<lsp::types::non_standard::ShortnameTreeElement> lsp::XmlParser::getChildren(const lsp::types::non_standard::GetChildrenParams &params)
{
    auto storage = parseFull(params.uri);
    std::vector<lsp::types::non_standard::ShortnameTreeElement> results;
    if(params.path.length())
    {
        ShortnameElement shortname = storage->getShortnameByFullPath(params.path);
        for (auto &elem : (shortname.children))
        {
            lsp::types::non_standard::ShortnameTreeElement treeElem;
            treeElem.name = elem->name;
            treeElem.path = elem->path;
            treeElem.cState = elem->children.size() ? 1 : 0;
            treeElem.pos = storage->getPositionFromOffset(elem->charOffset);
            results.push_back(treeElem);
        }
    }
    else
    {
        auto matching = storage->getShortnamesByPathOnly(params.path);
        for( auto &elem : matching)
        {
            lsp::types::non_standard::ShortnameTreeElement treeElem;
            treeElem.name = elem->name;
            treeElem.path = elem->path;
            treeElem.cState = elem->children.size() ? 1 : 0;
            treeElem.pos = storage->getPositionFromOffset(elem->charOffset);
            results.push_back(treeElem);
        }
    }
    return results;
}

void lsp::XmlParser::preParseFile(const lsp::types::DocumentUri uri)
{
    parseFull(uri);
}

std::shared_ptr<lsp::ArxmlStorage> lsp::XmlParser::parseFull(const lsp::types::DocumentUri uri)
{
    static uint32_t currentID = 0;
    std::shared_ptr<StorageElement> storageElem = nullptr;
    //Already in storage?
    for (auto &temp: storages_)
    {
        if(temp.uri == uri)
        {
            temp.lastUsedID = currentID++;
            return temp.storage;
        }
    }
    // Too many files open at the same time? -> delete oldest
    if (storages_.size() >= lsp::config::maxOpenFiles)
    {
        uint32_t lowest = ~0;
        std::list<StorageElement>::iterator oldest;
        for (auto it = storages_.begin(); it != storages_.end(); it++)
        {
            if(it->lastUsedID < lowest)
            {
                oldest = it;
                lowest = it->lastUsedID;
            }
        }
        storages_.erase(oldest);
    }
    //Parse the new file now
    StorageElement newStorage;
    newStorage.lastUsedID = currentID++;
    newStorage.uri = uri;
    newStorage.storage = std::make_shared<ArxmlStorage>();

    storages_.push_back(newStorage);
    std::string sanitizedFilePath = std::string(uri.begin() + 5, uri.end());
    auto repBegin = sanitizedFilePath.find("%3A");
    sanitizedFilePath.replace(repBegin, 3, ":");
    sanitizedFilePath = sanitizedFilePath.substr(repBegin - 1);
    boost::iostreams::mapped_file mmap(sanitizedFilePath, boost::iostreams::mapped_file::readonly);

    std::cout << "Parsing " << uri << "\n";
    auto t0 = std::chrono::high_resolution_clock::now();
    parseNewlines(mmap, newStorage.storage);
    auto t1 = std::chrono::high_resolution_clock::now();
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count() << "ms - Newlines\n";

    auto t2 = std::chrono::high_resolution_clock::now();
    parseShortnamesAndReferences(mmap, newStorage.storage);
    auto t3 = std::chrono::high_resolution_clock::now();
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t2).count() << "ms - Shortnames/References\n";

    mmap.close();
    return newStorage.storage;
}

void lsp::XmlParser::parseNewlines(boost::iostreams::mapped_file &mmap, std::shared_ptr<ArxmlStorage> storage)
{
    const char *const start = mmap.const_data();
    const char *current = start;
    const char *const end = current + mmap.size();

    if (current)
    {
        //First, go through once and count the number, so we can reserve enough space
        uint32_t numLines = std::count(start, end, '\n');
        storage->reserveNewlineOffsets(numLines + 1);
        storage->addNewlineOffset(0);

        while (current && current < end)
        {
            current = static_cast<const char *>(memchr(current, '\n', end - current));
            if (current)
            {
                storage->addNewlineOffset(current - start);
                ++current;
            }
        }
    }
}

void lsp::XmlParser::parseShortnamesAndReferences(boost::iostreams::mapped_file &mmap, std::shared_ptr<ArxmlStorage> storage)
{
    const char *const start = mmap.const_data();
    const char *current = start;
    const char *const end = current + mmap.size();

    uint32_t depth = 0;
    std::vector<std::pair<std::uint32_t, const lsp::ShortnameElement*>> depthElements;

    //Preparation for reference parsing
    const std::string searchPattern = "DEST";
    std::boyer_moore_searcher searcher(searchPattern.begin(), searchPattern.end());

    while (current && current < end)
    {
        //Go to the next tag
        current = static_cast<const char*>(memchr(current, '<', end - current));
        if (!current)
            break;
        ++current;

        ///////////////////
        /// parsing tag ///
        ///////////////////

        /// comment - skip ///
        if(*(current) == '!')
        {
            current = strstr(++current, "-->") + 3;
        }

        /// xml info - skip ///
        else if(*(current) == '?')
        {
            current = strstr(++current, "?>") + 2;
        }

        /// closing tag - decrease depth ///
        else if (*(current) == '/')
        {
            --depth;
            current = static_cast<const char *>(memchr(++current, '>', end - current)) + 1;
            if (!depthElements.empty())
            {
                while (depthElements.back().first > depth)
                {
                    depthElements.pop_back();
                    if (depthElements.empty())
                        break;
                }
            }
        }
        /// opening tag ///
        else
        {
            const char* tagEnd = static_cast<const char*>(memchr(current, '>', end - current));
            std::string tagContent(current, tagEnd);

            /// shortname ///
            if (!tagContent.compare("SHORT-NAME"))
            {
                ShortnameElement element;
                element.parent = nullptr;

                // skip to the end of the <SHORT-NAME> tag
                current += 11;

                const char *endChar = static_cast<const char*>(memchr(current, '<', end - current));
                std::string pathString = "";
                for (auto i : depthElements)
                {
                    pathString += i.second->name + "/";
                }
                if(pathString.size())
                {
                    pathString.pop_back();
                    element.parent = depthElements.back().second;
                }
                
                element.name = std::string(current, static_cast<uint32_t>(endChar - current));
                element.path = pathString;
                element.charOffset = current - start;

                const ShortnameElement* elementPtr = storage->addShortname(element);
                if(depthElements.size())
                {
                    depthElements.back().second->children.push_back(elementPtr);
                }
                depthElements.push_back(std::make_pair(depth, elementPtr));

                current = endChar + 13;
            }
            /// reference ///
            else if (std::search(tagContent.begin(), tagContent.end(), searcher) != tagContent.end())
            {
                current = static_cast<const char *>(memchr(current, '>', end - current)) + 2;
                const char* endOfReference = static_cast<const char*>(memchr(current, '<', end - current));

                ReferenceElement reference;
                auto nameBeginIndex = tagContent.find_first_of('\"') + 1;
                reference.name = tagContent.substr(nameBeginIndex, tagContent.find_last_of('\"') - nameBeginIndex);
                std::string refString = std::string(current, endOfReference);
                reference.targetPath = refString;
                reference.charOffset = current - start;
                if(depthElements.size())
                {
                    depthElements.back().second->references.push_back(storage->addReference(reference));
                }
                else
                {
                    storage->addReference(reference);
                }
                current = static_cast<const char*>(memchr(current, '>', end - current));
                
            }
            /// random tag - increase depth and skip ///
            else
            {
                current = static_cast<const char*>(memchr(current, '>', end - current)) + 1;
                if (*(current - 2) != '/')
                {
                    ++depth;
                }
            }
        }
    }
}