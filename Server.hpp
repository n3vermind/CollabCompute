#pragma once
#include <string>
#include <iostream>
#include <set>
#include <stdexcept>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/algorithm/string.hpp>

#include "Connection.hpp"
#include "Identify.hpp"

class Connection;

class Server
{
    public:
        Server(boost::asio::io_service &io, short port, std::string bootstrap = "");
		void add_peer(std::string peer);		
		std::set<std::string> get_peers();
		void set_next_hash(std::string str);
		std::string get_next_hash();
		void close_previous(Connection* new_prev);
		void ask_for_next(std::string address, std::string peer_hash);
		void set_next_connection(Connection* con);
		std::string get_hash();

    private:
        void accept();
 		void get_known_peers();
		
		short port;
        std::string hash;
		std::string next_hash;
        boost::asio::ip::tcp::acceptor acceptor;
		boost::asio::ip::tcp::resolver resolver;
        boost::asio::ip::tcp::socket socket;
		std::set< std::string > peers;
		std::shared_ptr<Connection> next_con,prev_con;
};
