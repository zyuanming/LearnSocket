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



#define DELAY (rand() % 5 + 1)
int semid;

void wait_for_2fork(int no)
{
    int left = no;
    int right = (no + 1) % 5;

    struct sembuf buf[2] = {
        {left, -1, 0},
        {right, -1, 0}
    };
    semop(semid, buf, 2);
}

void free_2fork(int no)
{
    int left = no;
    int right = (no + 1) % 5;

    struct sembuf buf[2] = {
        {left, 1, 0},
        {right, 1, 0}
    };
    semop(semid, buf, 2);
}


void philosophere(int no)
{
    srand(getpid());
    for (;;) {
        printf("%d is thinking\n", no);
        sleep(DELAY);
        printf("%d is hungry\n", no);
        wait_for_2fork(no);
        printf("%d is eating\n", no);
        sleep(DELAY);
        free_2fork(no);
    }
}



// 用信号量集解决哲学家就餐问题
int main(int argc, char* argv[])
{
    semid = semget(IPC_PRIVATE, 5, IPC_CREAT | 0666);
    if (semid == -1) {
        ERR_EXIT("semget");
    }

    union semun su;
    su.val = 1;
    int i;
    for (i = 0; i < 5; i++) {
        semctl(semid, i, SETVAL, su);
    }
    int no = 0;
    pid_t pid;
    for (i = 0; i < 5; i++) {
        pid = fork();
        if (pid == -1) {
            ERR_EXIT("fork");
        }
        if (pid == 0) {
            no = i;
            break;
        }
    }
    philosophere(no);

    return 0;

}








