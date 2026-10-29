#include "crash.h"

#include <windows.h>
#include <psapi.h>
#include <shellapi.h>

#include <cstdio>
#include <exception>
#include <stdexcept>
#include <mutex>
#include <string>
#include <vector>

// The build script writes build-stamp.h (commit and date) so crash.txt says which build crashed.
#if __has_include("build-stamp.h")
#include "build-stamp.h"
#endif
#ifndef TD_BUILD
#define TD_BUILD L"unknown"
#endif

namespace td {

namespace {

constexpr wchar_t kKey[] = L"Software\\Tourne\\Desktop";
constexpr wchar_t kStarting[] = L"Starting", kStartingTitle[] = L"StartingTitle";
constexpr wchar_t kLeftOff[] = L"LeftOff", kLeftOffTitle[] = L"LeftOffTitle";

std::wstring g_folder;
std::mutex g_lock;
std::vector<std::wstring> g_lines;  // the last log lines
std::wstring g_now, g_nowTitle;     // what's starting (the host's thread writes; a crash reads)
std::wstring g_leftOff, g_leftOffTitle;
LONG g_reporting = 0;

std::wstring ReadValue(const wchar_t* name) {
    wchar_t text[256] = L"";
    DWORD size = sizeof(text);
    if (RegGetValueW(HKEY_CURRENT_USER, kKey, name, RRF_RT_REG_SZ, nullptr, text, &size) != ERROR_SUCCESS) return L"";
    return text;
}

void WriteValue(const wchar_t* name, const std::wstring& v) {
    if (v.empty()) {
        RegDeleteKeyValueW(HKEY_CURRENT_USER, kKey, name);
        return;
    }
    RegSetKeyValueW(HKEY_CURRENT_USER, kKey, name, REG_SZ, v.c_str(), (DWORD)((v.size() + 1) * sizeof(wchar_t)));
}

// An address as module + offset (the exe's offsets named by TourneDesktop.symbols.exe).
std::wstring Where(DWORD64 address) {
    HMODULE module = nullptr;
    wchar_t text[400];
    if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCWSTR)address,
                           &module) && module) {
        wchar_t path[MAX_PATH] = L"";
        GetModuleFileNameW(module, path, MAX_PATH);
        const wchar_t* name = wcsrchr(path, L'\\');
        // Also the address as linked, which TourneDesktop.symbols.exe names (crash.h).
        auto* dos = (IMAGE_DOS_HEADER*)module;
        auto* nt = (IMAGE_NT_HEADERS*)((BYTE*)module + dos->e_lfanew);
        DWORD64 linked = nt->OptionalHeader.ImageBase;
        if (module == (HMODULE)GetModuleHandleW(nullptr))
            swprintf(text, 400, L"%ls+0x%llX (linked 0x%llX)", name ? name + 1 : path, address - (DWORD64)module,
                     address - (DWORD64)module + linked);
        else
            swprintf(text, 400, L"%ls+0x%llX", name ? name + 1 : path, address - (DWORD64)module);
    } else {
        swprintf(text, 400, L"0x%llX", address);
    }
    return text;
}

std::wstring Stack(CONTEXT context) {
    std::wstring out;
#ifdef _WIN64
    for (int i = 0; i < 40 && context.Rip; i++) {
        out += L"  " + Where(context.Rip) + L"\r\n";
        DWORD64 base = 0;
        RUNTIME_FUNCTION* fn = RtlLookupFunctionEntry(context.Rip, &base, nullptr);
        if (!fn) {  // a leaf: its return address is on top of the stack
            if (IsBadReadPtr((void*)context.Rsp, 8)) break;
            context.Rip = *(DWORD64*)context.Rsp;
            context.Rsp += 8;
            continue;
        }
        void* handlerData = nullptr;
        DWORD64 frame = 0;
        RtlVirtualUnwind(UNW_FLAG_NHANDLER, base, context.Rip, fn, &context, &handlerData, &frame, nullptr);
    }
#else
    (void)context;
#endif
    return out;
}

const wchar_t* CodeName(DWORD code) {
    switch (code) {
        case EXCEPTION_ACCESS_VIOLATION: return L"access violation";
        case EXCEPTION_STACK_OVERFLOW: return L"stack overflow";
        case EXCEPTION_ILLEGAL_INSTRUCTION: return L"illegal instruction";
        case EXCEPTION_INT_DIVIDE_BY_ZERO: return L"divide by zero";
        case EXCEPTION_ARRAY_BOUNDS_EXCEEDED: return L"array bounds";
        case 0xE06D7363: return L"C++ exception";
        case 0xE0000001: return L"C++ exception nobody caught (std::terminate)";
        default: return L"";
    }
}

std::string g_what;  // what the uncaught C++ exception said

void Report(EXCEPTION_POINTERS* info) {
    if (InterlockedExchange(&g_reporting, 1)) return;  // (one report, even if two threads fail)
    EXCEPTION_RECORD const& r = *info->ExceptionRecord;
    std::wstring step = g_now.empty() ? L"(running)" : g_nowTitle + L" (" + g_now + L")";
    // Next start leaves off what was starting.
    if (!g_now.empty()) {
        WriteValue(kLeftOff, g_now);
        WriteValue(kLeftOffTitle, g_nowTitle);
    }
    SYSTEMTIME t;
    GetLocalTime(&t);
    wchar_t head[1024];
    swprintf(head, 1024,
             L"Tourne Desktop crashed, %04d-%02d-%02d %02d:%02d:%02d\r\n"
             L"Build: " TD_BUILD L"\r\n"
             L"Starting: %ls\r\n"
             L"Error: 0x%08lX %ls\r\n"
             L"At: %ls\r\n",
             t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond, step.c_str(), r.ExceptionCode, CodeName(r.ExceptionCode),
             Where((DWORD64)r.ExceptionAddress).c_str());
    std::wstring text = head;
    if (r.ExceptionCode == EXCEPTION_ACCESS_VIOLATION && r.NumberParameters >= 2) {
        wchar_t av[128];
        swprintf(av, 128, L"It tried to %ls 0x%llX\r\n", r.ExceptionInformation[0] == 1 ? L"write" : r.ExceptionInformation[0] == 8 ? L"run" : L"read",
                 (unsigned long long)r.ExceptionInformation[1]);
        text += av;
    }
    if (!g_what.empty()) {
        std::wstring what(g_what.begin(), g_what.end());
        text += L"What: " + what + L"\r\n";
    }
    text += L"\r\nStack:\r\n" + Stack(*info->ContextRecord);
    text += L"\r\nLast log lines:\r\n";
    {
        std::unique_lock<std::mutex> lock(g_lock, std::try_to_lock);  // (not waited for: the crash may hold it)
        if (lock.owns_lock())
            for (auto const& l : g_lines) text += L"  " + l + L"\r\n";
    }
    std::wstring path = g_folder + L"\\crash.txt";
    HANDLE f = CreateFileW(path.c_str(), GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, 0, nullptr);
    if (f != INVALID_HANDLE_VALUE) {
        int n = WideCharToMultiByte(CP_UTF8, 0, text.c_str(), (int)text.size(), nullptr, 0, nullptr, nullptr);
        std::string bytes(n, 0);
        WideCharToMultiByte(CP_UTF8, 0, text.c_str(), (int)text.size(), bytes.data(), n, nullptr, nullptr);
        DWORD written;
        WriteFile(f, bytes.data(), (DWORD)bytes.size(), &written, nullptr);
        CloseHandle(f);
    }
    static std::wstring message;
    message = L"Tourne Desktop crashed";
    if (!g_now.empty()) message += L" while starting " + g_nowTitle + L". Next time it starts without it, until you save a setting";
    message += L".\n\nWhat happened is in\n" + path + L"\nAttach that file to a bug report. Open it now?";
    static std::wstring opened;
    opened = path;
    // Asked from a thread of its own while this one waits: a message box here would run this thread's
    // windows again, and the same fault would strike again inside the report.
    auto ask = [](LPVOID) -> DWORD {
        if (MessageBoxW(nullptr, message.c_str(), L"Tourne Desktop", MB_YESNO | MB_ICONERROR | MB_SETFOREGROUND | MB_TOPMOST) == IDYES)
            ShellExecuteW(nullptr, L"open", opened.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
        return 0;
    };
    if (HANDLE t = CreateThread(nullptr, 0, ask, nullptr, 0, nullptr)) {
        WaitForSingleObject(t, INFINITE);
        CloseHandle(t);
    }
}

LONG WINAPI Unhandled(EXCEPTION_POINTERS* info) {
    Report(info);
    return EXCEPTION_EXECUTE_HANDLER;
}

[[noreturn]] void Terminate() {
    if (auto e = std::current_exception()) {
        try {
            std::rethrow_exception(e);
        } catch (std::exception const& x) {
            g_what = x.what();
        } catch (...) {
            g_what = "(not a std::exception)";
        }
    }
    // An exception thrown and not caught: reported like a crash, from here.
    CONTEXT context{};
    RtlCaptureContext(&context);
    EXCEPTION_RECORD record{};
    record.ExceptionCode = 0xE0000001;
#ifdef _WIN64
    record.ExceptionAddress = (void*)context.Rip;
#endif
    EXCEPTION_POINTERS info{&record, &context};
    Report(&info);
    TerminateProcess(GetCurrentProcess(), 3);
    for (;;) {}
}

}  // namespace

void CrashGuardInstall(const std::wstring& folder) {
    g_folder = folder;
    SetUnhandledExceptionFilter(Unhandled);
    std::set_terminate(Terminate);
    // A stack overflow needs room to report in.
    ULONG reserve = 64 * 1024;
    SetThreadStackGuarantee(&reserve);
}

void CrashGuardBegin() {
    g_leftOff = ReadValue(kLeftOff);
    g_leftOffTitle = ReadValue(kLeftOffTitle);
    // Died while starting something, before the crash handler could say (a fast fail): that too.
    std::wstring starting = ReadValue(kStarting);
    if (g_leftOff.empty() && !starting.empty()) {
        g_leftOff = starting;
        g_leftOffTitle = ReadValue(kStartingTitle);
        WriteValue(kLeftOff, g_leftOff);
        WriteValue(kLeftOffTitle, g_leftOffTitle);
    }
    WriteValue(kStarting, L"");
    WriteValue(kStartingTitle, L"");
}

void CrashNote(const std::wstring& line) {
    std::lock_guard<std::mutex> lock(g_lock);
    g_lines.push_back(line);
    if (g_lines.size() > 60) g_lines.erase(g_lines.begin());
}

void CrashStarting(const std::wstring& part, const std::wstring& title) {
    g_now = part, g_nowTitle = title;
    WriteValue(kStarting, part);
    WriteValue(kStartingTitle, title);
}

void CrashStarted() {
    g_now.clear(), g_nowTitle.clear();
    WriteValue(kStarting, L"");
    WriteValue(kStartingTitle, L"");
}

std::wstring CrashLeftOff() { return g_leftOff; }
std::wstring CrashLeftOffTitle() { return g_leftOffTitle; }
bool CrashLeavesOff(const std::wstring& part) { return !g_leftOff.empty() && g_leftOff == part; }

void CrashTryAgain() {
    if (g_leftOff.empty()) return;
    g_leftOff.clear(), g_leftOffTitle.clear();
    WriteValue(kLeftOff, L"");
    WriteValue(kLeftOffTitle, L"");
}

}  // namespace td
