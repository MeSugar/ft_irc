#include "../include/Server.hpp"

void    Server::parseMOTD()
{
	std::ifstream	file("files/MOTD");
	std::string		str;
	while (file.is_open() && getline(file, str))
		this->_MOTD.push_back(str);
	file.close();
}

void	Server::printLog(Message &msg) const
{
	std::cout << "------------------------------------" << std::endl;
	std::cout << "message: " << this->_message;
	std::cout << "prefix: " << msg.prefix << std::endl;
	std::cout << "command: " << msg.command << std::endl;
	size_t param_num = 1;
	for (std::vector<std::string>::iterator it = msg.params.begin(); it != msg.params.end(); it++)
	{
		std::cout << "parameter[" << param_num << "]: " << *it << std::endl;
		param_num++;
	}
}

void	Server::sendMOTD(Client &client)
{
	if (this->_MOTD.empty())
	{
		this->sendReply(client, generateErrorReply(this->_servername, ERR_NOMOTD, client.getNickname()));
		return ;
	}
	this->sendReply(client, generateNormalReply(this->_servername, RPL_MOTDSTART, client.getNickname(), this->_servername));
	for (std::vector<std::string>::iterator it = this->_MOTD.begin(); it != this->_MOTD.end(); it++)
		this->sendReply(client, generateNormalReply(this->_servername, RPL_MOTD, client.getNickname(), *it));
	this->sendReply(client, generateNormalReply(this->_servername, RPL_ENDOFMOTD, client.getNickname()));
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
		// this->_clients.push_back(client);
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
				this->sendReply(client, generateErrorReply(this->_servername, ERR_WILDTOPLEVEL, client.getNickname(), "PRIVMSG", mask));
				return false;
			}
		}
		return true;
	}
	this->sendReply(client, generateErrorReply(this->_servername, ERR_NOTOPLEVEL, client.getNickname(), "PRIVMSG", mask));
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
		this->sendReply(from, generateErrorReply(this->_servername, ERR_TOOMANYTARGETS, from.getNickname(), "PRIVMSG", to->getNickname()));
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
						this->sendReply(client, generateErrorReply(this->_servername, ERR_CANNOTSENDTOCHAN, client.getNickname(), "PRIVMSG", (*it)));
						return NULL;
				}
				for (std::vector<Client *>::iterator ite = ch->get_members().begin(); ite != ch->get_members().end(); ite++)
					if (!this->addRecipientToList(recipients, client, *ite))
						return NULL;
			}
			else
			{
				int	recipientsAdded = 0;
				if ((*it)[0] == '#') // adds recipients that match the hostname mask
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
					this->sendReply(client, generateErrorReply(this->_servername, ERR_NOSUCHNICK, client.getNickname(), "PRIVMSG", (*it)));
					return NULL;
				}
			}		
		}
		else if (this->findClient((*it), this->_connectedClients)) // tries to find the client in the list of connected users and add it to recipients list
		{
			if (!this->addRecipientToList(recipients, client, this->findClient((*it), this->_connectedClients)))
				return NULL;
		}
		else
		{
			this->sendReply(client, generateErrorReply(this->_servername, ERR_NOSUCHNICK, client.getNickname(), "PRIVMSG", (*it)));
			return NULL;
		}
	}
	return (new std::set<std::string>(recipients));
}

bool	Server::floodCheck(Client &client)
{
	time_t t = time(0);
	if (t - client.getLastMessageTime() > client.getMessageTimeout())
	{
		client.setLastMessageTime(t);
		client.setMessageTimeout(2);
		return true;
	}
	client.setLastMessageTime(t);
	client.setMessageTimeout(client.getMessageTimeout() * 2);
	std::stringstream ss;
	ss << client.getMessageTimeout();
	this->sendReply(client, generateErrorReply(this->_servername, ERR_FLOOD, client.getNickname(), ss.str()));
	return false;
}
