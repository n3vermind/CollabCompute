#pragma once
#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "Server.hpp"

class Server; // KK

class Console
{
	public:
		Console(boost::asio::io_service &io, Server* server);
	private:
		Server* server;
		boost::asio::posix::stream_descriptor input;
		boost::asio::posix::stream_descriptor output;
		boost::asio::streambuf input_buffer;
		void handle_read(const boost::system::error_code& error, std::size_t length);
};

