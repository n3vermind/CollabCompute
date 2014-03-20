#include "Console.hpp"

Console::Console(boost::asio::io_service &io, Server* server) 
	: server(server),
	input(io, ::dup(STDIN_FILENO)),
	output(io, ::dup(STDOUT_FILENO))
{
		boost::asio::async_read_until(input, input_buffer, '\n',
				boost::bind(&Console::handle_read,this,
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
}

void Console::handle_read(const boost::system::error_code& error, std::size_t length)
{
	boost::asio::streambuf::const_buffers_type bufs = input_buffer.data();
	std::string in(boost::asio::buffers_begin(bufs), 
			boost::asio::buffers_begin(bufs) + input_buffer.size());
	int choice = -1;
	sscanf(in.c_str(),"%d",&choice);
	switch(choice) {
		case 0:
			std::cout << server->get_next_hash() << std::endl;
			break;
        case 1:
            server->search_for_volunteers("TESTING", 10);
            break;
	}
	input_buffer.consume(input_buffer.size());
	boost::asio::async_read_until(input, input_buffer, '\n',
			boost::bind(&Console::handle_read,this,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
}

void Console::handle_write(const boost::system::error_code& error)
{

}
