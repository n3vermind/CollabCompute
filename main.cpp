#include <iostream>
#include <ctime>
#include "Server.hpp"

int main(int argc, char **argv) { // MZ
	std::cout.setf( std::ios_base::unitbuf );
	boost::asio::io_service io;
    Server *s;

    if(argc == 2)
        s = new Server(io, 9999, std::string(argv[1]));
    else
        s = new Server(io, 9999);
    io.run();
    return 0;
}
