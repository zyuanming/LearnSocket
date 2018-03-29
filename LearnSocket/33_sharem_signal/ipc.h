//
//  ipc.h
//  LearnSocket
//
//  Created by Zhang Yuanming on 3/29/18.
//  Copyright © 2018 HansonStudio. All rights reserved.
//

#ifndef ipc_h
#define ipc_h

#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/sem.h>
#include <sys/shm.h>

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

int sem_create(key_t key);
int sem_open1(key_t key);
int sem_p(int semid);
int sem_v(int semid);
int sem_d(int semid);
int sem_setval(int semid, int val);
int sem_getval(int semid);

#endif /* ipc_h */
