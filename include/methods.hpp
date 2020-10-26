#ifndef METHODS_H
#define METHODS_H

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
    static jsonrpcpp::response_ptr request_textDocument_hover(const jsonrpcpp::Id &id, const jsonrpcpp::Parameter &params);
    static jsonrpcpp::response_ptr request_textDocument_definition(const jsonrpcpp::Id &id, const jsonrpcpp::Parameter &params);
    static void notification_initialized(const jsonrpcpp::Parameter &params);
    static void notification_exit(const jsonrpcpp::Parameter &params);
private:
    inline static std::map<std::string, std::shared_ptr<xmlParser>> parsers;
    static std::shared_ptr<xmlParser> prepareParser(const lsp::DocumentUri);
};

#endif /* METHODS_H */
