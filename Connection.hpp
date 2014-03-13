#pragma once
#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>
#include <queue>
#include <vector>
#include <iostream>
#include <string>

#include "Server.hpp"

class Server;

class Connection :
    public std::enable_shared_from_this<Connection>
{
    public:
        Connection(boost::asio::ip::tcp::socket s, Server *_server);
        Connection(boost::asio::ip::tcp::socket *s, Server *_server);
        ~Connection();
        void init(boost::asio::ip::tcp::resolver::iterator endpoint);
        void start(bool propose = false);
		void start_prev(std::string address, std::string hash);
		void start_send_peers(int ttl);
		void send_get_peers(std::string address, int ttl);
        std::string get_address();
		std::string get_hash();
		void end();
    private:
        void read();
		void write(std::string write_data);
		void get_next();
		void handle_prev();
		void send_peers(std::string address, int ttl);
		bool is_good_placement(std::string a, std::string b, std::string peer);
		
        boost::asio::ip::tcp::socket socket;
		Server* server;
        std::string con_hash;
		enum { max_msg = 1024};
        char data[max_msg];
        std::string current_msg;
        std::queue< std::string > msg_queue;
        const std::string msg_split_char = "~";
		enum packets { GET_HASH, AWAIT_QUERY, PREVIOUS, ACCEPTED, PROPOSED };
		std::vector< std::string > command_strings;
		packets state;
};

