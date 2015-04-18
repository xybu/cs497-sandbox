CPP_FILES = $(wildcard src/*.cpp)
CPP_OBJS = $(subst .cpp,.o,$(CPP_FILES))
C99_OBJS = $(subst .c,.o,$(wildcard src/*.c))
HEADERS = $(wildcard src/*.h)
PROGNAME = main
CFLAGS = -pthread -Wall
LIBS = -Linc/ -levent -levent_pthreads -lloci
INCLUDE_PATH = -I inc/loci/inc
DEBUG_FLAGS =

include Makefile.in

all: $(CPP_OBJS) $(HEADERS) $(C99_OBJS)
	$(GPP) $(DEBUG_FLAGS) $(OPTFLAGS) -o $(PROGNAME) $(CFLAGS) $(INCLUDE_PATH) $(CPP_OBJS) $(C99_OBJS) $(LIBS)

gdb: DEBUG_FLAGS += -g
gdb: clean all

debug: DEBUG_FLAGS += -g -D_DEBUG -D_COLORFUL
debug: clean all

src/stream.o: src/stream.c src/stream.h
	$(GCC) $(DEBUG_FLAGS) $(OPTFLAGS) $(CFLAGS) -c -o $@ $<

src/action.o: src/action.c src/action.h
	$(GCC) $(DEBUG_FLAGS) $(OPTFLAGS) $(CFLAGS) $(INCLUDE_PATH) -c -o $@ $< $(LIBS)

src/attack.o: src/attack.c src/attack.h
	$(GCC) $(DEBUG_FLAGS) $(OPTFLAGS) $(CFLAGS) $(INCLUDE_PATH) -c -o $@ $< $(LIBS)

src/csv.o: src/csv.c src/csv.h
	$(GCC) $(DEBUG_FLAGS) $(OPTFLAGS) $(CFLAGS) $(INCLUDE_PATH) -c -o $@ $< $(LIBS)

src/args.o: src/args.c src/args.h
	$(GCC) $(DEBUG_FLAGS) $(OPTFLAGS) $(CFLAGS) $(INCLUDE_PATH) -c -o $@ $< $(LIBS)

src/%.o: src/%.cpp $(HEADERS)
	$(GPP) $(DEBUG_FLAGS) $(OPTFLAGS) $(CFLAGS) -c -o $@ $<

check:
	@./utils/get_libevent.sh

valgrind:
	valgrind -v --leak-check=full --show-leak-kinds=all ./$(PROGNAME)

clean:
	rm -fv $(PROGNAME) src/*.o
