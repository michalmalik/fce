bits 32
global _start

struc   idtr
        .limit  resw    1
        .base   resd    1
endstruc

section .text

syscall_exit:
        mov al, 1
        jmp syscall_wrapper
        
syscall_read:
        mov al, 3
        jmp syscall_wrapper

syscall_write:
        mov al, 4
        jmp syscall_wrapper
        
syscall_open:
        mov al, 5
        jmp syscall_wrapper
        
syscall_close:
        mov al, 6
        jmp syscall_wrapper
        
syscall_stat:
        mov al, 18
        jmp syscall_wrapper
        
syscall_newfstat:
        mov al, 108
        jmp syscall_wrapper
        
syscall:
        int 0x80
        ret

syscall_wrapper:
        movzx eax, al
        mov ebx, dword [esp + 4]
        mov ecx, dword [esp + 8]
        mov edx, dword [esp + 12]
        mov esi, dword [esp + 16]
        mov edi, dword [esp + 20]
        call syscall
        ret
        
_start:
        ; VM check
        ; idtr.base > 0xfff00000 on a virtual machine
        sidt [m]
        cmp dword [m + idtr.base], 0xfff00000     
        jl cont1
        push vm_msg_length
        push vm_message
        push 1
        call syscall_write
        
cont1:
        ; VirtualBox check
        ; /proc/scsi/scsi contains VBOX
        push 0
        push vbox_check_path
        call syscall_open 
        push 512
        push vbox_file
        push eax
        call syscall_read 
        push eax
        call syscall_close
        pop eax
        
        xor ecx, ecx    ; found chars
        xor ebx, ebx    ; position in buffer
        xor edx, edx    ; char from file
_vxloop:
        mov esi, vbox_file
        mov edi, vbox_check_str
        add esi, ebx
        add edi, ecx
        mov dl, byte [esi]
        cmp dl, byte [edi] 
        jne _vxnf
        inc ecx
        cmp ecx, 4      ; compare found length with VBOX len
        je cont2
        jmp _vxp
_vxnf: 
        xor ecx, ecx
_vxp:
        inc ebx
        cmp ebx, eax
        jl _vxloop
cont2:
        cmp ecx, 4
        jne exit
        
        push vbox_msg_length
        push vbox_message
        push 1
        call syscall_write
               
        call syscall_close 
exit:
        push 0
        call syscall_exit
        
section .data

        m istruc idtr
                at idtr.limit,  dw 0
                at idtr.base,   dd 0
        iend
        
        vm_message:             db "Running under a VM", 0x0A, 0
        vm_msg_length:          equ $-vm_message
        
        vbox_message:           db "Running VirtualBox", 0x0A, 0
        vbox_msg_length:        equ $-vbox_message
        
        vbox_check_path:        db "/proc/scsi/scsi", 0
        vbox_check_str:         db "VBOX", 0
        vbox_file:              times 512 db 0