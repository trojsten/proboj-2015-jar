#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <queue>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <stack>
#include <list>
#define For(i,N) for(int i=0; i<N; i++)
using namespace std;

#include "common.h"
#include "marshal.h"
#include "update.h"


Mapa mapa;
Stav stav;   // vzdy som hrac cislo 0
Teren viditelnyTeren;

vector<Prikaz> prikazy;

Teren znamyTeren;
vector<Manik> nasiManici; 
vector<Manik> ichManici;
int kovacX, kovacY;
unordered_map<int,Bod> nasiTunelari;	// id banika, ciel
vector<pair<Bod, int> > starty;		// 0 - nic, 1 - ide, 2 - skoncil
int chceme_postavit=-1;
vector<int> armada(9,0);

int tah = 0;
int oldDotazene = 0;
int nStartov = 0;
int targetStart = 0;
bool wasFight;
int dotazene = 0;
bool druhyKovac = 0;
list<Bod> lastSeen;
  
void update_znamyTeren(){
  cerr << 0;
  // vymazeme jednotky
  For(i,mapa.h) For(j,mapa.w) if(znamyTeren.get(j,i) >= 10) znamyTeren.set(j,i,MAPA_VOLNO);
  cerr << 1;
  // updatujeme veci, ktore vidime
  For(i,mapa.h) For(j,mapa.w) if(viditelnyTeren.get(j,i) != MAPA_NEVIEM) znamyTeren.set(j,i,viditelnyTeren.get(j,i));
  cerr << 2;
  //updatujeme jednotky
  For(i,int(stav.manici.size()))
    znamyTeren.set(stav.manici[i].x,stav.manici[i].y,stav.manici[i].typ + 10*(stav.manici[i].ktorehoHraca+1));
  cerr << 3;
  //ulozime si do pola nase jednotky
  nasiManici.clear();
  ichManici.clear();
  armada.clear();
  armada.resize(9,0);
  cerr << 4;
  //TODO: Fixnut nejaky segfault tu
  For(i,int(stav.manici.size())) 
  {
    if(stav.manici[i].ktorehoHraca == 0){ 
      nasiManici.push_back(stav.manici[i]);
      armada[stav.manici[i].typ]++;
    }
    else
    {
      bool add = true;
      for(auto& bod : lastSeen)
	if(bod == stav.manici[i].pozicia())
	{add = false; break;}
      if(add)
	lastSeen.push_back(stav.manici[i].pozicia());
      ichManici.push_back(stav.manici[i]);
    }
  }
  cerr << 5;
  lastSeen.remove_if( [](Bod bod){return znamyTeren.get(bod) < 20;} );
  cerr << 6;
  //najdeme kovaca
  kovacX = -1; kovacY = -1;
  for(auto& it : nasiManici)  if(it.typ == MANIK_KOVAC) {kovacX = it.x; kovacY = it.y;}
  cerr << 7;
  //zomreli nam tunelari?
  vector<int> toErase;
  for(auto &i : nasiTunelari){
    bool zije = 0;
    for(auto j : nasiManici) if(j.id == i.first) zije=1;
    if(zije == 0){
      for(auto &k : starty) if( k.first == i.second) k.second = 0;
      toErase.push_back(i.first);
    }
  }
  cerr << 8;
  for(int i : toErase)
    if(nasiTunelari.count(i))
      nasiTunelari.erase(i);
  cerr << 9;
}

bool nasePriechodne(int p){return p==5 || p==6 || p==7;}

void naseBfs(const Teren& teren, Bod start, Teren& vzdialenost) { //prehladajBfs - nevidi_vodjakov, naseBfs - vidi_vojakov
  int inf = teren.w() * teren.h() * 2;
  vzdialenost.vyprazdni(teren.w(), teren.h(), inf);
  queue<Bod> Q;
  vzdialenost.set(start, 0);
  Q.push(start);
  while (!Q.empty()) {
    Bod p = Q.front(); Q.pop();
    for (int d = 0; d < 4; d++) {
      Bod n(p.x + DX[d], p.y + DY[d]);
      if (teren.get(n) == MAPA_OKRAJ) continue;
      if (vzdialenost.get(n) != inf) continue;
      vzdialenost.set(n, vzdialenost.get(p) + 1);
      if (nasePriechodne(teren.get(n))) Q.push(n);
    }
  }
}

void tunelarBfs(const Teren& teren, Bod start, Teren& vzdialenost){ //prehladajBfs - rata len steny
  int inf = teren.w() * teren.h() * 2;
  vzdialenost.vyprazdni(teren.w(), teren.h(), inf);
  queue<Bod> Q;
  vzdialenost.set(start, 0);
  Q.push(start);
  while (!Q.empty()) {
    Bod p = Q.front(); Q.pop();
    for (int d = 0; d < 4; d++) {
      Bod n(p.x + DX[d], p.y + DY[d]);
      if (teren.get(n) == MAPA_OKRAJ) continue;
      if (vzdialenost.get(n) != inf) continue;
      if (priechodne(teren.get(n))) 
	vzdialenost.set(n, vzdialenost.get(p) + 1);
      else
	vzdialenost.set(n, vzdialenost.get(p) + 4);
      //if(teren.get(n) <= 9)
      Q.push(n);
    }
  }
}

bool chodKuMiestu(const Manik &m, Bod ciel) { //vidime vojakov a nevieme cez nich chodit
  Teren vzdialenost;
  znamyTeren.set(m.x,m.y,MAPA_VOLNO);
  naseBfs(znamyTeren, ciel, vzdialenost);
  znamyTeren.set(m.x,m.y,m.typ+10);
  for (int d = 0; d < 4; d++) {
    Bod n(m.x + DX[d], m.y + DY[d]);
    if (nasePriechodne(znamyTeren.get(n)) &&
        vzdialenost.get(n) < vzdialenost.get(m.pozicia())) {
      prikazy.push_back(Prikaz(m.id, PRIKAZ_CHOD, n));
      return 1;
    }
  }
  return 0;
}

bool coRobiKovac(const Manik &m, int typ) {
  cerr << "0";
  For(i,4){
    Bod n(m.x+DX[i], m.y+DY[i]);
    if(znamyTeren.get(n) >= 20){
      prikazy.push_back(Prikaz(m.id, PRIKAZ_UTOC, n));
      cerr << "1";
      return 0;
    }
  }
  cerr << "2";
  For(d,4)
  {
    Bod n(m.x + DX[d], m.y + DY[d]);
    if(znamyTeren.get(n) < 10 && m.kovacEnergia >= kCenaEnergia[typ] && m.zelezo >= kCenaZelezo[typ] && m.zlato >= kCenaZlato[typ]){
      prikazy.push_back(Prikaz(m.id, PRIKAZ_KUJ, n , typ));
      cerr << "3";
      return 1;
    }
  }
  cerr << "4";
  return 0;
}

int inicializujTunelara(){	//posle tunelara k najblizsiemu zijucemu nepriatelovi
  //najdeme dvojicu <manik, start> s najmensou tunelarskou vzdialenostou, nastavime true, ze sme pustili
  //aby sme neutocili na seba - zavisle na kovacovi
  cerr << 1;
  Manik* tunelar = NULL;
  int minVzdialenost = mapa.w * mapa.h;
  pair<Bod, int>* start = NULL;
  cerr << 2;
  for(auto& bod : starty)
    if(bod.first.x != kovacX && bod.first.y != kovacY)// && bod.second==0)
    {
      Teren vzdialenost;
      tunelarBfs(znamyTeren, bod.first, vzdialenost);
      for(auto& manik : nasiManici)
	if(nasiTunelari.count(manik.id) == 0 && manik.typ == MANIK_BANIK && vzdialenost.get(manik.pozicia()) < minVzdialenost)
	{
	  minVzdialenost = vzdialenost.get(manik.pozicia());
	  tunelar = &manik;
	  start = &bod;
	}
    }
  if(tunelar == NULL || start == NULL)
    return -1;
    
  cerr << 3;
  cerr << 4;
  start->second = 1;
  nasiTunelari[tunelar->id] = start->first;
  return tunelar->id;
}

void coRobiTunelar(const Manik &m){
  for (int d = 0; d < 4; d++) {
    int nx = m.x + DX[d], ny = m.y + DY[d];
    if (znamyTeren.get(nx,ny) >= 20){
      prikazy.push_back(Prikaz(m.id, PRIKAZ_UTOC, nx, ny));
      return;
    }
  }
  Bod ciel = nasiTunelari[m.id];
  Teren vzdialenost;
  znamyTeren.set(m.pozicia(), MAPA_VOLNO);
  tunelarBfs(znamyTeren, ciel, vzdialenost);
  znamyTeren.set(m.pozicia(), 10 + MANIK_BANIK);
  Teren normalnaVzdialenost;
  prehladajBfs(znamyTeren, m.pozicia(), normalnaVzdialenost);
  if(normalnaVzdialenost.get(ciel) < mapa.w * mapa.h * 2)
  {
    for(auto& bod : starty)
      if(bod.first == nasiTunelari[m.id])
      {
	bod.second = 2;
      }
    nasiTunelari.erase(m.id);
    return;
  }
  for (int d = 0; d < 4; d++) {
    Bod n(m.x + DX[d], m.y + DY[d]);
    if (nasePriechodne(znamyTeren.get(n)) &&
        vzdialenost.get(n) < vzdialenost.get(m.pozicia())) {
      prikazy.push_back(Prikaz(m.id, PRIKAZ_CHOD, n));
      return;
    }
    if (znamyTeren.get(n) >= 2 && znamyTeren.get(n) <= 4 &&
        vzdialenost.get(n) < vzdialenost.get(m.pozicia())) {
      prikazy.push_back(Prikaz(m.id, PRIKAZ_UTOC, n));
      return;
    }
  }
  
}

void coRobiUtocnyVojak(const Manik &m){ //ked zabijeme nepriatela, nastavime mu start na kovaca 
  cerr << "0";
  For(i,4){
    Bod n(m.x+DX[i], m.y+DY[i]);
    if(znamyTeren.get(n) >= 20){
      prikazy.push_back(Prikaz(m.id, PRIKAZ_UTOC, n));
      wasFight = true;
      return;
    }
  }
  cerr << "1";
  for (int d = 0; d < 4; d++) {
    int nx = m.x + DX[d], ny = m.y + DY[d];
    // ak som hned vedla kovaca a mam mu co dat, dam mu to.
    if (nx == kovacX && ny == kovacY && m.zlato) {
      prikazy.push_back(Prikaz(m.id, PRIKAZ_DAJ_ZLATO, nx, ny, m.zlato));
      return;
    }
    if (nx == kovacX && ny == kovacY && m.zelezo) {
      prikazy.push_back(Prikaz(m.id, PRIKAZ_DAJ_ZELEZO, nx, ny, m.zelezo));
      return;
    }
  }
  cerr << "2";
  // ak som uz vytazil vela surovin, idem nazad za kovacom.
  if ((m.zlato >= 20 || m.zelezo >= 20) && kovacX != -1)
    if (chodKuMiestu(m, Bod(kovacX, kovacY))) return;
  cerr << "3";
  {
    //map<Bod, int> vzdialenost;
    const int R = 1000;
    //prehladajLokalneBfs(znamyTeren, m.pozicia(), R, vzdialenost);
    int minDist = R*R;
    Bod kam(-1, -1);
    /*for(auto& pair : vzdialenost)
      if(znamyTeren.get(pair.first) >= 20 && pair.second < minDist)
      {
	minDist = pair.second;
	kam = pair.first;
      }*/
    if(!lastSeen.empty())
    {
      Teren inaVzdialenost;
      prehladajBfs(znamyTeren, m.pozicia(), inaVzdialenost);
      for(auto& bod : lastSeen)
	if(inaVzdialenost.get(bod) < minDist)
	{
	  minDist = inaVzdialenost.get(bod);
	  kam = bod;
	}
    }
    cerr << "4";
    if(!lastSeen.empty() && minDist < R*R)
    {
      Teren vzdialenost2;
      int backup = znamyTeren.get(m.pozicia());
      znamyTeren.set(m.pozicia(), MAPA_VOLNO);
      naseBfs(znamyTeren, kam, vzdialenost2);
      znamyTeren.set(m.pozicia(), backup);
      int minDist2 = vzdialenost2.get(m.pozicia());
      Bod kam(-1, -1);
      for (int d = 0; d < 4; d++) 
      for (int e = 0; e < 4; e++){
	Bod n(m.x + DX[d], m.y + DY[d]);
	Bod nn(m.x + DX[d] + DX[e], m.y + DY[d] + DY[e]);
	if(nasePriechodne(znamyTeren.get(n)) && nasePriechodne(znamyTeren.get(nn)) && vzdialenost2.get(nn) < minDist2 )
	{
	  minDist2 = vzdialenost2.get(nn);
	  kam = n;
	}
	if(nasePriechodne(znamyTeren.get(n)) && vzdialenost2.get(n) < minDist2 )
	{
	  minDist2 = vzdialenost2.get(n);
	  kam = n;
	}
      }
      if(minDist2 < 100)
      {
	if(kam.x != -1)
	{
	  wasFight = true;
	  prikazy.push_back(Prikaz(m.id, PRIKAZ_CHOD, kam));
	  return;
	}
	Teren vzdialenost3;
	prehladajBfs(znamyTeren, kam, vzdialenost3);
	for (int d = 0; d < 4; d++) {
	  Bod n(m.x + DX[d], m.y + DY[d]);
	  if(nasePriechodne(znamyTeren.get(n)) && vzdialenost3.get(n) < vzdialenost3.get(m.pozicia()) )
	  {
	    wasFight = true;
	    prikazy.push_back(Prikaz(m.id, PRIKAZ_CHOD, n));
	    return;
	  }
	}
      }
    }
    cerr << "5";
  }
  Bod ciel(-1,-1);
  for(auto i : starty) if(i.second == 2 && i.first != Bod(kovacX,kovacY)) ciel = i.first;
  cerr << "6";
  if(znamyTeren.get(ciel) >= 10 && znamyTeren.get(ciel) < 20){
    for(auto &i : starty) if(i.first == ciel){ i.first = Bod(kovacX, kovacY); i.second = 0;} 
    wasFight = true;
    return;
  }
  cerr << "7";
  if(ciel.x == -1 ){ //|| true){
    /*Bod n(m.x+DX[rand()%4], m.y + DY[rand()%4]);
    if(priechodne(znamyTeren.get(n)))prikazy.push_back(Prikaz(m.id, PRIKAZ_CHOD,n ));
    return;*/
    /*if(!lastSeen.empty()) ciel = lastSeen.top(); 
    else*/ 
    ciel = starty[targetStart].first;
  }
  ciel.x = -1;
  ciel.y = -1;
  if(false)
  {
    cerr << "8";
    Teren vzdialenost; 
    int oldTeren = znamyTeren.get(ciel);
    znamyTeren.set(m.x,m.y,MAPA_VOLNO);
    znamyTeren.set(ciel, MAPA_VOLNO);
    naseBfs(znamyTeren,ciel,vzdialenost);
    znamyTeren.set(m.x,m.y,m.typ+10);
    znamyTeren.set(ciel, oldTeren);
    //for(auto i : starty) if(i.second == 2 && i.first != Bod(kovacX,kovacY)) ciel = i.first;
    cerr << "9";
    For(i,4){
      Bod n(m.x+DX[i], m.y+DY[i]);
      if (priechodne(znamyTeren.get(n)) &&
	  vzdialenost.get(n) < vzdialenost.get(m.pozicia())) {
	prikazy.push_back(Prikaz(m.id, PRIKAZ_CHOD, n));
	wasFight = true;
	return;
      }
    }
    cerr << "a";
    
    
    // ak nie je cesta okolo jednotiek tak sa tam natlacime
    prehladajBfs(znamyTeren, ciel, vzdialenost);
    For(i,4){
      Bod n(m.x+DX[i], m.y+DY[i]);
      if (priechodne(znamyTeren.get(n)) &&
	  vzdialenost.get(n) < vzdialenost.get(m.pozicia())) {
	prikazy.push_back(Prikaz(m.id, PRIKAZ_CHOD, n));
	wasFight = true;
	return;
      }
    }
  }
  cerr << "b";
  {
    Teren vzdialenost;
    znamyTeren.set(m.pozicia(), MAPA_VOLNO);
    naseBfs(znamyTeren, m.pozicia(), vzdialenost);
    znamyTeren.set(m.pozicia(), 10 + m.typ);
    int minDist = mapa.w * mapa.h * 2;
    Bod ciel(-1, -1);
    for(int x = 0; x < mapa.w; x++)
      for(int y = 0; y < mapa.h; y++)
      {
	Bod n(x, y);
	if(znamyTeren.get(n) == MAPA_NEVIEM && vzdialenost.get(n) < minDist)
	{
	  ciel = n;
	  minDist = vzdialenost.get(n);
	}
      }
    if(ciel.x != -1)
    {
      Teren vzdialenost2;
      
      int oldTeren = znamyTeren.get(ciel);
      znamyTeren.set(m.pozicia(), MAPA_VOLNO);
      znamyTeren.set(ciel, MAPA_VOLNO);
      naseBfs(znamyTeren, ciel, vzdialenost2);
      znamyTeren.set(ciel, oldTeren);
      znamyTeren.set(m.pozicia(), 10 + MANIK_SEKAC);
      for (int d = 0; d < 4; d++) {
	Bod n(m.x + DX[d], m.y + DY[d]);
	if(nasePriechodne(znamyTeren.get(n)) && vzdialenost2.get(n) < vzdialenost2.get(m.pozicia()) )
	{
	  prikazy.push_back(Prikaz(m.id, PRIKAZ_CHOD, n));
	  return;
	}
      }
    }
  }
  prikazy.push_back(Prikaz(m.id, PRIKAZ_CHOD, Bod(m.x + DX[rand()%4], m.y + DY[rand()%4])));
  cerr << "c";
}

void coRobiObrannyVojak(const Manik &m){
  int RAD = (m.typ==MANIK_STRELEC?2:1);
  For(i,4){
    for(int r = 1; r <= RAD; r++)
    {
      if(abs(DX[i])*r + abs(DY[i])*r > r)
	continue;
      Bod n(m.x+DX[i]*r, m.y+DY[i]*r);
      if(znamyTeren.get(n) >= 20){
	prikazy.push_back(Prikaz(m.id, PRIKAZ_UTOC, n));
	return;
      }
    }
  }
  
    map<Bod, int> vzdialenost;
    const int R = 20;
    prehladajLokalneBfs(znamyTeren, m.pozicia(), R, vzdialenost);
    int minDist = R*R;
    Bod kam;
    for(auto& pair : vzdialenost)
      if(znamyTeren.get(pair.first) >= 20 && pair.second < minDist)
      {
	minDist = pair.second;
	kam = pair.first;
      }
    if(minDist < R*R)
    {
      Teren vzdialenost2;
      int backup = znamyTeren.get(m.pozicia());
      znamyTeren.set(m.pozicia(), MAPA_VOLNO);
      naseBfs(znamyTeren, kam, vzdialenost2);
      znamyTeren.set(m.pozicia(), backup);
      for (int d = 0; d < 4; d++) {
	Bod n(m.x + DX[d], m.y + DY[d]);
	if(nasePriechodne(znamyTeren.get(n)) && vzdialenost2.get(n) < vzdialenost2.get(m.pozicia()) )
	{
	  prikazy.push_back(Prikaz(m.id, PRIKAZ_CHOD, n));
	  return;
	}
      }
      Teren vzdialenost3;
      prehladajBfs(znamyTeren, kam, vzdialenost3);
      for (int d = 0; d < 4; d++) {
	Bod n(m.x + DX[d], m.y + DY[d]);
	if(nasePriechodne(znamyTeren.get(n)) && vzdialenost3.get(n) < vzdialenost3.get(m.pozicia()) )
	{
	  prikazy.push_back(Prikaz(m.id, PRIKAZ_CHOD, n));
	  return;
	}
      }
    }
  
  
  Bod n(m.x+DX[rand()%4], m.y + DY[rand()%4]);
  if(abs(m.x-n.x) + abs(m.y-n.y) < 6 && priechodne(znamyTeren.get(n)))
  prikazy.push_back(Prikaz(m.id, PRIKAZ_CHOD, n));
}

void coRobiSkaut(const Manik &m)
{
  Teren vzdialenost;
  znamyTeren.set(m.pozicia(), MAPA_VOLNO);
  naseBfs(znamyTeren, m.pozicia(), vzdialenost);
  znamyTeren.set(m.pozicia(), 10 + MANIK_SKAUT);
  int minDist = mapa.w * mapa.h * 2;
  Bod ciel(-1, -1);
  Bod nepriatel(-1, -1);
  for(int x = 0; x < mapa.w; x++)
    for(int y = 0; y < mapa.h; y++)
    {
      Bod n(x, y);
      if(znamyTeren.get(n) == MAPA_NEVIEM && vzdialenost.get(n) < minDist)
      {
	ciel = n;
	minDist = vzdialenost.get(n);
      }
      if(znamyTeren.get(n) >= 20 && vzdialenost.get(n) < 12)
	nepriatel = n;
    }
  if(nepriatel.x != -1)
  {
    Teren vzdialenost2;
    prehladajBfs(znamyTeren, nepriatel, vzdialenost2);
    for (int d = 0; d < 4; d++) {
      Bod n(m.x + DX[d], m.y + DY[d]);
      if(vzdialenost2.get(n) > vzdialenost2.get(m.pozicia()) )
      {
	prikazy.push_back(Prikaz(m.id, PRIKAZ_CHOD, n));
	return;
      }
    }
  }
  if(ciel.x == -1)
  {
    for(int x = 0; x < mapa.w; x++)
      for(int y = 0; y < mapa.h; y++)
      {
	Bod n(x, y);
	if(znamyTeren.get(n) == MAPA_SUTER && vzdialenost.get(n) < minDist)
	{
	  ciel = n;
	  minDist = vzdialenost.get(n);
	}
      }
  }
  if(ciel.x == -1)
    return;
  Teren vzdialenost2;
  
  int oldTeren = znamyTeren.get(ciel);
  znamyTeren.set(m.pozicia(), MAPA_VOLNO);
  znamyTeren.set(ciel, MAPA_VOLNO);
  naseBfs(znamyTeren, ciel, vzdialenost2);
  znamyTeren.set(ciel, oldTeren);
  znamyTeren.set(m.pozicia(), 10 + MANIK_SKAUT);
  for (int d = 0; d < 4; d++) {
    Bod n(m.x + DX[d], m.y + DY[d]);
    if(nasePriechodne(znamyTeren.get(n)) && vzdialenost2.get(n) < vzdialenost2.get(m.pozicia()) )
    {
      prikazy.push_back(Prikaz(m.id, PRIKAZ_CHOD, n));
      return;
    }
  }
}

void coRobiBanik(const Manik &m) {
  for(auto& it: nasiTunelari) if (it.first == m.id) {coRobiTunelar(m); return;}
  if(oldDotazene > 10)
  {
    For(i,4){
      Bod n(m.x+DX[i], m.y+DY[i]);
      if(znamyTeren.get(n) == 10 + MANIK_BANIK){
	prikazy.push_back(Prikaz(m.id, PRIKAZ_UTOC, n));
	return;
      }
    }
    
    if (chodKuMiestu(m, Bod(kovacX, kovacY))) return;
    return;
  }
  
  {
    Teren vzdialenost;
    znamyTeren.set(m.pozicia(), MAPA_VOLNO);
    naseBfs(znamyTeren, m.pozicia(), vzdialenost);
    znamyTeren.set(m.pozicia(), 10 + MANIK_BANIK);
    Bod nepriatel(-1, -1);
    for(int x = 0; x < mapa.w && nepriatel.x == -1; x++)
      for(int y = 0; y < mapa.h && nepriatel.x == -1; y++)
      {
	Bod n(x, y);
	if(znamyTeren.get(n) >= 20 && vzdialenost.get(n) < 6)
	  nepriatel = n;
      }
    if(nepriatel.x != -1)
    {
      Teren vzdialenost2;
      prehladajBfs(znamyTeren, nepriatel, vzdialenost2);
      for (int d = 0; d < 4; d++) {
	Bod n(m.x + DX[d], m.y + DY[d]);
	if(vzdialenost2.get(n) > vzdialenost2.get(m.pozicia()) )
	{
	  prikazy.push_back(Prikaz(m.id, PRIKAZ_CHOD, n));
	  return;
	}
      }
    }
  }
  
  {
    map<Bod, int> vzdialenost;
    const int R = 10;
    prehladajLokalneBfs(znamyTeren, m.pozicia(), R, vzdialenost);
    int minDist = R*R;
    Bod kam;
    for(auto& pair : vzdialenost)
      if(znamyTeren.get(pair.first) % 10 == 0 && znamyTeren.get(pair.first)>10 && pair.second < minDist)
      {
	minDist = pair.second;
	kam = pair.first;
      }
    if(minDist < R*R)
    {
      Teren vzdialenost2;
      int backup = znamyTeren.get(m.pozicia());
      znamyTeren.set(m.pozicia(), MAPA_VOLNO);
      naseBfs(znamyTeren, kam, vzdialenost2);
      znamyTeren.set(m.pozicia(), backup);
      for (int d = 0; d < 4; d++) {
	Bod n(m.x + DX[d], m.y + DY[d]);
	if(nasePriechodne(znamyTeren.get(n)) && vzdialenost2.get(n) < vzdialenost2.get(m.pozicia()) )
	{
	  wasFight = true;
	  prikazy.push_back(Prikaz(m.id, PRIKAZ_CHOD, n));
	  return;
	}
      }
      Teren vzdialenost3;
      prehladajBfs(znamyTeren, kam, vzdialenost3);
      for (int d = 0; d < 4; d++) {
	Bod n(m.x + DX[d], m.y + DY[d]);
	if(nasePriechodne(znamyTeren.get(n)) && vzdialenost3.get(n) < vzdialenost3.get(m.pozicia()) )
	{
	  wasFight = true;
	  prikazy.push_back(Prikaz(m.id, PRIKAZ_CHOD, n));
	  return;
	}
      }
    }
  }
  
  for (int d = 0; d < 4; d++) {
    int nx = m.x + DX[d], ny = m.y + DY[d];
    // ak som hned vedla zlata alebo zeleza, tazim.
    if (znamyTeren.get(nx,ny) >= 20){
      prikazy.push_back(Prikaz(m.id, PRIKAZ_UTOC, nx, ny));
      return;
    }
    if (znamyTeren.get(nx, ny) == MAPA_ZLATO || znamyTeren.get(nx, ny) == MAPA_ZELEZO) {
      prikazy.push_back(Prikaz(m.id, PRIKAZ_UTOC, nx, ny));
      return;
    }
    // ak som hned vedla kovaca a mam mu co dat, dam mu to.
    if (nx == kovacX && ny == kovacY && m.zlato) {
      prikazy.push_back(Prikaz(m.id, PRIKAZ_DAJ_ZLATO, nx, ny, m.zlato));
      return;
    }
    if (nx == kovacX && ny == kovacY && m.zelezo) {
      prikazy.push_back(Prikaz(m.id, PRIKAZ_DAJ_ZELEZO, nx, ny, m.zelezo));
      return;
    }
  }

  // ak som uz vytazil vela surovin, idem nazad za kovacom.
  if ((m.zlato >= 20 || m.zelezo >= 20) && kovacX != -1) {
    if (chodKuMiestu(m, Bod(kovacX, kovacY))) return;
    //ak sa nepohneme, posunieme niekomu okolo
    else
    {
      Teren odKovaca;
      prehladajBfs(znamyTeren, Bod(kovacX, kovacY), odKovaca); 
      for (int d = 0; d < 4; d++) {
	Bod n(m.x + DX[d], m.y + DY[d]);
	if(odKovaca.get(n) < odKovaca.get(m.pozicia()) &&
	  znamyTeren.get(n) == 10
	) {
	  if(m.zlato > m.zelezo)
	    prikazy.push_back(Prikaz(m.id, PRIKAZ_DAJ_ZLATO, n));
	  else
	    prikazy.push_back(Prikaz(m.id, PRIKAZ_DAJ_ZELEZO, n));
	  return;
	}
      }
      
    }
  }

  // ak vidime nejake zlato alebo zelezo, idem k nemu. nie k najblizsiemu
  Teren vzdialenost;
  
  znamyTeren.set(m.x,m.y,MAPA_VOLNO);
  naseBfs(znamyTeren, m.pozicia(), vzdialenost);
  znamyTeren.set(m.x,m.y,m.typ+10);
  Bod bestpKov(-1, -1); 
  int bestdistKov = mapa.w * mapa.h * 2;
  for (int y = 0; y < mapa.h; y++) for (int x = 0; x < mapa.w; x++) {
    if ((znamyTeren.get(x, y) == MAPA_ZLATO || znamyTeren.get(x, y) == MAPA_ZELEZO) && vzdialenost.get(x, y) < bestdistKov) {
      bestpKov = Bod(x, y); bestdistKov = vzdialenost.get(x, y);
    }
  }

  // ak nie, tak idem za najblizsim sutrom a snad niekde nieco najdem...
  
  Bod bestpSuter(-1, -1); 
  int bestdistSuter = mapa.w * mapa.h * 2;
  for (int y = 0; y < mapa.h; y++) for (int x = 0; x < mapa.w; x++) {
    if (znamyTeren.get(x, y) == MAPA_SUTER && vzdialenost.get(x, y) < bestdistSuter && 
	(x%10<1 || y%3<1 || (tah > 50 && abs(x-kovacX) + abs(y-kovacY) < 7))) {
      bestpSuter = Bod(x, y); bestdistSuter = vzdialenost.get(x, y);
    }
  }
  
  if(bestdistKov == mapa.w * mapa.h * 2 && bestdistSuter == mapa.w * mapa.h * 2 && tah > 50)
    dotazene++;
  else
    dotazene = -20;
  
  if(bestdistKov < 3*bestdistSuter)
  {
    if (bestpKov.x != -1) {
      chodKuMiestu(m, bestpKov);
      return;
    }
  }
  else
  {
    if (bestpSuter.x != -1) {
      // ked uz pri tom najblizsom sutri stojim tak ho vykopem
      if (abs(bestpSuter.x - m.x) + abs(bestpSuter.y - m.y) == 1) {
	prikazy.push_back(Prikaz(m.id, PRIKAZ_UTOC, bestpSuter));
	return;
      }
      // inak sa k nemu priblizim
      chodKuMiestu(m, bestpSuter);
      return;
    }
  }
  {
    Teren vzdialenost;
    znamyTeren.set(m.pozicia(), MAPA_VOLNO);
    naseBfs(znamyTeren, m.pozicia(), vzdialenost);
    znamyTeren.set(m.pozicia(), 10 + m.typ);
    int minDist = mapa.w * mapa.h * 2;
    Bod ciel(-1, -1);
    for(int x = 0; x < mapa.w; x++)
      for(int y = 0; y < mapa.h; y++)
      {
	Bod n(x, y);
	if(znamyTeren.get(n) == MAPA_NEVIEM && vzdialenost.get(n) < minDist)
	{
	  ciel = n;
	  minDist = vzdialenost.get(n);
	}
      }
    if(ciel.x != -1)
    {
      Teren vzdialenost2;
      
      int oldTeren = znamyTeren.get(ciel);
      znamyTeren.set(m.pozicia(), MAPA_VOLNO);
      znamyTeren.set(ciel, MAPA_VOLNO);
      naseBfs(znamyTeren, ciel, vzdialenost2);
      znamyTeren.set(ciel, oldTeren);
      znamyTeren.set(m.pozicia(), 10 + MANIK_SEKAC);
      for (int d = 0; d < 4; d++) {
	Bod n(m.x + DX[d], m.y + DY[d]);
	if(nasePriechodne(znamyTeren.get(n)) && vzdialenost2.get(n) < vzdialenost2.get(m.pozicia()) )
	{
	  prikazy.push_back(Prikaz(m.id, PRIKAZ_CHOD, n));
	  oldDotazene--;
	  return;
	}
      }
    }
  }
  
  Bod n(m.x+DX[rand()%4], m.y + DY[rand()%4]);
  if(abs(m.x-n.x) + abs(m.y-n.y) < 10 && priechodne(znamyTeren.get(n)))
  prikazy.push_back(Prikaz(m.id, PRIKAZ_CHOD, n));

}

// main() zavola tuto funkciu, ked nacita mapu
void inicializuj() {
  srandom(time(NULL));
  znamyTeren.vyprazdni(mapa.w, mapa.h, MAPA_NEVIEM);
  //najdeme starty hracov pre tunelarov
  For(x, mapa.w)
    For(y, mapa.h)
    {
      if(mapa.pribliznyTeren.get(x, y) == MAPA_START)
      {
	Bod b(x, y);
	starty.push_back(pair<Bod,int>(b, 0));
	nStartov++;
      }
    }
}


// main() zavola tuto funkciu, ked chce vediet, ake prikazy chceme vykonat,
// co tato funkcia rozhodne pomocou: prikazy.push_back(Prikaz(...));
void zistiTah() {
  cerr << "A";
  update_znamyTeren();
  cerr << "B";
  int na_kolko_nepriatelov_mozeme_utocit=0;
  int kolko_nepriatelov_tunelujeme = 0;
  for(auto i : starty)
  {
    if(i.first != Bod(kovacX,kovacY) && i.second == 2) na_kolko_nepriatelov_mozeme_utocit++;
    else if(i.first != Bod(kovacX,kovacY) && i.second == 1) kolko_nepriatelov_tunelujeme++;
    if(tah > 150 && znamyTeren.get(i.first) == MAPA_NEVIEM)
      ;//inicializujTunelara(i.first); //TODO: spravit aby to generovalo viac tunelarov ako segfaultov
  }
  cerr << "C";
  if(tah > 80 && (kolko_nepriatelov_tunelujeme==0 || rand()%100 == 0))
      inicializujTunelara();
  cerr << "D";
  //fprintf(stderr,"%d %d\n",kolko_nepriatelov_tunelujeme,na_kolko_nepriatelov_mozeme_utocit);
  tah++;
  //fprintf(stderr,"%d\n",armada[MANIK_BANIK]);
  if(chceme_postavit == -1); // WAT
  
  // !wasFight ||
  int counter = 0;
  while((starty[targetStart].first.x == kovacX && starty[targetStart].first.y == kovacY) || 
    znamyTeren.get(starty[targetStart].first)%10 != MANIK_KOVAC)
  {
    targetStart = (targetStart + 1) % nStartov;
    if(counter++ > nStartov+1)
      break;
  }
  cerr << "E\n";
  
  oldDotazene = dotazene;
  wasFight = false;
  dotazene = 0;
  for(auto& it : nasiManici){
    cerr << "manik " << it.typ << " begin\n";
    switch (it.typ) {
      case MANIK_KOVAC:
	if (chceme_postavit == -1){
	  if(tah > 500 && armada[MANIK_KOVAC] == 1){chceme_postavit = MANIK_KOVAC;}
	  else if(tah > 30 && rand()%5==0 && armada[MANIK_STRAZNIK] < 15) chceme_postavit = MANIK_STRAZNIK;
	  else if(tah > 80 && rand()%10==0 && armada[MANIK_STRELEC] < 5) chceme_postavit = MANIK_STRELEC;
	  else if(tah > 400 && armada[MANIK_SKAUT] < 0 && rand()%3 == 0) chceme_postavit = MANIK_SKAUT;
	  else if((na_kolko_nepriatelov_mozeme_utocit > 0 || !lastSeen.empty()) && rand()%2==0) chceme_postavit = MANIK_SEKAC; // mozno otocit naspat
	  else if(tah < 400 && oldDotazene < 10 && armada[MANIK_BANIK] < 45)chceme_postavit = MANIK_BANIK;
	  else if(rand() % (1+2*armada[MANIK_SEKAC] + armada[MANIK_STRAZNIK]) > armada[MANIK_STRAZNIK]) chceme_postavit = MANIK_SEKAC;
	  else chceme_postavit = MANIK_STRAZNIK;
	}
	if (chceme_postavit != -1 && it.kovacEnergia >= kCenaEnergia[chceme_postavit]){
	  bool b = coRobiKovac(it,chceme_postavit);
	  if(!b)
	  {
	    for (int d = 0; d < 4; d++) {
	      Bod n(it.x + DX[d], it.y + DY[d]);
	      if(znamyTeren.get(n) == 10 + MANIK_KOVAC)
	      {
		if(it.zlato > it.zelezo)
		  prikazy.push_back(Prikaz(it.id, PRIKAZ_DAJ_ZLATO, n, it.zlato/2));
		else
		  prikazy.push_back(Prikaz(it.id, PRIKAZ_DAJ_ZELEZO, n, it.zelezo/2));
		break;
	      }
	    }
	  }
	  if(chceme_postavit == MANIK_KOVAC && b) druhyKovac = 1;
	  chceme_postavit = -1;
	}
	else
	{
	  for (int d = 0; d < 4; d++) {
	      Bod n(it.x + DX[d], it.y + DY[d]);
	      if(znamyTeren.get(n) == 10 + MANIK_KOVAC)
	      {
		if(it.zlato > it.zelezo)
		  prikazy.push_back(Prikaz(it.id, PRIKAZ_DAJ_ZLATO, n, it.zlato/2));
		else
		  prikazy.push_back(Prikaz(it.id, PRIKAZ_DAJ_ZELEZO, n, it.zelezo/2));
		break;
	      }
	    }
	}
        break;

      case MANIK_BANIK:
	coRobiBanik(it);
	break;
      
      case MANIK_STRELEC:
      case MANIK_STRAZNIK:
	coRobiObrannyVojak(it);
	break;
	
      case MANIK_SKAUT:
	coRobiSkaut(it);
	break;
      
      default:	//utocime
	coRobiUtocnyVojak(it);
	cerr << " end\n";
	break;
    }
    cerr << "manik end\n";
  }
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

