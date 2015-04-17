LOCI_SRC_FILES = $(wildcard loci/src/*.c)
LOCI_INC_FILES = $(wildcard loci/inc/loci/*.h)
LOCI_OBJ_FILES = $(subst .c,.o,$(LOCI_SRC_FILES))

ATTACKER_SRC_FILES = $(wildcard attacker/src/*.c)
ATTACKER_INC_FILES = $(wildcard attacker/inc/attacker/*.h)
ATTACKER_OBJ_FILES = $(subst .c,.o,$(ATTACKER_SRC_FILES))

include ../Makefile.in

all: libloci.a libattacker.a

libloci.a: $(LOCI_OBJ_FILES) $(LOCI_INC_FILES)
	ar rcu $@ $(LOCI_OBJ_FILES)

libattacker.a: $(ATTACKER_OBJ_FILES) $(ATTACKER_INC_FILES)
	ar rcu $@ $(ATTACKER_OBJ_FILES)

loci/src/%.o: loci/src/%.c $(LOCI_INC_FILES)
	$(GCC) $(OPTFLAGS) -I loci/inc -c $< -o $@

attacker/src/%.o: attacker/src/%.c $(ATTACKER_INC_FILES)
	$(GCC) $(OPTFLAGS) -I attacker/inc -c $< -o $@

clean:
	rm -fv $(LOCI_OBJ_FILES) loci.a
