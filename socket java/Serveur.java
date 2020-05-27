import java.io.IOException;
import java.net.InetAddress;
import java.net.DatagramSocket;
import java.net.DatagramPacket;

public class Serveur 
 {
  public static void main (String[] args) 
   {
    try 
     {
      DatagramSocket socket = new DatagramSocket (2000);  

      while (true)
       {
        System.out.println ("Serveur en attente sur le port 2000");

        byte[]         tampon    = new byte [512];
        DatagramPacket reception = new DatagramPacket (tampon, tampon.length);
       
        socket.receive (reception);
     
        String         requete   = new String (tampon);
        InetAddress    adresseIP = reception.getAddress();            
        int            port      = reception.getPort(); 
     
        System.out.println ("adresseIP : " + adresseIP);
        System.out.println ("Port      : " + port);
        System.out.println ("Requete   : " + requete);
        
        String         reponse   = "Serveur Maison !";
        DatagramPacket envoi     = new DatagramPacket ( reponse.getBytes(), reponse.length(), adresseIP, port);
        socket.send (envoi);
       }
     } 
    catch (IOException e) 
     {
      e.printStackTrace();
     }
   }
 }
