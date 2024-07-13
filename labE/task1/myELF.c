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

const char* section_type_names[] = {
    "NULL", "PROGBITS", "SYMTAB", "STRTAB", "RELA", "HASH",
    "DYNAMIC", "NOTE", "NOBITS", "REL", "SHLIB", "DYNSYM",
    "INIT_ARRAY", "FINI_ARRAY", "PREINIT_ARRAY", "GROUP", "SYMTAB_SHNDX"
};

typedef struct {
    char debug_mode;
    int fd[MAX_ELF_FILES];
    void *map_start[MAX_ELF_FILES];
    size_t file_size[MAX_ELF_FILES];
    char file_name[MAX_ELF_FILES][NAME_LEN];
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

            strcpy(s->file_name[i], filename);
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

void printSectionNames(state *s) {
    for (int i = 0; i < MAX_ELF_FILES; i++) {
        if (s->fd[i] == -1) {
            continue;
        }

        printf("File %s\n", s->file_name[i]);

        Elf32_Ehdr *header = (Elf32_Ehdr *)s->map_start[i];
        Elf32_Shdr *sections = (Elf32_Shdr *)(s->map_start[i] + header->e_shoff);
        const char *section_str_table = s->map_start[i] + sections[header->e_shstrndx].sh_offset;

        for (int j = 0; j < header->e_shnum; j++) {
            char *section_name = (char *)(section_str_table + sections[j].sh_name);
            printf("[%2d] %s %08x %06x %06x %s\n", 
                    j, 
                    section_name, 
                    sections[j].sh_addr, 
                    sections[j].sh_offset, 
                    sections[j].sh_size, 
                    sections[j].sh_type < 17 ? section_type_names[sections[j].sh_type] : "UNKNOWN");
        }
        printf("\n");
    }
}

void printSymbols(state *s) {
    for (int i = 0; i < MAX_ELF_FILES; i++) {
        if (s->fd[i] == -1) {
            continue;
        }

        Elf32_Ehdr *header = (Elf32_Ehdr *)s->map_start[i];
        Elf32_Shdr *sections = (Elf32_Shdr *)(s->map_start[i] + header->e_shoff);
        const char *section_str_table = s->map_start[i] + sections[header->e_shstrndx].sh_offset;

        for (int j = 0; j < header->e_shnum; j++) {
            if (sections[j].sh_type == SHT_SYMTAB) {
                Elf32_Sym *symtab = (Elf32_Sym *)(s->map_start[i] + sections[j].sh_offset);
                int num_symbols = sections[j].sh_size / sizeof(Elf32_Sym);
                const char *strtab = s->map_start[i] + sections[sections[j].sh_link].sh_offset;

                printf("File %s\n", s->file_name[i]);

                for (int k = 0; k < num_symbols; k++) {
                    const char *symbol_name = strtab + symtab[k].st_name;
                    int section_index = symtab[k].st_shndx;
                    const char *section_name = section_index < header->e_shnum ? section_str_table + sections[section_index].sh_name : "UNKNOWN";
                    printf("[%2d] %08x %d %s %s\n", 
                            k, 
                            symtab[k].st_value, 
                            section_index, 
                            section_name, 
                            symbol_name);
                }
                printf("\n");
            }
        }
    }
}

Elf32_Sym* findSymbolByName(const char* name, Elf32_Sym* symtab, int num_symbols, const char* strtab) {
    for (int i = 0; i < num_symbols; i++) {
        if (strcmp(name, strtab + symtab[i].st_name) == 0) {
            return &symtab[i];
        }
    }
    return NULL;
}

void checkMerge(state *s) {
    if (s->fd[0] == -1 || s->fd[1] == -1) {
        printf("Two ELF files must be opened and mapped\n");
        return;
    }

    Elf32_Shdr *sections1 = (Elf32_Shdr *)(s->map_start[0] + ((Elf32_Ehdr *)s->map_start[0])->e_shoff);
    Elf32_Shdr *sections2 = (Elf32_Shdr *)(s->map_start[1] + ((Elf32_Ehdr *)s->map_start[1])->e_shoff);

    Elf32_Sym *symtab1 = NULL, *symtab2 = NULL;
    int num_symbols1 = 0, num_symbols2 = 0;
    const char *strtab1 = NULL, *strtab2 = NULL;

    for (int i = 0; i < ((Elf32_Ehdr *)s->map_start[0])->e_shnum; i++) {
        if (sections1[i].sh_type == SHT_SYMTAB) {
            symtab1 = (Elf32_Sym *)(s->map_start[0] + sections1[i].sh_offset);
            num_symbols1 = sections1[i].sh_size / sizeof(Elf32_Sym);
            strtab1 = s->map_start[0] + sections1[sections1[i].sh_link].sh_offset;
            break;
        }
    }

    for (int i = 0; i < ((Elf32_Ehdr *)s->map_start[1])->e_shnum; i++) {
        if (sections2[i].sh_type == SHT_SYMTAB) {
            symtab2 = (Elf32_Sym *)(s->map_start[1] + sections2[i].sh_offset);
            num_symbols2 = sections2[i].sh_size / sizeof(Elf32_Sym);
            strtab2 = s->map_start[1] + sections2[sections2[i].sh_link].sh_offset;
            break;
        }
    }

    if (!symtab1 || !symtab2) {
        printf("Feature not supported\n");
        return;
    }

    for (int i = 1; i < num_symbols1; i++) {
        const char *name1 = strtab1 + symtab1[i].st_name;
        Elf32_Sym *sym2 = findSymbolByName(name1, symtab2, num_symbols2, strtab2);

        if (symtab1[i].st_shndx == SHN_UNDEF) {
            if (!sym2 || sym2->st_shndx == SHN_UNDEF) {
                printf("Symbol %s undefined\n", name1);
            }
        } else {
            if (sym2 && sym2->st_shndx != SHN_UNDEF) {
                printf("Symbol %s multiply defined\n", name1);
            }
        }
    }

    for (int i = 1; i < num_symbols2; i++) {
        const char *name2 = strtab2 + symtab2[i].st_name;
        Elf32_Sym *sym1 = findSymbolByName(name2, symtab1, num_symbols1, strtab1);

        if (symtab2[i].st_shndx == SHN_UNDEF) {
            if (!sym1 || sym1->st_shndx == SHN_UNDEF) {
                printf("Symbol %s undefined\n", name2);
            }
        } else {
            if (sym1 && sym1->st_shndx != SHN_UNDEF) {
                printf("Symbol %s multiply defined\n", name2);
            }
        }
    }
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
        {"Print Section Names", printSectionNames},
        {"Print Symbols", printSymbols},
        {"Check Files for Merge", checkMerge},
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
