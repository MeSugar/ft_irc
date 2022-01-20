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

bool	Server::containsText(std::vector<std::string> &params)
{
	for (std::vector<std::string>::iterator it = params.begin(); it != params.end(); it++)
		if ((*it)[0] == ':')
			return true;
	return false;
}

bool	Server::validateMask(Client &client, const std::string &mask)
{
	size_t lastDot = mask.rfind('.');
	if (lastDot != std::string::npos)
	{
		for (size_t i = lastDot; i < mask.length(); i++)
		{
			if (mask[i] == '*')
			{
				this->sendReply(client, generateErrorReply(this->_servername, ERR_WILDTOPLEVEL, client.getNickname(), "PRIVMSG"));
				return false;
			}
		}
		return true;
	}
	this->sendReply(client, generateErrorReply(this->_servername, ERR_NOTOPLEVEL, client.getNickname(), "PRIVMSG"));
	return false;
}

bool	Server::checkUserHostnameByMask(Client const &client, const std::string &mask)
{
	std::string hostname = client.getHostname();
	std::string submask = mask.substr(1, mask.length() - 1);
	std::string::iterator it = submask.begin();
	std::string::iterator ite = hostname.begin();
	while (it != submask.end() && ite != hostname.end())
	{
		for ( ; *it == '*'; it++)
			;
		for (; ite != hostname.end() && *ite != *it; ite++)
			;
		if (it != submask.end() && ite != hostname.end() && *ite == *it)
		{
			it++;
			ite++;
		}
	}
	if (it == submask.end() && ite == hostname.end())
		return true;
	return false;

}

bool	Server::addRecipientToList(std::set<std::string> &recipients, Client &from, Client *to)
{
	if (recipients.insert(to->getNickname()).second == false)
	{
		this->sendReply(from, generateErrorReply(this->_servername, ERR_TOOMANYTARGETS, from.getNickname(), "PRIVMSG"));
		return false;
	}
	return true;
}

std::set<std::string> *Server::checkAndComposeRecipientsList(Client &client, std::vector<std::string> &params)
{
	std::vector<std::string> list;
	std::set<std::string> recipients;
	divide_comma(list, params[0]);
	for (std::vector<std::string>::iterator it = list.begin(); it != list.end(); it++)
	{
		if ((*it)[0] == '#' || (*it)[0] == '&') // recipients are some channel members or those whose hostname matches the hostmask
		{
			Channel *ch = this->find_channel(*it);
			if (ch != NULL) // adds recipients from the channel
			{
				if ((!ch->have_member(client) && ch->get_outside_status())
					|| (ch->get_moder_status() && !ch->have_operator(client.getNickname()) && !ch->have_speaker(client.getNickname())))
				{
						this->sendReply(client, generateErrorReply(this->_servername, ERR_CANNOTSENDTOCHAN, client.getNickname(), "PRIVMSG"));
						return NULL;
				}
				for (std::vector<Client *>::iterator ite = ch->get_members().begin(); ite != ch->get_members().end(); ite++)
					if (!this->addRecipientToList(recipients, client, *ite))
						return NULL;
			}
			else
			{
				int	recipientsAdded = 0;
				if ((*it)[0] == '#' && (*it).find('*') != std::string::npos)// adds recipients that match the hostname mask
				{
					if (!validateMask(client, (*it)))
						return NULL;
					for (std::vector<Client *>::iterator iter = this->_clients.begin(); iter != this->_clients.end(); iter++)
					{
						if (this->checkUserHostnameByMask(*(*iter), (*it)))
						{
							if (!this->addRecipientToList(recipients, client, *iter))
								return NULL;
							recipientsAdded++;
						}
					}
				}
				if (recipientsAdded == 0)
				{
					this->sendReply(client, generateErrorReply(this->_servername, ERR_NOSUCHNICK, client.getNickname(), "PRIVMSG"));
					return NULL;
				}
			}		
		}
		else if (this->findClient((*it), this->_connectedClients))
		{
			if (!this->addRecipientToList(recipients, client, this->findClient((*it), this->_connectedClients)))
				return NULL;
		}
		else
		{
			this->sendReply(client, generateErrorReply(this->_servername, ERR_NOSUCHNICK, client.getNickname(), "PRIVMSG"));
			return NULL;
		}
	}
	return (new std::set<std::string>(recipients));
}