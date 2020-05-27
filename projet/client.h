#ifndef CLIENT_H
#define CLIENT_H

#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

void *client(void*);

typedef struct ClientArgs
{
    char *host;
    int pipeEnvoi;
} ClientArgs;

#endif