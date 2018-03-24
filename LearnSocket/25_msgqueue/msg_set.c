//
//  msg_set.c
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
    // 设置消息队列的权限
    sscanf("600", "%ho", &buf.msg_perm.mode);

    msgctl(msgid, IPC_SET, &buf);

    return 0;
}




