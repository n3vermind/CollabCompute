#include "Server.hpp"

Server::Server(boost::asio::io_service &io, short port, std::string bootstrap) :
    acceptor(io, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
    socket(io), hash(Identify::getId())
{
    if(bootstrap != "")
        std::cout << "Bootstraping to " << bootstrap << std::endl;
    accept();
}

void Server::accept()
{
    acceptor.async_accept(socket, 
        [this](boost::system::error_code ec)
        {
            if(!ec)
                std::make_shared<Connection>(std::move(socket))->start();
            accept();
        });
}
