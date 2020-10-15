#include <iostream>
#include <string>

#include "readWrite.hpp"
#include "methods.hpp"

#include "boost/asio.hpp"
#include "json.hpp"
#include "jsonrpcpp.hpp"

//Todo: refactor, maybe wrapper around the callback functions and a register_callback function
void processMessage(asio::ip::tcp::socket &socket, jsonrpcpp::Parser &parser, std::string &message)
{
    try
    {
        jsonrpcpp::entity_ptr entity = parser.do_parse(message);
        if (entity)
        {
            //response
            if(entity->is_response())
            {
                std::cout << "Response received:\n" << entity->to_json().dump(2) << "\n";
            }

            //request
            if(entity->is_request())
            {
                std::cout << "Request received:\n" << entity->to_json().dump(2) << "\n";
                jsonrpcpp::request_ptr request = std::dynamic_pointer_cast<jsonrpcpp::Request>(entity);
                std::cout << "Requested method: " << request->method() << "\n\n";
                //All Methods here
                jsonrpcpp::response_ptr response = nullptr;
                std::string reqMethod = request->method();
                
                if(reqMethod == "initialize")
                {
                    response = requests::initialize(request->id(), request->params());
                }

                std::string responseString = response->to_json().dump();
                write_(socket, responseString);
                std::cout << "Response sent:\n" << response->to_json().dump(2) << "\n\n";
            }

            //notification
            if(entity->is_notification())
            {
                std::cout << "Notification received:\n" << entity->to_json().dump(2)<< "\n";
                jsonrpcpp::notification_ptr notification = std::dynamic_pointer_cast<jsonrpcpp::Notification>(entity);

                //All Notifications here
                if(notification->method() == "initialized")
                {
                    notifications::initialized(notification->params());
                }
            }

            //batch
            if(entity->is_batch())
            {
                //TODO batch processing
            }
        }
    }
    catch(const jsonrpcpp::RequestException &e)
    {
        std::cerr << e.what() << "\n";
    }
    catch(const jsonrpcpp::ParseErrorException &e)
    {
        std::cerr << e.what() << "\n";
    }
    catch(const jsonrpcpp::RpcException &e)
    {
        std::cerr << e.what() << "\n";
    }
    catch(const std::exception &e)
    {
        std::cerr << e.what() << "\n";
    }
}

int main()
{
    asio::io_service ios;
    asio::ip::tcp::endpoint endPoint(asio::ip::address::from_string("127.0.0.1"), 12730);
    asio::ip::tcp::socket socket(ios, endPoint.protocol());
    socket.connect(endPoint);

        jsonrpcpp::Parser parser;
    parser.register_notification_callback("initialized", notifications::initialized);
    parser.register_request_callback("initialize", requests::initialize);
    parser.register_request_callback("textDocument/hover", requests::textDocument::hover);

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
    }
}
