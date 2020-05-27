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
    int serveursPorts[NOMBRE_DE_SERVERS_MAX];
    memcpy(serveursPorts, args->serveursPorts, sizeof(int) * NOMBRE_DE_SERVERS_MAX);
    int isFirst = args->isFirst;
    pid_t pid = args->pid;

    int horloge = 0;
    bool wantSC = false;
    int compteurReply = 0;
    int nbServeurs;
    for (int i = 0; i < NOMBRE_DE_SERVERS_MAX; i++)
    {
        if (serveursPorts[i] == 0)
        {
            nbServeurs = i;
            break;
        }
    }

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
            mylog(fdrpc, 2, "REPLY");
            if (compteurReply == nbServeurs)
            {
                // Section critique
                printf("Section Critique !\n");
                mylog(fdrpc, 4, "SECTION CRITIQUE");

                // Envoi RELEASE
                for (int i = 0; i < nbServeurs; i++)
                {
                    PipeClient toSend[1];
                    toSend->port = serveursPorts[i];
                    toSend->type = 3;

                    write(fdenvoi, toSend, sizeof(toSend));
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
            char *message = data->message;
            printf("Message : %s\n", message);
            mylog(fdrpc, data->type, data->message);
        }

        // Execute action
        int pickAction = rand() % 3;
        switch (pickAction)
        {
        case 0:
            // Action Locale
            horloge++;
            printf("Action Locale\n");
            mylog(fdrpc, 5, "ACTION LOCALE");
            break;
        case 1:
            // Envoi message
            horloge++;

            // Pick serveur
            int portIndex = rand() % nbServeurs;
            int port = serveursPorts[portIndex];

            // Get Message from user
            char message[50];
            printf("%d Entrez votre message :\n", pid);
            fgets(message, 50, stdin);
            strtok(message, "\n");

            // Prepare message to send
            PipeClient toSend[1];
            toSend->port = port;
            toSend->type = 0;

            toSend->message = message;

            // write to pipeClient
            write(fdenvoi, toSend, sizeof(toSend));
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
                toSend->pid = pid;

                write(fdenvoi, toSend, sizeof(toSend));
                mylog(fdrpc, toSend->type, "REQUEST");
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

void mylog(int fdrpc, int type, char *message)
{
    Trace toSendTrace[1];
    toSendTrace->type = type;
    toSendTrace->message = message;
    write(fdrpc, toSendTrace, sizeof(toSendTrace));
}