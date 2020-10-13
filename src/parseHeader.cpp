#include <string>
#include <iostream>

int parseHeader()
{
    //The header ends contains the lenght of the content encoded as a string after "Content-Lenght: "
    std::cin.ignore(14, ':');
    if(std::cin.peek() == ' ')
        std::cin.ignore();
    std::string contentSize;
    std::getline(std::cin, contentSize, '\r');
    std::cin.ignore();
    if (std::cin.get() == '\r' && std::cin.peek() == '\n')
    {
        std::cin.ignore();
        return std::stoi(contentSize);
    }
    else 
    {
        std::cerr << "Header doesn't end after \"Content-Length\" parameter";
        std::cin.unget();
        return 0;
    }
}