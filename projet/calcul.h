#ifndef CALCUL_H
#define CALCUL_H

#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

#define NOMBRE_DE_SERVERS_MAX 10

void send_msg(int, int, int);
void *calcul(void *);

typedef struct CalculArgs
{
    int pipeReception;
    int pipeEnvoi;
    int pipeTrace;
    int serveursPorts[NOMBRE_DE_SERVERS_MAX];
    int isFirst;
} CalculArgs;

typedef struct PipeClient
{
    int port;
    int type;
    char *message;
    pid_t pid;
    int horloge;
} PipeClient;

typedef struct DataSocket
{
    int type;
    char *message;
    pid_t pid;
    int horloge;
} DataSocket;

typedef struct Item
{
    pid_t pid;
    int horloge;
} Item;

#endif