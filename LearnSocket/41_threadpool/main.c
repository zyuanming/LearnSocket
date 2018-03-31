//
//  main.c
//  LearnSocket
//
//  Created by Zhang Yuanming on 4/1/18.
//  Copyright © 2018 HansonStudio. All rights reserved.
//

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include "threadpool.h"


void* mytask(void *arg)
{
    printf("thread 0x%0x is working on task %d\n", (int)pthread_self(), *(int *)arg);
    sleep(1);
    free(arg);

    return NULL;
}


int main(void)
{
    threadpool_t pool;
    threadpool_init(&pool, 3);

    int i;
    for (i = 0; i < 10; i++) {
        int *arg = (int *)malloc(sizeof(int));
        *arg = 1;
        threadpool_add_task(&pool, mytask, arg);
    }

    sleep(15);
    threadpool_destroy(&pool);

    return 0;
}
