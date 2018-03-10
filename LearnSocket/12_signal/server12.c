//
//  server10.c
//  LearnSocket
//
//  Created by Zhang Yuanming on 3/8/18.
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

#define ERR_EXIT(m) \
do \
{ \
perror(m); \
exit(EXIT_FAILURE); \
} while(0)




ssize_t recv_peak(int sockfd, void *buf, size_t len)
{
    while (1) {
        int ret = recv(sockfd, buf, len, MSG_PEEK);
        if (ret == -1 && errno == EINTR) {
            continue;
        }
        return ret;
    }
}


// 为了解决粘包问题
ssize_t readn(int fd, const void *buf, size_t count)
{
    size_t nleft = count;
    ssize_t nreadn;
    char *bufp = (char *)buf;

    while (nleft > 0) {
        if ((nreadn = read(fd, bufp, nleft)) < 0) {
            if (errno == EINTR) {
                continue;
            } else if (nreadn == 0) {
                continue;
            } else {
                return -1;
            }
        }

        bufp += nreadn;
        nleft -= nreadn;
    }

    return count;
}

// 读取一行数据为一条信息,解决粘包问题的一个方法
ssize_t readline(int sockfd, void *buf, size_t maxline)
{
    int ret;
    int nread;
    char *bufp = buf;
    int nleft = maxline;
    while (1) {
        ret = recv_peak(sockfd, bufp, nleft);
        if (ret < 0) {
            return ret;
        } else if (ret == 0) {
            return ret;
        }

        nread = ret;
        int i;
        for (i = 0; i < nread; i++) {
            if (bufp[i] == '\n') {
                ret = readn(sockfd, bufp, i+1);
                if (ret != i+1) {
                    exit(EXIT_FAILURE);
                }
                return ret;
            }
        }

        if (nread > nleft) {
            exit(EXIT_FAILURE);
        }

        nleft -= nread;
        ret = readn(sockfd, bufp, nread);
        if (ret != nread) {
            exit(EXIT_FAILURE);
        }
        bufp += nread;
    }

    return -1;
}

void do_service(int conn) {
    char recvbuf[1024];
    int n;
    while (1) {
        memset(&recvbuf, 0, sizeof(recvbuf));
        int ret = readline(conn, recvbuf, 1024);
        if (ret == -1) {
            ERR_EXIT("readline");
        } else if (ret == 0) {
            printf("client close\n");
            break;
        }
        fputs(recvbuf, stdout);
        write(conn, recvbuf, strlen(recvbuf));
    }
}




void handle_sigchld(int sig)
{
    // 捕获子进程的状态
//    wait(NULL);

    // 解决同时多个子进程退出时,丢失信号导致存在僵尸进程
    while (waitpid(-1, NULL, WNOHANG) > 0) ;
}


// gcc -Wall -g main.c -o main
// 观察TCP端口状态:
// netstat -an | grep tcp | grep 5188
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


//    // 1、 忽略这个信号,避免僵尸进程
//    signal(SIGCHLD, SIG_IGN);

    // 2、
    signal(SIGCHLD, handle_sigchld);


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

    pid_t pid;
    while (1) {
        if ((conn = accept(listenfd, (struct sockaddr *)&peeraddr, &peerlen)) < 0) {
            ERR_EXIT("accept");
        }

        printf("ip=%s port=%d\n", inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));

        pid = fork();
        if (pid == -1) {
            ERR_EXIT("fork");
        } else if (pid == 0) {
            close(listenfd);
            do_service(conn);
            exit(EXIT_SUCCESS);
        } else {
            close(conn);
        }
    }

    return 0;
}



