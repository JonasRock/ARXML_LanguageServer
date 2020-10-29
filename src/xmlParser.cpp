/**
 * @file xmlParser.cpp
 * @author Jonas Rock
 * @brief implementation of arxml file parsing
 * @version 0.1
 * @date 2020-10-27
 * 
 * 
 */

#include <string>
#include <iostream>
#include <chrono>
#include <utility>
#include <algorithm>
#include <cstdint>

#include "boost/iostreams/device/mapped_file.hpp"

#include "xmlParser.hpp"
#include "shortnameStorage.hpp"
#include "lspExceptions.hpp"

using namespace boost;

xmlParser::xmlParser()
{
}

xmlParser::~xmlParser()
{
}

/**
 * @brief If not yet parsed, parses the document and returns the storage of the parsed data
 * 
 * @param uri Document to be parsed
 * @return std::shared_ptr<shortnameStorage> the parsed data
 */
std::shared_ptr<shortnameStorage> xmlParser::parse(const lsp::DocumentUri uri)
{
    std::string docString = std::string(uri.begin() + 5, uri.end());
    auto repBegin = docString.find("%3A");
    docString.replace(repBegin, 3, ":");
    std::string sanitizedDocString = docString.substr(repBegin-1);

    auto it = storages.find(sanitizedDocString);
    if (it != storages.end())
    {
        return it->second;
    }
    else
    {
        auto ret = storages.emplace(std::make_pair(sanitizedDocString, std::make_shared<shortnameStorage>())).first->second;
        iostreams::mapped_file mmap(sanitizedDocString, iostreams::mapped_file::readonly);

        auto t0 = std::chrono::high_resolution_clock::now();
        parseNewlines(mmap, ret);
        auto t1 = std::chrono::high_resolution_clock::now();
        std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(t1-t0).count() << "ms - Newlines\n";
        auto t2 = std::chrono::high_resolution_clock::now();
        parseShortnames(mmap, ret);
        auto t3 = std::chrono::high_resolution_clock::now();
        std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(t3-t2).count() << "ms - Shortnames\n";
        auto t4 = std::chrono::high_resolution_clock::now();
        parseReferences(mmap, ret);
        auto t5 = std::chrono::high_resolution_clock::now();

        mmap.close();

        std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(t5-t4).count() << "ms - References\n";
        return ret;
    }
    
    
}

/**
 * @brief second step of the parsing process is extracting all shortnames and their corresponding information
 * 
 */
void xmlParser::parseShortnames(iostreams::mapped_file &mmap,std::shared_ptr<shortnameStorage> storage)
{
    const char* start = mmap.const_data();
    const char* begin = start;
    const char* const end = begin + mmap.size();

    uint32_t depth = 0;

    //The depths vector holds the depth and associated shortnames, so we can 
    //decide where in the tree to add the newest shortname when found
    std::vector<std::pair<std::uint32_t, std::string>> depths;

    while( begin && begin < end)
    {
        
        //Go to the beginning of the next tag
        begin = static_cast<const char*>(memchr(begin, '<', end - begin));
        if(!begin)
        {
            break;
        }
        ++begin;
        
        //Check the tag

        //Closing tag
        if ( *(begin) == '/' )
        {
            --depth;
            //Skip to the end
            begin = static_cast<const char*>(memchr(++begin, '>', end - begin)) + 1;
            if(!depths.empty())
            {
                while ( depths.back().first > depth )
                {
                    depths.pop_back();
                    if(depths.empty())
                    {
                        break;
                    }
                }
            }
        }

        //Shortname
        else if ( !strncmp(begin, "SHORT-NAME>", 11) )
        {
            //Skip to the end
            begin += 11;

            const char* endChar = static_cast<const char*>(memchr(begin, '<', end - begin)); //Find the closing tag
            std::string shortnameString(begin, static_cast<uint32_t>(endChar-begin));

            //add the found shortname to the tree, according to the depths of the other components
            std::string pathString = "";
            for(auto i: depths)
            {
                pathString += i.second + "/";
            }
            //remove the last '/'
            if(pathString.size())
            {
                pathString.pop_back();
            }
            depths.push_back(std::make_pair(depth, shortnameString));

            shortnameElement element;
            element.name = shortnameString;
            element.path = pathString;
            element.fileOffset = begin - start; //This one is correct

            storage->addShortname(element);
            begin = endChar + 13;
        }

        //XML Comment
        else if ( *(begin) == '!' )
        {
            //Skip to the end
            begin = strstr(++begin, "-->") + 3;
        }

        //XML Info
        else if( *(begin) == '?' )
        {
            //Skip to the end
            begin = strstr(++begin, "?>") + 2;
        }

        //Normal XML Element
        else
        {
            //Skip to the end
            begin = static_cast<const char*>(memchr(begin, '>', end - begin)) + 1;

            //Check for empty elements that don't increase the depth
            if ( *(begin - 2) != '/' )
            {
                depth++;
            };
        }
    }
}

/**
 * @brief first step of the parsing process is to process all linebreaks as the format between lsp::Position and offset representation differs
 * 
 */
void xmlParser::parseNewlines(iostreams::mapped_file &mmap, std::shared_ptr<shortnameStorage> storage)
{

    const char* const start = mmap.const_data();
    const char* begin = start;
    const char* const end = begin + mmap.size();


    if (begin)
    {
        //First, go through once and count the number, so we can reserve enough space
        uint32_t numLines = std::count(start, end, '\n');
        storage->newlineOffsets.reserve(numLines + 1);
        storage->newlineOffsets.push_back(0);

        while(begin && begin < end)
        {
            begin = static_cast<const char*>(memchr(begin, '\n', end - begin));
            if (begin)
            {
                storage->newlineOffsets.push_back(begin - start);
                begin++;
            }
        }
    }
}

/**
 * @brief last step of the parsing process to find and link up the references to the parsed shortnames
 * 
 */
void xmlParser::parseReferences(iostreams::mapped_file &mmap, std::shared_ptr<shortnameStorage> storage)
{
    const std::string searchPattern = "REF DEST=\"";
    std::boyer_moore_searcher searcher(searchPattern.begin(), searchPattern.end());
    auto it = mmap.const_begin();
    
    const char* end = mmap.const_end();
    const char* start = mmap.const_begin();

    while(1)
    {
        it = std::search(it, end, searcher);
        if (it < end)
        {
            it = static_cast<const char*>(memchr(it, '>', end - it));
            if(!it)
            {
                break;
            }
            it += 2; //Remove '/' at the front
            const char* endOfReference = static_cast<const char*>(memchr(it, '<', end - it));
            std::string refString(it, static_cast<uint32_t>(endOfReference-it));
            try{
                shortnameElement target = storage->getByFullPath(refString);
                referenceRange ref;
                ref.targetOffsetRange = std::make_pair(target.fileOffset, static_cast<uint32_t>(target.fileOffset + target.name.size()) - 1); //This is correct
                ref.refOffsetRange = std::make_pair(static_cast<uint32_t>(it - start - 1), static_cast<uint32_t>(endOfReference - start - 1)); //This is correct
                storage->addReference(ref);
            }
            catch (lsp::elementNotFoundException &e)
            {
                //no need to do anything, just ignore that reference
            }
        }
        else
        {
            break;
        }
    }
}



lsp::LocationLink xmlParser::getDefinition(const lsp::TextDocumentPositionParams &params)
{
    auto storage = parse(params.textDocument.uri);
    uint32_t offset = storage->getOffsetFromPosition(params.position);
    referenceRange ref = storage->getReferenceByOffset(offset);

    uint32_t cursorDistanceFromRefBegin = offset - ref.refOffsetRange.first;

    //Get the shortname pointed at, so we can see its path and calculate where
    //on the reference we clicked, so we can go to the different parts of the path
    shortnameElement elem = storage->getByOffset(ref.targetOffsetRange.first);
    std::string fullPath = elem.getFullPath();
    std::string searchPath = std::string(
        fullPath.begin(),
        std::find(fullPath.begin() + cursorDistanceFromRefBegin, fullPath.end(), '/')
    );
    elem = storage->getByFullPath(searchPath);
    
    lsp::LocationLink link;
    link.originSelectionRange.start = storage->getPositionFromOffset(elem.getOffsetRange().first);
    link.originSelectionRange.end = storage->getPositionFromOffset(elem.getOffsetRange().second);
    link.targetRange.start = storage->getPositionFromOffset(elem.getOffsetRange().first - 1);
    link.targetRange.end = storage->getPositionFromOffset(elem.getOffsetRange().second);
    link.targetSelectionRange = link.targetRange;
    link.targetUri = params.textDocument.uri;

    return link;
}

std::vector<lsp::Location> xmlParser::getReferences(const lsp::ReferenceParams &params)
{
    auto storage = parse(params.textDocument.uri);
    std::vector<lsp::Location> foundReferences;
    std::pair<uint32_t, uint32_t> shortnameRange;
    try
    {
        uint32_t offset = storage->getOffsetFromPosition(params.position);
        shortnameRange = storage->getByOffset(offset).getOffsetRange();

        for (auto &a : storage->references)
        {
            if( a.targetOffsetRange.first >= shortnameRange.first && a.targetOffsetRange.second <= shortnameRange.second)
            {
                lsp::Location toAdd;
                toAdd.uri = params.textDocument.uri;
                toAdd.range.start = storage->getPositionFromOffset(a.refOffsetRange.first - 1);
                toAdd.range.end = storage->getPositionFromOffset(a.refOffsetRange.second);
                foundReferences.push_back(toAdd);
            }
        }
    }
    //No shortname at this position, but maybe its part of a reference
    catch(const lsp::elementNotFoundException &e)
    {
        uint32_t offset = storage->getOffsetFromPosition(params.position);
        referenceRange ref = storage->getReferenceByOffset(offset);
        uint32_t cursorDistanceFromRefBegin = offset - ref.refOffsetRange.first;

        //Get the shortname pointed at, so we can see its path and calculate where
        //on the reference we clicked, so we can go to the different parts of the path
        shortnameElement elem = storage->getByOffset(ref.targetOffsetRange.first);
        std::string fullPath = elem.getFullPath();
        std::string searchPath = std::string(
            fullPath.begin(),
            std::find(fullPath.begin() + cursorDistanceFromRefBegin, fullPath.end(), '/')
        );
        elem = storage->getByFullPath(searchPath);
        lsp::ReferenceParams newParams = params;
        lsp::Position newPos = storage->getPositionFromOffset(elem.fileOffset);
        newParams.position = newPos;
        //This will at not recurse more than 1 time, because now we can be sure to have an actual shortname position
        return getReferences(newParams);
    }
    

    if (!foundReferences.size())
    {
        throw lsp::elementNotFoundException();
    } 
    else
    {
        if (params.context.includeDeclaration)
        {
            lsp::Location toAdd;
            toAdd.uri = params.textDocument.uri;
            toAdd.range.start = storage->getPositionFromOffset(shortnameRange.first - 1);
            toAdd.range.end = storage->getPositionFromOffset(shortnameRange.second);
            foundReferences.push_back(toAdd);
        }
        return foundReferences;
    }

    
}