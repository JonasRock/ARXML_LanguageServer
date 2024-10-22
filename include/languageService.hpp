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

#include "types.hpp"
#include "ioHandler.hpp"
#include "messageParser.hpp"
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
    static inline std::shared_ptr<lsp::MessageParser> messageParser_ = nullptr;
    static inline std::shared_ptr<XmlParser> xmlParser_ = nullptr;

    //Callbacks for Language Server Protocol

    static jsonrpcpp::response_ptr request_textDocument_hover(const jsonrpcpp::Id &id, const jsonrpcpp::Parameter &params);
    static jsonrpcpp::response_ptr request_initialize(const jsonrpcpp::Id &id, const jsonrpcpp::Parameter &params);
    static jsonrpcpp::response_ptr request_shutdown(const jsonrpcpp::Id &id, const jsonrpcpp::Parameter &params);
    static jsonrpcpp::response_ptr request_textDocument_references(const jsonrpcpp::Id &id, const jsonrpcpp::Parameter &params);
    static jsonrpcpp::response_ptr request_textDocument_definition(const jsonrpcpp::Id &id, const jsonrpcpp::Parameter &params);
    static jsonrpcpp::response_ptr request_textDocument_owner(const jsonrpcpp::Id &id, const jsonrpcpp::Parameter &params);
    static jsonrpcpp::response_ptr request_treeView_getChildren(const jsonrpcpp::Id &id, const jsonrpcpp::Parameter &params);
    static jsonrpcpp::response_ptr request_treeView_getParentElement(const jsonrpcpp::Id &id, const jsonrpcpp::Parameter &params);
    static jsonrpcpp::response_ptr request_treeView_getNearestShortname(const jsonrpcpp::Id &id, const jsonrpcpp::Parameter &params);
    
    static void notification_initialized(const jsonrpcpp::Parameter &params);
    static void notification_exit(const jsonrpcpp::Parameter &params);
    static void notification_workspace_didChangeConfiguration(const jsonrpcpp::Parameter &params);

    static void toClient_request_workspace_configuration();
    static void toClient_request_workspace_workspaceFolders();
    static void toClient_request_client_registerCapability(const std::string method);

    static void toClient_notification_telemetry_event(const json &params);
    static void toClient_notification_telemetry_event_error(const lsp::types::arxmlError error);

    static void response_workspace_configuration(const json &results);
    static void response_workspace_workspaceFolders(const json &results);
    static void response_void(const json &results);
};


}


#endif /* LANGUAGESERVICE_H */