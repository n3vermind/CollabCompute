#include <iostream>
#include <string>

#include "Connection.hpp"

Connection::Connection(boost::asio::ip::tcp::socket s, Server* _server) :
    socket(std::move(s)), server(_server)
{}

void Connection::start_accept()
{
	state = GET_HASH;
	std::cout << socket.remote_endpoint().address() << " connected" << std::endl;
    read();
}

void Connection::start_out()
{
	get_next();
	read();
}
void Connection::start_ask(std::string address, std::string hash)
{
	if(is_good_placement(server->get_next_hash(),server->get_hash(), hash)){
		// tell server to connect with that peer 
	} else {
		server->ask_for_next(address,hash);
	}
	read();
}
void Connection::write(std::string write_data)
{
	write_data += msg_split_char;
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
				current_msg += std::string(data);
                std::vector<std::string> split_vec;
                boost::split(split_vec,current_msg,boost::is_any_of(msg_split_char), boost::token_compress_on);
				for(int i = 0; i < split_vec.size()-1;i++) 
                {
                    if(split_vec[i].size())
                        msg_queue.push(split_vec[i]);
                }
                if(split_vec.back().size())
                    current_msg = split_vec.back();
                else current_msg = "";
                while(!msg_queue.empty())
                {
					std::cout << msg_queue.front() << " " << state << std::endl;
    				switch(state)
					{
						case AWAIT_QUERY:
							switch(msg_queue.front()[0])
							{
								case PREVIOUS:
									state = PREVIOUS;
									break;
								case LOOKING_FOR_SPOT:
									state = LOOKING_FOR_SPOT;
									break;
								case WAITING_FOR_SPOT:
									state = WAITING_FOR_SPOT;
								case GET_PEERS:
									state = GET_PEERS;
									break;
							}
							break;
						case GET_HASH: // GET_HASH only occurs in connection with our next.
							server->set_next_hash(msg_queue.front());
							state = AWAIT_QUERY;
							break;
						case PREVIOUS: //check if current endpoint fits as our previous. If not then we send broadcast in order to find proper next for him.
							if(is_good_placement(server->get_next_hash(), server->get_hash(), msg_queue.front()))
							{
								server->close_previous(this);
								write(ACCEPTED_PREVIOUS);
							} 
							else 
							{
								std::string address = socket.remote_endpoint().address().to_string();
								server->ask_for_next(address, msg_queue.front());
								end();
							}
							
							break;	
						case LOOKING_FOR_SPOT:
							break;
						case WAITING_FOR_SPOT:
							if(msg_queue.front() == ACCEPTED_PREVIOUS)
							{
								server->set_next_connection(this);
							}
							break;
						case GET_PEERS:
							break;
    				}
			    	msg_queue.pop();
                }
				read();
            }
        });
}

/*void Connection::send_peers()
{
	auto peers = server->get_peers();
	for(auto i = peers.begin(); i != peers.end(); i++)
	{
		write(*i);
	}
	write(std::string(END_OF_PEERS));
}
void Connection::get_peers()
{
	std::cout << "get_peers()\n";
	write(std::string(QUERY_SEND_PEERS));
	state = GET_PEERS;
}*/
void Connection::get_next() // ask our endpoint if he is our next. Connection will shutdown if hes not and endpoint will broadcast
{							// through network that we need a friend. 
	write(std::string(std::to_string(PREVIOUS)));
	write(server->get_hash());
	state = WAITING_FOR_SPOT;
}

void Connection::end()
{
	boost::system::error_code ec;
	socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
	socket.close();
}
bool Connection::is_good_placement(std::string a, std::string b, std::string peer)
{
	if((a < b && a <= peer && peer <= b)||
		(a > b && (a >= peer || peer >= b)))
		return true;
	return false;
}
