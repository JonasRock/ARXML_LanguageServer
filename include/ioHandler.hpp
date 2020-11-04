#ifndef IOHANDLER_H
#define IOHANDLER_H

#include <string>
#include <stack>

#include "boost/asio.hpp"

using namespace boost;

namespace lsp
{


class IOHandler
{
public:
    IOHandler(const std::string &address, uint32_t port);
    void addMessageToSend(const std::string &message);
    std::string readNextMessage();
    void writeAllMessages();
private:
    std::size_t read_(std::string &message);
    std::size_t write_(const std::string &message);

    std::stack<std::string> sendStack_;
    asio::io_context ioc_;
    asio::ip::tcp::endpoint endpoint_;
    asio::ip::tcp::socket socket_;
};


}

#endif /* IOHANDLER_H */