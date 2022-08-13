#include "framework.h"

#include "spy.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH: {
            // MessageBox(NULL, L"InitSpy", L"DllMain", 0);
            InitSpy(hModule);
            break;
        }
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            break;
        case DLL_PROCESS_DETACH: {
            // MessageBox(NULL, L"DestroySpy", L"DllMain", 0);
            DestroySpy();
            break;
        }
    }
    return TRUE;
}
