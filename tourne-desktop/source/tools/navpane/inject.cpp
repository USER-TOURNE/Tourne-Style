// inject <pid> <dll>: loads the DLL into the process (the test folder window).
#include <windows.h>
#include <cstdio>
#include <cwchar>
int wmain(int argc, wchar_t** argv) {
    HANDLE p = OpenProcess(PROCESS_ALL_ACCESS, FALSE, (DWORD)_wtoi(argv[1]));
    if (!p) { wprintf(L"open %lu\n", GetLastError()); return 1; }
    size_t n = (wcslen(argv[2]) + 1) * 2;
    void* mem = VirtualAllocEx(p, nullptr, n, MEM_COMMIT, PAGE_READWRITE);
    WriteProcessMemory(p, mem, argv[2], n, nullptr);
    HANDLE t = CreateRemoteThread(p, nullptr, 0, (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "LoadLibraryW"), mem, 0, nullptr);
    if (!t) { wprintf(L"thread %lu\n", GetLastError()); return 1; }
    WaitForSingleObject(t, 5000);
    DWORD code = 0;
    GetExitCodeThread(t, &code);
    wprintf(L"loaded: %ls\n", code ? L"yes" : L"no");
    return 0;
}
