#include "../include/Server.hpp"

//TEST
void	Server::server_test_client()
{
	Client	test_client;

	test_client.setRegistrationStatus();
	test_client.client_test_loop(*this);
}

Channel*	Server::add_channel(std::string name, Client& first)
{
	Channel*	channel = new Channel(name, &first);
	
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
	if (str.empty())
		return (false);
	if (str[0] != '#' && str[0] != '&')
		return (false);
	for (size_t	i = 1; i < str.size(); i++)
		if (str[i] == ' ' || str[i] == '\n' || str[i] == '\r' || str[i] == '\a')
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

void Server::commandJOIN(Client& client, Message& msg)
{
	if ((!msg.prefix.empty() && !comparePrefixAndNick(msg.prefix, client)) || !client.getRegistrationStatus())
		return ;
	if (msg.params.size() > 2)
		return ;
	if (msg.params.empty())
	{	
		sendReply(generateErrorReply(_servername, ERR_NEEDMOREPARAMS, "JOIN"));
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
				sendReply(generateErrorReply(_servername, ERR_TOOMANYCHANNELS, *it));
				continue;
			}
			if (channel->get_invite_status() && !client.check_invitation(*it))
			{
				sendReply(generateErrorReply(_servername, ERR_INVITEONLYCHAN, *it));
				continue;
			}
			if (channel->check_bans(client))
			{
				sendReply(generateErrorReply(_servername, ERR_BANNEDFROMCHAN, *it));
				continue;
			}
			if (channel->have_key() && (keys.size() < j || !(channel->check_key(keys[j]))))
			{	
				sendReply(generateErrorReply(_servername, ERR_BADCHANNELKEY, *it));
				continue;
			}
			if (!(channel->under_user_limit()))
			{
				sendReply(generateErrorReply(_servername, ERR_CHANNELISFULL, *it));
				continue;
			}
			channel->add_member(&client);
			if (!(channel->get_topic().empty()))
				sendReply(generateNormalReply(_servername, RPL_TOPIC, *it, channel->get_topic()));
			else
				sendReply(generateNormalReply(_servername, RPL_NOTOPIC, *it));
			Message  mes;
			mes.params.push_back(*it);
			//commandNAMES(client, mes);
		}
		else
		{
			if (!check_channel_name(*it))
			{	
				sendReply(generateErrorReply(_servername, ERR_NOSUCHCHANNEL, *it));
				continue;
			}
			if (!client.under_channels_limit())
			{
				sendReply(generateErrorReply(_servername, ERR_TOOMANYCHANNELS, *it));
				continue;
			}
			channel = add_channel(*it, client);
			sendReply(generateNormalReply(_servername, RPL_NOTOPIC, *it));
			Message  mes;
			mes.params.push_back(*it);
			//commandNAMES(client, mes);
		}
	}
}

void	Server::commandPART(Client &client, Message &msg)
{
	if ((!msg.prefix.empty() && !comparePrefixAndNick(msg.prefix, client)) || !client.getRegistrationStatus())
		return ;
	if (msg.params.size() > 1)
		return ;
	if (msg.params.empty())
	{	
		sendReply(generateErrorReply(_servername, ERR_NEEDMOREPARAMS, "PART"));
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
				sendReply(generateErrorReply(_servername, ERR_NOTONCHANNEL, *it));
				continue;
			}
			channel->remove_member(&client);
			if (channel->empty())
				remove_channel(channel);
		}
		else
		{	
			sendReply(generateErrorReply(_servername, ERR_NOSUCHCHANNEL, *it));
			continue;
		}
	}
}
