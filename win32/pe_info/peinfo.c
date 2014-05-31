#include <Windows.h>
#include <TlHelp32.h>
#include <stdio.h>

#define FAULT() {                               \
        printf("Error: %d\n", GetLastError());  \
        CloseHandle(tlh);                       \
        return -1;                              \
}

int htoi(char *s) {
	int n = 0, t = 0;
	int i = 0;

	while(s[i] != '\0') {
		int c = (int)s[i];
		if(c >= 'A' && c <= 'F') {
			t = c-'A'+10;
		} else if(c >= 'a' && c <= 'f') {
			t = c-'a'+10;
		} else if(c >= '0' && c <= '9') {
			t = c-'0';
		}
		n = n*16 + t;
		i++;
	}

	return n;
}

int main(int argc, char **argv)
{
        HANDLE hProcess;
        HMODULE hModule;
        HANDLE tlh;
        MODULEENTRY32 modEntry;
        DWORD pid;
        DWORD image_base;
        DWORD bytes_read;
        IMAGE_DOS_HEADER dos_header;
        IMAGE_NT_HEADERS nt_header;

        if(argc < 2) {
                printf("invalid PID\n");
                return -1;
        }

        pid = (DWORD)htoi(argv[1]);
        tlh = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
        hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
        modEntry.dwSize = sizeof(MODULEENTRY32);
        Module32First(tlh, &modEntry);

        hModule = modEntry.hModule;
        image_base = (DWORD)hModule;

        CloseHandle(tlh);

        if(!ReadProcessMemory(hProcess, (LPCVOID *)image_base, &dos_header, sizeof(IMAGE_DOS_HEADER), &bytes_read))
                FAULT();

        if(!ReadProcessMemory(hProcess, (LPCVOID *)(image_base+dos_header.e_lfanew), &nt_header, sizeof(IMAGE_NT_HEADERS), &bytes_read))
                FAULT();

        printf("Image base: 0x%08x\n", nt_header.OptionalHeader.ImageBase);
        printf("PE signature: 0x%04x\n", nt_header.Signature);
        printf("Size of image: 0x%08x\n", nt_header.OptionalHeader.SizeOfImage);
        printf("Entry point: 0x%08x\n", nt_header.OptionalHeader.AddressOfEntryPoint);

        return 0;
}