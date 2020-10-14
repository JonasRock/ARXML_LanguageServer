#include <iostream>
#include <string>
#include <vector>

#include "boost/asio.hpp"
#include "jsonrpcpp.hpp"

#include "methods.hpp"
#include "readWrite.hpp"

// void processMessage(jsonrpcpp::Parser &parser, std::string message)
// {
//     try
//         {
//             jsonrpcpp::entity_ptr entity = parser.parse(outs);
//             if (entity)
//             {
//                 if(entity->is_response())
//                 {
//                     std::cout << "Response received:\n" << entity->to_json().dump(2) << "\n";
//                 }
//                 if(entity->is_request())
//                 {
//                     std::cout << "Request received:\n" << entity->to_json().dump(2) << "\n";
//                     jsonrpcpp::Response response = handleRequest(std::dynamic_pointer_cast<jsonrpcpp::Request>(entity));
//                     //TODO Send back response
//                 }
//                 if(entity->is_notification())
//                 {
//                     std::cout << "Notification received:\n" << entity->to_json().dump(2)<< "\n";
//                     jsonrpcpp::notification_ptr notification = std::dynamic_pointer_cast<jsonrpcpp::Notification>(entity);
//                     notification->method();
//                 }
//                 if(entity->is_batch())
//                 {
//                     //TODO batch processing
//                 }
//             }
//         }
//         catch(const jsonrpcpp::RequestException &e)
//         {
//             std::cerr << e.what() << "\n";
//         }
//         catch(const jsonrpcpp::RpcException &e)
//         {
//             std::cerr << e.what() << "\n";
//         }
//         catch(const jsonrpcpp::ParseErrorException &e)
//         {
//             std::cerr << e.what() << "\n";
//         }
//         catch(const std::exception &e)
//         {
//             std::cerr << e.what() << "\n";
//         }
//     }
// }

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
        parser.register_notification_callback("initialized", initialized);
        parser.register_request_callback("initialize", initialize);

        jsonrpcpp::entity_ptr response = parser.parse(outs);
        if(response->is_response())
        {
            std::string responseString = std::dynamic_pointer_cast<jsonrpcpp::Response>(response)->to_json().dump();
            std::vector<char> responseBuffer(responseString.begin(), responseString.end());
            write_(socket, responseBuffer);
        }
    }
}
