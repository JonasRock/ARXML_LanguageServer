#include "xmlParser.hpp"
#include <iostream>

#include "lspExceptions.hpp"
#include "config.hpp"
#include <chrono>

const lsp::ShortnameElement &helper_getShortnameFromInnerPath(const lsp::ReferenceElement &reference, const uint32_t offset)
{
    uint32_t cursorDistance = offset-reference.charOffset;
    lsp::ShortnameElement* shortname = reference.target;
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

const lsp::types::Hover &lsp::XmlParser::getHover(const lsp::types::TextDocumentPositionParams &params)
{
    lsp::types::Hover res;
    res.range.start = params.position;
    res.range.end = params.position;
    res.contents = "Test Hover, please ignore";
    return res;
}

const lsp::types::LocationLink &lsp::XmlParser::getDefinition(const lsp::types::TextDocumentPositionParams &params)
{
    auto storage = parseFull(params.textDocument.uri);
    uint32_t offset = storage->getOffsetFromPosition(params.position);
    ReferenceElement reference = storage->getReferenceByOffset(offset);
    ShortnameElement shortname = helper_getShortnameFromInnerPath(reference, offset);
    
    //The number of '/' between where the user clicked and where the name ends is the number of times we need to get the parent
    lsp::types::LocationLink result;
    result.originSelectionRange.start = storage->getPositionFromOffset(reference.charOffset);
    result.originSelectionRange.start = storage->getPositionFromOffset(reference.charOffset + reference.target->getFullPath().length());
    result.targetUri = params.textDocument.uri;
    result.targetRange.start = storage->getPositionFromOffset(shortname.charOffset);
    result.targetRange.end = storage->getPositionFromOffset(shortname.charOffset + shortname.name.length());
    result.targetSelectionRange = result.targetRange;

    return result;
}

std::vector<lsp::types::Location> lsp::XmlParser::getReferences(const lsp::types::ReferenceParams &params)
{
    auto storage = parseFull(params.textDocument.uri);
    uint32_t offset = storage->getOffsetFromPosition(params.position);
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
        shortname = helper_getShortnameFromInnerPath(reference, offset);
    }
    std::vector<lsp::types::Location> results;
    for (auto &ref : (shortname.references))
    {
        lsp::types::Location loc;
        loc.uri = params.textDocument.uri;
        loc.range.start = storage->getPositionFromOffset(ref->charOffset);
        loc.range.end = storage->getPositionFromOffset(ref->charOffset + shortname.getFullPath().length());
        results.push_back(loc);
    }
    if (!(results.size()))
    {
        throw lsp::elementNotFoundException();
    }
    if (params.context.includeDeclaration)
    {
        lsp::types::Location declarationLoc;
        declarationLoc.uri = params.textDocument.uri;
        declarationLoc.range.start = storage->getPositionFromOffset(shortname.charOffset);
        declarationLoc.range.end = storage->getPositionFromOffset(shortname.charOffset + shortname.name.length());
    }
    return results;
}

std::vector<lsp::types::non_standard::ShortnameTreeElement> lsp::XmlParser::getChildren(const lsp::types::non_standard::GetChildrenParams &params)
{
    auto storage = parseFull(params.uri);
    ShortnameElement shortname = storage->getShortnameByFullPath(params.path);
    std::vector<lsp::types::non_standard::ShortnameTreeElement> results;
    for (auto &elem : (shortname.children))
    {
        lsp::types::non_standard::ShortnameTreeElement treeElem;
        treeElem.name = elem->name;
        treeElem.path = elem->path;
        treeElem.cState = elem->children.size();
        treeElem.pos = storage->getPositionFromOffset(elem->charOffset);
        results.push_back(treeElem);
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
        storage->reserveNewlines(numLines + 1);
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
    std::vector<std::pair<std::uint32_t, lsp::ShortnameElement*>> depthElements;

    //Preparation for reference parsing
    const std::string searchPattern = "-REF DEST =\"";
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
                    if (depths.empty())
                        break;
                }
            }
        }
        /// opening tag ///
        else
        {
            const char* tagEnd = static_cast<const char*>(memchr(current, '>', end - current)) - 1;
            std::string tagContent(current, tagEnd);

            /// shortname ///
            if (tagContent.compare("SHORT-NAME"))
            {
                ShortnameElement element;
                element.parent = nullptr;

                // skip to the end of the <SHORT-NAME> tag
                current += 11;

                const char *endChar = static_cast<const char*>(memchr(current, '<', end - current));
                std::string pathString = "";
                for (auto i : depthElements)
                {
                    pathString += i.second.name + "/";
                }
                if(pathString.size())
                {
                    pathString.pop_back();
                    element.parent = depthElements.back().second;
                }
                
                element.name = std::string(current, static_cast<uint32_t>(endChar - current));
                element.path = pathString;
                element.charOffset = current - start;
                depthElements.back().second->children.push_back(storage->addShortname(element));
                depthElements.push_back(std::make_pair(depth, element));

                current = endChar + 13;
            }
            /// reference ///
            else if (std::search(tagContent.begin(), tagContent.end(), searcher) != tagContent.end())
            {
                ReferenceElement reference;
                reference.name = std::string(tagContent.find_first_of('\"'), tagContent.find_last_of('\"') - 1);

                current = static_cast<const char *>(memchr(current, '>', end - current)) + 2;
                const char* endOfReference = static_cast<const char*>(memchr(current, '<', end - current));

                std::string refString = std::string(current, endOfReference);
                try
                {
                    const ShortnameElement &shortname = storage->getShortnameByFullPath(refString);
                    reference.target = shortname;
                    reference.charOffset = current;
                    shortname.references.push_back(storages->addReference(reference));
                }
                catch (lsp::elementNotFoundException &e)
                {

                }
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