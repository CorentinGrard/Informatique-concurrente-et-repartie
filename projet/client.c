#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include "client.h"
#include "calcul.h"

void *client(void *pt)
{

    ClientArgs *args = (ClientArgs *)pt;

    struct sockaddr_in adr;
    adr.sin_family = PF_INET;
    adr.sin_addr.s_addr = inet_addr(args->host);

    while (1)
    {
        /* Reçois la taille du port et le port */
        /*********************************/
        PipeClient dataClient[1];
        read(args->pipeEnvoi, dataClient, sizeof(dataClient));

        adr.sin_port = htons(dataClient->port);

        /* Creation de la socket */
        /*************************/
        int descripteurDeSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (descripteurDeSocket < 0)
        {
            printf("Problemes pour creer la socket");
            exit(1);
        }

        /* Connect de la socket */
        /*************************/
        if (connect(descripteurDeSocket, (struct sockaddr *)&adr, sizeof(adr)) < 0)
        {
            printf("Client : Problemes pour se connecter au serveur\n");
            exit(1);
        }
        DataSocket toSend[1];
        toSend->type = dataClient->type;
        switch (dataClient->type)
        {
        case 0: //Message
        {
            strcpy(toSend->message, dataClient->message);
            break;
        }

        case 1: //Request
            toSend->pid = dataClient->pid;
            toSend->horloge = dataClient->horloge;
            toSend->RequestPort = dataClient->RequestPort;
            break;
        }
        send(descripteurDeSocket, toSend, sizeof(toSend), 0);
        close(descripteurDeSocket);
    }
}
