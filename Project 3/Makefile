CFLAGS = -std=c99 -g
 
all: MyProj

MyProj: main.o Opcodes.o New.o
	$(CC) $(CFLAGS) -o MyProj Opcodes.o -lm main.o -lm New.o -lm

main.o: main.c Vars.h
	$(CC) $(CFLAGS) -c main.c -lm -o main.o -lm

Opcodes.o: Opcodes.c Vars.h
	$(CC) $(CFLAGS) -c Opcodes.c -lm -o Opcodes.o -lm

New.o: New.c Vars.h
	$(CC) $(CFLAGS) -c New.c -lm -o New.o -lm

clean: 
	rm *.o MyProj
