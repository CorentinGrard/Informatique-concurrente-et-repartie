#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include "trace_client.h"
#include "mytrace.h"
#include "calcul.h"

void *trace(void *pt)
{
    TraceArgs *args = (TraceArgs *)pt;
    int fdTrace = args->pipeTrace;
    int pid = args->pid;
    while (1)
    {
        Trace data[1];
        read(fdTrace, data, sizeof(data));
        send_rpc_msg(data->message, data->type ,args->pid);
    }
}