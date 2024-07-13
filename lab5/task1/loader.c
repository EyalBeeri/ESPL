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

// Function to print program header information
void print_phdr(Elf32_Phdr *phdr, int index) {
    const char *type_str;
    switch (phdr->p_type) {
        case PT_LOAD:
            type_str = "LOAD";
            break;
        case PT_PHDR:
            type_str = "PHDR";
            break;
        case PT_INTERP:
            type_str = "INTERP";
            break;
        case PT_DYNAMIC:
            type_str = "DYNAMIC";
            break;
        case PT_NOTE:
            type_str = "NOTE";
            break;
        default:
            type_str = "UNKNOWN";
            break;
    }

    printf("%-14s 0x%06x 0x%08x 0x%08x 0x%05x 0x%05x %c%c%c 0x%x  %s\n",
           type_str,
           phdr->p_offset, phdr->p_vaddr, phdr->p_paddr,
           phdr->p_filesz, phdr->p_memsz,
           (phdr->p_flags & PF_R) ? 'R' : '-',
           (phdr->p_flags & PF_W) ? 'W' : '-',
           (phdr->p_flags & PF_X) ? 'E' : '-',
           phdr->p_align,
           (phdr->p_flags & PF_X) ? "MAP_PRIVATE | MAP_FIXED" : "-");
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
	printf("%-14s %-8s %-8s %-8s %-6s %-6s %-3s %-4s  %s\n",
           "Type", "Offset", "VirtAddr", "PhysAddr", "FileSiz", "MemSiz", "Flg", "Align", "MmapFlags");
    foreach_phdr(map_start, print_phdr, 0);

    // Clean up
    munmap(map_start, file_size);
    close(fd);

    return 0;
}
