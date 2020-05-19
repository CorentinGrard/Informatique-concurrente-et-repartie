package TP4_producteur_consomateur;

import java.util.concurrent.ArrayBlockingQueue;

public class Moniteur {
  private ArrayBlockingQueue<Character> tableau;
  private int taille;
  private int index;

  public Moniteur (int size) 
   {
    this.taille  = size; 
    this.index   = 0;
    this.tableau = new ArrayBlockingQueue<>(size);
   }

  public boolean estVide() {
    return (index == 0);
  }

  public boolean estPlein() {
    return (index == taille);
  }

  public synchronized char retirer() {
    try {
      while (estVide()) {
        System.out.println("Consommateur endormi");
        wait();
      }
    } catch (InterruptedException e) {
      e.printStackTrace();
    }
    --index;
    Character val = tableau.poll();
    notifyAll();
    return val;
  }

  public synchronized void placer(Character value) {
    try {
      while (estPlein()) {
        System.out.println("Producteur endormi");
        wait();
      }
    } catch (InterruptedException e) {
      e.printStackTrace();
    }
    ++index;
    tableau.offer(value);
    notifyAll();
  }
}