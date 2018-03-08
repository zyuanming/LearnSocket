//
//  getip.c
//  LearnSocket
//
//  Created by Zhang Yuanming on 3/8/18.
//  Copyright © 2018 HansonStudio. All rights reserved.
//

#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <errno.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>


// 获取本机IP地址
int getlocalip(char *ip)
{
    char host[100] = {0};
    if (gethostname(host, sizeof(host)) < 0) {
        return -1;
    }
    printf("hostname %s\n", host);
    struct hostent *hp;
    if ((hp = gethostbyname(host)) == NULL)
    {
        return -1;
    }
    strcpy(ip, inet_ntoa(*(struct in_addr*)hp->h_addr));
    return 0;
}


// gcc -Wall -g main.c -o main
int main(int argc, const char * argv[])
{

    char ip[1024] = {0};
    getlocalip(ip);
    printf("%s\n", ip);

}
