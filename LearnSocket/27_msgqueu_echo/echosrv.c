//
//  echosrv.c
//  LearnSocket
//
//  Created by Zhang Yuanming on 3/24/18.
//  Copyright © 2018 HansonStudio. All rights reserved.
//


#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>


#define ERR_EXIT(m) \
do \
{ \
perror(m); \
exit(EXIT_FAILURE); \
} while(0)


struct msgbuf1 {
    long mtype;
    char mtext[1];
};

#define MSGMAX 2000


void echo_srv(int msgid)
{
    struct msgbuf1 msg;
    memset(&msg, 0, sizeof(msg));
    int n;
    while (1) {
        if ((n = msgrcv(msgid, &msg, MSGMAX, 1, 0)) < 0) {
            ERR_EXIT("msgrcv");
        }

        int pid;
        pid = *((int *)msg.mtext);

        fputs(msg.mtext + 4, stdout);
        msg.mtype = pid;
        msgsnd(msgid, &msg, n, 0);
    }
}

int main(int argc, char *argv[])
{

    int msgid;
    msgid = msgget(1234, IPC_CREAT | 0666);
    if (msgid == -1) {
        ERR_EXIT("msgget");
    }

    echo_srv(msgid);

    return 0;
}


