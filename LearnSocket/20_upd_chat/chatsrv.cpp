//
//  chatsrv.cpp
//  LearnSocket
//
//  Created by Zhang Yuanming on 3/29/18.
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








#include <list>
#include <algorithm>

using namespace std;

// C2S
#define C2S_LOGIN             0x01
#define C2S_LOOUT             0x02
#define C2S_ONLINE_USER       0x03

#define MSG_LEN               512

// S2C
#define S2C_LOGIN_OK          0x01
#define S2C_ALREADY_LOGINED   0x02
#define S2C_SOMEONE_LOGIN     0x03
#define S2C_SOMEONE_LOGOUT    0x04
#define S2C_ONLINE_USER       0x05

// C2C
#define C2C_CHAT              0x06

typedef struct message
{
    int cmd;
    char body[MSG_LEN];
} MESSAGE;

typedef struct user_info
{
    char username[16];
    unsigned int ip;
    unsigned short port;
} USER_INFO;

typedef struct chat_msg
{
    char username[16];
    char msg[100];
} CHAT_MSG;

typedef list<USER_INFO> USER_LIST;









#define ERR_EXIT(m) \
do \
{ \
perror(m); \
exit(EXIT_FAILURE); \
} while(0)



// 聊天室成员列表
USER_LIST client_list;

void do_login(MESSAGE& msg, int sock, struct sockaddr_in *cliaddr);
void do_logout(MESSAGE& msg, int sock, struct sockaddr_in *cliaddr);
void do_sendlist(int sock, struct sockaddr_in *cliaddr);


void do_login(MESSAGE& msg, int sock, struct sockaddr_in *cliaddr)
{
    USER_INFO user;
    strcpy(user.username, msg.body);
    user.ip = cliaddr->sin_addr.s_addr;
    user.port = cliaddr->sin_port;

    // 查找用户
    USER_LIST::iterator it;
    for (it = client_list.begin(); it != client_list.end(); ++it) {
        if (strcmp(it->username, msg.body) == 0) {
            break;
        }
    }

    if (it == client_list.end()) {   // 没找到用户
        printf("has a user login: %s <-> %s:%d\n", msg.body, inet_ntoa(cliaddr->sin_addr), ntohs(cliaddr->sin_port));
        client_list.push_back(user);

        // 登录成功应答
        MESSAGE reply_msg;
        memset(&reply_msg, 0, sizeof(reply_msg));
        reply_msg.cmd = htonl(S2C_LOGIN_OK);
        sendto(sock, &reply_msg, sizeof(msg), 0, (struct sockaddr *)cliaddr, sizeof(struct sockaddr_in));

        int count = htonl((int)client_list.size());
        // 发送在线人数
        sendto(sock, &count, sizeof(int), 0, (struct sockaddr *)cliaddr, sizeof(struct sockaddr_in));

        printf("sending user list information to: %s <-> %s:%d\n", msg.body, inet_ntoa(cliaddr->sin_addr), ntohs(cliaddr->sin_port));
        // 发送在线列表
        for (it = client_list.begin(); it != client_list.end(); ++it) {
            sendto(sock, &*it, sizeof(USER_INFO), 0, (struct sockaddr *)cliaddr, sizeof(struct sockaddr_in));
        }

        // 向其它用户通知有新用户登录
        for (it = client_list.begin(); it != client_list.end(); ++it) {
            if (strcmp(it->username, msg.body) == 0) {
                continue;
            }
            struct sockaddr_in peeraddr;
            memset(&peeraddr, 0, sizeof(peeraddr));
            peeraddr.sin_family = AF_INET;
            peeraddr.sin_port = it->port;
            peeraddr.sin_addr.s_addr = it->ip;

            msg.cmd = htonl(S2C_SOMEONE_LOGIN);
            memcpy(msg.body, &user, sizeof(user));

            if (sendto(sock, &msg, sizeof(msg), 0, (struct sockaddr*)&peeraddr, sizeof(peeraddr)) == -1) {
                ERR_EXIT("sendto");
            }
        }

    } else {  // 找到用户
        printf("user %s has already logined\n", msg.body);

        MESSAGE reply_msg;
        memset(&reply_msg, 0, sizeof(reply_msg));
        reply_msg.cmd = htonl(S2C_ALREADY_LOGINED);
        sendto(sock, &reply_msg, sizeof(reply_msg), 0, (struct sockaddr *)cliaddr, sizeof(struct sockaddr_in));
    }
}


void do_logout(MESSAGE& msg, int sock, struct sockaddr_in *cliaddr)
{
    printf("has user logout : %s <-> %s:%d\n", msg.body, inet_ntoa(cliaddr->sin_addr), ntohs(cliaddr->sin_port));

    USER_LIST::iterator it;
    for (it = client_list.begin(); it != client_list.end(); ++it) {
        if (strcmp(it->username, msg.body) == 0) {
            break;
        }
    }
    if (it != client_list.end()) {
        client_list.erase(it);
    }

    // 向其它用户通知有用户登出
    for (it = client_list.begin(); it != client_list.end(); ++it) {
        if (strcmp(it->username, msg.body) == 0) {
            continue;
        }

        struct sockaddr_in peeraddr;
        memset(&peeraddr, 0, sizeof(peeraddr));
        peeraddr.sin_family = AF_INET;
        peeraddr.sin_port = it->port;
        peeraddr.sin_addr.s_addr = it->ip;

        msg.cmd = htonl(S2C_SOMEONE_LOGOUT);
        if (sendto(sock, &msg, sizeof(msg), 0, (struct sockaddr*)&peeraddr, sizeof(peeraddr)) == -1) {
            ERR_EXIT("sendto");
        }
    }


}

void chat_srv(int sock)
{
    struct sockaddr_in cliaddr;
    socklen_t clilen;
    int n;
    MESSAGE msg;
    while (1) {
        memset(&msg, 0, sizeof(msg));
        clilen = sizeof(cliaddr);
        n = recvfrom(sock, &msg, sizeof(msg), 0, (struct sockaddr*)&cliaddr, &clilen);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            ERR_EXIT("recvfrom");
        }

        int cmd = ntohl(msg.cmd);
        switch (cmd) {
            case C2S_LOGIN:
                do_login(msg, sock, &cliaddr);
                break;
            case C2S_LOOUT:
                do_logout(msg, sock, &cliaddr);
                break;
            case C2S_ONLINE_USER:
                do_sendlist(sock, &cliaddr);
                break;
            default:
                break;
        }
    }
}

void do_sendlist(int sock, struct sockaddr_in *cliaddr)
{
    MESSAGE msg;
    msg.cmd = htonl(S2C_ONLINE_USER);
    sendto(sock, (const char*)&msg, sizeof(msg), 0, (struct sockaddr*)cliaddr, sizeof(struct sockaddr_in));

    int count = htonl((int)client_list.size());
    // 发送在线用户数
    sendto(sock, (const char*)&count, sizeof(int), 0, (struct sockaddr*)cliaddr, sizeof(struct sockaddr_in));

    // 发送在线用户列表
    USER_LIST::iterator it;
    for (it = client_list.begin(); it != client_list.end(); ++it) {
        sendto(sock, &*it, sizeof(USER_INFO), 0, (struct sockaddr*)cliaddr, sizeof(struct sockaddr_in));
    }

}



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

    chat_srv(sock);

    return 0;
}



