#include <iostream>

#include "json.hpp"
#include "jsonrpcpp.hpp"

using namespace nlohmann;

namespace requests
{
    jsonrpcpp::response_ptr initialize(const jsonrpcpp::Id &id, const jsonrpcpp::Parameter params)
    {
        json result = {
            "capabilities",
            {
                {
                    "declarationProvider", true
                },
                {
                    "referencesProvider", true
                }
            },
            "serverInfo",
            {
                {
                    "name", "ARXML_LanguageServer"
                },
                {
                    "version", "0.0.1"
                }
            }
        };

        return std::make_shared<jsonrpcpp::Response>(id, result);
    }
}
namespace notifications
{
    void initialized(jsonrpcpp::notification_ptr notification)
    {
        std::cout << "Received \"initialized\" notification\n";
    }
}