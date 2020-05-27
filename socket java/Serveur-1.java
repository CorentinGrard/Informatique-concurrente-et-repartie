import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.PrintStream;
import java.net.InetAddress;
import java.net.Socket;
import java.net.ServerSocket;

public class Serveur 
 {
  public static void main (String[] args) 
   {
    try 
     {
      ServerSocket srv = new ServerSocket (2000);
      
      while (true)
       {
        System.out.println ("Serveur en attente sur le port 2000");

        Socket             clt    = srv.accept();
        ObjectOutputStream sortie = new ObjectOutputStream (clt.getOutputStream ());
        ObjectInputStream  entree = new ObjectInputStream (clt.getInputStream());
  
        String requete = (String) entree.readObject();
        System.out.println ("Requete : " + requete);
        
        sortie.writeObject ( new String ("Mon petit serveur maison") );
        
        clt.close();
       }
     } 
    catch (IOException e) 
     {
      e.printStackTrace();
     }
    catch (ClassNotFoundException e)
     {
      e.printStackTrace();
     }
   }
 }
