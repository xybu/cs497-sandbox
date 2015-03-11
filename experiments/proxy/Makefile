SRC_FILES = $(wildcard src/*.cpp)
SRC_HEADERS = $(wildcard src/*.h)
GPP = g++
CFLAGS = -g -O1 -Wall
PROGNAME = mproxy

all: $(SRC_FILES) $(SRC_HEADERS)
	$(GPP) -o $(PROGNAME) $(CFLAGS) $(SRC_FILES)

clean:
	rm -v main
