#include "Server.hpp"

Server::Server(int port, std::string const &password) : _port(port), _password(password)
{
	this->parseMOTD();
}

// private methods
void    Server::parseMOTD()
{
	std::ifstream	file("files/MOTD");
	std::string		str;
	while (file.is_open() && getline(file, str))
		this->_MOTD.push_back(str);
	file.close();
}
