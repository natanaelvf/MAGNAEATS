INC_DIR = include
OBJ_DIR = obj
SRC_DIR = src
BIN_DIR = bin


client.o = main.h memory.h
driver.o = main.h memory.h
main.o = main.h memory.h
memory.o = memory.h main.h
process.o = main.h memory.h
restaurant.o =  main.h memory.h

CFLAGS = -g -Wall -O2 -I $(INC_DIR)
LIBS = -lm

vpath %.o $(OBJ_DIR)

CC = gcc
OBJECTOS = client.o driver.o main.o memory.o process.o restaurant.o

all: clean magnaeats

magnaeats: $(OBJECTOS)
	$(CC) $(OBJECTOS) -o $(BIN_DIR)/magnaeats $(LIBS)

%.o: src/%.c $($@)
	$(CC) $(CFLAGS) -o $(OBJ_DIR)/$@ -c $<

valgrind_client: magnaeats
	$(VLGD)magnaeats 10 1 1 1

clean:
	rm –f *.o out
	rm –f out
