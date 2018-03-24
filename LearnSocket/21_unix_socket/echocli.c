//
//  echocli.c
//  LearnSocket
//
//  Created by Zhang Yuanming on 3/22/18.
//  Copyright © 2018 HansonStudio. All rights reserved.
//

#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <errno.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/un.h>


#define ERR_EXIT(m) \
do \
{ \
perror(m); \
exit(EXIT_FAILURE); \
} while(0)


void echo_cli(int sock)
{
    char sendbuf[1024] = {0};
    char recvbuf[1024] = {0};
    while (fgets(sendbuf, sizeof(sendbuf), stdin) != NULL) {
        write(sock, sendbuf, strlen(sendbuf));
        read(sock, recvbuf, sizeof(recvbuf));
        fputs(recvbuf, stdout);
        memset(sendbuf, 0, sizeof(sendbuf));
        memset(recvbuf, 0, sizeof(recvbuf));
    }
    close(sock);
}


// gcc -Wall -g main.c -o main
int main(void)
{
    int sock;
    if ((sock = socket(PF_UNIX, SOCK_STREAM, 0)) < 0) {
        ERR_EXIT("socket");
    }
    struct sockaddr_un servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sun_family = AF_UNIX;
    strcpy(servaddr.sun_path, "test_socket");

    if (connect(sock, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        ERR_EXIT("connect");
    }

    echo_cli(sock);
    return 0;

}
