all:
	gcc -o build/raywolf src/main.c `sdl2-config --cflags --libs` -lm 
