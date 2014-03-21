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
    std::istream is(&input_buffer);

	int choice;
    is >> choice;
	switch(choice) {
		case 1:
			std::cout << server->get_next_hash() << std::endl;
			break;
        case 2:
            std::string file;
            is >> file;
            if(file.empty())
                break;
            server->read_file(file);
            server->search_for_volunteers("ME", 0);
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
