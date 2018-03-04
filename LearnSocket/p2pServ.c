//
//  p2pServ.c
//  LearnSocket
//
//  Created by Zhang Yuanming on 3/4/18.
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
#include <signal.h>

#define ERR_EXIT(m) \
do \
{ \
perror(m); \
exit(EXIT_FAILURE); \
} while(0)


void handler(int sig)
{
    printf("recv a signal = %d\n", sig);
    exit(EXIT_SUCCESS);
}

// gcc -Wall -g main.c -o main
int main(int argc, const char * argv[]) {

    unsigned int x = 0x12345678;
    unsigned char *p = (unsigned char *)&x;
    printf("%x %x %x %x\n", p[0], p[1], p[2], p[3]);

    unsigned int y = htonl(x);
    p = (unsigned char *)&y;
    printf("%x %x %x %x\n", p[0], p[1], p[2], p[3]);

    in_addr_t addr = inet_addr("192.168.0.100");
    printf("addr=%d\n", ntohl(addr));

    struct in_addr ipaddr;
    ipaddr.s_addr = addr;
    printf("%s\n", inet_ntoa(ipaddr));


    int listenfd;
    if ((listenfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        ERR_EXIT("socket");
    }

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(5188);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    inet_aton("127.0.0.1", &servaddr.sin_addr);

    int on = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
        ERR_EXIT("setsockopt");
    }

    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        ERR_EXIT("bind");
    }

    if (listen(listenfd, SOMAXCONN) < 0) {
        ERR_EXIT("listen");
    }

    struct sockaddr_in peeraddr;
    socklen_t peerlen = sizeof(peeraddr);
    int conn;


    while (1) {
        if ((conn = accept(listenfd, (struct sockaddr *)&peeraddr, &peerlen)) < 0) {
            ERR_EXIT("accept");
        }

        printf("ip=%s port=%d\n", inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));

        pid_t pid;
        pid = fork();
        if (pid == -1) {
            ERR_EXIT("fork");
        } else if (pid == 0) {
            signal(SIGUSR1, handler);
            char sendbuf[1024] = {0};
            while (fgets(sendbuf, sizeof(sendbuf), stdin) != NULL) {
                write(conn, sendbuf, strlen(sendbuf));
                memset(sendbuf, 0, sizeof(sendbuf));
            }
            printf("child close\n");
            exit(EXIT_SUCCESS);
        } else {
            char recvbuf[1024];
            while (1) {
                memset(recvbuf, 0, sizeof(recvbuf));
                size_t ret = read(conn, recvbuf, sizeof(recvbuf));
                if (ret == -1) {
                    ERR_EXIT("read");
                } else if (ret == 0) {
                    printf("peer close\n");
                    break;
                }
                fputs(recvbuf, stdout);
            }
            printf("parent close\n");
            kill(pid, SIGUSR1);
            exit(EXIT_SUCCESS);
        }
    }

    return 0;
}







