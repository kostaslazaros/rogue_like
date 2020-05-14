CC = gcc

CFLAGS = -lncurses -I$(IDIR)

IDIR = ./include/
SRCDIR = ./src/

SOURCES = $(SRCDIR)*.c

all: rogue

rogue:
	$(CC) $(SOURCES) $(CFLAGS) -o $@

clean:
	rm rogue