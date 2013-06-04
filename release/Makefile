CC  = gcc
GCC = $(CC)

CFLAGS = -c -Wall -I ../

SRC=*.c
OBJ=*.o

all: 
	$(GCC) $(CFLAGS) $(SRC)
	ar cr libcolibro.a $(OBJ)
clean:
	rm *.o -f
