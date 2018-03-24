//
//  socket_pair.c
//  LearnSocket
//
//  Created by Zhang Yuanming on 3/22/18.
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


#define ERR_EXIT(m) \
do \
{ \
perror(m); \
exit(EXIT_FAILURE); \
} while(0)






// gcc -Wall -g main.c -o main
// 用socketpair 创建一个全双工的流管道
int main(void)
{
    int sockfds[2];

    if (socketpair(PF_UNIX, SOCK_STREAM, 0, sockfds) < 0) {
        ERR_EXIT("socketpair");
    }

    pid_t pid;
    pid = fork();
    if (pid == 0) {
        ERR_EXIT("fork");
    }
    if (pid > 0) {
        int val = 0;
        close(sockfds[1]);
        while (1) {
            ++val;
            printf("sending data: %d\n", val);
            write(sockfds[0], &val, sizeof(val));
            read(sockfds[0], &val, sizeof(val));
            printf("data received: %d\n", val);
            sleep(1);
        }
    } else if (pid == 0) {
        int val;
        close(sockfds[0]);
        while (1) {
            read(sockfds[1], &val, sizeof(val));
            ++val;
            write(sockfds[1], &val, sizeof(val));
        }
    }

    return 0;
}








