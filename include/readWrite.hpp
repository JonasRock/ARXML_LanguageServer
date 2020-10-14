#ifndef READWRITE_H
#define READWRITE_H

#include <iostream>
#include <string>
#include <vector>

#include "boost/asio.hpp"
#include "boost/lexical_cast.hpp"


using namespace boost;

//Todo: ugly and probably unsafe
std::size_t read_(asio::ip::tcp::socket &socket, std::vector<char> &buffer)
{
    //Read and throw away the first 16 byte
    std::vector<char> throwAway;
    throwAway.resize(16);
    std::size_t readNum = asio::read(socket, asio::buffer(throwAway), asio::transfer_exactly(16));

    //Read the number
    std::vector<char> messageSize;
    messageSize.resize(10);
        boost::system::error_code ec;
    std::size_t numberSize = asio::read_until(socket, asio::dynamic_buffer(messageSize), static_cast<char>('\r'));
    readNum += numberSize;
    //now messageSize also contains the \r delemiter, so we resize to that and delete it
    messageSize.resize(numberSize);
    messageSize.pop_back();
    size_t toRead = boost::lexical_cast<size_t>(numberSize);

    //delete the "\n\r\n" thats left in the socket stream
    readNum += asio::read(socket, asio::buffer(throwAway), asio::transfer_exactly(3));

    //read the actual data
    std::size_t availBytes = socket.available();
    if(availBytes >= toRead)
    {
        buffer.resize(toRead);
        readNum += asio::read(socket, asio::buffer(buffer), asio::transfer_exactly(availBytes), ec);
        if(ec) { std::cerr << ec.message() << std::endl; }
    }
    else{
        buffer.resize(0);
    }

    std::cout.write(reinterpret_cast<char*>(&buffer[0]), buffer.size());

    return readNum;
}

std::size_t write_(asio::ip::tcp::socket &socket, std::vector<char> &buffer)
{
    //We need a header in form "Content-Length: x\r\n\r\n" according to Language Server Protocol Specification

    const std::vector<char> headerPrefix = {'C','o','n','t','e','n','t','-','L','e','n','g','t','h',':',' '};
    std::string headerNumberStr = lexical_cast<std::string>(buffer.size());
    std::vector<char> headerNumber(headerNumberStr.begin(), headerNumberStr.end());
    const std::vector<char> headerSuffix= {'\r','\n','\r','\n'};

     size_t sentBytes = asio::write(socket, asio::buffer(headerPrefix));
    sentBytes += asio::write(socket, asio::buffer(headerNumber));
    sentBytes += asio::write(socket, asio::buffer(headerSuffix));
    sentBytes += asio::write(socket, asio::buffer(buffer));

    std::cout.write(reinterpret_cast<const char*>(&headerPrefix[0]), headerPrefix.size());
    std::cout.write(reinterpret_cast<char*>(&headerNumber[0]), headerNumber.size());
    std::cout.write(reinterpret_cast<const char*>(&headerSuffix[0]), headerSuffix.size());
    std::cout.write(reinterpret_cast<char*>(&buffer[0]), buffer.size());

    return sentBytes;
}

#endif // READWRITE_H