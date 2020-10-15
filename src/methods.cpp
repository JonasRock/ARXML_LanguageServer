#include <iostream>
#include <sstream>
#include <string>

#include "json.hpp"
#include "jsonrpcpp.hpp"


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

    namespace textDocument
    {
        jsonrpcpp::response_ptr hover(const jsonrpcpp::Id &id, const jsonrpcpp::Parameter &params)
        {
            std::stringstream hoverResult;
            json paramsjson = params.to_json();
            hoverResult << "Hovering at line: " << paramsjson["textDocument"]["position"]["line"];
            hoverResult << " and character: " << paramsjson["textDocument"]["position"]["character"];

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
}