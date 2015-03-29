void coRobiSekac (const Manik &m, Teren &objavenyTeren, Teren &Xray){
    if (stav.cas >= GATHERING_TIME) {
        for (Manik tar: stav.manici) {
            if (tar.ktorehoHraca == 0 && tar.typ == MANIK_BANIK && tar.zelezo + tar.zlato == 0 && abs(tar.x - m.x) + abs(tar.y - m.y) == 1) {
                prikazy.push_back(Prikaz(m.id, PRIKAZ_UTOC, tar.x, tar.y));
            }
        }
    }
    

    cerr << "Sekac no." << sekaccount++ << endl;

    Teren vzdialenost;
    prehladajBfs(objavenyTeren, m.pozicia(), vzdialenost);

//Chcem ?
    for (int d = 0; d < 4; d++) {
        int nx = m.x + DX[d], ny = m.y + DY[d];

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



    if (m.zlato + m.zelezo >= SEKAC_MAX_RESOURCES) {
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


    vector <Bod> mohlobytubytjedlo, hybucesajedlo, exkluzivnejedlo;

    int mbtbjbd = bestdist, hsjbd = bestdist, ejbd = bestdist;

    for (int y = 0; y < mapa.h; y++){
        for (int x = 0; x < mapa.w; x++) {
            if ((objavenyTeren.get(x, y) == MAPA_NEVIEM) && vzdialenost.get(x, y) < mbtbjbd) mbtbjbd = vzdialenost.get(x, y);
        }   
    }

    for (Bod z: mozne_zakladne) {
        if (vzdialenost.get(z) < ejbd ) ejbd = vzdialenost.get(z);
    }


    for (Manik tar: stav.manici) {
        if (tar.ktorehoHraca == 0) continue;
        if (vzdialenost.get(tar.pozicia()) < hsjbd) hsjbd = vzdialenost.get(tar.pozicia());
    }

    cerr << "    Bestdistances - " << " " << mbtbjbd << " " << hsjbd << " " << ejbd << endl;
    cerr << "    Finding points" << endl;


    for (int y = 0; y < mapa.h; y++){
        for (int x = 0; x < mapa.w; x++) {
            if ((objavenyTeren.get(x, y) == MAPA_NEVIEM) && vzdialenost.get(x, y) <= mbtbjbd + SEKAC_PRESNOST) mohlobytubytjedlo.push_back(Bod(x, y));
        }
    }


    for (Bod z: mozne_zakladne) {
        if (vzdialenost.get(z) < ejbd + SEKAC_PRESNOST) exkluzivnejedlo.push_back(z);
    }

    for (Manik tar: stav.manici) {
        if (tar.ktorehoHraca == 0) continue;
        if (vzdialenost.get(tar.pozicia()) < hsjbd + SEKAC_PRESNOST) hybucesajedlo.push_back(tar.pozicia());
    }


    cerr << "    Points : " << " " << mohlobytubytjedlo.size () << " " << hybucesajedlo.size() << " " << exkluzivnejedlo.size() << endl;

    vector <Bod> best = mohlobytubytjedlo;
    bestdist = mbtbjbd * SEKAC_PRIOR_EXPLORE;

    if (bestdist > hsjbd * SEKAC_PRIOR_KILL) {
        best = hybucesajedlo;
        bestdist = hsjbd * SEKAC_PRIOR_KILL;
    }

    if (bestdist > ejbd * SEKAC_PRIOR_BASE_CAPTURE) {
        best = exkluzivnejedlo;
        bestdist = ejbd * SEKAC_PRIOR_BASE_CAPTURE;
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
