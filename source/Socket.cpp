#include "../include/Socket.hpp"

Socket::Socket() {
    this->host = "localhost";
    this->port = 8080;
    this->sockfd = -1;
    
    this->_getaddrinfo();
}

Socket::Socket(int port, std::string pass) {
    this->host = "localhost";
    this->port = port;
    this->pass = pass;
    this->sockfd = -1;

    this->_getaddrinfo();
}

Socket::Socket(std::string host, int port, std::string pass) {
    this->host = host;
    this->port = port;
    this->pass = pass;
    this->sockfd = -1;

    this->_getaddrinfo();
}

int Socket::_socket() {
    int err;
    this->sockfd = socket(this->res->ai_family,
                        this->res->ai_socktype,
                        this->res->ai_protocol);

    err = this->sockfd;
    if (err == -1) {
        printf("socket creation failed...\n");
        // exit(0);
        // error
    } else {
        printf("Socket successfully created..\n");
    }
    return (0);
}

int Socket::_bind() {
    int err;

    err = bind(this->sockfd, this->res->ai_addr, this->res->ai_addrlen);
    if (err != 0) {
        printf("socket bind failed...\n");
        // exit(0);
        // error
    } else {
        printf("Socket successfully binded..\n");
    }
    return (0);
}

int Socket::_connect() {
    int err;

    err = connect(this->sockfd, this->res->ai_addr, this->res->ai_addrlen);
    if (err != 0) {
        printf("connection with the server failed...\n");
        // exit(0);
        // error 
    } else {
        printf("connected to the server..\n");
    }
    return (0);
}

int Socket::_linsten(int backlog) {
    int err;

    err = listen(this->sockfd, backlog);
    if (err != 0) {
        printf("Listen failed...\n");
        // exit(0);
        // error
    } else {
        printf("Server listening..\n");
    }
    fcntl(this->sockfd, F_SETFL, O_NONBLOCK);
    return (0);
}

int Socket::_accept() {
    int err;

    struct sockaddr_storage addr;
    int size;

    size = sizeof(addr);

    this->connfd = accept(this->sockfd, (struct sockaddr *)&addr, (socklen_t *)&size);

    err = this->connfd;
    if (err < 0) {
        // printf("server accept failed...\n");
        // exit(0);
        // error
    } else {
        printf("server accept the client...\n");
    }
    char	host[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(this->res->ai_addr), host, INET_ADDRSTRLEN);
    return (0);
}

int Socket::_getaddrinfo() {
    struct addrinfo hints;

    bzero(&hints, sizeof(hints));
   
    hints.ai_family = AF_UNSPEC; 
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    // Заполняю res
    // const char *host = this->host.c_str();
    std::stringstream ss;
    ss << this->port;
    std::string str = ss.str();
    const char *port = str.c_str();
    getaddrinfo(NULL, port, &hints, &this->res);
    return (0);
}

int Socket::getPort() {
    return (this->port);
}

int Socket::getSockfd() {
    return (this->sockfd);
}

int Socket::getConnfd() {
    return (this->connfd);
}

const std::string &Socket::getHost() {
    return (this->host);
}

