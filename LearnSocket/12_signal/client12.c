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
#include <signal.h>

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
            } else if (nwriten == 0) {
                continue;
            }
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


        // writen(sock, sendbuf, strlen(sendbuf));

        // 模拟发生 SIG_PIPE 信号,就是客户端在收到Fin信号后,连续两次发送数据
        writen(sock, sendbuf, 1);
        writen(sock, sendbuf + 1, strlen(sendbuf) - 1);

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
        memset(sendbuf, 0, sizeof(recvbuf));
    }
    close(sock);
}



void handle_sigpipe(int sig) {
    printf("recv a sig=%d\n", sig);
}



/* gcc -Wall -g main.c -o main
 启动服务端和客户端后
 通过 ps -ef | grep server 找到服务端进程号
 通过 kill 来杀掉 服务端进程,
 使用 netstat -an | grep tcp | grep 5188 查看端口详情
 tcp4       0      0  127.0.0.1.5188         127.0.0.1.55097        FIN_WAIT_2
 tcp4       0      0  127.0.0.1.55097        127.0.0.1.5188         CLOSE_WAIT
 tcp4       0      0  127.0.0.1.5188         *.*                    LISTEN
 会发现服务端端口处于 FIN_WAIT_2 状态,
 客户端端口处于 CLOSE_WAIT状态, 为什么呢?
 因为客户端在收到 FIN x ACK 后立即回传了 ACK 后,就没有能够 返回0给服务端(调用close方法),
 导致服务端一直处于 FIN_WAIT_2 状态.

 */
int main(int argc, const char * argv[]) {

    signal(SIGPIPE, handle_sigpipe);
//    忽略管道信号
//    signal(SIGPIPE, SIG_IGN);
    int sock;
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        ERR_EXIT("socket");
    }

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(5188);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        ERR_EXIT("connect");
    }


    struct sockaddr_in localaddr;
    socklen_t addrlen = sizeof(localaddr);
    if (getsockname(sock, (struct sockaddr*)&localaddr, &addrlen) < 0) {
        ERR_EXIT("getsockname");
    }

    printf("ip=%s port=%d\n", inet_ntoa(localaddr.sin_addr), ntohs(localaddr.sin_port));

    echo_cli(sock);

    return 0;
}











