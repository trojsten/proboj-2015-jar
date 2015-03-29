#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <iostream>
#include <vector>
#include<set>
#include<algorithm>
#include<queue>
#include<cmath>
#define INF 1000000047
using namespace std;

#include "common.h"
#include "marshal.h"
#include "update.h"


Mapa mapa;
Stav stav;   // vzdy som hrac cislo 0
Teren viditelnyTeren;
Teren objavenyTeren;
vector<Prikaz> prikazy;
Bod base=Bod(-1,-1);
//najskor sa chce pozriet hore a dole ay potom doprava a dolava
vector<Bod>Smery={Bod(0,-1),Bod(0,1),Bod(-1,0),Bod(1,0)};

map<int,Bod>Banici,Sekaci,Strazci;
set<Bod>Kovy,Kamene,Poklad,SpawnSeen;
vector<vector<int>>Mriezka;
bool init=1;


int najdiManika(Bod b){
FOREACH(it,stav.manici){
if(it->pozicia()==b)
    return it->ktorehoHraca;
}
return -1;

}

int dst(Bod a,Bod b){
return abs(a.x-b.x)+abs(a.y-b.y);
}



// main() zavola tuto funkciu, ked nacita mapu
void inicializuj() {
  // (sem patri vas kod)
  // hlupy klient si vyrobi Teren (cize 2D pole) kde si pamata co objavil
  objavenyTeren.vyprazdni(mapa.w, mapa.h, MAPA_NEVIEM);
//vyrataj mapku
Mriezka.resize(mapa.w,vector<int>(mapa.h,0));

for(int i=0; i<mapa.w; i+=10)
for(int j=0; j<mapa.h; j++){
Mriezka[i][j]=1;
}

for(int i=0; i<mapa.w; i++)
for(int j=0; j<mapa.h; j+=3){
Mriezka[i][j]=1;
}


for(int i=0; i<mapa.w; i++)
for(int j=0; j<mapa.h; j++){
if(mapa.pribliznyTeren.get(Bod(i,j))==MAPA_VOLNO)
    
    Mriezka[i][j]=2;
}





/*
for(int i=0; i<mapa.w; i++){
for(int j=0; j<mapa.h; j++)
cerr<<Mriezka[i][j]<<" ";
cerr<<endl;
}
*/


}

void CHOD(Manik m,Bod ciel){
    //cerr<<"chcem sa pohnut z "<<m.x<<" "<<m.y <<" na "<<ciel.x<<" "<<ciel.y<<endl;
if(objavenyTeren.get(ciel)!=MAPA_VOLNO){
    prikazy.push_back(Prikaz(m.id,PRIKAZ_UTOC,ciel));
    //cerr<<"utocim "<<endl;
}
else{

    prikazy.push_back(Prikaz(m.id,PRIKAZ_CHOD,ciel));
    //cerr<<"idem "<<endl;
}
}




// pomocna funkcia co ked uz vieme kam ten manik chce ist tak ho tam posle
static void chodKuMiestu(const Manik &m, Bod ciel) {
  Teren vzdialenost;
  prehladajBfs(objavenyTeren, ciel, vzdialenost);
  bool pohnuty=false;
  for (int d = 0; d < 4; d++) {
    Bod n(m.x + DX[d], m.y + DY[d]);
    if (priechodne(objavenyTeren.get(n)) &&
        vzdialenost.get(n) < vzdialenost.get(m.pozicia())) {
        bool ok=true;
        FOREACH(it,stav.manici){
        if(it->pozicia()==n && it->ktorehoHraca==0)
            ok=false;
        }
    if(ok){
    CHOD(m,n);
      //prikazy.push_back(Prikaz(m.id, PRIKAZ_CHOD, n));
      pohnuty=true;
      break;
    }
    }
  }


if(!pohnuty){
int d=rand()%4;
    Bod n=Bod(m.x+DX[d],m.y+DY[d]);
bool ok=true;
FOREACH(it,stav.manici){
if(it->pozicia()==n && it->ktorehoHraca==0){
ok=false;
}
}
if(ok){
CHOD(m,n);

}


}
}

void coRobiStrazca(const Manik &m){

Mriezka[m.x][m.y]=2;


FOREACH(it,stav.manici){
if(it->ktorehoHraca!=0 && dst(m.pozicia(),it->pozicia())==1){
    prikazy.push_back(Prikaz(m.id,PRIKAZ_UTOC,it->pozicia()));
return;
}





if(Strazci.find(m.id)!=Strazci.end()){
if(Strazci[m.id]==m.pozicia())
    Strazci.erase(Strazci.find(m.id));
else if(rand()%20==0)
    Strazci.erase(Strazci.find(m.id));
}

    vector<Bod>X;
for(int i=max(base.x-20,0); i<=min(base.x+20,mapa.w-1); i++)
    for(int j=max(0,base.y-20); j<=min(base.x+20,mapa.h-1); j++){
    if(Mriezka[i][j]==2 && dst(Bod(i,j),base)>3)
        X.push_back(Bod(i,j));


    }
cerr<<"X ma velkost "<<X.size()<<endl;

if(X.size()>0){

int r=rand()%X.size();
Strazci[m.id]=X[r];

}
}





if(Strazci.find(m.id)!=Strazci.end()){
chodKuMiestu(m,Strazci[m.id]);
}

}

    




void coRobiSekac(const Manik &m){

Mriezka[m.x][m.y]=2;


FOREACH(it,stav.manici){
if(it->ktorehoHraca!=0 && dst(m.pozicia(),it->pozicia())==1){
    prikazy.push_back(Prikaz(m.id,PRIKAZ_UTOC,it->pozicia()));
return;
}





if(Sekaci.find(m.id)!=Sekaci.end()){
if(Sekaci[m.id]==m.pozicia())
    Sekaci.erase(Sekaci.find(m.id));
else if(rand()%20==0)
    Sekaci.erase(Sekaci.find(m.id));
}

if(Sekaci.find(m.id)==Sekaci.end()){
    if(Poklad.size()>0){
auto it=Poklad.begin();
int r=rand()%Poklad.size();
for(int i=0; i<r; i++)
it++;
//cerr<<"nastavujem na "<<it->x<<" "<<it->y<<endl;
Sekaci[m.id]=Bod(it->x,it->y);
}

else{

    vector<Bod>X;
for(int i=max(m.x-40,0); i<=min(m.x+40,mapa.w-1); i++)
    for(int j=max(0,m.y-40); j<=min(m.x+40,mapa.h-1); j++){
    if(Mriezka[i][j]==2 && dst(Bod(i,j),base)>4)
        X.push_back(Bod(i,j));


    }
cerr<<"X ma velkost "<<X.size()<<endl;

if(X.size()>0){

int r=rand()%X.size();
Sekaci[m.id]=X[r];

}
}


}


if(Sekaci.find(m.id)!=Sekaci.end()){
chodKuMiestu(m,Sekaci[m.id]);
}

}

    }




void coRobiKovac(const Manik &m) {
  // hlupy klient proste furt stavia banikov kolko moze...
  int d = rand()%4;
    //spocitaj jednotky
    vector<int>P;
    P.resize(47,0);
    FOREACH(it,stav.manici){
    if(it->ktorehoHraca==0)
        P[it->typ]++;
    }
/*
Bod b=Bod(-1,-1);
int zlato,zelezo;
FOREACH(it,stav.manici){
if(it->ktorehoHraca==0 && it->typ==MANIK_KOVAC && it->id!=m.id)
    b=it->pozicia();
    zlato=it->zlato;
    zelezo=it->zelezo;

}
if(b!=Bod(-1,-1)){
if(zlato*5<m.zlato){

  prikazy.push_back(Prikaz(m.id, PRIKAZ_DAJ_ZLATO, b, (m.zlato+zlato)/2-zlato));
  return;

}


if(zelezo*5<m.zelezo){

  prikazy.push_back(Prikaz(m.id, PRIKAZ_DAJ_ZELEZO, b, (m.zelezo+zelezo)/2-zelezo));
  return;

}
}

    if(stav.cas>300 && P[MANIK_KOVAC]==1)
  prikazy.push_back(Prikaz(m.id, PRIKAZ_KUJ, m.x + DX[d], m.y + DY[d], MANIK_KOVAC));
*/
  

    vector<Bod>X;
for(int i=max(m.x-20,0); i<=min(m.x+20,mapa.w-1); i++)
    for(int j=max(0,m.y-20); j<=min(m.x+20,mapa.h-1); j++){
    if(Mriezka[i][j]==2 && dst(Bod(i,j),base)>2)
        X.push_back(Bod(i,j));


    }

cerr<<"X.size()="<<X.size()<<endl;


if(stav.cas>200){
    if(rand()%5==0)
  prikazy.push_back(Prikaz(m.id, PRIKAZ_KUJ, m.x + DX[d], m.y + DY[d], MANIK_MLATIC));
    else if(rand()%2==0 && X.size()>5)
  prikazy.push_back(Prikaz(m.id, PRIKAZ_KUJ, m.x + DX[d], m.y + DY[d], MANIK_STRAZNIK));
    else if(X.size()>10)
  prikazy.push_back(Prikaz(m.id, PRIKAZ_KUJ, m.x + DX[d], m.y + DY[d], MANIK_SEKAC));
  }
    else{ 
        if(Banici.size()<5)
  prikazy.push_back(Prikaz(m.id, PRIKAZ_KUJ, m.x + DX[d], m.y + DY[d], MANIK_BANIK));
        else if(Banici.size()<25 && m.zelezo>20 && m.zlato>20)
  prikazy.push_back(Prikaz(m.id, PRIKAZ_KUJ, m.x + DX[d], m.y + DY[d], MANIK_BANIK));
  

}
}
void coRobiBanik(const Manik &m) {

Mriezka[m.x][m.y]=2;



FOREACH(it,stav.manici){
if(it->ktorehoHraca!=0 && dst(m.pozicia(),it->pozicia())==1){
    prikazy.push_back(Prikaz(m.id,PRIKAZ_UTOC,it->pozicia()));
return;
}

}




//ak este nema robotu
Bod kovac=Bod(-1,-1);
FOREACH(it,stav.manici){
if(it->typ==MANIK_KOVAC && it->ktorehoHraca==0)
    kovac=it->pozicia();
}
if(Bod(-1,-1)!=kovac);
base=kovac;
//cerr<<"kovac je na pozicii "<<kovac.x<<" "<<kovac.y<<endl;
//cerr<<"mam "<<m.zelezo<<" "<<m.zlato<<endl;
if(dst(m.pozicia(),kovac)==1 && m.zelezo>0){
prikazy.push_back(Prikaz(m.id,PRIKAZ_DAJ_ZELEZO,kovac,m.zelezo));
return;
}
if(dst(m.pozicia(),kovac)==1 && m.zlato>0){
prikazy.push_back(Prikaz(m.id,PRIKAZ_DAJ_ZLATO,kovac,m.zlato));
return;
}


if(dst(m.pozicia(),base)==1){
if(Banici.find(m.id)!=Banici.end()){
//Mriezka[Banici[m.id].x][Banici[m.id].y]=1;
Banici.erase(Banici.find(m.id));

}


}


if((((m.zelezo+m.zlato>=7 && Banici.size()<7) || (m.zelezo+m.zlato>20 && Banici.size()<20)|| m.zelezo+m.zlato>60) && dst(m.pozicia(),kovac)>1)){
    chodKuMiestu(m,kovac);
return;
}


for(int d=0; d<4; d++){
    Bod n=Bod(m.x+DX[d],m.y+DY[d]);
if(objavenyTeren.get(n)==MAPA_ZLATO || objavenyTeren.get(n)==MAPA_ZELEZO){
    CHOD(m,n);
    return;
}
}

if(Banici.find(m.id)==Banici.end() || Banici[m.id]==m.pozicia() || dst(m.pozicia(),kovac)==1){


  Teren vzdialenost;
  prehladajBfs(objavenyTeren, m.pozicia(), vzdialenost);

    Bod b=Bod(-1,-1);
    int naj=INF;

  for(int i=0; i<mapa.w; i++)
      for(int j=0; j<mapa.h; j++){
        if(Mriezka[i][j]==1 && vzdialenost.get(i,j)<naj){
            b=Bod(i,j);
            naj=vzdialenost.get(i,j);
        }
      }

if(b!=Bod(-1,-1)){
Banici[m.id]=b;
Mriezka[b.x][b.y]=2;
}



}


if(Banici.find(m.id)!=Banici.end()){
//vyrata cestu

//cerr<<"moj ciel je "<<Banici[m.id].x<<" "<<Banici[m.id].y<<endl;
if(dst(m.pozicia(),Banici[m.id])==1)
    CHOD(m,Banici[m.id]);
else
chodKuMiestu(m,Banici[m.id]);//cerr<<"idem randomne"<<endl;
}







}


// main() zavola tuto funkciu, ked chce vediet, ake prikazy chceme vykonat,
// co tato funkcia rozhodne pomocou: prikazy.push_back(Prikaz(...));
void zistiTah() {
    //cerr<<init<<" "<<(base!=Bod(-1,-1))<<endl;
  if(base!=Bod(-1,-1) && init==true){
   cerr<<"inicializujem "<<endl; 
cerr<<base.x<<" "<<base.y<<endl;
for(int i=max(base.x-5,0); i<=min(base.x+5,mapa.w-1); i++)
    for(int j=max(base.y-5,0); j<=min(base.y+5,mapa.h-1); j++)
        if(Mriezka[i][j]==0)
            Mriezka[i][j]=1;


/*
for(int i=0; i<mapa.w; i++){
for(int j=0; j<mapa.h; j++)
cerr<<Mriezka[i][j]<<" ";
cerr<<endl;
}
*/



    init=0;
  }
    
    
    // (sem patri vas kod)

  //fprintf(stderr, "zistiTah zacina %d\n", stav.cas);

  // zapamatame si teren co vidime a doteraz sme nevideli
  for (int y = 0; y < mapa.h; y++) for (int x = 0; x < mapa.w; x++) {
    if (viditelnyTeren.get(x, y) != MAPA_NEVIEM) {
        SpawnSeen.insert(Bod(x,y));
        objavenyTeren.set(x, y, viditelnyTeren.get(x, y));
      
    }
  }
Poklad.clear();
FOREACH(it,stav.manici){
if(it->ktorehoHraca!=0)
Poklad.insert(Bod(it->pozicia()));

//if(it->ktorehoHraca==0 && it->typ==0)
  //  objavenyTeren.set(it->pozicia(),MAPA_SUTER);

}

SpawnSeen.insert(base);

if(stav.cas>600)
for(int i=0; i<mapa.w; i++)
for(int j=0; j<mapa.h; j++)
if(objavenyTeren.get(i,j)==MAPA_VOLNO && Mriezka[i][j]==1 && Bod(i,j)!=base && SpawnSeen.find(Bod(i,j))==SpawnSeen.end()){
Poklad.insert(Bod(i,j));
}

/*
cerr<<"Zlato vidim na:"<<endl;

for(auto it=Poklad.begin(); it!=Poklad.end(); it++)
    cerr<<it->x<<" "<<it->y<<endl;
cerr<<"Koniec zlata"<<endl;
*/

  // kazdemu nasmu manikovi povieme co ma robit (na to mame pomocne funkcie)
  FOREACH(it, stav.manici) {
    if (it->ktorehoHraca != 0) continue;
    switch (it->typ) {
      case MANIK_KOVAC:
        coRobiKovac(*it);
        break;
        case MANIK_BANIK:
        coRobiBanik(*it);
        break;
      case MANIK_MLATIC:
        coRobiBanik(*it);
        break;
   case MANIK_SEKAC:
        coRobiSekac(*it);
        break;
   case MANIK_STRAZNIK:
        coRobiStrazca(*it);
        break;
    }
  }

  //fprintf(stderr, "prikazov %d\n", (int)prikazy.size());
}


int main() {
  // v tejto funkcii su vseobecne veci, nemusite ju menit (ale mozte).

  unsigned int seed = time(NULL) * getpid();
  srand(seed);

  nacitaj(cin, mapa);
  fprintf(stderr, "START pid=%d, seed=%u\n", getpid(), seed);
  inicializuj();

  while (cin.good()) {
    vector<int> zakodovanyTeren;
    nacitaj(cin, zakodovanyTeren);
    dekodujViditelnyTeren(zakodovanyTeren, viditelnyTeren);
    nacitaj(cin, stav);
    prikazy.clear();
    zistiTah();
    uloz(cout, prikazy);
    cout << ".\n" << flush;   // bodka a flush = koniec odpovede
  }

  return 0;
}

