#include "../include/Server.hpp"

bool	work = true;

void	sigHandler(int signum)
{
	(void)signum;
	work = false;
}

Server::Server(int port, std::string const &password)
{
	this->_port = port;
	this->_password = password;
	this->s = new Socket(port, password);
	this->_servername = "~Nasha_Iro4ka_1.0~";
	this->_operatorHosts.push_back("host");
	this->_operators.insert(std::pair<std::string, std::string>("admin", "admin"));
	this->_commands.insert(std::make_pair("PASS", &Server::commandPASS));
	this->_commands.insert(std::make_pair("NICK", &Server::commandNICK));
	this->_commands.insert(std::make_pair("USER", &Server::commandUSER));
	this->_commands.insert(std::make_pair("OPER", &Server::commandOPER));
	this->_commands.insert(std::make_pair("QUIT", &Server::commandQUIT));
	this->_commands.insert(std::make_pair("JOIN", &Server::commandJOIN));
	this->_commands.insert(std::make_pair("PART", &Server::commandPART));
	this->_commands.insert(std::make_pair("MODE", &Server::commandMODE));
	this->_commands.insert(std::make_pair("TOPIC", &Server::commandTOPIC));
	this->_commands.insert(std::make_pair("NAMES", &Server::commandNAMES));
	this->_commands.insert(std::make_pair("LIST", &Server::commandLIST));
	this->_commands.insert(std::make_pair("INVITE", &Server::commandINVITE));
	this->_commands.insert(std::make_pair("KICK", &Server::commandKICK));
	this->_commands.insert(std::make_pair("PRIVMSG", &Server::commandPRIVMSG));
	this->_commands.insert(std::make_pair("NOTICE", &Server::commandNOTICE));
	// this->_commands.insert(std::make_pair("KILL", &Server::commandKILL));
	// this->_commands.insert(std::make_pair("PING", &Server::commandPING));
	// this->_commands.insert(std::make_pair("PONG", &Server::commandPONG));
	// this->_commands.insert(std::make_pair("REHASH", &Server::commandREHASH));
	// this->_commands.insert(std::make_pair("RESTART", &Server::commandRESTART));
	this->_commands.insert(std::make_pair("AWAY", &Server::commandAWAY));
	this->parseMOTD();
}

Server::~Server() {}

Client &Server::_findclient(int sockfd) {
	for (size_t i = 0; i < this->_clients.size(); i++) {
		if (this->_clients[i]->getClientFd() == sockfd)
			return *this->_clients[i];
	}
	return *this->_clients[0];
}

void Server::_deletepoll(int sockfd) {
	for (size_t i = 0; i < this->_userfds.size(); i++) {
		if (this->_userfds[i].fd == sockfd) {
			this->_userfds.erase(this->_userfds.begin() + i);
			break;
		}
	}
	for (size_t i = 0; i < this->_clients.size(); i++) {
		if (this->_clients[i]->getClientFd() == sockfd) {
			delete this->_clients[i];
			this->_clients.erase(this->_clients.begin() + i);
			break;
		}
	}
	for (size_t i = 0; i < this->_connectedClients.size(); i++) {
		if (this->_connectedClients[i]->getClientFd() == sockfd) {
			delete this->_connectedClients[i];
			this->_connectedClients.erase(this->_connectedClients.begin() + i);
			break;
		}
	}
}

int Server::_creatpoll(int sockfd) {
	struct pollfd pf;

	if (sockfd > 0)
	{
		pf.fd = sockfd;
		pf.events = POLLIN;
		pf.revents = 0;
		this->_userfds.push_back(pf);
		this->_clients.push_back(new Client(sockfd));
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
	if (cntBytes == 0)
		return -1;
	if (msg.length() > 512)
		msg = msg.substr(0, 510) + "\r\n";
	else
	{
		while (msg.find("\n") != std::string::npos)
			msg.replace(msg.find("\n"), 1, "\r");
		msg += "\n";
	}
	this->_message = msg;
	return (0);
}

int Server::chat(Client &client)
{
	Message msg;
	std::cout << this->_message << std::endl;
	if (this->_recv(client.getClientFd()) == 0)
	{
		msg = client.parse(this->_message.c_str());
		this->commandHandler(client, msg);
	}
	else
		return -1;
	return (0);
}

int Server::run()
{
	this->s->_socket();
	this->s->_bind();
	this->s->_linsten(5);
	return (0);
}

int Server::loop() {
	const id_t	timeout(1000);

	this->run();

	// signal(SIGINT, sigHandler);
	while (work) {
		this->s->_accept();
		this->_creatpoll(this->s->getConnfd());
		// std::cout << "Vector userfds: \n";
		// for (std::vector<struct pollfd>::iterator it = this->_userfds.begin(); it != this->_userfds.end(); it++) {
		// 	// std::cout << "\tfd: " << (*it).fd << "\tevent: " << (*it).events << std::endl;
		// }
		// this->_clients.push_back(new Client());
		int ret = poll(this->_userfds.data(), this->_userfds.size(), timeout);
		if (ret == 0)
		{}
			// std::cout << "Error: timeout\n";
		else if (ret != -1) {
			for (size_t it = 0; it < this->_userfds.size(); it++) {
				if (this->_userfds[it].revents & POLLIN) {
					int fd = this->_userfds[it].fd;
					if (this->chat(this->_findclient(fd)) == -1) {
						_deletepoll(fd);
						break;
					}
					this->_userfds[it].revents = 0;
				}
			}
		}
	}
	return (0);
}
