public class Client
 {
  public static void main(String args[]) 
   {
    try
     {
      Calcul calDistant = (Calcul) java.rmi.Naming.lookup("rmi://localhost/servCal"); 
      System.out.println ("Le client est connecte au serveur de calcul");

      System.out.println ("Calcul de  10+20=" + calDistant.additionner( 10,20));
      System.out.println ("Calcul de  73+ 2=" + calDistant.additionner( 73, 2));
      System.out.println ("Calcul de 104+58=" + calDistant.additionner(104,58));
     }
    catch (java.rmi.NotBoundException e)
     {
      System.err.println ("servCal n'est pas enregistre");
     }
    catch (java.net.MalformedURLException e)
     {
      System.err.println ("L'URL est mal ecrit");
     }
    catch (java.rmi.RemoteException e)
     {
      System.err.println (e);
     } 
   }
 }
