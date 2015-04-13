//SECURITY WARNING
//Chod prec uboha dusa. temne su miesta kam vchadzas
#include <cstdio>
#include <sstream>
using namespace std;

/* co sa tyka C/C++ I/O:
 * vid scanftest.cpp a streamtest.cpp v minulom proboji (proboj-ctf)... neni
 * to ziadna slava. v mojom pripade sa mi viac hodia streamy (lebo failbitom
 * mozem vracat neuspech pri nacitavani), ale pouzivaju sa zle.
 *
 * FILE: je tam jednak obsah a potom nekonecno EOFov
 * istream: je tam obsah a potom "jeden" EOF, a akonahle sa na ten EOF nejaka
 * funkcia pozrie tak je chyba so streamom skoro cokolvek dalsie robit.
 */

int nacitajCislo(istream& in) {
  int result;
  in >> result;
  return result;
}

// peek, ktory sa velmi snazi nenastavit failbit.
int safepeek(istream& in) {
  if (in.eof()) return EOF;
  if (in.fail()) return EOF;
  return in.peek();
}

// ak na vstupe najde sentinel, tak ho nacita a vrati true, inak nenacita
// nic (okrem whitespacu) a vrati false. sentinel moze byt znak alebo EOF.
// fcia by nikdy nemala zmenit failbit.
bool nacitajSentinel(istream& in, int sentinel) {
  while (1) {   // precitame whitespace
    int c = safepeek(in);
    if (!(c >= 0 && c <= ' ')) break;
    in.ignore(1);
  }
  if (safepeek(in) == sentinel) {
    if (sentinel != EOF) in.ignore(1);   // precitame sentinel
    return true;
  }
  else return false;   // ak sme nenasli sentinel, precitame len whitespace
}

void printstate(istream& in) {
  printf("%d", (int)in.rdbuf()->pubseekoff(0, ios_base::cur, ios_base::in));   // tellg() zapne failbit ak uz je eofbit!
  if (in.eof()) printf(":eof");
  if (in.fail()) printf(":fail");
  if (in.bad()) printf(":bad");
}

void test(const char *s, int sentinel) {
  printf("\"%s\", %d:", s, sentinel);
  stringstream ss(s);
  while (1) {
    printf(" ");
    printstate(ss);
    bool foo = nacitajSentinel(ss, sentinel);
    printf("->[%d]->", foo);
    printstate(ss);

    if (foo) break;
    if (ss.fail()) break;

    int a = nacitajCislo(ss);
    printf("->{%d}->", a);
    printstate(ss);
  }
  printf("\n");
}

int main() {
  test("10 20 30 ", EOF);
  test("10 20 30", EOF);
  test("10 ", EOF);
  test("10", EOF);
  test("", EOF);
  test("-10", EOF);
  test("10 . ", EOF);
  test(". ", EOF);
  test("u ", EOF);
  test("10u", EOF);
  test("10-", EOF);
  test("000", EOF);
  test(" u", EOF);
  test(" u 10", EOF);
  test("   10", EOF);
  test("  ", EOF);
  test("10 20 . 30", '.');
  test("10 20 .30", '.');
  test("10.", '.');
  test("10 20 ", '.');
  test("10 20", '.');
  test("10u", '.');
  test("10-.", '.');
  test("   .  ", '.');
  test("  ", '.');
  test("", '.');
  return 0;
}

