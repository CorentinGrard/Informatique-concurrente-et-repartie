#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include "myrpc/trace_client.h"
#include "mytrace.h"


void *trace(void *pt)
{
    TraceArgs *args = (TraceArgs *)pt;
    int fdTrace = args->pipeTrace;
    char bufferTaille[8];
    read(fdTrace, bufferTaille, sizeof(bufferTaille));
    size_t taille = (size_t)*bufferTaille;
    char bufferMsg[taille];
    read(fdTrace, bufferMsg, sizeof(bufferMsg));
    send_rpc_msg(bufferMsg, args->pid);
}