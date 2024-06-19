section .bss
input_char resb 1          ; Reserve 1 byte of space for input character


section .data
global infile
global outfile
infile dd 0                ; Define infile with an initial value of 0
outfile dd 1               ; Define outfile with an initial value of 1



section .text
global _start
global system_call
global read_input
extern main
_start:
    pop    dword ecx    ; ecx = argc
    mov    esi,esp      ; esi = argv
    ;; lea eax, [esi+4*ecx+4] ; eax = envp = (4*ecx)+esi+4
    mov     eax,ecx     ; put the number of arguments into eax
    shl     eax,2       ; compute the size of argv in bytes
    add     eax,esi     ; add the size to the address of argv 
    add     eax,4       ; skip NULL at the end of argv
    push    dword eax   ; char *envp[]
    push    dword esi   ; char* argv[]
    push    dword ecx   ; int argc

    call    main        ; int main( int argc, char *argv[], char *envp[] )
	nop
    mov     ebx,eax
    mov     eax,1
    int     0x80
    
        
system_call:
    push    ebp             ; Save caller state
    mov     ebp, esp
    sub     esp, 4          ; Leave space for local var on stack
    pushad                  ; Save some more caller state

    mov     eax, [ebp+8]    ; Copy function args to registers: leftmost...        
    mov     ebx, [ebp+12]   ; Next argument...
    mov     ecx, [ebp+16]   ; Next argument...
    mov     edx, [ebp+20]   ; Next argument...
    int     0x80            ; Transfer control to operating system
    mov     [ebp-4], eax    ; Save returned value...
    popad                   ; Restore caller state (registers)
    mov     eax, [ebp-4]    ; place returned value where caller can see it
    add     esp, 4          ; Restore caller state
    pop     ebp             ; Restore caller state
    ret                     ; Back to caller

read_input:
	push ebp
    mov ebp, esp            ; Set up stack frame
	pushad                  ; Save some more caller state
	mov eax, [ebp+8]        ; Move the value at ebp+8 into eax
	mov [infile], eax       ; Move the value in eax into infile

	mov eax, [ebp+12]       ; Move the value at ebp+12 into eax
	mov [outfile], eax      ; Move the value in eax into outfile

read_input_loop:
	mov eax, 3          ; syscall number for sys_read
	mov ebx, [infile] 	; infile file descriptor
	mov ecx, input_char       ; buffer to store input
	mov edx, 1          ; read one byte
	int 0x80            ; call kernel

	cmp eax, 0          ; check if EOF or error
	je ret_to_main             ; if yes, exit program
	

encode_and_print:
	mov al, [input_char]      ; load the byte into al
	cmp al, 'a'               ; compare with 'a'
	jl print_char             ; if less, jump to print_char
	cmp al, 'z'               ; compare with 'z'
	jg print_char             ; if greater, jump to print_char

	inc al                    ; increment ASCII value by 1
	mov [input_char], al      ; store back the result

print_char:
	mov eax, 4                ; syscall number for sys_write
	mov ebx, [outfile]        ; outfile file descriptor
	mov ecx, input_char       ; buffer with character to print
	int 0x80                  ; call kernel

	jmp read_input_loop            ; loop back to read next character

ret_to_main:
	popad                   ; Restore caller state (registers)
	pop ebp
	ret
