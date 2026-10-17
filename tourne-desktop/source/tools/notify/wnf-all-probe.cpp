// Subscribes to every well-known WNF state name (HKLM\SYSTEM\CurrentControlSet\Control\Notifications)
// for a while and prints each change with its time, so a toast sent partway through shows which names
// fire on arrival and not otherwise. Reads nothing else.
//
//   wnf-all-probe.exe 20 [name]     (seconds; the name's data is printed as text)
#include <windows.h>

#include <cstdint>
#include <cstdio>
#include <cwchar>

using WnfCallback = LONG(NTAPI*)(uint64_t state, ULONG stamp, void* type, void* context, const void* buffer, ULONG size);
using Subscribe = LONG(NTAPI*)(void** subscription, uint64_t state, ULONG stamp, WnfCallback callback, void* context,
                               void* type, ULONG group, ULONG unknown);
using QueryState = LONG(NTAPI*)(const uint64_t* state, void* type, const void* scope, ULONG* stamp, void* buffer,
                                ULONG* size);

static uint64_t g_dump;  // (one name whose data is printed as text too)

static LONG NTAPI Changed(uint64_t state, ULONG stamp, void*, void*, const void* buffer, ULONG size) {
    printf("%lu %016llX stamp %lu size %lu\n", GetTickCount(), (unsigned long long)state, stamp, size);
    if (state == g_dump && buffer) {
        auto* bytes = (const unsigned char*)buffer;
        for (ULONG i = 0; i < size; i++) putchar(bytes[i] >= 32 && bytes[i] < 127 ? bytes[i] : '.');
        putchar('\n');
    }
    fflush(stdout);
    return 0;
}

int wmain(int argc, wchar_t** argv) {
    int seconds = argc > 1 ? _wtoi(argv[1]) : 20;
    if (argc > 2) g_dump = wcstoull(argv[2], nullptr, 16);
    HMODULE nt = GetModuleHandleW(L"ntdll.dll");
    auto subscribe = (Subscribe)GetProcAddress(nt, "RtlSubscribeWnfStateChangeNotification");
    auto query = (QueryState)GetProcAddress(nt, "NtQueryWnfStateData");
    if (!subscribe || !query) return 1;
    HKEY key;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control\\Notifications", 0, KEY_READ, &key))
        return 1;
    int ok = 0, failed = 0;
    for (DWORD i = 0;; i++) {
        wchar_t name[64];
        DWORD length = 64;
        if (RegEnumValueW(key, i, name, &length, nullptr, nullptr, nullptr, nullptr)) break;
        uint64_t state = wcstoull(name, nullptr, 16);
        if (!state) continue;
        // Subscribe from the current stamp so only later changes are reported.
        ULONG stamp = 0, size = 0;
        query(&state, nullptr, nullptr, &stamp, nullptr, &size);
        void* subscription = nullptr;
        if (subscribe(&subscription, state, stamp, Changed, nullptr, nullptr, 0, 0) == 0) ok++;
        else failed++;
    }
    RegCloseKey(key);
    printf("start %lu subscribed %d failed %d\n", GetTickCount(), ok, failed);
    fflush(stdout);
    Sleep(seconds * 1000);
    return 0;
}
