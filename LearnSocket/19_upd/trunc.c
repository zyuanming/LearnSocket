//
//  trunc.c
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

#define ERR_EXIT(m) \
do \
{ \
perror(m); \
exit(EXIT_FAILURE); \
} while(0)



int main(void)
{
    int sock;
    if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
        ERR_EXIT("socket");
    }

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(5188);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);


    if (bind(sock, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        ERR_EXIT("bind");
    }

    sendto(sock, "ABCD", 4, 0, (struct sockaddr*)&servaddr, sizeof(servaddr));

    char recvbuf[1];
    int n;
    int i;
    for (i = 0; i < 4; i++) {
        n = recvfrom(sock, recvbuf, sizeof(recvbuf), 0, NULL, NULL);
        if (n == -1) {
            if (errno == EINTR) {
                continue;
            }
            ERR_EXIT("recvfrom");
        } else if (n > 0) {
            printf("n=%d_%c\n", n, recvbuf[i]);
        }
    }
    return 0;

}
