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
	return (msg);
}

int Server::chat(int sockfd) {
	
	std::string str;
	while (1)
	{
		str = this->_recv(sockfd);
		// this->commandHandler(client, client.parse(str)); нужно передать объект клиента или создавать его в этой функции
		// тут не отправляем ответ, этим занимаются команды (у объекта клиента хранится в который его отправляем sockfd)
		// send(sockfd, str.c_str(), str.length(), 0);
	}
}

int Server::run()
{
	this->s->_socket();
	std::cout << "Main server: " << this->s->getSockfd() << std::endl;
	this->s->_bind();
	this->s->_linsten(5);
	this->s->_accept();
	this->chat(this->s->getConnfd());
	close(this->s->getSockfd());
	return (0);
}

int Server::loop()
{
	struct timeval tv;
	fd_set readfds;

	tv.tv_sec = 2;
	tv.tv_usec = 500000;
	FD_ZERO(&readfds);
	for (;;) {
		this->run();
		FD_SET(this->s->getConnfd(), &readfds);
		select(this->s->getConnfd(), &readfds, NULL, NULL, &tv);
	}
}
