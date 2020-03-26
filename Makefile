
CC = cc

CFLAGS = -g

ALL = palmread.o palmrec.o palmdump

all:	    $(ALL)

palmread.o: palmread.c pdb.h palmread.h

palmrec.o:  palmrec.c pdb.h palmread.h palmrec.h

palmdump:   palmdump.o palmread.o pdb.h palmread.h getopt.o getopt.h xdsub.o
	$(CC) $(CFLAGS) -o palmdump palmdump.o palmread.o xdsub.o

clean:
	rm -f *.bak *.o $(ALL) *.zip

zip:
	rm -f palmdump.zip
	cp -p /ftp/palm/palmdump/index.html palmdump.html
	zip palmdump.zip COPYING Makefile TRADEMARKS getopt.c getopt.h palmdump.c palmdump.exe palmdump.html palmread.c palmread.h palmrec.c palmrec.h pdb.h xdsub.c
	rm palmdump.html
