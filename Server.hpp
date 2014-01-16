#include <string>
#include <iostream>
#include <vector>
#include <stdexcept>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include "Connection.hpp"
#include "Identify.hpp"

class Server
{
    public:
        Server(boost::asio::io_service &io, short port, std::string bootstrap = "");
		
        
    private:
        void accept();
 		void get_known_peers();

        std::string hash;
        boost::asio::ip::tcp::acceptor acceptor;
        boost::asio::ip::tcp::socket socket;
		std::vector< std::string > peers;
};
