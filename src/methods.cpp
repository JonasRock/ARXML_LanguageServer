#include <iostream>
#include <sstream>
#include <string>

#include "json.hpp"
#include "jsonrpcpp.hpp"

#include "configurationGlobals.h"


using namespace nlohmann;

namespace requests
{
    jsonrpcpp::response_ptr initialize(const jsonrpcpp::Id &id, const jsonrpcpp::Parameter &params)
    {
        json result = {
            { "capabilities", {
                {"hoverProvider", true},
                {"referenceProvider", true}
            }}
        };
        return std::make_shared<jsonrpcpp::Response>(id, result);
    }

    jsonrpcpp::response_ptr shutdown(const jsonrpcpp::Id &id, const jsonrpcpp::Parameter &params)
    {
        configurationGlobals::receivedShutdownRequest = true;
        json result = nullptr;
        return std::make_shared<jsonrpcpp::Response>(id, result);
    }

    namespace textDocument
    {
        jsonrpcpp::response_ptr hover(const jsonrpcpp::Id &id, const jsonrpcpp::Parameter &params)
        {
            std::stringstream hoverResult;
            json paramsjson = params.to_json();
            hoverResult << "Hovering at line: " << paramsjson["position"]["line"];
            hoverResult << " and character: " << paramsjson["position"]["character"] << "\n";
            json result = {
                {"contents", hoverResult.str()}
            };

            return std::make_shared<jsonrpcpp::Response>(id, result);
        }
    }
}

namespace notifications
{
    void initialized(const jsonrpcpp::Parameter &params)
    {
    }
    void exit(const jsonrpcpp::Parameter &params)
    {
        if(configurationGlobals::receivedShutdownRequest)
            configurationGlobals::shutdownReady = true;
    }
}