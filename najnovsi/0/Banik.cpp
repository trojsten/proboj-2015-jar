void coRobiBanik(const Manik &m, Teren &objavenyTeren, Teren &Xray) {
    cerr << "Digger no." << banikcount << endl;
    banikcount ++;


    Teren vzdialenost;
    prehladajBfs(objavenyTeren, m.pozicia(), vzdialenost);


    if (stav.cas < GATHERING_TIME) {
        cerr << "    Not suicidal at the moment" << endl;
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

            if (vzdialenost.get(tar.pozicia()) < BANIK_FLEEING_RANGE) flee = true;
        }

        if (m.zlato + m.zelezo >= BANIK_MAX_RESOURCES || flee) {
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

        cerr << "    Finding best distances" << endl;

        Bod bestp(-1, -1);
        int bestdist = mapa.w * mapa.h;
        int r = rand()%4;


        vector <Bod> jedlo, zlejedlo, mohlobytubytjedlo;
        int jbd = bestdist, zjbd = bestdist, mbtbjbd = bestdist;


        for (int y = 0; y < mapa.h; y++){
            for (int x = 0; x < mapa.w; x++) {

                if ((objavenyTeren.get(x, y) == MAPA_ZLATO || objavenyTeren.get(x, y) == MAPA_ZELEZO) && vzdialenost.get(x, y) < jbd)               jbd = vzdialenost.get(x, y);
                if ((objavenyTeren.get(x, y) == MAPA_SUTER) && vzdialenost.get(x, y) < zjbd && ((x+xoff) % xmod == 0 || (y+yoff) %ymod==0))        zjbd = vzdialenost.get(x, y);
                if ((objavenyTeren.get(x, y) == MAPA_NEVIEM) && vzdialenost.get(x, y) < mbtbjbd && ((x+xoff) % xmod == 0 || (y+yoff) %ymod==0)) mbtbjbd = vzdialenost.get(x, y);

            }
        }

        cerr << "    Bestdistances - " << jbd << " " << zjbd << " " << mbtbjbd << endl;
        cerr << "    Finding points" << endl;


        for (int y = 0; y < mapa.h; y++){
            for (int x = 0; x < mapa.w; x++) {

                if ((objavenyTeren.get(x, y) == MAPA_ZLATO || objavenyTeren.get(x, y) == MAPA_ZELEZO) && vzdialenost.get(x, y) <= jbd+BANIK_PRESNOST) jedlo.push_back(Bod(x, y));
                if ((objavenyTeren.get(x, y) == MAPA_SUTER) && vzdialenost.get(x, y) <= zjbd+BANIK_PRESNOST && ((x+xoff) % xmod == 0 || (y+yoff) %ymod==0)) zlejedlo.push_back(Bod(x, y));
                if ((objavenyTeren.get(x, y) == MAPA_NEVIEM) && vzdialenost.get(x, y) <= mbtbjbd+BANIK_PRESNOST && ((x+xoff) % xmod == 0 || (y+yoff) %ymod==0)) mohlobytubytjedlo.push_back(Bod(x, y));

            }
        }

        cerr << "    Points : " << jedlo.size() << " " << zlejedlo.size() << " " << mohlobytubytjedlo.size () << endl;

        vector <Bod> best = jedlo;
        bestdist = jbd * BANIK_PRIOR_RES;

        if (bestdist > zjbd * BANIK_PRIOR_TUNELS) {
            best = zlejedlo;
            bestdist = zjbd * BANIK_PRIOR_TUNELS;
        }

        if (bestdist > mbtbjbd * BANIK_PRIOR_EXPLORE) {
            best = mohlobytubytjedlo;
            bestdist = mbtbjbd * BANIK_PRIOR_EXPLORE;
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
    }


    if (stav.cas >= GATHERING_TIME) {
        cerr << "    Suicidal instinct at the max" << endl;
        Bod bestp (-1, -1);
        int bestdist = 999999999;

        for (Manik tar: stav.manici){
            if (tar.ktorehoHraca !=0) continue;
            if (tar.typ != MANIK_KOVAC && tar.typ != MANIK_MLATIC && tar.typ != MANIK_SEKAC) continue;

            if (vzdialenost.get(tar.pozicia()) < bestdist){
                bestp = tar.pozicia();
                bestdist = vzdialenost.get(tar.pozicia());
            }
        }

        for (int d = 0; d < 4; d++) {
            int nx = m.x + DX[d], ny = m.y + DY[d];

            for (Manik tar: stav.manici){
                if (tar.ktorehoHraca !=0) continue;
                if (tar.typ != MANIK_KOVAC && tar.typ != MANIK_MLATIC && tar.typ != MANIK_SEKAC) continue;

                if (Bod(nx, ny) == tar.pozicia() && m.zlato) {
                    cerr << "    Giving gold" << endl;
                    prikazy.push_back(Prikaz(m.id, PRIKAZ_DAJ_ZLATO, nx, ny, m.zlato));
                    return;
                }
                if (Bod(nx, ny) == tar.pozicia() && m.zelezo) {
                    cerr << "    Giving iron" << endl;
                    prikazy.push_back(Prikaz(m.id, PRIKAZ_DAJ_ZELEZO, nx, ny, m.zelezo));
                    return;
                }
            }
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
}