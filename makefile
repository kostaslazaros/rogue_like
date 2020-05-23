CC = gcc

CFLAGS = -lncurses -I$(IDIR)

IDIR = ./include/
SRCDIR = ./src/

SOURCES = $(SRCDIR)*.c\
		  $(SRCDIR)utils/*.c

all: rogue

rogue: $(SOURCES)
	$(CC) $(SOURCES) $(CFLAGS) -o $@

clean:
	rm rogue