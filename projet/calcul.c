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
    int port = args->port;

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
            mylog(fdrpc, 2, "RECEIVE REPLY");
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
                    mylog(fdrpc, 3, "SEND RELEASE");
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
            mylog(fdrpc, 3, "RECEIVE RELEASE");
            queueIndex--;
        }
        // IF TYPE == REQUEST
        else if (data->type == 1)
        {
            // Store new request
            Item request;
            request.horloge = data->horloge;
            request.pid = data->pid;
            for (int i = 0; i < queueIndex + 1; i++)
            {
                // Sort by horloge
                if (request.horloge <= queue[i].horloge || queue[i].horloge == 0)
                {
                    queueIndex++;
                    for (int j = queueIndex; j > i + 1; j--)
                    {
                        queue[j] = queue[j - 1];
                    }
                    queue[i] = request;
                    mylog(fdrpc, 1, "RECEIVE REQUEST");
                    // Send the reply
                    PipeClient toSend[1];
                    toSend->type = 2;
                    toSend->port = data->RequestPort;
                    write(fdenvoi, toSend, sizeof(toSend));
                    mylog(fdrpc, 2, "SEND REPLY");
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
            int portPick = serveursPorts[portIndex];

            // Prepare message to send
            PipeClient toSend[1];
            toSend->port = portPick;
            toSend->type = 0;

            // Generate message
            strcpy(toSend->message, randstring());
            // write to pipeClient
            write(fdenvoi, toSend, sizeof(toSend));
            break;
        case 2:
            if (!wantSC)
            {
                // Veux SC
                horloge++;
                wantSC = true;
                for (int i = 0; i < nbServeurs; i++)
                {
                    PipeClient toSend[1];
                    toSend->port = serveursPorts[i];
                    toSend->type = 1;
                    toSend->horloge = horloge;
                    toSend->pid = pid;
                    toSend->RequestPort = port;

                    write(fdenvoi, toSend, sizeof(toSend));
                    mylog(fdrpc, toSend->type, "SEND REQUEST");
                }
            }
            break;
        default:
            printf("Calcul : erreur pick option");
            break;
        }
        sleep(1);
    }
    close(fdreception);
    close(fdenvoi);
}

void mylog(int fdrpc, int type, char *message)
{
    Trace toSendTrace[1];
    toSendTrace->type = type;
    strcpy(toSendTrace->message, message);
    write(fdrpc, toSendTrace, sizeof(toSendTrace));
}

char *randstring()
{

    size_t length = 10;
    static char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789,.-#'?!";
    char *randomString = NULL;

    if (length)
    {
        randomString = malloc(sizeof(char) * (length + 1));

        if (randomString)
        {
            for (int n = 0; n < length; n++)
            {
                int key = rand() % (int)(sizeof(charset) - 1);
                randomString[n] = charset[key];
            }

            randomString[length] = '\0';
        }
    }

    return randomString;
}