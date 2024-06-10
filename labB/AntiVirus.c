#include <stdlib.h>
#include <stdio.h>
#include <linux/limits.h>
#include <string.h>

char *sigFileName = "signatures-L";
int endian = 0; // 0 = little, 1 = big
FILE *sigFile;
char *suspectedFileName;

const char RET = 0xc3;

typedef struct virus
{
    unsigned short SigSize;
    char virusName[16];
    unsigned char *sig;
} virus;

typedef struct link
{
    struct link *nextVirus;
    struct virus *vir;
} link;

void SetSigFileName()
{
    sigFileName = malloc(PATH_MAX * sizeof(char));
    fgets(sigFileName, PATH_MAX, stdin);
    int len = strlen(sigFileName);
    if (len > 0 && sigFileName[len - 1] == '\n')
    {
        sigFileName[len - 1] = '\0';
    }
}

void LoadSignatures()
{
    sigFile = fopen(sigFileName, "rb");
    if (sigFile == NULL)
    {
        perror("Error opening file");
        exit(1);
    }

    unsigned char *buffer = malloc(4);
    if (buffer == NULL)
    {
        fprintf(stderr, "Error allocating memory\n");
        exit(1);
    }

    fread(buffer, 1, 4, sigFile);

    if ((strncmp(("VIRL"), (const char *)buffer, 4)) == 0)
    { // Little endian
        endian = 0;
    }
    else if ((strncmp(("VIRB"), (const char *)buffer, 4)) == 0)
    { // Big endian
        endian = 1;
    }
    else
    {
        perror("Not a virus signaure file");
        exit(1);
    }
    fseek(sigFile, 4, 0);
}

void reverseBuffer(unsigned char *buffer, int length)
{
    int start = 0;
    int end = length - 1;
    while (start < end)
    {
        // Swap characters
        char temp = buffer[start];
        buffer[start] = buffer[end];
        buffer[end] = temp;

        // Move towards the middle
        start++;
        end--;
    }
}

virus *readVirus(FILE *file)
{

    if (file == NULL) {
        perror("File not opened");
        exit(1);
    }

    virus *vir = malloc(sizeof(virus));

    unsigned char *buffer = malloc(18);
    if (buffer == NULL)
    {
        fprintf(stderr, "Error allocating memory\n");
        return NULL;
    }

    fread(buffer, 1, 18, sigFile);

    size_t N;
    if (endian == 0) // Little endian
    {
        N = (buffer[1] << 8) | buffer[0]; // Virus size

        if (N == 0)
        {
            return NULL;
        }

        for (int i = 0; i < 16; i++) // Virus name
        {
            vir->virusName[i] = buffer[i + 2];
        }

        free(buffer);

        buffer = malloc(N);
        fread(buffer, 1, N, sigFile);
    }
    else // Big endian
    {
        N = (buffer[0] << 8) | buffer[1]; // Virus size

        if (N == 0)
        {
            return NULL;
        }

        for (int i = 0; i < 16; i++) // Virus name
        {
            vir->virusName[i] = buffer[i + 2];
        }

        free(buffer);

        buffer = malloc(N);
        fread(buffer, 1, N, sigFile);
        reverseBuffer(buffer, N);
    }

    vir->SigSize = N;
    vir->sig = buffer;
    return vir;
}

void printVirus(virus *virus)
{
    if (virus != NULL)
    {
        printf("Virus name: %s\n", virus->virusName);
        printf("Virus signature size: %d\n", virus->SigSize);
        printf("Virus signature: ");
        for (size_t i = 0; i < virus->SigSize; i++)
        {
            printf("%02x ", virus->sig[i]);
        }
        printf("\n");
    }
}

void list_print(link *virus_list, FILE *fp)
{
    link *tmp = virus_list;
    while (tmp != NULL)
    {
        printVirus(tmp->vir);
        tmp = tmp->nextVirus;
    }
}

link *list_append(link *virus_list, virus *data)
{
    link *new_link = (link *)malloc(sizeof(link));
    new_link->vir = data;
    new_link->nextVirus = virus_list;
    return new_link;
}

void list_free(link *virus_list)
{
    link *tmp;
    while (virus_list != NULL)
    {
        tmp = virus_list;
        virus_list = virus_list->nextVirus;
        free(tmp->vir);
        free(tmp);
    }
}

link *list_reverse(link *virus_list)
{
    link *reverse_list = malloc(sizeof(link));

    while (virus_list != NULL)
    {
        reverse_list = list_append(reverse_list, virus_list->vir);
        virus_list = virus_list->nextVirus;
    }

    list_free(virus_list);

    return reverse_list;
}

link *extractViruses()
{ // pre: at least 1 virus in signatures file

    link *virusList = malloc(sizeof(link));
    virusList->vir = readVirus(sigFile);
    while (!feof(sigFile))
    {
        virusList = list_append(virusList, readVirus(sigFile));
    }
    return list_reverse(virusList);
}

void printSignatures()
{
    if (sigFile != NULL)
    {
        list_print(extractViruses(), sigFile);
        fseek(sigFile, 0, 0);
    }
}

void detect_single_virus(char *buffer, unsigned int size, virus *virus)
{
    for (int i = 0; i < size; i++)
    {
        if (virus != NULL && i + virus->SigSize < size && memcmp(&(buffer[i]), virus->sig, virus->SigSize) == 0)
        {
            printf("Starting byte location: %d (hex: %02x)\n", i, i);
            printf("Virus name: %s\n", virus->virusName);
            printf("Virus signature size: %d\n", virus->SigSize);
        }
    }
}

void detect_virus(char *buffer, unsigned int size, link *virus_list)
{
    while (virus_list != NULL)
    {
        detect_single_virus(buffer, size, virus_list->vir);
        virus_list = virus_list->nextVirus;
    }
}

void detectViruses()
{
    FILE *file = fopen(suspectedFileName, "rb");
    if (file == NULL)
    {
        perror("Error opening file");
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    rewind(file);

    char *buffer = malloc(10 * 1024);
    unsigned int size;

    if (fileSize > 10 * 1024) {
        size = 10 * 1024;
    } else {
        size = fileSize;
    }
    fread(buffer, 1, size, file);

    detect_virus(buffer, size, extractViruses());
    // fflush(file);
    fclose(file);
}


void fix_single_virus(char *buffer, unsigned int size, virus *virus, FILE* file)
{
    for (int i = 0; i < size; i++)
    {
        if (virus != NULL && i + virus->SigSize < size && memcmp(&(buffer[i]), virus->sig, virus->SigSize) == 0)
        {
            fseek(file, i, 0);
            fwrite(&RET, sizeof(char), 1, file);
        }
    }
}

void fix_virus(char *buffer, unsigned int size, link *virus_list, FILE* file)
{
    while (virus_list != NULL)
    {
        fix_single_virus(buffer, size, virus_list->vir, file);
        virus_list = virus_list->nextVirus;
    }
}

void fixFile()
{
    FILE *file = fopen(suspectedFileName, "r+b");
    if (file == NULL)
    {
        perror("Error opening file");
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    rewind(file);

    char *buffer = malloc(10 * 1024);
    unsigned int size;

    if (fileSize > 10 * 1024) {
        size = 10 * 1024;
    } else {
        size = fileSize;
    }
    fread(buffer, 1, size, file);

    fix_virus(buffer, size, extractViruses(), file);
    fclose(file);
}

void quit()
{
    exit(0);
}

struct fun_desc
{
    char *name;
    void (*fun)();
};

int main(int argc, char **argv)
{

    suspectedFileName = argv[1];

    struct fun_desc menu[] = {
        {"Set signatures file name", SetSigFileName},
        {"Load signatures", LoadSignatures},
        {"Print signatures", printSignatures},
        {"Detect viruses", detectViruses},
        {"Fix file", fixFile},
        {"Quit", quit},
        {NULL, NULL}};
    int choice;
    while (feof(stdin) == 0)
    {
        printf("Select operation from the following menu (ctrl^D for exit):\n");
        for (int i = 0; menu[i].name != NULL; i++)
        {
            printf("%d) %s\n", i, menu[i].name);
        }
        printf("Option : ");
        if (scanf("%d", &choice) == EOF)
        {
            return 0;
        }
        while ((getchar()) != '\n')
            ;
        if (choice < 0 || choice >= sizeof(menu) / sizeof(struct fun_desc) - 1)
        {
            printf("Not within bounds\n");
            return 0;
        }
        printf("\n");
        printf("Within bounds\n");
        menu[choice].fun();
        printf("DONE.\n\n");
    }
}