#include "Connection.hpp"

/*
    Tworzy nowy obiekt Connection dla przychodzacegp
    polaczenia, ustawia stan na GET_HASH
*/
Connection::Connection(boost::asio::ip::tcp::socket s, Server *_server) :
    socket(std::move(s)), server(_server), state(GET_HASH), outgoing(0),
    remaining_file(-1)
{
    std::cout << "Created connection" << std::endl;
}

/*
    Tworzy nowy obiekt Connection dla wychodzacego
    polaczenia, ustawia stan na GET_HASH
*/
Connection::Connection(boost::asio::ip::tcp::socket *s, Server *_server) :
     socket(s->get_io_service()), server(_server), state(GET_HASH), outgoing(1),
     remaining_file(-1)
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
    jestesmy jego poprzednikiem lub wysyla VOLUNTEER
*/
void Connection::init(boost::asio::ip::tcp::resolver::iterator endpoint, int cmd)
{
    auto self = shared_from_this();
    boost::asio::async_connect(socket, endpoint,
        [this, self, cmd](boost::system::error_code ec, boost::asio::ip::tcp::resolver::iterator)
        {
            if(!ec)
            {
                start(cmd);
            }
        });
}

/*
    Wysyla nasz identyfikator, jezeli propose == 1
    to pyta, czy jestesmy poprzednikiem, jezeli
    propose == 2 to wysyla VOLUNTEER
*/
void Connection::start(int propose)
{
    if(propose)
        std::cout << "Connected to : ";
    else
        std::cout << "Connection from : ";
    std::cout << get_address() << std::endl;
    read();
    write(server->get_hash());
    if(propose == 1)
        write(std::to_string(PREVIOUS));
    if(propose == 2)
        write(std::to_string(VOLUNTEER));
    outgoing = propose;
}

/*
    Wywoluje asynchroniczny write dla skojarzonego z obiektem
    socketu
*/
void Connection::write(std::string write_data)
{ 
	write_data += msg_split_char;
    auto ptr = std::make_shared<std::string>(write_data);
	auto self(shared_from_this());
	boost::asio::async_write(socket, boost::asio::buffer(ptr->c_str(), ptr->length()),
        [this, self, ptr](boost::system::error_code ec, std::size_t length)
		{});
}

/*
    Konczy polaczenie po wyslaniu adresu, ktory zastapil 
    to polaczenie.
*/
void Connection::redirect(std::string address)
{
    std::string data = std::to_string(REDIRECT) + msg_split_char + address + msg_split_char;
    auto self(shared_from_this());
    boost::asio::async_write(socket, boost::asio::buffer(data.c_str(), data.size()),
        [this, self](boost::system::error_code ec, std::size_t length)
        {
            end();
        });
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
                if(length != max_msg)
                    data[length] = 0;
				current_msg += std::string(data, length);
                std::vector<std::string> split_vec;
                boost::split(split_vec, current_msg, boost::is_any_of(msg_split_char));
				for(int i = 0; i < split_vec.size()-1;i++) 
                    msg_queue.push(split_vec[i]);
                if(split_vec.back().size())
                    current_msg = split_vec.back();
                else current_msg = "";
                while(!msg_queue.empty())
                {
//					std::cout << "Message queue : " << msg_queue.front() << ", state : " << state << std::endl;
    				switch(state)
					{
						case AWAIT_QUERY:
							switch(packets(msg_queue.front()[0]-'0'))
							{
								case PREVIOUS:
									handle_prev();
									break;
                                case REDIRECT:
                                    state = REDIRECT;
                                    break;
                                case VOLUNTEER:
                                    write(std::to_string(server->get_file_size()));
                                    write(server->get_file());
                                    break;
                                case SEARCH:
                                    state = SEARCH;
                                    break;
							}
							break;
						case GET_HASH:
							con_hash = msg_queue.front();
                            server->add_peer(get_address());
                            if(outgoing == 1)
                                state = PROPOSED;
                            else if(outgoing == 2)
                                state = VOLUNTEER;
                            else
    							state = AWAIT_QUERY;
							break;
                        case PROPOSED:
                            if(msg_queue.front().length() == 1 && 
                                msg_queue.front()[0]-'0' == ACCEPTED) {
                                server->change_next(shared_from_this());
                                state = AWAIT_QUERY;
                            } else {
                                server->connect_to(msg_queue.front());
                            }
                            break;
                        case REDIRECT:
                            server->connect_to(msg_queue.front());
                        case SEARCH:
                            command_strings.push_back(msg_queue.front());

                            if(command_strings.size() == 2) {
                                int ttl = std::stoi(command_strings[1]);

                                if(command_strings[0] == "ME")
                                    command_strings[0] = get_address();

                                if(ttl > 0)
                                        server->search_for_volunteers(command_strings[0], ttl-1);
                                server->connect_to(command_strings[0], 2);

                                command_strings.clear();
                                state = AWAIT_QUERY;
                            }
                            break;
                        case VOLUNTEER:
                            if(remaining_file == -1) {
                                remaining_file = std::stoi(msg_queue.front());
                            } else {
                                file += msg_queue.front();
                                remaining_file -= msg_queue.front().length();
                                if(remaining_file > 0) {
                                    file += msg_split_char;
                                    remaining_file--;
                                }
                                if(remaining_file <= 0) {
                                    server->handle_file(file);
                                    file = "";
                                    remaining_file = -1;
                                    end();
                                }
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
        std::string address = server->get_next_address();
		std::cout << "Refused as prev, redirecting to : " << address << std::endl;
        write(address);
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
std::string Connection::get_address()
{
    return socket.remote_endpoint().address().to_string();
}

/*
    Przekazuje pytanie o mozliwosc uruchomienia pliku
    wykonywalnego do nastepnika
*/
void Connection::search_for_volunteers(std::string who, int ttl)
{
    write(std::to_string(SEARCH) + msg_split_char + who + msg_split_char
         + std::to_string(ttl) + msg_split_char);
}
