#include <string>
#include <iostream>
#include "boost/iostreams/device/mapped_file.hpp"
#include "boost/property_tree/ptree.hpp"
#include <chrono>
#include <utility>
#include <stack>

using namespace boost;

void readFile()
{
    //For debugging
    auto t0 = std::chrono::high_resolution_clock::now();

    iostreams::mapped_file mmap("C:\\Users\\jr83522\\Desktop\\BMS48V.arxml", boost::iostreams::mapped_file::readonly);
    const char* start = mmap.const_data();
    const char* begin = start;
    const char* const end = begin + mmap.size();

    int depth = 0;

    //This tree contains the shortname path structure,
    //with the shortnames as keys and character offsets as values
    property_tree::basic_ptree<std::string, std::size_t> shortnameTree;

    //The depths vector holds the depth and associated shortnames, so we can 
    //decide where in the tree to add the newest shortname when found
    std::vector<std::pair<std::size_t, std::string>> depths;

    while( begin && begin < end)
    {
        

        //Go to the beginning of the next tag
        begin = static_cast<const char*>(memchr(begin, '<', end - begin));
        ++begin;
        //Check the tag

        //Closing tag
        if ( *(begin) == '/' )
        {
            --depth;
            //Skip to the end
            begin = static_cast<const char*>(memchr(++begin, '>', end - begin)) + 1;
        }

        //Shortname
        else if ( !strncmp(begin, "SHORT-NAME>", 11) )
        {
            //Skip to the end
            begin += 11;

            const char* endChar = static_cast<const char*>(memchr(begin, '<', end - begin)); //Find the closing tag

            std::string shortnameString(begin, static_cast<std::size_t>(endChar-begin));
            std::size_t charOffset = static_cast<std::size_t>(begin - start);

            //add the found shortname to the tree, according to the depths of the other components
            if(!depths.empty())
            {
                while ( depths.back().first >= depth )
                {
                    depths.pop_back();
                    if(depths.empty())
                    {
                        break;
                    }
                }
            }
            depths.push_back(std::make_pair(depth, shortnameString));

            std::string pathString = "";
            for(auto i: depths)
            {
                pathString += i.second + ".";
            }
            //remove the last '.'
            pathString.pop_back();

            shortnameTree.add( pathString, charOffset );

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
            if ( *(begin - 1) != '/' )
            {
                depth++;
            };
        }
    }

    //For debugging
    auto t1 = std::chrono::high_resolution_clock::now();
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count() << " ms\n";
    
    std::cout << "Tree Size: " << shortnameTree.size() << " Main Elements.\n";
}