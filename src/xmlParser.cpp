#include "xmlParser.hpp"

#include "lspExceptions.hpp"
#include "config.hpp"

#include "boost/filesystem.hpp"

#include <chrono>
#include <iostream>
#include <algorithm>

const std::string helper_makeURI(std::string sanitizedFilePath)
{
    sanitizedFilePath.replace(1, 1, "%3A");
    std::string ret = "file:///" + sanitizedFilePath;
    return ret;
}

const lsp::ShortnameElement &helper_getShortnameFromInnerPath(std::shared_ptr<lsp::ArxmlStorage> storage, lsp::ReferenceElement &reference, const uint32_t offset)
{
    uint32_t cursorDistance = offset - reference.charOffset;
    auto shortnames = (storage->getShortnamesByFullPath(reference.targetPath));
    if(shortnames.size() != 1)
    {
        throw lsp::multipleDefinitionException();
    }
    const lsp::ShortnameElement* shortname = shortnames[0];
    std::string fullPath = shortname->getFullPath();
    uint32_t num = std::count(fullPath.begin() + cursorDistance, fullPath.end(), '/');
    for(uint32_t i = 0; i < num; i++)
    {
        if(shortname->parent == nullptr)
        {
            throw lsp::elementNotFoundException();
        }
        shortname = shortname->parent;
    }
    return (*shortname);
}

std::vector<std::string> helper_getARXMLFilePathsInDirectory(boost::filesystem::path &path)
{
    std::vector<std::string> arxmlFilesInDirectory;
    for (boost::filesystem::directory_entry &entry : boost::filesystem::directory_iterator(path))
    {
        std::string filePathString = entry.path().generic_string();
        if (!filePathString.substr(filePathString.length() - 6, 6).compare(".arxml"))
        {
            arxmlFilesInDirectory.push_back(filePathString);
        }
    }
    return arxmlFilesInDirectory;
}

uint32_t helper_getNextUsageID()
{
    static uint32_t currentID = 0;
    return currentID++;
}

const std::string helper_sanitizeUri(std::string unsanitized)
{
    std::string sanitizedFilePath = unsanitized;
    auto colonPos = sanitizedFilePath.find("%3A");
    sanitizedFilePath.replace(colonPos, 3, ":");
    sanitizedFilePath = sanitizedFilePath.substr(colonPos - 1);
    return sanitizedFilePath;
}

const lsp::types::Hover lsp::XmlParser::getHover(const lsp::types::TextDocumentPositionParams &params)
{
    auto storage = getStorageForUri(params.textDocument.uri);
    uint32_t fileIndex = storage->getFileIndex(params.textDocument.uri);
    uint32_t offset = storage->getOffsetFromPosition(params.position, fileIndex) + 2;
    ShortnameElement shortname;
    //Is it a shortname?
    try
    {
        shortname = storage->getShortnameByOffset(offset, fileIndex);
    }
    //Maybe a reference, in that case get the references of whatever name the user pointed to
    catch (const lsp::elementNotFoundException &e)
    {
        ReferenceElement reference = storage->getReferenceByOffset(offset, fileIndex);
        shortname = helper_getShortnameFromInnerPath(storage, reference, offset);
    }
    lsp::types::Hover result;
    result.contents += "**Full path:** " + shortname.getFullPath() + "\n";
    for (uint32_t i = 0; i < shortname.references.size() && i < 10; ++i)
    {
        auto targets = storage->getShortnamesByFullPath(shortname.references[i]->targetPath);
        if(targets.size() == 1)
        {
            std::string link = storage->getUriFromFileIndex(targets[0]->fileIndex)
                + "#L" + std::to_string(storage->getPositionFromOffset(targets[0]->charOffset, targets[0]->fileIndex).line + 1);
            result.contents += "- **" + shortname.references[i]->name + ":** [" + shortname.references[i]->targetPath + "](" + link + ")\n";
        }
        else throw lsp::elementNotFoundException();
    }
    if(shortname.references.size() > 10)
        result.contents += "- ... (" + std::to_string(shortname.references.size()) + " reference element)";
    result.range.start = params.position;
    result.range.end = params.position;
    return result;
}

const lsp::types::LocationLink lsp::XmlParser::getDefinition(const lsp::types::TextDocumentPositionParams &params)
{
    auto storage = getStorageForUri(params.textDocument.uri);
    uint32_t fileIndex = storage->getFileIndex(params.textDocument.uri);
    uint32_t offset = storage->getOffsetFromPosition(params.position, fileIndex) + 2;
    ReferenceElement reference = storage->getReferenceByOffset(offset, fileIndex);
    ShortnameElement shortname = helper_getShortnameFromInnerPath(storage, reference, offset);
    
    //The number of '/' between where the user clicked and where the name ends is the number of times we need to get the parent
    lsp::types::LocationLink result;
    result.originSelectionRange.start = storage->getPositionFromOffset(reference.charOffset - 2, fileIndex);
    result.originSelectionRange.end = storage->getPositionFromOffset(reference.charOffset + shortname.getFullPath().length() - 1, fileIndex);
    result.targetUri = storage->getUriFromFileIndex(shortname.fileIndex);
    result.targetRange.start = storage->getPositionFromOffset(shortname.charOffset, shortname.fileIndex);
    result.targetRange.end = storage->getPositionFromOffset(shortname.charOffset + shortname.name.length(), shortname.fileIndex);
    result.targetSelectionRange = result.targetRange;

    return result;
}

std::vector<lsp::types::Location> lsp::XmlParser::getReferences(const lsp::types::ReferenceParams &params)
{
    std::vector<lsp::types::Location> results;
    
    auto storage = getStorageForUri(params.textDocument.uri);
    uint32_t fileIndex = storage->getFileIndex(params.textDocument.uri);
    uint32_t offset = storage->getOffsetFromPosition(params.position, fileIndex) + 2;
    lsp::ShortnameElement elem;
    try
    {
        elem = storage->getShortnameByOffset(offset, fileIndex);
    }
    catch(const lsp::elementNotFoundException& e)
    {
        auto reference = storage->getReferenceByOffset(offset, fileIndex);
        elem = helper_getShortnameFromInnerPath(storage, reference, offset);
    }

    if(lsp::config::referenceLinkToParentShortname)
    {
        for(auto &ref: storage->getReferencesByShortname(elem))
        {
            lsp::types::Location res;
            res.uri = storage->getUriFromFileIndex(ref->fileIndex);
            res.range.start = storage->getPositionFromOffset(ref->owner->charOffset - 1, ref->owner->fileIndex);
            res.range.end = storage->getPositionFromOffset(ref->owner->charOffset + ref->owner->name.length() - 1, ref->owner->fileIndex);
            results.push_back(res);
        }
    }
    else
    {
        for(auto &ref: storage->getReferencesByShortname(elem))
        {
            lsp::types::Location res;
            res.uri = storage->getUriFromFileIndex(ref->fileIndex);
            res.range.start = storage->getPositionFromOffset(ref->charOffset - 2, ref->fileIndex);
            res.range.end = storage->getPositionFromOffset(ref->charOffset + ref->targetPath.length() - 1, ref->fileIndex);
            results.push_back(res);
        }
    }
    return results;
}

std::vector<lsp::types::non_standard::ShortnameTreeElement> lsp::XmlParser::getChildren(const lsp::types::non_standard::GetChildrenParams &params)
{
    std::vector<lsp::types::non_standard::ShortnameTreeElement> results;
    try
    {
        auto storage = getStorageForUri(params.uri);
        auto shortnames = storage->getShortnamesByPathOnly(params.path);
        for (auto &shortname : shortnames)
        {
            bool duplicate = false;
            if (!params.unique)
            {
                //Check for duplicates. No duplicates are possible if the path is unique already
                for(auto &result : results)
                {
                    if (!result.name.compare(shortname->name))
                    {
                        result.unique = false;
                        duplicate = true;
                    }
                }
            }
            if(!duplicate)
            {
                lsp::types::non_standard::ShortnameTreeElement elem;
                elem.cState = shortname->children.size() ? 1 : 0;
                elem.name = shortname->name;
                elem.path = shortname->path;
                elem.pos = storage->getPositionFromOffset(shortname->charOffset, shortname->fileIndex);
                elem.unique = true;
                elem.uri = storage->getUriFromFileIndex(shortname->fileIndex);
                results.push_back(elem);
            }
        }
        return results;
    }
    catch (const lsp::elementNotFoundException &e)
    {
        return results;
    }
}

lsp::types::Location lsp::XmlParser::getOwner(const lsp::types::non_standard::OwnerParams &params)
{
    auto storage = getStorageForUri(params.uri);
    uint32_t fileIndex = storage->getFileIndex(params.uri);
    lsp::ReferenceElement elem = storage->getReferenceByOffset(storage->getOffsetFromPosition(params.pos, fileIndex) + 2, fileIndex);
    lsp::types::Location result;
    result.uri = params.uri;
    result.range.start = storage->getPositionFromOffset(elem.owner->charOffset - 1, fileIndex);
    result.range.end = storage->getPositionFromOffset(elem.owner->charOffset + elem.owner->name.length() - 1, fileIndex);
    return result;
}

void lsp::XmlParser::preParse(const lsp::types::DocumentUri uri)
{
    getStorageForUri(uri);
}

std::shared_ptr<lsp::ArxmlStorage> lsp::XmlParser::getStorageForUri(const lsp::types::DocumentUri uri)
{
    for(auto &element: storages_)
    {
        if(element.storage->containsFile(uri))
        {
            element.lastUsedID = helper_getNextUsageID();
            return element.storage;
        }      
    }
    //Need to make sure this only happens when the files in the workspace folder are parsed already, else this file will get its own storage
    StorageElement newStorage;
    newStorage.lastUsedID = helper_getNextUsageID();
    newStorage.storage = std::make_shared<lsp::ArxmlStorage>();
    parseSingleFile(uri, newStorage.storage);
    storages_.push_back(newStorage);
    return newStorage.storage;
}

void lsp::XmlParser::parseSingleFile(const std::string uri, std::shared_ptr<ArxmlStorage> storage)
{
    storage->addFileIndex(uri);
    uint32_t fileIndex = storage->getFileIndex(uri);

    boost::iostreams::mapped_file mmap(helper_sanitizeUri(uri), boost::iostreams::mapped_file::readonly);
    std::cout << "Parsing " << uri << "\n";
    auto t0 = std::chrono::high_resolution_clock::now();
    parseNewlines(mmap, storage, fileIndex);
    auto t1 = std::chrono::high_resolution_clock::now();
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count() << "ms - Newlines\n";
    auto t2 = std::chrono::high_resolution_clock::now();
    parseShortnamesAndReferences(mmap, storage, fileIndex);
    auto t3 = std::chrono::high_resolution_clock::now();
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t2).count() << "ms - Shortnames/References\n\n";
    mmap.close();
}

void lsp::XmlParser::parseFullFolder(const lsp::types::DocumentUri uri)
{
    std::string sanitizedFilePath = std::string(uri.begin(), uri.end());
    auto colonPos = sanitizedFilePath.find("%3A");
    sanitizedFilePath.replace(colonPos, 3, ":");
    sanitizedFilePath = sanitizedFilePath.substr(colonPos - 1);
    boost::filesystem::path path(sanitizedFilePath);

    StorageElement newStorage;
    newStorage.lastUsedID = helper_getNextUsageID();
    newStorage.storage = std::make_shared<lsp::ArxmlStorage>();

    if(boost::filesystem::is_directory(path))
    {
        std::vector<std::string> files = helper_getARXMLFilePathsInDirectory(path);
        for( auto &file : files)
        {
            parseSingleFile(helper_makeURI(file), newStorage.storage);
        }
    }
    else
    {
        throw lsp::elementNotFoundException();
    }
    storages_.push_back(newStorage);
}

void lsp::XmlParser::parseNewlines(boost::iostreams::mapped_file &mmap, std::shared_ptr<ArxmlStorage> storage, uint32_t fileIndex)
{
    const char *const start = mmap.const_data();
    const char *current = start;
    const char *const end = current + mmap.size();

    if (current)
    {
        //First, go through once and count the number, so we can reserve enough space
        uint32_t numLines = std::count(start, end, '\n');
        storage->reserveNewlineOffsets(numLines + 1, fileIndex);
        storage->addNewlineOffset(0, fileIndex);

        while (current && current < end)
        {
            current = static_cast<const char *>(memchr(current, '\n', end - current));
            if (current)
            {
                storage->addNewlineOffset(current - start, fileIndex);
                ++current;
            }
        }
    }
}

enum class tagType
{
    opening_tag,
    closing_tag,
    shortname,
    reference,
    undefined_tag,
};

void lsp::XmlParser::parseShortnamesAndReferences(boost::iostreams::mapped_file &mmap, std::shared_ptr<ArxmlStorage> storage, uint32_t fileIndex)
{
    tagType lastTag = tagType::undefined_tag;
    tagType currentTag = tagType::undefined_tag;
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
            ++current;
            current = strstr(current, "-->") + 3;
        }

        /// xml info - skip ///
        else if(*(current) == '?')
        {
            ++current;
            current = strstr(current, "?>") + 2;
        }

        /// closing tag - decrease depth ///
        else if (*(current) == '/')
        {
            if (currentTag == tagType::shortname && lastTag == tagType::opening_tag)
            {
                static const std::string ident_compare = "IDENT>";
                std::string tagContent(current + 1, 6);
                if(!ident_compare.compare(tagContent))
                    --(depthElements.back().first);
            }
            --depth;
            ++current;
            current = static_cast<const char *>(memchr(current, '>', end - current)) + 1;
            //if we have the form <open><shortname>name</shortname></open> then we want to ignore the open tag in the depth calculation
            //in order to associate name with other elements of that depth, but name would be getting removed from the depthElements because of the indentation
            if (!depthElements.empty())
            {
                while (depthElements.back().first > depth)
                {
                    depthElements.pop_back();
                    if (depthElements.empty())
                        break;
                }
            }
            lastTag = currentTag;
            currentTag = tagType::closing_tag;
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
                element.fileIndex = fileIndex;

                const ShortnameElement* elementPtr = storage->addShortname(element);
                if(depthElements.size())
                {
                    depthElements.back().second->children.push_back(elementPtr);
                }
                depthElements.push_back(std::make_pair(depth, elementPtr));

                current = endChar + 13;
                lastTag = currentTag;
                currentTag = tagType::shortname;
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
                reference.owner = nullptr;
                reference.fileIndex = fileIndex;
                if(depthElements.size())
                {
                    reference.owner = depthElements.back().second;
                    depthElements.back().second->references.push_back(storage->addReference(reference));
                }
                else
                {
                    storage->addReference(reference);
                }
                current = static_cast<const char*>(memchr(current, '>', end - current));
                lastTag = currentTag;
                currentTag = tagType::reference;
            }
            /// random tag - increase depth and skip ///
            else
            {
                current = static_cast<const char*>(memchr(current, '>', end - current)) + 1;
                if (*(current - 2) != '/')
                {
                    ++depth;
                    lastTag = currentTag;
                    currentTag = tagType::opening_tag;
                }
            }
        } 
    }
}