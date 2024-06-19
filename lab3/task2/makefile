all: main

start.o: start.s
	nasm -f elf32 start.s -o start.o

util.o: util.c util.h
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector util.c -o util.o

main.o: main.c
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector main.c -o main.o

main: start.o main.o util.o
	ld -m elf_i386 start.o main.o util.o -o main

clean:
	rm -f *.o main
