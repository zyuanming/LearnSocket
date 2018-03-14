//
//  sysutil.c
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
#include "sysutil.h"
#include <fcntl.h>


/**
 read timeout 读超时检测函数,不含读操作

 @param fd 文件描述符
 @param wait_seconds 等待超时秒数,如果为0表示不检测超时
 @return 成功(未超时)返回0, 失败返回-1, 超时返回-1并且errno = ETIMEDOUT
 */
int read_timeout(int fd, unsigned int wait_seconds)
{
    int ret = 0;
    if (wait_seconds > 0) {
        fd_set read_fdset;
        struct timeval timeout;

        FD_ZERO(&read_fdset);
        FD_SET(fd, &read_fdset);

        timeout.tv_sec = wait_seconds;
        timeout.tv_usec = 0;
        do {
            ret = select(fd + 1, &read_fdset, NULL, NULL, &timeout);
        } while(ret < 0 && errno == EINTR);

        if (ret == 0) {
            ret = -1;
            errno = ETIMEDOUT;
        } else if (ret == 1) {
            ret = 0;
        }
    }

    return ret;
}


/**
 read timeout 写超时检测函数,不含写操作

 @param fd 文件描述符
 @param wait_seconds 等待超时秒数,如果为0表示不检测超时
 @return 成功(未超时)返回0, 失败返回-1, 超时返回-1并且errno = ETIMEDOUT
 */
int write_timeout(int fd, unsigned int wait_seconds)
{
    int ret = 0;
    if (wait_seconds > 0) {
        fd_set write_fdset;
        struct timeval timeout;

        FD_ZERO(&write_fdset);
        FD_SET(fd, &write_fdset);

        timeout.tv_sec = wait_seconds;
        timeout.tv_usec = 0;
        do {
            ret = select(fd + 1, NULL, &write_fdset, NULL, &timeout);
        } while(ret < 0 && errno == EINTR);

        if (ret == 0) {
            ret = -1;
            errno = ETIMEDOUT;
        } else if (ret == 1) {
            ret = 0;
        }
    }

    return ret;
}



/**
 accept timeout 带超时的accept

 @param fd 套接字
 @param addr 输出参数, 返回对方的地址
 @param wait_seconds 等待超时秒数,如果为0表示正常模式
 @return 成功(未超时)返回已连接套接字,超时返回-1并且errno = ETIMEDOUT
 */
int accept_timeout(int fd, struct sockaddr_in *addr, unsigned int wait_seconds)
{
    int ret = 0;
    socklen_t addrlen = sizeof(struct sockaddr_in);

    if (wait_seconds > 0) {
        fd_set accept_fdset;
        struct timeval timeout;
        FD_ZERO(&accept_fdset);
        FD_SET(fd, &accept_fdset);
        timeout.tv_usec = wait_seconds;
        timeout.tv_usec = 0;

        do {
            ret = select(fd + 1, &accept_fdset, NULL, NULL, &timeout);
        } while (ret < 0 && errno == EINTR);
        if (ret == -1) {
            return -1;
        } else if (ret == 0) {
            errno = ETIMEDOUT;
            return -1;
        }
    }

    if (addr != NULL) {
        ret = accept(fd, (struct sockaddr*)addr, &addrlen);
    } else {
        ret = accept(fd, NULL, NULL);
    }

    if (ret == -1) {
        ERR_EXIT("accept");
    }

    return ret;
}



/**
 设置I/O为非阻塞模式

 @param fd 文件描述符
 */
void active_nonblock(int fd)
{
    int ret;
    int flags = fcntl(fd, F_GETFL);
    if (flags == -1) {
        ERR_EXIT("fcntl");
    }

    flags |= O_NONBLOCK;
    ret = fcntl(fd, F_SETFL, flags);
    if (ret == -1) {
        ERR_EXIT("fcntl");
    }
}


/**
 设置I/O为阻塞模式

 @param fd 文件描述符
 */
void deactive_nonblock(int fd)
{
    int ret;
    int flags = fcntl(fd, F_GETFL);
    if (flags == -1) {
        ERR_EXIT("fcntl");
    }

    flags &= ~O_NONBLOCK;
    ret = fcntl(fd, F_SETFL, flags);
    if (ret == -1) {
        ERR_EXIT("fcntl");
    }
}



/**
 连接超时

 @param fd 套接字
 @param addr 要连接的对方的地址
 @param wait_seconds 等待超时秒数,如果为0表示正常模式
 @return 成功(未超时)返回0, 失败返回-1,超时返回-1并且errno = ETIMEDOUT
 */
int connect_timeout(int fd, struct sockaddr_in *addr, unsigned int wait_seconds)
{
    int ret;
    socklen_t addrlen = sizeof(struct sockaddr_in);

    if (wait_seconds > 0) {
        active_nonblock(fd);
    }

    // 如果不把套接字设为非阻塞的模式,那么调用connnect 函数就会一直阻塞
    ret = connect(fd, (struct sockaddr*)addr, addrlen);
    // 正在处理中
    if (ret < 0 && errno == EINPROGRESS) {
        fd_set connect_fdset;
        struct timeval timeout;
        FD_ZERO(&connect_fdset);
        FD_SET(fd, &connect_fdset);
        timeout.tv_sec = wait_seconds;
        timeout.tv_usec = 0;

        do {
            // 一旦连接建立,套接字就可以写
            ret = select(fd + 1, NULL, &connect_fdset, NULL, &timeout);
        } while(ret < 0 && errno == EINTR);

        if (ret == 0) {
            ret = -1;
            errno = ETIMEDOUT;
        } else if (ret < 0) {
            return -1;
        } else if (ret == 1) {
            // ret 返回为1,可能有两种情况:
            // 一种是连接建立成功,
            // 一种是套接字产生错误,此时错误信息不会保存至errno变量中,因此,需要调用
            // getsockopt 函数获取.
            int err;
            socklen_t socklen = sizeof(err);
            int sockoptret = getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &socklen);
            if (sockoptret == -1) {
                return -1;
            }

            if (err == 0) {
                ret = 0;
            } else {
                errno = err;
                ret = -1;
            }
        }
    }

    if (wait_seconds > 0) {
        deactive_nonblock(fd);
    }

    return ret;
}










