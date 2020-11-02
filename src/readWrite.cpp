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

using namespace boost;

/**
 * @brief read from socket and remove protocol header (blocking/synchronous)
 * 
 * @param socket 
 * @param message string to write result to
 * @return std::size_t number of read bytes, including header
 */
std::size_t read_(asio::ip::tcp::socket &socket, std::string &message)
{
    while(socket.available() < 26)
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

    std::size_t headerLength = asio::read_until(socket, headerbuf, "\r\n\r\n", ec);
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
    asio::read(socket, contentbuf, asio::transfer_exactly(contentLength - contentInHeader.size()), ec);
    if (ec) { std::cerr << ec.message() << std::endl;} 

    message.reserve(contentLength);
    message = contentInHeader + std::string{
        asio::buffers_begin(contentbuf.data()),
        asio::buffers_end(contentbuf.data())
    };

    contentbuf.consume(contentLength);

    return headerLength + contentLength;
}

/**
 * @brief generate protocol header and send to socket (blocking/synchronous)
 * 
 * @param socket 
 * @param message string to send
 * @return std::size_t number of bytes sent, including header
 */
std::size_t write_(asio::ip::tcp::socket &socket, const std::string &message)
{
    asio::streambuf sendBuf;
    std::ostream sendStream(&sendBuf);

    sendStream << "Content-Length: ";
    sendStream << lexical_cast<std::string>(message.length());
    sendStream << "\r\n\r\n";
    sendStream << message;

    size_t sentBytes = asio::write(socket, sendBuf);
    return sentBytes;
}