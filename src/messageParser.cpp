#include "messageParser.hpp"
#include "lspExceptions.hpp"

void lsp::MessageParser::register_response_callback(const uint32_t id, response_callback callback)
{
    if(callback)
        response_callbacks_[id] = callback;
}

void lsp::MessageParser::register_notification_callback(const std::string &notification, jsonrpcpp::notification_callback callback)
{
    if(callback)
        notification_callbacks_[notification] = callback;
}

void lsp::MessageParser::register_request_callback(const std::string &request, jsonrpcpp::request_callback callback)
{
    if(callback)
        request_callbacks_[request] = callback;
}

jsonrpcpp::entity_ptr lsp::MessageParser::parse(const std::string &json_str)
{
    jsonrpcpp::entity_ptr entity = do_parse(json_str);
    if (entity && entity->is_notification())
    {
        jsonrpcpp::notification_ptr notification = std::dynamic_pointer_cast<jsonrpcpp::Notification>(entity);
        if (notification_callbacks_.find(notification->method()) != notification_callbacks_.end())
        {
            jsonrpcpp::notification_callback callback = notification_callbacks_[notification->method()];
            if (callback)
            {
                callback(notification->params());
            }
            return nullptr;
        }
    }
    else if (entity && entity->is_request())
    {
        jsonrpcpp::request_ptr request = std::dynamic_pointer_cast<jsonrpcpp::Request>(entity);
        if (request_callbacks_.find(request->method()) != request_callbacks_.end())
        {
            jsonrpcpp::request_callback callback = request_callbacks_[request->method()];
            if (callback)
            {
                jsonrpcpp::response_ptr response = callback(request->id(), request->params());
                if (response)
                    return response;
            }
            else
            {
                jsonrpcpp::Error err("MethodNotFound", -32601);
                jsonrpcpp::Response(request->id(), err);
            }
            
        }
    }
    else if (entity && entity->is_response())
    {
        jsonrpcpp::response_ptr response = std::dynamic_pointer_cast<jsonrpcpp::Response>(entity);
        if (response_callbacks_.find(response->id().int_id()) != response_callbacks_.end())
        {
            response_callback callback = response_callbacks_[response->id().int_id()];
            if (callback)
            {
                //Remove the callback for this id
                response_callbacks_.erase(response->id().int_id());
                callback(response->result());
                return nullptr;
            }
        }
    }
    throw lsp::badEntityException();
    return nullptr;
}