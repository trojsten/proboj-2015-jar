
#include <ostream>
#include <vector>
#include <queue>
#include <set>
#include <algorithm>
#include <cmath>
using namespace std;

#include "common.h"
#include "update.h"

// clockwise zhora
const int DX[] = { 0, 1, 0, -1 };
const int DY[] = { -1, 0, 1, 0 };


const int kSkoreSurvivor = 20;
const int kSkoreFrag = 10;
const int kCasNaBody = 50;
const int kPenaltaSamovrazda = 3;
const int kPenaltaSmrtTeren = 3;
const int kPenaltaSmrtTvrdohlavost = 2;


const int kZaciatocnaVelkost = 15;
const int kCasNaSkratenie = 10;
const int kOKolkoSkrati = 1;
const int kVelkostZasobnika = 3;

const int kSoftMaximalnaDlzkaHry = 20000;
const int kMaximalnaDlzkaHry = 25000;
vector<Bod> Spawny;
//TODO vymisliet bonusy


static ostream* g_observation;
void zapniObservation(ostream* observation) { g_observation = observation; }

#define OBSERVE(s,...) do {                                                    \
    if (!g_observation) break;                                                 \
    *g_observation << (s);                                                     \
    int __m[] = { __VA_ARGS__ };                                               \
    for (unsigned __i = 0; __i < sizeof(__m)/sizeof(*__m); __i++)              \
      *g_observation << " " << __m[__i];                                       \
    *g_observation << endl;                                                    \
  } while(0)


static void spravHada(Stav& stav, Bod p, int hrac) {
  Snake novy = Snake();   // tento zapis vsetko inicializuje na nuly
  novy.ktorehoHraca = hrac;
  novy.telo.push_back(p);
  novy.smer = HORE;
  novy.dorastie = kZaciatocnaVelkost-1;
  novy.zije=1;
  stav.hadi.push_back(novy);
}

//pogeneruje pociatoocny stav. spawne hadov, daco predrata a tak
Stav zaciatokHry(const Mapa& mapa) {
  Stav stav = Stav();   // tento zapis vsetko inicializuje na nuly
  
  vector<Bod> starty;
  for (int y = 0; y < mapa.h; y++) for (int x = 0; x < mapa.w; x++) {
    if (mapa.pribliznyTeren.get(x, y) == MAPA_START) {
      starty.push_back(Bod(x, y));
    }
  }
  random_shuffle(starty.begin(), starty.end());
    
  stav.teren.vyprazdni(mapa.w, mapa.h, MAPA_VOLNO);
  for (int y = 0; y < mapa.h; y++) for (int x = 0; x < mapa.w; x++) {
    switch (mapa.pribliznyTeren.get(x, y)) {
      case MAPA_VOLNO:
        stav.teren.set(x, y, MAPA_VOLNO);        
        break;
      case MAPA_START:
        stav.teren.set(x, y, MAPA_VOLNO);
        break;
      case MAPA_SPAWN:
        stav.teren.set(x, y, MAPA_SPAWN);
        break;
      case MAPA_SUTER:
        stav.teren.set(x, y, MAPA_SUTER);
        break;
    }
  }

  for (int i = 0; i < mapa.pocetHracov && i < (int)starty.size(); i++) {
    spravHada(stav, starty[i], i);
  }
  
  stav.hraci.resize(mapa.pocetHracov);
  for (int i = 0; i < mapa.pocetHracov; i++) {
    Hrac& h = stav.hraci[i];

    // zoberieme nahodnu permutaciu, kde mapovanie[i] == 0
    h.mapovanie.resize(mapa.pocetHracov);
    for (int j = 0; j < mapa.pocetHracov; j++) h.mapovanie[j] = j;
    random_shuffle(h.mapovanie.begin() + 1, h.mapovanie.end());
    swap(h.mapovanie[0], h.mapovanie[i]);
  }
  
  for (int y = 0; y < mapa.h; y++) for (int x = 0; x < mapa.w; x++) {
    if (mapa.pribliznyTeren.get(x, y)== MAPA_SPAWN) {
      Spawny.push_back(Bod(x,y));
  }}
  
  return stav;
}

void urobJedlo(const Mapa& mapa, Stav& stav, map<Bod,int>& mapJedla, const vector<Bod>&mapSpawns, const map<Bod,int> zabrateHadmi){
    //rob rozne typy s roznymi pravdepodobnostami
    Jedlo jed;
    jed.pozicia = mapSpawns[rand()% mapSpawns.size()];
    int ake = rand()%100;
    if(ake < 10){
        jed.typ = JEDLO_REVERS;
        jed.prirastok = 0;
        jed.zivotnost = 10+ (rand()%80);
    
    }
    else
    if(ake < 20){
        jed.typ = JEDLO_BOMBA;
        jed.prirastok = -5;
        jed.zivotnost = 10+ (rand()%80);
    
    }
    else
    if(ake < 30){
        jed.typ = JEDLO_PREZEN;
        jed.prirastok = 0;
        jed.zivotnost = 10+ (rand()%80);
    
    }
    else
    if(ake < 40){
        jed.typ = JEDLO_OTRAVA;
        jed.prirastok = 0;
        jed.zivotnost = 10+ (rand()%80);
    
    }
    else
    if(ake < 50){
        jed.typ = JEDLO_CHOLERA;
        jed.prirastok = 0-((rand()%10) +2);
        jed.zivotnost = 10+ (rand()%80);
    
    }
    else
    if(ake < 60){
        jed.typ = JEDLO_MIXER;
        jed.prirastok = 0;
        jed.zivotnost = 10+ (rand()%80);
    
    }
    else{
        jed.typ = JEDLO_TRAPNE;
        jed.prirastok = (rand()%10) +2;
        jed.zivotnost = 10+ (rand()%80);
    }
    
    if(mapJedla.count(jed.pozicia)==0 && priechodne(stav.teren.get(jed.pozicia)) && zabrateHadmi.count(jed.pozicia)==0){
      stav.jedlo.push_back(jed);
      mapJedla[jed.pozicia] = -1;
    }
}

void odsimulujKolo(const Mapa& mapa, Stav& stav, const vector<Odpoved>& akcie) {
  OBSERVE("odsimulujKolo.zacina", stav.cas, stav.cas + 1);
  fprintf(stderr, "zaciatok\n");
  map<Bod,int> plnePolicka;
  //zisti, kde su hadi
  FOREACH (it, stav.hadi) {
    FOREACH (itt, it->telo){
        plnePolicka[*itt] = it->ktorehoHraca;        
    }
  }
  
  fprintf(stderr, "kuknemm pouzitie bonusov\n");

  //pories papanie bonusov
  for (int hrac = 0; hrac < mapa.pocetHracov; hrac++){
      Snake *had = &stav.hadi[hrac];
      if( had->zije ==0 ) continue; //hrac je dead, lebo jeho had je dead
      Prikaz pom = akcie[hrac];
      Prikaz *p = &pom;
      
      if((0<= p->pouzi && p->pouzi < had->zasoba.size()) || had->zasoba.size()> kVelkostZasobnika){
          int ktory = (0<= p->pouzi && p->pouzi < had->zasoba.size())?p->pouzi:0 ;
          Jedlo J = had-> zasoba[ktory];
          had->zasoba.erase(had->zasoba.begin()+ktory);
          //vyhodnoti sa, co had zjedol
          had->dorastie+=J.prirastok; //vzdy dorastie, alebo sa skrati
          int dokopy=0;
          Bod n_head = Bod(had->telo[0].x+DX[had->smer],had->telo[0].y+DY[had->smer]);
          switch (J.typ){
            case JEDLO_TRAPNE: //tak uz nic
              break;
            case JEDLO_OTRAVA:
              break;
            case JEDLO_REVERS: //
              reverse(had->telo.begin(),had->telo.end());
              break;
            case JEDLO_MIXER:
              FOREACH(it,had->zasoba) dokopy+= 2*it->prirastok;
              had->zasoba.resize(0);
              had->dorastie+=dokopy;
              break;
            case JEDLO_BOMBA:
              if (stav.teren.get(n_head)==MAPA_SUTER){
                  stav.teren.set(n_head,MAPA_VOLNO);
              }
              n_head = Bod(n_head.x+DX[had->smer],n_head.y+DY[had->smer]);
              if (stav.teren.get(n_head)==MAPA_SUTER){
                  stav.teren.set(n_head,MAPA_VOLNO);
              }              
              break;
            case JEDLO_PREZEN:
              had->zasoba.resize(0);
              break;
            case JEDLO_CHOLERA:
              FOREACH(it, stav.hadi){
                if(it->zije){
                  if(it->ktorehoHraca==hrac) continue;
                  Jedlo muhaha;
                  muhaha.typ= JEDLO_OTRAVA;
                  muhaha.prirastok = -((rand()%10) +2);
                  muhaha.zivotnost = 10+ (rand()%80);
                  it->zasoba.push_back(muhaha);
                }
              }
              break;
          }
      }
  }
  
  fprintf(stderr, "pohyb predzistenie\n");

  //Zisti, kam chcu ist a ci tam nechcu ist dvaja naraz
  map<Bod,vector<int> > semChceIst;
  for (int hrac = 0; hrac < mapa.pocetHracov; hrac++) {
      Snake *had = &stav.hadi[hrac];
      if( had->zije ==0 ) continue; //hrac je dead, lebo jeho had je dead
      Prikaz pom = akcie[hrac];
      Prikaz *p = &pom;
      if(p->smer<0 || p->smer > 3) p->smer = had->smer;
      Bod nova_hlava = Bod(had->telo[0].x + DX[p->smer],had->telo[0].y + DY[p->smer]);
      semChceIst[nova_hlava].push_back(had->ktorehoHraca);
  }
  
  fprintf(stderr, "fakt pohyb\n");

  //pohneme. trochu nefunguje, ked had ide na policku ktore uvolnil iny had. to nefunguje. nevadi. to bol zamer!
  for (int hrac = 0; hrac < mapa.pocetHracov; hrac++){
      Snake *had = &stav.hadi[hrac];
      if( had->zije ==0 ) continue; //hrac je dead, lebo jeho had je dead
      Prikaz pom = akcie[hrac];
      Prikaz *p = &pom;
      if(p->smer<0 || p->smer > 3) p->smer = had->smer;
      
      Bod nova_hlava = Bod(had->telo[0].x + DX[p->smer],had->telo[0].y + DY[p->smer]);
      bool vieist = priechodne(stav.teren.get(nova_hlava)) && !plnePolicka.count(nova_hlava);
      vieist= vieist&& semChceIst[nova_hlava].size()==1;
      if(vieist){
          if(had->dorastie>0){
              had->dorastie--;
              had->telo.push_back(Bod(-1,-1));
          }
          if(had->dorastie<0){
            if(-had->dorastie>= had->telo.size()){
              had->zije=0;
            }
            else{
              had->telo.resize(had->telo.size()+had->dorastie);              
            }
            had->dorastie=0;
          }
          for(int cast = had->telo.size()-1; cast>0;cast--){
              had->telo[cast] = had->telo[cast-1];
          }
          had->telo[0]= nova_hlava;
          plnePolicka[nova_hlava]= hrac;
      }
      else{
          //tak zomrel
          had->zije = 0;
          if(plnePolicka.count(nova_hlava)){
              int dokoho = plnePolicka[nova_hlava];
              if(dokoho == had->ktorehoHraca){
                  //samovrazda. to sa nerobii
                  stav.hraci[dokoho].skore-= kPenaltaSamovrazda;
              }
              else{
                  stav.hraci[dokoho].skore += kSkoreFrag;
              }
          }
          if(!priechodne(stav.teren.get(nova_hlava))){
              //nevie sa vyhnut terenu
              stav.hraci[had->ktorehoHraca].skore-= kPenaltaSmrtTeren;
          }
          if(semChceIst[nova_hlava].size()>1 && semChceIst[nova_hlava][0]==had->ktorehoHraca){
            for(int i=0;i<semChceIst[nova_hlava].size();i++){
                stav.hraci[semChceIst[nova_hlava][i]].skore-=kPenaltaSmrtTvrdohlavost;
            }
          }
      }
      
  }
  
  
  //updatni posledny smer
  for (int hrac = 0; hrac < mapa.pocetHracov; hrac++) {
      Snake *had = &stav.hadi[hrac];
      if( had->zije ==0 ) continue; //hrac je dead, lebo jeho had je dead
      if(!(akcie[hrac].smer<0 || akcie[hrac].smer > 3)) had->smer = akcie[hrac].smer;
  }


  fprintf(stderr, "co sa spapalo\n");
  //zisti, co sa spapalo
  map<Bod, int > jedla;
  vector<Jedlo> pomJedla;
  for ( int i=0; i<stav.jedlo.size(); i++){
    jedla[stav.jedlo[i].pozicia] = i;
    pomJedla.push_back(stav.jedlo[i]);
        
  }
  FOREACH(it, stav.hadi){
    if(it->zije){
      if(jedla.count(it->telo[0])){
        int co = jedla[it->telo[0]];
        jedla.erase(it->telo[0]);
        it->zasoba.push_back(stav.jedlo[co]);
        
      }
    }
  }
  
  stav.jedlo.clear();
  FOREACH(it, jedla){
      Jedlo pomJed = pomJedla[it->second];
      pomJed.zivotnost--;
      if (pomJed.zivotnost>0)
        stav.jedlo.push_back(pomJed);
  }

  fprintf(stderr, "urob nove jedlo\n");

  //napchaj nove jedlo a pohni starym
  
  if(((int)stav.cas) < kSoftMaximalnaDlzkaHry){
    for(int i =0; i<Spawny.size(); i++){
      if(rand()%100 <1){
            urobJedlo(mapa,stav,jedla,Spawny,plnePolicka);
      }
    }
  }
  // poskracuj zijucich, ak je na to cas
  if( ((int)stav.cas-1)% kCasNaSkratenie == 0){
    FOREACH(had, stav.hadi){
      if(had->zije) had->dorastie-= kOKolkoSkrati;
    }
  }
  
  //prida skore prezivsim
  if( ((int)stav.cas+5)% kCasNaBody == 0){
    FOREACH(had, stav.hadi){
      if(had->zije) stav.hraci[had->ktorehoHraca].skore+=1;
    }
  }
  
  
  //ci uz nahodov nebude koniec a netreba pochvalit vytazov
  if(hraSkoncila(mapa,stav)){
    FOREACH(had, stav.hadi){
      if(had->zije) stav.hraci[had->ktorehoHraca].skore+=kSkoreSurvivor;
    }
  }
  
  OBSERVE("odsimulujKolo.konci", stav.cas, stav.cas + 1);
  stav.cas++;
}


bool compid(const Snake& a, const Snake& b){
  return a.ktorehoHraca< b.ktorehoHraca;
}

void zamaskujStav(const Mapa& mapa, const Stav& stav, int hrac, Stav& novy) {
  const vector<int>& mapovanie = stav.hraci[hrac].mapovanie;
  novy.hraci.resize(mapa.pocetHracov);
  for (int i = 0; i < mapa.pocetHracov; i++) {
    novy.hraci[mapovanie[i]].skore = stav.hraci[i].skore;
  }

  FOREACH(it, stav.hadi) {
    novy.hadi.push_back(*it);
    novy.hadi.rbegin()->ktorehoHraca = mapovanie[it->ktorehoHraca];
  }
  sort(novy.hadi.begin(), novy.hadi.end(),compid);
  novy.jedlo = stav.jedlo;
  novy.teren = stav.teren;  
  
  novy.dalsiId = stav.dalsiId;
  novy.cas = stav.cas;
}


void odmaskujOdpoved(const Mapa& mapa, const Stav& stav, int hrac, Odpoved& odpoved) {
  // this function intentionally left blank //WARNING SECURITY TODO WTF???
}




vector<int> ktoriZiju(const Mapa& mapa, const Stav& stav) {
  set<int> zijuci;
  FOREACH(it, stav.hadi) {
      if(it->zije)
        zijuci.insert(it->ktorehoHraca);
  }
  return vector<int>(zijuci.begin(), zijuci.end());
}


bool hraSkoncila(const Mapa& mapa, const Stav& stav) {
  
  return ktoriZiju(mapa,stav).size() <= 1 || stav.cas >= kMaximalnaDlzkaHry;
}


int zistiSkore(const Stav& stav, int hrac) {
  return stav.hraci[hrac].skore;
}
