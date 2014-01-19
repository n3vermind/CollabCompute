#include "Server.hpp"

Server::Server(boost::asio::io_service &io, short port, std::string bootstrap, bool init) :
    acceptor(io, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
    socket(io), hash(Identify::getId()), resolver(io), port(port), init(init),
	timer(io, boost::posix_time::seconds(1))
{
	prev_con = NULL;
	next_con = NULL;
	get_known_peers();
    if(bootstrap != "") {
        std::cout << "Bootstraping to " << bootstrap << std::endl;
		find_next(bootstrap);
	} else
		find_next();
	//timer.async_wait(boost::bind(&Server::console, this));
	accept();
}

void Server::console()
{
	std::cout << "console" << std::endl;
	std::string in;
	std::cin >> in;
	if(in == "get_peers")
		ask_for_peers(number_of_peers);
	timer.expires_at(timer.expires_at() + boost::posix_time::seconds(1));
	timer.async_wait(boost::bind(&Server::console, this));
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
	std::cout<< "aaa" << std::endl;
	std::cout<< hash << std::endl;
	
	return hash;
}

std::string Server::get_next_hash()
{
	if(init) 
	{
		init = false;
		return hash;
	}
	if(next_con!=NULL){
		return next_con->get_hash();
	}else {
		find_next();
		get_next_hash();
	}
}

void Server::close_previous(Connection* new_prev)
{
	std::cout << "Picked new prev." << std::endl;
	if(prev_con)
		prev_con->end();
	connections.push_back(prev_con);
	prev_con = std::shared_ptr<Connection>(new_prev);
}

void Server::set_next_connection(Connection* con)
{
	std::cout << "Picked new next." << std::endl;
	connections.push_back(next_con);
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

void Server::change_prev(std::string address, std::string peer_hash)
{

	auto endpoint = resolver.resolve({ address, std::to_string(port)});
	boost::asio::async_connect(socket,endpoint,
			[this,address,peer_hash](boost::system::error_code ec, boost::asio::ip::tcp::resolver::iterator)
			{
				if(!ec)
				{
					std::make_shared<Connection>(std::move(socket), this)->start_prev(address, peer_hash);
				}
			});
}
void Server::find_next()
{
	auto current = get_peers();
	find_next(*current.begin());	
}
void Server::find_next(std::string address)
{
	auto endpoint = resolver.resolve({ address, std::to_string(port)});
	boost::asio::async_connect(socket,endpoint,
			[this](boost::system::error_code ec, boost::asio::ip::tcp::resolver::iterator)
			{
				if(!ec)
				{
					std::make_shared<Connection>(std::move(socket), this)->start_out();
				}
			});
}

void Server::send_peers(std::string address, int ttl)
{
	auto endpoint = resolver.resolve({ address, std::to_string(port)});
	boost::asio::async_connect(socket,endpoint,
			[this,ttl](boost::system::error_code ec, boost::asio::ip::tcp::resolver::iterator)
			{
				if(!ec)
				{
					std::make_shared<Connection>(std::move(socket), this)->start_send_peers(ttl - 1);
				}
			});
}

void Server::ask_for_peers(int ttl)
{
	verify_next();
	next_con->send_get_peers(socket.local_endpoint().address().to_string(),ttl);
}
void Server::verify_next()
{
}
