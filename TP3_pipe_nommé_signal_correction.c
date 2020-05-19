#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

int descripteur;

void fct (int signalRecu, siginfo_t* info, void* pasUtileIci)
 {
  switch (signalRecu)
   {
    case SIGUSR1 : {
                    unsigned char nbOctets;
                    char*         message; 

                    printf ("Le processus fils a reçu SIGUSR1\n");

                    read (descripteur, &nbOctets, 1);

                    printf ("Lecture de %u octets\n", nbOctets);

                    message = (char*) calloc (nbOctets+1, 1);

                    if (message != NULL)
                     {
                      read (descripteur, message, nbOctets);

                      printf ("Processus fils - Reception de \"%s\" (%u)\n", message, nbOctets);
                     }

                    kill (info->si_pid, SIGUSR2);
                    break;
                   }

    case SIGUSR2 :  printf ("Le processus pere a reçu SIGUSR2 et envoie SIGTERM vers le processus fils\n");
                    kill (info->si_pid, SIGTERM);
   }
 }

int main (int argc, char** argv) 
 {
  unlink ("/tmp/tubeNomme");   /* On essaye de detruire le tube s'il existe deja */

  if ( mkfifo ( "/tmp/tubeNomme", S_IFIFO | 0666) == -1)
   {
    perror ("Creation du tube nomme impossible");
    exit (-1);
   }

  printf ("Creation du tube nomme /tmp/tubeNomme\n");

  struct sigaction prepaSignal;
                   prepaSignal.sa_sigaction = &fct;
                   prepaSignal.sa_flags     = SA_SIGINFO;

  sigemptyset(&prepaSignal.sa_mask);

  pid_t pid = fork ();

  switch (pid)
   {
    case -1 : perror ("Impossible de creer un processus fils");
              exit (-1);

    case  0 : printf ("Processus fils - PID = %d\n", getpid ());

              sigaction (SIGUSR1, &prepaSignal, NULL);

              if ((descripteur = open ("/tmp/tubeNomme", O_RDONLY, 0)) == -1)
               {
                perror ("Ouverture impossible du tube nomme en lecture");
                exit (-1);
               }

              while (1);

              break; 

    default : {
               char*         messageAEnvoyer = "Bonjour, il fait beau !";
               unsigned char nbOctets        = strlen (messageAEnvoyer);            

               printf ("Processus pere - PID = %d\n", getpid ());

               sleep (2);

               sigaction (SIGUSR2, &prepaSignal, NULL);

               if ((descripteur = open ("/tmp/tubeNomme", O_WRONLY, 0) )== -1) /* Bloqué si pas de lecteur */
                {
                 perror ("Ouverture impossible du tube nomme en lecture");

                 kill (pid, SIGTERM);           

                 exit (-1);
                }

               write (descripteur, &nbOctets, 1);

               write (descripteur, messageAEnvoyer, nbOctets);
   
               printf ("Processus pere - Envoi de \"%s\" (%u)\n", messageAEnvoyer, nbOctets);

               kill (pid, SIGUSR1);

               wait (NULL);
              }
   }
 }
