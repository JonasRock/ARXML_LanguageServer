#include <iostream>
#include <string>
#include "boost/asio.hpp"
#include <string>
#include <vector>

using namespace boost;

size_t read_(asio::ip::tcp::socket &socket, std::vector<char> &buffer)
{

    size_t availBytes = socket.available();
    buffer.resize(availBytes);

    boost::system::error_code ec;
    size_t readNum = asio::read(socket, asio::buffer(buffer), asio::transfer_exactly(availBytes), ec);

    if(ec) { std::cerr << ec.message() << std::endl; }
    
    return readNum;
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
    return 0;
}