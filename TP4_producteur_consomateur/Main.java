package TP4_producteur_consomateur;

public class Main {

    public static void main(String[] args) throws InterruptedException {
        String message = "Coucou";

        Moniteur m = new Moniteur(5);
        Reader r = new Reader(m);
        Producteur p = new Producteur(m, message);
        r.start();
        p.start();
        r.join();
    }
}