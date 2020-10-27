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

xmlParser::xmlParser(std::string pathname)
{
    mmap.open(pathname, iostreams::mapped_file::readonly);
}

xmlParser::~xmlParser()
{
}

void xmlParser::parse()
{
    auto t0 = std::chrono::high_resolution_clock::now();
    parseNewlines();
    auto t1 = std::chrono::high_resolution_clock::now();
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(t1-t0).count() << "ms - Newlines\n";
    auto t2 = std::chrono::high_resolution_clock::now();
    parseShortnames();
    auto t3 = std::chrono::high_resolution_clock::now();
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(t3-t2).count() << "ms - Shortnames\n";
    auto t4 = std::chrono::high_resolution_clock::now();
    parseReferences();
    auto t5 = std::chrono::high_resolution_clock::now();
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(t5-t4).count() << "ms - References\n";
    mmap.close();
}

void xmlParser::parseShortnames()
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
            element.fileOffset = begin - start - 1;

            storage.addShortname(element);
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

void xmlParser::parseNewlines()
{

    const char* const start = mmap.const_data();
    const char* begin = start;
    const char* const end = begin + mmap.size();


    if (begin)
    {
        //First, go through once and count the number, so we can reserve enough space
        uint32_t numLines = std::count(start, end, '\n');
        newLineOffsets.reserve(numLines + 1);
        newLineOffsets.push_back(0);

        while(begin && begin < end)
        {
            begin = static_cast<const char*>(memchr(begin, '\n', end - begin));
            if (begin)
            {
                newLineOffsets.push_back(begin - start);
                begin++;
            }
        }
    }
}

void xmlParser::parseReferences()
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
                shortnameElement target = storage.getByFullPath(refString);
                referenceRange ref;
                ref.targetOffsetRange = std::make_pair(target.fileOffset, static_cast<uint32_t>(target.fileOffset + target.name.size()));
                ref.refOffsetRange = std::make_pair(static_cast<uint32_t>(it - start - 2), static_cast<uint32_t>(endOfReference - start - 1));
                storage.addReference(ref);
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

lsp::Position xmlParser::getPositionFromOffset(const uint32_t offset)
{
    lsp::Position ret;
    uint32_t index = std::lower_bound(newLineOffsets.begin(), newLineOffsets.end(), offset) - newLineOffsets.begin() - 1;
    ret.character = offset - newLineOffsets[index];
    ret.line = index;
    return ret;
}

uint32_t xmlParser::getOffsetFromPosition(const lsp::Position &pos)
{
    return newLineOffsets[pos.line] + pos.character;
}


lsp::LocationLink xmlParser::getDefinition(const lsp::TextDocumentPositionParams &params)
{
    uint32_t offset = getOffsetFromPosition(params.position);
    referenceRange ref = storage.getReferenceByOffset(offset);

    uint32_t cursorDistanceFromRefBegin = offset - ref.refOffsetRange.first;

    //Get the shortname pointed at, so we can see its path and calculate where
    //on the reference we clicked, so we can go to the different parts of the path
    shortnameElement elem = storage.getByOffset(ref.targetOffsetRange.first);
    std::string fullPath = elem.getFullPath();
    std::string searchPath = std::string(
        fullPath.begin(),
        std::find(fullPath.begin() + cursorDistanceFromRefBegin, fullPath.end(), '/')
    );
    elem = storage.getByFullPath(searchPath);
    
    lsp::LocationLink link;
    link.originSelectionRange.start = getPositionFromOffset(elem.getOffsetRange().first);
    link.originSelectionRange.end = getPositionFromOffset(elem.getOffsetRange().second);
    link.targetRange.start = getPositionFromOffset(elem.getOffsetRange().first);
    link.targetRange.end = getPositionFromOffset(elem.getOffsetRange().second);
    link.targetSelectionRange = link.targetRange;
    link.targetUri = params.textDocument.uri;

    return link;
}

std::vector<lsp::Location> xmlParser::getReferences(const lsp::ReferenceParams &params)
{
    uint32_t offset = getOffsetFromPosition(params.position);
    std::pair<uint32_t, uint32_t> ref = storage.getByOffset(offset).getOffsetRange();

    std::vector<lsp::Location> foundReferences;

    if (params.context.includeDeclaration)
    {
        lsp::Location toAdd;
        toAdd.uri = params.textDocument.uri;
        toAdd.range.start = getPositionFromOffset(ref.first);
        toAdd.range.end = getPositionFromOffset(ref.second);
        foundReferences.push_back(toAdd);
    }

    for (auto &a : storage.references)
    {
        if( a.targetOffsetRange.first >= ref.first && a.targetOffsetRange.second <= ref.second)
        {
            lsp::Location toAdd;
            toAdd.uri = params.textDocument.uri;
            toAdd.range.start = getPositionFromOffset(a.refOffsetRange.first);
            toAdd.range.end = getPositionFromOffset(a.refOffsetRange.second);
            foundReferences.push_back(toAdd);
        }
    }

    if (!foundReferences.size())
    {
        throw lsp::elementNotFoundException();
    } else
    {
        return foundReferences;
    }
    
}