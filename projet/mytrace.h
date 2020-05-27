#ifndef MYTRACE_H
#define MYTRACE_H

#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include "myrpc/trace_client.h"

void *trace(void *);

typedef struct TraceArgs
{
    int pipeTrace;
    pid_t pid;
} TraceArgs;

#endif