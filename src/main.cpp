/**
 * @file main.cpp
 * @author Jonas Rock
 * @brief socket connection and callback registration goes here
 * @version 0.1
 * @date 2020-10-27
 *
 * 
 */

#include <iostream>
#include <string>

#include "boost/asio.hpp"
#include "json.hpp"
#include "lspParser.hpp"

#include "readWrite.hpp"
#include "methods.hpp"
#include "xmlParser.hpp"
#include "configurationGlobals.h"

int main(int argc, char** argv)
{
    int portNr;
    //Get the port from the command line
    if( argc == 1 )
    {
        //No port specified
        std::cout << "Specify port to connect to: ";
        std::cin >> portNr;
    }
    else if (argc > 1)
    {
        portNr = atoi(argv[1]);
    }

    //init
    asio::io_service ios;
    asio::ip::tcp::endpoint endPoint(asio::ip::address::from_string("127.0.0.1"), portNr);
    asio::ip::tcp::socket socket(ios, endPoint.protocol());
    std::cout << "Connecting to 127.0.0.1:" << portNr << "\n";
    socket.connect(endPoint);

    lspParser_ptr = std::make_shared<lsp::Parser>();

    //Register the callbacks for the LPS messages
    parser.register_notification_callback("initialized", methods::notification_initialized);
    parser.register_notification_callback("exit", methods::notification_exit);
    parser.register_request_callback("initialize", methods::request_initialize);
    parser.register_request_callback("shutdown", methods::request_shutdown);
    parser.register_request_callback("textDocument/documentColor", methods::request_textDocument_documentColor);
    parser.register_request_callback("textDocument/references", methods::request_textDocument_references);
    parser.register_request_callback("textDocument/definition", methods::request_textDocument_definition);

    while(1)
    {
        std::string message;
        std::size_t numRead = read_(socket, message);
        std::cout << "Receiving Message:\n" << message << "\n\n";

        jsonrpcpp::entity_ptr ret = parser.parse(message);
        if (ret->is_response())
        {
            std::string responseMessage = std::dynamic_pointer_cast<jsonrpcpp::Response>(ret)->to_json().dump();
            write_(socket, responseMessage);
            std::cout << "Sending Message:\n" << responseMessage << "\n\n";
        }

        // I counldn't get jsonrpcpp to parse the shutdown request with a null parameter so it crashes on parsing that,
        // but since its supposed to shut down anyways, its a fix for later
        // if (shutdown) {return 0;}
    }
}