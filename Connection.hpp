#pragma once
#include <boost/asio.hpp>

#include "Server.hpp"

class Server;

class Connection :
    public std::enable_shared_from_this<Connection>
{
    public:
        Connection(boost::asio::ip::tcp::socket s, Server* server);
        void start_accept();
		void start_out();
    private:
        void read();
		void write(std::string write_data);
		void send_peers();
		void get_peers();

        boost::asio::ip::tcp::socket socket;
		Server* server;
        enum { max_msg = 1024};
        char data[max_msg];
		enum connection_state { AWAIT_QUERY, GET_PEERS };
		connection_state state;
		const char* QUERY_SEND_PEERS = "Y i need peers mate?";
		const char* END_OF_PEERS = "So peers ended mate. Get lost.";
};

