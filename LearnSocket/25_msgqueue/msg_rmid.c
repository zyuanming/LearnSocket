//
//  msg_rmid.c
//  LearnSocket
//
//  Created by Zhang Yuanming on 3/24/18.
//  Copyright © 2018 HansonStudio. All rights reserved.
//

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

    // 删除消息队列
    msgctl(msgid, IPC_RMID, NULL);
    return 0;
}




