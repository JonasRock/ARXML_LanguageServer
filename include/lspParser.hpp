/**
 * @file lspParser.hpp
 * @author Jonas Rock
 * @brief Manages message parsing and callback assignment/execution
 * @version 0.1
 * @date 2020-11-05
 */

#ifndef LSPPARSER_H
#define LSPPARSER_H

#include <functional>
#include <map>

#include "json.hpp"
#include "jsonrpcpp.hpp"

using namespace nlohmann;

namespace lsp
{


/**
 * @brief Manages message parsing and callback assignment/execution
 * 
 */
class Parser: public jsonrpcpp::Parser
{
typedef std::function<void(const json &results)> response_callback;

public:
    /**
     * @brief Parse message and execute assigned callback, if applicable
     * 
     * @param json_str message to be parsed
     * @return jsonrpcpp::entity_ptr result of callback or nullptr
     */
    jsonrpcpp::entity_ptr parse(const std::string &json_str);

    /**
     * @brief Register callback for response from client to request with given ID.
     * 
     * Use this when requesting something *from the client* by registering a callback that uses the returned results.
     * Since the id is not assigned to a request type but to a particular request, register exactly one callback to exactly one request.
     * The callback assignment is removed on execution.
     * 
     * This can be used for example in an notification callback. After getting notified of a setting change, we can send a request to the client
     * and register a function to update the program using the returned settings
     *  
     * @param id ID used in the corresponding request sent to the client
     * @param callback function to execute with returned results
     */
    void register_response_callback(const uint32_t id, response_callback callback);

    /**
     * @brief Register a callback to a notification from the client
     * 
     * @param notification The method string on which the callback should execute
     * @param callback function to execute with message parameters
     */
    void register_notification_callback(const std::string &notification, jsonrpcpp::notification_callback callback);

    /**
     * @brief REgister a callback to a request from the client
     * 
     * @param request The method string on which the callback should execute
     * @param callback function to execute with message paramters. Should return the result to the corresponding request
     */
    void register_request_callback(const std::string &request, jsonrpcpp::request_callback callback);

private:
    std::map<uint32_t, response_callback> response_callbacks_;
    std::map<std::string, jsonrpcpp::notification_callback> notification_callbacks_;
    std::map<std::string, jsonrpcpp::request_callback> request_callbacks_;
};


}

#endif /* LSPPARSER_H */