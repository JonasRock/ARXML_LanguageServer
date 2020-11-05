/**
 * @file ioHandler.hpp
 * @author Jonas Rock
 * @brief Manages socket connection, read/write and protocol header generation/stripping
 * @version 0.1
 * @date 2020-11-05
 */

#ifndef IOHANDLER_H
#define IOHANDLER_H

#include <string>
#include <stack>

#include "boost/asio.hpp"

using namespace boost;

namespace lsp
{

/**
 * @brief Manages socket connection, read/write and protocol header generation/stripping 
 * 
 */
class IOHandler
{
public:
    /**
     * @brief Construct a new IOHandler object and start the connection
     * 
     * @param address Address to connect to without port suffix, e.g. "127.0.0.1"
     * @param port Port to use for the connection
     */
    IOHandler(const std::string &address, uint32_t port);

    /**
     * @brief Add a message to be sent out on the next write, so multiple messages can be sent out on next write
     * 
     * @param message message to be sent out, without protocol header
     */
    void addMessageToSend(const std::string &message);

    /**
     * @brief read one message from the socket. Blocks execution until a message is read
     * 
     * @return std::string - message without protocol header
     */
    std::string readNextMessage();

    /**
     * @brief write all added messages to the socket in reverse order of addition (last added will be sent first)
     * 
     */
    void writeAllMessages();

private:
    /**
     * @brief blocking read of one message, stripping the protocol header
     * 
     * @param message result will be written to here
     * @return std::size_t number of bytes read
     */
    std::size_t read_(std::string &message);

    /**
     * @brief blocking write of one message, adding the protocol header
     * 
     * @param message message to be written to socket
     * @return std::size_t number of bytes sent
     */
    std::size_t write_(const std::string &message);

    std::stack<std::string> sendStack_;
    asio::io_context ioc_;
    asio::ip::tcp::endpoint endpoint_;
    asio::ip::tcp::socket socket_;
};


}

#endif /* IOHANDLER_H */