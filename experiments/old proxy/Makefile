SRC_FILES = $(wildcard src/*.cpp)
SRC_HEADERS = $(wildcard src/*.h)
GPP = g++
CFLAGS = -g -O1 -Wall -pthread -std=gnu++11
PROGNAME = mproxy
LIBS = 

all: $(SRC_FILES) $(SRC_HEADERS)
	$(GPP) -o $(PROGNAME) $(CFLAGS) $(SRC_FILES) $(LIBS)

check:
	@vendor/get_libtins.sh

clean:
	rm -v main
