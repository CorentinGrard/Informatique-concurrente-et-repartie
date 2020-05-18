/*******************************************************/
/* Producteur - Consommateur avec les thread POSIX et  */
/* la biblioth�que semaphore.h - version du 23/01/2004 */
/*******************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>

#define N 4

sem_t* plein;
sem_t* vide;
sem_t* mutex;

char*  bufferCommun;
int    tourProducteur;
int    tourConsommateur;
int    nombreDeTours;
char*  chaineAEnvoyer;
char*  chaineRecue;

unsigned int getCompteur (sem_t* _semaphore)
 {
  unsigned int valeur = 0;
  
  sem_getvalue (_semaphore, &valeur);
  return valeur;
 }

sem_t* creerSemaphore (unsigned int _compteur)
 {
  sem_t* semaphore = (sem_t*) malloc (sizeof(sem_t)); 
  
  if (sem_init (semaphore, 0, _compteur) == 0) 
        {
         printf("Creation d'un semaphore ayant pour valeur initiale %d\n", getCompteur (semaphore)); 
        }
   else {
         perror("Echec dans la cr�ation du s�maphore ! \n");
        }
  return semaphore;
 }

int detruireSemaphore (sem_t* _semaphore)
 {
  unsigned int resultat = sem_destroy(_semaphore);
  free (_semaphore); 
  
  return resultat;
 }

int P (sem_t* _semaphore)
 {
  return sem_wait (_semaphore);
 }

int V (sem_t* _semaphore)
 {
  return sem_post (_semaphore);
 }

/***************************************************************************************************/
/*                          G E S T I O N   D U   B U F F E R   C O M M U N                        */
/***************************************************************************************************/

/***************************************************************************************************/
/* Initialisation d'une zone de memoire commune pour echanger des messages (une suite de char). Le */
/* Le premier element de ce tableau sera l'index du dernier element insere afin qu'il soit connu   */
/* du producteur et du consommateur : le tempon a donc une taille de 256 octets par construction.  */
/*                                                                                                 */
/* Ce buffer se comporte comme une queue de type FIFO :                                            */
/*  - le producteur empile les elements a l'adresse bufferCommun + (255 - nbElem) et incremente    */
/*    nbEleme (A est l'adresse (bufferCommun + 256), B a l'adresse (bufferCommun + 255) ...etc     */
/*  - le consommateur recupere tourjous l'element de l'adresse (bufferCommun + 256) puis il        */
/*    deplace tous les elements du tableau d'une case et decremente le compteur nbEleme            */
/*                                                                                                 */
/*          +----------+---+---+---+- - - - - - - - - -+---+---+---+                               */
/*  ABC ->  | nbElem=3 |   |   |   |                   | C | B | A |                               */
/*          +----------+---+---+---+- - - - - - - - - -+---+---+---+                               */
/*                                                                                                 */
/***************************************************************************************************/

void superMalloc ()
 {
      bufferCommun          = (void*) malloc (N + 1);
  int index                 = 0;
  
  *bufferCommun = 0;

  for (index=1; index<=N; index++)
   *(bufferCommun + index) = ' ';
 }

int superFree ()
 {
  free  (bufferCommun);
 }

int getNombreDeCaracteresDansLeBufferCommun ()
 {
  return (int) *bufferCommun;
 }

char getElement ()
 {
  char caractereARetourner = *(bufferCommun + N);
  int  index;
   
  for (index = N; index > 1; index--)
   {
    *(bufferCommun + index) = *(bufferCommun + index - 1);
   }

  *bufferCommun = (char) ((int) *bufferCommun - 1);

  return caractereARetourner;
 }

/* Affichage du tableau sous la forme : [ , , , , ...  ...,C,B,A]
 * Il faut N cases, (N-1) virgules, 2 crochets et le \0 terminal */

char* getAffichageDuBufferCommun ()
 {
  char* chaineDeCaracteres = (char*) malloc (N + N - 1 + 2 + 1);
  int   indexDansLaChaineDeCaracteres = 1;
  
  int   indexDansLeBufferCommun;
  int   indexDuProchainCaractereDansLeBufferCommun = N - getNombreDeCaracteresDansLeBufferCommun();
  
  *chaineDeCaracteres = '[';

  for (indexDansLeBufferCommun = 1; indexDansLeBufferCommun <= N; indexDansLeBufferCommun++)
   {
    if (indexDansLeBufferCommun <= indexDuProchainCaractereDansLeBufferCommun)
          {
	   *(chaineDeCaracteres + indexDansLaChaineDeCaracteres) = ' ';
	  }
     else {
	   *(chaineDeCaracteres + indexDansLaChaineDeCaracteres) = *(bufferCommun + indexDansLeBufferCommun);
	  }
     
    indexDansLaChaineDeCaracteres++;
    
    if (indexDansLeBufferCommun == N)
	  {
	   *(chaineDeCaracteres + indexDansLaChaineDeCaracteres) = ']';
	  }
     else {
           *(chaineDeCaracteres + indexDansLaChaineDeCaracteres) = ',';
          }
    indexDansLaChaineDeCaracteres++;
   }

  *(chaineDeCaracteres + indexDansLaChaineDeCaracteres) = 0;

  return chaineDeCaracteres;
 }

void addElement (char _caractereAAjouter)
 {
  *(bufferCommun + N - getNombreDeCaracteresDansLeBufferCommun()) = _caractereAAjouter;
  *bufferCommun = (char) ((int) *bufferCommun + 1);
 }




void afficherTitre ()
 {
  int   longueur = strlen (getAffichageDuBufferCommun ()) - 7;
  char*	tirets   = (char*) malloc (longueur+1);
  char* blancs   = (char*) malloc (longueur+1);
  int   i;
  
  for (i=0; i<longueur; i++) 
   {
    *(tirets + i) = '-';
    *(blancs + i) = ' ';
   }

  *(tirets + longueur) = 0;
  *(blancs + longueur) = 0;
    
  printf ("\n");
  printf ("\n");
  printf ("+--------------+--------------+-----------------------+------------------%s+\n", tirets);
  printf ("|  Producteur  | Consommateur |       Semaphores      | buffer commun    %s|\n", blancs);
  printf ("|              |              |                       |                  %s|\n", blancs);
  printf ("|              |              | Plein | Vide  | Mutex | nbElem | contenu %s|\n", blancs);
  printf ("+--------------+--------------+-------+-------+-------+--------+---------%s+\n", tirets);
 }

void afficherDerniereLigne ()
 {
  int   longueur = strlen (getAffichageDuBufferCommun ()) - 7;
  char* tirets   = (char*) malloc (longueur+1);
  int   i;
 
  for (i=0; i<longueur; i++)
   {
    *(tirets + i) = '-';
   }
 
  *(tirets + longueur) = 0;
 
  printf ("+--------------+--------------+-----------------------+--------+---------%s+\n", tirets);
 }

void retirerObjet ()
 {
  *(chaineRecue+tourConsommateur)=getElement(); 
  printf ("|              | Retirer      |  %3d  |  %3d  |  %3d  |  %3d   | %s |\n", 
		    getCompteur (plein),
		    getCompteur (vide),
		    getCompteur (mutex),
		    getNombreDeCaracteresDansLeBufferCommun(),
		    getAffichageDuBufferCommun() );
 }

void consommerObjet ()
 {
  printf ("|              | Consommer    |  %3d  |  %3d  |  %3d  |  %3d   | %s |\n", 
		    getCompteur (plein),
		    getCompteur (vide),
		    getCompteur (mutex),
		    getNombreDeCaracteresDansLeBufferCommun(),
		    getAffichageDuBufferCommun() );
 }

void mettreObjet ()
 {
  addElement (*(chaineAEnvoyer+tourProducteur));	 
  printf ("| Placer       |              |  %3d  |  %3d  |  %3d  |  %3d   | %s |\n", 
		    getCompteur (plein),
		    getCompteur (vide),
		    getCompteur (mutex),
		    getNombreDeCaracteresDansLeBufferCommun(),
		    getAffichageDuBufferCommun() );
 }

void produireObjet ()
 {
  printf ("| Produire     |              |  %3d  |  %3d  |  %3d  |  %3d   | %s |\n", 
		    getCompteur (plein),
		    getCompteur (vide),
		    getCompteur (mutex),
		    getNombreDeCaracteresDansLeBufferCommun(),
		    getAffichageDuBufferCommun()  );
 }

void consommateur ()   /* Processus consommateur */
 {
  while (tourConsommateur<nombreDeTours)  
   {  
    P (plein);         /* Attente d'un objet */ 
    P (mutex);         /* On bloque la file */ 
    retirerObjet ();   /* Consommer l'objet courant */ 
    V (mutex);         /* Liberation de la file */ 
    V (vide);          /* Une place est a prendre */ 
    consommerObjet (); /* Consommer l'objet courant */ 
    tourConsommateur++;
   } 
   
  afficherDerniereLigne();
		
  printf ("\n\nMessage re�u : %s\n\n\n", chaineRecue);
 } 

void producteur ()     /* Processus producteur */
 {  
  while (tourProducteur<nombreDeTours)             
   {  
    produireObjet ();  /* Produire un objet */ 
    P (vide);          /* On veut une place vide */ 
    P (mutex);         /* On bloque la file */ 
    mettreObjet ();    /* Mettre l'objet en file */ 
    V (mutex);         /* Liberation de la file */ 
    V (plein);         /* Un objet est a prendre */ 
    tourProducteur++;
   } 
 } 


int main (int argc, char* argv[])
 {
  pthread_t threadConsommateur; /* Structure de donn�e pour accueillir le thread */

  printf ("\n\n");

  plein = creerSemaphore (0);
  vide  = creerSemaphore (N);
  mutex = creerSemaphore (1);
  tourProducteur  = 0;
  tourConsommateur  = 0;
 
  int cumulDesLongueurs = 0;
  int mot               = 1;

  for (mot=1; mot<argc; mot++)
   cumulDesLongueurs = cumulDesLongueurs + strlen (*(argv+mot)) + 1;

  chaineAEnvoyer = (char*) malloc (cumulDesLongueurs);
  chaineRecue    = (char*) malloc (cumulDesLongueurs);
  
  for (mot=1; mot<argc; mot++)
   {
    strcat (chaineAEnvoyer, *(argv+mot));
    strcat (chaineAEnvoyer, " ");
   }

  printf ("\nChaine a envoyer : %s\n", chaineAEnvoyer);
  
  nombreDeTours = strlen(chaineAEnvoyer);
  
  superMalloc (N);
  
  afficherTitre();
  
  /***********************************/
  /* Creation du thread consommateur */
  /***********************************/

  if (pthread_create(&threadConsommateur, NULL, (void *(*)()) consommateur, NULL) == -1)
   {
    perror("Unable to create the thread");
   }

  /****************************************************/
  /* Processus de production dans le thread principal */
  /****************************************************/

  producteur ();

  /**********************************************************/
  /* Le thread principal attend la fin du thread secondaire */
  /**********************************************************/

  pthread_join(threadConsommateur, NULL);
 
  detruireSemaphore (plein);
  detruireSemaphore (vide);
  detruireSemaphore (mutex);
               
  superFree ();
 
  return 0;
 }
