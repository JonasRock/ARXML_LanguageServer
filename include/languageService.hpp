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


class LanguageService
{
public:
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