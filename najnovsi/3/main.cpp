#include <cstdio>
#include<ctime>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <iostream>
#include <vector>
#include<queue>
#include<set>
using namespace std;

#define inf 1023456789
#define linf 1023456789123456789ll;
#define db(x) cerr << #x << " = " << x << endl

#include "magicke_konstanty.h"
#include "common.h"
#include "marshal.h"
#include "update.h"

#define MOJ_TYP_UNEXPLORED 8
#define OFFSET_HMLA 9
#define OFFSET_JEDNOTKA 17
#define OFFSET_MOJA_JEDNOTKA 9

struct meta_manik
{
	int log_level;
	Manik ja;
	
	meta_manik()
	{
	}
	
	void updt(Manik ty)
	{
		ja = ty;
	}
	
	virtual void konaj() = 0;
};


const int dX[4] = {0,1,0,-1}, dY[4] = {-1,0,1,0};

Mapa mapa;
Stav stav;   // vzdy som hrac cislo 0
Teren viditelnyTeren;
vector<Prikaz> prikazy;

int width, height;
bool nainicializovane;
vector<vector<bool> > explored;
vector<vector<int> > aky_typ_tu_je, co_tu_bolo_na_posledy, kto_tu_je;
map<int, meta_manik*> moj_manik;
map<int,Bod> kde_som;
vector<vector<int> > ku_zelezu, ku_zlatu, ku_nepriatelom, ku_hmle, ku_mojim_vojakom, ku_mojim_banikom, ku_staremu_zelezu, ku_staremu_zlatu, ku_mojmu_kovacovi;
vector<vector<vector<int> > > do_neznama, do_neznama_b, do_steny, ku_logistom_a, ku_logistom_b;
int vidim_nepriatelov, vidim_nasich, vidim_banikov;

vector<int> permutation(int poc)
{
	vector<int> res(poc);
	for(int i=0; i<poc; i++)res[i] = i;
	for(int i=0; i<poc; i++)
	{
		int j = (rand()%(poc-i))+i;
		swap(res[i],res[j]);
	}
	return res;
}


int ziskaj_typ(int x,int y)
{
	if(x < 0 || y < 0 || x >= width || y >= height)return MAPA_OKRAJ;
	if(!explored[y][x])return MOJ_TYP_UNEXPLORED;
	if(viditelnyTeren.get(x,y) == MAPA_NEVIEM)return OFFSET_HMLA + co_tu_bolo_na_posledy[y][x];
	if(aky_typ_tu_je[y][x] != -1)return OFFSET_JEDNOTKA + aky_typ_tu_je[y][x];
	return viditelnyTeren.get(x,y);
}

struct meta_agresor : meta_manik
{
	meta_agresor(Manik ty, int lol)
	{
		ja = ty;
		log_level = lol;
	}
	
	void konaj()
	{
		vector<int> perm = permutation(4);
		for(int i = 0; i<4; i++)
		{
			int nx = ja.x + dX[perm[i]], ny = ja.y + dY[perm[i]];
			if(ziskaj_typ(nx, ny) >= 17 && ziskaj_typ(nx,ny) < 26)
			{
				prikazy.push_back(Prikaz(ja.id,PRIKAZ_UTOC,nx,ny));
				return;
			}
		}
		int naj_log = log_level, best = -1;
		for(int i=0; i<4; i++)
		{
			int nx = ja.x + dX[perm[i]], ny = ja.y + dY[perm[i]];
			if(ziskaj_typ(nx,ny) >= 26)
			{
				if(kto_tu_je[ny][nx] >= 0)
				{
					if(moj_manik[kto_tu_je[ny][nx]] -> log_level < naj_log)
					{
						naj_log = moj_manik[kto_tu_je[ny][nx]]->log_level;
						best = perm[i];
					}
				}
			}
		}
		if(best != -1)
		{
			if(ja.zelezo > ja.zlato)
			{
				prikazy.push_back(Prikaz(ja.id,PRIKAZ_DAJ_ZELEZO, ja.x + dX[best], ja.y+dY[best], ja.zelezo));
				return;
			}
			if(ja.zlato >= ja.zelezo && ja.zlato > 0)
			{
				prikazy.push_back(Prikaz(ja.id, PRIKAZ_DAJ_ZLATO, ja.x + dX[best], ja.y + dY[best], ja.zlato));
				return;
			}
		}
		
		best = -1;
		long long najbl = linf;
		for(int i=0; i<4; i++)
		{
			int nx = ja.x + dX[perm[i]], ny = ja.y + dY[perm[i]];
			if(ziskaj_typ(nx,ny) != MAPA_OKRAJ)
			{
				long long dist = linf;
				for(int j=0; j<4; j++)
				{
					dist = min(dist,(long long)(do_neznama[j][ny][nx] + hendikep[j]*agresor_exploruje));
				}
				
				dist = min(dist, (long long)(ku_nepriatelom[ny][nx]));
				for(int j=0; j<log_level; j++)
				{
					if(cena_zlata*ja.zlato + cena_zeleza*ja.zelezo >= chcem_aspon((int)(ku_logistom_b.size()) - j))
					{
						dist = min(dist, (long long)(ku_logistom_b[j][ny][nx]));
					}
				}
				dist += ku_hmle[ny][nx]*zvedavost;
				if(dist < najbl)
				{
					najbl = dist;
					best = perm[i];
				}
			}
		}
		prikazy.push_back(Prikaz(ja.id,PRIKAZ_CHOD,ja.x + dX[best], ja.y + dY[best]));
		return;
	}
};

struct meta_kovac : meta_manik
{
	meta_kovac(Manik ty)
	{
		ja = ty;
		log_level = 0;
	}
	
	void konaj()
	{
		vector<int> perm = permutation(4);
		for(int i=0; i<4; i++)
		{
			int nx = ja.x + dX[perm[i]], ny = ja.y + dY[perm[i]];
			if(ziskaj_typ(nx,ny) == MAPA_VOLNO)
			{
				if(rand()%(vidim_nasich + vidim_nepriatelov*celkova_agresivita) < vidim_nasich && vidim_banikov < 30)prikazy.push_back(Prikaz(ja.id, PRIKAZ_KUJ, ja.x+1, ja.y, MANIK_BANIK));
				else 
				{
					if(rand()%2 == 0)prikazy.push_back(Prikaz(ja.id, PRIKAZ_KUJ, ja.x, ja.y+1, MANIK_SEKAC));
					else prikazy.push_back(Prikaz(ja.id, PRIKAZ_KUJ, ja.x, ja.y+1, MANIK_MLATIC));
				}
				return;
			}
		}
	}
};

struct meta_ochranca : meta_manik
{
	meta_ochranca(Manik ty, int lol)
	{
		ja = ty;
		log_level = lol;
	}
	
	void konaj()
	{
		
	}
};

struct meta_banik : meta_manik
{
	meta_banik(Manik ty, int lol)
	{
		ja = ty;
		log_level = lol;
	}
	
	void konaj()
	{
		vector<int> perm = permutation(4);
		int naj_log = log_level, best = -1;
		for(int i=0; i<4; i++)
		{
			int nx = ja.x + dX[perm[i]], ny = ja.y + dY[perm[i]];
			if(ziskaj_typ(nx,ny) >= 26)
			{
				if(kto_tu_je[ny][nx] >= 0)
				{
					if(moj_manik[kto_tu_je[ny][nx]] -> log_level < naj_log)
					{
						naj_log = moj_manik[kto_tu_je[ny][nx]]->log_level;
						best = perm[i];
					}
				}
			}
		}
		if(best != -1)
		{
			if(ja.zelezo > ja.zlato)
			{
				prikazy.push_back(Prikaz(ja.id,PRIKAZ_DAJ_ZELEZO, ja.x + dX[best], ja.y+dY[best], ja.zelezo));
				return;
			}
			if(ja.zlato >= ja.zelezo && ja.zlato > 0)
			{
				prikazy.push_back(Prikaz(ja.id, PRIKAZ_DAJ_ZLATO, ja.x + dX[best], ja.y + dY[best], ja.zlato));
				return;
			}
		}
		
		best = -1;
		long long najbl = linf;
		for(int i=0; i<4; i++)
		{
			int nx = ja.x + dX[perm[i]], ny = ja.y + dY[perm[i]];
			if(ziskaj_typ(nx,ny) != MAPA_OKRAJ)
			{
				long long dist = linf;
				for(int j=0; j<4; j++)
				{
					dist = min(dist, (long long)(do_steny[j][ny][nx] + hendikep[j]));
				}
				dist = min(dist,(long long)(min(ku_zlatu[ny][nx], ku_zelezu[ny][nx])));
				dist = min(dist, (long long)(zastarany_hendikep + min(ku_staremu_zelezu[ny][nx], ku_staremu_zlatu[ny][nx])));
				
				for(int j=0; j<log_level; j++)
				{
					if(cena_zlata*ja.zlato + cena_zeleza*ja.zelezo >= chcem_aspon((int)(ku_logistom_b.size()) - j))
					{
						dist = min(dist, (long long)(ku_logistom_b[j][ny][nx] + logisticky_bonus));
					}
				}
				
				dist += banikov_movement[ziskaj_typ(nx,ny)];
				//dist -= hmlofobia * ku_hmle[ny][nx];
				//dist -= xenofobia * ku_nepriatelom[ny][nx];
				//dist += paranoja * ku_mojim_vojakom[ny][nx];
				
				if(dist < najbl)
				{
					najbl = dist;
					best = perm[i];
				}
			}
		}
		int nx=  ja.x + dX[best];
		int ny = ja.y + dY[best];
		
		if(ziskaj_typ(nx,ny) == MAPA_ZELEZO || ziskaj_typ(nx,ny) == MAPA_ZLATO || ziskaj_typ(nx,ny) == MAPA_SUTER)
		{
			prikazy.push_back(Prikaz(ja.id, PRIKAZ_UTOC, nx,ny));
			return;
		}
		prikazy.push_back(Prikaz(ja.id, PRIKAZ_CHOD, ja.x + dX[best], ja.y + dY[best]));
		return;
	}
};



void dijkstra(vector<vector<int> >& vzd, vector<Bod> start, int* movement_cost)
{
	vzd = vector<vector<int> > (height, vector<int> (width,inf));
	priority_queue<pair<int, Bod>, vector<pair<int, Bod> >, greater<pair<int, Bod> > > halda;
	for(int i=0; i<(int)(start.size()); i++)
	{
		vzd[start[i].y][start[i].x] = 0;
		halda.push(make_pair(0, start[i]));
	}
	while(!halda.empty())
	{
		pair<int,Bod> pom = halda.top();
		halda.pop();
		Bod ja = pom.second;
		if(vzd[ja.y][ja.x] < pom.first)continue;
		int cost = (vzd[ja.y][ja.x] == 0) ? 1 : movement_cost[ziskaj_typ(ja.x,ja.y)];
		for(int i=0; i<4; i++)
		{
			int nx = ja.x+dX[i], ny = ja.y+dY[i];
			if(nx < 0 || nx >= width || ny < 0 || ny >= height)continue;
			if(vzd[ny][nx] > vzd[ja.y][ja.x] + cost)
			{
				vzd[ny][nx] = vzd[ja.y][ja.x] + cost;
				halda.push(make_pair(vzd[ny][nx], Bod(nx,ny)));
			}
		}
	}
}


struct Koordinator
{
	vector<int> v_hlbke;
	
	Koordinator()
	{
		do_neznama.resize(5);
		do_steny.resize(5);
		do_neznama_b.resize(5);
	}
	
	void inicializuj()
	{
		 updatni_uvarene_info();
	}
	
	meta_manik* co_vyrobit(Manik manik)
	{
		if(manik.typ == MANIK_KOVAC)
		{
			if(v_hlbke.size() > 0)v_hlbke[0]++;
			else v_hlbke.push_back(1);
			return new meta_kovac(manik);
		}
		int log_hl = 1;
		for(log_hl = 1; ((int)(v_hlbke.size()) > log_hl ? v_hlbke[log_hl]:0) >= v_hlbke[log_hl-1]*zaklad_logistickeho_stromu; log_hl++);   // TY SI CHUDAK MAS TU BUG --Tomi
		if(log_hl >= (int)(v_hlbke.size()))v_hlbke.push_back(1);
		else v_hlbke[log_hl]++;
		if(manik.typ == MANIK_STRAZNIK || manik.typ == MANIK_STRELEC)
		{
			return new meta_ochranca(manik, log_hl);
		}
		if(manik.typ == MANIK_SEKAC || manik.typ == MANIK_MLATIC || manik.typ == MANIK_SKAUT || manik.typ == MANIK_LOVEC)
		{
			db("ag");
			return new meta_agresor(manik, log_hl);
		}
		if(manik.typ == MANIK_BANIK)return new meta_banik(manik, log_hl);
		db("crap");
		return new meta_banik(manik, log_hl);
	}
	
	void updatni_uvarene_info()
	{
		vector<Bod> ferum, aurum, stare_ferum, stare_aurum, enemy, fog, ally, banik, kovac;
		vector<vector<Bod> > neznamo(5), stena(5), logistom(v_hlbke.size());
		vector<vector<int> > kolko_objavi(height, vector<int> (width,0));
		for(int y=0; y<height; y++)
		{
			for(int x=0; x<width; x++)
			{
				int typ = ziskaj_typ(x,y);
				if(typ == MAPA_ZELEZO)ferum.push_back(Bod(x,y));
				if(typ == MAPA_ZLATO)aurum.push_back(Bod(x,y));
				if(typ == MOJ_TYP_UNEXPLORED + MAPA_ZELEZO + 1)stare_ferum.push_back(Bod(x,y));
				if(typ == MOJ_TYP_UNEXPLORED + MAPA_ZLATO + 1)stare_aurum.push_back(Bod(x,y));
				if(typ >= 17 && typ < 26)enemy.push_back(Bod(x,y));
				if(typ >= 9 && typ < 17)fog.push_back(Bod(x,y));
				if(typ == 26)banik.push_back(Bod(x,y));
				if(typ >= 27 && typ < 33)ally.push_back(Bod(x,y));
				if(typ == 33)kovac.push_back(Bod(x,y));
				if(kto_tu_je[y][x] >= 0)logistom[moj_manik[kto_tu_je[y][x]]->log_level].push_back(Bod(x,y));
				
				if(typ == 8)
				{
					for(int i=0;i<4; i++)
					{
						int nx = x+dX[i], ny = y+dY[i];
						if(ziskaj_typ(nx,ny) != MAPA_OKRAJ)
						{
							kolko_objavi[ny][nx]++;
						}
					}
				}
			}
		}
		for(int y=0; y<height; y++)
		{
			for(int x=0; x<width; x++)
			{
				int typ = ziskaj_typ(x,y);
				if(banikov_movement[typ] < inf && typ < 17)stena[kolko_objavi[y][x]].push_back(Bod(x,y));
				if(agresivny_movement[typ] < inf && typ < 17)neznamo[kolko_objavi[y][x]].push_back(Bod(x,y));
			}
		}
		dijkstra(ku_zelezu, ferum, banikov_movement);
		dijkstra(ku_zlatu, aurum, banikov_movement);
		dijkstra(ku_staremu_zelezu, stare_ferum, banikov_movement);
		dijkstra(ku_staremu_zlatu, stare_aurum, banikov_movement);
		dijkstra(ku_nepriatelom, enemy, agresivny_movement);
		dijkstra(ku_hmle, fog, agresivny_movement);
		dijkstra(ku_mojim_banikom, banik, agresivny_movement);
		dijkstra(ku_mojim_vojakom, ally, banikov_movement);
		dijkstra(ku_mojmu_kovacovi, kovac, agresivny_movement);
		
		for(int i=0; i<4; i++)
		{
			dijkstra(do_neznama[i], neznamo[i], agresivny_movement);
			dijkstra(do_neznama_b[i], neznamo[i], banikov_movement);
			dijkstra(do_steny[i], stena[i], banikov_movement);
		}
		
		ku_logistom_a.resize(v_hlbke.size());
		ku_logistom_b.resize(v_hlbke.size());
		for(int i=0; i<(int)ku_logistom_a.size(); i++)
		{
			dijkstra(ku_logistom_a[i], logistom[i], agresivny_movement);
			dijkstra(ku_logistom_b[i], logistom[i], banikov_movement);
		}
	}
};

Koordinator koordinator_diktator;

void updatni_surove_info()
{
	for(int y=0; y<height; y++)
	{
		for(int x=0; x<width; x++)
		{
			int ter = viditelnyTeren.get(x,y);
			if(ter != MAPA_NEVIEM)
			{
				explored[y][x] = 1;
				co_tu_bolo_na_posledy[y][x] = ter;
			}
			aky_typ_tu_je[y][x] = -1;
			kto_tu_je[y][x] = -1;
		}
	}
	vidim_nasich = 0;
	vidim_nepriatelov = 0;
	vidim_banikov = 0;
	for(int i=0; i<(int)(stav.manici.size()); i++)
	{
		Manik manik = stav.manici[i];
		if(manik.ktorehoHraca == 0)
		{
			vidim_nasich++;
			if(manik.typ == MANIK_BANIK)vidim_banikov++;
			aky_typ_tu_je[manik.y][manik.x] = OFFSET_MOJA_JEDNOTKA + manik.typ;
			kto_tu_je[manik.y][manik.x] = manik.id;
			kde_som[manik.id] = manik.pozicia();
			if(moj_manik.count(manik.id) < 1)
			{
				moj_manik[manik.id] = koordinator_diktator.co_vyrobit(manik);
			}
			moj_manik[manik.id]->updt(manik);
		}
		else
		{
			vidim_nepriatelov++;
			aky_typ_tu_je[manik.y][manik.x] = manik.typ;
			kto_tu_je[manik.y][manik.x] = -1;
		}
	}
}


void nainicializuj()
{
	width = viditelnyTeren.w();
	height = viditelnyTeren.h();
	explored = vector<vector<bool> > (height, vector<bool> (width,0));
	aky_typ_tu_je = vector<vector<int> > (height, vector<int> (width,-1));
	kto_tu_je = vector<vector<int> > (height, vector<int> (width, -1));
	co_tu_bolo_na_posledy = vector<vector<int> > (height, vector<int> (width,-1));
	db(clock());
	updatni_surove_info();
	db(clock());
	koordinator_diktator.inicializuj();
	db(clock());
	nainicializovane = 1; 
}

void inicializuj() 
{
	nainicializovane = 0;
}



void zistiTah() 
{
	if(!nainicializovane)
	{
		nainicializuj();
	}
	db("a");
	updatni_surove_info();
	db("b");
	koordinator_diktator.updatni_uvarene_info();
	db("c");
	for(int i=0; i<(int)(stav.manici.size()); i++)
	{
		db(i);
		Manik manik = stav.manici[i];
		if(manik.ktorehoHraca == 0)
		{
			moj_manik[manik.id]->konaj();
			db(ziskaj_typ(manik.x,manik.y));
			db(moj_manik[manik.id]->log_level);
		}
	}
	
}


int main() 
{
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

