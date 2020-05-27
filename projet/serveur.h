#ifndef SERVEUR_H
#define SERVEUR_H

#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include "calcul.h"

void *serveur(void*);
void *serverWorker(void*);

typedef struct ServeurArgs
{
    int pipeReception;
    int port;
} ServeurArgs;

typedef struct ServerWorkerArgs
{
    int pipeReception;
    int socketClient;
} ServerWorkerArgs;

#endif