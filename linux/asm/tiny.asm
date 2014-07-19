bits 32
org 0x08048000

ehdr:
                db      0x7F, "ELF", 1, 1, 1, 0                 ; e_ident
        times 8 db      0
                dw      2                                       ; e_type (ET_EXEC)
                dw      3                                       ; e_machine (EM_386)
                dd      1                                       ; e_version (EV_CURRENT)
                dd      _start                                  ; e_entry
                dd      phdr - $$                               ; e_phoff
                dd      0                                       ; e_shoff
                dd      0                                       ; e_flags
                dw      ehdrsize                                ; e_ehsize
                dw      phdrsize                                ; e_phentsize
                dw      1                                       ; e_phnnum
                dw      0                                       ; e_shentsize
                dw      0                                       ; e_shnum
                dw      0                                       ; e_shstrndx
        
ehdrsize:       equ     $ - ehdr

phdr:
                dd      1                                       ; p_type (PT_LOAD)
                dd      0                                       ; p_offset
                dd      $$                                      ; p_vaddr
                dd      $$                                      ; p_paddr
                dd      filesize                                ; p_filesz
                dd      filesize                                ; p_memsz
                dd      5                                       ; p_flags
                dd      0x1000                                  ; p_align
                
phdrsize:       equ     $ - phdr

_start:

        mov al, 1
        mov bl, 0x42
        int 80h
        
filesize:       equ     $ - $$