//
//  cli.c
//  LearnSocket
//
//  Created by Zhang Yuanming on 3/14/18.
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

int main(void)
{
    int count = 0;
    // 验证当前进程能打开的最大文件描述符限制
    while (1) {
        int sock;
        if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
            ERR_EXIT("socket");
        }

        struct sockaddr_in servaddr;
        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(5188);
        servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

        int ret = connect(sock, (struct sockaddr*)&servaddr, sizeof(servaddr));
        if (ret == -1) {
            ERR_EXIT("connect");
        }

        struct sockaddr_in localaddr;
        socklen_t addrlen = sizeof(localaddr);
        if (getsockname(sock, (struct sockaddr*)&localaddr, &addrlen) < 0) {
            ERR_EXIT("getsockname");
        }

        printf("ip=%s port=%d\n", inet_ntoa(localaddr.sin_addr), ntohs(localaddr.sin_port));

        printf("count = %d\n", ++count);
    }

    return 0;
}
