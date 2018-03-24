//
//  send_fd.c
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




// 通过套接字 sock_fd 发送 send_fd 文件描述符
void send_fd(int sock_fd, int send_fd)
{
    int ret;
    struct msghdr msg;
    struct cmsghdr *p_cmsg;   // 结构体指针
    struct iovec vec;
    // CMSG_SPACE 获取整块辅助数据实际占用的长度
    char cmsgbuf[CMSG_SPACE(sizeof(send_fd))];   // 辅助数据的缓冲区
    int *p_fds;
    char sendchar = 0;
    msg.msg_control = cmsgbuf;
    msg.msg_controllen = sizeof(cmsgbuf);
    p_cmsg = CMSG_FIRSTHDR(&msg);   // 获取msg 的第一个辅助消息的指针
    p_cmsg->cmsg_level = SOL_SOCKET;
    p_cmsg->cmsg_type = SCM_RIGHTS;
    p_cmsg->cmsg_len = CMSG_LEN(sizeof(send_fd));
    p_fds = (int*)CMSG_DATA(p_cmsg);
    *p_fds = send_fd;

    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = &vec;  // 指定一个字节的缓冲区,发送最少的数据
    msg.msg_iovlen = 1;
    msg.msg_flags = 0;

    vec.iov_base = &sendchar; // 只有一个字符,因为只发送一个字符
    vec.iov_len = sizeof(sendchar);

    ret = sendmsg(sock_fd, &msg, 0);
    if (ret != 1) {
        ERR_EXIT("sendmsg");
    }

}


int recv_fd(const int sock_fd)
{
    int ret;
    struct msghdr msg;
    char recvchar;
    struct iovec vec;
    int recv_fd;
    char cmsgbuf[CMSG_SPACE(sizeof(recv_fd))];
    struct cmsghdr *p_cmsg;
    int *p_fd;
    vec.iov_base = &recvchar;
    vec.iov_len = sizeof(recvchar);
    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = &vec;
    msg.msg_iovlen = 1;
    msg.msg_control = cmsgbuf;
    msg.msg_controllen = sizeof(cmsgbuf);
    msg.msg_flags = 0;

    p_fd = (int*)CMSG_DATA(CMSG_FIRSTHDR(&msg));
    *p_fd = -1;
    ret = recvmsg(sock_fd, &msg, 0);
    if (ret != 1) {
        ERR_EXIT("recvmsg");
    }

    p_cmsg = CMSG_FIRSTHDR(&msg);  // 取出辅助数据的第一个消息
    if (p_cmsg == NULL) {
        ERR_EXIT("no passed fd");
    }

    p_fd = (int*)CMSG_DATA(p_cmsg);
    recv_fd = *p_fd;

    if (recv_fd == -1) {
        ERR_EXIT("no passed fd");
    }

    return recv_fd;
}


// gcc -Wall -g main.c -o main
// 用socketpair 发送一个文件描述符给父进程,父进程通过
// 这个传过来的文件描述符直接读取内容
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
        close(sockfds[1]);
        int fd = recv_fd(sockfds[0]);
        char buf[1024] = {0};
        read(fd, buf, sizeof(buf));
        printf("buf=%s\n", buf);

    } else if (pid == 0) {
        close(sockfds[0]);
        int fd;
        fd = open("test.txt", O_RDONLY);
        if (fd == -1) {
            ERR_EXIT("open");
        }
        send_fd(sockfds[1], fd);
    }

    return 0;
}



