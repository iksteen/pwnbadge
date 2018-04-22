all: pwnbadge

CFLAGS	= -Wall -g -O3 -std=c++11
LDFLAGS	= -lm -lrt -lpthread
CCP = g++
CC = gcc

pwnbadge: pwnbadge.cpp librpitx/src/librpitx.a
	$(CCP) $(CFLAGS) -o pwnbadge pwnbadge.cpp librpitx/src/librpitx.a $(LDFLAGS)

librpitx/src/librpitx.a:
	$(MAKE) -C librpitx/src

clean:
	rm -f pwnbadge
	$(MAKE) -C librpitx/src clean
