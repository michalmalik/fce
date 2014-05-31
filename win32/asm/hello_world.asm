format PE GUI
entry start

include 'macro/import32.inc'

section '.text' code readable executable

	start:
		mov eax, caption
		mov ebx, message

		push 0
		push caption
		push message
		push 0
		call [MessageBox]

		push 0
		call [ExitProcess]

section '.data' code readable writeable

	message 	db "Generic window text",0
	caption 	db "Generic window title",0

section '.idata' import data readable

	library kernel32,'KERNEL32.DLL',\
		user32,'USER32.DLL'

	import kernel32,\
	       ExitProcess, 'ExitProcess'

	import user32,\
	       MessageBox, 'MessageBoxA'

section '.reloc' fixups data readable discardable