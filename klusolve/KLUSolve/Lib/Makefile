default: all

ccode: all

include ../../UFconfig/UFconfig.mk

# for testing only:
# TEST = -DTESTING

C = $(CC) $(CFLAGS) -Wno-write-strings
#C = g++ $(CFLAGS)

INC = ../Include/klusolve.h ../Include/klusystem.h \
    ../../UFconfig/UFconfig.h Makefile

I = -I../../AMD/Include -I../../COLAMD/Include -I../../BTF/Include \
    -I../../CSparse/Include -I../../CZSparse/Include -I../../KLU/Include \
    -I../Include -I../../UFconfig

all: library

library: libklusolve.a

KLUSOLVE = \
    klusolve.o klusystem.o

libklusolve.a: $(KLUSOLVE)
	$(ARCHIVE) libklusolve.a $(KLUSOLVE)
	$(RANLIB) libklusolve.a

$(KLUSOLVE): $(INC)

#-------------------------------------------------------------------------------

klusolve.o:
	$(C) $(I) -c ../Source/KLUSolve.cpp -o $@

klusystem.o:
	$(C) $(I) -c ../Source/KLUSystem.cpp -o $@

#-------------------------------------------------------------------------------

purge: distclean

distclean: clean
	- $(RM) libklusolve.a 

clean:
	- $(RM) $(CLEAN)

#-------------------------------------------------------------------------------
