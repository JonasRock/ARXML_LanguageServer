#include <iostream>
#include <string>

#include "boost/asio.hpp"
#include "json.hpp"
#include "jsonrpcpp.hpp"

#include "readWrite.hpp"
#include "methods.hpp"
#include "xmlParser.hpp"

//TODO: parseNewlines
//TODO: toCharOffset, fromCharOffset
//TODO: parseReferences - maybe add the positions as a array to the tree
//      the shortnames are sorted in the propertytree, so we can use this info to find
//          the shortname for a given position relatively quickly

//TODO: change size_t to unit32_t, should be plenty

int main()
{
    auto t0 = std::chrono::high_resolution_clock::now();
    auto xparse = new xmlParser("C:/Users/jr83522/Desktop/BMS48V.arxml");
    (*xparse).parseShortnames();
    (*xparse).parseReferences();
    (*xparse).parseNewlines();
    delete xparse;
    auto t1 = std::chrono::high_resolution_clock::now();

    auto t2 = std::chrono::high_resolution_clock::now();
    auto xparse2 = new xmlParser("C:/Users/jr83522/Desktop/E3_1_2_Premium_V12.04.20A_AR430_20201011_RSG_48V_BP.arxml");
    (*xparse2).parseShortnames();
    (*xparse2).parseReferences();
    (*xparse2).parseNewlines();
    delete xparse2;
    auto t3 = std::chrono::high_resolution_clock::now();

    auto t4 = std::chrono::high_resolution_clock::now();
    auto xparse3 = new xmlParser("C:/Users/jr83522/Desktop/E3_1_2_Premium_V12.04.20A_AR430_20201011_IPB_BP.arxml");
    (*xparse3).parseShortnames();
    (*xparse3).parseReferences();
    (*xparse3).parseNewlines();
    delete xparse3;
    auto t5 = std::chrono::high_resolution_clock::now();

    auto t6 = std::chrono::high_resolution_clock::now();
    auto xparse4 = new xmlParser("C:/Users/jr83522/Desktop/E3_1_2_Premium_V12.04.20A_AR430_20201011_Airbag_6D_BP.arxml");
    (*xparse4).parseShortnames();
    (*xparse4).parseReferences();
    (*xparse4).parseNewlines();
    delete xparse4;
    auto t7 = std::chrono::high_resolution_clock::now();

    auto t8 = std::chrono::high_resolution_clock::now();
    auto xparse5 = new xmlParser("C:/Users/jr83522/Desktop/E3_1_2_Premium_V12.04.20A_AR430_20201011_HCP4_uC1_1_BP.arxml");
    (*xparse5).parseShortnames();
    (*xparse5).parseReferences();
    (*xparse5).parseNewlines();
    delete xparse5;
    auto t9 = std::chrono::high_resolution_clock::now();

    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(t1-t0).count() << "ms\n";
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(t3-t2).count() << "ms\n";
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(t5-t4).count() << "ms\n";
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(t7-t6).count() << "ms\n";
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(t9-t8).count() << "ms\n";

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
    parser.register_request_callback("textDocument/hover", requests::textDocument::hover);
    parser.register_request_callback("shutdown", requests::shutdown);


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
