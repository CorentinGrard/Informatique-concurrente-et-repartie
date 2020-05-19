package TP4_producteur_consomateur;

public class Producteur extends Thread {

  private Moniteur m;
  private String message;

  public Producteur(Moniteur m, String message) {
    this.m = m;
    this.message = message;
  }

  public void run() {
    char[] chars = message.toCharArray();
    for (char ch : chars) {
      m.placer(new Character(ch));
    }
  }
}