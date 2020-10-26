#include <iostream>
#include <sstream>
#include <string>
#include <memory>
#include <map>

#include "json.hpp"
#include "jsonrpcpp.hpp"

#include "types.hpp"
#include "xmlParser.hpp"
#include "methods.hpp"
#include "configurationGlobals.h"
#include "lspExceptions.hpp"


using namespace nlohmann;
    
    jsonrpcpp::response_ptr methods::request_initialize(const jsonrpcpp::Id &id, const jsonrpcpp::Parameter &params)
    {
        json result = {
            {"capabilities", {
                {"hoverProvider", true},
                {"definitionProvider", true},
            }}
        };
        return std::make_shared<jsonrpcpp::Response>(id, result);
    }

    jsonrpcpp::response_ptr methods::request_shutdown(const jsonrpcpp::Id &id, const jsonrpcpp::Parameter &params)
    {
        configurationGlobals::receivedShutdownRequest = true;
        json result = nullptr;
        return std::make_shared<jsonrpcpp::Response>(id, result);
    }

    jsonrpcpp::response_ptr methods::request_textDocument_hover(const jsonrpcpp::Id &id, const jsonrpcpp::Parameter &params)
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

    jsonrpcpp::response_ptr methods::request_textDocument_definition(const jsonrpcpp::Id &id, const jsonrpcpp::Parameter &params)
    {
        lsp::TextDocumentPositionParams p = params.to_json().get<lsp::TextDocumentPositionParams>();
        json result;
        try
        {
            lsp::LocationLink link = prepareParser(p.textDocument.uri)->getDefinition(p);
            result = link;
        }
        catch (lsp::elementNotFoundException &e)
        {
            result = nullptr;
        }

        return std::make_shared<jsonrpcpp::Response>(id, result);
    }

    void methods::notification_initialized(const jsonrpcpp::Parameter &params)
    {
    }

    void methods::notification_exit(const jsonrpcpp::Parameter &params)
    {
        if(configurationGlobals::receivedShutdownRequest)
            configurationGlobals::shutdownReady = true;
    }

    std::shared_ptr<xmlParser> methods::prepareParser(const lsp::DocumentUri uri)
    {
        std::string docString = std::string(uri.begin() + 5, uri.end());
        docString.replace(docString.find("%3A"), 3, ":");

        auto it = parsers.find(docString);
        if(it == parsers.end())
        {
            auto ret = parsers.emplace(std::make_pair(docString, std::make_shared<xmlParser>(docString)));
            ret.first->second->parse();
            return ret.first->second;
            
        }

        return it->second;
    }