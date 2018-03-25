//
//  shmwrite.c
//  LearnSocket
//
//  Created by Zhang Yuanming on 3/25/18.
//  Copyright © 2018 HansonStudio. All rights reserved.
//

#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <sys/mman.h>

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

typedef struct stu
{
    char name[32];
    int age;
} STU;


int main(int argc, char *argv[])
{
    int shmid;
    shmid = shmget(1234, sizeof(STU), IPC_CREAT | 0666);
    if (shmid == -1) {
        ERR_EXIT("shmget");
    }

    STU *p;
    p = shmat(shmid, NULL, 0);
    if (p == (void *)-1) {
        ERR_EXIT("shmat");
    }

    strcpy(p->name, "lisi");
    p->age = 20;

    while (1) {
        if (memcpy(p, "quit", 4) == 0) {
            break;
        }
    }

    // 将共享内存段与当前进程脱离
    shmdt(p);

    // 删除共享内存段
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}









