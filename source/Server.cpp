#include "../include/Server.hpp"

Server::Server(int port, std::string const &password)
: _port(port), _password(password), _servername("Nasha Iro4ka 1.0")
{
	this->parseMOTD();
}

Server::~Server() {}

// private methods
void    Server::parseMOTD()
{
	std::ifstream	file("files/MOTD");
	std::string		str;
	while (file.is_open() && getline(file, str))
		this->_MOTD.push_back(str);
	file.close();
}

void	Server::sendReply(std::string const &reply) const
{
	std::cout << reply << std::endl;
}

// commands
void	Server::commandPASS(Client &client, Message &msg)
{
	if (msg.prefix.empty() && client.getNickname().empty() && client.getUsername().empty())
	{
		if (client.getRegistrationStatus())
			this->sendReply(generateErrorReply(this->_servername, ERR_ALREADYREGISTRED));
		else if (msg.params.empty())
			this->sendReply(generateErrorReply(this->_servername, ERR_NEEDMOREPARAMS, "PASS"));
		else
			client.setPassword(msg.params[0]);
	}
}
