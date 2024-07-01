all: hexeditplus dig_count

hexeditplus: hexeditplus.o
	gcc -g -m32 -Wall -o hexeditplus hexeditplus.o

hexeditplus.o: task1.c
	gcc -g -m32 -Wall -c -o hexeditplus.o task1.c

dig_count: dig_count.o
	gcc -g -m32 -Wall -o dig_count dig_count.o

dig_count.o: task4.c
	gcc -g -m32 -Wall -c -fno-pie -fno-stack-protector -o dig_count.o task4.c

clean:
	rm -f *.o hexeditplus dig_count