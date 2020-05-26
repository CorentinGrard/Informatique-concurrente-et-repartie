#include <calculateur.hh>

#include <iostream>
using namespace std;

/***************************************/
/* Implémentation de la partie cliente */
/***************************************/

static void additionner (Additionneur_ptr a)
 {
  if( CORBA::is_nil (a) )
   {
    cerr << "additionner: La reference sur l'objet est nulle" << endl;
    return;
   }

  cout << "Partie cliente : Envoi de 4 et 7" << endl;

  resultat res = a->additionner (4, 7);

  cout << "Partie cliente : Reception de " << res.valeur << " et " << res.codeErreur << endl;
 }

int main(int argc, char** argv)
 {
  try
   {
    if (argc != 2)
     {
      cerr << "usage : version2_clt <la reference affichee par version2_srv>" << endl;
      return 1;
     }

    /***************************/
    /* Initialisation de l'ORB */
    /***************************/

    CORBA::ORB_var orb = CORBA::ORB_init (argc, argv);


    /*****************************/

    CORBA::Object_var       obj = orb->string_to_object (argv[1]);


    /*****************************************************/
    /* Récupération d'une référence sur l'objet serveur  */
    /*****************************************************/

    Additionneur_var additionneur_implementation_reference = Additionneur::_narrow(obj);

    if (CORBA::is_nil (additionneur_implementation_reference))
     {
      cerr << "Can't narrow reference to type Echo (or it was nil)." << endl;
      return 1;
     }

    /******************************************************/
    /* Appel de la fonction cliente avec la référence sur */
    /* l'objet serveur en paramètre                       */
    /******************************************************/

    additionner (additionneur_implementation_reference);

    orb->destroy();
   }
  catch (CORBA::TRANSIENT&) 
   {
    cerr << "Caught system exception TRANSIENT -- unable to contact the "
         << "server." << endl;
   }
  catch (CORBA::SystemException& ex) 
   {
    cerr << "Caught a CORBA::" << ex._name() << endl;
   }
  catch (CORBA::Exception& ex) 
   {
    cerr << "Caught CORBA::Exception: " << ex._name() << endl;
   }
 
  return 0;
 }
