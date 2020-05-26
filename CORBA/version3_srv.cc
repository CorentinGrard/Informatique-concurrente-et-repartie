/********************************************************/
/* Version 3 : exemple encore plus réaliste où on l'IOR */
/* est remplace par le serveur de noms                  */
/********************************************************/

#include <calculateur.hh>

#include <iostream>
using namespace std;

CORBA::Boolean bindObjectToName (CORBA::ORB_ptr orb, CORBA::Object_ptr objref, string contexte, string objet);

/***************************************/
/* Implémentation de la partie serveur */
/***************************************/

class Additionneur_implementation : public POA_Additionneur
 {
  public:

  inline Additionneur_implementation() {}
  virtual ~Additionneur_implementation() {}
  virtual resultat additionner (::CORBA::Short op1, ::CORBA::Short op2);
 };


resultat Additionneur_implementation::additionner (::CORBA::Short op1, ::CORBA::Short op2)
 {
  resultat res;

  res.valeur     = op1 + op2;
  res.codeErreur = 0;

  cout << "Partie serveur : " << op1 << "+" << op2 << "=" << res.valeur 
       << " avec codeErreur=" << res.codeErreur << endl;

  return res;
 }

int main(int argc, char** argv)
 {
  try
   {
    /***************************/
    /* Initialisation de l'ORB */
    /***************************/

    CORBA::ORB_var orb = CORBA::ORB_init (argc, argv);

    /*****************************************************/
    /* Obtention d'une référence vers le POA racine qui  */
    /* permettra de récupérer le POA vers l'additionneur */
    /*****************************************************/

    CORBA::Object_var       obj = orb->resolve_initial_references("RootPOA");
    PortableServer::POA_var poa = PortableServer::POA::_narrow(obj);

    /*****************************************************/
    /* Instanciation de l'objet serveur (incrémentation  */
    /* d'un compteur au niveau de l'ORB)                 */
    /*****************************************************/

    PortableServer::Servant_var<Additionneur_implementation> additionneur_implementation = new Additionneur_implementation();

    /*****************************************************/
    /* Activation du serveur pour le rendre apte à       */
    /* récupérer des requêtes issues de l'ORB            */
    /*****************************************************/

    PortableServer::ObjectId_var additionneur_implementation_id = poa->activate_object (additionneur_implementation);

    /*****************************************************/
    /* Obtenir une référence sur le gestionnaire de POA  */
    /* afin de le mettre en mode écoute (des requêtes)   */
    /*****************************************************/

    PortableServer::POAManager_var gestionnaireDePOA = poa->the_POAManager();
    gestionnaireDePOA->activate();

    /*****************************************************/
    /* Récupération d'une référence sur l'objet serveur  */
    /*****************************************************/

    Additionneur_var additionneur_implementation_reference = additionneur_implementation->_this();

    /*****************************************************/
    /* Affichage de l'IOR du serveyr                     */
    /*****************************************************/

    CORBA::String_var sior (orb->object_to_string (additionneur_implementation_reference));
    cout << endl << sior << endl;

    /*******************************************************/
    /* Enregistement du serveur dans le système de nommage */
    /*******************************************************/

    bindObjectToName (orb, additionneur_implementation_reference, "Calcul", "Additionneur");

    /********************************************************/
    /* Lancement de l'ORB et blocage en attente de requêtes */
    /********************************************************/

    orb->run();
   }
  catch (CORBA::SystemException& ex) 
   {
    cerr << "Caught CORBA::" << ex._name() << endl;
   }
  catch (CORBA::Exception& ex) 
   {
    cerr << "Caught CORBA::Exception: " << ex._name() << endl;
   }

  
  return 0;
 }

CORBA::Boolean bindObjectToName (CORBA::ORB_ptr orb, CORBA::Object_ptr objref, string contexte, string objet)
 {
  /********************************************************/
  /* Obtention d'une référence vers le service de nommage */
  /* qui permettra de récupérer le contexte "Racine       */
  /********************************************************/

  CosNaming::NamingContext_var rootContext;
 
  try 
   {
    CORBA::Object_var obj = orb->resolve_initial_references("NameService");

    rootContext = CosNaming::NamingContext::_narrow(obj);

    if (CORBA::is_nil(rootContext)) 
     {
      cerr << "Failed to narrow the root naming context." << endl;

      return 0;
     }
   }
  catch (CORBA::NO_RESOURCES&) 
   {
    cerr << "Caught NO_RESOURCES exception. You must configure omniORB "
         << "with the location" << endl
         << "of the naming service." << endl;

    return 0;
   }
  catch (CORBA::ORB::InvalidName&) 
   {
    // This should not happen!
    cerr << "Service required is invalid [does not exist]." << endl;

    return 0;
   }

  cout << "On obtient une reference sur le serveur de nommage" << endl;

  try 
   {
    // Ajout d'un contexte au contexte "Racine"

    CosNaming::Name nomContexte;
                    nomContexte.length(1);
                    nomContexte[0].id   = (const char*) contexte.c_str();
                    nomContexte[0].kind = (const char*) "Contexte";

    // Le champ "kind" permet d'indiquer le type de l'objet 
    // Cela permet d'eviter des convention utilisant des extensions
    // exemple : test.ps => postscript 

    CosNaming::NamingContext_var referenceContexte;

    /*****************************************************************/
    /* Lorsqu'on ajoute le contexte dans le systeme de nommage, on   */
    /* recupere la référence sur ce nouveau contexte pour ajouter le */
    /* nom. Si ce contexte existe déjà, on se contente de résoudre   */
    /* le nom pour obtenir la référence (dans le catch).             */
    /*****************************************************************/

    try 
     {
      referenceContexte = rootContext->bind_new_context (nomContexte);
     }
    catch(CosNaming::NamingContext::AlreadyBound& ex) 
     {
      CORBA::Object_var obj = rootContext->resolve(nomContexte);

      referenceContexte = CosNaming::NamingContext::_narrow (obj);

      if (CORBA::is_nil(referenceContexte)) 
       {
        cerr << "Failed to narrow naming context." << endl;
        return 0;
       }
     }

    cout << "On a ajoute le contexte Calcul" << endl; 

    CosNaming::Name nomObjet;
                    nomObjet.length(1);
                    nomObjet[0].id   = (const char*) objet.c_str();
                    nomObjet[0].kind = (const char*) "Objet";

    try 
     {
      referenceContexte->bind (nomObjet, objref);
     }
    catch(CosNaming::NamingContext::AlreadyBound& ex) 
     {
      referenceContexte->rebind (nomObjet, objref);
     }

    cout << "On a ajoute l'objet Additionneur" << endl; 
   }
  catch (CORBA::TRANSIENT& ex) 
   {
    cerr << "Caught system exception TRANSIENT -- unable to contact the "  
         << "naming service." << endl
         << "Make sure the naming server is running and that omniORB is "
         << "configured correctly." << endl;

    return 0;
   }
  catch (CORBA::SystemException& ex) 
   {
    cerr << "Caught a CORBA::" << ex._name()
         << " while using the naming service." << endl;

    return 0;
   }

  return 1;
 }


// g++ -c calculateurSK.cc -o calculateurSK.o -I. -I/home/manu/CORBA/omniORB/include
// g++ version3_srv.cc calculateurSK.o -I. -I/home/emmanuel/CORBA/omniORB/include -L/home/emmanuel/CORBA/omniORB/lib -lomniORB4 -lomnithread -lomniDynamic4 -o version3_srv
// Ne pas oublier d'activer le service de nommage


