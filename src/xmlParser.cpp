#include <string>
#include <iostream>
#include <chrono>
#include <utility>
#include <algorithm>
#include <cstdint>
#include "boost/property_tree/ptree.hpp"
#include "boost/iostreams/device/mapped_file.hpp"
#include "xmlParser.hpp"

using namespace boost;

xmlParser::xmlParser(std::string filepath)
{
    mmap = iostreams::mapped_file(filepath, boost::iostreams::mapped_file::readonly);
}

xmlParser::~xmlParser()
{
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
        if(begin)
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
            shortnameProps props;
            props.offset = static_cast<uint32_t>(begin - start);

            //add the found shortname to the tree, according to the depths of the other components
            depths.push_back(std::make_pair(depth, shortnameString));

            std::string pathString = "";
            for(auto i: depths)
            {
                pathString += i.second + ".";
            }
            //remove the last '.'
            pathString.pop_back();

            shortnameTree.add( pathString, props );

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

    //For debugging
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
    const std::string searchPattern = " DEST=\"";
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
            it += 2; //also remove the preceding '/'
            const char* endOfReference = static_cast<const char*>(memchr(it, '<', end - it));
            std::string refString(it, static_cast<uint32_t>(endOfReference-it));
            auto opt = shortnameTree.get_optional<shortnameProps>(property_tree::path(refString, '/'));
            if (opt)
            {
                (*opt).referenceOffsetRange.push_back(std::make_pair(it - start, endOfReference - start));
            }
        }
        else
        {
            break;
        }
    }
}

position xmlParser::getPositionFromOffset(uint32_t offset)
{
    position ret;
    uint32_t index = std::lower_bound(newLineOffsets.begin(), newLineOffsets.end(), offset) - newLineOffsets.begin();
    ret.lineNr = index;
    ret.charPos = offset - newLineOffsets[index];
    return ret;
}

uint32_t xmlParser::getOffsetFromPosition(position pos)
{
    return newLineOffsets[pos.lineNr] + pos.charPos;
}