const int zaklad_logistickeho_stromu = 5;
const int cena_zlata = 1, cena_zeleza = 1;
const int bonus_za_tmu = 1000, bonus_za_hmlu = 1;
const double sanca_mlatic = 0, sanca_banik = 0.98, sanca_kovac = 0.02, hmlofobia = 0.01, paranoja = 0.0001, xenofobia = 0.001, zvedavost = 0.02,dost_vela = 0.4;

const int hendikep[4] = {inf, 1000, 200, 60}, zastarany_hendikep = 30, logisticky_bonus = -500000, agresor_exploruje = 4;

const int celkova_agresivita = 50;

//------------------------------ 0    1    2   3   4     5     6     7     8          9    10   11   12   13    14   15   16  17  18  19  20  21  22  23  24   25   26 27 28 29 30 31 32   33
//------------------------------ -    -    Fe  Au  suter volno pasca start unexplored -    -    hFe  hAu  hSut  hVol hPas hSt eBa eSe eMl eGu eSc eSt eLo eKu  eKv  Ba Se Ml Gu Sc St Lo   Ku   Kv
 int banikov_movement[35]     = { inf, inf, 10, 10, 10,   5,    inf,  5,    11,        inf, inf, 12,  12,  12,   6,   inf, 6,  5,  5,  5,  5,  5,  5,  5,  inf, inf, 8, 80, 80,80, 80, 80, 80, inf, inf};
int agresivny_movement[35]   = { inf, inf, inf,inf,inf,  5,    inf,  5,    inf,       inf, inf, inf, inf, inf,  6,   inf, 6,  5,  5,  5,  5,  5,  5,  5,  10,  20,  8, 80, 80,80, 80, 80, 80, inf, inf};



int magicka_funkcia(int expl)
{
	if(expl == 0)return 1023456789;
	if(expl > bonus_za_tmu)expl -= (expl%bonus_za_tmu);
	return 12000/expl;
}

long long chcem_aspon(int spod)
{
	long long a= 1;
	for(int i=1; i<spod; i++)a += zaklad_logistickeho_stromu;
	return (int)(dost_vela*a)+1;
}