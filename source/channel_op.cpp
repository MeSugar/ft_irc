#include "../include/Server.hpp"
#include <climits>
#include <cctype>
#include <cerrno>

//TEST
void	Server::server_test_client()
{
	Client	test_client;

	test_client.setRegistrationStatus();
	test_client.client_test_loop(*this);
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
			client.remove_channel(channel);
			if (channel->empty())
				remove_channel(channel);
			else if (channel->operators_empty())
				channel->make_any_operator();
		}
		else
		{	
			sendReply(generateErrorReply(_servername, ERR_NOSUCHCHANNEL, *it));
			continue;
		}
	}
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
	

bool	Server::check_channel_modes(const std::string& str, const Message& msg)
{
	if (str.size() < 2 || str.size() > 11)
		return (false);
	if (str[0] != '+' && str[0] != '-')
		return (false);
	int	param_modes = 3;
	for (size_t i = 1; i < str.size(); i++)
	{
		if (!is_channel_mode(str[i]))
		{
			sendReply(generateErrorReply(_servername, ERR_UNKNOWNMODE, std::string(1, str[i])));
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
		sendReply(generateErrorReply(_servername, ERR_NEEDMOREPARAMS, "MODE"));
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

void	Server::handle_channel_mode(char sign, char mode, Channel* channel, std::string param)
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
				sendReply(generateErrorReply(_servername, ERR_NOSUCHNICK, param));
		}
		else if (sign == '-')
		{
			if (channel->have_operator(param))
			{	
				channel->remove_operator(param);
				if (channel->operators_empty())
					channel->make_any_operator();
			}
			else
				sendReply(generateErrorReply(_servername, ERR_NOSUCHNICK, param));
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
	}
	else if (mode == 'i')
	{
		if (sign == '+' && !(channel->get_invite_status()))
			channel->set_invite_status(true);
		else if (sign == '-' && channel->get_invite_status())
			channel->set_invite_status(false);
	}
	else if (mode == 't')
	{
		if (sign == '+' && !(channel->get_topic_status()))
			channel->set_topic_status(true);
		else if (sign == '-' && channel->get_topic_status())
			channel->set_topic_status(false);
	}
	else if (mode == 'n')
	{
		if (sign == '+' && !(channel->get_outside_status()))
			channel->set_outside_status(true);
		else if (sign == '-' && channel->get_outside_status())
			channel->set_outside_status(false);
	}
	else if (mode == 'm')
	{
		if (sign == '+' && !(channel->get_moder_status()))
			channel->set_moder_status(true);
		else if (sign == '-' && channel->get_moder_status())
			channel->set_moder_status(false);
	}
	else if (mode == 'm')
	{
		if (sign == '+' && !(channel->get_moder_status()))
			channel->set_moder_status(true);
		else if (sign == '-' && channel->get_moder_status())
			channel->set_moder_status(false);
	}
	else if (mode == 'l')
	{	
		if ((sign == '-' && channel->get_user_limit() != MAX_MEMBERS)
			|| (sign == '+' && channel->get_user_limit() != static_cast<size_t>(atoi(param.c_str()))))
			channel->set_user_limit(param, sign);
	}
	else if (mode == 'b')
	{
		if (sign == '+' && !(channel->have_banmask(param)))
			channel->add_banmask(param);
		else if (sign == '-' && channel->have_banmask(param))
			channel->remove_banmask(param);
	}
	else if (mode == 'v')
	{
		if (!(channel->have_member(param)))
			sendReply(generateErrorReply(_servername, ERR_NOSUCHNICK, param));
		else if (sign == '+' && !(channel->have_speaker(param)))
			channel->add_speaker(param);
		else if (sign == '-' && channel->have_speaker(param))
			channel->remove_speaker(param);
	}
	else if (mode == 'k')
	{
		if (sign == '+' && channel->have_key())
			sendReply(generateErrorReply(_servername, ERR_KEYSET, param));
		else if (sign == '+')
			channel->set_key(param);
		else if (sign == '-' && channel->have_key())
			channel->remove_key();
	}
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
	sendReply(generateNormalReply(_servername, RPL_CHANNELMODEIS, channel->get_name(), tmp, ss.str()));
	if (channel->have_key())
	{	
		tmp = "+k";
		sendReply(generateNormalReply(_servername, RPL_CHANNELMODEIS, channel->get_name(), tmp, channel->get_key()));
	}
}

void	Server::banlist_rpl(Channel* channel, Client& client)
{
	(void)client;
	const std::vector<std::string> bans = channel->get_banlist();

	for (std::vector<std::string>::const_iterator it = bans.begin(); it != bans.end(); it++)
		sendReply(generateNormalReply(_servername, RPL_BANLIST, channel->get_name(), *it));
	sendReply(generateNormalReply(_servername, RPL_ENDOFBANLIST, channel->get_name()));
}

void	Server::channel_mode(Client &client, Message &msg)
{
	Channel* channel = find_channel(msg.params[0]);
	
	if (!channel)
	{	
		sendReply(generateErrorReply(_servername, ERR_NOSUCHCHANNEL, msg.params[0]));
		return;
	}
	if (!channel->have_member(client))
	{
		sendReply(generateErrorReply(_servername, ERR_NOTONCHANNEL, msg.params[0]));
		return;
	}
	if (msg.params.size() == 1)
	{
		channel_mods_rpl(channel, client);
		return;
	}
	if (!channel->have_operator(client))
	{
		sendReply(generateErrorReply(_servername, ERR_CHANOPRIVSNEEDED, msg.params[0]));
		return;
	}
	if (msg.params.size() == 2 && (msg.params[1] == "+b" || msg.params[1] == "-b" || msg.params[1] == "b"))
	{
		banlist_rpl(channel, client);
		return;
	}
	std::string	mod = msg.params[1];
	if (!check_channel_modes(mod, msg))
		return;
	size_t	param = 2;
	size_t	max_param = 1;
	for (size_t i = 1; i < mod.size(); i++)
		if (mod[i] == 'o' || mod[i] == 'b' || (mod[i] == 'l' && mod[0] == '+') || mod[i] == 'v'
			|| (mod[i] == 'k' && mod[0] == '+'))
			max_param++;
	for (size_t i = 1; i < mod.size(); i++)
	{
		if (max_param < param)
			handle_channel_mode(mod[0], mod[i], channel, std::string());
		else
			handle_channel_mode(mod[0], mod[i], channel, msg.params[param]);
		if (mod[i] == 'o' || mod[i] == 'b' || (mod[i] == 'l' && mod[0] == '+') || mod[i] == 'v'
			|| (mod[i] == 'k' && mod[0] == '+'))
			param++;
	}
}

bool	Server::check_user_modes(const std::string& mod)
{
	if (mod.size() < 2 || mod.size() > 5)
		return (false);
	if (mod[0] != '+' && mod[0] != '-')
		return (false);
	for (size_t i = 1; i < mod.size(); i++)
	{
		if (mod[i] != 'i' && mod[i] != 'w' && mod[i] != 's' && mod[i] != 'o')
		{
			sendReply(generateErrorReply(_servername, ERR_UMODEUNKNOWNFLAG));
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
	}
	else if (mode == 's')
	{
		if (sign == '+' && !(client.get_receive_notices()))
			client.set_receive_notices(true);
		else if (sign == '-' && client.get_receive_notices())
			client.set_receive_notices(false);
	}
	else if (mode == 'w')
	{
		if (sign == '+' && !(client.get_receive_wallops()))
			client.set_receive_wallops(true);
		else if (sign == '-' && client.get_receive_wallops())
			client.set_receive_wallops(false);
	}
	else if (mode == 'o')
		if (sign == '-' && client.get_operator_status())
			client.set_operator_status(false);
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
	sendReply(generateNormalReply(_servername, RPL_UMODEIS, tmp));
}

void	Server::user_mode(Client &client, Message &msg)
{
	if (!findClient(msg.params[0], _clients))
	{
		sendReply(generateErrorReply(_servername, ERR_NOSUCHNICK, msg.params[0]));
		return;
	}
	if (client.getNickname() != msg.params[0])
	{	
		sendReply(generateErrorReply(_servername, ERR_USERSDONTMATCH));
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
	if (!check_user_modes(mod))
		return;
	for (size_t i = 1; i < mod.size(); i++)
		handle_user_mode(mod[0], mod[i], client);
}

void	Server::commandMODE(Client &client, Message &msg)
{
	if ((!msg.prefix.empty() && !comparePrefixAndNick(msg.prefix, client)) || !client.getRegistrationStatus())
		return;
	if (msg.params.empty())
	{	
		sendReply(generateErrorReply(_servername, ERR_NEEDMOREPARAMS, "MODE"));
		return;
	}
	if (msg.params[0].empty())
	{
		sendReply(generateErrorReply(_servername, ERR_NOSUCHNICK, msg.params[0]));
		return;
	}
	if (check_channel_name(msg.params[0]))
		channel_mode(client, msg);
	else
		user_mode(client, msg);
}
