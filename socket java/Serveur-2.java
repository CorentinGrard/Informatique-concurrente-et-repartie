 
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.BufferedReader;
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

        Socket         clt    = srv.accept();
        BufferedReader entree = new BufferedReader (new InputStreamReader (clt.getInputStream()));
        PrintStream    sortie = new PrintStream (clt.getOutputStream ());
  
        System.out.println ("Requete : " + entree.readLine());
        sortie.println ("Mon petit serveur maison");
        clt.close();
       }
     } 
    catch (IOException e) 
     {
      e.printStackTrace();
     }
   }
 }
