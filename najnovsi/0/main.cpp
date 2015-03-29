#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

#include "common.h"
#include "marshal.h"
#include "update.h"

#define GATHERING_TIME 500
#define MAPA_JA -1
#define SIRKA_CHODIEB_X 8
#define SIRKA_CHODIEB_Y 3
#define OFFSET_X 2  //zatial nepouzivane == pouzivame chodby inych
#define OFFSET_Y 5
#define POCET_BANIKOV 35
#define VYROVNAVACI_ROZDIEL_KOVACOV 42 //nie som si isty ze ci to funguje
#define SANCA_KOVAC (rand()%50<1)
#define BANIK_MAX_RESOURCES 30
#define BANIK_PRESNOST 1
#define POMER_MLATICOV 5
#define MLATIC_MAX_RESOURCES 20
#define SEKAC_MAX_RESOURCES 20
#define BANIK_PRIOR_RES 1
#define BANIK_PRIOR_TUNELS 2
#define BANIK_PRIOR_EXPLORE 20 
#define MLATIC_PRIOR_RES 15
#define MLATIC_PRIOR_TUNELS 30
#define MLATIC_PRIOR_EXPLORE 5
#define MLATIC_PRIOR_KILL 3
#define MLATIC_PRIOR_BASE_CAPTURE 1
#define MLATIC_PRESNOST 1
#define POMER_MLATIC_SEKAC false
#define BANIK_FLEEING_RANGE 3
#define SEKAC_PRIOR_EXPLORE 6
#define SEKAC_PRIOR_KILL 4
#define SEKAC_PRIOR_BASE_CAPTURE 2
#define SEKAC_PRESNOST 1
#define MLATIC_FLEEING_RANGE 2

Mapa mapa;
Stav stav;
Teren viditelnyTeren;
Teren objavenyTeren;
vector<Prikaz> prikazy;
vector<Manik> kovacovia;
vector<Bod> mozne_zakladne;


int xoff, yoff, xmod, ymod;


void inicializuj() {
    xmod = SIRKA_CHODIEB_X;
    ymod = SIRKA_CHODIEB_Y;
    xoff = OFFSET_X;
    yoff = OFFSET_Y;

    objavenyTeren.vyprazdni(mapa.w, mapa.h, MAPA_NEVIEM);

    for (int i=0; i<mapa.w; i++) {
        for (int j=0; j<mapa.h; j++){
            if (mapa.pribliznyTeren.get(Bod(i, j)) == MAPA_START) mozne_zakladne.push_back(Bod(i, j));
        }
    }

}
static void chodKuMiestu(const Manik &m, Bod ciel, const Teren &objavenyTeren) {
    Teren vzdialenost;
    prehladajBfs(objavenyTeren, ciel, vzdialenost);

    vector<int> data = {0, 1, 2, 3};
    random_shuffle(data.begin(), data.end());

    for (int d: data) {
        Bod n(m.x + DX[d], m.y + DY[d]);
        if (priechodne(objavenyTeren.get(n)) && vzdialenost.get(n) < vzdialenost.get(m.pozicia())) {
            prikazy.push_back(Prikaz(m.id, PRIKAZ_CHOD, n));

            //if(objavenyTeren.get(n) != MAPA_PASCA) prikazy.push_back(Prikaz(mfid, PRIKAZ_UTOC, n));
            break;
        }
    }
}
int chcesKovaca = -1;
int kovaccount = 0;


bool DFS (int i, int j, Teren &objavenyTeren, bool prepis = false) {
    if (objavenyTeren.get(i, j) == MAPA_NEVIEM) return false;
    if (objavenyTeren.get(i, j) == MAPA_OKRAJ) return true;
    if (objavenyTeren.get(i, j) == MAPA_VOLNO) return true;
    if (objavenyTeren.get(i, j) == MAPA_START) return true;
    if (objavenyTeren.get(i, j) <  MAPA_VOLNO) return true;


    if (objavenyTeren.get(i, j) == MAPA_PASCA) return true;

    int temp = objavenyTeren.get(i, j);
    objavenyTeren.set(i, j, MAPA_PASCA);

    for (int d=0; d<4; d++) {
        if (!DFS(i + DX[d], j+DY[d], objavenyTeren, prepis)) {
            if (!prepis) objavenyTeren.set(i, j, temp);
            return false;
        }
    }

    if (!prepis) objavenyTeren.set(i, j, temp);
    return true;
}
int banikcount = 0;
int mlaticcount = 0;
int sekaccount = 0;

#include "Kovac.cpp"
#include "Banik.cpp"
#include "Mlatic.cpp"
#include "Sekac.cpp"


void zistiTah() {
    cerr << "Turn begining " << stav.cas << endl;

    for (int y = 0; y < mapa.h; y++) for (int x = 0; x < mapa.w; x++) {
            if (viditelnyTeren.get(x, y) != MAPA_NEVIEM) {
                objavenyTeren.set(x, y, viditelnyTeren.get(x, y));
            }
        }

    vector <Bod> nove_zakladne;

    for (Bod z: mozne_zakladne) {
        if (viditelnyTeren.get(z) == MAPA_NEVIEM) {
            nove_zakladne.push_back(z);
            continue;
        }

        bool can = false;

        for (Manik enemy: stav.manici) {
            if (enemy.ktorehoHraca == 0 || enemy.typ != MANIK_KOVAC) continue;
            if (z == enemy.pozicia()) can = true;
        }

        if (can) nove_zakladne.push_back (z);
    }

    mozne_zakladne = nove_zakladne;

    cerr << "Zakladne: ";
    for (auto z: mozne_zakladne) cerr << "("<< z.x << ", " << z.y << ") ";
    cerr << endl;

    banikcount = 0;
    kovaccount = 0;
    mlaticcount = 0;
    sekaccount = 0;

    Teren temp = objavenyTeren;

    Teren dfs;
    dfs.vyprazdni(mapa.w, mapa.h, MAPA_NEVIEM);

    for (int i=0; i<mapa.w; i++) {
        for(int j=0; j<mapa.h; j++) {
            if (DFS (i, j, temp)) {
                DFS (i, j, temp, true);
            }
        }
    }

    kovacovia.clear();


    FOREACH(it, stav.manici) {
        if (it->ktorehoHraca == 0 && it->typ == MANIK_KOVAC) {
            kovacovia.push_back(*it);
        }

        if (it->ktorehoHraca == 0) temp.set (it->x, it->y, MAPA_JA);
    }

    cerr << "Map constructed" << endl;

    /*for (int y = 0; y < mapa.h; y++){
        for (int x = 0; x < mapa.w; x++) {
            cerr << objavenyTeren.get (Bod(x, y)) << " " ;
        }   
        cerr << endl;
    }*/

    FOREACH(it, stav.manici) {
        if (it->ktorehoHraca != 0) continue;
        switch (it->typ) {
        case MANIK_KOVAC:
            coRobiKovac(*it);
            break;

        case MANIK_BANIK:
            coRobiBanik(*it, temp, objavenyTeren);
            break;

        case MANIK_MLATIC:
            coRobiMlatic(*it, temp, objavenyTeren);
            break;

        case MANIK_SEKAC:
            coRobiSekac(*it, temp, objavenyTeren);
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

