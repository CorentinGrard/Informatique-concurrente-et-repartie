#include <calculateur.hh>

#include <iostream>
using namespace std;

static CORBA::Object_ptr getObjectReference(CORBA::ORB_ptr orb, string contexte, string objet);

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
    if (argc != 3)
     {
      cerr << "usage : version3_clt nomContexte nomObjet" << endl;
      return 1;
     }

    /***************************/
    /* Initialisation de l'ORB */
    /***************************/

    CORBA::ORB_var orb = CORBA::ORB_init (argc, argv);


    /*****************************/

    // CORBA::Object_var obj = orb->string_to_object (argv[1]);

    CORBA::Object_var obj = getObjectReference (orb, argv[1], argv[2]);

    /*****************************************************/
    /* Récupération d'une référence sur l'objet serveur  */
    /*****************************************************/

    Additionneur_var additionneur_implementation_reference = Additionneur::_narrow(obj);

    if (CORBA::is_nil (additionneur_implementation_reference))
     {
      cerr << "La reference sur l'objet Additionneur n'existe pas" << endl;

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

static CORBA::Object_ptr getObjectReference(CORBA::ORB_ptr orb, string contexte, string objet)
 {
  CosNaming::NamingContext_var rootContext;

  try 
   {
    CORBA::Object_var obj = orb->resolve_initial_references("NameService");

    rootContext = CosNaming::NamingContext::_narrow(obj);

    if (CORBA::is_nil(rootContext)) 
     {
      cerr << "Failed to narrow the root naming context." << endl;

      return CORBA::Object::_nil();
     }
   }
  catch (CORBA::NO_RESOURCES&) 
   {
    cerr << "Caught NO_RESOURCES exception. You must configure omniORB "
         << "with the location" << endl
         << "of the naming service." << endl;

    return CORBA::Object::_nil();
   }
  catch (CORBA::ORB::InvalidName& ex) 
   {
    // This should not happen!

    cerr << "Service required is invalid [does not exist]." << endl;

    return CORBA::Object::_nil();
   }

  cout << "On recupere une reference sur le service de nommage" << endl;

  // Create a name object, containing the name test/context:

  CosNaming::Name name;
                  name.length(2);

                  name[0].id = (const char*) contexte.c_str();
                  name[0].kind = (const char*) "Contexte"; 

                  name[1].id = (const char*) objet.c_str();
                  name[1].kind = (const char*) "Objet";

  try 
   {
    return rootContext->resolve(name);
   }
  catch (CosNaming::NamingContext::NotFound& ex) 
   {
    cerr << "L'objet " << contexte << "/" << objet << endl;
   }
  catch (CORBA::TRANSIENT& ex) 
   {
    cerr << "Caught system exception TRANSIENT -- unable to contact the "
         << "naming service." << endl
         << "Make sure the naming server is running and that omniORB is "
         << "configured correctly." << endl;
   }
  catch (CORBA::SystemException& ex) 
   {
    cerr << "Caught a CORBA::" << ex._name()
         << " while using the naming service." << endl;
   }

  return CORBA::Object::_nil();
 }


// g++ -c calculateurSK.cc -o calculateurSK.o -I. -I/home/manu/CORBA/omniORB/include
// g++ version3_clt.cc calculateurSK.o -I. -I/home/emmmanuel/CORBA/omniORB/include -L/home/emmanuel/CORBA/omniORB/lib -lomniORB4 -lomnithread -lomniDynamic4 -o version3_clt
// Ne pas oublier d'activer le service de nommage


