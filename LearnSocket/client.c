//
//  client.c
//  test_network
//
//  Created by Zhang Yuanming on 2/25/18.
//  Copyright © 2018 None. All rights reserved.
//

#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <errno.h>
#include <netinet/in.h>
#include <unistd.h>

#define ERR_EXIT(m) \
    do \
    { \
        perror(m); \
        exit(EXIT_FAILURE); \
    } while(0)

void client() {
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

    char sendbuf[10] = {0};
    char recvbuf[10] = {0};
    while (fgets(sendbuf, sizeof(sendbuf), stdin) != NULL) {
        write(sock, sendbuf, strlen(sendbuf));
        read(sock, recvbuf, sizeof(recvbuf));
        fputs(recvbuf, stdout);
        printf("reset....");
//        memset(sendbuf, 0, sizeof(sendbuf));
//        memset(sendbuf, 0, sizeof(recvbuf));
    }
    close(sock);
    
}
