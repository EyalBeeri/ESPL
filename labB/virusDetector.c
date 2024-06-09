#include <stdlib.h>
#include <stdio.h>
#include <linux/limits.h>

char *sigFileName = "signatures-L";
FILE *sigFile;

typedef struct virus
{
    unsigned short SigSize;
    char virusName[16];
    unsigned char *sig;
} virus;

void SetSigFileName()
{
    char sigFile[PATH_MAX];
    fgets(sigFile, PATH_MAX, stdin);
    sigFileName = sigFile; // Memory leak?
    printf("%s", sigFileName);
}

virus *readVirus(FILE *file)
{
    virus *vir = malloc(sizeof(virus));
    

    unsigned char *buffer = malloc(18);
    if (buffer == NULL)
    {
        fprintf(stderr, "Error allocating memory\n");
        return NULL;
    }

    fread(buffer, 1, 18, sigFile);

    char Nstr[2];
    Nstr[0] = buffer[0];
    Nstr[1] = buffer[1];
    size_t N = atoi(Nstr);

    vir->SigSize = N;

    for (int i = 2; i < 18; i++) {
        vir->virusName[i-2] = buffer[i];
    }

    free(buffer);
    buffer = malloc(N);
    fseek(sigFile, 18, SEEK_CUR);
    fread(buffer, 1, N, sigFile);
    vir->sig = buffer;
    return vir;
}

void printVirus(virus* virus) {
    printf("Virus name: %s\n",virus->virusName);
    printf("Virus signature size: %d\n", virus->SigSize);
    printf("Virus signature: ");
    for(size_t i = 0; i < virus->SigSize; i++) {
        printf("%02x ", virus->sig[i]);
    }
    printf("\n");
}

int main(int argc, char **argv)
{

    // SetSigFileName();

    sigFile = fopen(sigFileName, "rb");
    if (sigFile == NULL)
    {
        perror("Error opening file");
        return 1;
    }

    unsigned char *buffer = malloc(4);
    if (buffer == NULL)
    {
        fprintf(stderr, "Error allocating memory\n");
        return 1;
    }

    fread(buffer, 1, 4, sigFile);

    unsigned int x = 0x76543210;
    char *c = (char *)&x;

    if (*c == 0x10)
    {
        // printf("Little endian\n");
        char magicNumber[4] = "VIRL";
        for (int i = 0; i < 4; i++)
        {
            if (buffer[i] != magicNumber[i])
            {
                perror("Not a signature file.");
                exit(1);
            }
        }
    }
    else
    {
        // printf("Big endian\n");
        char magicNumber[4] = "VIRB";
        for (int i = 0; i < 4; i++)
        {
            if (buffer[i] != magicNumber[i])
            {
                perror("Not a signature file.");
                exit(1);
            }
        }
    }
    free(buffer);
    fseek(sigFile, 4, 0);

    printVirus(readVirus(sigFile));
}