#pragma once
#include <string>
#include <iostream>
#include <set>
#include <stdexcept>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/bind.hpp>

#include "Connection.hpp"
#include "Identify.hpp"

class Connection;

class Server
{
    public:
        Server(boost::asio::io_service &io, short port, std::string bootstrap = "", bool init = false);
		void add_peer(std::string peer);		
		std::set<std::string> get_peers();
		std::string get_next_hash();
		void close_previous(Connection* new_prev);
		void ask_for_next(std::string address, std::string peer_hash);
		void set_next_connection(Connection* con);
		std::string get_hash();
		void change_prev(std::string address, std::string peer_hash);
		void send_peers(std::string address, int ttl);
		void ask_for_peers(int ttl);
    private:
        void accept();
 		void get_known_peers();
		void find_next();
		void find_next(std::string address);
		void console();
		void verify_next(); // if we disconnected from next we need to find another one
		enum { number_of_peers = 50 };
		bool init;
		short port;
        std::string hash;
        boost::asio::ip::tcp::acceptor acceptor;
		boost::asio::ip::tcp::resolver resolver;
        boost::asio::ip::tcp::socket socket;
		boost::asio::deadline_timer timer;
		std::set< std::string > peers;
		std::shared_ptr<Connection> next_con,prev_con;
		std::vector<std::shared_ptr<Connection> > pierdolekurwa;
};
