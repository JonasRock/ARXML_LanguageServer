/**
 * @file readWrite.cpp
 * @author Jonas Rock
 * @brief Implementation of read/write
 * @version 0.1
 * @date 2020-10-27
 * 
 * 
 */

#include <iostream>
#include <string>

#include "boost/asio.hpp"
#include "boost/lexical_cast.hpp"

#include "ioHandler.hpp"

using namespace boost;

lsp::IOHandler::IOHandler(const std::string &address, uint32_t port)
    : ioc_(), endpoint_(asio::ip::address::from_string(address), port), socket_(ioc_, endpoint_.protocol())
{
    std::cout << "Connecting to " << address << ":" << port << "...\n";
    socket_.connect(endpoint_);
    std::cout << "Connection established\n";
}

void lsp::IOHandler::addMessageToSend(const std::string &message)
{
    sendStack_.emplace(message);
}

std::string lsp::IOHandler::readNextMessage()
{
    std::string ret;
    if (read_(ret))
    {
        std::cout << "Receiving Message:\n" << ret << "\n\n";
        return ret;
    }
    else
    {
        return "";
    }
}

void lsp::IOHandler::writeAllMessages()
{
    while(!sendStack_.empty())
    {
        std::string toSend = sendStack_.top();
        sendStack_.pop();
        write_(toSend);
        std::cout << "Sending Message:\n" << toSend << "\n\n";
    }
}

std::size_t lsp::IOHandler::read_(std::string &message)
{
    while(socket_.available() < 26)
    {
        // busy waiting
    }

    /////////////////////////// Header ///////////////////////////

    //read_until() and read() are incompatible, as read_until can read over the delimiter, when calling consecutive
    //read_until()s, it looks into the buffer first if the condition is already met, so it does not read from the socket in that case
    //read does not look at the buffer, so it will probably miss a majority of the content, as that is in the buffer.
    //for this reason, the streambuffer for the header is limited to 26 characters, 20 for the "Content-Lenght: " and "\r\n\r\n"
    //and 6 characters for the number, as requests should not be bigger than 999999 characters anyways.
    //This is necessary because the header is delimited by a string delimiter, while the body is delmitited by the provided length
    //read_until() for the delimiter, read() for the fixed length
    asio::streambuf headerbuf(26);
    boost::system::error_code ec;

    std::size_t headerLength = asio::read_until(socket_, headerbuf, "\r\n\r\n", ec);
    if (ec) { std::cerr << ec.message() << std::endl; }

    size_t contentLength = lexical_cast<size_t>(std::string{
        // Remove the "Content-Length: "
        asio::buffers_begin(headerbuf.data()) + 16,
        // Remove the "\r\n\r\n"
        asio::buffers_begin(headerbuf.data()) + headerLength - 4
    });

    headerbuf.consume(headerLength);

    //We didn't know how big the header was before, so we have probably had too much space in the streambuffer, some of the content might still be here
    //We can extract that now and continue reading with read, since we know exactly how much read_until() read.
    std::istream headerStream(&headerbuf);
    std::string contentInHeader;
    headerStream >> contentInHeader;

    /////////////////////////// Content //////////////////////////
    asio::streambuf contentbuf(contentLength - contentInHeader.size());

    //read the rest
    asio::read(socket_, contentbuf, asio::transfer_exactly(contentLength - contentInHeader.size()), ec);
    if (ec) { std::cerr << ec.message() << std::endl;} 

    message.reserve(contentLength);
    message = contentInHeader + std::string{
        asio::buffers_begin(contentbuf.data()),
        asio::buffers_end(contentbuf.data())
    };

    contentbuf.consume(contentLength);

    return headerLength + contentLength;
}

std::size_t lsp::IOHandler::write_(const std::string &message)
{
    asio::streambuf sendBuf;
    std::ostream sendStream(&sendBuf);

    sendStream << "Content-Length: ";
    sendStream << lexical_cast<std::string>(message.length());
    sendStream << "\r\n\r\n";
    sendStream << message;

    size_t sentBytes = asio::write(socket_, sendBuf);
    return sentBytes;
}