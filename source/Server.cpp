#include "../include/Server.hpp"

Server::Server(int port, std::string const &password)
: _port(port), _password(password), _servername("Nasha Iro4ka 1.0")
{
	this->parseMOTD();
}

Server::~Server() {}

// private methods
void    Server::parseMOTD()
{
	std::ifstream	file("files/MOTD");
	std::string		str;
	while (file.is_open() && getline(file, str))
		this->_MOTD.push_back(str);
	file.close();
}

void	Server::sendReply(std::string const &reply) const
{
	std::cout << reply << std::endl;
}

// connection managment
int Server::chat(int sockfd) {
    printf("Server: %i\n", sockfd);
    char buff[MAX];
    int n;
    for (;;) {
        bzero(buff, MAX);
        read(sockfd, buff, sizeof(buff));
        printf("From client: %s\t To client : ", buff);
        bzero(buff, MAX);
        n = 0;
        while ((buff[n++] = getchar()) != '\n')
            ;
        write(sockfd, buff, sizeof(buff));
        if (strncmp("exit", buff, 4) == 0) {
            printf("Server Exit...\n");
            break;
        }
    }
    return 0;
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

// commands
void	Server::commandPASS(Client &client, Message &msg)
{
	if (msg.prefix.empty() && client.getNickname().empty() && client.getUsername().empty())
	{
		if (client.getRegistrationStatus())
			this->sendReply(generateErrorReply(this->_servername, ERR_ALREADYREGISTRED));
		else if (msg.params.empty())
			this->sendReply(generateErrorReply(this->_servername, ERR_NEEDMOREPARAMS, "PASS"));
        else if (msg.params[0] != this->_password || msg.params.size() > 1)
			this->sendReply(generateErrorReply(this->_servername, ERR_PASSWDMISMATCH, "PASS"));
		else
			client.setPassword(msg.params[0]);
	}
}
