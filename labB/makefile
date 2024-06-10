# all: bubblesort hexaPrint AntiVirus

# bubblesort: bubblesort.o
	# gcc -g -m32 -Wall -o bubblesort bubblesort.o

# bubblesort.o: bubblesort.c
	# gcc -g -m32 -Wall -c -o bubblesort.o bubblesort.c

# hexaPrint: hexaPrint.o
	# gcc -g -m32 -Wall -o hexaPrint hexaPrint.o

# hexaPrint.o: hexaPrint.c
	# gcc -g -m32 -Wall -c -o hexaPrint.o hexaPrint.c

AntiVirus: AntiVirus.o
	gcc -g -m32 -Wall -o AntiVirus AntiVirus.o

AntiVirus.o: AntiVirus.c
	gcc -g -m32 -Wall -c -o AntiVirus.o AntiVirus.c

clean:
	# rm -f *.o bubblesort hexaPrint AntiVirus
	rm -f *.o AntiVirus
