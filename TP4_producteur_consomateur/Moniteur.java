package TP4_producteur_consomateur;

public class Moniteur {
  private char buffer[];
  private int N;
  private int count, in, out;

  public Moniteur(int argSize) {
    N = argSize;
    buffer = new char[N];
    count = 0;
    in = 0;
    out = 0;
  }

  public synchronized void append(char data) {
    buffer[in] = data;
    in = (in + 1) % N;
    ++count;
  }

  public synchronized char take(){
    char data;
    data = buffer[out];
    out = (out +1)%N;
    --count;
    return data;
  }
}