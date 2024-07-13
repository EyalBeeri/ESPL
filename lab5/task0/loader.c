#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <elf.h>

void foreach_phdr(void *map_start, void (*func)(Elf32_Phdr *, int), int arg) {
    Elf32_Ehdr *elf_header = (Elf32_Ehdr *)map_start;
    Elf32_Phdr *program_headers = (Elf32_Phdr *)((char *)map_start + elf_header->e_phoff);
    int num_program_headers = elf_header->e_phnum;

    for (int i = 0; i < num_program_headers; ++i) {
        func(&program_headers[i], i);
    }
}

// Example function to print program header information
void print_phdr(Elf32_Phdr *phdr, int index) {
    printf("Program header number %d at address 0x%x\n", index, phdr->p_vaddr);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <ELF file>\n", argv[0]);
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        return 1;
    }

    // Get the file size
    off_t file_size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    // Map the file into memory
    void *map_start = mmap(NULL, file_size, PROT_EXEC, MAP_PRIVATE, fd, 0);
    if (map_start == MAP_FAILED) {
        perror("Error mapping file");
        close(fd);
        return 1;
    }

    // Call foreach_phdr with the print_phdr function
    foreach_phdr(map_start, print_phdr, 0);

    // Clean up
    munmap(map_start, file_size);
    close(fd);

    return 0;
}
