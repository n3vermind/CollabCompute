#include "Server.hpp"

Server::Server(boost::asio::io_service &io, short port, std::string bootstrap) :
    acceptor(io, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
    socket(io), hash(Identify::getId())
{
	get_known_peers();
    if(bootstrap != "")
        std::cout << "Bootstraping to " << bootstrap << std::endl;
	for(auto i = peers.begin(); i != peers.end(); i++)
	{
		boost::asio::ip::tcp::resolver resolver(io);
		auto endpoint = resolver.resolve({ *i, "9999"});
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
			peers.push_back(std::string(address));
		}
	}
	if(peers.empty())
		throw std::runtime_error("Couldn't find any peers in peers file\n");
	std::cout << "End of known peers" << std::endl;
}

void Server::add_peer(std::string peer)
{
	std::cout << "Adding new peer : " << peer << std::endl;
	peers.push_back(peer);
}

std::vector<std::string> Server::get_peers()
{
	return peers;
}
