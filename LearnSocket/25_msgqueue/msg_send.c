//
//  msg_send.c
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


int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <bytes> <type>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int len = atoi(argv[1]);
    int type = atoi(argv[2]);
    int msgid;
    // 打开消息队列
    msgid = msgget(1234, 0);

    if (msgid == -1) {
        ERR_EXIT("msgget");
    }

    printf("msgget succ\n");
    printf("msgid=%d\n", msgid);

    struct msgbuf1 *ptr;
    ptr = (struct msgbuf1*)malloc(sizeof(long) + len);
    ptr->mtype = type;
    // 0表示消息队列满的时候以阻塞的方式发送
    // if (msgsnd(msgid, ptr, len, 0) < 0) {
    if (msgsnd(msgid, ptr, len, IPC_NOWAIT) < 0) {
        ERR_EXIT("msgsnd");
    }

    return 0;
}





