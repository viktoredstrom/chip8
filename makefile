CC=gcc

chip8: chip8.c
	$(CC) -Wall chip8.c -o chip8 -lSDL2
