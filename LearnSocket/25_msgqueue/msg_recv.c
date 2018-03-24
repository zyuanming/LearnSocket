//
//  msg_recv.c
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


#define MSGMAX 2000


// 不传参数,表示顺序接收
int main(int argc, char *argv[])
{
    int flag = 0;
    int type = 0;
    int opt;

    while (1) {
        opt = getopt(argc, argv, "nt:");
        // 解析到不认识的参数
        if (opt == '?') {
            exit(EXIT_FAILURE);
        }

        // 所有参数解析完毕
        if (opt == -1) {
            break;
        }

        switch (opt)
        {
            case 'n':
                flag |= IPC_NOWAIT;
                break;
            case 't':
                printf("BBBB\n");
                // 获取t 后面跟的参数
                type = atoi(optarg);
                break;
        }
    }

    int msgid;
    // 打开消息队列
    msgid = msgget(1234, 0);

    if (msgid == -1) {
        ERR_EXIT("msgget");
    }

    printf("msgget succ\n");
    printf("msgid=%d\n", msgid);

    struct msgbuf1 *ptr;
    ptr = (struct msgbuf1*)malloc(sizeof(long) + MSGMAX);
    ptr->mtype = type;
    // 0表示消息队列满的时候以阻塞的方式发送
    // if (msgsnd(msgid, ptr, len, 0) < 0) {
    ssize_t n = 0;
    if ((n = msgrcv(msgid, ptr, MSGMAX, type, flag)) < 0) {
        ERR_EXIT("msgrcv");
    }

    printf("read %zd bytes, type = %ld\n", n, ptr->mtype);

    return 0;
}





