
CXXFLAGS = -O2 -MMD -MP -Wall

all: server

server: main.o klient.o mapa.o marshal.o proces.o update.o util.o
	$(LINK.cpp) $^ $(LDLIBS) -o $@

streamtest: streamtest.o
	$(LINK.cpp) $^ $(LDLIBS) -o $@

clean:
	-rm -f *.d *.o server streamtest

-include *.d

