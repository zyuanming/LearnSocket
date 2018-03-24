//
//  srv21.c
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

void echo_srv(int conn)
{
    char recvbuf[1024];
    int n;
    while (1) {
        memset(recvbuf, 0, sizeof(recvbuf));
        n = read(conn, recvbuf, sizeof(recvbuf));
        if (n == -1) {
            if (errno == EINTR) {
                continue;
            }
            ERR_EXIT("read");
        } else if (n == 0) {
            printf("client close\n");
            break;
        }

        fputs(recvbuf, stdout);
        write(conn, recvbuf, strlen(recvbuf));
    }
    close(conn);
}



// gcc -Wall -g main.c -o main
// UNIX 域流式套接字编程,本地进程间通信
int main(void)
{
    int listenfd;
    if ((listenfd = socket(PF_UNIX, SOCK_STREAM, 0)) < 0) {
        ERR_EXIT("socket");
    }
    // 删除 test_socket 文件
    unlink("test_socket");
    struct sockaddr_un servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sun_family = AF_UNIX;
    strcpy(servaddr.sun_path, "test_socket");

    // bind 成功会创建一个文件,权限为 0777 & ~umask
    // 绑定UNIX域的SOCKET地址时,会在当前目录下生成一个 test_socket 的文件
    // 执行命令: ls -l test_socket
    // srwxr-xr-x  1 zhangyuanming  staff  0 Mar 22 14:39 test_socket
    // 前面的 s 表示这是一个socket 套接字文件
    if (bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        ERR_EXIT("bind");
    }

    if (listen(listenfd, SOMAXCONN) < 0) {
        ERR_EXIT("listen");
    }

    int conn;
    pid_t pid;
    while (1) {
        conn = accept(listenfd, NULL, NULL);
        if (conn == -1) {
            if (errno == EINTR) {
                continue;
            }
            ERR_EXIT("accept");
        }

        pid = fork();
        if (pid == -1) {
            ERR_EXIT("fork");
        }

        if (pid == 0) {
            close(listenfd);
            echo_srv(conn);
            exit(EXIT_SUCCESS);
        }
        close(conn);
    }

    return 0;
}










