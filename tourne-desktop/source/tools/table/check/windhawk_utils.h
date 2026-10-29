#pragma once
#include <windows.h>
#include <commctrl.h>
namespace WindhawkUtils {
struct StringSetting { PCWSTR s = L""; static StringSetting make(PCWSTR, ...) { return {}; } PCWSTR get() const { return s; } operator PCWSTR() const { return s; } };
inline BOOL SetWindowSubclassFromAnyThread(HWND, SUBCLASSPROC, DWORD_PTR) { return TRUE; }
inline BOOL RemoveWindowSubclassFromAnyThread(HWND, SUBCLASSPROC) { return TRUE; }
}
