#include "../include/Server.hpp"

void	Server::commandHandler(Client &client, Message &msg)
{
		this->printLog(msg);
		if (this->_commands.find(msg.command) != this->_commands.end())
			this->commandProcessor(client, msg);
		else if (!msg.command.empty())
			this->sendReply(client, generateErrorReply(this->_servername, ERR_UNKNOWNCOMMAND, client.getNickname(), msg.command));
}

void	Server::commandProcessor(Client &client, Message &msg)
{
	if (!client.getRegistrationStatus() && msg.command != "PASS" && msg.command != "NICK" && msg.command != "USER" && msg.command != "QUIT")
		this->sendReply(client, generateErrorReply(this->_servername, ERR_NOTREGISTERED, client.getNickname(), msg.command));
	else
	{
		if (client.getRegistrationStatus() && !this->floodCheck(client))
			return ;
		CommandsMap::iterator it = this->_commands.find(msg.command);
		(this->*it->second)(client, msg);
	}
}

void	Server::commandPASS(Client &client, Message &msg)
{
	time_t t = time(0);
	client.setLastMessageTime(t);
	if (msg.prefix.empty() && client.getNickname().empty() && client.getUsername().empty())
	{
		if (client.getRegistrationStatus())
			this->sendReply(client, generateErrorReply(this->_servername, ERR_ALREADYREGISTRED));
		else if (msg.params.empty())
			this->sendReply(client, generateErrorReply(this->_servername, ERR_NEEDMOREPARAMS, "", "PASS"));
		else if (msg.params[0] != this->_password || msg.params.size() > 1)
			this->sendReply(client, generateErrorReply(this->_servername, ERR_PASSWDMISMATCH, "", "PASS"));
		else
			client.setPassword(msg.params[0]);
	}
}

void	Server::commandNICK(Client &client, Message &msg)
{
	time_t t = time(0);
	client.setLastMessageTime(t);
	if ((msg.prefix.empty() || this->comparePrefixAndNick(msg.prefix, client)) && !client.getPassword().empty())
	{
		if (msg.params.empty())
			this->sendReply(client, generateErrorReply(this->_servername, ERR_NONICKNAMEGIVEN, client.getNickname(), "NICK"));
		else if (msg.params.size() != 1 || !this->validateNickname(msg.params[0]))
			this->sendReply(client, generateErrorReply(this->_servername, ERR_ERRONEUSNICKNAME, client.getNickname(), msg.params[0]));
		else if (this->findClient(msg.params[0], this->_connectedClients))
			this->sendReply(client, generateErrorReply(this->_servername, ERR_NICKNAMEINUSE, client.getNickname(), msg.params[0]));
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
	time_t t = time(0);
	client.setLastMessageTime(t);
	if (msg.prefix.empty() && !client.getPassword().empty())
	{
		if (client.getRegistrationStatus())
			this->sendReply(client, generateErrorReply(this->_servername, ERR_ALREADYREGISTRED, client.getNickname()));
		else if (msg.params.size() < 4)
			this->sendReply(client, generateErrorReply(this->_servername, ERR_NEEDMOREPARAMS, client.getNickname(), "USER"));
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
		if (msg.params.size() < 2)
			this->sendReply(client, generateErrorReply(this->_servername, ERR_NEEDMOREPARAMS, client.getNickname(), "OPER"));
		else if (msg.params.size() == 2 && this->checkOperatorList(msg.params[0], msg.params[1]) == ERR_WRONGUSERNAME)
			this->sendReply(client, generateErrorReply(this->_servername, ERR_WRONGUSERNAME, client.getNickname(), "OPER"));
		else if (msg.params.size() == 2 && this->checkOperatorList(msg.params[0], msg.params[1]) == ERR_PASSWDMISMATCH)
			this->sendReply(client, generateErrorReply(this->_servername, ERR_PASSWDMISMATCH, client.getNickname(), "OPER"));
		else if (!this->checkHostnameList(client.getHostname()))
			this->sendReply(client, generateErrorReply(this->_servername, ERR_NOOPERHOST, client.getNickname(), "OPER"));
		else
		{
			client.set_operator_status(true);
			this->sendReply(client, generateNormalReply(this->_servername, RPL_YOUREOPER, client.getNickname(), "OPER"));
		}
	}
}

void	Server::commandPRIVMSG(Client &client, Message &msg)
{
	if (msg.prefix.empty() || this->comparePrefixAndNick(msg.prefix, client))
	{
		if (msg.params.empty() || msg.params[0][0] == ':')
			this->sendReply(client, generateErrorReply(this->_servername, ERR_NORECIPIENT, client.getNickname(), msg.command));
		else if (msg.params.size() < 2 && !this->containsText(msg.params))
			this->sendReply(client, generateErrorReply(this->_servername, ERR_NOTEXTTOSEND, client.getNickname(), msg.command));
		else
		{
			std::map<std::string, std::string> channel_members;
			std::set<std::string> *recipients = this->checkAndComposeRecipientsList(client, msg, msg.params, &channel_members);
			if (recipients != NULL && !recipients->empty())
			{
				for (std::set<std::string>::iterator it = recipients->begin(); it != recipients->end(); it++)
				{
					Client &cl = *this->findClient((*it), this->_connectedClients);
					if (cl.getAwayStatus() && msg.command != "NOTICE")
						sendReply(client, generateNormalReply(this->_servername, RPL_AWAY, client.getNickname(), msg.command, cl.getNickname(), cl.getAwayMessage()));
					else if (client.getNickname() != cl.getNickname())
					{
						if (channel_members.find(cl.getNickname()) != channel_members.end())
							sendReply(cl, client.get_full_name() + " " + msg.command
							+ " " + channel_members.find(cl.getNickname())->second + " " + ":" + msg.params[1] + "\n");
						else
							sendReply(cl, client.get_full_name() + " " + msg.command + " "
							+ client.getNickname() + " " + ":" + msg.params[1] + "\n");
					}
				}
				delete recipients;
			}
		}
	}
}

void	Server::commandNOTICE(Client &client, Message &msg)
{
	this->commandPRIVMSG(client, msg);
}

void	Server::commandAWAY(Client &client, Message &msg)
{
	if (!msg.params.empty())
	{
		this->sendReply(client, generateNormalReply(this->_servername, RPL_NOWAWAY, client.getNickname(), "AWAY"));
		client.setAwayStatus(msg.params[0]);
	}
	else
	{
		this->sendReply(client, generateNormalReply(this->_servername, RPL_UNAWAY, client.getNickname(), "AWAY"));
		client.setAwayStatus();
	}
}

void	Server::commandQUIT(Client &client, Message &msg)
{
	(void)msg;
	Message partMsg = Message();
	partMsg.command = "PART";
	std::string params;
	std::vector<Channel *> channels = client.getChannel();
	for (size_t i = 0; i < channels.size(); i++)
		params.append(channels[i]->get_name() + ",");
	partMsg.params.push_back(params);
	if (!partMsg.params[0].empty())
		this->commandPART(client, partMsg);
	this->_deletepoll(client.getClientFd());
}

void Server::commandJOIN(Client& client, Message& msg)
{
	if ((!msg.prefix.empty() && !comparePrefixAndNick(msg.prefix, client)) || !client.getRegistrationStatus())
		return ;
	if (msg.params.size() > 2)
		return ;
	if (msg.params.empty())
	{	
		sendReply(client, generateErrorReply(_servername, ERR_NEEDMOREPARAMS, client.getNickname(), msg.command));
		return ;
	}
	std::vector<std::string>	channels;
	std::vector<std::string>	keys;
	divide_comma(channels, msg.params[0]);
	if (msg.params.size() == 2)
		divide_comma(keys, msg.params[1]);
	Channel*	channel;
	size_t		j = 0;
	for (std::vector<std::string>::iterator it = channels.begin(); it != channels.end(); it++, j++)
	{
		if ((channel = find_channel(*it)))
		{
			if (channel->have_member(client))
				continue;
			if (!client.under_channels_limit())
			{
				sendReply(client, generateErrorReply(_servername, ERR_TOOMANYCHANNELS, client.getNickname(), *it));
				continue;
			}
			if (channel->get_invite_status() && !client.check_invitation(*it))
			{
				sendReply(client, generateErrorReply(_servername, ERR_INVITEONLYCHAN, client.getNickname(), *it));
				continue;
			}
			if (channel->check_bans(client))
			{
				sendReply(client, generateErrorReply(_servername, ERR_BANNEDFROMCHAN, client.getNickname(), *it));
				continue;
			}
			if (channel->have_key() && (keys.size() <= j || !(channel->check_key(keys[j]))))
			{	
				sendReply(client, generateErrorReply(_servername, ERR_BADCHANNELKEY, client.getNickname(), *it));
				continue;
			}
			if (!(channel->under_user_limit()))
			{
				sendReply(client, generateErrorReply(_servername, ERR_CHANNELISFULL, client.getNickname(), *it));
				continue;
			}
			channel->add_member(&client);
		}
		else
		{
			if (!check_channel_name(*it))
			{	
				sendReply(client, generateErrorReply(_servername, ERR_NOSUCHCHANNEL, client.getNickname(), *it));
				continue;
			}
			if (!client.under_channels_limit())
			{
				sendReply(client, generateErrorReply(_servername, ERR_TOOMANYCHANNELS, client.getNickname(), *it));
				continue;
			}
			channel = add_channel(*it, client);
		}
		std::string	tmp;
		tmp = client.get_full_name() + " JOIN :" + *it + "\r\n";
		channel->send_message(tmp);
		Message  mes;
		mes.params.push_back(*it);
		if (!(channel->get_topic().empty()))
			sendReply(client, generateNormalReply(_servername, RPL_TOPIC, client.getNickname(), *it, channel->get_topic()));
		else
			sendReply(client, generateNormalReply(_servername, RPL_NOTOPIC, client.getNickname(), *it));
		commandNAMES(client, mes);
	}
}

void	Server::commandPART(Client &client, Message &msg)
{
	if ((!msg.prefix.empty() && !comparePrefixAndNick(msg.prefix, client)) || !client.getRegistrationStatus())
		return ;
	if (msg.params.size() > 2)
		return ;
	if (msg.params.empty())
	{	
		sendReply(client, generateErrorReply(_servername, ERR_NEEDMOREPARAMS, client.getNickname(), msg.command));
		return ;
	}
	std::vector<std::string>	channels;
	divide_comma(channels, msg.params[0]);
	Channel*	channel;
	for (std::vector<std::string>::iterator it = channels.begin(); it != channels.end(); it++)
	{
		if ((channel = find_channel(*it)))
		{
			if (!(channel->have_member(client)))
			{	
				sendReply(client, generateErrorReply(_servername, ERR_NOTONCHANNEL, client.getNickname(), *it));
				continue;
			}
			std::string	tmp;
			tmp = client.get_full_name() + " PART " + *it;
			if (msg.params.size() == 2)
				tmp += " :" + msg.params[1];
			tmp += "\r\n";
			channel->send_message(tmp);
			channel->remove_member(&client);
			client.remove_channel(channel);
			if (channel->empty())
				remove_channel(channel);
			else if (channel->operators_empty())
				channel->make_any_operator();
		}
		else
		{	
			sendReply(client, generateErrorReply(_servername, ERR_NOSUCHCHANNEL, client.getNickname(), *it));
			continue;
		}
	}
}

void	Server::commandMODE(Client &client, Message &msg)
{
	if ((!msg.prefix.empty() && !comparePrefixAndNick(msg.prefix, client)) || !client.getRegistrationStatus())
		return;
	if (msg.params.empty())
	{	
		sendReply(client, generateErrorReply(_servername, ERR_NEEDMOREPARAMS, client.getNickname(), msg.command));
		return;
	}
	if (msg.params[0].empty())
	{
		sendReply(client, generateErrorReply(_servername, ERR_NOSUCHNICK, client.getNickname(), msg.params[0]));
		return;
	}
	if (check_channel_name(msg.params[0]))
		channel_mode(client, msg);
	else
		user_mode(client, msg);
}

void	Server::commandTOPIC(Client &client, Message &msg)
{
	if ((!msg.prefix.empty() && !comparePrefixAndNick(msg.prefix, client)) || !client.getRegistrationStatus())
		return;
	if (msg.params.empty() || msg.params[0].empty())
	{	
		sendReply(client, generateErrorReply(_servername, ERR_NEEDMOREPARAMS, client.getNickname(), msg.command));
		return;
	}
	if (msg.params.size() > 2)
		return;
	Channel*	channel = find_channel(msg.params[0]);
	if (!channel || !(channel->have_member(client)))
	{
		sendReply(client, generateErrorReply(_servername, ERR_NOTONCHANNEL, client.getNickname(), msg.params[0]));
		return;
	}
	if (msg.params.size() == 1)
	{
		if (channel->get_topic().empty())
			sendReply(client, generateNormalReply(_servername, RPL_NOTOPIC, client.getNickname(), msg.params[0]));
		else
			sendReply(client, generateNormalReply(_servername, RPL_TOPIC, client.getNickname(), msg.params[0], channel->get_topic()));
	}
	else
	{
		if (channel->get_topic_status() && !(channel->have_operator(client)))
			sendReply(client, generateErrorReply(_servername, ERR_CHANOPRIVSNEEDED, client.getNickname(), msg.params[0]));
		else
		{	
			channel->set_topic(msg.params[1]);
			std::string tmp;
			tmp = client.get_full_name() + " TOPIC " + msg.params[0] + " :"
					+ msg.params[1] + "\r\n";
			channel->send_message(tmp);
		}
	}
}

void	Server::commandNAMES(Client &client, Message &msg)
{
	if ((!msg.prefix.empty() && !comparePrefixAndNick(msg.prefix, client)) || !client.getRegistrationStatus())
		return;
	if (msg.params.size() > 1)
		return;
	if (msg.params.empty() || msg.params[0].empty())
	{	
		all_names_rpl(client);
		return;
	}
	std::vector<std::string>	channels;
	divide_comma(channels, msg.params[0]);
	Channel*	channel;
	for (std::vector<std::string>::iterator it = channels.begin(); it != channels.end(); it++)
	{
		if (!(channel = find_channel(*it)))
			continue;
		if (!(channel->have_member(client)) && (channel->get_private_status() || channel->get_secret_status()))
			continue;
		std::string tmp = channel->list_all_members(client);
		if (!(tmp.empty()))
			sendReply(client, generateNormalReply(_servername, RPL_NAMREPLY, client.getNickname(), *it, tmp));
	}
	sendReply(client, generateNormalReply(_servername, RPL_ENDOFNAMES, client.getNickname(), channels.back()));
}

void	Server::commandLIST(Client &client, Message &msg)
{
	if ((!msg.prefix.empty() && !comparePrefixAndNick(msg.prefix, client)) || !client.getRegistrationStatus())
		return;
	if (msg.params.size() > 2)
		return;
	if (msg.params.size() == 2 && msg.params[1] != _servername)
	{	
		sendReply(client, generateErrorReply(_servername, ERR_NOSUCHSERVER, client.getNickname(), msg.params[1]));
		return;
	}
	if (msg.params.empty())
	{	
		all_list_rpl(client);
		return;
	}
	std::vector<std::string>	channels;
	divide_comma(channels, msg.params[0]);
	Channel*	channel;
	sendReply(client, generateNormalReply(_servername, RPL_LISTSTART, client.getNickname()));
	for (std::vector<std::string>::iterator it = channels.begin(); it != channels.end(); it++)
	{
		std::string			name;
		std::stringstream	ss;
		std::string			topic;
		if (!(channel = find_channel(*it)))
			continue;
		if (!(channel->have_member(client)) && channel->get_secret_status())
			continue;
		if (!(channel->have_member(client)) && channel->get_private_status())
			name = "Prv";
		else
		{	
			name = channel->get_name();
			topic = channel->get_topic();
		}
		ss << channel->size();
		sendReply(client, generateNormalReply(_servername, RPL_LIST, client.getNickname(), name, ss.str(), topic));
	}
	sendReply(client, generateNormalReply(_servername, RPL_LISTEND, client.getNickname()));
}

void	Server::commandINVITE(Client &client, Message &msg)
{
	if ((!msg.prefix.empty() && !comparePrefixAndNick(msg.prefix, client)) || !client.getRegistrationStatus())
		return;
	if (msg.params.size() > 2)
		return;
	if (msg.params.size() < 2)
	{	
		sendReply(client, generateErrorReply(_servername, ERR_NEEDMOREPARAMS, client.getNickname(), msg.command));
		return;
	}
	Client*	target;
	if (!(target = findClient(msg.params[0], _connectedClients)))
	{
		sendReply(client, generateErrorReply(_servername, ERR_NOSUCHNICK, client.getNickname(), msg.params[0]));
		return;
	}
	if (target == &client)
		return;
	Channel* channel = find_channel(msg.params[1]);
	if (channel && !(channel->have_member(client)))
	{
		sendReply(client, generateErrorReply(_servername, ERR_NOTONCHANNEL, client.getNickname(), msg.params[1]));
		return;
	}
	if (channel && channel->have_member(*target))
	{
		sendReply(client, generateErrorReply(_servername, ERR_USERONCHANNEL, client.getNickname(), msg.params[0], msg.params[1]));
		return;
	}
	if (channel && channel->get_invite_status() && !(channel->have_operator(client)))
	{
		sendReply(client, generateErrorReply(_servername, ERR_CHANOPRIVSNEEDED, client.getNickname(), msg.params[1]));
		return;
	}
	if (target->getAwayStatus())
	{
		sendReply(client, generateNormalReply(_servername, RPL_AWAY, client.getNickname(), msg.params[0], target->getAwayMessage()));
		return;
	}
	target->add_invite(msg.params[1]);
	sendReply(client, generateNormalReply(_servername, RPL_INVITING, client.getNickname(), msg.params[0], msg.params[1]));
	std::string tmp;
	tmp = client.get_full_name() + " INVITE " + msg.params[0] + ' '
			+ msg.params[1] + "\r\n";
	sendReply(*target, tmp); //to target
}

void	Server::commandKICK(Client &client, Message &msg)
{
	if ((!msg.prefix.empty() && !comparePrefixAndNick(msg.prefix, client)) || !client.getRegistrationStatus())
		return;
	if (msg.params.size() > 3)
		return;
	if (msg.params.size() < 2)
	{	
		sendReply(client, generateErrorReply(_servername, ERR_NEEDMOREPARAMS, client.getNickname(), msg.command));
		return;
	}
	Channel* channel = find_channel(msg.params[0]);
	if (!channel)
	{
		sendReply(client, generateErrorReply(_servername, ERR_NOSUCHCHANNEL, client.getNickname(), msg.params[0]));
		return;
	}
	if (!(channel->have_operator(client)))
	{
		sendReply(client, generateErrorReply(_servername, ERR_CHANOPRIVSNEEDED, client.getNickname(), msg.params[0]));
		return;
	}
	Client*	target;
	if (!(target = findClient(msg.params[1], _connectedClients)))
	{
		sendReply(client, generateErrorReply(_servername, ERR_NOSUCHNICK, client.getNickname(), msg.params[1]));
		return;
	}
	if (!(channel->have_member(msg.params[1])))
	{
		sendReply(client, generateErrorReply(_servername, ERR_USERNOTINCHANNEL, client.getNickname(), msg.params[1], msg.params[0]));
		return;
	}
	if (target == &client)
		return;
	std::string	tmp;
	tmp = client.get_full_name() + " KICK " + msg.params[0] + ' ' + msg.params[1] + " :";
	if (msg.params.size() == 3)
		tmp += msg.params[2];
	else
		tmp += client.getNickname();
	tmp += "\r\n";
	channel->send_message(tmp);
	channel->remove_member(target);
	target->remove_channel(channel);
}

void	Server::commandKILL(Client &client, Message &msg)
{
	Client *cl = findClient(msg.params[0], this->_connectedClients);
	if (!client.get_operator_status())
		sendReply(client, generateErrorReply(_servername, ERR_NOPRIVILEGES, client.getNickname(), msg.command));
	else if (msg.params.size() < 2)
		sendReply(client, generateErrorReply(_servername, ERR_NEEDMOREPARAMS, client.getNickname(), msg.command));
	else if (msg.params[0] == this->_servername)
		sendReply(client, generateErrorReply(_servername, ERR_CANTKILLSERVER, client.getNickname(), msg.command, msg.params[0]));
	else if (cl == NULL)
		sendReply(client, generateErrorReply(_servername, ERR_NOSUCHNICK, client.getNickname(), msg.command, msg.params[0]));
	else
	{
		this->sendReply(*cl, generateNormalReply(this->_servername, RPL_KILLUSER, cl->getNickname(), "KILL", msg.params[1]));
		this->commandQUIT(*cl, msg);
	}
}
