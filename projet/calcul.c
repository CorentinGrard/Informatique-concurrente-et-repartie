#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include "calcul.h"

void *calcul(void *pt)
{
    //get fd
    CalculArgs *args = (CalculArgs *)pt;
    int fdreception = args->pipeReception;
    int fdenvoi = args->pipeEnvoi;
    int fdrpc = args->pipeTrace;
    int *serveursPorts = args->serveursPorts;
    int isFirst = args->isFirst;

    int horloge = 0;
    bool wantSC = false;
    int compteurReply = 0;
    int nbServeurs = sizeof(serveursPorts) / sizeof(serveursPorts[0]);

    Item queue[1000];
    int queueIndex = 0;

    srand(time(NULL));

    while (1)
    {
        // Read Socket
        DataSocket data[1];
        read(fdreception, data, sizeof(data));

        // If Type == REPLY
        if (wantSC && data->type == 2)
        {
            compteurReply++;
            if (compteurReply == nbServeurs)
            {
                // Section critique
                printf("Section Critique !\n");
                // Envoi RELEASE
                for (int i = 0; i < nbServeurs; i++)
                {
                    PipeClient toSend[1];
                    toSend->port = serveursPorts[i];
                    toSend->type = 3;

                    write(fdenvoi, toSend, sizeof(toSend));
                    // write(fdrpc, toSend, sizeof(toSend));
                }
                // Reset
                compteurReply = 0;
                wantSC = false;
            }
        }
        // IF TYPE == RELEASE
        else if (data->type == 3)
        {
            // Remove fist request
            for (int i = 0; i < queueIndex - 1; i++)
            {
                queue[i] = queue[i + 1];
            }
            queueIndex--;
        }
        // IF TYPE == REQUEST
        else if (data->type == 1)
        {
            // Store new request
            Item request;
            request.horloge = data->horloge;
            request.pid = data->pid;
            for (int i = 0; i < queueIndex; i++)
            {
                // Sort by horloge
                if (request.horloge <= queue[i].horloge)
                {
                    queueIndex++;
                    for (int j = queueIndex; j > i + 1; j--)
                    {
                        queue[j] = queue[j - 1];
                    }
                    queue[i] = request;
                    break;
                }
            }
        }
        // IF TYPE == MESSAGE
        else if (data->type == 0)
        {
            // Display message
            char* message = data->message;
            printf("Message : %s\n", message);

            // RPC
            PipeClient toSend[1];
            toSend->message = data->message;

            write(fdrpc, toSend, sizeof(toSend));
        }

        // Execute action
        int pickAction = rand() % 3;
        switch (pickAction)
        {
        case 0:
            // Action Locale
            horloge++;
            printf("Action Locale\n");
            // TODO RPC
            break;
        case 1:
            // Envoi message
            horloge++;

            // Pick serveur
            int portIndex = rand() % nbServeurs;
            int port = serveursPorts[portIndex];

            // Get Message from user
            char message[50];
            printf("Entrez votre message :\n");
            fgets(message, 50, stdin);
            strtok(message, "\n");
            size_t taille = strlen(message) + 1;

            // Prepare message to send
            PipeClient toSend[1];
            toSend->port = port;
            toSend->type = 0;

            toSend->message = message;

            // write to pipeClient
            write(fdenvoi, toSend, sizeof(toSend));

            // TODO RPC
            write(fdrpc, toSend, sizeof(toSend));
            break;
        case 2:
            // Veux SC
            horloge++;
            wantSC = true;
            for (int i = 0; i < nbServeurs; i++)
            {
                int port = serveursPorts[i];
                PipeClient toSend[1];
                toSend->port = port;
                toSend->type = 1;
                toSend->horloge = horloge;
                toSend->pid = getpid();

                write(fdenvoi, toSend, sizeof(toSend));
                // TODO RPC
                write(fdrpc, toSend, sizeof(toSend));
            }
            break;
        default:
            printf("Calcul : erreur pick option");
            break;
        }
    }
    close(fdreception);
    close(fdenvoi);
}