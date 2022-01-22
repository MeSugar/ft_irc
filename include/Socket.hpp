#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <iostream>
#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sstream>
#include <poll.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define MAX 80

class Socket {
    private:
        int         sockfd;
        int         connfd;
        int         port;
        // std::string msg;
		std::string	pass;
        std::string host;
        struct addrinfo *res;
    public:
        Socket();
        Socket(int port, std::string pass);
        Socket(std::string host, int port, std::string pass);
        int _socket();
        int _bind();
        int _connect();
        int _linsten(int backlog);
        int _accept();
        int _getaddrinfo();
        int getPort();
        int getSockfd();
        int getConnfd();
        const std::string &getHost();
        

};

#endif