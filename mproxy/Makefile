CPP_FILES = $(wildcard src/*.cpp)
CPP_OBJS = $(subst .cpp,.o,$(CPP_FILES))
C99_OBJS = src/stream.o src/action.o
HEADERS = $(wildcard src/*.h)
PROGNAME = main
CFLAGS = -pthread
LIBS = -Linc/ -levent -levent_pthreads -lloci
INCLUDE_PATH = -I inc/loci/inc
DEBUG_FLAGS = -g

include Makefile.in

all: $(CPP_OBJS) $(HEADERS) $(C99_OBJS)
	$(GPP) $(DEBUG_FLAGS) $(OPTFLAGS) -o $(PROGNAME) $(CFLAGS) $(INCLUDE_PATH) $(CPP_OBJS) $(C99_OBJS) $(LIBS)

debug: DEBUG_FLAGS += -D_DEBUG -D_COLORFUL
debug: clean all

src/stream.o: src/stream.c src/stream.h
	$(GCC) $(DEBUG_FLAGS) $(OPTFLAGS) -c -o $@ $<

src/action.o: src/action.c src/action.h
	$(GCC) $(DEBUG_FLAGS) $(OPTFLAGS) $(INCLUDE_PATH) -c -o $@ $< $(LIBS)

src/%.o: src/%.cpp $(HEADERS)
	$(GPP) $(DEBUG_FLAGS) $(OPTFLAGS) -c -o $@ $<

check:
	@./utils/get_libevent.sh

valgrind:
	valgrind -v --leak-check=full --show-leak-kinds=all ./$(PROGNAME)

clean:
	rm -fv $(PROGNAME) src/*.o
