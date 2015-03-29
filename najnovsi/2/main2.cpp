#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <queue>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <functional>
#define For(i,N) for(int i=0; i<N; i++)
using namespace std;

#include "common.h"
#include "marshal.h"
#include "update.h"

#define ULOHA_KOPAC 0
#define ULOHA_TUNELAR 1
#define ULOHA_UTOCNIK 2
#define ULOHA_OBRANCA 3
#define ULOHA_KOVAC 4

Mapa mapa;
Stav stav;   // vzdy som hrac cislo 0
Teren viditelnyTeren;

const int inf = 1000000000;

struct Typek
{
	Typek(Manik& _manik, int _uloha) : manik(_manik), uloha(_uloha) {}
	Manik& manik;
	int uloha;
	int stav;
	Bod target;
	Prikaz prikaz;
	int prioritaPrikazu;
	bool zije;
};

struct Policko
{
	Policko(int _typ=MAPA_NEVIEM, Manik _manik=NULL) : typ(_typ), manik(_manik) {}
	int typ;
	Manik* manik;
};

vector<Prikaz> prikazy;

Teren znamyTeren;
vector<vector<int>> idMapa;
unordered_map<int, Typek> nasiTypci;
unordered_map<int, Manik> nasiManici;
vector<Typek> nasiMrtvi;

Bod spawn;

vector<pair<Bod, int> > starty;		//- // 0 - nic, 1 - ide, 2 - skoncil
int chceme_postavit=-1;
vector<int> armada(9,0);

int tah = 0;

Policko policko(Bod n)
{
	return znamyTeren[n.x][n.y];
}

int mapaIdManika(Manik manik)
{
	return
}

void update_znamyTeren(){

	nasiMrtvi.clear();
	ichManici.clear();
	nasiManici.clear();
	armada.resize(9,0);

	// updatujeme veci, ktore vidime
	For(i,mapa.h) For(j,mapa.w) 
		if(viditelnyTeren.get(j,i) != MAPA_NEVIEM) 
			znamyTeren.set(j,i,viditelnyTeren.get(j,i));
	//updatujeme jednotky
	for(auto& typek : nasiTypci)
		typek.second.zije = false;
	for(auto& manik : stav.manici)
	{
		if(manik.ktorehoHraca == 0)
		{
			int id = manik.id;
			armada[manik.typ]++;
			if(nasiManici.count(id))
				nasiManici[id].zije = true;
			else
				nasiManici[id] = Typek(manik, 0); //TODO nastavit ulohu
		}
		idMapa[manik.x][manik.y] = manik.id;
	}
	// mrtvi typci
	for(auto& typek : nasiTypci)
		if(!typek.zije)
		{
			typci.erase(typek);
			nasiMrtvi.push_back(typek);
		}

	//najdeme kovaca
	for(auto& it : nasiManici)  
		if(it.typ == MANIK_KOVAC) {spawn.x = it.x; spawn.y = it.y;}
}

void bfs(const Teren& teren, Bod start, Teren& vzdialenost, function<int(int)> cenaPrechodu)
{
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
			int prechod = cenaPrechodu(teren.get(n))
				vzdialenost.set(n, vzdialenost.get(p) + abs(prechod));
			if (prechod <= 0) Q.push(n);
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

void coRobiKovac(const Manik &m, int typ) {
	// hlupy klient proste furt stavia banikov kolko moze...
	For(d,4)
	{
		Bod n(m.x + DX[d], m.y + DY[d]);
		if(znamyTeren.get(n) < 10){
			prikazy.push_back(Prikaz(m.id, PRIKAZ_KUJ, n , typ));
			return;
		}
	}
}

int inicializujTunelara(){	//posle tunelara k najblizsiemu zijucemu nepriatelovi
	//najdeme dvojicu <manik, start> s najmensou tunelarskou vzdialenostou, nastavime true, ze sme pustili
	//aby sme neutocili na seba - zavisle na kovacovi
	Manik* tunelar;
	int minVzdialenost = mapa.w * mapa.h;
	pair<Bod, int>* start;
	for(auto& bod : starty)
		if(bod.first.x != kovacX && bod.first.y != kovacY && bod.second==0)
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

	For(i,4){
		Bod n(m.x+DX[i], m.y+DY[i]);
		if(znamyTeren.get(n) >= 20){
			prikazy.push_back(Prikaz(m.id, PRIKAZ_UTOC, n));
			wasFight = true;
			return;
		}
	}
	{
		map<Bod, int> vzdialenost;
		const int R = 10;
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
				if(vzdialenost2.get(n) < vzdialenost2.get(m.pozicia()) )
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
				if(vzdialenost3.get(n) < vzdialenost3.get(m.pozicia()) )
				{
					wasFight = true;
					prikazy.push_back(Prikaz(m.id, PRIKAZ_CHOD, n));
					return;
				}
			}
		}
	}
	Bod ciel(-1,-1);
	for(auto i : starty) if(i.second == 2 && i.first != Bod(kovacX,kovacY)) ciel = i.first;
	if(znamyTeren.get(ciel) >= 10 && znamyTeren.get(ciel) < 20){
		for(auto &i : starty) if(i.first == ciel){ i.first = Bod(kovacX, kovacY); i.second = 0;} 
		wasFight = true;
		return;
	}
	if(ciel.x == -1){
		/*Bod n(m.x+DX[rand()%4], m.y + DY[rand()%4]);
		  if(priechodne(znamyTeren.get(n)))prikazy.push_back(Prikaz(m.id, PRIKAZ_CHOD,n ));
		  return;*/
		ciel = starty[targetStart].first;
	}
	Teren vzdialenost; 
	znamyTeren.set(m.x,m.y,MAPA_VOLNO);
	prehladajBfs(znamyTeren,ciel,vzdialenost);
	znamyTeren.set(m.x,m.y,m.typ+10);
	for(auto i : starty) if(i.second == 2 && i.first != Bod(kovacX,kovacY)) ciel = i.first;
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

void coRobiObrannyVojak(const Manik &m){
	For(i,4){
		Bod n(m.x+DX[i], m.y+DY[i]);
		if(znamyTeren.get(n) >= 20){
			prikazy.push_back(Prikaz(m.id, PRIKAZ_UTOC, n));
			return;
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
			if(vzdialenost2.get(n) < vzdialenost2.get(m.pozicia()) )
			{
				prikazy.push_back(Prikaz(m.id, PRIKAZ_CHOD, n));
				return;
			}
		}
		Teren vzdialenost3;
		prehladajBfs(znamyTeren, kam, vzdialenost3);
		for (int d = 0; d < 4; d++) {
			Bod n(m.x + DX[d], m.y + DY[d]);
			if(vzdialenost3.get(n) < vzdialenost3.get(m.pozicia()) )
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

void coRobiBanik(const Manik &m) {
	for(auto& it: nasiTunelari) if (it.first == m.id) {coRobiTunelar(m); return;}
	For(i,4){
		Bod n(m.x+DX[i], m.y+DY[i]);
		if(znamyTeren.get(n) >= 20){
			prikazy.push_back(Prikaz(m.id, PRIKAZ_UTOC, n));
			return;
		}
	}
	{
		map<Bod, int> vzdialenost;
		const int R = 10;
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
				if(vzdialenost2.get(n) < vzdialenost2.get(m.pozicia()) )
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
				if(vzdialenost3.get(n) < vzdialenost3.get(m.pozicia()) )
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
	if ((m.zlato >= 30 || m.zelezo >= 30) && kovacX != -1) {
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
		if (znamyTeren.get(x, y) == MAPA_SUTER && vzdialenost.get(x, y) < bestdistSuter && (x%10<1 || y%3<1 || abs(x-kovacX) + abs(y-kovacY) < 10)) {
			bestpSuter = Bod(x, y); bestdistSuter = vzdialenost.get(x, y);
		}
	}

	if(bestdistKov == mapa.w * mapa.h * 2 && bestdistSuter == mapa.w * mapa.h * 2 && tah > 50)
		dotazene++;
	else
		dotazene = 0;

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
	Bod n(m.x+DX[rand()%4], m.y + DY[rand()%4]);
	if(abs(m.x-n.x) + abs(m.y-n.y) < 10 && priechodne(znamyTeren.get(n)))
		prikazy.push_back(Prikaz(m.id, PRIKAZ_CHOD, n));

}

// main() zavola tuto funkciu, ked nacita mapu
void inicializuj() {
	srandom(time(NULL));
	znamyTeren.resize(mapa.w);
	for(auto& stlpec : znamyTeren)
		stlpec.resize(mapa.h);
	//najdeme starty hracov pre tunelarov
	For(x, mapa.w)
		For(y, mapa.h)
		{
			if(mapa.pribliznyTeren.get(x, y) == MAPA_START)
			{
				Bod b(x, y);
				starty.push_back(pair<Bod,int>(b, 0));
			}
		}
}


// main() zavola tuto funkciu, ked chce vediet, ake prikazy chceme vykonat,
// co tato funkcia rozhodne pomocou: prikazy.push_back(Prikaz(...));
void zistiTah() {
	update_znamyTeren();
	int na_kolko_nepriatelov_mozeme_utocit=0;
	int kolko_nepriatelov_tunelujeme = 0;
	for(auto i : starty) 
		if(i.first != Bod(kovacX,kovacY) && i.second == 2) na_kolko_nepriatelov_mozeme_utocit++;
		else if(i.first != Bod(kovacX,kovacY) && i.second == 1) kolko_nepriatelov_tunelujeme++;
	if(tah > 20 && kolko_nepriatelov_tunelujeme==0)
		inicializujTunelara();
	//fprintf(stderr,"%d %d\n",kolko_nepriatelov_tunelujeme,na_kolko_nepriatelov_mozeme_utocit);
	tah++;
	fprintf(stderr,"%d\n",armada[MANIK_BANIK]);
	if(chceme_postavit == -1){
		if(tah > 30 && rand()%5==0 && armada[MANIK_STRAZNIK] < 20) chceme_postavit = MANIK_STRAZNIK;
		else if(na_kolko_nepriatelov_mozeme_utocit > 0 && rand()%2==0) chceme_postavit = MANIK_SEKAC; // mozno otocit naspat
		else if (armada[MANIK_BANIK] < 40)chceme_postavit = MANIK_BANIK;
	}

	for(auto& typek : nasiTypci){
		switch(typek.uloha) {
			case ULOHA_KOVAC:
				coRobiKovac(typek);
				break;

			case ULOHA_KOPAC:
				coRobiBanik(typek);

			case ULOHA_OBRANCA:
				coRobiObrannyVojak(typek);
				break;

			case ULOHA_UTOCNIK:	//utocime
				coRobiUtocnyVojak(typek);
				break;

			case ULOHA_TUNELAR:
				coRobiTunelar(typek);
				break;
		}
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

