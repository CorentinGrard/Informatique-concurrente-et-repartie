#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>

int sockfd, newsockfd;
void *client_response();

void ctrlc_handler(int signum, siginfo_t *info, void *pasUtileIci)
{
    printf("Arret du serveur !");
    close(sockfd);
    unlink("/tmp/socketLocale.1");
    exit(0);
}

int main(int argc, char **argv)
{

    struct sigaction prepaSignal;
    prepaSignal.sa_sigaction = ctrlc_handler;
    prepaSignal.sa_flags = SIGINT;
    if (sigaction(SIGINT, &prepaSignal, NULL))
    {
     
    listen(sockfd, 5);

    pthread_t sniffer_thread;
    while(true) {
        if(pthread_create(&sniffer_thread , NULL , client_response) < 0) {
            perror("could not create thread");
            return 1;
        }
    }
    pthread_exit(NULL);
    return 0;
}

    char tampon[30];
    int nbOctets;
    if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
    {
        printf("Erreur de creation de socket\n");
        exit(1);
    }
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sun_family = AF_UNIX;
    strcpy(serv_addr.sun_path, "/tmp/socketLocale.1");
    servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);
    if (bind(sockfd, (struct sockaddr *)&serv_addr, servlen) < 0)
    {
        printf("Erreur de bind\n");
        exit(1);
    }
    listen(sockfd, 5);    listen(sockfd, 5);
        clilen = sizeof(cli_addr);
        printf("serveur: En attente...\n");
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if (newsockfd < 0)
            printf("serveur: Erreur de accept\n");
        int nbOctets = 0;
        exit(1);
    }
    listen(sockfd, 5);    listen(sockfd, 5);
        clilen = sizeof(cli_addr);
        printf("serveur: En attente...\n");
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if (newsockfd < 0)
            printf("serveur: Erreur de accept\n");
        int nbOctets = 0;
        bzero((char *)tampon, 30);
        read(newsockfd, tampon, 20);
        printf("Serveur reçoit : %s\n", tampon);
        write(newsockfd, "Message reçu", 13);
        printf("Serveur envoie : Message recu\n");
        close(newsockfd);
}
























































