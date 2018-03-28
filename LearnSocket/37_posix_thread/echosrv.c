//
//  echosrv.c
//  LearnSocket
//
//  Created by Zhang Yuanming on 3/28/18.
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
#include <pthread.h>
#include <unistd.h>


#define ERR_EXIT(m) \
do \
{ \
perror(m); \
exit(EXIT_FAILURE); \
} while(0)

void echo_srv(int conn)
{
    char recvbuf[1024];
    while (1) {
        memset(recvbuf, 0, sizeof(recvbuf));
        ssize_t ret = read(conn, recvbuf, sizeof(recvbuf));
        if (ret == -1) {
            ERR_EXIT("read");
        } else if (ret == 0) {
            printf("client close\n");
            break;
        }

        fputs(recvbuf, stdout);
        write(conn, recvbuf, ret);
    }
    close(conn);
}

void* thread_routine(void *arg)
{
    pthread_detach(pthread_self());
//    int conn = (int)arg;
    int conn = *((int *)arg);
    free(arg);
    echo_srv(conn);
    printf("exiting thread....\n");
    return NULL;
}


// gcc -Wall -g main.c -o main
// UNIX 域流式套接字编程,本地进程间通信
int main(void)
{
    int listenfd;
    if ((listenfd = socket(PF_UNIX, SOCK_STREAM, 0)) < 0) {
        ERR_EXIT("socket");
    }
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(5188);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    int on = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) {
        ERR_EXIT("setsockopt");
    }

    if (bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        ERR_EXIT("bind");
    }

    if (listen(listenfd, SOMAXCONN) < 0) {
        ERR_EXIT("listen");
    }

    struct sockaddr_in peeraddr;
    socklen_t peerlen = sizeof(peeraddr);
    int conn;

    while (1) {
        if ((conn = accept(listenfd, (struct sockaddr*)&peeraddr, &peerlen)) < 0) {
            ERR_EXIT("accept");
        }
        printf("ip=%s port=%d\n", inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));

        pthread_t pid;
        int ret;

        int *p = malloc(sizeof(int));
        *p = conn;
        if ((ret = pthread_create(&pid, NULL, thread_routine, p)) != 0) {
//        if ((ret = pthread_create(&pid, NULL, thread_routine, (void *)conn)) != 0) {
            fprintf(stderr, "pthread_create:%s\n", strerror(ret));
            exit(EXIT_FAILURE);
        }
    }

    
    return 0;
}




