BINFOLDER := bin/
INCFOLDER := inc/
SRCFOLDER := src/
OBJFOLDER := obj/

CC := gcc
CFLAGS := -Wall
SRCFILES := $(wildcard src/*.c)

all: $(SRCFILES:src/%.c=obj/%.o)
	$(CC) $(CFLAGS) obj/*.o -o bin/binary -lpthread

obj/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@ -I./inc

.PHONY: clean
clean:
	rm -rf obj/*
	rm -rf bin/*
	rm -rf doc/*.txt
	rm -rf *.o
	rm -f *.txt

run:
		bin/binary
test:
		bin/binary doc/file.txt
