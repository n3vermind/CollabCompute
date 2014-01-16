#include <iostream>
#include <string>

#include "Connection.hpp"

Connection::Connection(boost::asio::ip::tcp::socket s, Server* _server) :
    socket(std::move(s)), server(_server)
{}

void Connection::start_accept()
{
	state = AWAIT_QUERY;
	std::cout << socket.remote_endpoint().address() << " connected" << std::endl;
    read();
}

void Connection::start_out()
{
	get_peers();
	read();
}
void Connection::write(std::string write_data)
{
	std::cout << "Trying to send : " << write_data << " to : " << socket.remote_endpoint().address() << std::endl;
	auto self(shared_from_this());
	boost::asio::async_write(socket, boost::asio::buffer(write_data.c_str(),write_data.size()),
		[this, self](boost::system::error_code ec, std::size_t length)
		{
			if(!ec) 
			{
				std::cout << "Sent msg with length : " << length << std::endl;
			}
		});
}

void Connection::read()
{
    auto self(shared_from_this());
    socket.async_read_some(boost::asio::buffer(data, max_msg),
        [this, self](boost::system::error_code ec, std::size_t length)
        {
            if(!ec) {
                data[length] = 0;
                std::cout << "Read msg with length : " << length << std::endl << data << std::endl
					<< " from : " << socket.remote_endpoint().address() << std::endl;
				if(state == AWAIT_QUERY)
				{
					if(std::strcmp(data,QUERY_SEND_PEERS) == 0)
					{
						send_peers();
					}
				}
				else if(state == GET_PEERS)
				{
					if(std::strcmp(data, END_OF_PEERS))
						server->add_peer(std::string(data));
					else 
						state = AWAIT_QUERY;
				}
				read();
            }
        });
}

void Connection::send_peers()
{
	auto peers = server->get_peers();
	for(auto i = peers.begin(); i < peers.end(); i++)
	{
		write(*(i+"\n"));
	}
	write(std::string(END_OF_PEERS));
}
void Connection::get_peers()
{
	std::cout << "get_peers()\n";
	write(std::string(QUERY_SEND_PEERS));
	state = GET_PEERS;
}
