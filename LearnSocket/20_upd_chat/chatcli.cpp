//
//  chatcli.c
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



// 当前用户名
char username[16];

// 聊天室成员列表
USER_LIST client_list;

void do_someone_login(MESSAGE& msg);
void do_someone_loout(MESSAGE& msg);
void do_getlist(int sock);
void do_chat(const MESSAGE& msg);

void parse_cmd(char *cmdline, int sock, struct sockaddr_in *servaddr);
bool sendnsgto(int sock, char * username, char *msg);

void parse_cmd(char *cmdline, int sock, struct sockaddr_in *servaddr)
{
    char cmd[10] = {0};
    char *p;
    p = strchr(cmdline, ' ');
    if (p != NULL) {
        *p = '\0';
    }

    strcpy(cmd, cmdline);

    if (strcmp(cmd, "exit") == 0) {
        MESSAGE msg;
        memset(&msg, 0, sizeof(msg));
        msg.cmd = htonl(C2S_LOOUT);
        strcpy(msg.body, username);

        if (sendto(sock, &msg, sizeof(msg), 0, (struct sockaddr *)servaddr, sizeof(struct sockaddr_in)) == -1) {
            ERR_EXIT("sendto");
        }

        printf("user %s has Logout server\n", username);
        exit(EXIT_SUCCESS);
    } else if (strcmp(cmd, "send") == 0) {
        char peername[16] = {0};
        char msg[MSG_LEN] = {0};

        while (*p++ == ' ');
        char *p2;
        p2 = strchr(p, ' ');
        if (p2 == NULL) {
            printf("bad command\n");
            printf("\nCommands are:\n");
            printf("send username msg\n");
            printf("list\n");
            printf("exit\n");
            printf("\n");
            return;
        }
        *p2 = '\0';
        strcpy(peername, p);
        while (*p2++ == ' ');
        strcpy(msg, p2);
        sendnsgto(sock, peername, msg);
    } else if (strcmp(cmd, "list") == 0) {
        MESSAGE msg;
        memset(&msg, 0, sizeof(msg));
        msg.cmd = htonl(C2S_ONLINE_USER);

        if (sendto(sock, &msg, sizeof(msg), 0, (struct sockaddr*)servaddr, sizeof(struct sockaddr_in)) == -1) {
            ERR_EXIT("sendto");
        }
    } else {
        printf("bad command\n");
        printf("\nCommands are:\n");
        printf("send username msg\n");
        printf("list\n");
        printf("exit\n");
        printf("\n");
    }
}

bool sendnsgto(int sock, char *name, char *msg)
{
    if (strcmp(name, username) == 0) {
        printf("can't send message to self\n");
        return false;
    }

    USER_LIST::iterator it;
    for (it = client_list.begin(); it != client_list.end(); ++it) {
        if (strcmp(it->username, name) == 0) {
            break;
        }
    }

    if (it == client_list.end()) {
        printf("user %s has not logined server\n", name);
        return false;
    }

    MESSAGE m;
    memset(&m, 0, sizeof(m));
    m.cmd = htonl(C2C_CHAT);

    CHAT_MSG cm;
    strcpy(cm.username, username);
    strcpy(cm.msg, msg);

    memcpy(m.body, &cm, sizeof(cm));

    struct sockaddr_in peeraddr;
    memset(&peeraddr, 0, sizeof(peeraddr));
    peeraddr.sin_family = AF_INET;
    peeraddr.sin_addr.s_addr = it->ip;
    peeraddr.sin_port = it->port;

    in_addr tmp;
    tmp.s_addr = it->ip;

    printf("sending message [%s] to user [%s] <-> %s:%d\n", msg, name, inet_ntoa(tmp), ntohl(it->port));
    sendto(sock, (const char*)&m, sizeof(m), 0, (struct sockaddr *)&peeraddr, sizeof(peeraddr));

    return true;
}


void do_getlist(int sock)
{
    int count;
    recvfrom(sock, &count, sizeof(int), 0, NULL, NULL);
    printf("has %d users logined server\n", ntohl(count));
    client_list.clear();

    int n = ntohl(count);
    for (int i = 0; i < n; i++) {
        USER_INFO user;
        recvfrom(sock, &user, sizeof(USER_INFO), 0, NULL, NULL);
        client_list.push_back(user);
        in_addr tmp;
        tmp.s_addr = user.ip;

        printf("%s <-> %s:%d\n", user.username, inet_ntoa(tmp), ntohs(user.port));
    }
}

void do_someone_login(MESSAGE& msg)
{
    USER_INFO *user = (USER_INFO *)msg.body;
    in_addr tmp;
    tmp.s_addr = user->ip;
    printf("%s <-> %s:%d has logined server\n", user->username, inet_ntoa(tmp), ntohl(user->port));
    client_list.push_back(*user);
}

void do_someone_logout(MESSAGE& msg)
{
    USER_LIST::iterator it;
    for (it = client_list.begin(); it != client_list.end(); ++it) {
        if (strcmp(it->username, msg.body) == 0) {
            break;
        }
    }

    if (it != client_list.end()) {
        client_list.erase(it);
    }
    printf("user %s has logout server\n", msg.body);
}

void do_chat(const MESSAGE& msg)
{
    CHAT_MSG *cm = (CHAT_MSG*)msg.body;
    printf("recv a msg [%s] from [%s]\n", cm->msg, cm->username);
}

void chat_cli(int sock)
{
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(5188);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    struct sockaddr_in peeraddr;
    socklen_t peerlen;

    MESSAGE msg;
    while (1) {
        memset(username, 0, sizeof(username));
        printf("please input your name:");
        fflush(stdout);
        scanf("%s", username);

        memset(&msg, 0, sizeof(msg));
        msg.cmd = htonl(C2S_LOGIN);
        strcpy(msg.body, username);

        sendto(sock, &msg, sizeof(msg), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));

        memset(&msg, 0, sizeof(msg));
        recvfrom(sock, &msg, sizeof(msg), 0, NULL, NULL);
        int cmd = ntohl(msg.cmd);
        if (cmd == S2C_ALREADY_LOGINED) {
            printf("user %s already logined server, please use another username\n", username);
        } else if (cmd == S2C_LOGIN_OK) {
            printf("user %s has logined server\n", username);
            break;
        }
    }

    int count;
    recvfrom(sock, &count, sizeof(int), 0, NULL, NULL);

    int n = ntohl(count);
    printf("has %d users logined server\n", n);

    for (int i = 0; i < n; i++) {
        USER_INFO user;
        recvfrom(sock, &user, sizeof(USER_INFO), 0, NULL, NULL);
        client_list.push_back(user);
        in_addr tmp;
        tmp.s_addr = user.ip;

        printf("%d %s <-> %s:%d\n", i, user.username, inet_ntoa(tmp), ntohs(user.port));
    }

    printf("\nCommands are:\n");
    printf("send username msg\n");
    printf("list\n");
    printf("exit\n");
    printf("\n");

    fd_set rset;
    FD_ZERO(&rset);
    int nready;
    while (1) {
        FD_SET(STDIN_FILENO, &rset);
        FD_SET(sock, &rset);
        nready = select(sock + 1, &rset, NULL, NULL, NULL);
        if (nready == -1) {
            ERR_EXIT("select");
        }
        if (nready == 0) {
            continue;
        }
        if (FD_ISSET(sock, &rset)) {
            peerlen = sizeof(peeraddr);
            memset(&msg, 0, sizeof(msg));
            recvfrom(sock, &msg, sizeof(msg), 0, (struct sockaddr *)&peeraddr, &peerlen);
            int cmd = ntohl(msg.cmd);
            switch (cmd) {
                case S2C_SOMEONE_LOGIN:
                    do_someone_login(msg);
                    break;
                case S2C_SOMEONE_LOGOUT:
                    do_someone_logout(msg);
                    break;
                case S2C_ONLINE_USER:
                    do_getlist(sock);
                    break;
                case C2C_CHAT:
                    do_chat(msg);
                    break;
                default:
                    break;
            }
        }

        if (FD_ISSET(STDIN_FILENO, &rset)) {
            char cmdline[100] = {0};
            if (fgets(cmdline, sizeof(cmdline), stdin) == NULL) {
                break;
            }
            if (cmdline[0] == '\n') {
                continue;
            }
            cmdline[strlen(cmdline) - 1] = '\0';
            parse_cmd(cmdline, sock, &servaddr);
        }
    }

    memset(&msg, 0, sizeof(msg));
    msg.cmd = htonl(C2S_LOOUT);
    strcpy(msg.body, username);

    sendto(sock, (const char *)&msg, sizeof(msg), 0, (struct sockaddr*)&servaddr, sizeof(servaddr));
    close(sock);
}



// g++ -Wall -g chatcli.cpp -o chatcli
int main(void)
{
    int sock;
    if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
        ERR_EXIT("socket");
    }
    chat_cli(sock);
    return 0;
}
