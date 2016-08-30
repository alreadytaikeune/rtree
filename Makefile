
CC=gcc -g -O0


LINKER   = gcc 
# linking flags here
LFLAGS   = -Wall -shared 

CFLAGS   = -fPIC

TARGET_LIB=librtree 
TARGET_TEST=test_tree

C_FILES := $(wildcard *.c)
OBJ := $(notdir $(C_FILES:.c=.o))
OBJ_LIB = geometry.o list.o rtree.o


$(TARGET_LIB): $(OBJ_LIB)
	$(LINKER) $(LFLAGS) -Wl,-soname,$@.1 -o $@ $^

$(TARGET_TEST): $(OBJ)
	$(LINKER) -o $@ $^

$(OBJ): %.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@


server: server.o geometry.o rtree.o list.o
	$(LINKER) -L/usr/local/lib $^ -o $@ -lonion -lpthread 

server.o: server.c geometry.o rtree.o list.o
	$(CC) $(CFLAGS) -I/usr/local/include -c server.c -o $@

# geometry.o: geometry.c geometry.h
# 	$(CC) -c geometry.c -o $@

# rtree.o: rtree.c rtree.h
# 	$(CC) -c rtree.c -o $@

# list.o: list.c list.h
# 	$(CC) -c list.c -o $@

# main.o: main.c 
# 	$(CC) -c main.c -o $@




.PHONY: clean install

clean:
	-$(RM) $(OBJ) $(TARGET_LIB) $(TARGET_TEST) server.o server

install:
	@- cp $(TARGET_LIB) /usr/local/lib/$(TARGET_LIB)