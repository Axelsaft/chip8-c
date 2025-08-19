program: main display chip8
	gcc build/display.o build/chip8.o build/main.o -lSDL3 -g -o bin/main && cd bin && ./main

main: src/main.c
	gcc -c src/main.c -o build/main.o

display: src/display.c include/display.h
	gcc -c src/display.c -o build/display.o

chip8: src/chip8.c include/chip8.h
	gcc -c src/chip8.c -o build/chip8.o
