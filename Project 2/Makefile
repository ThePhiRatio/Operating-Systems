CFLAGS = -std=c99 -g
  
all: Project2

Project2: main.o Opcodes.o
	$(CC) $(CFLAGS) -o Project2 Opcodes.o main.o

main.o: main.c Vars.h
	$(CC) $(CFLAGS) -c main.c -o main.o

Opcodes.o: Opcodes.c Vars.h
	$(CC) $(CFLAGS) -c Opcodes.c -o Opcodes.o

clean: 
	rm *.o Project2
