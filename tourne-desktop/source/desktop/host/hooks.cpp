// Hooks inside Tourne Desktop's own process, for the modules built to run beside the Windhawk suite
// (Icon Redirect: the icons Tourne Desktop itself draws, since the suite stays out of it). Never in
// another program: that's the suite's job. MinHook (third_party\minhook) does the patching.
//
// As Windhawk does: hooks a module sets while it starts are put in once it has started, and taken
// out if it fails. A module that stops has its hooks turned off, not removed, so a thread still
// inside one can call on through it; starting again turns the same ones back on.

#include <windows.h>

#include <cstdarg>
#include <string>
#include <vector>

#include "MinHook.h"
#include "td.h"

namespace td {

namespace {

struct Hook {
    std::wstring owner;  // the module's prefix
    void* target;
    void* trampoline;
};

SRWLOCK g_lock = SRWLOCK_INIT;
bool g_ready = false;
std::vector<Hook> g_hooks;

void Log(const wchar_t* format, ...) {
    va_list args;
    va_start(args, format);
    LogV(L"hooks", format, args);
    va_end(args);
}

void Apply() {
    MH_STATUS s = MH_ApplyQueued();
    if (s != MH_OK) Log(L"applying failed (%S)", MH_StatusToString(s));
}

}  // namespace

BOOL SetFunctionHook(const wchar_t* owner, void* target, void* hook, void** original) {
    if (!target || !hook || !original) return FALSE;
    AcquireSRWLockExclusive(&g_lock);
    BOOL ok = FALSE;
    if (!g_ready) {
        MH_STATUS s = MH_Initialize();
        g_ready = s == MH_OK || s == MH_ERROR_ALREADY_INITIALIZED;
        if (!g_ready) Log(L"MinHook didn't start (%S)", MH_StatusToString(s));
    }
    if (g_ready) {
        Hook* made = nullptr;
        for (auto& h : g_hooks)
            if (h.target == target) made = &h;
        if (made) {
            // (made by an earlier start of the same module: the same hook again)
            ok = made->owner == owner;
            if (ok) *original = made->trampoline;
        } else {
            void* trampoline = nullptr;
            MH_STATUS s = MH_CreateHook(target, hook, &trampoline);
            if (s == MH_OK) {
                g_hooks.push_back({owner, target, trampoline});
                *original = trampoline;
                ok = TRUE;
            } else {
                Log(L"%s couldn't hook %p (%S)", owner, target, MH_StatusToString(s));
            }
        }
        if (ok) ok = MH_QueueEnableHook(target) == MH_OK;
    }
    ReleaseSRWLockExclusive(&g_lock);
    return ok;
}

void ApplyHooks() {
    AcquireSRWLockExclusive(&g_lock);
    if (g_ready) Apply();
    ReleaseSRWLockExclusive(&g_lock);
}

void DisableHooks(const wchar_t* owner) {
    AcquireSRWLockExclusive(&g_lock);
    bool any = false;
    for (auto const& h : g_hooks)
        if (h.owner == owner) any = MH_QueueDisableHook(h.target) == MH_OK || any;
    if (any) Apply();
    ReleaseSRWLockExclusive(&g_lock);
}

}  // namespace td
