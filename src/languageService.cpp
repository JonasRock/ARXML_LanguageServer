#include "languageService.hpp"

#include <string>
#include <iostream>

#include "types.hpp"
#include "lspExceptions.hpp"
#include "config.hpp"


void lsp::LanguageService::start(std::string address, uint32_t port)
{
    ioHandler_ = std::make_shared<lsp::IOHandler>(address, port);
    messageParser_ = std::make_shared<lsp::MessageParser>();
    xmlParser_ = std::make_shared<XmlParser>();

    //register Callbacks here
    messageParser_->register_notification_callback("initialized", lsp::LanguageService::notification_initialized);
    messageParser_->register_notification_callback("exit", lsp::LanguageService::notification_exit);
    messageParser_->register_notification_callback("$/cancelRequest", lsp::LanguageService::notification_special_cancelRequest);

    messageParser_->register_request_callback("initialize", lsp::LanguageService::request_initialize);
    messageParser_->register_request_callback("shutdown", lsp::LanguageService::request_shutdown);
    messageParser_->register_request_callback("textDocument/definition", lsp::LanguageService::request_textDocument_definition);
    messageParser_->register_request_callback("textDocument/references", lsp::LanguageService::request_textDocument_references);
    messageParser_->register_request_callback("textDocument/hover", lsp::LanguageService::request_textDocument_hover);
    messageParser_->register_request_callback("treeView/getChildren", lsp::LanguageService::request_treeView_getChildren);
    messageParser_->register_request_callback("textDocument/goToOwner", lsp::LanguageService::request_textDocument_owner);
    //begin the main run loop
    run();
}

void lsp::LanguageService::run()
{
    while(1)
    {
        try
        {
            std::string message = ioHandler_->readNextMessage();
            jsonrpcpp::entity_ptr ret = messageParser_->parse(message);
            if(lsp::config::shutdown)
            {
                //Stop the server
                break;
            }
            if(ret)
            {
                if(ret->is_response())
                {
                    ioHandler_->addMessageToSend(std::dynamic_pointer_cast<jsonrpcpp::Response>(ret)->to_json().dump());
                }
            }
            ioHandler_->writeAllMessages();
        }
        catch (lsp::badEntityException &e)
        {

        }
    }
}

uint32_t getNextRequestID()
{
    static uint32_t id = 0;
    return id++;
}

//Callback implementations

void lsp::LanguageService::notification_initialized(const jsonrpcpp::Parameter &params __attribute__((unused)))
{
    toClient_request_workspace_workspaceFolders();
    toClient_request_workspace_configuration();
}

void lsp::LanguageService::notification_exit(const jsonrpcpp::Parameter &params __attribute__((unused)))
{
    lsp::config::shutdown = true;
    //exit
}

void lsp::LanguageService::notification_special_cancelRequest(const jsonrpcpp::Parameter &params __attribute__((unused)))
{
    //do nothing
}

jsonrpcpp::response_ptr lsp::LanguageService::request_initialize(const jsonrpcpp::Id &id, const jsonrpcpp::Parameter &params __attribute__((unused)))
{
    json result = {
        {"capabilities", {
            {"referencesProvider", true},
            {"definitionProvider", true},
            {"hoverProvider", true},
            {"workspace", {
                {"workspacefolders", {
                    {"supported, true"}
                }}
            }}
        }}
    };
    return std::make_shared<jsonrpcpp::Response>(id, result);
}

jsonrpcpp::response_ptr lsp::LanguageService::request_shutdown(const jsonrpcpp::Id &id, const jsonrpcpp::Parameter &params __attribute__((unused)))
{
    json result = nullptr;
    return std::make_shared<jsonrpcpp::Response>(id, result);
}

jsonrpcpp::response_ptr lsp::LanguageService::request_textDocument_references(const jsonrpcpp::Id &id, const jsonrpcpp::Parameter &params)
{
    lsp::types::ReferenceParams p = params.to_json().get<lsp::types::ReferenceParams>();
    json result;
    try
    {
        std::vector<lsp::types::Location> resVec = xmlParser_->getReferences(p);
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
    lsp::types::TextDocumentPositionParams p = params.to_json().get<lsp::types::TextDocumentPositionParams>();
    json result;
    try
    {
        lsp::types::LocationLink link = xmlParser_->getDefinition(p);
        result = link;
    }
    catch (lsp::elementNotFoundException &e)
    {
        result = nullptr;
    }

    return std::make_shared<jsonrpcpp::Response>(id, result);
}

jsonrpcpp::response_ptr lsp::LanguageService::request_textDocument_hover(const jsonrpcpp::Id &id, const jsonrpcpp::Parameter &params)
{
    lsp::types::TextDocumentPositionParams p = params.to_json().get<lsp::types::TextDocumentPositionParams>();
    json result;
    try
    {
        result = xmlParser_->getHover(p);
        return std::make_shared<jsonrpcpp::Response>(id, result);     
    }
    catch(lsp::elementNotFoundException &e)
    {
        result = nullptr;
    }
    return std::make_shared<jsonrpcpp::Response>(id, result);
}

jsonrpcpp::response_ptr lsp::LanguageService::request_treeView_getChildren(const jsonrpcpp::Id &id, const jsonrpcpp::Parameter &params)
{
    lsp::types::non_standard::GetChildrenParams p;

    if((params.to_json().contains("path")))
        p = params.to_json().get<lsp::types::non_standard::GetChildrenParams>();
    else
    {
        p.uri = (params.to_json())["uri"];
        p.path = "";
        p.unique = (params.to_json())["unique"].get<bool>();
    }
    std::vector<lsp::types::non_standard::ShortnameTreeElement> resShortnames = xmlParser_->getChildren(p);
    json result = resShortnames;
    return std::make_shared<jsonrpcpp::Response>(id, result);
}

jsonrpcpp::response_ptr lsp::LanguageService::request_textDocument_owner(const jsonrpcpp::Id &id, const jsonrpcpp::Parameter &params)
{
    lsp::types::non_standard::OwnerParams p = params.to_json();
    try
    {
        lsp::types::Location ret = xmlParser_->getOwner(p);
        json result = ret;
        return std::make_shared<jsonrpcpp::Response>(id, result);
    }
    catch (const lsp::elementNotFoundException &e)
    {
        json result = nullptr;
        return std::make_shared<jsonrpcpp::Response>(id, result);
    }
}

void lsp::LanguageService::toClient_request_workspace_configuration()
{
    json paramsjson = lsp::types::ConfigurationParams{std::vector<lsp::types::ConfigurationItem>{{"arxmlNavigationHelper"}}};
    jsonrpcpp::Id id(getNextRequestID());
    jsonrpcpp::Request request(id, "workspace/configuration", paramsjson);
    ioHandler_->addMessageToSend(request.to_json().dump());
    messageParser_->register_response_callback(id.int_id(), response_workspace_configuration);
}

void lsp::LanguageService::response_workspace_configuration(const json &results)
{
    lsp::config::maxOpenFiles = results[0]["maxOpenFiles"].get<uint32_t>();
    lsp::config::precalculateOnOpeningFiles = results[0]["precalculateOnOpeningFiles"].get<bool>();
    lsp::config::referenceLinkToParentShortname = results[0]["referenceLinkToParentShortname"].get<bool>();
}

void lsp::LanguageService::toClient_request_workspace_workspaceFolders()
{
    json paramsjson = nullptr;
    jsonrpcpp::Id id(getNextRequestID());
    jsonrpcpp::Request request(id, "workspace/workspaceFolders", paramsjson);
    ioHandler_->addMessageToSend(request.to_json().dump());
    messageParser_->register_response_callback(id.int_id(), response_workspace_workspaceFolders);
}

void lsp::LanguageService::response_workspace_workspaceFolders(const json &results)
{
#ifndef NO_TERMINAL_OUTPUT
    std::cout << "WorkspaceFolders received:\n" << results.dump(2) << "\n\n";
#endif
    if (results != nullptr)
    {
        for (auto result : results)
        {
            lsp::types::DocumentUri fileUri = result["uri"].get<std::string>();
            xmlParser_->parseFullFolder(fileUri);
        }
    }
    json params = {{"event", "treeViewReady"}};
    toClient_notification_telemetry_event(params);
}

void lsp::LanguageService::toClient_notification_telemetry_event(const json &params)
{
    jsonrpcpp::Notification notification("telemetry/event", params);
    ioHandler_->addMessageToSend(notification.to_json().dump());
}