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

Channel::Channel(std::string name, Client* first) : _name(name), _isInviteOnly(false), _user_limit(65535)
{
	_members.push_back(first);
	_channelOperators.push_back(first);
	first->add_channel(this);
}

bool	Channel::check_bans(const Client& client)
{
	for (std::vector<std::string>::iterator it = _bans.begin(); it != _bans.end(); it++)
		if (*it == client.getNickname())
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

bool	Channel::have_member(const Client& client)
{
	for (std::vector<Client *>::iterator it = _members.begin(); it != _members.end(); it++)
		if (*it == &client)
			return (true);
	return (false);
}

void	Channel::add_member(Client* client)
{
	_members.push_back(client);
	client->add_channel(this);
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

const bool&	Channel::get_invite_status() const
{
	return (_isInviteOnly);
}
