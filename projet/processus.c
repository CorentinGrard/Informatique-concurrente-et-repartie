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
} CalculArgs;

void *serveur(void *pt)
{
    ServeurArgs *arguments = (ServeurArgs *)pt;
    int pipeReception = arguments->pipeReception;
    int port = arguments->port;

    struct sockaddr_in adresseDuServeur;
    adresseDuServeur.sin_family = AF_INET;
    adresseDuServeur.sin_port = htons(port);
    adresseDuServeur.sin_addr.s_addr = INADDR_ANY;

    int socketServeur = socket(PF_INET, SOCK_STREAM, 0);
    if (socketServeur < 0)
    {
        perror("Serveur socket :");
        exit(1);
    }
    printf("Serveur socket crée\n");
    if (bind(socketServeur, (struct sockaddr *)&adresseDuServeur, sizeof(struct sockaddr_in)) < 0)
    {
        perror("Serveur bind");
        exit(1);
    }
    printf("Serveur bindé\n");

    if (listen(socketServeur, 1) < 0)
    {
        perror("Serveur listen");
        exit(1);
    }
    while (1)
    {
        struct sockaddr_in adresseDuClient;
        unsigned int longueurDeAdresseDuClient;
        printf("Socket en attente\n");
        int socket = accept(socketServeur, (struct sockaddr *)&adresseDuClient, &longueurDeAdresseDuClient);

        // Reception taille
        char bufferSize[4];
        recv(socket, bufferSize, sizeof(bufferSize), 0);
        int nbOctets = atoi(bufferSize);
        write(pipeReception, bufferSize, sizeof(bufferSize));

        // Reception message
        char buffer[nbOctets];
        recv(socket, buffer, sizeof(buffer), 0);
        write(pipeReception, buffer, sizeof(buffer));

        close(socket);
    }
    close(socketServeur);
}

void *calcul(void *pt)
{
    //get fd
    CalculArgs *myArgs = (CalculArgs *)pt;
    int fdreception = myArgs->pipeReception;
    int fdenvoi = myArgs->pipeEnvoi;

    while (1)
    {
        // get buffersize
        char bufferSize[4];
        read(fdreception, bufferSize, sizeof(bufferSize));

        // get msg
        unsigned int tailleMessage = atoi(bufferSize);
        char msg[tailleMessage];
        read(fdreception, &msg, sizeof(char) * tailleMessage);

        // define response
        int tailleFull = 17 + strlen(msg);
        char toConcat[] = "Message reçu : ";
        char response[tailleFull];
        strcat(response, toConcat);
        strcat(response, msg);

        // add buffersize
        char sizeResponse[4];
        snprintf(sizeResponse, sizeof(sizeResponse), "%zu", strlen(response));
        write(fdenvoi, &sizeResponse, sizeof(sizeResponse));

        // add msg
        write(fdenvoi, &response, sizeof(response));
    }
    close(fdreception);
    close(fdenvoi);
}

void *client(void *pt)
{

    ClientArgs *args = (ClientArgs *)pt;
    /*****************/
    /* Socket client */
    /*****************/
    /* Déclaration de l'adresse IP, du numéro de port et du protocole */
    /******************************************************************/
    struct sockaddr_in adr;
    adr.sin_family = PF_INET;
    adr.sin_port = htons(args->port);
    adr.sin_addr.s_addr = inet_addr(args->host);
    /* Affichage des informations de connexion */
    /*******************************************/
    printf("Connexion vers la machine ");
    unsigned char *adrIP = (unsigned char *)&(adr.sin_addr.s_addr);
    printf("%d.", *(adrIP));
    printf("%d.", *(adrIP + 1));
    printf("%d.", *(adrIP + 2));
    printf("%d", *(adrIP + 3));
    printf(" sur le port %u \n", args->port);
    /* Creation de la socket */
    /*************************/
    int descripteurDeSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (descripteurDeSocket < 0)
    {
        printf("Problemes pour creer la socket");
        exit(1);
    }
    printf("socket cree\n");
    /* Connexion de la socket au serveur */
    /*************************************/
    if (connect(descripteurDeSocket,
                (struct sockaddr *)&adr,
                sizeof(adr)) < 0)
    {
        printf("Problemes pour se connecter au serveur");
        exit(1);
    }
    printf("socket connectee\n");
    /* Envoi la taille grâce à send */
    /*********************************/
    char bufferTaille;
    read(args->pipeEnvoi, &bufferTaille, sizeof(char));

    send(descripteurDeSocket, &bufferTaille, sizeof(char), 0);

    /* Envoi la reponse grâce à send */
    /*********************************/
    int taille = atoi(&bufferTaille);
    char bufferMsg[taille];
    read(args->pipeEnvoi, &bufferTaille, sizeof(char) * taille);

    send(descripteurDeSocket, &bufferMsg, sizeof(char) * taille, 0);

    close(descripteurDeSocket);
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

    // Descripteurs pipes
    int *descripteurReception = malloc(sizeof(int) * 2);
    int *descripteurEnvoi = malloc(sizeof(int) * 2);

    // Pipes
    pipe(descripteurReception);
    pipe(descripteurEnvoi);

    if (isFirst)
    {
        char message[] = "pipe en bois🤠";
        write(descripteurEnvoi[1], message, sizeof(message));
    }

    // Creation des strucs pour threads
    ServeurArgs *serveurArgs = (ServeurArgs *)malloc(sizeof(ServeurArgs));
    serveurArgs->port = portServeur;
    serveurArgs->pipeReception = descripteurReception[1];

    ClientArgs *clientArgs = (ClientArgs *)malloc(sizeof(ClientArgs));
    clientArgs->host = "localhost";
    clientArgs->pipeEnvoi = descripteurEnvoi[0];
    clientArgs->port = portClient;

    CalculArgs *calculArgs = (CalculArgs *)malloc(sizeof(CalculArgs));
    calculArgs->pipeEnvoi = descripteurEnvoi[1];
    calculArgs->pipeReception = descripteurReception[0];

    // Creation des threads
    if (pthread_create(&threadServeur, NULL, (void *(*)())serveur, serveurArgs) == -1)
    {
        perror("Unable to create the server thread");
    }
    if (pthread_create(&threadClient, NULL, (void *(*)())client, NULL) == -1)
    {
        perror("Unable to create the client thread");
    }
    if (pthread_create(&threadCalcul, NULL, (void *(*)())calcul, NULL) == -1)
    {
        perror("Unable to create the calcul thread");
    }

    // Wait for the end of the threads
    pthread_join(threadServeur, NULL);
    pthread_join(threadClient, NULL);
    pthread_join(threadCalcul, NULL);

    return 0;
}
