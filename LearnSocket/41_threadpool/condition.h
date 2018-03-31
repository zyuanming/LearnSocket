//
//  condition.h
//  LearnSocket
//
//  Created by Zhang Yuanming on 4/1/18.
//  Copyright © 2018 HansonStudio. All rights reserved.
//

#ifndef condition_h
#define condition_h

#include <pthread.h>

typedef struct condition
{
    pthread_mutex_t pmutex;
    pthread_cond_t pcond;
} condition_t;

int condition_init(condition_t *cond);
int condition_lock(condition_t *cond);
int condition_unlock(condition_t *cond);
int condition_wait(condition_t *cond);
int condition_timedwait(condition_t *cond, const struct timespec *abstime);
int condition_signal(condition_t *cond);
int condition_broadcast(condition_t *cond);
int condition_destroy(condition_t *cond);

#endif /* condition_h */
