#pragma once
#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>
#include <queue>
#include <vector>
#include <iostream>
#include <string>

#include "Server.hpp"

class Server;																	// KK

class Connection :																// MZ
    public std::enable_shared_from_this<Connection>
{
    public:
        Connection(boost::asio::ip::tcp::socket s, Server *_server);
        Connection(boost::asio::ip::tcp::socket *s, Server *_server);
        ~Connection();
        void init(boost::asio::ip::tcp::resolver::iterator endpoint, int cmd);
        void start(int propose = 0);
        std::string get_address();											    // KK
		std::string get_hash();
		void end();
        void redirect(std::string address);										// MZ
        void search_for_volunteers(std::string who, int ttl);
    private:
        void read();
		void write(std::string write_data);										// KK
		void get_next();
		void handle_prev();
		bool is_good_placement(std::string a, std::string b, std::string peer);
		
        boost::asio::ip::tcp::socket socket;									// MZ
		Server* server;															// KK
        std::string con_hash;	
		enum { max_msg = 2048};
        char data[max_msg];
        std::string current_msg;
        std::queue< std::string > msg_queue;
        const std::string msg_split_char = "~";
		enum packets { GET_HASH, AWAIT_QUERY, PREVIOUS, ACCEPTED, PROPOSED,
            REDIRECT, VOLUNTEER, SEARCH };
		std::vector< std::string > command_strings;
		packets state;													
        int outgoing;															// MZ
        int remaining_file;
        std::string file;
};

