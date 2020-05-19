#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>

int tube[2];

void sigusr1_handler(int signum, siginfo_t *info, void* pasUtileIci)
{
    printf("Received SIGUSR1 !\n");
    char buf[8];
    if(read(tube[0], buf, sizeof(size_t)) == -1) {
        perror("Error read 1 fils");
        exit(0);
    }
    size_t nb_lettre = (size_t) *buf;
    char buffer[nb_lettre];
    if(read(tube[0], buffer, nb_lettre) == -1) {
        perror("Error read 2 fils");
        exit(0);
    }
    buffer[nb_lettre] = '\0';
    printf("Message reçu : %s\n", buffer);
    kill(info->si_pid, SIGUSR2);

}

void sigusr2_handler(int signum, siginfo_t *info, void* pasUtileIci)
{
    printf("Received SIGUSR2 !\n");
    kill(info->si_pid, SIGQUIT);
}


int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("Usage   : ./pipe.o chaine\n");
        printf("Exemple : ./pipe.o Bonjour\n");
        exit(1);
    }
    
    if(pipe(tube)) {
        printf("Erreur lors de la creation du tube");
        perror("Error sigsuspend");
    }

    pid_t fork_res = fork();
    switch (fork_res)
    {
    case -1:
        printf("Erreur dans la creation du processus fils.\n");
        perror("Erreur fork");
        break;
    case 0:
        {
            printf("Fils : PID=%d\n", getpid());
            // Creation signal siguser1
            struct sigaction prepaSignal;
            prepaSignal.sa_sigaction=sigusr1_handler;
            prepaSignal.sa_flags=SA_SIGINFO;
            if(sigemptyset(&prepaSignal.sa_mask)){
                perror("Error sigemptyset fils"); 
            }
            if(sigaction(SIGUSR1, &prepaSignal, NULL)){
                perror("Error sigaction fils");
            }
            
            sigset_t *masque = malloc(sizeof(sigset_t));
            if(sigemptyset(masque)) {
                perror("Error sigemptyset fils");
            }
            if (sigdelset(masque, SIGUSR1)){
                perror("Error sigdelset fils");
            }
            sigsuspend(masque);
            exit(0);
        }
    default:
        {
            printf("Pere : PID=%d\n", getpid());
            // creation handler signal siguser2
            struct sigaction prepaSignal;
            prepaSignal.sa_sigaction=sigusr2_handler;
            prepaSignal.sa_flags=SA_SIGINFO;
            if(sigemptyset(&prepaSignal.sa_mask)){
                perror("Error sigemptyset pere");
            }
            if(sigaction(SIGUSR2, &prepaSignal, NULL)){
                perror("Error sigaction pere");
            }

            if(close(tube[0])){
               perror("Error close pere"); 
            }
            size_t len = strlen(argv[1]);
            if(write(tube[1], &len , sizeof(size_t)) == -1){
                perror("Error write 1 père");
            }
            if(write(tube[1], argv[1] , len) == -1){
                perror("Error write 2 père");
            }
            // La sieste
            sleep(1);
            // Envoie du signal
            if(kill(fork_res, SIGUSR1)) {
                perror("Error kill pere to fils");
            }

            sigset_t *masque = malloc(sizeof(sigset_t));
            if(sigemptyset(masque)) {
                perror("Error sigemptyset pere");
            }
            if (sigdelset(masque, SIGUSR2)) {
                perror("Error sigdelset pere");
            }
            sigsuspend(masque);
        }
    }
    return 0;
}