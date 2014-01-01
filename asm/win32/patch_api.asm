format PE GUI
entry start

include 'macro/import32.inc'

section '.text' code readable writeable executable

	start:
		jmp patch

		org_msgbox:

		push 0
		push org_caption
		push org_message
		push 0
		call [MessageBox]

		exit:

		push 0
		call [ExitProcess]

section '.dvrnd' code readable executable

	patch:
		mov esi,dword [MessageBox]
		mov dword [mb_copy],esi
		mov dword [MessageBox],ptc_msgbox

		jmp org_msgbox

		ptc_msgbox:

		push 0
		push ptc_caption
		push ptc_message
		push 0
		call [mb_copy]

		jmp exit

section '.data' data readable writeable

	org_caption		db 'Original caption',0
	org_message		db 'Original message',0
	ptc_caption		db 'Patched caption',0
	ptc_message		db 'Patched message',0

	mb_copy 		dd 0

section '.import' import readable writeable

	library kernel32, 'KERNEL32.DLL',\
		user32, 'USER32.DLL'

	import kernel32,\
	       ExitProcess, 'ExitProcess'

	import user32,\
	       MessageBox, 'MessageBoxA'