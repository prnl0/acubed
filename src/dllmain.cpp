#include <stdexcept>

#include <Windows.h>

#include "init.h"

DWORD WINAPI main_thread(HMODULE module)
{
    HWND hwnd = ::FindWindowA(nullptr, "AssaultCube");
    if (!hwnd) {
        ::FreeLibraryAndExitThread(module, 1);
        return 1;
    }

    /* TEMPORARY */
    ::AllocConsole();
    FILE *fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);

    try {
        Init::init(hwnd);

        while (true) {
            if (::GetAsyncKeyState(VK_TAB) & 0x1) {
                break;
            }

            Sleep(20);
            //system("cls"); // TEMPORARY
        }
    } catch (const std::exception &e) {
        ::MessageBoxA(hwnd, e.what(), "Fatal error", MB_OK);
    }

    Init::shutdown();

    /* TEMPORARY */
    fclose(fp);
    ::FreeConsole();

    ::Sleep(1000);
    ::FreeLibraryAndExitThread(module, 0);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE module,
                      DWORD reason_for_call,
                      LPVOID reserved)
{
    switch (reason_for_call) {
        case DLL_PROCESS_ATTACH: {
            ::CloseHandle(::CreateThread(nullptr, 0,
                                         reinterpret_cast<LPTHREAD_START_ROUTINE>(main_thread),
                                         module, 0, nullptr));
            break;
        }
    }

    return TRUE;
}