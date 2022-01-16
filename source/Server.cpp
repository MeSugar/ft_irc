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

	pf.fd = sockfd;
	pf.events = POLLIN;
	pf.revents = 0;
	this->_userfds.push_back(pf);
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
	std::cout << "sockfd: " << sockfd << std::endl;
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
	// while (true) {
		if (this->_recv(sockfd) == 0) {
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
	// this->s->_accept();
	// this->_creatpoll(this->s->getConnfd());
	// this->_clients.push_back(new Client());
	// this->s->getConnfd()
	// this->chat(this->s->getConnfd());
	// close(this->s->getSockfd());
	return (0);
}

// int Server::loop()
// {
// 	struct pollfd fds[2];
// 	fds[0].fd = sock1;
// 	fds[0].events = POLLIN;
	
// 	// а от sock2 - исходящих
// 	fds[1].fd = sock2;
// 	fds[1].events = POLLOUT;
	
// 	// ждём до 10 секунд
// 	int ret = poll( &fds, 2, 10000 );
// 	// проверяем успешность вызова
// 	if ( ret == -1 )
// 		// ошибка
// 	else if ( ret == 0 )
// 		// таймаут, событий не произошло
// 	else
// 	{
// 		// обнаружили событие, обнулим revents чтобы можно было переиспользовать структуру
// 		if ( pfd[0].revents & POLLIN )
// 			pfd[0].revents = 0;
// 			// обработка входных данных от sock1

// 		if ( pfd[1].revents & POLLOUT )
// 			pfd[1].revents = 0;
// 			// обработка исходящих данных от sock2
// 	}
// 	return (0);
// }

int Server::loop() {
	this->run();
	std::cout << "Size: " << this->_userfds.size() << std::endl;
	// const id_t								timeout(2);
	struct pollfd 			pfarr[10];
	int i = 0;
	while (true) {
		this->s->_accept();

		// struct pollfd pf;
		// pf.fd = this->s->getConnfd();
		// pf.events = POLLIN;
		// pf.revents = 0;
		std::cout << "fd: " << pfarr[i].fd << "\t" << pfarr[i].events << "\t" << pfarr[i].revents << std::endl;
		this->_creatpoll(this->s->getConnfd());
		pfarr[i] = this->_userfds[i];
		// std::cout << "Connfd: " << this->s->getConnfd() << std::endl;
		// std::cout << "Size: " << this->_userfds.size() << std::endl;
		// std::cout << "Data[0]: " << this->_userfds[0].fd << "\t" << this->_userfds[0].events << std::endl;
		// char	host[INET_ADDRSTRLEN];
		// inet_ntop(AF_INET, &(this->s->res->ai_addr), host, INET_ADDRSTRLEN);
		std::cout << "Vector userfds: \n";
		for (std::vector<struct pollfd>::iterator it = this->_userfds.begin(); it != this->_userfds.end(); it++) {
			std::cout << "\tfd: " << (*it).fd << "\tevent: " << (*it).events << std::endl;
		}
		this->_clients.push_back(new Client());
		// std::vector<struct pollfd> fds = this->_userfds;
		// fds.push_back((struct pollfd){0, 0, 0});
		int ret =  poll((struct pollfd *)this->_userfds.data(), this->_userfds.size(), -1);
		// int ret =  poll((struct pollfd *)&pfarr, 10, timeout);
		if (ret == 0)
			std::cout << "Error: timeout\n";
		else if (ret != -1) {
			std::cout << "Wow\n";
			pfarr[i].events = POLLIN;
			for (int it = 0; it < i + 1; it++) {
				int a  = pfarr[it].revents & POLLIN;
				std::cout << a << std::endl;
				if (pfarr[it].revents & POLLIN) {
					pfarr[it].revents = 0;
					this->chat(pfarr[it].fd);
				}
			}
		}
		i++;
	}
}
