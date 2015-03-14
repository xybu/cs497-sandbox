SRC_FILES = $(wildcard src/*.cpp)
SRC_HEADERS = $(wildcard src/*.h)
GPP = g++
CFLAGS = -g -O3 -Wall -pthread -std=gnu++11
PROGNAME = main
LIBS = -levent -levent_pthreads

all: $(SRC_FILES) $(SRC_HEADERS)
	$(GPP) -o $(PROGNAME) $(CFLAGS) $(SRC_FILES) $(LIBS)

debug: $(SRC_FILES) $(SRC_HEADERS)
	$(GPP) -D_DEBUG -D_COLORFUL -o $(PROGNAME) $(CFLAGS) $(SRC_FILES) $(LIBS)

check:
	@./utils/get_libevent.sh

valgrind:
	valgrind -v --leak-check=full --show-leak-kinds=all ./$(PROGNAME)

clean:
	rm -v $(PROGNAME)
