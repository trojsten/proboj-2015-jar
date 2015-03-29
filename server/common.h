
#ifndef COMMON_H
#define COMMON_H

// vseobecne datove struktury a tak podobne

#include <vector>
#include <string>

#define FOREACH(it,c) for (typeof((c).begin()) it = (c).begin(); it != (c).end(); ++it)


#define HORE 0
#define PRAVO 1
#define DOLE 2
#define LAVO 3


#define MAPA_SUTER   0
#define MAPA_VOLNO   1
#define MAPA_START   2
#define MAPA_BONUS   3


#define NIC -1

#define priechodne(p) ((p) >= MAPA_VOLNO)


struct Bod {
  int x, y;
  Bod() : x(0), y(0) {}
  Bod(int _x, int _y) : x(_x), y(_y) {}
  bool operator==(const Bod& b) const { return x == b.x && y == b.y; }
  bool operator!=(const Bod& b) const { return !(x == b.x && y == b.y); }
  bool operator<(const Bod& b) const { return y < b.y || (y == b.y && x < b.x); }
};


struct Prikaz {
  int smer;   // id manika co to ma vykonat
  int pouzi;
  int special;
  
  Prikaz() {
      smer = HORE;
      pozil = NIC;
      special = NIC;
  }
  
  char* toString(){
      char bufer [50];
      int n = sprintf(bufer,"smer:%d\npouzi: %d\nspecial: %d\n",this.smer,this.pouzi, this.special);
      return bufer;      
  }
  
};


typedef Prikaz Odpoved;


struct Hrac {
  int skore;
  std::vector<int> mapovanie;   // klienti nevidia
};


struct Snake {
  int ktorehoHraca;   // vy ste 0
  vector<Bod> telo;
  int smer;
  int dorastie;
  vector<int> bonusy;
  vector<int> buffy;
  int zije;
};


struct Teren {
  std::vector<std::vector<int> > data;

  int h() const { return data.size(); }
  int w() const { return data.empty() ? 0 : data[0].size(); }
  int get(int x, int y) const {
    return (y >= 0 && y < (int)data.size() &&
            x >= 0 && x < (int)data[y].size() ? data[y][x] : MAPA_OKRAJ);
  }
  int get(Bod b) const { return get(b.x, b.y); }
  void set(int x, int y, int t) {
    if (y >= 0 && y < (int)data.size() &&
        x >= 0 && x < (int)data[y].size()) data[y][x] = t;
  }
  void set(Bod b, int t) { set(b.x, b.y, t); }
  void vyprazdni (int w, int h, int t) {
    data.resize(h);
    for (int y = 0; y < h; y++) data[y].assign(w, t);
  }
};


struct Stav {
  std::vector<Hrac> hraci;
  std::vector<Snake> hadi;   // klienti vidia ciastocne
  Teren teren;   // klienti nevidia, maju iba viditelnyTeren
  int dalsiId;
  int cas;
};


struct Mapa {
  int pocetHracov;
  int w;
  int h;
  Teren pribliznyTeren;   // zlato a zelezo nemaju presne miesta
};


#define FORMAT_VERSION 1

#endif

#ifdef reflection
// tieto udaje pouziva marshal.cpp aby vedel ako tie struktury ukladat a nacitavat

reflection(Bod);
  member(x);
  member(y);
end();

reflection(Prikaz);
  member(kto);
  member(typPrikazu);
  member(ciel);
  member(parameter);
end();

reflection(Hrac);
  member(skore);
  member(mapovanie);
end();

reflection(Manik);
  member(id);
  member(x);
  member(y);
  member(ktorehoHraca);
  member(typ);
  member(zlato);
  member(zelezo);
  member(spenat);
  member(kovacEnergia);
end();

reflection(Teren);
  member(data);
end();

reflection(Stav);
  member(hraci);
  member(manici);
  // teren neserializujeme
  member(dalsiId);
  member(cas);
end();

reflection(Mapa);
  member(pocetHracov);
  member(w);
  member(h);
  member(pribliznyTeren);
end();

#endif
