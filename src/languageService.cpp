#include <string>
#include <iostream>

#include "types.hpp"
#include "lspExceptions.hpp"
#include "languageService.hpp"
#include "config.hpp"

void lsp::LanguageService::start(std::string address, uint32_t port)
{
    ioHandler_ = std::make_shared<lsp::IOHandler>(address, port);
    lspParser_ = std::make_shared<lsp::Parser>();
    xmlParser_ = std::make_shared<xmlParser>();

    //register Callbacks here
    lspParser_->register_notification_callback("initialized", lsp::LanguageService::notification_initialized);
    lspParser_->register_notification_callback("exit", lsp::LanguageService::notification_exit);
    lspParser_->register_notification_callback("$/cancelRequest", lsp::LanguageService::notification_special_cancelRequest);

    lspParser_->register_request_callback("initialize", lsp::LanguageService::request_initialize);
    lspParser_->register_request_callback("shutdown", lsp::LanguageService::request_shutdown);
    lspParser_->register_request_callback("textDocument/definition", lsp::LanguageService::request_textDocument_definition);
    lspParser_->register_request_callback("textDocument/references", lsp::LanguageService::request_textDocument_references);
    lspParser_->register_request_callback("textDocument/documentColor", lsp::LanguageService::request_textDocument_documentColor);
    //begin the main run loop
    run();
}

void lsp::LanguageService::run()
{
    while(1)
    {
        std::string message = ioHandler_->readNextMessage();
        jsonrpcpp::entity_ptr ret = lspParser_->parse(message);
        if(ret)
        {
            if(ret->is_response())
            {
                ioHandler_->addMessageToSend(std::dynamic_pointer_cast<jsonrpcpp::Response>(ret)->to_json().dump());
            }
        }
        ioHandler_->writeAllMessages();
    }
}

uint32_t getNextRequestID()
{
    static uint32_t id = 0;
    return id++;
}

//Callback implementations

void lsp::LanguageService::notification_initialized(const jsonrpcpp::Parameter &params)
{
    toClient_request_workspace_configuration();
}

void lsp::LanguageService::notification_exit(const jsonrpcpp::Parameter &params)
{
    //exit
}

void lsp::LanguageService::notification_special_cancelRequest(const jsonrpcpp::Parameter &params)
{
    //do nothing
}

jsonrpcpp::response_ptr lsp::LanguageService::request_initialize(const jsonrpcpp::Id &id, const jsonrpcpp::Parameter &params)
{
    json result = {
        {"capabilities", {
            {"referencesProvider", true},
            {"definitionProvider", true},
            {"colorProvider", true}
        }}
    };
    return std::make_shared<jsonrpcpp::Response>(id, result);
}

jsonrpcpp::response_ptr lsp::LanguageService::request_shutdown(const jsonrpcpp::Id &id, const jsonrpcpp::Parameter &params)
{
    json result = nullptr;
    return std::make_shared<jsonrpcpp::Response>(id, result);
}

jsonrpcpp::response_ptr lsp::LanguageService::request_textDocument_references(const jsonrpcpp::Id &id, const jsonrpcpp::Parameter &params)
{
    lsp::ReferenceParams p = params.to_json().get<lsp::ReferenceParams>();
    json result;
    try
    {
        std::vector<lsp::Location> resVec = xmlParser_->getReferences(p);
        result = resVec;
    }
    catch (lsp::elementNotFoundException &e)
    {
        result = nullptr;
    }
    return std::make_shared<jsonrpcpp::Response>(id, result);
}

jsonrpcpp::response_ptr lsp::LanguageService::request_textDocument_definition(const jsonrpcpp::Id &id, const jsonrpcpp::Parameter &params)
{
        lsp::TextDocumentPositionParams p = params.to_json().get<lsp::TextDocumentPositionParams>();
    json result;
    try
    {
        lsp::LocationLink link = xmlParser_->getDefinition(p);
        result = link;
    }
    catch (lsp::elementNotFoundException &e)
    {
        result = nullptr;
    }

    return std::make_shared<jsonrpcpp::Response>(id, result);
}

jsonrpcpp::response_ptr lsp::LanguageService::request_textDocument_documentColor(const jsonrpcpp::Id &id, const jsonrpcpp::Parameter &params)
{
    lsp::DocumentColorParams p = params.to_json().get<lsp::DocumentColorParams>();
    if(lsp::config::precalculateOnOpeningFiles)
    {
        xmlParser_->preParseFile(p.textDocument.uri);
    }
    json result = json::array();
    return std::make_shared<jsonrpcpp::Response>(id, result);
}

void lsp::LanguageService::toClient_request_workspace_configuration()
{
    json paramsjson = lsp::ConfigurationParams{std::vector<lsp::ConfigurationItem>{{"arxmlNavigationHelper"}}};
    jsonrpcpp::Id id(getNextRequestID());
    jsonrpcpp::Request request(id, "workspace/configuration", paramsjson);
    ioHandler_->addMessageToSend(request.to_json().dump());
    lspParser_->register_response_callback(id.int_id(), response_workspace_configuration);
}

void lsp::LanguageService::response_workspace_configuration(const json &results)
{
    std::cout << "Configuration received:\n" << results.dump(2) << "\n\n";
    lsp::config::maxOpenFiles = results[0]["maxOpenFiles"].get<uint32_t>();
    lsp::config::precalculateOnOpeningFiles = results[0]["precalculateOnOpeningFiles"].get<bool>();
}