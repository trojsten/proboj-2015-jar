#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <set>
using namespace std;

#include "common.h"
#include "marshal.h"
#include "update.h"

#include "util.cpp" /////TODO


Mapa mapa;
Stav stav;   // vzdy som hrac cislo 0
Teren viditelnyTeren;
Teren objavenyTeren;
vector<Prikaz> prikazy;
map<int,Bod> cieleMlaticov;
set<Bod> nepriatelskeStarty;


// main() zavola tuto funkciu, ked nacita mapu
void inicializuj() {
  // (sem patri vas kod)

  objavenyTeren.vyprazdni(mapa.w, mapa.h, MAPA_NEVIEM);

  for (int y = 0; y < mapa.h; y++) for (int x = 0; x < mapa.w; x++) {
    if (mapa.pribliznyTeren.get(x, y) == MAPA_START) {
      nepriatelskeStarty.insert(Bod(x, y));
    }
  }
}


static void chodKuMiestu(const Manik &m, Bod ciel) {
  Teren vzdialenost;
  prehladajBfs(objavenyTeren, ciel, vzdialenost);
  for (int d = 0; d < 4; d++) {
    Bod n(m.x + DX[d], m.y + DY[d]);
    if (priechodne(objavenyTeren.get(n)) &&
        vzdialenost.get(n) < vzdialenost.get(m.pozicia())) {
      prikazy.push_back(Prikaz(m.id, PRIKAZ_CHOD, n));
      break;
    }
  }
}


void coRobiKovac(const Manik &m) {
  nepriatelskeStarty.erase(m.pozicia());

  int kolkoMamBanikov = 0;
  FOREACH(it, stav.manici) {
    if (it->ktorehoHraca == 0 && it->typ == MANIK_BANIK) {
      kolkoMamBanikov++;
    }
  }

  int d = rand() % 4;
  int typ = kolkoMamBanikov < 2 || rand() % 4 == 0 ? MANIK_BANIK : MANIK_MLATIC;
  prikazy.push_back(Prikaz(m.id, PRIKAZ_KUJ, m.x + DX[d], m.y + DY[d], typ));
}


void coRobiBanik(const Manik &m) {
//  fprintf(stderr, "som banik %d sedim na %d,%d\n", m.id, m.x, m.y);
  // najprv si zistim kde je kovac.
  int kovacX = -1, kovacY = -1;
  FOREACH(it, stav.manici) {
    if (it->ktorehoHraca == 0 && it->typ == MANIK_KOVAC) {
      kovacX = it->x; kovacY = it->y;
    }
  }

  for (int d = 0; d < 4; d++) {
    int nx = m.x + DX[d], ny = m.y + DY[d];
    // ak som hned vedla zlata alebo zeleza, tazim.
    if (objavenyTeren.get(nx, ny) == MAPA_ZLATO || objavenyTeren.get(nx, ny) == MAPA_ZELEZO) {
//      fprintf(stderr, "tazim vedla mna na %d,%d\n", nx, ny);
      prikazy.push_back(Prikaz(m.id, PRIKAZ_UTOC, nx, ny));
      return;
    }
    // ak som hned vedla kovaca a mam mu co dat, dam mu to.
    if (nx == kovacX && ny == kovacY && m.zlato) {
//      fprintf(stderr, "davam kovacovi zlato\n");
      prikazy.push_back(Prikaz(m.id, PRIKAZ_DAJ_ZLATO, nx, ny, m.zlato));
      return;
    }
    if (nx == kovacX && ny == kovacY && m.zelezo) {
//      fprintf(stderr, "davam kovacovi zelezo\n");
      prikazy.push_back(Prikaz(m.id, PRIKAZ_DAJ_ZELEZO, nx, ny, m.zelezo));
      return;
    }
  }

  // ak som uz vytazil vela surovin, idem nazad za kovacom.
  if ((m.zlato >= 30 || m.zelezo >= 30) && kovacX != -1) {
//    fprintf(stderr, "mierim za kovacom\n");
    chodKuMiestu(m, Bod(kovacX, kovacY));
    return;
  }

  // ak vidim pobliz nejake zlato alebo zelezo, idem tam.
  Teren vzdialenost;
  prehladajBfs(objavenyTeren, m.pozicia(), vzdialenost);
  Bod bestp(-1, -1); int bestdist = mapa.w * mapa.h;
  for (int y = 0; y < mapa.h; y++) for (int x = 0; x < mapa.w; x++) {
    if ((objavenyTeren.get(x, y) == MAPA_ZLATO || objavenyTeren.get(x, y) == MAPA_ZELEZO) && vzdialenost.get(x, y) < bestdist) {
      bestp = Bod(x, y); bestdist = vzdialenost.get(x, y);
    }
  }
  if (bestp.x != -1) {
//    fprintf(stderr, "objavil som pobliz zlato/zelezo %d,%d\n", bestp.x, bestp.y);
    chodKuMiestu(m, bestp);
    return;
  }
  // ak nie, tak idem za najblizsim sutrom a snad niekde nieco najdem...
  for (int y = 0; y < mapa.h; y++) for (int x = 0; x < mapa.w; x++) {
    if (objavenyTeren.get(x, y) == MAPA_SUTER && vzdialenost.get(x, y) < bestdist) {
      bestp = Bod(x, y); bestdist = vzdialenost.get(x, y);
    }
  }
  if (bestp.x != -1) {
    if (abs(bestp.x - m.x) + abs(bestp.y - m.y) == 1) {
//      fprintf(stderr, "neviem nic lepsie tak tazim suter %d,%d\n", bestp.x, bestp.y);
      prikazy.push_back(Prikaz(m.id, PRIKAZ_UTOC, bestp));
      return;
    }
//    fprintf(stderr, "neviem nic lepsie tak idem k sutru %d,%d\n", bestp.x, bestp.y);
    chodKuMiestu(m, bestp);
    return;
  }
//  fprintf(stderr, "wut nevidim ani suter?\n");
}


void coRobiMlatic(Manik &m) {
  // ak je pri mne nejaky nepriatel, tak ho mlatim.
  FOREACH(it, stav.manici) {
    if (it->ktorehoHraca == 0) continue;
    if (abs(it->x - m.x) + abs(it->y - m.y) == 1) {
      prikazy.push_back(Prikaz(m.id, PRIKAZ_UTOC, it->x, it->y));
      return;
    }
  }

  Teren vzdialenost;
  prehladajBfs(objavenyTeren, m.pozicia(), vzdialenost);

  // ak vidime nejakeho nepriatela, tak za nim idem.
  Bod bestp(-1, -1); int bestdist = mapa.w * mapa.h;
  FOREACH(it, stav.manici) {
    if (it->ktorehoHraca == 0) continue;
    if (vzdialenost.get(it->pozicia()) < bestdist) {
      bestdist = vzdialenost.get(it->pozicia());
      bestp = it->pozicia();
    }
  }
  if (bestp.x != -1) {
    chodKuMiestu(m, bestp);
    return;
  }

  // inak sa idem prerubat ku niektoremu nepriatelskemu startu.
  if (!cieleMlaticov.count(m.id)) {
    if (nepriatelskeStarty.empty()) return;
    vector<Bod> nsv(nepriatelskeStarty.begin(), nepriatelskeStarty.end());
    cieleMlaticov[m.id] = nsv[rand() % nsv.size()];
  }
  Bod n;
  if (cieleMlaticov[m.id].x < m.x) n = Bod(m.x-1, m.y);
  else if (cieleMlaticov[m.id].x > m.x) n = Bod(m.x+1, m.y);
  else if (cieleMlaticov[m.id].y < m.y) n = Bod(m.x, m.y-1);
  else if (cieleMlaticov[m.id].y > m.y) n = Bod(m.x, m.y+1);
  else {
    nepriatelskeStarty.erase(cieleMlaticov[m.id]);
    cieleMlaticov.erase(m.id);
    return;
  }
  prikazy.push_back(Prikaz(m.id, priechodne(objavenyTeren.get(n)) ? PRIKAZ_CHOD : PRIKAZ_UTOC, n));
//  int d = rand() % 4;
//  prikazy.push_back(Prikaz(m.id, PRIKAZ_CHOD, m.x + DX[d], m.y + DY[d]));
}


// main() zavola tuto funkciu, ked chce vediet, ake prikazy chceme vykonat
void zistiTah() {
  // (sem patri vas kod)

  fprintf(stderr, "zistiTah zacina %d\n", stav.cas);

  // zapamatame si teren co vidime a doteraz sme nevideli
  for (int y = 0; y < mapa.h; y++) for (int x = 0; x < mapa.w; x++) {
    if (viditelnyTeren.get(x, y) != MAPA_NEVIEM) {
      objavenyTeren.set(x, y, viditelnyTeren.get(x, y));
    }
  }

  if (stav.cas == 5) {
    for (int y = 0; y < mapa.h; y++) {
      for (int x = 0; x < mapa.w; x++) fprintf(stderr, "%d", objavenyTeren.get(x, y));
      fprintf(stderr, "\n");
    }
  }

  // kazdemu nasmu manikovi povieme co ma robit
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
        coRobiMlatic(*it);
        break;
    }
  }

  fprintf(stderr, "prikazov %d\n", (int)prikazy.size());
//  if (stav.cas == 990) exit(0);
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
    long long u = gettime();
    dekodujViditelnyTeren(zakodovanyTeren, viditelnyTeren);
    nacitaj(cin, stav);
    prikazy.clear();
    zistiTah();
    fprintf(stderr, "trvalo %lld\n", gettime() - u);
    uloz(cout, prikazy);
    cout << ".\n" << flush;   // bodka a flush = koniec odpovede
  }

  return 0;
}

