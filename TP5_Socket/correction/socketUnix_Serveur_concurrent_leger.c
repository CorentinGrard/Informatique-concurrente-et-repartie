#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <pthread.h>
#include <semaphore.h>
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

typedef struct
 {
  pthread_t    thread;
  unsigned int tid;
  int          descripteur;
  sem_t*       semaphore;
 }
 Connexion;

void* traiterRequete ( void* pt )
 {
  Connexion* connexion = (Connexion*) pt;
  
  sem_wait (connexion->semaphore);
     
  printf ("Connexion client (TID=%d) : activé\n", connexion->tid);

  unsigned long tailleMessage;

  read (connexion->descripteur, &tailleMessage, sizeof (unsigned long));

  printf ("Connexion client (TID=%d) : Allocation de %lu octets\n", connexion->tid, tailleMessage);

  char* message = (char*) malloc (tailleMessage);
                        
  if ( message )
        {
         read (connexion->descripteur, message, tailleMessage);
                                  
         printf ("Connexion client (TID=%d) : Reception de \"%s\" (%lu)\n", connexion->tid, message, tailleMessage);
                               
         char reponse [50] = "";
                               
         sprintf ( reponse , "Reponse du serveur via le processus leger %d", connexion->tid );
                               
         unsigned long tailleReponse = strlen (reponse) + 1;
                               
         write (connexion->descripteur, &tailleReponse, sizeof (unsigned long) );
         write (connexion->descripteur, reponse, tailleReponse );
                               
        for (long i=0; i<1000000000; i++);
        }
   else {
         perror ("Connexion client : Erreur de malloc");
        }
                              
  close (connexion->descripteur);
                        
  connexion->descripteur = -1;
  
  sem_post (connexion->semaphore);
  
  printf ("Connexion client (TID=%d) : desactive\n", connexion->tid);
  
  return NULL;
 }
 
#define MAX_CONNEXIONS 10
 
int main (int argc, char** argv)
 {
  int                sockfd;
  socklen_t          servlen;
  struct sockaddr_un serv_addr;
  sem_t              semaphore;
  
  sem_init ( &semaphore, 0, MAX_CONNEXIONS );
  
  Connexion connexions [MAX_CONNEXIONS];
                     
  for ( int i=0; i<MAX_CONNEXIONS; i++ )
   {
    connexions[i].tid         = i+1;
    connexions[i].descripteur = -1;
    connexions[i].semaphore   = &semaphore;
   }    
  
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
           printf ("serveur (PID=%d) : Connexion detectee...\n", getpid());
    
           int indice = -1;
           /*
           do 
            {
             for (int i=0; i<MAX_CONNEXIONS && indice==-1; i++)
              if ( connexions[i].descripteur == -1 )
               indice = i;
            }
           while ( indice == -1 );  // attente active
           */
           
           sem_wait (&semaphore);
           
           for (int i=0; i<MAX_CONNEXIONS && indice==-1; i++)
            if ( connexions[i].descripteur == -1 )
             indice = i;
           
           connexions[indice].descripteur = newsockfd;

           sem_post (&semaphore);

           printf ("serveur (PID=%d) : Connexion %d prise en charge par le thread %d\n", getpid(), connexions[indice].descripteur, connexions[indice].tid);

           if ( pthread_create ( &(connexions[indice].thread), NULL, traiterRequete, connexions+indice ) != 0 )
            {
             perror ("serveur: Erreur de lancement du thread");
             close ( connexions[indice].descripteur );
             connexions[indice].descripteur = -1;
            }

           /*
           Connexion* connexion = (Connexion*) malloc (sizeof (Connexion));
           
           connexion->tid         = 0;
           connexion->descripteur = newsockfd;
         
           if ( connexion )
            {
             if ( pthread_create ( &(connexion->thread), NULL, traiterRequete, connexion ) != 0 )
              {
               perror ("serveur: Erreur de lancement du thread");
              }
            }
           */
          }
   }
   while ( !fin );
   
  sem_destroy (&semaphore);

  return 0;
 }


