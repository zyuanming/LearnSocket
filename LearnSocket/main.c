//
//  main.c
//  test_network
//
//  Created by Zhang Yuanming on 2/24/17.
//  Copyright © 2017 None. All rights reserved.
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

#define ERR_EXIT(m) \
    do \
    { \
        perror(m); \
        exit(EXIT_FAILURE); \
    } while(0)



struct packet
{
    int len;
    char buf[1024];
};


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




void do_service(int conn) {
//    char recvbuf[1024];
    struct packet recvbuf;
    int n;
    while (1) {
        memset(&recvbuf, 0, sizeof(recvbuf));
        int ret = readn(conn, &recvbuf.len, 4);
        if (ret == -1) {
            ERR_EXIT("read");
        } else if (ret < 4) {
            printf("client close\n");
            break;
        }
        n = ntohl(recvbuf.len);
        ret = readn(conn, recvbuf.buf, n);
        if (ret == -1) {
            ERR_EXIT("read");
        } else if (ret < n) {
            printf("client close\n");
            break;
        }
        fputs(recvbuf.buf, stdout);
        write(conn, &recvbuf, 4+n);
    }
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































