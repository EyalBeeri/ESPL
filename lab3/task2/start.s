section .data
hello_message db 'Hello, Infected File', 0xA ; Message followed by newline character
hello_len equ $-hello_message

virus_message db 'Hello, I am a virus', 0xA
virus_len equ $-virus_message

section .bss
file_descriptor resd 1 ; Reserve space for file descriptor (4 bytes)

section .text
global _start
global system_call
global infection
global infector
extern main
extern strlen

_start:
    pop dword ecx ; ecx = argc
    mov esi, esp ; esi = argv
    mov eax, ecx ; put the number of arguments into eax
    shl eax, 2 ; compute the size of argv in bytes
    add eax, esi ; add the size to the address of argv
    add eax, 4 ; skip NULL at the end of argv
    push dword eax ; char *envp[]
    push dword esi ; char* argv[]
    push dword ecx ; int argc

    call main ; int main(int argc, char *argv[], char *envp[])

    mov ebx, eax
    mov eax, 1
    int 0x80
    nop

system_call:
    push ebp ; Save caller state
    mov ebp, esp
    sub esp, 4 ; Leave space for local var on stack
    pushad ; Save some more caller state

    mov eax, [ebp + 8] ; Copy function args to registers: leftmost...
    mov ebx, [ebp + 12] ; Next argument...
    mov ecx, [ebp + 16] ; Next argument...
    mov edx, [ebp + 20] ; Next argument...
    int 0x80 ; Transfer control to operating system
    mov [ebp - 4], eax ; Save returned value...
    popad ; Restore caller state (registers)
    mov eax, [ebp - 4] ; place returned value where caller can see it
    add esp, 4 ; Restore caller state
    pop ebp ; Restore caller state
    ret ; Back to caller

infection:
    mov eax, 4 ; System call number for write
    mov ebx, 1 ; File descriptor number for stdout
    mov ecx, hello_message ; Message to write
    mov edx, hello_len ; Number of bytes to write
    int 0x80 ; Call kernel
    ret

infector:
    push ebp
    mov ebp, esp

    ; Print file name
    
    mov ebx, 1 ; File descriptor number for stdout
    mov ecx, [ebp + 8] ; Filename is at [ebp+8]
	push ecx ;
    call strlen
    mov edx, eax ; Number of bytes to write
	mov eax, 4 ; System call number for write
    int 0x80 ; Call kernel

    ; Open file for appending
    mov eax, 5 ; System call number for open
    mov ebx, [ebp + 8] ; Pointer to filename
    mov ecx, 2 | 1024 ; Flags (O_RDWR | O_APPEND)
    mov edx, 0 ; Mode (not used)
    int 0x80 ; Call kernel
    cmp eax, 0
    jl error ; If error opening file
    mov [file_descriptor], eax ; Save file descriptor

    ; Write virus message to file
    mov eax, 4 ; System call number for write
    mov ebx, [file_descriptor] ; File descriptor number for infected file
    mov ecx, virus_message ; Message to write
    mov edx, virus_len ; Number of bytes to write
    int 0x80 ; Call kernel

    ; Close the file
    mov eax, 6 ; System call number for close
    mov ebx, [file_descriptor] ; File descriptor to close
    int 0x80 ; Call kernel

    pop ebp
    ret

error:
    mov eax, 1
    mov ebx, 0x55
    int 0x80
