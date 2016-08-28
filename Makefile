
CC=gcc -g -O0


LINKER   = gcc -o
# linking flags here
LFLAGS   = -Wall 

TARGET=test_tree

$(TARGET): geometry.o rtree.o main.o list.o
	$(LINKER) $@ geometry.o rtree.o main.o list.o

geometry.o: geometry.c geometry.h
	$(CC) -c geometry.c -o $@

rtree.o: rtree.c rtree.h
	$(CC) -c rtree.c -o $@

list.o: list.c list.h
	$(CC) -c list.c -o $@

main.o: main.c 
	$(CC) -c main.c -o $@
	
.PHONY: clean

clean:
	-$(RM) main.o geometry.o rtree.o