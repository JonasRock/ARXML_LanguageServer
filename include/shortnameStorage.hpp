#ifndef SHORTNAMESTORAGE_H
#define SHORTNAMESTORAGE_H

#include <string>
#include <cstdint>
#include <deque>
#include <list>

class shortnameElement
{
public:
    //the stringpath through the shortname hierarchy, without leading or trailing '/'
    std::string path;

    //the shortname string itself
    std::string name;

    //the offset in the file to the first character of the shortname
    uint32_t fileOffset;

    //vector of the start positions of the references in the file
    std::list<uint32_t> referenceOffsets;

    std::pair<uint32_t, uint32_t> getOffsetRange() const
    {
        return std::make_pair(fileOffset, name.size());
    }

    //Returns the full path of the shortname including itself
    std::string getFullPath () const
    {
        return "/" + path + "/" + name;
    }
};

//If the deque turns out too slow, try using a vector instead,
//but this requires counting the amount of shortnames beforehand in an extra parse (probably?)
//maybe add another deque/vector/whatever to keep track of reference Ranges and corresponding shortnameElements
class shortnameStorage
{
public:
    void add(const shortnameElement &elem);
    //Throws std::elementNotFoundException
    shortnameElement &getByName(const std::string &searchName);
    //Throws std::elementNotFoundException
    shortnameElement &getByFullPath(const std::string &searchPath);
    //Throws std::elementNotFoundException
    shortnameElement &getByOffset(const uint32_t searchOffset);

private:
    std::deque<shortnameElement> shortnames;
};

#endif /* SHORTNAMESTORAGE_H */