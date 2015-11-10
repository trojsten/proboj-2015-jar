#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <string>
#include <set>
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
    Snake* moj = &stav.hadi[0];
    if( moj->zasoba.size()>0) prikaz.pouzi =0;
    else prikaz.pouzi = NIC;
      
    set<Bod> plneHad;
    FOREACH(it,stav.hadi){
        FOREACH(itt,it->telo){
          plneHad.insert(*itt);
        }
    }
    int vyhovovalo = 0;
    prikaz.smer =0;
    Bod hlava = moj->telo[0];
    Bod nezmenena = Bod(hlava.x+DX[moj->smer], hlava.y+DY[moj->smer]);
    
    if((!(plneHad.count(nezmenena)==0 && priechodne(stav.teren.get(nezmenena)))) || rand()%100<5){
      for(int i=0; i< 4; i++){
        Bod nova_hlava = Bod(hlava.x+DX[i], hlava.y+DY[i]);
        
        if(plneHad.count(nova_hlava)==0 && priechodne(stav.teren.get(nova_hlava))){
          vyhovovalo++;
          if(rand()%vyhovovalo ==0){
            prikaz.smer = i;
          }
        }
      }
    }
    else{
        prikaz.smer = moj->smer;
    }
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

