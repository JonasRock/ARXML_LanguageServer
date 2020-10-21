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
            {"capabilities", {
                {"hoverProvider", true},
                {"definitionProvider", true},
                {"declarationProvider", true},
                {"implementationProvider", true}
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
            json paramsjson = params.to_json();
            std::stringstream hoverResult;
            hoverResult << "Hovering at line: " << paramsjson["position"]["line"];
            hoverResult << " and character: " << paramsjson["position"]["character"] << "\n";
            json result = {
                {"contents", hoverResult.str()}
            };

            return std::make_shared<jsonrpcpp::Response>(id, result);
        }

        jsonrpcpp::response_ptr definition(const jsonrpcpp::Id &id, const jsonrpcpp::Parameter &params)
        {
            json paramsjson = params.to_json();
            uint32_t line = paramsjson["position"]["line"];
            uint32_t character = paramsjson["position"]["character"];
            json result = {
                {"uri", "file:///c%3a/Users%/jr83522/Desktop/E3_1_2_Premium_V12.04.20A_AR430_20201011_Airbag_6D_BP.arxml" },
                {"range", {
                    {"start", {
                        {"line", line - 2},
                        {"character", character - 2}
                    }},
                    {"end", {
                        {"line", line - 2},
                        {"character", character + 2}
                    }}
                }}
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