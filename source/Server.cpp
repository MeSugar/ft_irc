#include "../include/Server.hpp"

Server::Server(int port, std::string const &password) : TemplateRun(port, password)
{
	this->_port = port;
	this->_password = password;
	this->_servername = "Nasha Iro4ka 1.0";
	this->_operatorHosts.push_back("host");
	this->_operators.insert(std::pair<std::string, std::string>("admin", "admin"));
	this->parseMOTD();
}

Server::~Server() {}

// utils
void    Server::parseMOTD()
{
	std::ifstream	file("files/MOTD");
	std::string		str;
	while (file.is_open() && getline(file, str))
		this->_MOTD.push_back(str);
	file.close();
}

void	Server::sendMOTD()
{
	for (std::vector<std::string>::iterator it = this->_MOTD.begin(); it != this->_MOTD.end(); it++)
		this->sendReply(*it);
}

void	Server::sendReply(std::string const &reply) const
{
	std::cout << reply << std::endl;
}

//TEST
void	Server::server_test_client()
{
	Client	test_client;

	test_client.setRegistrationStatus();
	test_client.client_test_loop(*this);
}


bool	Server::validateNickname(std::string const &nick)
{
	size_t len = nick.size();
	if (!nick.empty() && len <= 9)
	{
		size_t i = 0;
		while (i < len)
		{
			if (!std::isalnum(nick[i]) || nick[i] != '-'
				|| nick[i] != '[' || nick[i] != ']'
				|| nick[i] != '{' || nick[i] != '}'
				|| nick[i] != '^' || nick[i] != '\\')
				return false;
			i++;
		}
		return true;
	}
	return false;
}

bool	Server::comparePrefixAndNick(std::string const &prefix, Client const &client)
{
	std::string nick = ':' + client.getNickname();
	if (prefix == nick)
		return true;
	return false;
}

Client	*Server::findClient(std::string const &nick, std::vector<Client *> &clients)
{
	std::vector<Client *>::iterator it = clients.begin();
	std::vector<Client *>::iterator ite = clients.end();
	for (; it != ite; it++)
		if ((*it)->getNickname() == nick)
			return *it;
	return NULL;
}

void	Server::removeClient(Client *client, std::vector<Client *> &clients)
{
	std::vector<Client *>::iterator it = clients.begin();
	std::vector<Client *>::iterator ite = clients.end();
	for (; it != ite; it++)
	{
		if ((*it) == client)
		{
			delete *it;
			clients.erase(it);
		}
	}
}

void	Server::addClient(Client *client)
{
	if (!client->getRegistrationStatus() && client->getPassword() == this->_password
			&& !client->getNickname().empty() && !client->getUsername().empty())
	{
		client->setRegistrationStatus();
		this->_clients.push_back(client);
		this->_connectedClients.push_back(client);
		this->sendMOTD();
	}
}

int		Server::checkOperatorList(std::string const &user, std::string const &pass)
{
	std::map<std::string, std::string>::iterator it = this->_operators.find(user);
	if (it == _operators.end())
		return ERR_WRONGUSERNAME;
	else if (it->second != pass)
		return ERR_PASSWDMISMATCH;
	else
		return 0;
}

bool	Server::checkHostnameList(std::string const &host)
{
	for (std::vector<std::string>::iterator it = this->_operatorHosts.begin(); it != this->_operatorHosts.end(); it++)
		if ((*it) == host)
			return true;
	return false;
}

int Server::chat(int sockfd) {
	std::string msg;
	char buff[100];
	int cntBytes;

	while ((cntBytes = recv(sockfd, buff, 99, 0)) > 0) {
		buff[cntBytes] = 0;
		msg += buff;
		buff[0] = 0;
		if (msg.find('\n') != std::string::npos) {
			break;
		}
	}
	if (msg.length() > 512)
		msg = msg.substr(0, 510) + "\r\n";
	while (msg.find("\r\n") != std::string::npos)
		msg.replace(msg.find("\r\n"), 2, "\n");
	return (0);
}

int Server::run()
{
	this->s->_socket();
	std::cout << "Main server: " << this->s->getSockfd() << std::endl;
	this->s->_bind();
	this->s->_linsten(5);
	this->s->_accept();
	this->chat(this->s->getConnfd());
	close(this->s->getSockfd());
	return (0);
}

int Server::loop()
{
	struct timeval tv;
	fd_set readfds;

	tv.tv_sec = 2;
	tv.tv_usec = 500000;
	FD_ZERO(&readfds);
	for (;;) {
		this->run();
		FD_SET(this->s->getConnfd(), &readfds);
		select(this->s->getConnfd(), &readfds, NULL, NULL, &tv);
	}
}

// commands
void	Server::commandPASS(Client &client, Message &msg)
{
	if (msg.prefix.empty() && client.getNickname().empty() && client.getUsername().empty())
	{
		if (client.getRegistrationStatus())
			this->sendReply(generateErrorReply(this->_servername, ERR_ALREADYREGISTRED));
		else if (msg.params.empty())
			this->sendReply(generateErrorReply(this->_servername, ERR_NEEDMOREPARAMS, "", "PASS"));
		else if (msg.params[0] != this->_password || msg.params.size() > 1)
			this->sendReply(generateErrorReply(this->_servername, ERR_PASSWDMISMATCH, "", "PASS"));
		else
			client.setPassword(msg.params[0]);
	}
}

void	Server::commandNICK(Client &client, Message &msg)
{
	if ((msg.prefix.empty() || this->comparePrefixAndNick(msg.prefix, client)) && !client.getPassword().empty())
	{
		if (msg.params.empty())
			this->sendReply(generateErrorReply(this->_servername, ERR_NONICKNAMEGIVEN, client.getNickname(), "NICK"));
		else if (msg.params.size() != 1 || !this->validateNickname(msg.params[0]))
			this->sendReply(generateErrorReply(this->_servername, ERR_ERRONEUSNICKNAME, client.getNickname(), msg.params[0]));
		else if (this->findClient(msg.params[0], this->_connectedClients))
			this->sendReply(generateErrorReply(this->_servername, ERR_NICKNAMEINUSE, client.getNickname(), msg.params[0]));
		else
		{
			Client *tmp = this->findClient(msg.params[0], this->_clients);
			if (tmp != NULL)
				removeClient(tmp, this->_clients);
			client.setNickname(msg.params[0]);
			this->addClient(&client);
		}
	}
}

void	Server::commandUSER(Client &client, Message &msg)
{
	if (msg.prefix.empty() && !client.getPassword().empty())
	{
		if (client.getRegistrationStatus())
			this->sendReply(generateErrorReply(this->_servername, ERR_ALREADYREGISTRED, client.getNickname()));
		else if (msg.params.size() < 4)
			this->sendReply(generateErrorReply(this->_servername, ERR_NEEDMOREPARAMS, client.getNickname(), "USER"));
		else
		{
			client.setUser(msg.params);
			this->addClient(&client);
		}
	}
}

void	Server::commandOPER(Client &client, Message &msg)
{
	if (msg.prefix.empty() || this->comparePrefixAndNick(msg.prefix, client))
	{
		if (msg.params.size() < 4)
			this->sendReply(generateErrorReply(this->_servername, ERR_NEEDMOREPARAMS, client.getNickname(), "OPER"));
		else if (msg.params.size() == 2 && this->checkOperatorList(msg.params[0], msg.params[1]) == ERR_WRONGUSERNAME)
			this->sendReply(generateErrorReply(this->_servername, ERR_WRONGUSERNAME, client.getNickname(), "OPER"));
		else if (msg.params.size() == 2 && this->checkOperatorList(msg.params[0], msg.params[1]) == ERR_PASSWDMISMATCH)
			this->sendReply(generateErrorReply(this->_servername, ERR_PASSWDMISMATCH, client.getNickname(), "OPER"));
		else if (!this->checkHostnameList(client.getHostname()))
			this->sendReply(generateErrorReply(this->_servername, ERR_NOOPERHOST, client.getNickname(), "OPER"));
		else
		{
			client.setOperatorStatus();
			this->sendReply(generateNormalReply(this->_servername, RPL_YOUREOPER, client.getNickname(), "OPER"));
		}
	}
}

