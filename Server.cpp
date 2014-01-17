#include "Server.hpp"

Server::Server(boost::asio::io_service &io, short port, std::string bootstrap) :
    acceptor(io, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
    socket(io), hash(Identify::getId()), resolver(io), port(port)
{
	get_known_peers();
    if(bootstrap != "")
        std::cout << "Bootstraping to " << bootstrap << std::endl;
	auto current = get_peers();
	for(auto i = current.begin(); i != current.end(); i++)
	{
		auto endpoint = resolver.resolve({ *i, std::to_string(port)});
		boost::asio::async_connect(socket,endpoint,
				[this](boost::system::error_code ec, boost::asio::ip::tcp::resolver::iterator)
				{
					if(!ec)
					{
						std::make_shared<Connection>(std::move(socket), this)->start_out();
					}
				});
	}
	accept();
}

void Server::accept()
{
    acceptor.async_accept(socket, 
        [this](boost::system::error_code ec)
        {
            if(!ec)
                std::make_shared<Connection>(std::move(socket), this)->start_accept();
            accept();
        });
}

void Server::get_known_peers()
{
	boost::filesystem::path peers_file("peers");
	if(!boost::filesystem::exists(peers_file) || !boost::filesystem::is_regular_file(peers_file))
		throw std::runtime_error("peers file is missing");
	boost::filesystem::fstream peers_stream;
	peers_stream.open(peers_file, boost::filesystem::fstream::in);
	while(!peers_stream.eof()) 
	{
		char address[256];
		peers_stream.getline(address,sizeof(address)/sizeof(char));
		if(std::string(address).size() > 1)
		{
			std::cout << "Peer found in peers file: " << address << "\n";
			peers.insert(std::string(address));
		}
	}
	if(peers.empty())
		throw std::runtime_error("Couldn't find any peers in peers file\n");
	std::cout << "End of known peers" << std::endl;
}

void Server::add_peer(std::string peer)
{
	if(!peers.count(peer))
	{
		boost::algorithm::trim(peer);
		std::cout << "Adding new peer : " << peer << std::endl;
		peers.insert(peer);
	}
}

std::set<std::string> Server::get_peers()
{
	return peers;
}

std::string Server::get_hash()
{
	return hash;
}

std::string Server::get_next_hash()
{
	return next_hash;
}

void Server::set_next_hash(std::string str)
{
	next_hash = str;	
}

void Server::close_previous(Connection* new_prev)
{
	prev_con = std::shared_ptr<Connection>(new_prev);
}

void Server::set_next_connection(Connection* con)
{
	next_con = std::shared_ptr<Connection>(con);
}

void Server::ask_for_next(std::string address, std::string peer_hash)
{
	auto endpoint = resolver.resolve({ address, std::to_string(port)});
	boost::asio::async_connect(socket,endpoint,
			[this,address,peer_hash](boost::system::error_code ec, boost::asio::ip::tcp::resolver::iterator)
			{
				if(!ec)
				{
					std::make_shared<Connection>(std::move(socket), this)->start_ask(address, peer_hash);
				}
			});
}

