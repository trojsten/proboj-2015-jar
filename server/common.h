
#ifndef COMMON_H
#define COMMON_H

// vseobecne datove struktury a makra

#include <vector>
#include <string>
#include <cstdio>

#define FOREACH(it,c) for (typeof((c).begin()) it = (c).begin(); it != (c).end(); ++it)


#define HORE 0
#define PRAVO 1
#define DOLE 2
#define LAVO 3


#define MAPA_OKRAJ   1
#define MAPA_SUTER   2
#define MAPA_VOLNO   5
#define MAPA_START   6
#define MAPA_SPAWN   8

#define JEDLO_TRAPNE      0 //nic sa nestane, len sa predlzite
#define JEDLO_REVERS      1 //had sa otoci (pojdete dozadu), teda vector telo sa reverzne. vyhodnoti sa este pred pohybom a pohnete sa z novov hlavov
#define JEDLO_PREZEN      2 //vyprazdni sa vam zasobnik, nic sa nepouzije.
#define JEDLO_BOMBA       3 //vyparia sa kamene na najblizsich dvoch polickach v smere posledneho pohybu (neplati na oraj mapy)
#define JEDLO_OTRAVA      4 //vyznacuje sa zapornym predlzenim
#define JEDLO_CHOLERA     5 //kazdy okrem vas dostane otravu
#define JEDLO_MIXER       6 //predlzite sa o predlzenie kazdeho jedla v zasobe. krat 2. potom sa zasobnik vyprazdni ale nic sa v skutocnosti nepouzije


#define NIC -1

#define priechodne(p) ((p) >= MAPA_VOLNO)


struct Bod {
  int x, y;
  Bod() : x(0), y(0) {}
  Bod(int _x, int _y) : x(_x), y(_y) {}
  bool operator==(const Bod& b) const { return x == b.x && y == b.y; }
  bool operator!=(const Bod& b) const { return !(x == b.x && y == b.y); }
  bool operator<(const Bod& b) const { return y < b.y || (y == b.y && x < b.x); }
  //vlejd extension, aby sa to spravalo ako vektor
  Bod operator+(const Bod& b) const { return Bod(x+b.x,y+b.y); }
  Bod operator-(const Bod& b) const { return Bod(x-b.x,y-b.y); }
  
  
};


struct Prikaz {
  int smer;   // Smer, kam sa ma hlava pohnut (vid makra a DX DY v update.cpp)
  int pouzi;  // ktory bonus chceme pouzit (jeho poradove cislo, cislovane od 0)
  
  Prikaz() {
      smer = HORE;
      pouzi = NIC;
  }
  
  string toString(){
      char bufer [50];
      sprintf(bufer,"smer:%d\npouzi: %d\n",this->smer,this->pouzi);
      string b (bufer);
      return b;      
  }
  
};


typedef Prikaz Odpoved;


struct Hrac {
  int skore;
  std::vector<int> mapovanie;   // klienti nevidia, tajne prehadzanie
};

struct Jedlo {
    int typ;
    Bod pozicia;  //kde sa nachadza
    int prirastok;    //o kolko urcite narastiete, ked ho spapate
    int zivotnost;    // kolko este vydrzi, potom zmizne
};

struct Snake {
  int ktorehoHraca;   // WARNING vy ste 0
  vector<Bod> telo;   //zoznam bodov, na zaciatku je hlava
  int smer;           //kam sa naposledy pohol
  int dorastie;       // kolko kvol este bude rast (o kolko sa postupne zvatsi
  vector<Jedlo> zasoba; //ake jedlo ma v zasobe
  vector<int> buffy;  // je pod vplybom nejakych bufov
  int zije;         //zije vobec?
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
  std::vector<Hrac> hraci;  //zoznam hracov
  std::vector<Snake> hadi;  //zoznam hadov
  std::vector<Jedlo> jedlo; //zoznam jedal
  Teren teren;   // proste teran, skoro az mapa
  int dalsiId;
  int cas;
};


struct Mapa { //velmi podobne ako stav teren, ale ma aj rozmery a nemeni sa. je to pociatocna mapa
  int pocetHracov; // informacia o tom, kolko prave hracov mame
  int w;//sirka
  int h;//vyska
  Teren pribliznyTeren;   // ako to vyzera na zaciatku
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
  member(smer);
  member(pouzi);
end();

reflection(Hrac);
  member(skore);
  member(mapovanie);
end();

reflection(Snake);
  member(ktorehoHraca);
  member(telo);
  member(smer);
  member(dorastie);
  member(zasoba);
  member(buffy);
  member(zije);
end();


reflection(Jedlo);
  member(typ);
  member(pozicia);
  member(prirastok);
  member(zivotnost);
end();

reflection(Teren);
  member(data);
end();

reflection(Stav);
  member(hraci);
  member(hadi);
  member(jedlo);
  member(teren);
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
