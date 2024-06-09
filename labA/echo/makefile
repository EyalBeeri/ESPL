my_echo: my_echo.o
	gcc -g -m32 -Wall -o my_echo my_echo.o

my_echo.o: my_echo.c
	gcc -g -m32 -Wall -c -o my_echo.o my_echo.c

clean:
	rm -f *.o my_echo