main:
	gcc -c src/display.c -o build/display.o
	gcc -c src/main.c -o build/main.o
	gcc build/display.o build/main.o -lSDL3 -lm -o bin/main
