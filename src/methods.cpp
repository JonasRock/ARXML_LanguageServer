/**
 * @file methods.cpp
 * @author Jonas Rock
 * @brief Implementations of Language Server Protocol callbacks
 * @version 0.1
 * @date 2020-10-27
 * 
 * 
 */

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
                {"referencesProvider", true},
                {"definitionProvider", true},
                {"colorProvider", true}
            }}
        };
        parser = std::make_shared<xmlParser>();
        return std::make_shared<jsonrpcpp::Response>(id, result);
    }

    jsonrpcpp::response_ptr methods::request_shutdown(const jsonrpcpp::Id &id, const jsonrpcpp::Parameter &params)
    {
        configurationGlobals::receivedShutdownRequest = true;
        json result = nullptr;
        return std::make_shared<jsonrpcpp::Response>(id, result);
    }

    jsonrpcpp::response_ptr methods::request_textDocument_references(const jsonrpcpp::Id &id, const jsonrpcpp::Parameter &params)
    {
        lsp::ReferenceParams p = params.to_json().get<lsp::ReferenceParams>();
        json result;
        try
        {
            std::vector<lsp::Location> resVec = parser->getReferences(p);
            result = resVec;
        }
        catch (lsp::elementNotFoundException &e)
        {
            result = nullptr;
        }
        return std::make_shared<jsonrpcpp::Response>(id, result);
    }

    jsonrpcpp::response_ptr methods::request_textDocument_definition(const jsonrpcpp::Id &id, const jsonrpcpp::Parameter &params)
    {
        lsp::TextDocumentPositionParams p = params.to_json().get<lsp::TextDocumentPositionParams>();
        json result;
        try
        {
            lsp::LocationLink link = parser->getDefinition(p);
            result = link;
        }
        catch (lsp::elementNotFoundException &e)
        {
            result = nullptr;
        }

        return std::make_shared<jsonrpcpp::Response>(id, result);
    }
    jsonrpcpp::response_ptr methods::request_textDocument_documentColor(const jsonrpcpp::Id &id, const jsonrpcpp::Parameter &params)
    {
        lsp::DocumentColorParams p = params.to_json().get<lsp::DocumentColorParams>();
        parser->preParseFile(p.textDocument.uri);
        json result = json::array();
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