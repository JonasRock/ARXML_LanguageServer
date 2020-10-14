#ifndef METHODS_H
#define METHODS_H

#include "json.hpp"
#include "jsonrpcpp.hpp"

using namespace nlohmann;

//Requests
jsonrpcpp::response_ptr initialize(const jsonrpcpp::Id &id,const jsonrpcpp::Parameter &params);

//Notifications
void initialized(const jsonrpcpp::Parameter &params);

#endif /* METHODS_H */
