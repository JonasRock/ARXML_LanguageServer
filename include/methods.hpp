#ifndef METHODS_H
#define METHODS_H

#include "json.hpp"
#include "jsonrpcpp.hpp"

using namespace nlohmann;

namespace requests
{
    jsonrpcpp::response_ptr initialize(const jsonrpcpp::Id &id,const jsonrpcpp::Parameter &params);
}

namespace notifications
{
    void initialized(const jsonrpcpp::Parameter &params);
}

#endif /* METHODS_H */
