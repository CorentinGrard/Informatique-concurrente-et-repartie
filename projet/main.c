/**********************************/
/*      Membres :                 */
/*          * Léa Crepin          */
/*          * Bastien Bellino     */
/*          * Matthieu Tinnes     */
/*          * Corentin Grard      */
/**********************************/

#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

#include "trace_client.h"
#include "calcul.h"
#include "client.h"
#include "serveur.h"
#include "mytrace.h"

void ctrlc_handler(int signum, siginfo_t *info, void *pasUtileIci)
{
    printf("Arret du serveur !\n");
    //TODO close les sockets + pipes
    exit(0);
}

int mainProcessus(int portsServeurs[],int portSocket, int isFirst)
{
    // Thread
    pthread_t threadServeur;
    pthread_t threadClient;
    pthread_t threadCalcul;
    pthread_t threadTrace;

    // Descripteurs pipes
    int *descripteurReception = malloc(sizeof(int) * 2);
    int *descripteurEnvoi = malloc(sizeof(int) * 2);
    int *descripteurTrace = malloc(sizeof(int) * 2);

    // Pipes
    pipe(descripteurReception);
    pipe(descripteurEnvoi);
    pipe(descripteurTrace);

    if (isFirst)
    {
        DataSocket dataInit[1];
        dataInit->type = 0;
        strcpy(dataInit->message,"Message d'initialisation");
        write(descripteurReception[1], dataInit, sizeof(dataInit));
        sleep(1);
    }

    // Creation des strucs pour threads
    ServeurArgs *serveurArgs = (ServeurArgs *)malloc(sizeof(ServeurArgs));
    serveurArgs->port = portSocket;
    serveurArgs->pipeReception = descripteurReception[1];

    ClientArgs *clientArgs = (ClientArgs *)malloc(sizeof(ClientArgs));
    clientArgs->host = "127.0.0.1";
    clientArgs->pipeEnvoi = descripteurEnvoi[0];

    CalculArgs *calculArgs = (CalculArgs *)malloc(sizeof(CalculArgs));
    calculArgs->pipeEnvoi = descripteurEnvoi[1];
    calculArgs->pipeReception = descripteurReception[0];
    calculArgs->pipeTrace = descripteurTrace[1];
    calculArgs->port = portSocket;
    memcpy(calculArgs->serveursPorts,portsServeurs,sizeof(int)* NOMBRE_DE_SERVERS_MAX);
    calculArgs->isFirst = isFirst;
    calculArgs->pid = getpid();

    TraceArgs *traceArgs = (TraceArgs *)malloc(sizeof(TraceArgs));
    traceArgs->pipeTrace = descripteurTrace[0];
    traceArgs->pid = getpid();

    // Creation des threads
    if (pthread_create(&threadServeur, NULL, (void *(*)())serveur, serveurArgs) == -1)
    {
        perror("Unable to create the server thread");
    }
    if (pthread_create(&threadClient, NULL, (void *(*)())client, clientArgs) == -1)
    {
        perror("Unable to create the client thread");
    }
    if (pthread_create(&threadCalcul, NULL, (void *(*)())calcul, calculArgs) == -1)
    {
        perror("Unable to create the calcul thread");
    }
    if (pthread_create(&threadTrace, NULL, (void *(*)())trace, traceArgs) == -1)
    {
        perror("Unable to create the trace thread");
    }

    // Wait for the end of the threads
    pthread_join(threadServeur, NULL);
    pthread_join(threadClient, NULL);
    pthread_join(threadCalcul, NULL);

    return 0;
}

int main(int argc, char **argv)
{
    // Signals pour CTRL + C
    struct sigaction prepaSignal;
    prepaSignal.sa_sigaction = ctrlc_handler;
    prepaSignal.sa_flags = SA_SIGINFO;
    if (sigaction(SIGINT, &prepaSignal, NULL))
    {
        perror("Error sigaction ");
    }

    // Read serveurs.conf
    FILE *fdConf = fopen("serveurs.conf", "r");
    if (fdConf == NULL)
    {
        perror("Error opening conf file");
        return -1;
    }
    char port[2048];
    char delim[] = ",";
    char *nbRead = fgets(port, 2048, fdConf);
    int nbPort = strlen(port) / 5;
    char *ptr = strtok(port, delim);

    int *client_list = malloc(sizeof(int) * NOMBRE_DE_SERVERS_MAX);
    int i = 0;
    while (ptr != NULL)
    {
        client_list[i] = atoi(ptr);
        ptr = strtok(NULL, delim);
        ++i;
    }
    int isFirst = 1;
    for (int j = 0; j < i; ++j)
    {
        int test = client_list[j];
        pid_t pid = fork();
        if (pid < 0)
        {
            perror("Error fork");
            exit(-1);
        }
        else if (pid == 0)
        {
            mainProcessus(client_list, client_list[j], isFirst);
            break;
        }
        isFirst = 0;
    }
    sleep(300);
    return 0;
}
