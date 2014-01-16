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

class Server
{
    public:
        Server(boost::asio::io_service &io, short port, std::string bootstrap = "");
		void add_peer(std::string peer);		
		std::set<std::string> get_peers();
        
    private:
        void accept();
 		void get_known_peers();

        std::string hash;
        boost::asio::ip::tcp::acceptor acceptor;
        boost::asio::ip::tcp::socket socket;
		std::set< std::string > peers;
};
