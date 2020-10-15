
#include <iostream>
#include <string>


#include "boost/asio.hpp"
#include "boost/lexical_cast.hpp"

using namespace boost;


std::size_t read_(asio::ip::tcp::socket &socket, std::string &message)
{
    while(socket.available() < 26)
    {
        // busy waiting
    }

    /////////////////////////// Header ///////////////////////////
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

    //read_until can read over the delimiter, so that data is part of the content
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

    // For debugging
    // std::cout << "Read -- ContentSize according to Header: " << contentLength << "\n";
    // std::cout << "Content -- Contentsize according to string.size(): " << message.size();
    // std::cout<< message << "\n" << message << "\n";

    return headerLength + contentLength;
}


std::size_t write_(asio::ip::tcp::socket &socket, const std::string &message)
{
    asio::streambuf sendBuf;
    std::ostream sendStream(&sendBuf);

    sendStream << "Content-Length: ";
    sendStream << lexical_cast<std::string>(message.length());
    sendStream << "\r\n\r\n";
    sendStream << message;

    // For debugging
    // std::cout << "Sent:\n" << sendStream.rdbuf() << "\n";

    size_t sentBytes = asio::write(socket, sendBuf);
    return sentBytes;
}