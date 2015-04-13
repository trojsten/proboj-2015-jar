#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
using namespace std;

#include "mapa.h"

#define chyba(...) (fprintf(stderr, __VA_ARGS__), false)

//nacita povodnu mapu. len start, sutre, bonusove spavnovatka  a volno.
bool nacitajMapu(Mapa& mapa, string filename, int pocetHracov) {
  FILE *in = fopen(filename.c_str(), "r");
  if (!in) return chyba("neviem citat '%s'\n", filename.c_str());

  if (fgetc(in) != 'P') return chyba("'%s' ma zly format, chcem raw PPM\n", filename.c_str());
  if (fgetc(in) != '6') return chyba("'%s' ma zly format, chcem raw PPM\n", filename.c_str());

  // podporujeme komentare len medzi headerom a zvyskom (aj ked PPM standard umoznuje skoro kdekolvek)
  char c;
  fscanf(in, " %c", &c);
  while (c == '#') {
    while (c != '\n') c = fgetc(in);
    fscanf(in, " %c", &c);
  }
  ungetc(c, in);

  unsigned w, h, maxval;
  fscanf(in, "%u%u%u", &w, &h, &maxval);
  fgetc(in);
  if (maxval != 255) return chyba("'%s' ma zlu farebnu hlbku, podporujem len 24bpp\n", filename.c_str());

  mapa.w = w;
  mapa.h = h;
  mapa.pocetHracov = pocetHracov;
  mapa.pribliznyTeren.data.clear();
  mapa.pribliznyTeren.data.resize(h);

  for (unsigned y = 0; y < h; y++) {
    mapa.pribliznyTeren.data[y].resize(w);
    for (unsigned x = 0; x < w; x++) {
      int r = fgetc(in);
      int g = fgetc(in);
      int b = fgetc(in);
      if (r == EOF || g == EOF || b == EOF) return chyba("necakany EOF pri citani '%s'\n", filename.c_str());
      if (r == 255 && g == 255 && b == 255) mapa.pribliznyTeren.data[y][x] = MAPA_VOLNO;
      else if (r == 0 && g == 0 && b == 0) mapa.pribliznyTeren.data[y][x] = MAPA_SUTER;
      //else if (r == 255 && g == 0 && b == 0) mapa.pribliznyTeren.data[y][x] = MAPA_VOLNO;
      else if (r == 255 && g == 240 && b == 0) mapa.pribliznyTeren.data[y][x] = MAPA_SPAWN;
      else if (r == 255 && g == 255 && b == 0) mapa.pribliznyTeren.data[y][x] = MAPA_SPAWN;
      else if (r == 0 && g == 255 && b == 0) mapa.pribliznyTeren.data[y][x] = MAPA_START;
//       else mapa.pribliznyTeren.data[y][x] = MAPA_VOLNO;
      else return chyba("zla farba %d,%d,%d na pozicii %d,%d v '%s'\n", r, g, b, x, y, filename.c_str());
    }
  }

  fclose(in);
  return true;
}

