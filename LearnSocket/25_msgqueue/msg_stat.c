//
//  msg_stat.c
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



int main(void)
{
    int msgid;
    // 打开消息队列
    msgid = msgget(1234, 0);


    if (msgid == -1) {
        ERR_EXIT("msgget");
    }

    printf("msgget succ\n");
    printf("msgid=%d\n", msgid);


    struct msqid_ds buf;
    msgctl(msgid, IPC_STAT, &buf);
    // 消息队列的权限
    printf("mode=%o\n", buf.msg_perm.mode);

    // 消息队列的字节数
    printf("bytes=%ld\n", buf.msg_cbytes);

    // 消息队列的消息个数
    printf("number=%lu\n", buf.msg_qnum);

    // 消息队列所能容纳的最大字节数
    printf("max bytes on the queue=%lu\n", buf.msg_qbytes);

    return 0;
}




