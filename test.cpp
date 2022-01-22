#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/poll.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#define CNT 3

struct sockaddr_in sad;

void *
client(void *arg)
{
   int s;
   const char *p = "Some data\n";

   if ((s = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
      perror("socket");
      return NULL;
   }

   if (connect(s, (struct sockaddr *)&sad, sizeof(sad)) == -1) {
       perror("connect");
       return NULL;
   }

   write(s, p, strlen(p));
   close(s);

   return NULL;
}

int 
main(int ac, char **av)
{
    struct pollfd pfd[CNT];
    char buff[100];

    for (int i = 0; i < CNT; i++) {
        pfd[i].fd = -1;
        pfd[i].events = 0;
        pfd[i].revents = 0;
    }
    int sockfd;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in sockaddr;
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // INADDR_ANY; Was 0.0.0.0, now 127.0.0.1
	sockaddr.sin_port = htons(atoi(av[1])); // htons is necessary to convert a number to network byte order
    int trueFlag = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &trueFlag, sizeof(int)) < 0)
        printf("Error: set\n");
    if (bind(sockfd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0)
        printf("Error: bind\n");
    if (listen(sockfd, 2) < 0)
        printf("Error: listen\n");
    fcntl(sockfd, F_SETFL, O_NONBLOCK);
    int i = 0;
    const id_t								timeout(1000);
    while (1) {
        memset(buff, 0, 100);
        size_t addrlen = sizeof(sockaddr);
	    int connection = accept(sockfd, (struct sockaddr*)&sockaddr, (socklen_t*)&addrlen);
        if (connection > 0) {
            pfd[i].fd = connection;
            pfd[i].events = POLLIN;
		    pfd[i].revents = 0;
            // int ret = poll(pfd, CNT, -1);
        } else {
            printf("Error: accept\n");
        }
        int ret = poll(pfd, CNT, timeout);
        if (ret != 0) {
            for (int j = 0; j < CNT; j++) {
                if (pfd[j].revents & POLLIN) {
                    read(pfd[j].fd, buff, 100);
                    printf("%s\n", buff);
                }
                pfd[j].revents = 0;
            }
        } else {
            printf("Error: poll\n");
        }
    }

   return 0;
}