all: loader

start.o: start.s
	nasm -f elf32 start.s -o start.o

# startup.o: startup.s
# 	nasm -f elf32 startup.s -o startup.o

loader.o: loader.c
	gcc -m32 -Wall -c loader.c -o loader.o

loader: start.o loader.o # startup.o
	ld -o loader loader.o startup.o start.o -L/usr/lib32 -lc -T linking_script -dynamic-linker /lib32/ld-linux.so.2

clean:
	rm -f *.o loader
