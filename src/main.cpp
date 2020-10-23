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

    std::string pathToFile = "C:/Users/jr83522/Desktop/E3_1_2_Premium_V12.04.20A_AR430_20201011_IPB_BP.arxml";
    auto xparse5 = std::make_shared<xmlParser>(pathToFile);
    xparse5->parse();
    xParsePtr = xparse5;

    //init
    asio::io_service ios;
    asio::ip::tcp::endpoint endPoint(asio::ip::address::from_string("127.0.0.1"), 12730);
    asio::ip::tcp::socket socket(ios, endPoint.protocol());
    socket.connect(endPoint);


    jsonrpcpp::Parser parser;

    //Register the callbacks for the LPS messages
    parser.register_notification_callback("initialized", notifications::initialized);
    parser.register_notification_callback("exit", notifications::exit);
    parser.register_request_callback("initialize", requests::initialize);
    parser.register_request_callback("shutdown", requests::shutdown);
    parser.register_request_callback("textDocument/hover", requests::textDocument::hover);
    parser.register_request_callback("textDocument/definition", requests::textDocument::definition);

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
