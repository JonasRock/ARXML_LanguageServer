#ifndef READWRITE_H
#define READWRITE_H

/**
 * @file readWrite.hpp
 * @author Jonas Rock
 * @brief declarations of read/write functionality
 * @version 0.1
 * @date 2020-10-27
 * 
 * 
 */

#include <string>
#include "boost/asio.hpp"

using namespace boost;

std::size_t read_(asio::ip::tcp::socket &socket, std::string &message);
std::size_t write_(asio::ip::tcp::socket &socket, const std::string &message);

#endif // READWRITE_H