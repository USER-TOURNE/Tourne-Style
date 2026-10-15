// sizerprobe.dll: loaded into a test folder window only. Patches shell32's
// CDUISizerElement::_ComputeBoundedSize (RVAs for this PC's 26100 shell32) and logs each call to
// X:\...\navpane\sizerprobe.txt. With navpane\sizerprobe-relax.txt holding 1, the side pane's lower
// bound is relaxed to 40 (what Explorer Style's hook does). Built with:
//   clang++ -target x86_64-w64-mingw32 -O1 -shared sizerprobe.cpp -o sizerprobe.dll -static
//   clang++ -target x86_64-w64-mingw32 -O1 -municode inject.cpp -o inject.exe -static
#include <windows.h>
#include <cstdio>
#include <cstring>
#include <cstdint>
static const wchar_t* kLog = L"%USERPROFILE%\\Downloads\\tourne-suite-sab-clone\\tools\\navpane\\sizerprobe.txt";
static const wchar_t* kRelax = L"%USERPROFILE%\\Downloads\\tourne-suite-sab-clone\\tools\\navpane\\sizerprobe-relax.txt";
typedef int (*Bounded_t)(void*, int);
typedef unsigned short (*Ushort_t)(void*);
typedef int (*Int_t)(void*);
static Bounded_t g_orig;
static Ushort_t g_target;
static Int_t g_min, g_reserved;
static unsigned short g_reading;
static bool g_relax;
static int Hook(void* sizer, int size) {
    int bounded = g_orig(sizer, size);
    if (!g_reading)
        if (HMODULE dui = GetModuleHandleW(L"dui70.dll"))
            if (auto f = (unsigned short (*)(const wchar_t*))GetProcAddress(dui, "StrToID")) g_reading = f(L"ReadingPane");
    unsigned short t = g_target(sizer);
    int out = bounded;
    WCHAR tname[64] = L"?", sname[64] = L"?";
    if (!GetAtomNameW(t, tname, 64)) GlobalGetAtomNameW(t, tname, 64);
    if (HMODULE dui = GetModuleHandleW(L"dui70.dll"))
        if (auto getId = (unsigned short (*)(void*))GetProcAddress(dui, "?GetID@Element@DirectUI@@QEAAGXZ"))
            if (unsigned short id = getId(sizer); !GetAtomNameW(id, sname, 64)) GlobalGetAtomNameW(id, sname, 64);
    if (g_relax && (t == g_reading || !wcscmp(tname, L"DetailsContainer")) && size < bounded) out = size < 40 ? 40 : size;
    if (FILE* f = _wfopen(kLog, L"a")) {
        fwprintf(f, L"sizer %ls target %ls (reading atom %u) %u%ls size %d -> %d (out %d) min %d reserved %d\n", sname, tname, g_reading, t, t == g_reading ? L" (reading)" : L"",
                 size, bounded, out, g_min(sizer), g_reserved(sizer));
        fclose(f);
    }
    return out;
}
static DWORD WINAPI Install(LPVOID) {
    auto shell = (uint8_t*)GetModuleHandleW(L"shell32.dll");
    uint8_t* fn = shell + 0x761c8;
    static const uint8_t expect[] = {0x40, 0x55, 0x53, 0x56, 0x57, 0x41, 0x56, 0x41, 0x57, 0x48, 0x8b, 0xec, 0x48, 0x83, 0xec, 0x28};
    FILE* f = _wfopen(kLog, L"a");
    uint8_t* chain = nullptr;  // an existing hook's jmp: chained through, after noting whose it is
    if (memcmp(fn, expect, sizeof(expect))) {
        if (fn[0] != 0xe9) {
            if (f) { fwprintf(f, L"prologue differs:"); for (int i = 0; i < 16; i++) fwprintf(f, L" %02x", fn[i]); fwprintf(f, L"\n"); fclose(f); }
            return 0;
        }
        chain = fn + 5 + *(int32_t*)(fn + 1);
        uint8_t* dest = chain;
        for (int hop = 0; hop < 4; hop++) {  // through jmp stubs to the code that owns it
            HMODULE owner = nullptr;
            WCHAR name[MAX_PATH] = L"(no module)";
            if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCWSTR)dest, &owner))
                GetModuleFileNameW(owner, name, MAX_PATH);
            if (f) fwprintf(f, L"hop %d: %p in %ls, bytes %02x %02x\n", hop, dest, name, dest[0], dest[1]);
            if (dest[0] == 0xff && dest[1] == 0x25) dest = *(uint8_t**)(dest + 6 + *(int32_t*)(dest + 2));
            else if (dest[0] == 0xe9) dest = dest + 5 + *(int32_t*)(dest + 1);
            else break;
        }
    }
    g_target = (Ushort_t)(shell + 0x954f8);
    g_min = (Int_t)(shell + 0x7634c);
    g_reserved = (Int_t)(shell + 0x762f0);
    if (FILE* r = _wfopen(kRelax, L"r")) { g_relax = fgetc(r) == '1'; fclose(r); }
    // Trampoline: the 16 prologue bytes (no rip-relative), then jmp back.
    uint8_t* tramp = nullptr;  // within 2 GB of shell32, so a 5-byte jmp reaches it
    for (uint64_t at = ((uint64_t)shell & ~0xffffull) - 0x10000; !tramp && at > (uint64_t)shell - 0x70000000; at -= 0x10000)
        tramp = (uint8_t*)VirtualAlloc((void*)at, 64, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    uint8_t jmp[14] = {0xff, 0x25, 0, 0, 0, 0};
    if (chain) {
        uint64_t to = (uint64_t)chain;
        memcpy(jmp + 6, &to, 8);
        memcpy(tramp, jmp, 14);
    } else {
        memcpy(tramp, fn, 16);
        uint64_t back = (uint64_t)(fn + 16);
        memcpy(jmp + 6, &back, 8);
        memcpy(tramp + 16, jmp, 14);
    }
    g_orig = (Bounded_t)tramp;
    uint64_t to = (uint64_t)&Hook;
    memcpy(jmp + 6, &to, 8);
    DWORD old;
    VirtualProtect(fn, 16, PAGE_EXECUTE_READWRITE, &old);
    if (chain) {  // a 5-byte jmp to a stub next to the trampoline
        uint8_t* stub = (uint8_t*)tramp + 32;
        memcpy(stub, jmp, 14);
        fn[0] = 0xe9;  // only reachable if within 2 GB: checked below
        int64_t rel = stub - (fn + 5);
        if (rel != (int32_t)rel) { if (f) { fwprintf(f, L"stub too far\n"); fclose(f); } return 0; }
        *(int32_t*)(fn + 1) = (int32_t)rel;
    } else {
        memcpy(fn, jmp, 14);
        fn[14] = 0x90, fn[15] = 0x90;
    }
    VirtualProtect(fn, 16, old, &old);
    FlushInstructionCache(GetCurrentProcess(), fn, 16);
    if (f) { fwprintf(f, L"patched, relax %d, dui70 %p\n", g_relax, GetModuleHandleW(L"dui70.dll")); fclose(f); }
    return 0;
}
BOOL WINAPI DllMain(HINSTANCE, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) CloseHandle(CreateThread(nullptr, 0, Install, nullptr, 0, nullptr));
    return TRUE;
}





