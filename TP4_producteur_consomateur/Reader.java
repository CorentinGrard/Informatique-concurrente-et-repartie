package TP4_producteur_consomateur;

public class Reader extends Thread {

  private Moniteur m;

  public Reader(Moniteur m) {
    this.m = m;
  }

  public void run() {
    char c;
    do {
      c = m.retirer();
      System.out.print(c);
    } while (c != '\0');
    System.out.println("Fini");
  }
}
