#include <iostream>
#include <string>

#include "Connection.hpp"

Connection::Connection(boost::asio::ip::tcp::socket s) :
    socket(std::move(s))
{}

void Connection::start()
{
    auto self(shared_from_this());
    socket.async_read_some(boost::asio::buffer(data, max_msg),
        [this, self](boost::system::error_code ec, std::size_t length)
        {
            if(!ec) {
                data[length] = 0;

            }
        });

    read();
}

void Connection::read()
{
    auto self(shared_from_this());
    socket.async_read_some(boost::asio::buffer(data, max_msg),
        [this, self](boost::system::error_code ec, std::size_t length)
        {
            if(!ec) {
                data[length] = 0;
                std::cout << "Read msg with length : " << length << std::endl << data << std::endl;
                read();
            }
        });
}
