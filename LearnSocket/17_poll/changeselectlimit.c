//
//  changeselectlimit.c
//  LearnSocket
//
//  Created by Zhang Yuanming on 3/17/18.
//  Copyright © 2018 HansonStudio. All rights reserved.
//

#include <stdio.h>
#include <sys/resource.h>
#include <stdlib.h>

#define ERR_EXIT(m) \
do \
{ \
perror(m); \
exit(EXIT_FAILURE); \
} while(0)

int main()
{

    struct rlimit rl;
    if (getrlimit(RLIMIT_NOFILE, &rl) < 0) {
        ERR_EXIT("getrlimit");
    }
    printf("The maximum number of open files for this process is\n %llu.\n", rl.rlim_max);

    rl.rlim_cur = 2048;
    rl.rlim_max = 2048;

    if (setrlimit(RLIMIT_NOFILE, &rl) < 0) {
        ERR_EXIT("setrlimit");
    }

    if (getrlimit(RLIMIT_NOFILE, &rl) < 0) {
        ERR_EXIT("getrlimit");
    }

    printf("The maximum number of open files for this process is\n %llu.\n", rl.rlim_max);

    return 0;
}
