#include "../include/Server.hpp"

Server::Server(int port, std::string const &password) : TemplateRun(port, password)
{
	this->_port = port;
	this->_password = password;
	this->_servername = "Nasha Iro4ka 1.0";
	this->_operatorHosts.push_back("host");
	this->_operators.insert(std::pair<std::string, std::string>("admin", "admin"));
	this->_commands.insert(std::make_pair("PASS", &Server::commandPASS));
	this->_commands.insert(std::make_pair("NICK", &Server::commandNICK));
	this->_commands.insert(std::make_pair("USER", &Server::commandUSER));
	this->_commands.insert(std::make_pair("OPER", &Server::commandOPER));
	// this->_commands.insert(std::make_pair("QUIT", &Server::commandQUIT));
	// this->_commands.insert(std::make_pair("JOIN", &Server::commandJOIN));
	// this->_commands.insert(std::make_pair("PART", &Server::commandPART));
	// this->_commands.insert(std::make_pair("MODE", &Server::commandMODE));
	// this->_commands.insert(std::make_pair("TOPIC", &Server::commandTOPIC));
	// this->_commands.insert(std::make_pair("NAMES", &Server::commandNAMES));
	// this->_commands.insert(std::make_pair("LIST", &Server::commandLIST));
	// this->_commands.insert(std::make_pair("INVITE", &Server::commandINVITE));
	// this->_commands.insert(std::make_pair("LICK", &Server::commandLICK));
	// this->_commands.insert(std::make_pair("PRIVMSG", &Server::commandPRIVMSG));
	// this->_commands.insert(std::make_pair("NOTICE", &Server::commandNOTICE));
	// this->_commands.insert(std::make_pair("KILL", &Server::commandKILL));
	// this->_commands.insert(std::make_pair("PING", &Server::commandPING));
	// this->_commands.insert(std::make_pair("PONG", &Server::commandPONG));
	// this->_commands.insert(std::make_pair("REHASH", &Server::commandREHASH));
	// this->_commands.insert(std::make_pair("RESTART", &Server::commandRESTART));
	this->parseMOTD();
}

Server::~Server() {}

std::string	Server::_recv(int sockfd)
{
<<<<<<< HEAD
	std::ifstream	file("files/MOTD");
	std::string		str;
	while (file.is_open() && getline(file, str))
		this->_MOTD.push_back(str);
	file.close();
}

void	Server::sendMOTD()
{
	for (std::vector<std::string>::iterator it = this->_MOTD.begin(); it != this->_MOTD.end(); it++)
		this->sendReply(*it);
}

void	Server::sendReply(std::string const &reply) const
{
	std::cout << reply << std::endl;
}

bool	Server::validateNickname(std::string const &nick)
{
	size_t len = nick.size();
	if (!nick.empty() && len <= 9)
	{
		size_t i = 0;
		while (i < len)
		{
			if (!std::isalnum(nick[i]) || nick[i] != '-'
				|| nick[i] != '[' || nick[i] != ']'
				|| nick[i] != '{' || nick[i] != '}'
				|| nick[i] != '^' || nick[i] != '\\')
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
		this->_clients.push_back(client);
		this->_connectedClients.push_back(client);
		this->sendMOTD();
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

int Server::_recv(int sockfd) {
	std::string msg;
	char buff[100];
	int cntBytes;

	while ((cntBytes = recv(sockfd, buff, 99, 0)) > 0) {
		buff[cntBytes] = 0;
		msg += buff;
		buff[0] = 0;
		if (msg.find('\n') != std::string::npos) {
			break;
		}
	}
	if (msg.length() > 512)
		msg = msg.substr(0, 510) + "\r\n";
	while (msg.find("\r\n") != std::string::npos)
		msg.replace(msg.find("\r\n"), 2, "\n");
	this->_message = msg;
	return (0);
}

int Server::_handler(std::string msg) {
	std::string newMsg;
	// check connection
	// if !connection {check command for connection}
	// else {other command}
	if (msg.find("Hello") != std::string::npos) {
		newMsg = "world!";
		send(this->s->getConnfd(), newMsg.c_str(), newMsg.length(), 0);
	}
	return (0);
}

int Server::chat(int sockfd) {
	while (true) {
		if (this->_recv(sockfd) == 0) {
			this->_handler(this->_message);
		}
		// this->commandHandler(client, client.parse(str)); нужно передать объект клиента или создавать его в этой функции
		// тут не отправляем ответ, этим занимаются команды (у объекта клиента хранится в который его отправляем sockfd)
		// send(sockfd, str.c_str(), str.length(), 0);
	}
	return (0);
}

int Server::run()
{
	this->s->_socket();
	std::cout << "Main server: " << this->s->getSockfd() << std::endl;
	this->s->_bind();
	this->s->_linsten(5);
	this->s->_accept();
	this->chat(this->s->getConnfd());
	// close(this->s->getSockfd());
	return (0);
}

int Server::loop()
{
	struct pollfd fds[2];
	fds[0].fd = sock1;
	fds[0].events = POLLIN;
	
	// а от sock2 - исходящих
	fds[1].fd = sock2;
	fds[1].events = POLLOUT;
	
	// ждём до 10 секунд
	int ret = poll( &fds, 2, 10000 );
	// проверяем успешность вызова
	if ( ret == -1 )
		// ошибка
	else if ( ret == 0 )
		// таймаут, событий не произошло
	else
	{
		// обнаружили событие, обнулим revents чтобы можно было переиспользовать структуру
		if ( pfd[0].revents & POLLIN )
			pfd[0].revents = 0;
			// обработка входных данных от sock1

		if ( pfd[1].revents & POLLOUT )
			pfd[1].revents = 0;
			// обработка исходящих данных от sock2
}
