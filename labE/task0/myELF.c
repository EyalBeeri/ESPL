#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <elf.h>

#define MAX_ELF_FILES 2
#define NAME_LEN 128

typedef struct {
    char debug_mode;
    int fd[MAX_ELF_FILES];
    void *map_start[MAX_ELF_FILES];
    size_t file_size[MAX_ELF_FILES];
} state;

struct fun_desc {
    char *name;
    void (*fun)(state*);
};

void toggleDebugMode(state *s) {
    if (s->debug_mode) {
        s->debug_mode = 0;
        printf("Debug flag now off\n");
    } else {
        s->debug_mode = 1;
        printf("Debug flag now on\n");
    }
	printf("\n");
}

void examineElfFile(state *s) {
    char filename[NAME_LEN];
    printf("Enter ELF file name: ");
    fgets(filename, NAME_LEN, stdin);
    filename[strlen(filename) - 1] = '\0';

    for (int i = 0; i < MAX_ELF_FILES; i++) {
        if (s->fd[i] == -1) {
            s->fd[i] = open(filename, O_RDONLY);
            if (s->fd[i] < 0) {
                perror("Error opening file");
                return;
            }

            struct stat st;
            if (fstat(s->fd[i], &st) != 0) {
                perror("Error getting file size");
                close(s->fd[i]);
                s->fd[i] = -1;
                return;
            }
            s->file_size[i] = st.st_size;

            s->map_start[i] = mmap(NULL, s->file_size[i], PROT_READ, MAP_PRIVATE, s->fd[i], 0);
            if (s->map_start[i] == MAP_FAILED) {
                perror("Error mapping file");
                close(s->fd[i]);
                s->fd[i] = -1;
                return;
            }

            Elf32_Ehdr *header = (Elf32_Ehdr *)s->map_start[i];
            if (header->e_ident[EI_MAG0] != ELFMAG0 || 
                header->e_ident[EI_MAG1] != ELFMAG1 || 
                header->e_ident[EI_MAG2] != ELFMAG2 || 
                header->e_ident[EI_MAG3] != ELFMAG3) {
                printf("Not an ELF file\n");
                munmap(s->map_start[i], s->file_size[i]);
                close(s->fd[i]);
                s->fd[i] = -1;
                return;
            }
			printf("\n");

            printf("Magic:   %.3s\n", header->e_ident+1);
            printf("Data:    %s\n", header->e_ident[EI_DATA] == ELFDATA2LSB ? "2's complement, little endian" : "2's complement, big endian");
            printf("Entry point address:               0x%x\n", header->e_entry);
            printf("Start of section headers:          %d (bytes into file)\n", header->e_shoff);
            printf("Number of section headers:         %d\n", header->e_shnum);
            printf("Size of each section header:       %d (bytes)\n", header->e_shentsize);
            printf("Start of program headers:          %d (bytes into file)\n", header->e_phoff);
            printf("Number of program headers:         %d\n", header->e_phnum);
            printf("Size of each program header:       %d (bytes)\n", header->e_phentsize);
			printf("\n");
            return;
        }
    }
    printf("Already handling maximum number of ELF files\n");
	printf("\n");
}

void stubFunction(state *s) {
    printf("Not implemented yet\n");
	printf("\n");
}

void quit(state *s) {
    for (int i = 0; i < MAX_ELF_FILES; i++) {
        if (s->map_start[i]) {
            munmap(s->map_start[i], s->file_size[i]);
        }
        if (s->fd[i] != -1) {
            close(s->fd[i]);
        }
    }
    if (s->debug_mode) {
        fprintf(stderr, "Quitting\n");
    }
	printf("\n");
    exit(0);
}

int main(int argc, char **argv) {
    state *s = malloc(sizeof(state));
    s->debug_mode = 0;
    for (int i = 0; i < MAX_ELF_FILES; i++) {
        s->fd[i] = -1;
        s->map_start[i] = NULL;
    }

    struct fun_desc menu[] = {
        {"Toggle Debug Mode", toggleDebugMode},
        {"Examine ELF File", examineElfFile},
        {"Print Section Names", stubFunction},
        {"Print Symbols", stubFunction},
        {"Check Files for Merge", stubFunction},
        {"Merge ELF Files", stubFunction},
        {"Quit", quit},
        {NULL, NULL}
    };

    int choice;
    while (1) {
        if (s->debug_mode) {
            fprintf(stderr, "Debug log:\n");
            for (int i = 0; i < MAX_ELF_FILES; i++) {
                fprintf(stderr, "File %d: %s\n", i, s->fd[i] != -1 ? "open" : "closed");
            }
			printf("\n");
        }
        printf("Choose action:\n");
        for (int i = 0; menu[i].name != NULL; i++) {
            printf("%d-%s\n", i, menu[i].name);
        }
        printf("Option: ");
        scanf("%d", &choice);
        getchar(); // To consume the newline character after scanf

        if (choice >= 0 && choice < sizeof(menu)/sizeof(menu[0]) - 1) {
            menu[choice].fun(s);
        } else {
            printf("Not within bounds\n");
            quit(s);
        }
    }
}
