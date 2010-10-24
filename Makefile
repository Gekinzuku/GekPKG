all:
	gcc compression.c main.c str.c -o gekpkg -lz -lm
