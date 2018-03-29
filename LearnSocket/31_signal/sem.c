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

    return semid;
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


int semid;

void print(char op_char)
{
    int pause_time;
    srand(getpid());
    int i;
    for (i = 0; i < 10; i++) {
        sem_p(semid);
        printf("%c", op_char);
        fflush(stdout);
        pause_time = rand() % 3;
        sleep(pause_time);
        printf("%c", op_char);
        // 因为上面的输出没有换行符,所以不一定会立即输出到标准输出,
        // 所以使用下面的方法来刷新缓存
        fflush(stdout);
        sem_v(semid);
        pause_time = rand() % 2;
        sleep(pause_time);
    }
}



int main(int argc, char* argv[])
{
    semid = sem_create(IPC_PRIVATE);
    sem_setval(semid, 0);
    pid_t pid;
    pid = fork();
    if (pid == -1) {
        ERR_EXIT("fork");
    }
    if (pid > 0) {
        sem_setval(semid, 1);
        print('o');
        wait(NULL);
        sem_d(semid);
    } else {
        print('x');
    }
}


