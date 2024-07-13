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
        func(&program_headers[i], arg);
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

void load_phdr(Elf32_Phdr *phdr, int fd) {
    if (phdr->p_type != PT_LOAD) {
        // Skip program headers that are not of type PT_LOAD
        return;
    }

    // Determine protection flags
    int prot_flags = 0;
    if (phdr->p_flags & PF_R)
        prot_flags |= PROT_READ;
    if (phdr->p_flags & PF_W)
        prot_flags |= PROT_WRITE;
    if (phdr->p_flags & PF_X)
        prot_flags |= PROT_EXEC;

    // Calculate the aligned virtual address and offset
    Elf32_Addr vaddr = phdr->p_vaddr & 0xfffff000;
    Elf32_Off offset = phdr->p_offset & 0xfffff000;
    size_t padding = phdr->p_vaddr & 0xfff;

    // Map the segment into memory
    void *map_start = mmap((void *)vaddr, phdr->p_memsz + padding, prot_flags, MAP_PRIVATE | MAP_FIXED, fd, offset);
    if (map_start == MAP_FAILED) {
        perror("Error mapping segment");
        exit(1);
    }

    // Print information about the mapped segment
	printf("%-14s %-8s %-8s %-8s %-6s %-6s %-3s %-4s  %s\n",
		"Type", "Offset", "VirtAddr", "PhysAddr", "FileSiz", "MemSiz", "Flg", "Align", "MmapFlags");
	print_phdr(phdr, 0);	
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


    foreach_phdr(map_start, load_phdr, fd);

    // Clean up
    munmap(map_start, file_size);
    close(fd);

    return 0;
}
