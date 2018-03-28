//
//  shm.c
//  LearnSocket
//
//  Created by Zhang Yuanming on 3/27/18.
//  Copyright © 2018 HansonStudio. All rights reserved.
//

#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/sem.h>
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



void unlink1(void)
{
    shm_unlink("/xyz");
}

void test_create(void)
{
    unlink1();
    int shmid;
    shmid = shm_open("/xyz", O_CREAT | O_RDWR, 0666);
    if (shmid == -1) {
        ERR_EXIT("shm_open");
    }

    printf("shm open success\n");
    if (ftruncate(shmid, sizeof(STU)) == -1) {
        ERR_EXIT("ftruncate");
    }

    struct stat buf;
    if (fstat(shmid, &buf) == -1) {
        ERR_EXIT("fstat");
    }
    printf("size=%lld mode=%o\n", buf.st_size, buf.st_mode & 07777);

    close(shmid);
}


void test_write(void)
{
    int shmid;
    shmid = shm_open("/xyz", O_WRONLY, 0);
    if (shmid == -1) {
        ERR_EXIT("shm_open");
    }

    struct stat buf;
    if (fstat(shmid, &buf) == -1) {
        ERR_EXIT("fstat");
    }
    printf("size=%lld mode=%o\n", buf.st_size, buf.st_mode & 07777);

    STU *p;
    p = mmap(NULL, buf.st_size, PROT_WRITE, MAP_SHARED, shmid, 0);
    if (p == MAP_FAILED) {
        ERR_EXIT("mmap");
    }
    strcpy(p->name, "test");
    p->age = 20;

    close(shmid);
}

void test_read(void)
{
    int shmid;
    shmid = shm_open("/xyz", O_WRONLY, 0);
    if (shmid == -1) {
        ERR_EXIT("shm_open");
    }

    struct stat buf;
    if (fstat(shmid, &buf) == -1) {
        ERR_EXIT("fstat");
    }
    printf("size=%lld mode=%o\n", buf.st_size, buf.st_mode & 07777);

    STU *p;
    p = mmap(NULL, buf.st_size, PROT_WRITE, MAP_SHARED, shmid, 0);
    if (p == MAP_FAILED) {
        ERR_EXIT("mmap");
    }
    printf("name=%s age=%d\n", p->name, p->age);

    close(shmid);
}

int main(void)
{
    test_read();

    return 0;
}







