//
//  echocli.c
//  LearnSocket
//
//  Created by Zhang Yuanming on 3/20/18.
//  Copyright © 2018 HansonStudio. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>



#define ERR_EXIT(m) \
do \
{ \
perror(m); \
exit(EXIT_FAILURE); \
} while(0)


void echo_cli(int sock)
{
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(5188);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // UDP也是可以调用连接的,一旦连接了,这个socket 只能向这个地址发送数据
    // 同时还能收到异步的ICMP错误
    connect(sock, (struct sockaddr*)&servaddr, sizeof(servaddr));

    int ret;
    char sendbuf[1024] = {0};
    char recvbuf[1024] = {0};
    while (fgets(sendbuf, sizeof(sendbuf), stdin) != NULL) {
        sendto(sock, sendbuf, strlen(sendbuf), 0, (struct sockaddr*)&servaddr, sizeof(servaddr));
//        sendto(sock, sendbuf, strlen(sendbuf), 0, NULL, 0);

        // 如果没有连接这个套接字,那么如果客户端启动时,服务器端没有启动,会一直
        // 阻塞在recvfrom方法中,客户端收不到异步ICMP错误报文
        ret = recvfrom(sock, recvbuf, sizeof(recvbuf), 0, NULL, NULL);

        if (ret == -1) {
            if (errno == EINTR) {
                continue;
            }
            ERR_EXIT("recvfrom");
        }

        fputs(recvbuf, stdout);
        memset(sendbuf, 0, sizeof(sendbuf));
        memset(recvbuf, 0, sizeof(recvbuf));
    }
    close(sock);
}



int main(void)
{
    int sock;
    if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
        ERR_EXIT("socket");
    }

    echo_srv(sock);
    return 0;

}



