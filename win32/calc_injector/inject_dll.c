#include <Windows.h>

BOOL WINAPI DllMain(
        __in HINSTANCE hInstance, 
        __in DWORD fdwReason,
        __in LPVOID lpvReserved)
{
        switch(fdwReason) {
                case DLL_PROCESS_ATTACH: {
                        MessageBox(NULL, TEXT("Hello reversers!"), TEXT("Greetings"), MB_OK);
                } break;
                case DLL_PROCESS_DETACH: break;
                case DLL_THREAD_ATTACH: break;
                case DLL_THREAD_DETACH: break;
        }

        return 0;
}