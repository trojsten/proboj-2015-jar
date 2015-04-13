#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <string>
using namespace std;

#include "common.h"
#include "marshal.h"
#include "update.h"


Mapa mapa;
Stav stav;   // vzdy som hrac cislo 0
Prikaz prikaz;


void inicializuj(){
    
}

// main() zavola tuto funkciu, ked chce vediet, aky prikaz chceme vykonat,
// co tato funkcia rozhodne pomocou toho, ako nastavi prikaz;
void zistiTah(){
    prikaz.smer = (rand()%100<5)? (prikaz.smer+1)%4 : prikaz.smer;
    prikaz.smer = (rand()%100<5)? (prikaz.smer+3)%4 : prikaz.smer;
    string s = prikaz.toString();
    
    fprintf(stderr, "som prikaza: %d\n", s.size()); 
    for(int i=0; i < s.size(); i++){
        fprintf(stderr, "%c", s[i]); 
    }
    fprintf(stderr, "\n"); 
    
}

int main() {
  // v tejto funkcii su vseobecne veci, nemusite ju menit (ale mozte).

  unsigned int seed = time(NULL) * getpid();
  srand(seed);

  nacitaj(cin, mapa);
  fprintf(stderr, "START pid=%d, seed=%u\n", getpid(), seed);
  inicializuj();

  while (cin.good()) {
    nacitaj(cin, stav);
    zistiTah();
    uloz(cout, prikaz);
    cout << ".\n" << flush;   // bodka a flush = koniec odpovede
  }

  return 0;
}

