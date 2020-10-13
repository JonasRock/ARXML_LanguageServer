#include <iostream>
#include <string>
#include <vector>

#include "boost/asio.hpp"
#include "boost/lexical_cast.hpp"


using namespace boost;


//Todo: split into header and content
size_t read_(asio::ip::tcp::socket &socket, std::vector<char> &buffer)
{

    size_t availBytes = socket.available();
    buffer.resize(availBytes);

    boost::system::error_code ec;
    size_t readNum = asio::read(socket, asio::buffer(buffer), asio::transfer_exactly(availBytes), ec);

    if(ec) { std::cerr << ec.message() << std::endl; }
    
    return readNum;
}

//all yeahhhh, ugly
size_t write_(asio::ip::tcp::socket &socket, std::vector<char> &buffer)
{
    //We need a header in form "Content-Length: x\r\n\r\n" according to Language Server Protocol Specification

    std::vector<char> headerPrefix = {'C','o','n','t','e','n','t','-','L','e','n','g','t','h',':',' '};
    std::vector<char> headerNumber = lexical_cast<std::vector<char>>(buffer.size());
    std::vector<char> headerSuffix= {'\r','\n','\r','\n'};

    size_t sentBytes = asio::write(socket, asio::buffer(headerPrefix));
    sentBytes += asio::write(socket, asio::buffer(headerNumber));
    sentBytes += asio::write(socket, asio::buffer(headerSuffix));
    sentBytes += asio::write(socket, asio::buffer(buffer));
    std::cout.write(reinterpret_cast<char*>(&headerPrefix[0]), headerPrefix.size());
    std::cout.write(reinterpret_cast<char*>(&headerNumber[0]), headerNumber.size());
    std::cout.write(reinterpret_cast<char*>(&headerSuffix[0]), headerSuffix.size());
    std::cout.write(reinterpret_cast<char*>(&buffer[0]), buffer.size());
    return sentBytes;
}



int main()
{
    asio::io_service ios;
    asio::ip::tcp::endpoint endPoint(asio::ip::address::from_string("127.0.0.1"), 12730);
    asio::ip::tcp::socket socket(ios, endPoint.protocol());

    socket.connect(endPoint);

    std::vector<char> out;
    size_t numRead = read_(socket, out);
    std::string outs(out.begin(), out.end());
    std::cout << numRead << ": " << outs << std::endl;
    write_(socket, out);
    return 0;
}