#include "../include/Client.hpp"

Client::Client()
: _isRegistered(false), _channelsLimit(10)
{}

Client::~Client() {}

// getters
bool				Client::getRegistrationStatus() const { return this->_isRegistered; }
std::string	const	&Client::getPassword() const { return this->_password; }
std::string const	&Client::getNickname() const { return this->_nickname; }
std::string const	&Client::getUsername() const { return this->_username; }

// setters
void    Client::setPassword(std::string const &pass) { this->_password = pass; }

void	Client::setNickname(std::string const &nick)
{
	this->_nickname = nick;
	this->_nicknameHistory.push_back(nick);
}

void	Client::setRegistrationStatus() { this->_isRegistered = true; }

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
	funptr		f[] = {&Server::commandPASS};

	if (check_command(mes) == 0)
		(serv.*f[0])(*this, mes);
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
		std::cout << "prefix: " << mes.prefix << std::endl << "command: " << mes.command << std::endl << "params: " << std::endl;
		for (std::vector<std::string>::iterator it = mes.params.begin(); it != mes.params.end(); it++)
			std::cout << *it << std::endl;
		command_handle(mes, serv);
	}
}
