
SERVERDIR = ../../server
CXXFLAGS = -O2 -MMD -MP -Wall -I$(SERVERDIR)

CLEAN = *.d *.o hrac klient.tar.gz

all: hrac klient.tar.gz
naserveri: hrac

main.o: main.cpp 
	g++ -c $(CXXFLAGS) -std=gnu++0x main.cpp -o main.o

.PHONY: klient.tar.gz
klient.tar.gz:
	tar czf $@ * $(addprefix --exclude=,$(CLEAN))

hrac: main.o server_marshal.o server_update.o
	$(LINK.cpp) $^ $(LDLIBS) -o $@
		
server_%.o: $(SERVERDIR)/%.cpp
	$(COMPILE.cpp) -o $@ $<

clean:
	-rm -f $(CLEAN)

-include *.d

