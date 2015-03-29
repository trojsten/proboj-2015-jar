void coRobiKovac(const Manik &m) {
    cerr << "Kovac no." << kovaccount << endl;
    cerr << "    resources - gold: " << m.zlato << "; iron: " << m.zelezo << ";" << endl;

    kovaccount++;


    int d = rand() % 4;
    if (chcesKovaca != m.id) chcesKovaca = -1;

    int cbanik = 0, cmlatic = 0;
    for (Manik tar: stav.manici) {
        if (tar.ktorehoHraca == 0 && tar.typ == MANIK_BANIK)  cbanik ++;
        if (tar.ktorehoHraca == 0 && (tar.typ == MANIK_MLATIC || tar.typ == MANIK_SEKAC)) cmlatic++;
    }

    //|| (cbanik/(double) (cmlatic + 0.00000001)) > POMER_MLATICOV

    int typ = MANIK_BANIK;
    if (stav.cas >= GATHERING_TIME || cbanik >= POCET_BANIKOV ) {
        if (POMER_MLATIC_SEKAC) typ = MANIK_MLATIC;
        else typ = MANIK_SEKAC;
    }

    if (kovacovia.size() > 1) {
        int i = 0;
        if (kovacovia[i].id == m.id) i++;

        if (kovacovia[i].zlato + VYROVNAVACI_ROZDIEL_KOVACOV < m.zlato) {
            cerr << "    Givng gold" << endl;
            prikazy.push_back(Prikaz(m.id, PRIKAZ_DAJ_ZLATO, kovacovia[i].pozicia(), (m.zlato - kovacovia[i].zlato)/2 ));
        }

        if (kovacovia[i].zelezo + VYROVNAVACI_ROZDIEL_KOVACOV < m.zelezo){
            cerr << "    Giving iron" << endl;
            prikazy.push_back(Prikaz(m.id, PRIKAZ_DAJ_ZELEZO, kovacovia[i].pozicia(), (m.zelezo - kovacovia[i].zelezo)/2 ));
        }
    }


    if (SANCA_KOVAC && stav.cas > GATHERING_TIME && kovacovia.size () < 2) {
        chcesKovaca = m.id;
        cerr << "Cakam na kovaca"  << stav.cas << endl;
    }

    if (chcesKovaca > -1) {
        cerr << "    Recruiting Blacksmith" << endl;
        prikazy.push_back(Prikaz(m.id, PRIKAZ_KUJ, m.x + DX[d], m.y + DY[d], MANIK_KOVAC));
    }

    else  {
        prikazy.push_back(Prikaz(m.id, PRIKAZ_KUJ, m.x + DX[d], m.y + DY[d], typ));
        cerr << "    Recruinting " << (typ == MANIK_BANIK? "Miner" : "Mlatic") << endl;
    }
    //prikazy.push_back(Prikaz(m.id, PRIKAZ_KUJ, m.x + 1, m.y + 0, MANIK_KOVAC));

}