LOCI_SRC_FILES = $(wildcard loci/src/*.c)
LOCI_INC_FILES = $(wildcard loci/inc/loci/*.h)
LOCI_OBJ_FILES = $(subst .c,.o,$(LOCI_SRC_FILES))

include ../Makefile.in

all: libloci.a

libloci.a: $(LOCI_OBJ_FILES) $(LOCI_INC_FILES)
	ar rcu $@ $(LOCI_OBJ_FILES)

loci/src/%.o: loci/src/%.c $(LOCI_INC_FILES)
	$(GCC) $(OPTFLAGS) -I loci/inc -c $< -o $@

clean:
	rm -fv $(LOCI_OBJ_FILES) libloci.a
