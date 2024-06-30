#include <stdlib.h>
#include <stdio.h>

int count_digits (char* arg) {
	int count = 0;
	for (int i = 0; arg[i] != '\0'; i++) {
		if (arg[i] >= '0' && arg[i] <= '9'){
			count++;
		}
	}
	return count;
}


int main(int argc, char** argv) {
	printf("String contains %d digits.\n", count_digits(argv[1]));
	return 0;
}