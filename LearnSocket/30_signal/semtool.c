//
//  semtool.c
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

int sem_getmode(int semid)
{
    union semun su;
    struct semid_ds sem;
    su.buf = &sem;
    int ret = semctl(semid, 0, IPC_STAT, su);
    if (ret == -1) {
        ERR_EXIT("semctl");
    }
    printf("current permission is %o\n", su.buf->sem_perm.mode);

    return ret;
}

int sem_setmode(int semid, char* mode)
{
    union semun su;
    struct semid_ds sem;
    su.buf = &sem;
    int ret = semctl(semid, 0, IPC_STAT, su);
    if (ret == -1) {
        ERR_EXIT("semctl");
    }
    printf("current permission is %o\n", su.buf->sem_perm.mode);
    sscanf(mode, "%o", (unsigned int*)&su.buf->sem_perm.mode);
    ret = semctl(semid, 0, IPC_SET, su);
    if (ret == -1) {
        ERR_EXIT("semctl");
    }
    printf("permission updated...\n");

    return ret;
}

void usage() {
    fprintf(stderr, "usage:\n");
    fprintf(stderr, "semtool -c\n");
    fprintf(stderr, "semtool -d\n");
    fprintf(stderr, "semtool -p\n");
    fprintf(stderr, "semtool -v\n");
    fprintf(stderr, "semtool -s <val>\n");
    fprintf(stderr, "semtool -g\n");
    fprintf(stderr, "semtool -f\n");
    fprintf(stderr, "semtool -m <mode>\n");
}


int main(int argc, char *argv[])
{
    int opt;

    opt = getopt(argc, argv, "cdpvs:gfm:");
    if (opt == '?') {
        exit(EXIT_FAILURE);
    }

    if (opt == -1) {
        usage();
        exit(EXIT_FAILURE);
    }

    key_t key = ftok(".", 's');
    int semid;
    switch (opt) {
        case 'c':
            sem_create(key);
            break;
        case 'p':
            semid = sem_open1(key);
            sem_p(semid);
            sem_getval(semid);
            break;
        case 'v':
            semid = sem_open1(key);
            sem_v(semid);
            sem_getval(semid);
            break;
        case 'd':
            semid = sem_open1(key);
            sem_d(semid);
            break;
        case 's':
            semid = sem_open1(key);
            sem_setval(semid, atoi(optarg));
            break;
        case 'g':
            semid = sem_open1(key);
            sem_getval(semid);
            break;
        case 'f':
            semid = sem_open1(key);
            sem_getmode(semid);
            break;
        case 'm':
            semid = sem_open1(key);
            sem_setmode(semid, argv[2]);
            break;
        default:
            break;
    }


    return 0;
}
