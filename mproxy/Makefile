SRC_FILES = $(wildcard src/*.cpp)
SRC_HEADERS = $(wildcard src/*.h)
GPP = g++
PROGNAME = main
CFLAGS = -Wall -pthread -std=gnu++11
LIBS = -Linc/ -levent -levent_pthreads -lloci
INCLUDE_PATH = -I inc/loci/inc

include Makefile.in

all: $(SRC_FILES) $(SRC_HEADERS)
	$(GPP) $(OPTFLAGS) -o $(PROGNAME) $(CFLAGS) $(INCLUDE_PATH) $(SRC_FILES) $(LIBS)

debug: $(SRC_FILES) $(SRC_HEADERS)
	$(GPP) $(OPTFLAGS) -D_DEBUG -D_COLORFUL -g -o $(PROGNAME) $(CFLAGS) $(INCLUDE_PATH) $(SRC_FILES) $(LIBS)

check:
	@./utils/get_libevent.sh

valgrind:
	valgrind -v --leak-check=full --show-leak-kinds=all ./$(PROGNAME)

clean:
	rm -v $(PROGNAME)
