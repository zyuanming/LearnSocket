//
//  mmap_read.c
//  LearnSocket
//
//  Created by Zhang Yuanming on 3/25/18.
//  Copyright © 2018 HansonStudio. All rights reserved.
//

#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
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
    char name[4];
    int age;
} STU;



// > ls -l <file>
// 查看创建的文件信息
// > od -c <file>
// 以二进制的形式查看文件内容
int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int fd;
    fd = open(argv[1], O_CREAT | O_RDWR | O_TRUNC, 0666);
    if (fd == -1) {
        ERR_EXIT("open");
    }

    STU *p;
    p = (STU *)mmap(NULL, sizeof(STU) * 5, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
    if (p == NULL) {
        ERR_EXIT("mmap");
    }

    int i;
    for (i = 0; i < 5; i++) {
        printf("name = %s age = %d\n", (p + i)->name, (p + i)->age);
    }
    munmap(p, sizeof(STU) * 5);
    printf("exit....\n");

    return 0;
}





