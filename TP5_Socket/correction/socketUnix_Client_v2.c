#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>

int main (int argc, char** argv)
 {
  int                sockfd;
  socklen_t          servlen;
  struct sockaddr_un serv_addr;

  if ( (sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
   {
    printf ("Erreur de creation de socket\n"); 
    exit (1); 
   }

  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sun_family = AF_UNIX;

  strcpy(serv_addr.sun_path, "/tmp/socketUnix");
  servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);

  if ( connect(sockfd, (struct sockaddr *) &serv_addr, servlen) < 0)
   {
    perror ("Erreur de creation de connect\n");
    exit (1);
   }

  char requete [50] = "";
   
  sprintf ( requete, "Bonjour, moi c'est le client pid=%d", getpid() );
  
  unsigned long tailleRequete = strlen (requete) + 1;
  
  write  (sockfd, &tailleRequete, sizeof (unsigned long) );
  write  (sockfd, requete, tailleRequete );
  
  printf ("Client (Pid=%d) envoie : %s (%lu)\n", getpid (), requete, tailleRequete);

  unsigned long tailleReponse;
  
  read (sockfd, &tailleReponse, sizeof (unsigned long) );
  
  char* reponse = (char*) malloc ( tailleReponse );
  
  if (reponse)
        {
         read (sockfd, reponse, tailleReponse );
         printf ("Client (pid=%d) reçoit : %s (%lu) \n", getpid (), reponse, tailleReponse);
        }
   else {
         perror ("Client : erreur de malloc");
        }

  close(sockfd);

  return 0;
 }

