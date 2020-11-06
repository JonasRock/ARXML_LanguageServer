#include <iostream>

#include "boost/asio.hpp"

#include "languageService.hpp"

using namespace boost;

int main(int argc, char** argv)
{
    uint32_t portNr;
    //Get the port from the command line
    if( argc == 1 )
    {
        //No port specified
        std::cout << "Specify port to connect to: ";
        std::cin >> portNr;
    }
    else if (argc > 1)
    {
        portNr = std::stoi(argv[1]);
    }

    //init
    lsp::LanguageService::start("127.0.0.1", portNr);
}