#include <stdio.h>
#include <stdlib.h>

void PrintHex(unsigned char *buffer, size_t length) {
    for(size_t i = 0; i < length; i++) {
        printf("%02x ", buffer[i]);
    }
    printf("\n");
}

int main(int argc, char *argv[]) {
    if(argc != 2) {
        fprintf(stderr, "Usage: %s FILE\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "rb");
    if(file == NULL) {
        perror("Error opening file");
        return 1;
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    rewind(file);

    unsigned char *buffer = malloc(fileSize);
    if(buffer == NULL) {
        fprintf(stderr, "Error allocating memory\n");
        return 1;
    }

    size_t bytesRead = fread(buffer, 1, fileSize, file);
    if(bytesRead != fileSize) {
        fprintf(stderr, "Error reading file\n");
        return 1;
    }

    PrintHex(buffer, bytesRead);

    free(buffer);
    fclose(file);

    return 0;
}
