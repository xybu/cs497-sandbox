SRC_FILES = $(wildcard src/*.cpp)
SRC_HEADERS = $(wildcard src/*.h)
GPP = g++
CFLAGS = -g -O3 -Wall -pthread -std=gnu++11
PROGNAME = mproxy
LIBS = 

all: $(SRC_FILES) $(SRC_HEADERS)
	$(GPP) -o $(PROGNAME) $(CFLAGS) $(SRC_FILES) $(LIBS)

debug: $(SRC_FILES) $(SRC_HEADERS)
	$(GPP) -D_DEBUG -D_COLORFUL -o $(PROGNAME) $(CFLAGS) $(SRC_FILES) $(LIBS)

check:
	@echo -e "\033[91mNo library dependency to check.\033[0m"

clean:
	rm -v mproxy
