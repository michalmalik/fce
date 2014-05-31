#include <Windows.h>
#include <stdio.h>

#define FAULT() {                               \
        printf("Error: %d\n", GetLastError());  \
        CloseHandle(hProcess);                  \
        return -1;                              \
}

int main(int argc, char **argv)
{
        LPSTR executable = "C:\\Windows\\System32\\calc.exe";
        LPSTR injectDll = "inject.dll";

        STARTUPINFO SI = {0};
        PROCESS_INFORMATION PI = {0};
        HANDLE hProcess;
        DWORD pid;
        DWORD threadID;
        LPVOID injectAddr;
        DWORD lpNumOfWritten;
        HMODULE hKernel32;
        FARPROC hLoadLibrary;

        SI.dwFlags = 1;
        SI.wShowWindow = 0;
        SI.cb = sizeof(SI);

        if(CreateProcess(executable, NULL, NULL, NULL, NULL, CREATE_NEW_CONSOLE, NULL, NULL, &SI, &PI)) {
                printf("[OK] CreateProcess %s\n", executable);
        } else
                FAULT();

        pid = PI.dwProcessId;
        hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

        injectAddr = VirtualAllocEx(hProcess, NULL, strlen(injectDll), MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);

        if(injectAddr == NULL)
                FAULT();

        printf("[OK] VirtualAllocEx to 0x%08X\n", (int *)injectAddr);

        if(WriteProcessMemory(hProcess, injectAddr, injectDll, strlen(injectDll), &lpNumOfWritten) == 0)
                FAULT();

        printf("[OK] WriteProcessMememory to 0x%08X :: \"%s\",%d\n", (int *)injectAddr, injectDll, strlen(injectDll));

        hKernel32 = GetModuleHandle("kernel32.dll");
        hLoadLibrary = (FARPROC)GetProcAddress(hKernel32, "LoadLibraryA");

        Sleep(2000);

        if(CreateRemoteThread(hProcess, NULL, NULL, (LPTHREAD_START_ROUTINE)hLoadLibrary, injectAddr, NULL, &threadID) == NULL)
                FAULT();

        printf("[OK] CreateRemoteThread at 0x%08X :: LoadLibrary(\"%s\")\n", (int *)injectAddr, injectDll);

        getchar();
        return 0;
}