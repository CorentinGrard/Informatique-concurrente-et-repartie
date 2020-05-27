import java.io.IOException;
import java.net.InetAddress;
import java.net.DatagramSocket;
import java.net.DatagramPacket;
import java.net.UnknownHostException;

public class Client 
 {
  public static void main(String[] args) 
   {
    try 
     {
      InetAddress    adresseIP = InetAddress.getByName ("localhost");
      DatagramSocket socket    = new DatagramSocket();

      String         requete   = "HEAD / HTTP/1.0\n\n";
      DatagramPacket envoi     = new DatagramPacket ( requete.getBytes(), requete.length(), adresseIP, 2000);
      socket.send (envoi);
 
      byte[]         tampon    = new byte [512];
      DatagramPacket reception = new DatagramPacket (tampon, tampon.length);
      socket.receive (reception);
      String         reponse   = new String (tampon);
     
      System.out.println ("Reponse : " + reponse);
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
 
 
 
