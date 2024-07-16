# CC = cc
# CF = -O

default: all

include ../../UFconfig/UFconfig.mk

I = -I../../UFconfig -I../Include -I../../CSparse/Include

all: libczsparse.a

csz_stuff.o: 
	$(CC) $(CF) -c $(I) ../Source/csz_stuff.c -o $@
	
libczsparse.a: csz_stuff.o
	$(ARCHIVE)  libczsparse.a csz_stuff.o
	- $(RANLIB) libczsparse.a

clean:
	- $(RM) *.o

purge: distclean

distclean: clean
	- $(RM) *.a
