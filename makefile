CC = gcc

CFLAGS = -lncurses

SRCDIR = ./src/

SOURCES = $(SRCDIR)*.c

all: rogue

rogue:
	$(CC) $(SOURCES) $(CFLAGS) -o $@