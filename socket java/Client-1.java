import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.InetAddress;
import java.net.Socket;
import java.net.UnknownHostException;

public class Client 
 {
  public static void main(String[] args) 
   {
    try 
     {
      Socket             clt    = new Socket (InetAddress.getByName("localhost"), 2000);
      ObjectOutputStream sortie = new ObjectOutputStream (clt.getOutputStream ());
      ObjectInputStream  entree = new ObjectInputStream (clt.getInputStream());
  
      sortie.writeObject ( new String ("HEAD / HTTP/1.0\n\n") );

      String reponse = (String) entree.readObject();
      
      System.out.println ("Reponse : " + reponse);
     
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
    catch (ClassNotFoundException e)
     {
      e.printStackTrace();
     }
   }
 }
