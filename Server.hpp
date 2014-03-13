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
        Server(boost::asio::io_service &io, short port, std::string bootstrap = "");
		void add_peer(std::string peer);
		std::set<std::string> get_peers();
		std::string get_next_hash();
        std::string get_next_address();
        bool restore_next();
		void change_prev(std::shared_ptr<Connection> new_prev);
		void change_next(std::shared_ptr<Connection> con);
		std::string get_hash();
		void connect_to(std::string address);
    private:
        void accept();
 		void get_known_peers();
		void find_next();
		short port;
        std::string hash;
        boost::asio::ip::tcp::acceptor acceptor;
		boost::asio::ip::tcp::resolver resolver;
        boost::asio::ip::tcp::socket socket;
		std::set< std::string > peers;
		std::weak_ptr<Connection> next_con,prev_con;
		std::vector<std::shared_ptr<Connection> > connections;
};
