#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <fcntl.h>

typedef struct SharedMem
{
    int descripteur;
    void *adresse;
} MemoirePartagee;

int P(int _idSemaphore);
int V(int _idSemaphore);
int creerSemaphore(int _compteur);
int detruireSemaphore(int _idSem);
char prendre(char *fifo);
void placer(char *fifo, char *toSend);

int main(int argc, char *argv[])
{
    int taille = 8;
    // Creation du buffer
    MemoirePartagee buffer = superMalloc(taille);

    // Creation des semaphores
    int plein = creerSemaphore(0);
    int vide = creerSemaphore(taille);
    int mutex = creerSemaphore(1);

    pid_t pid_consommateur = fork();
    pid_t pid_producteur;
    int etat;
    switch (pid_consommateur)
    {
    case -1:
        printf("Erreur dans la creation du processus fils.\n");
        perror("Erreur");
        break;
    case 0:
    {
        printf("Fils : PID=%d\n", getpid());
        while (1)
        {
            P(plein);
            P(mutex);
            char result = prendre(buffer, );
            V(mutex);
            V(vide);
            // Print
            printf("%c\n", result);
        }
        printf("\n");
        printf("Fils : Fin\n");
        exit(0);
        break
    }
    default:
    {
        printf("Pere : PID=%d\n", getpid());
        char message[] = "Bonjour";
        for (int i = 0; i < strlen(message) + 1; i++)
        {
            char toSend = message[i];
            P(vide);
            P(mutex);
            placer(buffer, &toSend);
            V(mutex);
            V(plein);
        }
        pid_producteur = wait(&etat);
        printf("Pere : Le fils %d s'est arrete avec le code %d\n", pid_producteur, etat);
        printf("Pere : Fin\n");
    }
    }
    return 0;
}

int creerSemaphore(int _compteur)
{
    int idSem = semget(IPC_PRIVATE, 1, 0666 | IPC_CREAT | IPC_EXCL);
    semctl(idSem, 0, SETVAL, _compteur);
    return idSem;
}

int detruireSemaphore(int _idSem)
{
    return semctl(_idSem, 0, IPC_RMID, 0);
}

int P(int _idSemaphore)
{
    struct sembuf sem;
    sem.sem_num = 0;
    sem.sem_op = -1;
    sem.sem_flg = 0;
    return semop(_idSemaphore, &sem, 1);
}

int V(int _idSemaphore)
{
    struct sembuf sem;
    sem.sem_num = 0;
    sem.sem_op = 1;
    sem.sem_flg = 0;
    return semop(_idSemaphore, &sem, 1);
}

char prendre(char *fifo)
{
    int fd = open(fifo, O_RDONLY);
    if (fd < 0)
    {
        perror("Erreur");
    }
    char *buf;
    int res = read(fd, buf, sizeof(char));
    if (res < 0)
    {
        perror("Erreur");
    }
    close(fd);
    return *buf;
}

void placer(char *fifo, char *toSend)
{
    int fd = open(fifo, O_WRONLY);
    if (fd < 0)
    {
        perror("Erreur");
    }
    int res = write(fd, toSend, sizeof(char));
    if (res < 0)
    {
        perror("Erreur");
    }
    close(fd);
}

MemoirePartagee superMalloc(int taille)
{
    MemoirePartagee m;
    m.descripteur = shmget(IPC_PRIVATE, taille, IPC_CREAT | IPC_EXCL | 0600);
    m.adresse = (void *)-1; /* On suppose que ça se passe mal */
    if (m.descripteur != -1)
    {
        m.adresse = shmat(m.descripteur, NULL, 0);
    }
    return m;
}

int superFree(MemoirePartagee m)
{
    int retour = shmdt(m.adresse);
    if (retour != -1)
    {
        retour = shmctl(m.descripteur, IPC_RMID, 0);
    }
    return retour;
}