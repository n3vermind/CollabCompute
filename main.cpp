#include <iostream>

#include "Server.hpp"

int main(int argc, char **argv) {
    boost::asio::io_service io;
    std::string bootstrap;
    Server *s;

    if(argc == 2)
        s = new Server(io, 9999, std::string(argv[1]));
    else
        s = new Server(io, 9999);

    io.run();
    return 0;
}
