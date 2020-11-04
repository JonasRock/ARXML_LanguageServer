#ifndef LSPPARSER_H
#define LSPPARSER_H

#include <functional>
#include <map>

#include "json.hpp"
#include "jsonrpcpp.hpp"

using namespace nlohmann;

namespace lsp
{



class Parser: public jsonrpcpp::Parser
{
typedef std::function<void(const json &results)> response_callback;

public:
    jsonrpcpp::entity_ptr parse(const std::string &json_str);
    void register_response_callback(const uint32_t id, response_callback callback);
    void register_notification_callback(const std::string &notification, jsonrpcpp::notification_callback callback);
    void register_request_callback(const std::string &request, jsonrpcpp::request_callback callback);

private:
    std::map<uint32_t, response_callback> response_callbacks_;
    std::map<std::string, jsonrpcpp::notification_callback> notification_callbacks_;
    std::map<std::string, jsonrpcpp::request_callback> request_callbacks_;
};


}

#endif /* LSPPARSER_H */