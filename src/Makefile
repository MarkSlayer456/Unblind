IDIR = ../include
CC=gcc
CFLAGS=-I$(IDIR) -Wall

ODIR=obj
LDIR=../lib

LIBS=-lncurses -lpthread

_DEPS = unblind.h actions.h double_linked_list.h mainframe.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = unblind2.o main.o actions.o double_linked_list.o mainframe.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

unblind: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~
