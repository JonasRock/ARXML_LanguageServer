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
#include "jsonrpcpp.hpp"

#include "readWrite.hpp"
#include "methods.hpp"
#include "xmlParser.hpp"
#include "configurationGlobals.h"

//TODO: parseNewlines
//TODO: toCharOffset, fromCharOffset
//TODO: parseReferences - maybe add the positions as a array to the tree
//      the shortnames are sorted in the propertytree, so we can use this info to find
//          the shortname for a given position relatively quickly

//TODO: change size_t to unit32_t, should be plenty

int main()
{
    //init
    asio::io_service ios;
    asio::ip::tcp::endpoint endPoint(asio::ip::address::from_string("127.0.0.1"), 12730);
    asio::ip::tcp::socket socket(ios, endPoint.protocol());
    socket.connect(endPoint);

    jsonrpcpp::Parser parser;

    //Register the callbacks for the LPS messages
    parser.register_notification_callback("initialized", methods::notification_initialized);
    parser.register_notification_callback("exit", methods::notification_exit);
    parser.register_request_callback("initialize", methods::request_initialize);
    parser.register_request_callback("shutdown", methods::request_shutdown);
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
            //std::cout << "Sending Message:\n" << responseMessage << "\n\n";
        }

        // I counldn't get jsonrpcpp to parse the shutdown request with a null parameter so it crashes on parsing that,
        // but since its supposed to shut down anyways, its a fix for later
        // if (shutdown) {return 0;}
    }
}
