#include "../include/Client.hpp"

Client::Client(int sockfd)
: _clientFd(sockfd), _isRegistered(false), _isAway(false), _channelsLimit(10)
{}

Client::~Client() {}

// getters
int					Client::getClientFd() const { return this->_clientFd; }
bool				Client::getRegistrationStatus() const { return this->_isRegistered; }
bool				Client::getAwayStatus() const { return this->_isAway; }
std::string	const	&Client::getAwayMessage() const { return this->_awayMessage; }
std::string	const	&Client::getPassword() const { return this->_password; }
std::string const	&Client::getNickname() const { return this->_nickname; }
std::string const	&Client::getUsername() const { return this->_username; }
std::string	const	&Client::getHostname() const { return this->_hostname; } 

// setters
void	Client::setClientFd(int fd) { this->_clientFd = fd; }
void    Client::setPassword(std::string const &pass) { this->_password = pass; }

void	Client::setNickname(std::string const &nick)
{
	this->_nickname = nick;
	this->_nicknameHistory.push_back(nick);
}

void	Client::setRegistrationStatus() { this->_isRegistered = true; }
void	Client::setAwayStatus(const std::string &msg)
{
	if (msg.size() > 0)
	{
		this->_awayMessage = msg;
		this->_isAway = true;
	}
	else
		this->_isAway = false;
}

void	Client::setOperatorStatus() { this->_isOperator = true; }
void	Client::setHostname(std::string const &hostname) { this->_hostname = hostname; }

void	Client::setUser(std::vector<std::string> &params)
{
	if (params.size() == 4)
	{
		this->_username = params[0];
		this->_hostname = params[1];
		if (params[3][0] == ':')
			params[3].erase(params[3].begin());
		this->_realname = params[3];
	}
}

// parser utils
int     Client::check_length(const char* buf)
{
	int	i;
	
	i = 0;
	while(buf[i] && buf[i] != '\r' && i < 510)
		i++;
	if (buf[i] == '\r' && buf[i + 1] == '\n')
		return (1);
	else
		return (0);
}

int		Client::get_prefix(const char* buf, Message& res)
{
	int	i;
	
	i = 1;
	while (buf[i] != '\r' && buf[i] != ' ')
		i++;
	if (i - 1 > 0)
		res.prefix.append(&buf[1], i - 1);
	while (buf[i] == ' ')
		i++;
	return (i);
}

void	Client::get_command(const char *buf, Message& res, int& i)
{
	int	start = i;

	while (buf[i] != '\r' && buf[i] != ' ')
		i++;
	if (i - start > 0)
		res.command.append(&buf[start], i - start);
}

void	Client::get_params(const char *buf, Message& res, int& i)
{
	int	start;
	std::string	temp;

	while (buf[i] != '\r')
	{
		while (buf[i] == ' ')
			i++;
		start = i;
		if (buf[i] == ':')
		{
			start++;
			while (buf[i] != '\r')
				i++;
			res.params.push_back(temp.append(&buf[start], i - start));
			return ;
		}
		while (buf[i] != '\r' && buf[i] != ' ')
			i++;
		res.params.push_back(temp.append(&buf[start], i - start));
		temp.clear();
	}
}

int		Client::check_command(Message& mes)
{
	const char*	com_array[] = {"PASS", "NICK", "USER", "OPER", "QUIT",
						"JOIN", "PART", "MODE", "TOPIC", "NAMES", "LIST", "INVITE", "KICK",
						"PRIVMSG", "NOTICE",
						"KILL", "PING", "PONG", "AWAY"};
	if (!mes.command.size())
	{
		if (!mes.prefix.size() && !mes.params.size())
			return (-3); //empty message, ignore
		return (-2); //empty command
	}
	if (isalpha(mes.command[0]))
	{
		for (int i = 0; i <= 18; i++)
			if (mes.command == com_array[i])
				return (i);
	}
	if (mes.command.size() == 3 && isdigit(mes.command[0]) && isdigit(mes.command[1]) && isdigit(mes.command[2]))
	{
		int	d = atoi(mes.command.c_str());
		if ((d >= 200 && d <= 206) || d == 208 || d == 209 || (d >= 211 && d <= 219)
			|| d == 221 || (d >= 231 && d <= 235) || (d >= 241 && d <= 244)
			|| (d >= 251 && d <= 259) || d == 261 || (d >= 300 && d <= 303)
			|| d == 305 || d == 306 || (d >= 311 && d <= 319) || (d >= 321 && d <= 324)
			|| d == 331 || d == 332 || d == 341 || d == 342 || (d >= 351 && d <= 353)
			|| (d >= 361 && d <= 369) || (d >= 371 && d <= 376) || d == 381 || d == 382
			|| d == 384 || (d >= 391 && d <= 395) || (d >= 401 && d <= 407) || d == 409
			|| (d >= 411 && d <= 414) || (d >= 421 && d <= 424) || (d >= 431 && d <= 433)
			|| d == 436 || (d >= 441 && d <= 446) || d == 451 || (d >= 461 && d <= 467)
			|| (d >= 471 && d <= 476) || (d >= 481 && d <= 483) || d == 491 || d == 492
			|| d == 501 || d == 502)
			return (-3); //ignore command
	}
	return (-1); //no such command
}

// parser
Message		Client::parse(const char* buf)
{
	Message	res;
	int		i;
	
	if (!check_length(buf))
		return (res);
	i = 0;
	if (buf[i] == ':')
		i = get_prefix(buf, res);
	if (buf[i] != '\r')
		get_command(buf, res, i);
	if (buf[i] != '\r')
		get_params(buf, res, i);
	return (res);
}


// channel utils
bool	Client::check_invitation(const std::string&	ch_name)
{
	for (std::vector<std::string>::iterator it = _channels_invited.begin(); it != _channels_invited.end(); it++)
		if (*it == ch_name)
			return (true);
	return (false);
}

void	Client::add_channel(Channel* channel)
{
	_channels.push_back(channel);
}

void	Client::remove_channel(Channel *channel)
{
	for (std::vector<Channel *>::iterator it = _channels.begin(); it != _channels.end(); it++)
		if (*it == channel)
		{	
			_channels.erase(it);
			break;
		}
}

bool	Client::under_channels_limit() const
{
	if (_channels.size() < _channelsLimit)
		return (true);
	return (false);
}