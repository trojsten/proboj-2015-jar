
Čo je proboj
------------

Proboj je počítačová hra, ktorej hráčmi nie ste vy, ale programy, čo napíšete.

Tento krat to bude snake. Ale trosku krvavejsi snake.


Zdrojáky
--------

Štandardný hráč čiže klient (v adresári `klienti/template`) sa skladá z jediného
zdrojáku `main.cpp`. Ale môžte ho rozdeliť aj na viacero.

V serveri je tiež zopár zdrojákov, čo vás bude zaujímať.

- `common.h` obsahuje základné štruktúry, čo váš klient dostane k dispozícii.
- `update.cpp` a `update.h` obsahujú všetky herné konštanty, a tiež
  implementáciu väčšiny herných pravidiel, takže ak v pravidlách nie je niečo
  jasné, skúste sa tam pozrieť.
- v `main.cpp` sú tiež nejaké pravidlá (ako sa ťahá apod.), ale to je asi menej
  dôležité.

Kľudne si prečítajte aj ostatné zdrojáky, ja sa len poteším, ale pri kódení
vášho klienta vám asi nepomôžu.

Ako kódiť klienta
-----------------

Skopírujte obsah `klienti/template` do iného adresára a niečo v ňom nakóďte.

V koreni proboju spustite `make`, čím všetko skompilujete. (Ak váš klient nie je
vnútri `klienti`, nastavte v jeho `Makefile` správny `SERVERDIR` a spustite
`make` aj v ňom.)

Potom spustite `./server/server zaznamy/01 mapy/mapa1.ppm klienti/vasklient
klienti/vasklient klienti/hlupy` To spustí hru s troma hráčmi (vaším, druhým
vaším a hlúpym) a uloží záznam do `zaznamy/01`. Ten si môžete pozrieť s príkazom
`./observer/observer zaznamy/01`.

Server sa vášho klienta pýta, čo chce robiť. Ak klient neodpovie včas, bude
automaticky zabitý a reštartovaný. Prvýkrát dostane viac času, aby sa mohol
inicializovať.

Keď server spustíte u vás, je to len na skúšku. Na hlavnom počítači to beží na
ostro. Je tam aj webové rozhranie, cez ktoré môžete uploadovať vašich klientov.
Uploadujú sa zdrojáky a tie sa potom skompilujú (konkrétne sa spustí `make
naserveri SERVERDIR=/adresar/kde/je/server`).


Aky je proboj
-------------

Ste had. Had v nehodtinom prostredi. Hybete sa ako had, rastiete ako had, myslite 
ako had. Casom ale hladnete a skracujete sa. Ked sa skratite prilis (0), tak zomriete.
Su tu ale rozne veci. tie viete brat do zasoby a nasledne papat. Rozne veci vam 
umoznia robit rozne veci po ich spapani. Napriklad narast, alebo aj ine. 

Vasa zasoba ma ale obmedzenu velkost a nic z nej nemozete vyhodit. Vsetko musite 
spapat. Ak by ste v nej mali toho prilis, tak sa spapa automaticky prve. 

Nechcete ale narazit. Ani do kamenov, ani do hadov. Ked narazite, tak zomriete! 

Ako sa ťahá
-----------

V kazdom kole dostanete kompletny stav hry, teda ako vyzera cela mapa, kde je ake jedlo, 
kde je aky hrac, atd. Jedine co mozete urobit je rozhodnut, do ktoreho smeru natocite
svoju hlavu (ako snake). Cele telo sa potom pohne za nov. Pokial by ste mali narast,
tak dorastiete na posledne policko, ktore by sa normalne uvolnilo (v skutocnosti teda
dorastate maximalne 1 za kolo). 

Dalej mozete spapat (jedno) jedlo, ktore mate v zasobe. Tak ze poslete v kolonke 
prikaz->pouzi, jeho poradove cislo vo vasej zasobe.


Pravidlá hry
------------

Pravidla su velmi jednoduche, ako bolo z casti popisane vyssie. pre konkretne informacie 
odporucam plakat, alebo nahliadnut do update.cpp a polovit v komentaroch.

inak bonusy su ako je v common.h

Mapy
----

Mapy sa kreslia vo formate ppm. Optimalna velkost je okolo 100x100, lepsie je menej, 
dlho to potom trva.

legenda: 
R   G   B
255 255 255 MAPA_VOLNO
0   0   0   MAPA_SUTER
255 255 0   MAPA_SPAWN (tu sa rodia bonusy      
0   255 0   MAPA_START tu sa rodia hadi    

potom ich treba dat do adresara mapy