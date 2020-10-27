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
    
    /**
     * @brief for initialize request
     * 
     * @param id 
     * @param params 
     * @return jsonrpcpp::response_ptr 
     */
    jsonrpcpp::response_ptr methods::request_initialize(const jsonrpcpp::Id &id, const jsonrpcpp::Parameter &params)
    {
        json result = {
            {"capabilities", {
                {"referencesProvider", true},
                {"definitionProvider", true},
            }}
        };
        return std::make_shared<jsonrpcpp::Response>(id, result);
    }

    /**
     * @brief for shutdown request
     * 
     * @param id 
     * @param params 
     * @return jsonrpcpp::response_ptr 
     */
    jsonrpcpp::response_ptr methods::request_shutdown(const jsonrpcpp::Id &id, const jsonrpcpp::Parameter &params)
    {
        configurationGlobals::receivedShutdownRequest = true;
        json result = nullptr;
        return std::make_shared<jsonrpcpp::Response>(id, result);
    }

    /**
     * @brief for textDocument/references request
     * 
     * @param id 
     * @param params 
     * @return jsonrpcpp::response_ptr 
     */
    jsonrpcpp::response_ptr methods::request_textDocument_references(const jsonrpcpp::Id &id, const jsonrpcpp::Parameter &params)
    {
        lsp::ReferenceParams p = params.to_json().get<lsp::ReferenceParams>();
        json result;
        try
        {
            std::vector<lsp::Location> resVec = prepareParser(p.textDocument.uri)->getReferences(p);
            result = resVec;
        }
        catch (lsp::elementNotFoundException &e)
        {
            result = nullptr;
        }
        return std::make_shared<jsonrpcpp::Response>(id, result);
    }

    /**
     * @brief for textDocument/definition requests
     * 
     * @param id 
     * @param params 
     * @return jsonrpcpp::response_ptr 
     */
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

    /**
     * @brief for initialized notification
     * 
     * @param params 
     */
    void methods::notification_initialized(const jsonrpcpp::Parameter &params)
    {
    }

    /**
     * @brief for exit notification
     * 
     * @param params 
     */
    void methods::notification_exit(const jsonrpcpp::Parameter &params)
    {
        if(configurationGlobals::receivedShutdownRequest)
            configurationGlobals::shutdownReady = true;
    }

    /**
     * @brief use to prepare parser for data extraction and get correct parser for file
     * 
     * @param uri URI of the file thats queried
     * @return std::shared_ptr<xmlParser> correct parser for file, guaranteed to have parsed the file
     */
    std::shared_ptr<xmlParser> methods::prepareParser(const lsp::DocumentUri uri)
    {
        std::string docString = std::string(uri.begin() + 5, uri.end());
        auto repBegin = docString.find("%3A");
        docString.replace(repBegin, 3, ":");
        std::string sanitizedDocString = docString.substr(repBegin-1);

        auto it = parsers.find(sanitizedDocString);
        if(it == parsers.end())
        {
            auto ret = parsers.emplace(std::make_pair(sanitizedDocString, std::make_shared<xmlParser>(sanitizedDocString)));
            ret.first->second->parse();
            return ret.first->second;
        }

        return it->second;
    }