
SERVERDIR = ../server
CXXFLAGS = -O2 -MMD -MP -Wall -I$(SERVERDIR) -std=gnu++11

CXXFLAGS += $(shell sdl-config --cflags)
LDLIBS += $(shell sdl-config --libs) -lSDL_ttf -lSDL_image

CLEAN = *.d *.o observer

all: observer

observer: main.o logic.o server_marshal.o server_util.o
	$(LINK.cpp) $^ $(LDLIBS) -o $@

server_%.o: $(SERVERDIR)/%.cpp
	$(COMPILE.cpp) -o $@ $<

clean:
	-rm -f $(CLEAN)

-include *.d
