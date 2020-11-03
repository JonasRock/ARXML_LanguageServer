#ifndef METHODS_H
#define METHODS_H

/**
 * @file methods.hpp
 * @author Jonas Rock
 * @brief All callback declarations for Language Server Protocol notifications/requests go here
 * @version 0.1
 * @date 2020-10-27
 * 
 * 
 */

#include <memory>
#include <map>
#include <utility>

#include "jsonrpcpp.hpp"
#include "xmlParser.hpp"
#include "types.hpp"


class methods
{
public:
    static jsonrpcpp::response_ptr request_initialize(const jsonrpcpp::Id &id, const jsonrpcpp::Parameter &params);
    static jsonrpcpp::response_ptr request_shutdown(const jsonrpcpp::Id &id, const jsonrpcpp::Parameter &params);
    static jsonrpcpp::response_ptr request_textDocument_references(const jsonrpcpp::Id &id, const jsonrpcpp::Parameter &params);
    static jsonrpcpp::response_ptr request_textDocument_definition(const jsonrpcpp::Id &id, const jsonrpcpp::Parameter &params);
    //We only use documentColor to see when VSCode opens a file so we can begin parsing it without needing to wait for the user to request something first
    static jsonrpcpp::response_ptr request_textDocument_documentColor(const jsonrpcpp::Id &id, const jsonrpcpp::Parameter &params);
    static void notification_initialized(const jsonrpcpp::Parameter &params);
    static void notification_exit(const jsonrpcpp::Parameter &params);
private:
    inline static std::shared_ptr<xmlParser> parser = nullptr;
};

#endif /* METHODS_H */
