//
//  msg_get.c
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



// > ipcs
// 列举当前系统的所有的进程间通信的对象(消息队列,信号量,共享内存)
// > ipcrm -q  ID
// 删除某个进程间通信对象
// > ipcrm -Q KEY
// 删除某个进程间通信对象
int main(void)
{
    int msgid;
    // msgid = msgget(1234, 0666);
    // msgget 相当于open 函数,如果消息队列不存在,同时又不指定 IPC_CREAT,则会创建失败
    msgid = msgget(1234, 0666 | IPC_CREAT);

    // 如果指定 IPC_EXCL 和 IPC_CREAT,那么如果文件已经存在了,则会出错
    // msgid = msgget(1234, 0666 | IPC_CREAT | IPC_EXCL);

    // 如果指定 IPC_PRIVATE, 则每次调用都创建一个KEY 为 0的 新消息队列
    // 不同的两个进程将不能共享这个消息队列,父子进程和兄弟进程可以共享
    // 同时第二个参数的选项是可以被忽略的,没有指定 IPC_CREAT 也会创建成功
    // msgid = msgget(IPC_PRIVATE, 0666 | IPC_CREAT | IPC_EXCL);


    // 打开一个指定的消息队列
    // msgid = msgget(1234, 0);

    if (msgid == -1) {
        ERR_EXIT("msgget");
    }

    printf("msgget succ\n");
    printf("msgid=%d\n", msgid);

    return 0;
}
