//
//  shmfifo_free.c
//  LearnSocket
//
//  Created by Zhang Yuanming on 3/29/18.
//  Copyright © 2018 HansonStudio. All rights reserved.
//

#include "shmfifo.h"


typedef struct stu
{
    char name[32];
    int age;
} STU;


int main(void)
{
    shmfifo_t *fifo = shmfifo_init(1234, sizeof(STU), 3);
    shmfifo_destroy(fifo);
    return 0;
}
