#include "../include/Server.hpp"

void    Server::parseMOTD()
{
	std::ifstream	file("files/MOTD");
	std::string		str;
	while (file.is_open() && getline(file, str))
		this->_MOTD.push_back(str);
	file.close();
}

void	Server::sendMOTD(Client &client)
{
	for (std::vector<std::string>::iterator it = this->_MOTD.begin(); it != this->_MOTD.end(); it++)
		this->sendReply(client, *it);
	this->sendReply(client, "\n");
}

void	Server::sendReply(Client &client, std::string const &reply) const
{
	send(client.getClientFd(), reply.c_str(), reply.length(), 0);
}

bool	Server::validateNickname(std::string const &nick)
{
	size_t len = nick.size();
	if (!nick.empty() && len <= 9)
	{
		size_t i = 0;
		while (i < len)
		{
			if (!std::isalnum(nick[i]) && nick[i] != '-'
				&& nick[i] != '[' && nick[i] != ']'
				&& nick[i] != '{' && nick[i] != '}'
				&& nick[i] != '^' && nick[i] != '\\')
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
		this->sendMOTD(*client);
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