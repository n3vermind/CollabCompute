#include "Server.hpp"

/*
    Tworzy obiekt Server, jezeli bootstrap == "" podlacza sie do sieci
    do pierwszego hosta obecnego w pliku peers, wpp laczy sie z hostem
    bootstrap. Oczekuje na polaczenia przychodzace na porcie 9999
*/
Server::Server(boost::asio::io_service &io, short port, std::string bootstrap) :
    acceptor(io, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
    socket(io), hash(Identify::getId()), resolver(io), port(port),
    console(std::unique_ptr<Console>(new Console(io, this)))
{
    std::cout << "Hash : " << hash << std::endl;
	get_known_peers();
    if(bootstrap != "") {
        std::cout << "Bootstraping to " << bootstrap << std::endl;
		connect_to(bootstrap);
	} else
		find_next();
	accept();
}

/*
    Funckja wywolywana asynchronicznie dla przychodzacego polaczenia,
    tworzy nowy obiekt Connection i ponownie wywoluje async_accept
*/
void Server::accept()
{
    acceptor.async_accept(socket, 
        [this](boost::system::error_code ec)
        {
            if(!ec) {
                std::make_shared<Connection>(std::move(socket), this)->start();
            }
            accept();
        });
}

/*
    Wczytuje z pliku peers adresy znanych czlonkow sieci P2P i zapisuje 
    je do std::set peers
*/
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
			add_peer(std::string(address));
		}
	}
	if(peers.empty())
		throw std::runtime_error("Couldn't find any peers in peers file\n");
}

/*
    Dodaje peer do zbioru znanych hostow jezeli go jeszcze tam nie bylo 
*/
void Server::add_peer(std::string peer)
{
	if(!peers.count(peer))
	{
		boost::algorithm::trim(peer);
		std::cout << "Adding new peer : " << peer << std::endl;
		peers.insert(peer);
	}
}

/*
    Zwraca std::set zawierajacy znanych czlonkow sieci
*/
std::set<std::string> Server::get_peers()
{
	return peers;
}

/*
    Zwraca randomizowany przy uruchomieniu identyfikator serwera
*/
std::string Server::get_hash()
{
	return hash;
}

/*
    Zwraca identyfikator naszego nastepnika w sieci, jezeli
    utracilismy z nim polaczenie probojemy sie laczyc z kolejnymi
    z zapamietanych czlonkow sieci
*/
std::string Server::get_next_hash()
{
    auto ptr = next_con.lock();
    if(ptr)
        return ptr->get_hash();
    if(!restore_next())
        return hash;
    return get_next_hash();
}

/*
    Zwraca adres naszego nastepnika, jezeli utracilismy z nim
    polaczenie probojemy sie laczyc z kolejnymi z zapamietanych
    czlonkow sieci
*/
std::string Server::get_next_address()
{
    auto ptr = next_con.lock();
    if(ptr)
        return ptr->get_address();
    restore_next();
    return get_next_address();
}

/*
    Jezeli jestesmy ostatnia znana osoba w sieci zwraca false,
    wpp nawiazuje polaczenie z kolejnym zapamietanym hostem
    i zwraca true
*/
bool Server::restore_next()
{
    if(peers.size() == 1) 
        return false;
    std::string address = *peers.begin();
    connect_to(address);
    peers.erase(peers.begin());
    return true;
}

/*
    Konczy polaczenie z poprzednikiem i zapisuje na jego miejscu
    nowe polaczenie - new_prev
*/
void Server::change_prev(std::shared_ptr<Connection> new_prev)
{
    auto ptr = prev_con.lock();
	if(ptr) {
		ptr->redirect(new_prev->get_address());
    }
	prev_con = new_prev;
}

/*
    Konczy polaczenie z nastepnikiem i zapisuje na jego miejscu
    nowe polaczenie - con
*/
void Server::change_next(std::shared_ptr<Connection> con)
{
    auto ptr = next_con.lock();
    if(ptr) {
        ptr->end();
    }
	next_con = con;
	std::cout << "Picked new next." << std::endl;
}

/*
    Laczy sie z hostem z std::set peers i pyta, czy jest jego
    poprzednikiem
*/
void Server::find_next()
{
    connect_to(*peers.begin());
}

/*
    Laczy sie z podanym adresem, tworzy nowy obiekt Connection
*/
void Server::connect_to(std::string address, int what)
{
	auto endpoint = resolver.resolve({ address, std::to_string(port) });
    std::make_shared<Connection>(&socket, this)->init(endpoint, what);
}

/*
    Wysyla zapytanie o mozliwosc uruchomienia pliku wykonywalnego 
*/
void Server::search_for_volunteers(std::string who, int ttl)
{
    auto ptr = next_con.lock();
    if(ptr)
        return ptr->search_for_volunteers(who, ttl);
    restore_next();
    search_for_volunteers(who, ttl);
}

/*
    Otwiera i wczytuje do std::string file podany plik
*/
void Server::read_file(std::string path)
{
	boost::filesystem::path file_name(path);
	if(!boost::filesystem::exists(file_name) || !boost::filesystem::is_regular_file(file_name))
		throw std::runtime_error("File is missing");
	boost::filesystem::fstream file_stream;
	file_stream.open(file_name, boost::filesystem::fstream::in);

    std::stringstream sstr;
    sstr << file_stream.rdbuf();
    file = sstr.str();
}

/*
    Zwraca std::string file
*/
int Server::get_file_size()
{
    return file.length();
}

/*
    Zwraca dlugosc pliku
*/
std::string Server::get_file()
{
    return file;
}
