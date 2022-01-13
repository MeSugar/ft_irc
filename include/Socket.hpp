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

#define MAX 80

class Socket {
    private:
        int         sockfd;
        int         connfd;
        int         port;
		// std::string	password;
        // char        buff[MAX];
        std::string host;
        struct addrinfo *res;
    public:
        Socket();
        int _socket();
        int _bind();
        int _connect();
        int _linsten(int backlog);
        int _accept();
        int getPort();
        int getSockfd();
        int getConnfd();
        const std::string &getHost();
        

};

#endif