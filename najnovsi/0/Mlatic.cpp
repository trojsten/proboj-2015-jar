void coRobiMlatic (const Manik &m, Teren &objavenyTeren, Teren &Xray){
    if (stav.cas >= GATHERING_TIME) {
        for (Manik tar: stav.manici) {
            if (tar.ktorehoHraca == 0 && tar.typ == MANIK_BANIK && tar.zelezo + tar.zlato == 0 && abs(tar.x - m.x) + abs(tar.y - m.y) == 1) {
                prikazy.push_back(Prikaz(m.id, PRIKAZ_UTOC, tar.x, tar.y));
            }
        }
    }
    
    cerr << "Mlatic no." << mlaticcount++ << endl;

    Teren vzdialenost;
    prehladajBfs(objavenyTeren, m.pozicia(), vzdialenost);

//Chcem ?
    for (int d = 0; d < 4; d++) {
        int nx = m.x + DX[d], ny = m.y + DY[d];

        if (objavenyTeren.get(nx, ny) == MAPA_ZLATO || objavenyTeren.get(nx, ny) == MAPA_ZELEZO) {
            cerr << "    Mining" << endl;
            prikazy.push_back(Prikaz(m.id, PRIKAZ_UTOC, nx, ny));
            return;
        }

        for (Manik kovacM: kovacovia){
            Bod kovac = kovacM.pozicia();
            if (Bod(nx, ny) == kovac && m.zlato) {
                cerr << "    Giving gold" << endl;
                prikazy.push_back(Prikaz(m.id, PRIKAZ_DAJ_ZLATO, nx, ny, m.zlato));
                return;
            }
            if (Bod(nx, ny) == kovac && m.zelezo) {
                cerr << "    Giving iron" << endl;
                prikazy.push_back(Prikaz(m.id, PRIKAZ_DAJ_ZELEZO, nx, ny, m.zelezo));
                return;
            }
        }
    }

    bool flee = false;
    for (Manik tar: stav.manici) {
        if (tar.ktorehoHraca == 0) continue;
        if (vzdialenost.get(tar.pozicia()) < MLATIC_FLEEING_RANGE && tar.typ == MANIK_SEKAC) flee = true;
    }



    if (m.zlato + m.zelezo >= MLATIC_MAX_RESOURCES || flee) {
        cerr << "    Trying to go to base" << endl;
        Bod bestp (-1, -1);
        int bestdist = 999999999;

        for (Manik kovacM: kovacovia){
            Bod kovac = kovacM.pozicia();

            if (vzdialenost.get(kovac) < bestdist){
                bestp = kovac;
                bestdist = vzdialenost.get(kovac);
            }
        }

        if (bestp != Bod(-1, -1)) {
            cerr << "    Going to base with resources" << endl;
            chodKuMiestu(m, bestp, objavenyTeren);
            return;
        }
    }

    for (Manik tar: stav.manici) {
        if (tar.ktorehoHraca == 0) continue;
        if (vzdialenost.get(tar.pozicia()) == 1){
            cerr << "Attacking at " << tar.x << " " << tar.y << " from " << m.x << " " << m.y << endl;
            prikazy.push_back (Prikaz(m.id, PRIKAZ_UTOC, tar.pozicia())); 
            return;
        }
    }

    cerr << "    Finding best distances" << endl;

    Bod bestp(-1, -1);
    int bestdist = mapa.w * mapa.h;
    int r = rand()%4;


    vector <Bod> jedlo, zlejedlo, mohlobytubytjedlo, hybucesajedlo, exkluzivnejedlo;

    int jbd = bestdist, zjbd = bestdist, mbtbjbd = bestdist, hsjbd = bestdist, ejbd = bestdist;


    for (int y = 0; y < mapa.h; y++){
        for (int x = 0; x < mapa.w; x++) {

            if ((objavenyTeren.get(x, y) == MAPA_ZLATO || objavenyTeren.get(x, y) == MAPA_ZELEZO) && vzdialenost.get(x, y) < jbd)               jbd = vzdialenost.get(x, y);
            if ((objavenyTeren.get(x, y) == MAPA_SUTER) && vzdialenost.get(x, y) < zjbd && ((x+xoff) % xmod == 0 || (y+yoff) %ymod==0))        zjbd = vzdialenost.get(x, y);
            if ((objavenyTeren.get(x, y) == MAPA_NEVIEM) && vzdialenost.get(x, y) < mbtbjbd && ((x+xoff) % xmod == 0 || (y+yoff) %ymod==0)) mbtbjbd = vzdialenost.get(x, y);
            if ((objavenyTeren.get(x, y) == MAPA_START) && vzdialenost.get(x, y) < ejbd ) ejbd = vzdialenost.get(x, y);

        }
    }

    for (Manik tar: stav.manici) {
        if (tar.ktorehoHraca == 0) continue;
        if (vzdialenost.get(tar.pozicia()) < hsjbd) hsjbd = vzdialenost.get(tar.pozicia());
    }

    cerr << "    Bestdistances - " << jbd << " " << zjbd << " " << mbtbjbd << " " << hsjbd << " " << ejbd << endl;
    cerr << "    Finding points" << endl;


    for (int y = 0; y < mapa.h; y++){
        for (int x = 0; x < mapa.w; x++) {

            if ((objavenyTeren.get(x, y) == MAPA_ZLATO || objavenyTeren.get(x, y) == MAPA_ZELEZO) && vzdialenost.get(x, y) <= jbd+MLATIC_PRESNOST) jedlo.push_back(Bod(x, y));
            if ((objavenyTeren.get(x, y) == MAPA_SUTER) && vzdialenost.get(x, y) <= zjbd+MLATIC_PRESNOST && ((x+xoff) % xmod == 0 || (y+yoff) %ymod==0)) zlejedlo.push_back(Bod(x, y));
            if ((objavenyTeren.get(x, y) == MAPA_NEVIEM) && vzdialenost.get(x, y) <= mbtbjbd+MLATIC_PRESNOST && ((x+xoff) % xmod == 0 || (y+yoff) %ymod==0)) mohlobytubytjedlo.push_back(Bod(x, y));
            if ((objavenyTeren.get(x, y) == MAPA_START) && vzdialenost.get(x, y) <= ejbd + MLATIC_PRESNOST) exkluzivnejedlo.push_back(Bod(x, y));
        }
    }

    for (Manik tar: stav.manici) {
        if (tar.ktorehoHraca == 0) continue;
        if (vzdialenost.get(tar.pozicia()) < hsjbd + MLATIC_PRESNOST) hybucesajedlo.push_back(tar.pozicia());
    }


    cerr << "    Points : " << jedlo.size() << " " << zlejedlo.size() << " " << mohlobytubytjedlo.size () << " "<< hybucesajedlo.size() << " " << exkluzivnejedlo.size()<< endl;

    vector <Bod> best = jedlo;
    bestdist = jbd * MLATIC_PRIOR_RES;

    if (bestdist > zjbd * MLATIC_PRIOR_TUNELS) {
        best = zlejedlo;
        bestdist = zjbd * MLATIC_PRIOR_TUNELS;
    }

    if (bestdist > mbtbjbd * MLATIC_PRIOR_EXPLORE) {
        best = mohlobytubytjedlo;
        bestdist = mbtbjbd * MLATIC_PRIOR_EXPLORE;
    }

    if (bestdist > hsjbd * MLATIC_PRIOR_KILL) {
        best = hybucesajedlo;
        bestdist = hsjbd * MLATIC_PRIOR_KILL;
    }

    if (bestdist > ejbd * MLATIC_PRIOR_BASE_CAPTURE) {
        best = exkluzivnejedlo;
        bestdist = ejbd * MLATIC_PRIOR_BASE_CAPTURE;
    }

    if (best.size() > 0) bestp = best[rand()%best.size()];

    cerr << "    Going to " << bestp.x << " "<< bestp.y << endl;

    if (bestp.x != -1) {
        if (abs(bestp.x - m.x) + abs(bestp.y - m.y) == 1) {
            prikazy.push_back(Prikaz(m.id, PRIKAZ_UTOC, bestp));
            return;
        }

        chodKuMiestu(m, bestp, objavenyTeren);
        return;
    }


    for (Manik tar: stav.manici) {
        if (tar.ktorehoHraca == 0 && tar.typ == MANIK_BANIK && tar.zelezo + tar.zlato == 0 && abs(tar.x - m.x) + abs(tar.y - m.y) == 1) {
            prikazy.push_back(Prikaz(m.id, PRIKAZ_UTOC, tar.x, tar.y));
        }
    }


    if (bestp != Bod(-1, -1)) {
        cerr << "    Going to Commanders" << endl;
        chodKuMiestu(m, bestp, Xray);
        return;
    }
}
