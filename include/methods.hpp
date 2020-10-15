#ifndef METHODS_H
#define METHODS_H

#include "jsonrpcpp.hpp"

namespace requests
{
    jsonrpcpp::response_ptr initialize(const jsonrpcpp::Id &id,const jsonrpcpp::Parameter &params);
}

namespace notifications
{
    void initialized(const jsonrpcpp::Parameter &params);
}

#endif /* METHODS_H */
