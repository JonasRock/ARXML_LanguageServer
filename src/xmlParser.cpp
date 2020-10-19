#include <string>
#include <iostream>
#include "boost/iostreams/device/mapped_file.hpp"
#include <chrono>
#include <utility>

using namespace boost;

struct shortnamePosition
{
    std::string shortname;
    std::size_t lineNr;
    std::size_t charNrStart;
};

void readFile()
{
    auto t0 = std::chrono::high_resolution_clock::now();

    iostreams::mapped_file mmap("C:\\Users\\jr83522\\Desktop\\BMS48V.arxml", boost::iostreams::mapped_file::readonly);
    const char* begin = mmap.const_data();
    const char* const end = begin + mmap.size();
    std::vector<shortnamePosition> shortnames;
    int numLines = 5;

    //VLT Besser?
    // ~5-8ms for a 2 mb file with 27000 lines -< 2.5 to 4 seconds for a 1gb file //in release mode
    while (begin && begin < end)
    {
        if(begin = static_cast<const char*>(memchr(begin, '<', end - begin)))
        {
            if(!strncmp(begin + 1, "SHORT-NAME>", 11))
            {
                //Check how many spaces were before we found the '<' for the tree calculation
                //Found a tag, check if its a "<SHORT-NAME>""
                int numSpaces = 0;
                while(*(begin - numSpaces - 1)== ' ')
                {
                    numSpaces += 1;
                }
                begin += 12; //We already compared with "SHORT-NAME>, we can skip ahead that much"
                
                const char* endChar = static_cast<const char*>(memchr(begin, '<', end - begin)); //Find the closing tag

                shortnamePosition temp;
                temp.lineNr = numLines;
                temp.charNrStart = numSpaces + 12;
                temp.shortname = std::string(begin, static_cast<size_t>(endChar-begin));
                shortnames.push_back(temp);

                //We can skip the next twelve chars, they are "/SHORT-NAME>""
                begin = endChar + 12;
            }
            numLines++;
            begin = static_cast<const char*>(memchr(begin, '\n', end - begin));

            if(begin) // Else it would be an error
            {
                begin++;
            }
        }
    }
        
    auto t1 = std::chrono::high_resolution_clock::now();
    std::cout << std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count() << " us: ";
    std::cout << numLines << " Lines parsed\n";
}