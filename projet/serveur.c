#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include "serveur.h"

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
        int socket = accept(socketServeur, (struct sockaddr *)&adresseDuClient, &longueurDeAdresseDuClient);

        // Reception taille
        DataSocket data[1];
        if (recv(socket, data, sizeof(data), 0) < 0)
        {
            perror("Serveur : Error read Socket");
        }
        if (write(pipeReception, data, sizeof(data)) < 0)
        {
            perror("Serveur : Error write pipe");
        }

        close(socket);
    }
    close(socketServeur);
}