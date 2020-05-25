#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

int descripteur[2];

void fct(int signalRecu, siginfo_t *info, void *pasUtileIci)
{
  switch (signalRecu)
  {
  case SIGUSR1:
  {
    unsigned char nbOctets;
    char *message;

    printf("Le processus fils a reçu SIGUSR1\n");

    read(descripteur[0], &nbOctets, 1);

    message = (char *)calloc(nbOctets + 1, 1);

    if (message != NULL)
    {
      read(descripteur[0], message, nbOctets);

      printf("Processus fils - Reception de \"%s\" (%u)\n", message, nbOctets);
    }

    kill(info->si_pid, SIGUSR2);
    break;
  }

  case SIGUSR2:
    printf("Le processus pere a reçu SIGUSR2 et envoie SIGTERM vers le processus fils\n");
    kill(info->si_pid, SIGTERM);
  }
}

int main(int argc, char **argv)
{
  pipe(descripteur);

  struct sigaction prepaSignal;
  prepaSignal.sa_sigaction = &fct;
  prepaSignal.sa_flags = SA_SIGINFO;

  sigemptyset(&prepaSignal.sa_mask);

  pid_t pid = fork();

  switch (pid)
  {
  case -1:
    perror("Impossible de creer un processus fils");
    exit(-1);

  case 0:
    printf("Processus fils - PID = %d\n", getpid());

    sigaction(SIGUSR1, &prepaSignal, NULL);

    while (1)
      pause();

    break;

  default:
  {
    char *messageAEnvoyer = "Bonjour, il fait beau !";
    unsigned char nbOctets = strlen(messageAEnvoyer);

    printf("Processus pere - PID = %d\n", getpid());

    sleep(2);

    sigaction(SIGUSR2, &prepaSignal, NULL);

    write(descripteur[1], &nbOctets, 1);

    write(descripteur[1], messageAEnvoyer, nbOctets);

    printf("Processus pere - Envoi de \"%s\" (%u)\n", messageAEnvoyer, nbOctets);

    kill(pid, SIGUSR1);

    wait(NULL);
  }
  }
}
