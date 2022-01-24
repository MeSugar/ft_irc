#include "../include/Channel.hpp"

Channel::Channel() {}

Channel::~Channel() {}

Channel::Channel(Channel const &other)
{
	*this = other;
}

Channel& Channel::operator=(Channel const &other)
{
	(void)other;

	return (*this);
}

Channel::Channel(Server* server, std::string name, Client* first) : _server(server), _name(name), _isInviteOnly(false), _isPrivate(false), _isSecret(false),
													_opTopicOnly(false), _noMessagesFromOutside(false), _isModerated(false), _user_limit(MAX_MEMBERS)
{
	_members.push_back(first);
	_channelOperators.push_back(first);
	first->add_channel(this);
}

static bool	mask_compare(const char* str, const char* mask)
{
	if (*str == *mask && *str != '\0')
		return (mask_compare(str + 1, mask + 1));
	else if (*str == *mask && *str == '\0')
		return (true);
	else if (*str == '\0' && *mask == '*')
		return (mask_compare(str, mask + 1));
	else if (*str != '\0' && *mask == '*')
		return ((mask_compare(str + 1, mask)) || (mask_compare(str, mask + 1)));
	return (false);
}

bool	Channel::check_bans(const Client& client)
{
	for (std::vector<std::string>::iterator it = _bans.begin(); it != _bans.end(); it++)
	{
		size_t	i;
		size_t	start = 0;
		i = (*it).find('!', start);
		if (mask_compare(client.getNickname().c_str(), (*it).substr(start, i - start).c_str()))
			return (true);
		start = i + 1;
		i = (*it).find('@', start);
		if (mask_compare(client.getUsername().c_str(), (*it).substr(start, i - start).c_str()))
			return (true);
		start = i + 1;
		if (mask_compare(client.getHostname().c_str(), (*it).substr(start, (*it).size() - start).c_str()))
			return (true);
	}
	return (false);
}

bool	Channel::have_banmask(const std::string& ban)
{
	for (std::vector<std::string>::iterator it = _bans.begin(); it != _bans.end(); it++)
		if (*it == ban)
			return (true);
	return (false);
}

bool	Channel::have_key() const
{
	if (_key.empty())
		return (false);
	return (true);
}

bool	Channel::check_key(std::string key) const
{
	if (key == _key)
		return (true);
	return (false);
}

bool	Channel::under_user_limit() const
{
	if (_members.size() < _user_limit)
		return (true);
	return (false);
}

bool	Channel::have_member(const Client& client) const
{
	for (std::vector<Client *>::const_iterator it = _members.begin(); it != _members.end(); it++)
		if (*it == &client)
			return (true);
	return (false);
}

bool	Channel::have_member(const std::string& nickname) const
{
	for (std::vector<Client *>::const_iterator it = _members.begin(); it != _members.end(); it++)
		if ((*it)->getNickname() == nickname)
			return (true);
	return (false);
}

bool	Channel::have_operator(const Client& client) const
{
	for (std::vector<Client *>::const_iterator it = _channelOperators.begin(); it != _channelOperators.end(); it++)
		if (*it == &client)
			return (true);
	return (false);
}

bool	Channel::have_operator(const std::string& nickname) const
{
	for (std::vector<Client *>::const_iterator it = _channelOperators.begin(); it != _channelOperators.end(); it++)
		if ((*it)->getNickname() == nickname)
			return (true);
	return (false);
}

bool	Channel::have_speaker(const std::string& nickname) const
{
	for (std::vector<std::string>::const_iterator it = _speakers.begin(); it != _speakers.end(); it++)
		if (*it == nickname)
			return (true);
	return (false);
}

bool	Channel::empty() const
{
	return (_members.empty());
}

bool	Channel::operators_empty() const
{
	return (_channelOperators.empty());
}

std::string	Channel::list_all_members(const Client& client) const
{
	std::string res;
	bool	from_member = have_member(client);

	for (std::vector<Client *>::const_iterator it = _members.begin(); it != _members.end(); it++)
	{
		if ((*it)->get_invisible() && !from_member)
			continue;
		if (have_operator(**it))
			res += '@';
		else if (have_speaker((*it)->getNickname()))
			res += '+';
		res += (*it)->getNickname();
		if ((it + 1) != _members.end())
			res += ' ';
	}
	return (res);
}

size_t	Channel::size() const
{
	return (_members.size());
}

void	Channel::add_member(Client* client)
{
	_members.push_back(client);
	client->add_channel(this);
}

void	Channel::remove_member(Client* client)
{
	for (std::vector<Client *>::iterator it = _members.begin(); it != _members.end(); it++)
		if (*it == client)
		{	
			_members.erase(it);
			break;
		}
	remove_operator(client);
	remove_speaker(client->getNickname());
}

void	Channel::send_message(const std::string& str)
{
	for (std::vector<Client *>::iterator it = _members.begin(); it != _members.end(); it++)
		_server->sendReply(str);
}

void	Channel::add_operator(Client* client)
{
	_channelOperators.push_back(client);
	send_message(generateNormalReply(_server->get_servername(), RPL_CHANNELMODEIS, _name, "+o", client->getNickname()));
}

void	Channel::add_operator(const std::string& nickname)
{
	for (std::vector<Client *>::iterator it = _members.begin(); it != _members.end(); it++)
		if ((*it)->getNickname() == nickname)
		{	
			add_operator(*it);
			break;
		}
}

void	Channel::remove_operator(Client* client)
{
	for (std::vector<Client *>::iterator it = _channelOperators.begin(); it != _channelOperators.end(); it++)
		if (*it == client)
		{	
			_channelOperators.erase(it);
			send_message(generateNormalReply(_server->get_servername(), RPL_CHANNELMODEIS, _name, "-o", client->getNickname()));
			break;
		}
}

void	Channel::remove_operator(const std::string& nickname)
{
	for (std::vector<Client *>::iterator it = _channelOperators.begin(); it != _channelOperators.end(); it++)
		if ((*it)->getNickname() == nickname)
		{	
			_channelOperators.erase(it);
			send_message(generateNormalReply(_server->get_servername(), RPL_CHANNELMODEIS, _name, "-o", nickname));
			break;
		}
}

void	Channel::make_any_operator()
{
	add_operator(_members[0]);
}

void	Channel::add_speaker(const std::string& nickname)
{
	_speakers.push_back(nickname);
	send_message(generateNormalReply(_server->get_servername(), RPL_CHANNELMODEIS, _name, "+v", nickname));
}

void	Channel::remove_speaker(const std::string& nickname)
{
	for (std::vector<std::string>::iterator it = _speakers.begin(); it != _speakers.end(); it++)
		if (*it == nickname)
		{	
			_speakers.erase(it);
			send_message(generateNormalReply(_server->get_servername(), RPL_CHANNELMODEIS, _name, "-v", nickname));
			break;
		}
}

void	Channel::add_banmask(const std::string& ban)
{
	_bans.push_back(ban);
	send_message(generateNormalReply(_server->get_servername(), RPL_CHANNELMODEIS, _name, "+b", ban));
}

void	Channel::remove_banmask(const std::string& ban)
{
	for (std::vector<std::string>::iterator it = _bans.begin(); it != _bans.end(); it++)
		if (*it == ban)
		{
			_bans.erase(it);
			break;
		}
	send_message(generateNormalReply(_server->get_servername(), RPL_CHANNELMODEIS, _name, "-b", ban));
}

void	Channel::remove_key()
{
	_key.clear();
	send_message(generateNormalReply(_server->get_servername(), RPL_CHANNELMODEIS, _name, "-k", std::string()));
}

//setters
void	Channel::set_invite_status(bool status)
{
	_isInviteOnly = status;
	std::string mode;
	if (status == true)
		mode = "+i";
	else
		mode = "-i";
	send_message(generateNormalReply(_server->get_servername(), RPL_CHANNELMODEIS, _name, mode, std::string()));
}

void	Channel::set_private_status(bool status)
{
	_isPrivate = status;
	std::string mode;
	if (status == true)
		mode = "+p";
	else
		mode = "-p";
	send_message(generateNormalReply(_server->get_servername(), RPL_CHANNELMODEIS, _name, mode, std::string()));
}

void	Channel::set_secret_status(bool status)
{
	_isSecret = status;
	std::string mode;
	if (status == true)
		mode = "+s";
	else
		mode = "-s";
	send_message(generateNormalReply(_server->get_servername(), RPL_CHANNELMODEIS, _name, mode, std::string()));
}

void	Channel::set_topic_status(bool status)
{
	_opTopicOnly = status;
	std::string mode;
	if (status == true)
		mode = "+t";
	else
		mode = "-t";
	send_message(generateNormalReply(_server->get_servername(), RPL_CHANNELMODEIS, _name, mode, std::string()));
}

void	Channel::set_outside_status(bool status)
{
	_noMessagesFromOutside = status;
	std::string mode;
	if (status == true)
		mode = "+n";
	else
		mode = "-n";
	send_message(generateNormalReply(_server->get_servername(), RPL_CHANNELMODEIS, _name, mode, std::string()));
}

void	Channel::set_moder_status(bool status)
{
	_isModerated = status;
	std::string mode;
	if (status == true)
		mode = "+m";
	else
		mode = "-m";
	send_message(generateNormalReply(_server->get_servername(), RPL_CHANNELMODEIS, _name, mode, std::string()));
}

void	Channel::set_user_limit(std::string n, char sign)
{
	if (sign == '+')
	{	
		_user_limit = atoi(n.c_str());
		send_message(generateNormalReply(_server->get_servername(), RPL_CHANNELMODEIS, _name, "+l", n));
	}
	else
	{	
		_user_limit = MAX_MEMBERS;
		send_message(generateNormalReply(_server->get_servername(), RPL_CHANNELMODEIS, _name, "-l", std::string()));
	}
}

void	Channel::set_key(const std::string& key)
{
	_key = key;
	send_message(generateNormalReply(_server->get_servername(), RPL_CHANNELMODEIS, _name, "+k", key));
}

void	Channel::set_topic(const std::string& topic)
{
	_topic = topic;
	send_message(generateNormalReply(_server->get_servername(), RPL_TOPIC, _name, _topic));
}

//getters
const std::string&	Channel::get_name() const
{
	return (_name);
}

const std::string&	Channel::get_topic() const
{
	return (_topic);
}

const std::string&	Channel::get_key() const
{
	return (_key);
}

const bool&	Channel::get_invite_status() const
{
	return (_isInviteOnly);
}

const bool&	Channel::get_private_status() const
{
	return (_isPrivate);
}

const bool&	Channel::get_secret_status() const
{
	return (_isSecret);
}

const bool&	Channel::get_topic_status() const
{
	return (_opTopicOnly);
}

const bool&	Channel::get_outside_status() const
{
	return (_noMessagesFromOutside);
}

const bool&	Channel::get_moder_status() const
{
	return (_isModerated);
}

const size_t&	Channel::get_user_limit() const
{
	return (_user_limit);
}

const std::vector<std::string>&	Channel::get_banlist() const
{
	return (_bans);
}
