public class CalculImpl extends java.rmi.server.UnicastRemoteObject implements Calcul
 {
  public CalculImpl () throws java.rmi.RemoteException
   {
    System.out.println ("Le serveur de calcul est instancie");
   }  

  public int additionner (int a,
                          int b) throws java.rmi.RemoteException
   {
    System.out.println ("J'ai calcule " + a + "+" + b);
    return (a+b);
   }

 public static void main(String args[]) 
   {
    try
     {
      java.rmi.registry.LocateRegistry.getRegistry ();
      Calcul cal = new CalculImpl ();
      java.rmi.Naming.bind ("rmi://localhost:1099/servCal", cal); 
     }
    catch (java.io.IOException e)
     {
      System.err.println (e);
     }
    catch (java.rmi.AlreadyBoundException e)
     {
      System.err.println (e);
     }
   }
 }
