
#ifndef UPDATE_H
#define UPDATE_H

#include <ostream>
#include <map>

#include "common.h"


extern const int DX[4];
extern const int DY[4];

extern const int kSkoreFrag;
extern const int kCasNaNarast;
extern const int kZaciatocnaVelkost;

extern const int kMaximalnaDlzkaHry;


void zapniObservation(std::ostream* observation);

Stav zaciatokHry(const Mapa& mapa);
//void prehladajBfs(const Teren& teren, Bod start, Teren& vzdialenost);
//void prehladajLokalneBfs(const Teren& teren, Bod start, int rozsahLimit, std::map<Bod,int>& vzdialenost);
void odsimulujKolo(const Mapa& mapa, Stav& stav, const std::vector<Odpoved>& akcie);
//void zistiCoVidi(const Stav& stav, int hrac, Teren& viditelne);
void zamaskujStav(const Mapa& mapa, const Stav& stav, int hrac, Stav& novy);
void odmaskujOdpoved(const Mapa& mapa, const Stav& stav, int hrac, Odpoved& odpoved);
//void zakodujViditelnyTeren(const Teren &vstup, vector<int> &vystup);
//void dekodujViditelnyTeren(const vector<int> &vstup, Teren &vystup);
std::vector<int> ktoriZiju(const Mapa& mapa, const Stav& stav);
bool hraSkoncila(const Mapa& mapa, const Stav& stav);
int zistiSkore(const Stav& stav, int hrac);

#endif
