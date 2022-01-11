#include "../include/Server.hpp"

Server::Server(int port, std::string const &password)
: _port(port), _password(password), _servername("Nasha_Iro4ka_1.0")
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

//TEST
void	Server::server_test_client()
{
	Client	test_client;

	test_client.client_test_loop(*this);
}

// commands
void	Server::commandPASS(Client &client, Message &msg)
{
	if (msg.prefix.empty())
	{
		if (client.getRegistrationStatus())
			this->sendReply(generateErrorReply(this->_servername, ERR_ALREADYREGISTRED));
		else if (msg.params.empty())
			this->sendReply(generateErrorReply(this->_servername, ERR_NEEDMOREPARAMS, "PASS"));
		else
			client.setPassword(msg.params[0]);
	}
}
