format PE GUI
entry start

include 'macro/import32.inc'
include 'macro/proc32.inc'

section '.text' code readable writeable executable

	start:
		push [tid]
		push 0
		push 0
		push fnThread
		push 0
		push 0
		call [CreateThread]
		push eax

		push 1
		call [Sleep]

		pop eax
		push 1
		push eax
		call [WaitForSingleObject]

		push 0
		push org_caption
		push org_message
		push 0
		call [MessageBox]
	exit:
		push 0
		call [ExitProcess]

proc fnThread
    mov esi,dword [MessageBox]
    mov dword [mb_copy],esi
    mov dword [MessageBox],patchAPI

    push 0
    call [ExitThread]

    ret
endp

proc patchAPI
     push 0
     push ptc_caption
     push ptc_message
     push 0
     call [mb_copy]
     ret
endp

section '.data' data readable writeable

	org_caption		db 'Original caption',0
	org_message		db 'Original message',0
	ptc_caption		db 'Patched caption',0
	ptc_message		db 'Patched message',0

	mb_copy 		dd 0
	tid			dd ?

section '.import' import readable writeable

	library kernel32, 'KERNEL32.DLL',\
		user32, 'USER32.DLL'

	import kernel32,\
	       CreateThread, 'CreateThread',\
	       ExitThread, 'ExitThread',\
	       ExitProcess, 'ExitProcess',\
	       Sleep, 'Sleep',\
	       WaitForSingleObject, 'WaitForSingleObject'

	import user32,\
	       MessageBox, 'MessageBoxA'