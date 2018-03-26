//
//  sem.c
//  LearnSocket
//
//  Created by Zhang Yuanming on 3/26/18.
//  Copyright © 2018 HansonStudio. All rights reserved.
//

#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/sem.h>

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

int sem_create(key_t key)
{
    int semid;
    semid = semget(key, 1, IPC_CREAT | IPC_EXCL | 0666);
    if (semid == -1) {
        ERR_EXIT("semget");
    }

    return semid;
}


int sem_open1(key_t key)
{
    int semid;
    semid = semget(key, 0, 0);
    if (semid == -1) {
        ERR_EXIT("semget");
    }

    return 0;
}

int sem_setval(int semid, int val)
{
    union semun su;
    su.val = val;
    int ret;
    ret = semctl(semid, 0, SETVAL, su);
    if (ret == -1) {
        ERR_EXIT("semctl");
    }

    return 0;
}

int sem_getval(int semid)
{
    int ret;
    ret = semctl(semid, 0, GETVAL, 0);
    if (ret == -1) {
        ERR_EXIT("semctl");
    }

    return ret;
}

int sem_d(int semid)
{
    int ret;
    ret = semctl(semid, 0, IPC_RMID, 0);
    if (ret == -1) {
        ERR_EXIT("semctl");
    }

    return 0;
}


int sem_p(int semid)
{
    struct sembuf sb = {0, -1, 0};
    int ret;
    ret = semop(semid, &sb, 1);
    if (ret == -1) {
        ERR_EXIT("semop");
    }

    return ret;
}


int sem_v(int semid)
{
    struct sembuf sb = {0, 1, 0};
    int ret;
    ret = semop(semid, &sb, 1);
    if (ret == -1) {
        ERR_EXIT("semop");
    }

    return ret;
}

int main(int argc, char *argv[])
{

    int semid;
    semid = sem_create(1234);
    sleep(5);
    sem_d(semid);

    return 0;
}








