import java.io.IOException;
import java.io.InputStreamReader;
import java.io.BufferedReader;
import java.io.PrintStream;
import java.net.InetAddress;
import java.net.Socket;
import java.net.UnknownHostException;

public class Client 
 {
  public static void main(String[] args) 
   {
    try 
     {
      Socket         clt    = new Socket (InetAddress.getByName("localhost"), 2000);
      BufferedReader entree = new BufferedReader (new InputStreamReader (clt.getInputStream()));
      PrintStream    sortie = new PrintStream (clt.getOutputStream ());
  
      sortie.println ("HEAD / HTTP/1.0\n\n");
  
      System.out.println ("Reponse : ");
      
      String ligne = null;
      
      while ( (ligne = entree.readLine()) != null) System.out.println (ligne);
      
      clt.close();
     } 
    catch (UnknownHostException e) 
     {
      e.printStackTrace();
     }
    catch (IOException e) 
     {
      e.printStackTrace();
     }
   }
 }
