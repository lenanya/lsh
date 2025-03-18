all: lsh 

lsh: main.c
	cc main.c -o lsh -ggdb