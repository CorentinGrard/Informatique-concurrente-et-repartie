#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <signal.h>

int fin = 0;

void arret (int signalRecu, siginfo_t* info, void* pasUtileIci)
 {
  if ( signalRecu == SIGINT )
   {
    fin = 1;

    printf (" => On demande l'arrêt du serveur !\n");    
   }
 }

int main (int argc, char** argv)
 {
  int                sockfd;
  socklen_t          servlen;
  struct sockaddr_un serv_addr;

  struct sigaction prepaSignal;
                   prepaSignal.sa_sigaction = &arret;
                   prepaSignal.sa_flags     = SA_SIGINFO;

  sigemptyset(&prepaSignal.sa_mask);

  sigaction (SIGINT, &prepaSignal, NULL);
  
  if ( (sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
   {
    printf ("Erreur de creation de socket\n"); 
    exit (1); 
   }

  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sun_family = AF_UNIX;

  strcpy(serv_addr.sun_path, "/tmp/socketUnix");
  servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);

  unlink ("/tmp/socketUnix");

  if ( bind (sockfd, (struct sockaddr *) &serv_addr, servlen) < 0)
   {
    printf ("Erreur de bind\n");
    exit (1);
   }

  listen(sockfd, 5);

  do
   {
    printf ("serveur (PID=%d) : En attente...\n", getpid());

    int newsockfd = accept (sockfd, NULL, 0);    
    
    if (newsockfd < 0)
          {
           perror ("serveur: Erreur de accept");
          }
     else {
           switch ( fork () )
            {
             case -1 : perror ("serveur: Erreur de fork");
                       break;
                       
             case  0 : printf ("Connexion client (PID=%d) : activé\n", getpid());

                       {         
                        unsigned long tailleMessage;

                        read (newsockfd, &tailleMessage, sizeof (unsigned long));

                        char* message = (char*) malloc (tailleMessage);
                        
                        if ( message )
                              {
                               read (newsockfd, message, tailleMessage);
                                  
                               printf ("Connexion client (PID=%d) : Reception de \"%s\" (%lu)\n", getpid(), message, tailleMessage);
                               
                               char reponse [50] = "";
                               
                               sprintf ( reponse , "Reponse du serveur via le processus %d", getpid () );
                               
                               unsigned long tailleReponse = strlen (reponse) + 1;
                               
                               write (newsockfd, &tailleReponse, sizeof (unsigned long) );
                               write (newsockfd, reponse, tailleReponse );
                               
                               sleep (10);
                              }
                         else {
                               perror ("Connexion client : Erreur de malloc");
                              }
                              
                        close(newsockfd);
                        
                        printf ("Connexion client (PID=%d) : desactive\n", getpid());
                               
                        exit (0);
                       }
                       
             default : close(newsockfd);
            }
          }
   }
   while ( !fin );

  return 0;
 }


