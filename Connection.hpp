#pragma once
#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>
#include <queue>
#include <vector>

#include "Server.hpp"

class Server;

class Connection :
    public std::enable_shared_from_this<Connection>
{
    public:
        Connection(boost::asio::ip::tcp::socket s, Server* server);
        void start_accept();
		void start_out();
		void start_ask(std::string address, std::string hash);
		void end();
    private:
        void read();
		void write(std::string write_data);
		//void send_peers();
		//void get_peers();
		void get_next();
		bool is_good_placement(std::string a, std::string b, std::string peer);

        boost::asio::ip::tcp::socket socket;
		Server* server;
        enum { max_msg = 1024};
        char data[max_msg];
        std::string current_msg;
        std::queue< std::string > msg_queue;
        const std::string msg_split_char = "~";
		enum states { GET_HASH, AWAIT_QUERY, PREVIOUS, LOOKING_FOR_SPOT, WAITING_FOR_SPOT, GET_PEERS };
		const std::string ACCEPTED_PREVIOUS = "OK";
		states state;
};

