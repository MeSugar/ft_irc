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

int Server::_creatpoll(int sockfd) {
	struct pollfd pf;

	if (sockfd > 0) {
		pf.fd = sockfd;
		pf.events = POLLIN;
		pf.revents = 0;
		this->_userfds.push_back(pf);
	}
	return (0);
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

int Server::_handler(std::string msg, int sockfd) {
	std::string newMsg;

	if (msg.find("Hello") != std::string::npos) {
		newMsg = "world!";
		send(sockfd, newMsg.c_str(), newMsg.length(), 0);
	}
	return (0);
}

int Server::chat(int sockfd) {
	if (this->_recv(sockfd) == 0) {
		std::cout << "msg: " << this->_message << std::endl;
		this->_handler(this->_message, sockfd);
	}
		// this->commandHandler(client, client.parse(str)); нужно передать объект клиента или создавать его в этой функции
		// тут не отправляем ответ, этим занимаются команды (у объекта клиента хранится в который его отправляем sockfd)
		// send(sockfd, str.c_str(), str.length(), 0);
	// }
	return (0);
}

int Server::run()
{
	this->s->_socket();
	std::cout << "Main server: " << this->s->getSockfd() << std::endl;
	this->s->_bind();
	this->s->_linsten(5);
	return (0);
}

int Server::loop() {
	const id_t	timeout(1000);

	this->run();
	while (true) {
		this->s->_accept();
		this->_creatpoll(this->s->getConnfd());
		std::cout << "Vector userfds: \n";
		for (std::vector<struct pollfd>::iterator it = this->_userfds.begin(); it != this->_userfds.end(); it++) {
			std::cout << "\tfd: " << (*it).fd << "\tevent: " << (*it).events << std::endl;
		}
		this->_clients.push_back(new Client());
		int ret =  poll(this->_userfds.data(), this->_userfds.size(), timeout);
		if (ret == 0)
			std::cout << "Error: timeout\n";
		else if (ret != -1) {
			for (size_t it = 0; it < this->_userfds.size(); it++) {
				if (this->_userfds[it].revents & POLLIN) {
					this->chat(this->_userfds[it].fd);
					this->_userfds[it].revents = 0;
				}
			}
		}
	}
}
