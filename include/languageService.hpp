/**
 * @file languageService.hpp
 * @author Jonas Rock
 * @brief Main run routine, callback definitions for lsp methods
 * @version 0.1
 * @date 2020-11-05
 */

#ifndef LANGUAGESERVICE_H
#define LANGUAGESERVICE_H

#include <string>
#include <memory>

#include "json.hpp"
#include "jsonrpcpp.hpp"

#include "ioHandler.hpp"
#include "lspParser.hpp"
#include "xmlParser.hpp"


namespace lsp
{

/**
 * @brief Main run routine, callback definitions for lsp methods
 * 
 */
class LanguageService
{
public:
    /**
     * @brief start connection, register callbacks and instantiate parsers and IOHandler, then begin main run routine
     * 
     * @param address Address to connect to without port suffix, e.g. "127.0.0.1"
     * @param port Port to use for the connectio
     */
    static void start(std::string address, uint32_t port);
private:
    static void run();
    static uint32_t getRequestID();
    static inline std::shared_ptr<lsp::IOHandler> ioHandler_ = nullptr;
    static inline std::shared_ptr<lsp::Parser> lspParser_ = nullptr;
    static inline std::shared_ptr<xmlParser> xmlParser_ = nullptr;

    //Callbacks for Language Server Protocol
    static jsonrpcpp::response_ptr request_initialize(const jsonrpcpp::Id &id, const jsonrpcpp::Parameter &params);
    static jsonrpcpp::response_ptr request_shutdown(const jsonrpcpp::Id &id, const jsonrpcpp::Parameter &params);
    static jsonrpcpp::response_ptr request_textDocument_references(const jsonrpcpp::Id &id, const jsonrpcpp::Parameter &params);
    static jsonrpcpp::response_ptr request_textDocument_definition(const jsonrpcpp::Id &id, const jsonrpcpp::Parameter &params);
    static jsonrpcpp::response_ptr request_textDocument_documentColor(const jsonrpcpp::Id &id, const jsonrpcpp::Parameter &params);
    
    static void notification_initialized(const jsonrpcpp::Parameter &params);
    static void notification_exit(const jsonrpcpp::Parameter &params);
    static void notification_special_cancelRequest(const jsonrpcpp::Parameter &params);

    static void toClient_request_workspace_configuration();
    static void response_workspace_configuration(const json &results);
};


}


#endif /* LANGUAGESERVICE_H */