TODO : 
* Récupérer les signals CTRL + C
* Faire les estampilles bizarres
* Séparer les threads dans d'autres fichiers
* Annuaire de processus
* MultiThread le serveur
* Faire le thread calcul :
  * Si en attente de SC : 
    * Lire les reply
    * tant que t'as pas tous les reply:
      * attend
    * Fais SC (print qqc)
    * envoi des RELEASE
  * Pick action aleatoirement :
    * Action local => horloge+1
    * Envoyer message utilisateur à serveur au pif; horloge +1
    * Envoi d'un request + horloge +1 + entré en mode attente SC
* Pour le thread client :
  * Faire dans une fonction envoi taille port, port, type de request :
    * Si message alors on garde comme ici
    * Si Request :
      * envoi pid
      * envoi Horloge
    * Si Reply Rien
    * Si Release rien
* Ajouter à Thread calcul des appels de Trace
* 