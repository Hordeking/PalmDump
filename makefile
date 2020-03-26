
CC = cc

CFLAGS = -g

ALL = palmread.o palmrec.o palmdump

all:	    $(ALL)

palmread.o: palmread.c pdb.h palmread.h

palmrec.o:  palmrec.c pdb.h palmread.h palmrec.h

palmdump:   palmdump.o palmread.o pdb.h palmread.h xdsub.o
	$(CC) $(CFLAGS) -o palmdump palmdump.o palmread.o xdsub.o

clean:
	rm -f *.bak *.o

distclean:
	rm -f *.bak *.o palmdump
