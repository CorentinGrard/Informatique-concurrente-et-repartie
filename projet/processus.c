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

typedef struct
{
    int pipeReception;
    int port;
} ServeurArgs;

typedef struct
{
    int port;
    char *host;
    int pipeEnvoi;
} ClientArgs;

typedef struct
{
    int pipeReception;
    int pipeEnvoi;
    int pipeTrace;
    int isFirst;
} CalculArgs;

typedef struct
{
    int pipeTrace;
    pid_t pid;
} TraceArgs;

void *serveur(void *pt)
{
    ServeurArgs *arguments = (ServeurArgs *)pt;
    int pipeReception = arguments->pipeReception;
    int port = arguments->port;

    struct sockaddr_in adresseDuServeur;
    adresseDuServeur.sin_family = AF_INET;
    adresseDuServeur.sin_port = htons(port);
    adresseDuServeur.sin_addr.s_addr = INADDR_ANY;

    int socketServeur = socket(AF_INET, SOCK_STREAM, 0);
    if (socketServeur < 0)
    {
        perror("Serveur : socket");
        exit(1);
    }

    if (bind(socketServeur, (struct sockaddr *)&adresseDuServeur, sizeof(struct sockaddr_in)) < 0)
    {
        perror("Serveur bind");
        exit(1);
    }

    if (listen(socketServeur, 1) < 0)
    {
        perror("Serveur listen");
        exit(1);
    }
    while (1)
    {
        struct sockaddr_in adresseDuClient;
        unsigned int longueurDeAdresseDuClient;
        printf("Serveur : Socket en attente\n");
        int socket = accept(socketServeur, (struct sockaddr *)&adresseDuClient, &longueurDeAdresseDuClient);

        // Reception taille
        size_t bufferSize[1];
        recv(socket, bufferSize, sizeof(bufferSize), 0);
        int nbOctets = (size_t)*bufferSize;
        write(pipeReception, bufferSize, sizeof(bufferSize));

        // Reception message
        char buffer[nbOctets];
        recv(socket, buffer, sizeof(buffer), 0);
        write(pipeReception, buffer, sizeof(buffer));

        close(socket);
    }
    close(socketServeur);
}

void send_msg(int fdreception, int fdenvoi, int fdrpc)
{
    // get buffersize
    size_t bufferSize[1];
    read(fdreception, bufferSize, sizeof(bufferSize));

    // get msg
    size_t tailleMessage = (size_t)*bufferSize;
    char msg[tailleMessage];
    read(fdreception, msg, sizeof(msg));

    printf("Message reçu : %s\n", msg);

    // define response
    char message[50];
    printf("Entrez votre message :\n");
    fgets(message, 50, stdin);
    strtok(message, "\n");

    size_t tailleFull = 3 + strlen(msg) + strlen(message) + 1;
    char toConcat[] = " , ";
    char response[tailleFull];
    bzero(response, tailleFull);
    strcat(response, message);
    strcat(response, toConcat);
    strcat(response, msg);

    // add buffersize envoie
    write(fdenvoi, &tailleFull, sizeof(tailleFull));

    // add msg envoie
    write(fdenvoi, &response, sizeof(response));

    // add buffersize rpc
    write(fdrpc, &tailleFull, sizeof(tailleFull));

    //add msg rpc
    write(fdrpc, &response, sizeof(response));
}

void *calcul(void *pt)
{
    //get fd
    CalculArgs *myArgs = (CalculArgs *)pt;
    int fdreception = myArgs->pipeReception;
    int fdenvoi = myArgs->pipeEnvoi;
    int fdrpc = myArgs->pipeTrace;
    int isFirst = myArgs->isFirst;

    if (isFirst)
    {
        char message[50];
        printf("Entrez votre message :\n");
        fgets(message, 50, stdin);
        strtok(message, "\n");

        size_t tailleFull = strlen(message) + 1;
        char response[tailleFull];
        bzero(response, tailleFull);
        strcat(response, message);

        // add buffersize envoie
        write(fdenvoi, &tailleFull, sizeof(tailleFull));

        // add msg envoie
        write(fdenvoi, &response, sizeof(response));

        // add buffersize rpc
        write(fdrpc, &tailleFull, sizeof(tailleFull));

        //add msg rpc
        write(fdrpc, &response, sizeof(response));
        
        isFirst = 0;
    }

    while (1)
        send_msg(fdreception, fdenvoi, fdrpc);

    close(fdreception);
    close(fdenvoi);
}

void *client(void *pt)
{

    ClientArgs *args = (ClientArgs *)pt;

    struct sockaddr_in adr;
    adr.sin_family = PF_INET;
    adr.sin_port = htons(args->port);
    adr.sin_addr.s_addr = inet_addr(args->host);

    while (1)
    {
        /* Creation de la socket */
        /*************************/
        int descripteurDeSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (descripteurDeSocket < 0)
        {
            printf("Problemes pour creer la socket");
            exit(1);
        }

        /* Envoi la taille grâce à send */
        /*********************************/
        size_t bufferTaille[1];
        read(args->pipeEnvoi, bufferTaille, sizeof(bufferTaille));

        if (connect(descripteurDeSocket, (struct sockaddr *)&adr, sizeof(adr)) < 0)
        {
            printf("Client : Problemes pour se connecter au serveur\n");
            exit(1);
        }

        send(descripteurDeSocket, bufferTaille, sizeof(bufferTaille), 0);

        /* Envoi le msg grâce à send */
        /*********************************/
        size_t taille = (size_t)*bufferTaille;
        char bufferMsg[taille];
        read(args->pipeEnvoi, bufferMsg, sizeof(bufferMsg));

        send(descripteurDeSocket, &bufferMsg, sizeof(bufferMsg), 0);

        printf("Client : Envoi du message \"%s\" de taille %ld\n", bufferMsg, taille);
        close(descripteurDeSocket);
    }
}

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

int main(int argc, char **argv)
{
    // Arguments
    if (argc < 3)
    {
        printf("Usage : %s portClient portServeur isFirst\n", argv[0]);
        exit(1);
    }
    int portClient = atoi(argv[1]);
    int portServeur = atoi(argv[2]);
    int isFirst = atoi(argv[3]);

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

    // Creation des strucs pour threads
    ServeurArgs *serveurArgs = (ServeurArgs *)malloc(sizeof(ServeurArgs));
    serveurArgs->port = portServeur;
    serveurArgs->pipeReception = descripteurReception[1];

    ClientArgs *clientArgs = (ClientArgs *)malloc(sizeof(ClientArgs));
    clientArgs->host = "127.0.0.1";
    clientArgs->pipeEnvoi = descripteurEnvoi[0];
    clientArgs->port = portClient;

    CalculArgs *calculArgs = (CalculArgs *)malloc(sizeof(CalculArgs));
    calculArgs->pipeEnvoi = descripteurEnvoi[1];
    calculArgs->pipeReception = descripteurReception[0];
    calculArgs->pipeTrace = descripteurTrace[1];
    calculArgs->isFirst = isFirst;

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
