//
//  pthread1.c
//  LearnSocket
//
//  Created by Zhang Yuanming on 3/28/18.
//  Copyright © 2018 HansonStudio. All rights reserved.
//

#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>

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


void* thread_routine(void *arg)
{
    int i;
    for (i = 0; i < 20; i++) {
        printf("B");
        fflush(stdout);
        usleep(20);

        if (i == 5) {
            pthread_exit("ABC");
        }
    }
    return 0;
}


int main(void)
{
    pthread_t tid;
    int ret;
    if ((ret = pthread_create(&tid, NULL, thread_routine, NULL)) != 0) {
        fprintf(stderr, "pthread create:%s\n", strerror(ret));
        exit(EXIT_FAILURE);
    }

    int i;
    for (i = 0; i < 20; i++) {
        printf("A");
        fflush(stdout);
        usleep(20);
    }

    void *value;
    if ((ret = pthread_join(tid, &value)) != 0) {
        fprintf(stderr, "pthread join:%s\n", strerror(ret));
        exit(EXIT_FAILURE);
    }

    printf("\n");
    printf("return msg=%s\n", (char *)value);

    return 0;
}


