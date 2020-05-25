#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>

int main(int argc, char **argv)
{
  int sockfd, servlen;
  struct sockaddr_un serv_addr;
  char tampon[30];
  int nbOctets;
  bzero((char *)&serv_addr, sizeof(serv_addr));
  serv_addr.sun_family = AF_UNIX;
  strcpy(serv_addr.sun_path, "/tmp/socketLocale.2");
  servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);
  if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
  {
    printf("Erreur de creation de socket\n");
    exit(1);
  }
  if (connect(sockfd, (struct sockaddr *)&serv_addr, servlen) < 0)
  {
    printf("Erreur de creation de connect\n");
    exit(1);
  }
  write(sockfd, "Bonjour le serveur !", 20);
  printf("Client envoie : Bonjour le serveur !\n");
  nbOctets = 0;
  bzero((char *)tampon, 30);
  read(sockfd, tampon, 13);
  printf("Client reçoit : %s\n", tampon);
  close(sockfd);
  return 0;
}