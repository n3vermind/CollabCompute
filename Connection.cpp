#include "Connection.hpp"

/*
    Tworzy nowy obiekt Connection dla przychodzacegp
    polaczenia, ustawia stan na GET_HASH
*/
Connection::Connection(boost::asio::ip::tcp::socket s, Server *_server) :
    socket(std::move(s)), server(_server), state(GET_HASH)
{
    std::cout << "Created connection" << std::endl;
}

/*
    Tworzy nowy obiekt Connection dla wychodzacego
    polaczenia, ustawia stan na GET_HASH
*/
Connection::Connection(boost::asio::ip::tcp::socket *s, Server *_server) :
     socket(s->get_io_service()), server(_server), state(GET_HASH)
{
    std::cout << "Created connection" << std::endl;
}

/*
    Destruktor klasy Connection
*/
Connection::~Connection()
{
    std::cout << "Destructor for connection : " << con_hash << std::endl;
}

/*
    Inicjuje polaczenie z danym endpointem, pyta, czy
    jestesmy jego poprzednikiem
*/
void Connection::init(boost::asio::ip::tcp::resolver::iterator endpoint)
{
    auto self = shared_from_this();
    boost::asio::async_connect(socket, endpoint,
        [this, self](boost::system::error_code ec, boost::asio::ip::tcp::resolver::iterator)
        {
            if(!ec)
            {
                start(true);
            }
        });
}

/*
    Wysyla nasz identyfikator, jezeli propose == true
    to pyta, czy jestesmy poprzednikiem
*/
void Connection::start(bool propose)
{
    if(propose)
        std::cout << "Connected to : ";
    else
        std::cout << "Connection from : ";
    std::cout << get_address() << std::endl;
    read();
    write(server->get_hash());
    if(propose)
        write(std::to_string(PREVIOUS));
}

/*
    Wywoluje asynchroniczny write dla skojarzonego z obiektem
    socketu
*/
void Connection::write(std::string write_data)
{
	write_data += msg_split_char;
	auto self(shared_from_this());
	boost::asio::async_write(socket, boost::asio::buffer(write_data.c_str(),write_data.size()),
		[this, self](boost::system::error_code ec, std::size_t length)
		{});
}

/*
    Funkcja wywolywana przez async_read w momencie gdy mamy
    dane do przeczytania. Wokorzystujac std::vector dzieli
    pakiet wykorzystujac delimiter, a nastepnie parsuje
    kazde z otrzymanych polecen
*/
void Connection::read()
{
    auto self(shared_from_this());
    socket.async_read_some(boost::asio::buffer(data, max_msg),
        [this, self](boost::system::error_code ec, std::size_t length)
        {
            if(!ec) {
                data[length] = 0;
				current_msg += std::string(data);
                std::vector<std::string> split_vec;
                boost::split(split_vec, current_msg, boost::is_any_of(msg_split_char), boost::token_compress_on);
				for(int i = 0; i < split_vec.size()-1;i++) 
                {
                    if(split_vec[i].size())
                        msg_queue.push(split_vec[i]);
                }
                if(split_vec.back().size())
                    current_msg = split_vec.back();
                else current_msg = "";
                while(!msg_queue.empty())
                {
					std::cout << "Message queue : " << msg_queue.front() << ", state : " << state << std::endl;
    				switch(state)
					{
						case AWAIT_QUERY:
							switch(packets(msg_queue.front()[0]-'0'))
							{
								case PREVIOUS:
									handle_prev();
									break;
                                case ACCEPTED:
                                    server->change_next(shared_from_this());
                                    break;
							}
							break;
						case GET_HASH:
							con_hash = msg_queue.front();
							state = AWAIT_QUERY;
                            server->add_peer(get_address());
							break;
                        case PROPOSED:
                            if(msg_queue.front().length() == 0 && 
                                msg_queue.front()[0]-'0' == ACCEPTED) {
                                server->change_next(shared_from_this());
                            } else {
                                server->connect_to(msg_queue.front());
                            }
                            break;
    				}
			    	msg_queue.pop();
                }
				read();
            }
        });
}

/*
    Sprawdza, czy host rzeczywiscie jest naszym poprzednikiem,
    jezeli tak to odpowiadamy ACCEPTED oraz zapisujemy Connection
    jako Server->prev_con, wpp odpowiadamy adresem nastepnej osoby
    i konczymy polaczenie 
*/
void Connection::handle_prev()
{
	if(is_good_placement(server->get_next_hash(), server->get_hash(), con_hash))
	{
		server->change_prev(shared_from_this());
		write(std::to_string(ACCEPTED));
		std::cout<< "Accepted as prev" << std::endl;

	} 
	else 
	{
		std::cout << "Refused as prev" << std::endl;
        write(server->get_next_address());
		end();
	}
	state = AWAIT_QUERY;
}

/*
    Konczymy polaczenie, zamykamy socket
*/
void Connection::end()
{
	boost::system::error_code ec;
	socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
}

/*
    Sprawdzamy, czy peer jest naszym poprzednikiem
    na podstawie identyfikatorow a i b
*/
bool Connection::is_good_placement(std::string a, std::string b, std::string peer)
{
    if(a == b)
        return true;
	if((a <= b && a <= peer && peer <= b) ||
        (a >= b && (a >= peer || peer >= b)))
		return true;
	return false;
}

/*
    Zwraca identyfikator drugiego konca polaczenia
*/
std::string Connection::get_hash()
{
	return con_hash;
}

/*
    Zwraca adres drugiego konca polaczenia
*/
std::string Connection::get_address() {
    return socket.remote_endpoint().address().to_string();
}
