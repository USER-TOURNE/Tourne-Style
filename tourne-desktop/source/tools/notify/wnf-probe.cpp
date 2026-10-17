// Which Windows notification state changes when a toast arrives? Subscribes to WNF state names (the
// shell's own notification signals) for a while and prints each change, so a toast sent meanwhile shows
// which ones fire. Reads nothing else.
//
//   wnf-probe.exe 20     (seconds)
#include <windows.h>

#include <cstdint>
#include <cstdio>

using WnfCallback = LONG(NTAPI*)(uint64_t state, ULONG stamp, void* type, void* context, const void* buffer, ULONG size);
using Subscribe = LONG(NTAPI*)(void** subscription, uint64_t state, ULONG stamp, WnfCallback callback, void* context,
                               void* type, ULONG group, ULONG unknown);

struct Name {
    uint64_t state;
    const char* name;
};
static const Name kNames[] = {
    {0x0D83063EA3BC1035ull, "WNF_SHEL_NOTIFICATIONS"},
    {0x0D83063EA3BC1835ull, "WNF_SHEL_NOTIFICATIONS (next)"},
    {0x0D83063EA3BD0035ull, "WNF_SHEL_TOAST_PUBLISHED?"},
    {0x0D83063EA3BF1C75ull, "WNF_SHEL_QUIET_MOMENT_SHELL_MODE_CHANGED"},
};

static LONG NTAPI Changed(uint64_t, ULONG stamp, void*, void* context, const void* buffer, ULONG size) {
    unsigned value = size >= 4 ? *(const unsigned*)buffer : 0;
    printf("%lu ms  %s  stamp %lu  size %lu  first dword %u\n", GetTickCount() % 100000, (const char*)context, stamp, size, value);
    fflush(stdout);
    return 0;
}

int main(int argc, char** argv) {
    int seconds = argc > 1 ? atoi(argv[1]) : 20;
    auto subscribe = (Subscribe)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "RtlSubscribeWnfStateChangeNotification");
    if (!subscribe) return 1;
    for (auto const& n : kNames) {
        void* subscription = nullptr;
        LONG status = subscribe(&subscription, n.state, 0, Changed, (void*)n.name, nullptr, 0, 0);
        printf("subscribe %s: 0x%08lX\n", n.name, (unsigned long)status);
    }
    fflush(stdout);
    Sleep(seconds * 1000);
    return 0;
}
