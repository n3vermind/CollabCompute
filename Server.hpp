#include <string>
#include <iostream>
#include <boost/asio.hpp>

#include "Connection.hpp"
#include "Identify.hpp"

class Server
{
    public:
        Server(boost::asio::io_service &io, short port, std::string bootstrap = "");
        
    private:
        void accept();

        std::string hash;
        boost::asio::ip::tcp::acceptor acceptor;
        boost::asio::ip::tcp::socket socket;
};
