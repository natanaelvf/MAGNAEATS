INCLUDE_DIR = include
OBJ_DIR = obj
SRC_DIR = src
BIN_DIR = bin


client.o = # dependencias de client.c
driver.o = # dependencias de driver.o
main.o = # dependencias de main.o
memory.o = # dependencias de memory.o
process.o = # dependencias de process.o
restaurant.o =  # dependencias de restaurant.o

CFLAGS = -Wall –I $(INC_DIR)
LIBS = -lm

vpath %.o $(OBJ_DIR)

CC = gcc
OBJECTOS = client.o driver.o main.o memory.o process.o restaurant.o

make : $(OBJECTOS)
	$(CC) $(OBJECTOS) -o MAGNAEATS$ (LIBS)

%.o: src/%.c $($@)
	$(CC) $(CFLAGS) -o $(OBJ_dir)/$@ -c $<

clean:
	rm –f *.o out
	rm –f out
