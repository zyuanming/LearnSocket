//
//  ptest.c
//  LearnSocket
//
//  Created by Zhang Yuanming on 3/29/18.
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
#include <sys/semaphore.h>


#define ERR_EXIT(m) \
do \
{ \
perror(m); \
exit(EXIT_FAILURE); \
} while(0)


#define CONSUMERS_COUNT 1
#define PRODUCERS_COUNT 1

pthread_mutex_t g_mutex;
pthread_cond_t g_cond;

pthread_t g_thread[CONSUMERS_COUNT + PRODUCERS_COUNT];

int nready = 0;

void* consume(void *arg)
{
    int num = (int)arg;
    while (1) {
        pthread_mutex_lock(&g_mutex);
        while (nready == 0) {
            printf("%d begin wait a condition...\n", num);
            pthread_cond_wait(&g_cond, &g_mutex);
        }
        printf("%d end wait a condition...\n", num);
        --nready;
        pthread_mutex_unlock(&g_mutex);
        sleep(1);
    }
    return NULL;
}

void* produce(void *arg)
{
    int num = (int)arg;
    while (1) {
        pthread_mutex_lock(&g_mutex);
        ++nready;
        pthread_cond_signal(&g_cond);
        printf("%d signal....\n", num);
        pthread_mutex_unlock(&g_mutex);
        sleep(2);
    }
    return NULL;
}

int main(void) {
    int i;
    pthread_mutex_init(&g_mutex, NULL);
    pthread_cond_init(&g_cond, NULL);

    // 创建消费者线程
    for (i = 0; i < CONSUMERS_COUNT; i++) {
        pthread_create(&g_thread[i], NULL, consume, (void *)i);
    }

    sleep(1);

    for (i = 0; i < PRODUCERS_COUNT; i++) {
        pthread_create(&g_thread[CONSUMERS_COUNT + i], NULL, produce, (void *)i);
    }

    for (i = 0; i < CONSUMERS_COUNT + PRODUCERS_COUNT; i++) {
        pthread_join(g_thread[i], NULL);
    }

    pthread_mutex_destroy(&g_mutex);
    pthread_cond_destroy(&g_cond);

    return 0;
}





