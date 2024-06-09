#include <stdio.h>

int main(int argc, char **argv) {
	for (int i = 1; i < argc; i++) {
		printf("%s ", argv[i]);
	}
	printf("\b\n");
	return 0;
}