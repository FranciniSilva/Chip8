all: chip8

chip8: chip8.o
	  gcc chip8.o -o chip8

chip8.o: chip8.c
	  gcc -c chip8.c

clean:
	rm -rf *o chip8
