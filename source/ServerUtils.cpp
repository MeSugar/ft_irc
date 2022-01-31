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

bool	Server::validateMask(Client &client, const std::string &mask, Message &msg)
{
	size_t lastDot = mask.rfind('.');
	if (lastDot != std::string::npos)
	{
		for (size_t i = lastDot; i < mask.length(); i++)
		{
			if (mask[i] == '*')
			{
				this->sendReply(client, generateErrorReply(this->_servername, ERR_WILDTOPLEVEL, client.getNickname(), msg.command, mask));
				return false;
			}
		}
		return true;
	}
	this->sendReply(client, generateErrorReply(this->_servername, ERR_NOTOPLEVEL, client.getNickname(), msg.command, mask));
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

bool	Server::addRecipientToList(std::set<std::string> &recipients, Client &from, Client *to, Message &msg)
{
	if (recipients.insert(to->getNickname()).second == false)
	{
		this->sendReply(from, generateErrorReply(this->_servername, ERR_TOOMANYTARGETS, from.getNickname(), msg.command, to->getNickname()));
		return false;
	}
	return true;
}

std::set<std::string> *Server::checkAndComposeRecipientsList(Client &client, Message &msg,
																std::vector<std::string> &params, 
																std::map<std::string, std::string> *channel_members)
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
						this->sendReply(client, generateErrorReply(this->_servername, ERR_CANNOTSENDTOCHAN, client.getNickname(), msg.command, (*it)));
						return NULL;
				}
				for (std::vector<Client *>::iterator ite = ch->get_members().begin(); ite != ch->get_members().end(); ite++)
				{
					if (!this->addRecipientToList(recipients, client, *ite, msg))
						return NULL;
					channel_members->insert(std::pair<std::string, std::string>((*ite)->getNickname(), ch->get_name()));
				}
			}
			else
			{
				int	recipientsAdded = 0;
				if ((*it)[0] == '#') // adds recipients that match the hostname mask
				{
					if (!validateMask(client, (*it), msg))
						return NULL;
					for (std::vector<Client *>::iterator iter = this->_clients.begin(); iter != this->_clients.end(); iter++)
					{
						if (this->checkUserHostnameByMask(*(*iter), (*it)))
						{
							if (!this->addRecipientToList(recipients, client, *iter, msg))
								return NULL;
							recipientsAdded++;
							channel_members->insert(std::pair<std::string, std::string>((*iter)->getNickname(), (*it)));
						}
					}
				}
				if (recipientsAdded == 0)
				{
					this->sendReply(client, generateErrorReply(this->_servername, ERR_NOSUCHNICK, client.getNickname(), msg.command, (*it)));
					return NULL;
				}
			}		
		}
		else if (this->findClient((*it), this->_connectedClients)) // tries to find the client in the list of connected users and add it to recipients list
		{
			if (!this->addRecipientToList(recipients, client, this->findClient((*it), this->_connectedClients), msg))
				return NULL;
		}
		else
		{
			this->sendReply(client, generateErrorReply(this->_servername, ERR_NOSUCHNICK, client.getNickname(), msg.command, (*it)));
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
	if (client.getMessageTimeout() < (2 << 6))
		client.setMessageTimeout(client.getMessageTimeout() * 2);
	std::stringstream ss;
	ss << client.getMessageTimeout();
	this->sendReply(client, generateErrorReply(this->_servername, ERR_FLOOD, client.getNickname(), ss.str()));
	return false;
}

const std::string&	Server::get_servername() const
{
	return (_servername);
}

Channel*	Server::add_channel(std::string name, Client& first)
{
	Channel*	channel = new Channel(this, name, &first);
	
	_channels.push_back(channel);
	return (channel);
}

void	Server::remove_channel(Channel *to_remove)
{
	for (std::vector<Channel *>::iterator it = _channels.begin(); it != _channels.end(); it++)
		if (*it == to_remove)
		{
			delete (*it);
			_channels.erase(it);
			break;
		}
}

Channel*	Server::find_channel(const std::string &name)
{
	for (std::vector<Channel *>::iterator it = _channels.begin(); it != _channels.end(); it++)
		if ((*it)->get_name() == name)
			return (*it);
	return (NULL);
}

bool	Server::check_channel_name(const std::string& str) const
{
	if (str.size() < 2 || str.size() > 200)
		return (false);
	if (str[0] != '#' && str[0] != '&')
		return (false);
	for (size_t	i = 1; i < str.size(); i++)
		if (str[i] == ' ' || str[i] == '\n' || str[i] == '\r' || str[i] == '\a' || str[i] == ',')
			return (false);
	return (true);
}

void Server::divide_comma(std::vector<std::string> &to, std::string str)
{
	size_t i;
	size_t start;

	start = 0;
	i = str.find(',');
	while (i != std::string::npos)
	{
		to.push_back(str.substr(start, i - start));
		if (i + 1 < str.size())
		{
			start = i + 1;
			i = str.find(',', start);
		}
		else
			return;
	}
	to.push_back(str.substr(start, str.size() - start));
}

static bool	is_channel_mode(char c)
{
	if (c == 'o' || c == 'p' || c == 's' || c == 'i' || c == 't'
		|| c == 'n' || c == 'm' || c == 'l' || c == 'b' || c == 'v' || c == 'k')
		return (true);
	return (false);
}

static bool	check_ban_mask(const std::string& str)
{
	size_t	i = 1;

	if (str.size() < 5)
		return (false);
	if ((i = str.find('!', i)) == std::string::npos)
		return (false);
	if ((i = str.find('@', i + 2)) == std::string::npos)
		return (false);
	if (i + 1 >= str.size())
		return (false);
	return (true);
}

static bool	check_number(const std::string& str)
{
	if (str.empty())
		return (false);
	for (size_t i = 0; i < str.size(); i++)
		if (!isdigit(str[i]))
			return (false);
	double tmp = strtod(str.c_str(), NULL);
	if (errno == ERANGE)
		return (false);
	if (tmp > INT_MAX)
		return (false);
	return (true);
}

bool	Server::check_channel_modes(const std::string& str, const Message& msg, Client& client)
{
	if (str.size() < 2 || str.size() > 11)
		return (false);
	if (str[0] != '+' && str[0] != '-')
	{	
		sendReply(client, generateErrorReply(_servername, ERR_UNKNOWNMODE, client.getNickname(), std::string(1, str[0])));
		return (false);
	}
	int	param_modes = 3;
	for (size_t i = 1; i < str.size(); i++)
	{
		if (!is_channel_mode(str[i]))
		{
			sendReply(client, generateErrorReply(_servername, ERR_UNKNOWNMODE, client.getNickname(), std::string(1, str[i])));
			return (false);
		}
		if (str[i] == 'o' || str[i] == 'b' || (str[i] == 'l' && str[0] == '+') || str[i] == 'v'
			|| (str[i] == 'k' && str[0] == '+'))
			param_modes--;
		if (param_modes < 0)
			return (false);
		if (str[i] != 'o' && str[i] != 'b' && str[i] != 'v' && str.find(str[i], i + 1) != std::string::npos)
			return (false);
	}
	if (msg.params.size() < static_cast<size_t>(2 + 3 - param_modes))
	{	
		sendReply(client, generateErrorReply(_servername, ERR_NEEDMOREPARAMS, client.getNickname(), msg.command));
		return (false);
	}
	if (msg.params.size() > static_cast<size_t>(2 + 3 - param_modes))
		return (false);
	param_modes = 2;
	for (size_t i = 1; i < str.size(); i++)
	{
		if (str[i] == 'o' || str[i] == 'v')
		{
			if (!validateNickname(msg.params[param_modes]))
				return (false);
			param_modes++;
		}
		else if (str[i] == 'b')
		{
			if (!check_ban_mask(msg.params[param_modes]))
				return (false);
			param_modes++;
		}
		else if (str[i] == 'l' && str[0] == '+')
		{
			if (!check_number(msg.params[param_modes]))
				return (false);
			param_modes++;
		}
		else if (str[i] == 'k' && str[0] == '+')
			param_modes++;
	}
	return (true);
}

void	Server::handle_channel_mode(char sign, char mode, Channel* channel, std::string param, Client& client)
{
	if (mode == 'o')
	{
		if (sign == '+')
		{
			if (channel->have_member(param) && !channel->have_operator(param))
				channel->add_operator(param);
			else if (channel->have_member(param) && channel->have_operator(param))
				return;
			else
			{	
				sendReply(client, generateErrorReply(_servername, ERR_NOSUCHNICK, client.getNickname(), param));
				return;
			}
		}
		else if (sign == '-')
		{
			if (channel->have_operator(param))
				channel->remove_operator(param);
			else
			{	
				sendReply(client, generateErrorReply(_servername, ERR_NOSUCHNICK, client.getNickname(), param));
				return;
			}
		}
	}
	else if (mode == 'p')
	{
		if (sign == '+' && !(channel->get_private_status()))
		{	
			channel->set_private_status(true);
			if (channel->get_secret_status())
				channel->set_secret_status(false);
		}
		else if (sign == '-' && channel->get_private_status())
			channel->set_private_status(false);
		else
			return;
	}
	else if (mode == 's')
	{
		if (sign == '+' && !(channel->get_secret_status()))
		{	
			channel->set_secret_status(true);
			if (channel->get_private_status())
				channel->set_private_status(false);
		}
		else if (sign == '-' && channel->get_secret_status())
			channel->set_secret_status(false);
		else
			return;
	}
	else if (mode == 'i')
	{
		if (sign == '+' && !(channel->get_invite_status()))
			channel->set_invite_status(true);
		else if (sign == '-' && channel->get_invite_status())
			channel->set_invite_status(false);
		else
			return;
	}
	else if (mode == 't')
	{
		if (sign == '+' && !(channel->get_topic_status()))
			channel->set_topic_status(true);
		else if (sign == '-' && channel->get_topic_status())
			channel->set_topic_status(false);
		else
			return;
	}
	else if (mode == 'n')
	{
		if (sign == '+' && !(channel->get_outside_status()))
			channel->set_outside_status(true);
		else if (sign == '-' && channel->get_outside_status())
			channel->set_outside_status(false);
		else
			return;
	}
	else if (mode == 'm')
	{
		if (sign == '+' && !(channel->get_moder_status()))
			channel->set_moder_status(true);
		else if (sign == '-' && channel->get_moder_status())
			channel->set_moder_status(false);
		else
			return;
	}
	else if (mode == 'm')
	{
		if (sign == '+' && !(channel->get_moder_status()))
			channel->set_moder_status(true);
		else if (sign == '-' && channel->get_moder_status())
			channel->set_moder_status(false);
		else
			return;
	}
	else if (mode == 'l')
	{	
		if ((sign == '-' && channel->get_user_limit() != MAX_MEMBERS)
			|| (sign == '+' && channel->get_user_limit() != static_cast<size_t>(atoi(param.c_str()))))
			channel->set_user_limit(param, sign);
		else
			return;
	}
	else if (mode == 'b')
	{
		if (sign == '+' && !(channel->have_banmask(param)))
			channel->add_banmask(param);
		else if (sign == '-' && channel->have_banmask(param))
			channel->remove_banmask(param);
		else
			return;
	}
	else if (mode == 'v')
	{
		if (!(channel->have_member(param)))
		{
			sendReply(client, generateErrorReply(_servername, ERR_NOSUCHNICK, client.getNickname(), param));
			return;
		}
		else if (sign == '+' && !(channel->have_speaker(param)) && !(channel->have_operator(param)))
			channel->add_speaker(param);
		else if (sign == '-' && channel->have_speaker(param))
			channel->remove_speaker(param);
		else
			return;
	}
	else if (mode == 'k')
	{
		if (sign == '+' && channel->have_key())
		{	
			sendReply(client, generateErrorReply(_servername, ERR_KEYSET, client.getNickname(), param));
			return;
		}
		else if (sign == '+')
			channel->set_key(param);
		else if (sign == '-' && channel->have_key())
			channel->remove_key();
		else
			return;
	}
	std::string tmp;
	tmp = client.get_full_name() + " MODE " + channel->get_name() + ' '
			+ sign + mode;
	if (!(param.empty()))
		tmp += " :";
	tmp += param + "\r\n";
	channel->send_message(tmp);
	if (channel->operators_empty())
		channel->make_any_operator();
}

void	Server::channel_mods_rpl(Channel* channel, Client& client)
{
	(void)client;
	std::string	tmp;
	std::stringstream ss;

	if (channel->get_invite_status() || channel->get_private_status() || channel->get_secret_status()
		|| channel->get_topic_status() || channel->get_outside_status() || channel->get_moder_status()
		|| channel->get_user_limit() != MAX_MEMBERS)
		tmp += '+';
	if (channel->get_invite_status())
		tmp += 'i';
	if (channel->get_private_status())
		tmp += 'p';
	if (channel->get_secret_status())
		tmp += 's';
	if (channel->get_topic_status())
		tmp += 't';
	if (channel->get_outside_status())
		tmp += 'n';
	if (channel->get_moder_status())
		tmp += 'm';
	if (channel->get_user_limit() != MAX_MEMBERS)
	{	
		tmp += 'i';
		ss << channel->get_user_limit();
	}
	sendReply(client, generateNormalReply(_servername, RPL_CHANNELMODEIS, client.getNickname(), channel->get_name(), tmp, ss.str()));
	if (channel->have_key())
	{	
		tmp = "+k";
		sendReply(client, generateNormalReply(_servername, RPL_CHANNELMODEIS, client.getNickname(), channel->get_name(), tmp, channel->get_key()));
	}
}

void	Server::banlist_rpl(Channel* channel, Client& client)
{
	(void)client;
	const std::vector<std::string> bans = channel->get_banlist();

	for (std::vector<std::string>::const_iterator it = bans.begin(); it != bans.end(); it++)
		sendReply(client, generateNormalReply(_servername, RPL_BANLIST, client.getNickname(), channel->get_name(), *it));
	sendReply(client, generateNormalReply(_servername, RPL_ENDOFBANLIST, client.getNickname(), channel->get_name()));
}

void	Server::channel_mode(Client &client, Message &msg)
{
	Channel* channel = find_channel(msg.params[0]);
	
	if (!channel)
	{	
		sendReply(client, generateErrorReply(_servername, ERR_NOSUCHCHANNEL, client.getNickname(), msg.params[0]));
		return;
	}
	if (!channel->have_member(client))
	{
		sendReply(client, generateErrorReply(_servername, ERR_NOTONCHANNEL, client.getNickname(), msg.params[0]));
		return;
	}
	if (msg.params.size() == 1)
	{
		channel_mods_rpl(channel, client);
		return;
	}
	if (msg.params.size() == 2 && (msg.params[1] == "+b" || msg.params[1] == "-b" || msg.params[1] == "b"))
	{
		banlist_rpl(channel, client);
		return;
	}
	if (!channel->have_operator(client))
	{
		sendReply(client, generateErrorReply(_servername, ERR_CHANOPRIVSNEEDED, client.getNickname(), msg.params[0]));
		return;
	}
	std::string	mod = msg.params[1];
	if (!check_channel_modes(mod, msg, client))
		return;
	size_t	param = 2;
	for (size_t i = 1; i < mod.size(); i++)
	{
		if (mod[i] == 'o' || mod[i] == 'b' || (mod[i] == 'l' && mod[0] == '+') || mod[i] == 'v'
			|| (mod[i] == 'k' && mod[0] == '+'))
		{	
			handle_channel_mode(mod[0], mod[i], channel, msg.params[param], client);
			param++;
		}
		else
			handle_channel_mode(mod[0], mod[i], channel, std::string(), client);
	}
}

bool	Server::check_user_modes(const std::string& mod, Client& client)
{
	if (mod.size() < 2 || mod.size() > 5)
		return (false);
	if (mod[0] != '+' && mod[0] != '-')
	{
		sendReply(client, generateErrorReply(_servername, ERR_UMODEUNKNOWNFLAG, client.getNickname()));	
		return (false);
	}
	for (size_t i = 1; i < mod.size(); i++)
	{
		if (mod[i] != 'i' && mod[i] != 'w' && mod[i] != 's' && mod[i] != 'o')
		{
			sendReply(client, generateErrorReply(_servername, ERR_UMODEUNKNOWNFLAG, client.getNickname()));
			return (false);
		}
		if (mod.find(mod[i], i + 1) != std::string::npos)
			return (false);
	}
	return (true);
}

void	Server::handle_user_mode(char sign, char mode, Client& client)
{
	if (mode == 'i')
	{
		if (sign == '+' && !(client.get_invisible()))
			client.set_invisible(true);
		else if (sign == '-' && client.get_invisible())
			client.set_invisible(false);
		else
			return;
	}
	else if (mode == 's')
	{
		if (sign == '+' && !(client.get_receive_notices()))
			client.set_receive_notices(true);
		else if (sign == '-' && client.get_receive_notices())
			client.set_receive_notices(false);
		else
			return;
	}
	else if (mode == 'w')
	{
		if (sign == '+' && !(client.get_receive_wallops()))
			client.set_receive_wallops(true);
		else if (sign == '-' && client.get_receive_wallops())
			client.set_receive_wallops(false);
		else
			return;
	}
	else if (mode == 'o')
	{	
		if (sign == '-' && client.get_operator_status())
			client.set_operator_status(false);
		else
			return;
	}
	std::string tmp;
	tmp = client.get_full_name() + " MODE " + client.getNickname() + " :"
			+ sign + mode + "\r\n";
	sendReply(client, tmp);
}

void	Server::user_mods_rpl(Client& client)
{
	std::string	tmp;

	if (client.get_invisible() || client.get_receive_notices() || client.get_receive_wallops()
		|| client.get_operator_status())
		tmp += '+';
	if (client.get_invisible())
		tmp += 'i';
	if (client.get_receive_notices())
		tmp += 's';
	if (client.get_receive_wallops())
		tmp += 'w';
	if (client.get_operator_status())
		tmp += 'o';
	sendReply(client, generateNormalReply(_servername, RPL_UMODEIS, client.getNickname(), tmp));
}

void	Server::user_mode(Client &client, Message &msg)
{
	if (!findClient(msg.params[0], _connectedClients))
	{
		sendReply(client, generateErrorReply(_servername, ERR_NOSUCHNICK, client.getNickname(), msg.params[0]));
		return;
	}
	if (client.getNickname() != msg.params[0])
	{	
		sendReply(client, generateErrorReply(_servername, ERR_USERSDONTMATCH, client.getNickname()));
		return;
	}
	if (msg.params.size() == 1)
	{
		user_mods_rpl(client);
		return;
	}
	if (msg.params.size() > 2)
		return;
	std::string	mod = msg.params[1];
	if (!check_user_modes(mod, client))
		return;
	for (size_t i = 1; i < mod.size(); i++)
		handle_user_mode(mod[0], mod[i], client);
}

void	Server::all_names_rpl(Client &client)
{
	for (std::vector<Channel *>::iterator it = _channels.begin(); it != _channels.end(); it++)
	{
		if (!((*it)->have_member(client)) && ((*it)->get_private_status() || (*it)->get_secret_status()))
			continue;
		std::string tmp = (*it)->list_all_members(client);
		if (!(tmp.empty()))
			sendReply(client, generateNormalReply(_servername, RPL_NAMREPLY, client.getNickname(), (*it)->get_name(), tmp));
	}
	std::string tmp;
	for (std::vector<Client *>::iterator it = _connectedClients.begin(); it != _connectedClients.end(); it++)
	{
		if (!((*it)->getRegistrationStatus()) || (*it)->get_invisible())
			continue;
		if ((*it)->check_names_visibility(client))
		{
			tmp += (*it)->getNickname();
			if ((it + 1) != _connectedClients.end())
				tmp += ' ';
		}
	}
	if (!(tmp.empty()))
			sendReply(client, generateNormalReply(_servername, RPL_NAMREPLY, client.getNickname(), "*", tmp));
	sendReply(client, generateNormalReply(_servername, RPL_ENDOFNAMES, client.getNickname(), "*"));
}

void	Server::all_list_rpl(Client& client)
{
	sendReply(client, generateNormalReply(_servername, RPL_LISTSTART, client.getNickname()));
	for (std::vector<Channel *>::iterator it = _channels.begin(); it != _channels.end(); it++)
	{
		std::string			name;
		std::stringstream	ss;
		std::string			topic;
		if (!((*it)->have_member(client)) && (*it)->get_secret_status())
			continue;
		if (!((*it)->have_member(client)) && (*it)->get_private_status())
			name = "Prv";
		else
		{	
			name = (*it)->get_name();
			topic = (*it)->get_topic();
		}
		ss << (*it)->size();
		sendReply(client, generateNormalReply(_servername, RPL_LIST, client.getNickname(), name, ss.str(), topic));
	}
	sendReply(client, generateNormalReply(_servername, RPL_LISTEND, client.getNickname()));
}