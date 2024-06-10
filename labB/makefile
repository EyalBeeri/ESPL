# all: bubblesort hexaPrint virusDetector

# bubblesort: bubblesort.o
	# gcc -g -m32 -Wall -o bubblesort bubblesort.o

# bubblesort.o: bubblesort.c
	# gcc -g -m32 -Wall -c -o bubblesort.o bubblesort.c

# hexaPrint: hexaPrint.o
	# gcc -g -m32 -Wall -o hexaPrint hexaPrint.o

# hexaPrint.o: hexaPrint.c
	# gcc -g -m32 -Wall -c -o hexaPrint.o hexaPrint.c

virusDetector: virusDetector.o
	gcc -g -m32 -Wall -o virusDetector virusDetector.o

virusDetector.o: virusDetector.c
	gcc -g -m32 -Wall -c -o virusDetector.o virusDetector.c

clean:
	# rm -f *.o bubblesort hexaPrint virusDetector
	rm -f *.o virusDetector
