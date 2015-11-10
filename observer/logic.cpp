
#include <cstdio>
#include <cstring>
#include <cmath>
#include <vector>
#include <map>
#include <set>
#include <fstream>
#include <sstream>
#include "SDL.h"
#include "SDL_ttf.h"
using namespace std;

#include "logic.h"
#include "common.h"
#include "marshal.h"
#include "util.h"

#define DEFAULT_SCALE 10
#define FINAL_DELAY 100
#define CHCEM_VELKOST 1000

#define JEDLO_TRAPNE      0 
#define JEDLO_REVERS      1 
#define JEDLO_PREZEN      2
#define JEDLO_BOMBA       3

char jedlo[]={'T','R','P','B','O','C','M'};

static int scale;
static int formatVersion;
static Mapa mapa;
static vector<vector<map<int,int> > > teren;
static vector<Stav> stavy;
static vector<pair<string,vector<int> > > observations;
static vector<string> titles;

static set<int> frameTimes;

static TTF_Font *font;
static int fontWidth, fontHeight;
static SDL_Surface *mapSurface;

vector<pair<string, string> > ranklist;

const int farbyHracov[] = {
    0xcc00ff,
    0x3737c8,
    0x2ad4ff,
    0x00ff66,
    0x008000,
    0xccff00,
    0xffcc00,
    0xff7f2a,
    0x3c6432,
    0x00300f,
    0x00043d,
    0x000a32,
};

const int farbyBonusov[] = {
  0x662020,
  0x206620,
  0x202066,
  0x992066,
  0x996620,
  0x209966,
  0x206699,
  0x662099,
  0x669920  
};

template<class T> void checkStream(T& s, string filename) {
  if (s.fail()) {
    fprintf(stderr, "neviem citat z %s\n", filename.c_str());
    exit(1);
  }
}


void nacitajMedia() {
  const char *command = "fc-match monospace -f %{file}";
  FILE *pipe = popen(command, "r");
  if (pipe == NULL) {
    fprintf(stderr, "neviem spustit '%s'\n", command);
    exit(1);
  }
  char fontfile[4096 + 1];
  int len = fread(fontfile, 1, 4096, pipe);
  if (ferror(pipe) || !feof(pipe) || !len) {
    fprintf(stderr, "neviem precitat vystup '%s'\n", command);
    exit(1);
  }
  fontfile[len] = 0;

  font = TTF_OpenFont(fontfile, 12);
  if (!font) {
    fprintf(stderr, "neviem otvorit %s: %s\n", fontfile, TTF_GetError());
    exit(1);
  }

  fontHeight = TTF_FontLineSkip(font);

  SDL_Surface *space = TTF_RenderUTF8_Shaded(font, " ", SDL_Color(), SDL_Color());
  fontWidth = space->w;
  SDL_FreeSurface(space);

  mapSurface = SDL_CreateRGBSurface(SDL_SWSURFACE, mapa.w*scale, mapa.h*scale, 32,
      0xFF0000, 0x00FF00, 0x0000FF, 0x000000);
  if (mapSurface->pitch != mapSurface->w * 4) {
    fprintf(stderr, "mapSurface ma zly pitch\n");
    exit(1);
  }
}


void nacitajAdresar(string zaznamovyAdresar) {
  const char *scalevar = getenv("SCALE");
  scale = atoi(scalevar ? scalevar : "");
  if (!scale) scale = DEFAULT_SCALE;
  ifstream formatstream((zaznamovyAdresar+"/format").c_str());
  formatstream >> formatVersion;
  checkStream(formatstream, zaznamovyAdresar+"/format");
  formatstream.close();

  ifstream rankliststream((zaznamovyAdresar+".ranklist").c_str());
  string tempa, tempb;
  while (rankliststream >> tempa >> tempb) {
    ranklist.push_back(make_pair(tempa, tempb));   
  }

  ifstream observationstream((zaznamovyAdresar+"/observation").c_str());
  string line;
  while (getline(observationstream, line)) {
    stringstream linestream(line);
    string type;
    linestream >> type;
    vector<int> args;
    int arg;
    while (linestream >> arg) args.push_back(arg);
    observations.push_back(make_pair(type, args));
  }
  observationstream.close();

  ifstream mapastream((zaznamovyAdresar+"/map").c_str());
  nacitaj(mapastream, mapa);
  scale= CHCEM_VELKOST/max(mapa.w, mapa.h);
  checkStream(mapastream, zaznamovyAdresar+"/map");
  mapastream.close();

  ifstream logstream((zaznamovyAdresar+"/log").c_str());
  Teren startovnyTeren;
  nacitaj(logstream, startovnyTeren);
  stavy.resize(1);
  nacitaj(logstream, stavy[0]);
  checkStream(logstream, zaznamovyAdresar+"/log");
  while (true) {
    vector<Odpoved> odpovede;
    Stav s;
    nacitaj(logstream, odpovede);
    nacitaj(logstream, s);
    if (logstream.fail()) break;
    stavy.push_back(s);
  }
  logstream.close();

  ifstream titlestream((zaznamovyAdresar+"/titles").c_str());
  if (titlestream.fail()) {
    for (int i = 0; i < mapa.pocetHracov; i++) {
      stringstream ss;
      ss << "Hrac " << (i+1);
      titles.push_back(ss.str());
    }
  } else {
    for (int i = 0; i < mapa.pocetHracov; i++) {
      string line;
      getline(titlestream, line);
      getline(titlestream, line);
      getline(titlestream, line);
      titles.push_back(line);
    }
    checkStream(titlestream, zaznamovyAdresar+"/titles");
  }
  titlestream.close();

  int cas = 1;
  teren.resize(mapa.h);
  for (int y = 0; y < mapa.h; y++) for (int x = 0; x < mapa.w; x++) {
    if (x == 0) teren[y].resize(mapa.w);
    teren[y][x][0] = startovnyTeren.get(x, y);
  }
  FOREACH(it, observations) {
    if (it->first == "odsimulujKolo.zacina") cas = it->second[1];
    if (it->first == "teren") {
      int x = it->second[0], y = it->second[1], t = it->second[2];
      teren[y][x][cas] = t;
    }
  }
}


void zistiVelkostObrazovky(int *w, int *h) {
  *w = mapa.w*scale + 300;
  *h = mapa.h*scale;
}


static void putpixel(double x, double y, Uint32 c) {
  Uint32* pixels = (Uint32 *)mapSurface->pixels;
  x *= scale; y *= scale;
  for (int xx = 0; xx < scale; xx++) for (int yy = 0; yy < scale; yy++) {
    pixels[(int)(((floor(y)+yy) * mapSurface->w) + (floor(x)+xx))] = c;
  }
}


class Printer {
public:
  Printer(SDL_Surface *_screen, int _y) : screen(_screen), x(mapSurface->w), y(_y * fontHeight) {
  }
  void print(const char *text, int width, bool right = true, Uint32 color = 0xFFFFFF, Uint32 bgcolor = 0x000000) {
    SDL_Color fg; fg.r = (color>>16)&0xFF; fg.g = (color>>8)&0xFF; fg.b = (Uint8)(color&0xFF);
    SDL_Color bg; bg.r = (bgcolor>>16)&0xFF; bg.g = (bgcolor>>8)&0xFF; bg.b = (Uint8)(bgcolor&0xFF);

    if (x != 0) {
      x++;
      SDL_Rect line; line.x = x; line.y = y; line.w = 1; line.h = fontHeight;
      SDL_FillRect(screen, &line, SDL_MapRGB(screen->format, 255, 255, 255));
      x++;
    }

    SDL_Surface *image = TTF_RenderUTF8_Shaded(font, text, fg, bg);
    SDL_Rect src; src.x = 0; src.y = 0; src.w = min((int)image->w, width * fontWidth); src.h = image->h;
    SDL_Rect dest; dest.x = x + (right && image->w < width * fontWidth ? width * fontWidth - image->w : 0); dest.y = y;
    SDL_BlitSurface(image, &src, screen, &dest);
    SDL_FreeSurface(image);
    x += width * fontWidth;
  }
private:
  SDL_Surface *screen;
  int x, y;
};


void vykresluj(SDL_Surface *screen, double dnow) {
  if (dnow > stavy.size() + FINAL_DELAY) exit(0);
  int now = min((int)dnow, (int)stavy.size() - 1);
  double interpolation = dnow-(floor(dnow));
  const Stav& stav = stavy[now];
  const Stav& nstav = stavy[min(now+1,(int)stavy.size()-1)];
  
  
  SDL_LockSurface(mapSurface);
  for (int y = 0; y < mapa.h; y++) {
    for (int x = 0; x < mapa.w; x++) {
      int tuto = stav.teren.get(Bod(x,y));
      switch (tuto) {
        case MAPA_SUTER:  putpixel(x, y, 0x000000); break;
        case MAPA_VOLNO:  putpixel(x, y, 0xFFFFFF); break;
        case MAPA_SPAWN:  putpixel(x, y, 0xFFF000); break;
        default:
          putpixel(x, y, 0x000000);
          break;
      }
    }
  }
  
  //nakresli interpolovanych hadov
  
  for( int i = 0; i< nstav.hadi.size();i++) {
    for(int j =0; j< max(nstav.hadi[i].telo.size(),stav.hadi[i].telo.size());j++){
      if(j>= nstav.hadi[i].telo.size()){
	Bod olds = stav.hadi[i].telo[j];
	putpixel(olds.x, olds.y, farbyHracov[nstav.hadi[i].ktorehoHraca]); //TODO shadni to
	continue;
      }
      
      if(j>= stav.hadi[i].telo.size()){
	Bod news = nstav.hadi[i].telo[j];
	putpixel(news.x, news.y, farbyHracov[nstav.hadi[i].ktorehoHraca]); //TODO shadni to
	continue;
      }
      Bod olds = stav.hadi[i].telo[j];
      Bod news = nstav.hadi[i].telo[j];
      double xxx = (1.0-interpolation)*olds.x+interpolation*news.x;
      double yyy = (1.0-interpolation)*olds.y+interpolation*news.y;
      //printf("%lf %lf %lf %d %d %d %d\n",interpolation,xxx,yyy,olds.x,olds.y, news.x,news.y);
      putpixel(xxx,yyy , farbyHracov[nstav.hadi[i].ktorehoHraca]); //TODO shadni to
      if(j<stav.hadi[i].telo.size()-1){ //aby to bolo v rohoch hranate
	putpixel(olds.x,olds.y, farbyHracov[nstav.hadi[i].ktorehoHraca]); //TODO shadni to
      }
    }
  }
  
  //nakresli jedlo a bonusy
  //TODO interpolovat jedlo, aj ked to asi nechcem. nech zomruuu
  FOREACH(it, stav.jedlo){
    putpixel(it->pozicia.x, it->pozicia.y, farbyBonusov[it->typ]);
  }
  
  SDL_UnlockSurface(mapSurface);

  SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
  SDL_BlitSurface(mapSurface, NULL, screen, NULL);
  
  Printer header(screen, 0);
  header.print("Hráč", 20, false); header.print("Skóre", 5);
  header.print("telo", 4); 
  header.print("zije", 4);
  header.print("zasoba", 6);
  
  
  SDL_Rect hline; hline.x = 0; hline.y = mapSurface->h + fontHeight - 1; hline.w = screen->w; hline.h = 1;
  SDL_FillRect(screen, &hline, SDL_MapRGB(screen->format, 255, 255, 255));
   
  for (int i = 0; i < mapa.pocetHracov; i++) {
    const Snake *s = &stav.hadi[i];
    Printer p(screen, i + 1);
    if (dnow > stavy.size() && s->zije) {
        p.print(titles[i].c_str(), 20, false, farbyHracov[i], 0xFF0000);
        p.print(itos(stav.hraci[i].skore).c_str(), 5, true, 0xFFFFFF, 0xFF0000);
        p.print(itos(s->telo.size()).c_str(), 4, true, 0xFFFFFF, 0xFF0000);
        p.print(itos(s->zije).c_str(), 4, true, 0xFFFFFF, 0xFF0000);
    } else {
        p.print(titles[i].c_str(), 20, false, farbyHracov[i]);
        p.print(itos(stav.hraci[i].skore).c_str(), 5);
        p.print(itos(s->telo.size()).c_str(), 4);
        p.print(itos(s->zije).c_str(), 4);
    }
    string zasoba = " ";
    for(int j=0;j<s->zasoba.size();j++) zasoba+=jedlo[s->zasoba[j].typ];
    
    p.print(zasoba.c_str(), 6);
  
  }
  
  /*
  */
  int realtimenow = SDL_GetTicks();
  frameTimes.insert(realtimenow);
  while (*frameTimes.begin() < realtimenow - 5000) frameTimes.erase(frameTimes.begin());

  char buf[1000];
  sprintf(buf, "čas %d", now);
  if (realtimenow > 5000) sprintf(buf+strlen(buf), ", fps %.1f", frameTimes.size()/5.0);
  Printer(screen, mapa.pocetHracov + 1).print(buf, 30, false);
  for (int i = 0; i < ranklist.size(); i++) {
    Printer p(screen, mapa.pocetHracov + 2 + i);
    p.print(ranklist[i].first.c_str(), 10);
    p.print(ranklist[i].second.c_str(), 20);
  }
}
