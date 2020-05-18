/*****************************************************/
/* Producteur-consommateur avec des processus lourds */
/*****************************************************/
/* Version du 24 janvier 2015                        */
/* Avec une chaine reçue de taille fixe (40 carac.)  */
/*****************************************************/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <semaphore.h>
#include <pthread.h>

/************************************************************************************************/
/*          G E S T I O N   D E   L A   Z O N E   D E   M E M O I R E   P A R T A G E E         */
/************************************************************************************************/

typedef struct SharedMem
{
    int taille;
    char *buffer;
    int adresse;
} MemoirePartagee;

typedef struct args
{
    sem_t plein;
    sem_t vide;
    sem_t mutex;
    MemoirePartagee m;
} args;
/************************************************************************************************/
/*                              G E S T I O N   D U   T A M P O N                               */
/************************************************************************************************/

/************************************************************************************************/
/* Les fonction de gestion de la zone de memoire commune ont permis de créer un tampon pour     */
/* echanger des messages (une suite de char). Le premier element de ce tableau sera l'index du  */
/* dernier element insere afin qu'il soit connu du producteur et du consommateur : le tempon a  */
/* donc une taille d'au plus 256 octets par construction (1 octet pour conserver le nombre      */
/* d'éléments effectivement contenus dans le tampon et au plus 255 cases).                      */
/*                                                                                              */
/* Ce buffer se comporte comme une queue de type FIFO :                                         */
/*                                                                                              */
/*  - le producteur empile les elements a l'adresse bufferCommun + (255 - nbElem) et incremente */
/*    nbEleme (A est l'adresse (bufferCommun + 256), B a l'adresse (bufferCommun + 255) ...etc  */
/*  - le consommateur recupere toujours l'element de l'adresse (bufferCommun + 256) puis il     */
/*    deplace tous les elements du tableau d'une case et decremente le compteur nbEleme         */
/*                                                                                              */
/*          +----------+---+---+---+- - - - - - - - - -+---+---+---+                            */
/*  ABC ->  | nbElem=3 |   |   |   |                   | C | B | A |                            */
/*          +----------+---+---+---+- - - - - - - - - -+---+---+---+                            */
/*                                                                                              */
/************************************************************************************************/

void initialiser(MemoirePartagee *m)
{
    int index;

    (m->adresse) = 0;

    for (index = 1; index <= m->taille - 1; index++)
        m->buffer[(m->adresse + index)] = ' ';
}

int getNombreDeCaracteresDansLeBufferCommun(MemoirePartagee *m)
{
    return (m->taille);
}

char getElement(MemoirePartagee *m)
{
    char caractereARetourner = m->buffer[(m->adresse + m->taille - 1)];
    int index;

    for (index = m->taille - 1; index > 1; index--)
    {
        m->buffer[(m->adresse + index)] = m->buffer[(m->adresse + index - 1)];
    }

    m->adresse = m->adresse - 1;

    return caractereARetourner;
}

void addElement(MemoirePartagee *m, char _caractereAAjouter)
{
    printf("test %d\n",(m->adresse));
    m->buffer[(m->adresse - 1 - getNombreDeCaracteresDansLeBufferCommun(m))] = _caractereAAjouter;
    m->buffer[m->adresse] = (m->adresse) + 1;
}

/************************************************************************************************/
/*                         G E S T I O N   D E S   S E M A P H O R E S                          */
/************************************************************************************************/

sem_t creerSemaphore(int _compteur)
{
    sem_t sem;
    int test = sem_init(&sem, 0, _compteur);
    if (test < 0)
    {
        perror("Error");
        exit(1);
    }
    return sem;
}

int detruireSemaphore(sem_t sem)
{
    return sem_destroy(&sem);
}

int P(sem_t sem)
{
    return sem_wait(&sem);
}

int V(sem_t sem)
{
    return sem_post(&sem);
}

int getCompteur(sem_t sem)
{
    int *res = malloc(sizeof(int));
    sem_getvalue(&sem, res);
    return *res;
}

/************************************************************************************************/
/*                            A F F I C H A G E   D U   T A M P O N                             */
/************************************************************************************************/

/* Affichage du tableau sous la forme : [ , , , , ...  ...,C,B,A]
 * Il faut N cases, (N-1) virgules, 2 crochets et le \0 terminal :
 *     N cases                     donc m->taille-1
 *     N-1 virgules                donc m->taille-1 - 1
 *     2 croches et le \0 terminal donc 2 + 1 
 */

char *getAffichageDuBufferCommun(MemoirePartagee *m)
{
    char *chaineDeCaracteres = (char *)malloc(m->taille - 1 + m->taille - 1 - 1 + 2 + 1);
    int indexDansLaChaineDeCaracteres = 1;

    int indexDansLeBufferCommun;
    int indexDuProchainCaractereDansLeBufferCommun = m->adresse - 1 - getNombreDeCaracteresDansLeBufferCommun(m);

    *chaineDeCaracteres = '[';

    for (indexDansLeBufferCommun = 1; indexDansLeBufferCommun <= m->taille - 1; indexDansLeBufferCommun++)
    {
        if (indexDansLeBufferCommun <= indexDuProchainCaractereDansLeBufferCommun)
        {
            *(chaineDeCaracteres + indexDansLaChaineDeCaracteres) = ' ';
        }
        else
        {
            *(chaineDeCaracteres + indexDansLaChaineDeCaracteres) =
                (((indexDansLeBufferCommun) == '\0') ? ' ' : (indexDansLeBufferCommun));
        }

        indexDansLaChaineDeCaracteres++;

        if (indexDansLeBufferCommun == m->taille - 1)
        {
            *(chaineDeCaracteres + indexDansLaChaineDeCaracteres) = ']';
        }
        else
        {
            *(chaineDeCaracteres + indexDansLaChaineDeCaracteres) = ',';
        }
        indexDansLaChaineDeCaracteres++;
    }

    *(chaineDeCaracteres + indexDansLaChaineDeCaracteres) = 0;

    return chaineDeCaracteres;
}

void afficherTitre(MemoirePartagee *m)
{
    int longueur = strlen(getAffichageDuBufferCommun(m)) - 7;
    char *tirets = (char *)malloc(longueur + 1);
    char *blancs = (char *)malloc(longueur + 1);
    int i;

    for (i = 0; i < longueur; i++)
    {
        *(tirets + i) = '-';
        *(blancs + i) = ' ';
    }

    *(tirets + longueur) = 0;
    *(blancs + longueur) = 0;

    printf("\n");
    printf("\n");
    printf("+--------------+--------------+-----------------------+------------------%s+\n", tirets);
    printf("|  Producteur  | Consommateur |       Semaphores      | buffer commun    %s|\n", blancs);
    printf("|              |              |                       |                  %s|\n", blancs);
    printf("|              |              | Plein | Vide  | Mutex | nbElem | contenu %s|\n", blancs);
    printf("+--------------+--------------+-------+-------+-------+--------+---------%s+\n", tirets);
}

void afficherDerniereLigne(MemoirePartagee *m)
{
    int longueur = strlen(getAffichageDuBufferCommun(m)) - 7;
    char *tirets = (char *)malloc(longueur + 1);
    int i;

    for (i = 0; i < longueur; i++)
    {
        *(tirets + i) = '-';
    }

    *(tirets + longueur) = 0;

    printf("+--------------+--------------+-----------------------+--------+---------%s+\n", tirets);
}

#define RETIRER 1
#define CONSOMMER 2
#define PLACER 3
#define PRODUIRE 4

void afficherLigne(MemoirePartagee *m, sem_t plein, sem_t vide, sem_t mutex, int ordre, char lettre)
{
    switch (ordre)
    {
    case RETIRER:
        printf("|              | Retirer %c    ", ((lettre != '\0') ? lettre : ' '));
        break;
    case CONSOMMER:
        printf("|              | Consommer %c  ", ((lettre != '\0') ? lettre : ' '));
        break;
    case PLACER:
        printf("| Placer %c     |              ", ((lettre != '\0') ? lettre : ' '));
        break;
    case PRODUIRE:
        printf("| Produire %c   |              ", ((lettre != '\0') ? lettre : ' '));
        break;
    }

    printf("|  %3d  |  %3d  |  %3d  |  %3d   | %s |\n", getCompteur(plein),
           getCompteur(vide),
           getCompteur(mutex),
           getNombreDeCaracteresDansLeBufferCommun(m),
           getAffichageDuBufferCommun(m));
}

/************************************************************************************************/
/*                         F O N C T I O N S   U T I L I S E E S   P A R                        */
/*                L E   P R O D U C T E U R   E T   L E   C O N S O M M A T E U R               */
/************************************************************************************************/

char retirerObjet(MemoirePartagee *m)
{
    char c = getElement(m);
    return c;
}

void consommerObjet(MemoirePartagee *m, char c, char *chaineRecue, int numeroLettre)
{
    chaineRecue[numeroLettre] = c;
}

void placerObjet(MemoirePartagee *m, char c)
{
    addElement(m, c);
}

char produireObjet(MemoirePartagee *m, char *chaineAEnvoyer, int numeroLettre)
{
    char c = chaineAEnvoyer[numeroLettre];
    return c;
}

void consommateur(MemoirePartagee *m, sem_t plein, sem_t vide, sem_t mutex, char *chaineRecue) /* Processus consommateur */
{
    int numeroLettre = 0;
    char c = 0;

    do
    {
        P(plein);            /* On essaye de prendre un emplacement plein (contenant une lettre) */
        P(mutex);            /* On bloque le tampon pour etre sur d'etre le seul a le modifier */
        c = retirerObjet(m); /* On retire l'objet (lettre) du tampon) */
        afficherLigne(m, plein, vide, mutex, RETIRER, c);
        V(mutex);                                        /* On débloque le tampon pour permettre son eventuel acces par le producteur */
        V(vide);                                         /* On rend un emplacement vide (pour eventuellement debloquer le producteur) */
        consommerObjet(m, c, chaineRecue, numeroLettre); /* On consomme l'objet */
        afficherLigne(m, plein, vide, mutex, CONSOMMER, c);
        numeroLettre++;
    } while (c != 0);
}

void producteur(MemoirePartagee *m, sem_t plein, sem_t vide, sem_t mutex, char *chaineAEnvoyer) /* Processus producteur */
{
    int numeroLettre = 0;
    char c = 0;

    do
    {
        c = produireObjet(m, chaineAEnvoyer, numeroLettre); /* On produit un objet (une lette) */
        afficherLigne(m, plein, vide, mutex, PRODUIRE, c);
        P(vide);           /* On essaye de prendre un emplacement vide */
        P(mutex);          /* On bloque le tampon pour etre sur d'etre le seul a le modifier */
        printf("Producteur oui\n");
        placerObjet(m, c); /* On met l'objet dans le tampon */
        printf("Producteur grg\n");
        afficherLigne(m, plein, vide, mutex, PLACER, c);
        V(mutex); /* On débloque le tampon pour permettre son eventuel acces par le consommateur */
        V(plein); /* On rend un emplacement plein (pour eventuellement debloquer le consommateur) */
        printf("Producteur fin\n");
        numeroLettre++;
    } while (c != '\0');
}

void *thread_fnc(void *arguments)
{
    int etat;
    args *param = arguments;
    char chaineRecue[40];
    consommateur(&param->m, param->plein, param->vide, param->mutex, chaineRecue);
    afficherDerniereLigne(&param->m);
    printf("\nReception de \"%s\"\n\n", chaineRecue);
    exit(EXIT_SUCCESS);
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        printf("Usage   : ./producteurConsommateur taille chaine\n");
        printf("Exemple : ./producteurConsommateur 4 Bonjour\n");
        exit(1);
    }
    int taille = atoi(argv[1]);
    sem_t plein = creerSemaphore(0);
    sem_t vide = creerSemaphore(taille);
    sem_t mutex = creerSemaphore(1);

    MemoirePartagee *m = (MemoirePartagee *)malloc(sizeof(MemoirePartagee));

    m->taille = taille;
    m->adresse = -1;
    m->buffer = "";

    args *arguments = (args *)malloc(sizeof(args));
    arguments->m = *m;
    arguments->plein = plein;
    arguments->vide = vide;
    arguments->mutex = mutex;

    pthread_t thread;
    pthread_create(&thread, NULL, thread_fnc, arguments);
    printf("\nEnvoi de \"%s\" \n", argv[2]);
    afficherTitre(m);
    producteur(m, plein, vide, mutex, argv[2]);

    pthread_join(thread, NULL);

    return EXIT_SUCCESS;
}