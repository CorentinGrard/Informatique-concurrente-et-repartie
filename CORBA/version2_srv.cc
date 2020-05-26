/******************************************************/
/* Version 2 : exemple plus réaliste où le serveur et */
/* le client se trouve dans des processus distincts.  */
/* Le serveur génère un IOR a passer en paramètre du  */
/* programme client                                   */
/******************************************************/

#include <calculateur.hh>

#include <iostream>
using namespace std;

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
    /* permettra de récupérer la POA vers l'additionneur */
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

// g++ -c calculateurSK.cc -o calculateurSK.o -I. -I/home/manu/CORBA/omniORB/include
// g++ version2_srv.cc calculateurSK.o -I. -I/home/manu/CORBA/omniORB/include -L/home/manu/CORBA/omniORB/lib -lomniORB4 -lomnithread -lomniDynamic4 -o version2_srv
