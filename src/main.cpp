#include <iostream>
#include <string>
#include <vector>

#include "methods.hpp"
#include "readWrite.hpp"

#include "boost/asio.hpp"
#include "json.hpp"
#include "jsonrpcpp.hpp"

//Todo: refactor, maybe wrapper around the callback functions and a register_callback function
void processMessage(asio::ip::tcp::socket &socket, jsonrpcpp::Parser &parser, std::string &message)
{
    try
    {
        jsonrpcpp::entity_ptr entity = parser.parse(message);
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

                //All Methods here
                jsonrpcpp::response_ptr response;
                if(request->method() == "initalize")
                {
                    response = requests::initialize(request->id(), request->params());
                }


                std::string responseString = response->to_json().dump();
                std::vector<char> responseBuffer(responseString.begin(), responseString.end());
                write_(socket, responseBuffer);
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

    while(1)
    {
        std::vector<char> out;
        std::size_t numRead = read_(socket, out);
        std::string outs(out.begin(), out.end());
        std::cout << numRead << ": " << outs << std::endl;

        jsonrpcpp::Parser parser;
        processMessage(socket, parser, outs);
    }
}
