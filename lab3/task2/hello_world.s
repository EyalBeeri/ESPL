section .data
    hello db 'Hello, World!', 0xA  ; The string to print, 0xA is newline

section .text
    global _start  ; The program entry point

_start:
    ; Write 'Hello, World!' to the standard output
    mov eax, 4  ; The system call number for sys_write
    mov ebx, 1  ; File descriptor 1 is stdout
    mov ecx, hello  ; Pointer to the message
    mov edx, 14  ; Length of the message string
    int 0x80  ; Call the kernel

    ; Terminate the program
    mov eax, 1  ; The system call number for sys_exit
    xor ebx, ebx  ; Exit code 0
    int 0x80  ; Call the kernel