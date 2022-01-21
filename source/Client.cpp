#include "../include/Client.hpp"

Client::Client()
: _isRegistered(false), _isOperator(false), _invisible(false), _receive_server_notices(false), _receive_wallops(false), _channelsLimit(10)
{}

Client::~Client() {}

// getters
bool				Client::getRegistrationStatus() const { return this->_isRegistered; }
std::string	const	&Client::getPassword() const { return this->_password; }
std::string const	&Client::getNickname() const { return this->_nickname; }
std::string const	&Client::getUsername() const { return this->_username; }
std::string	const	&Client::getHostname() const { return this->_hostname; } 
const bool&			Client::get_invisible() const { return (_invisible); }
const bool&			Client::get_receive_notices() const { return (_receive_server_notices); }
const bool&			Client::get_receive_wallops() const { return (_receive_wallops); }
const bool&			Client::get_operator_status() const { return (_isOperator); }

// setters
void    Client::setPassword(std::string const &pass) { this->_password = pass; }

void	Client::setNickname(std::string const &nick)
{
	this->_nickname = nick;
	this->_nicknameHistory.push_back(nick);
}

void	Client::setRegistrationStatus() { this->_isRegistered = true; }
void	Client::setOperatorStatus() { this->_isOperator = true; }
void	Client::set_invisible(bool status) { _invisible = status; }
void	Client::set_receive_notices(bool status) { _receive_server_notices = status; }
void	Client::set_receive_wallops(bool status) { _receive_wallops = status; }
void	Client::set_operator_status(bool status) { _isOperator = status; }

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

int		Client::check_command(const Message& mes)
{
	const char*	com_array[] = {"PASS", "NICK", "USER", "OPER", "QUIT",
						"JOIN", "PART", "MODE", "TOPIC", "NAMES", "LIST", "INVITE", "KICK",
						"PRIVMSG", "NOTICE",
						"KILL", "PING", "PONG"};
	if (!mes.command.size())
	{
		if (!mes.prefix.size() && !mes.params.size())
			return (-3); //empty message, ignore
		return (-2); //empty command
	}
	if (isalpha(mes.command[0]))
	{
		for (int i = 0; i <= 17; i++)
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

void	Client::command_handle(Message& mes, Server& serv)
{
	typedef		void (Server::*funptr)(Client&, Message&);
	funptr		f[] = {&Server::commandPASS, &Server::commandNICK, NULL, NULL, NULL, &Server::commandJOIN, &Server::commandPART};

	int	i = check_command(mes);
	//(serv.*f[i])(*this, mes);
	if (i == 0 || i == 1 || i == 5 || i == 6)
		(serv.*f[i])(*this, mes);
}

//TEST
void	Client::client_test_loop(Server& serv)
{
	std::string	temp;
	Message		mes;
	
	while (getline(std::cin, temp) && temp != "EXIT")
	{
		temp += "\r\n";
		mes = parse(temp.c_str());
		command_handle(mes, serv);
	}
}
