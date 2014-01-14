#include <boost/asio.hpp>

class Connection :
    public std::enable_shared_from_this<Connection>
{
    public:
        Connection(boost::asio::ip::tcp::socket s);
        void start();

    private:
        void read();

        boost::asio::ip::tcp::socket socket;
        enum { max_msg = 1024};
        char data[max_msg];
};
