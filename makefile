INC_DIR = include
OBJ_DIR = obj
SRC_DIR = src
BIN_DIR = bin


client.o = main.h memory.h client.h
driver.o = main.h memory.h driver.h
main.o = main.h memory.h process.h
memory.o = memory.h main.h
process.o = main.h memory.h process.h restaurant.h driver.h client.h
restaurant.o =  main.h memory.h restaurant.h

CFLAGS = -std=gnu99 -g -Wall -I $(INC_DIR)
LIBS = -lm

vpath %.o $(OBJ_DIR)

CC = gcc
OBJECTOS = client.o driver.o main.o memory.o process.o restaurant.o

magnaeats: $(OBJECTOS)
	$(CC) $(CFLAGS) $(addprefix $(OBJ_DIR)/,$(OBJECTOS)) -o bin/magnaeats -pthread -lrt

%.o: src/%.c $($@)
	$(CC) $(CFLAGS) -o $(OBJ_DIR)/$@ -c $<

debug:
	valgrind --leak-check=full --track-origins=yes ./bin/magnaeats 10 1 1 1 1

clean:
	rm  $(OBJ_DIR)/*
	rm  $(BIN_DIR)/*
