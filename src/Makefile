IDIR = ../include
CC=gcc
CFLAGS=-I$(IDIR) -Wall

ODIR=obj
LDIR=../lib

LIBS=-lncurses

_DEPS = unblind.h actions.h
DEPS = $(patsubst %,$(ODIR)/%,$(_OBJ))

_OBJ = unblind2.o main.o actions.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

unblind: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~