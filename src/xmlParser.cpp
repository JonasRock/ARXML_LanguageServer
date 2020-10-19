#include <string>
#include <iostream>
#include "boost/iostreams/device/mapped_file.hpp"
#include <chrono>

using namespace boost;

void readFile()
{
    iostreams::mapped_file mmap("C:\\Users\\jr83522\\Desktop\\BMS48V.arxml", boost::iostreams::mapped_file::readonly);
    const char* begin = mmap.const_data();
    const char* end = begin + mmap.size();
    const char* temp = begin;
    std::vector<std::string> shortnames;
    int numLines = 0;

    auto t0 = std::chrono::high_resolution_clock::now();
    //ca. 100 000/140 000 Shortnames pro Sekunde -> ca. 5 - 8 Sekunden für 1 GB
    while (begin && begin < end)
    {
        if (begin = static_cast<const char*>(strstr(begin, "<SHORT-NAME>")))
        {
            begin += 12;
            temp = strstr(begin, "</SHORT-NAME>");
            shortnames.push_back(std::string(begin, static_cast<size_t>(temp-begin)));
            numLines++;
            begin += 13;
        }
    }
    auto t1 = std::chrono::high_resolution_clock::now();
    std::cout << std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count() << " us: ";
    std::cout << numLines << " SHORT-NAMES found\n";
}