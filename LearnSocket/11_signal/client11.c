//
//  client10.c
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
ssize_t writen(int fd, const void *buf, size_t count)
{
    size_t nleft = count;
    ssize_t nwriten;
    char *bufp = (char *)buf;

    while (nleft > 0) {
        if ((nwriten = write(fd, bufp, nleft)) < 0) {
            if (errno == EINTR) {
                continue;
            } else {
                return -1;
            }
        } else if (nwriten == 0) {
            continue;
        }
        bufp += nwriten;
        nleft -= nwriten;
    }

    return count;
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


void echo_cli(int sock) {
    char sendbuf[1024] = {0};
    char recvbuf[1024] = {0};
    while (fgets(sendbuf, sizeof(sendbuf), stdin) != NULL) {
        writen(sock, sendbuf, strlen(sendbuf));


        int ret = readline(sock, recvbuf, sizeof(recvbuf));
        if (ret == -1) {
            ERR_EXIT("readline");
        } else if (ret == 0) {
            printf("client close\n");
            break;
        }

        fputs(recvbuf, stdout);
        printf("reset....");
        memset(sendbuf, 0, sizeof(sendbuf));
        memset(recvbuf, 0, sizeof(recvbuf));
    }
    close(sock);
}



// gcc -Wall -g main.c -o main
int main(int argc, const char * argv[]) {
    int sock[5] = {0};
    int i;
    for (i = 0; i < 5; i++)
    {
        if ((sock[i] = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
            ERR_EXIT("socket");
        }

        struct sockaddr_in servaddr;
        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(5188);
        servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

        if (connect(sock[i], (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
            ERR_EXIT("connect");
        }


        struct sockaddr_in localaddr;
        socklen_t addrlen = sizeof(localaddr);
        if (getsockname(sock[i], (struct sockaddr*)&localaddr, &addrlen) < 0) {
            ERR_EXIT("getsockname");
        }

        printf("ip=%s port=%d\n", inet_ntoa(localaddr.sin_addr), ntohs(localaddr.sin_port));
    }

    echo_cli(sock[0]);

    return 0;
}











