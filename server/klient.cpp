//SECURITY WARNING
//Chod prec uboha dusa. temne su miesta kam vchadzas
#include <climits>
#include <unistd.h>
using namespace std;

#include "klient.h"
#include "util.h"

#define CAS_KLIENTA_NA_INICIALIZACIU 4000
#define CAS_KLIENTA_NA_ODPOVED 500
#define MAXIMUM_RESTARTOV 3


Klient::Klient(string _label, string _uvodneData, string cwd, string zaznamovyAdresar)
    : label(_label), uvodneData(_uvodneData), timeout(0),
      zostavaRestartov(MAXIMUM_RESTARTOV) {
  vector<string> command;
  command.push_back("./hrac");
  proces.setProperties(command, cwd,
                       zaznamovyAdresar + "/stderr." + label);
}


void Klient::restartuj() {
  if (zostavaRestartov > 0) {
    zostavaRestartov--;
    log("restartujem klienta %s", label.c_str());
    proces.restartuj();
    proces.write(uvodneData);
    timeout = max(timeout, gettime() + CAS_KLIENTA_NA_INICIALIZACIU);
  }
  else {
    if (zostavaRestartov == 0) {
      log("vzdavam restartovanie klienta %s", label.c_str());
    }
    zostavaRestartov = -1;
    proces.zabi();
  }
}


void Klient::zabi() {
  proces.zabi();
}


void Klient::vypniTimeout() {
  timeout = max(timeout, LLONG_MAX);
}


string Klient::komunikuj(string request) {
  return komunikujNaraz(vector<Klient*>(1, this), vector<string>(1, request))[0];
}


vector<string> Klient::komunikujNaraz(vector<Klient*> klienti, vector<string> requesty) {
  int pocet = klienti.size();
  requesty.resize(pocet);

  vector<string> odpovede(pocet, "");
  vector<bool> cakam(pocet, false);

  for (int i = 0; i < pocet; i++) {
    if (klienti[i]->zostavaRestartov != -1) {
      cakam[i] = true;
      klienti[i]->proces.write(requesty[i]);
      klienti[i]->timeout = max(klienti[i]->timeout, gettime() + CAS_KLIENTA_NA_ODPOVED);
    }
  }

  bool hotovo = false;
  while (!hotovo) {
    hotovo = true;
    long long now = gettime();
    for (int i = 0; i < pocet; i++) {
      if (cakam[i] && now < klienti[i]->timeout) {
        hotovo = false;
        odpovede[i] += klienti[i]->proces.nonblockRead();
        if (odpovede[i].size() >= 3 && odpovede[i].substr(odpovede[i].size()-3, 3) == "\n.\n") {
          cakam[i] = false;
        }
      }
    }
    usleep(1);
  }

  for (int i = 0; i < pocet; i++) {
    if (cakam[i]) {
      log("klient %s nestihol odpovedat", klienti[i]->label.c_str());
    }
  }
  return odpovede;
}
