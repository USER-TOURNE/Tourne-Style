// ==WindhawkMod==
// @id              tourne-hid-watch
// @name            Tourne HID Watch (temporary)
// @description     Logs what Alienware Command Center sends to and reads from the Alienware headset and mouse, to learn their battery query. Log only; remove when done.
// @version         1.5
// @author          USER-TOURNE
// @include         AlienFXSubAgent.exe
// @include         AWCC.UCSubAgent.exe
// @include         AWCC.exe
// @license         MIT
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
A diagnostic, not a feature. It watches only handles AWCC opens to the
Alienware Pro Wireless headset (413C:A529) and the AW720M mouse (413C:3029),
and appends every report written to or read from them to a text file in this
mod's storage folder:

C:\ProgramData\Windhawk\Engine\ModsWritable\mod-storage\local@tourne-hid-watch

It never changes or sends anything itself. Turn it off or remove it once the
battery query is known.
*/
// ==/WindhawkModReadme==

#include <windows.h>
#include <winioctl.h>

#include <map>
#include <mutex>
#include <string>

std::mutex g_lock;

// Every hook hands the caller the last-error value the real function left:
// logging calls other functions, and .NET reads the error right after I/O.
struct KeepLastError {
    DWORD error = GetLastError();
    ~KeepLastError() { SetLastError(error); }
};
std::map<HANDLE, std::wstring> g_handles;  // watched handle -> short device name

struct PendingRead {
    HANDLE handle;
    BYTE* buffer;
    DWORD length;
};
std::map<LPOVERLAPPED, PendingRead> g_pending;
HANDLE g_log = INVALID_HANDLE_VALUE;

void Write(const std::string& line) {
    if (g_log == INVALID_HANDLE_VALUE) {
        return;
    }
    DWORD written;
    WriteFile(g_log, line.data(), (DWORD)line.size(), &written, nullptr);
}

std::string Narrow(const std::wstring& s) {
    std::string out;
    for (wchar_t c : s) {
        out += c < 128 ? (char)c : '?';
    }
    return out;
}

// One line: time, device, what, then the bytes. Bytes are copied with
// ReadProcessMemory, so a buffer freed early can't crash the host.
void Log(HANDLE handle, const char* what, const void* data, DWORD length) {
    std::lock_guard<std::mutex> guard(g_lock);
    auto it = g_handles.find(handle);
    // Handles opened before the watcher loaded are logged by what the data
    // looks like; they show as "hid?" with the handle value.
    char unknown[32];
    snprintf(unknown, sizeof(unknown), "hid? %p", handle);
    std::string device = it == g_handles.end() ? std::string(unknown) : Narrow(it->second);
    SYSTEMTIME st;
    GetLocalTime(&st);
    char head[160];
    snprintf(head, sizeof(head), "%02d:%02d:%02d.%03d %s %s (%lu):", st.wHour, st.wMinute, st.wSecond,
             st.wMilliseconds, device.c_str(), what, length);
    std::string line = head;
    BYTE copy[1024];
    DWORD n = length < sizeof(copy) ? length : sizeof(copy);
    SIZE_T got = 0;
    if (data && n && ReadProcessMemory(GetCurrentProcess(), data, copy, n, &got)) {
        // Trailing zeros trimmed, so replies stay readable.
        while (got > 1 && copy[got - 1] == 0) {
            got--;
        }
        for (SIZE_T i = 0; i < got; i++) {
            char hex[4];
            snprintf(hex, sizeof(hex), " %02x", copy[i]);
            line += hex;
        }
    }
    line += "\r\n";
    Write(line);
}

bool IsWatched(HANDLE handle) {
    std::lock_guard<std::mutex> guard(g_lock);
    return g_handles.count(handle) != 0;
}

// The first bytes of a buffer, copied safely.
bool Peek(const void* data, DWORD length, BYTE* out, DWORD n) {
    SIZE_T got = 0;
    return data && length >= n && ReadProcessMemory(GetCurrentProcess(), data, out, n, &got) && got == n;
}

// Writes that look like the headset's command report (report 06, 62 bytes)
// or the mouse's (report 0 then c0, 65 bytes), on any handle.
bool LooksLikeCommand(const void* data, DWORD length) {
    BYTE b[2];
    if (length == 62 && Peek(data, length, b, 1)) {
        return b[0] == 0x06;
    }
    if (length == 65 && Peek(data, length, b, 2)) {
        return b[0] == 0x00 && b[1] == 0xC0;
    }
    return false;
}

// Reads the sizes of those devices' replies. The buffer of a read is logged
// when the next read on the same handle starts: by then it has completed,
// however its completion was delivered.
bool LooksLikeReplySize(DWORD length) {
    return length == 62 || length == 65 || length == 8;
}
std::map<HANDLE, PendingRead> g_lastRead;

using CreateFileW_t = decltype(&CreateFileW);
CreateFileW_t CreateFileW_Original;
HANDLE WINAPI CreateFileW_Hook(LPCWSTR name, DWORD access, DWORD share, LPSECURITY_ATTRIBUTES sa, DWORD disposition,
                               DWORD flags, HANDLE templateFile) {
    HANDLE h = CreateFileW_Original(name, access, share, sa, disposition, flags, templateFile);
    KeepLastError keep;
    if (h != INVALID_HANDLE_VALUE && name) {
        std::wstring lower = name;
        for (auto& c : lower) c = towlower(c);
        const wchar_t* device = lower.find(L"vid_413c&pid_a529") != std::wstring::npos   ? L"headset"
                                : lower.find(L"vid_413c&pid_3029") != std::wstring::npos ? L"mouse"
                                                                                         : nullptr;
        if (device) {
            std::wstring label = std::wstring(device);
            size_t mi = lower.find(L"&mi_");
            if (mi != std::wstring::npos) {
                label += L" " + lower.substr(mi + 1, lower.find(L'#', mi) - mi - 1);
            }
            {
                std::lock_guard<std::mutex> guard(g_lock);
                g_handles[h] = label;
            }
            Log(h, "open", nullptr, 0);
        }
    }
    return h;
}

using CloseHandle_t = decltype(&CloseHandle);
CloseHandle_t CloseHandle_Original;
BOOL WINAPI CloseHandle_Hook(HANDLE h) {
    {
        KeepLastError keep;
        std::lock_guard<std::mutex> guard(g_lock);
        g_handles.erase(h);
        g_lastRead.erase(h);
    }
    return CloseHandle_Original(h);
}

using WriteFile_t = decltype(&WriteFile);
WriteFile_t WriteFile_Original;
BOOL WINAPI WriteFile_Hook(HANDLE h, LPCVOID buffer, DWORD length, LPDWORD written, LPOVERLAPPED ov) {
    {
    KeepLastError keep;
    if (h != g_log && (IsWatched(h) || LooksLikeCommand(buffer, length))) {
        Log(h, "write", buffer, length);
    }
    }
    return WriteFile_Original(h, buffer, length, written, ov);
}

using ReadFile_t = decltype(&ReadFile);
ReadFile_t ReadFile_Original;
BOOL WINAPI ReadFile_Hook(HANDLE h, LPVOID buffer, DWORD length, LPDWORD read, LPOVERLAPPED ov) {
    if (KeepLastError keep; h != g_log && LooksLikeReplySize(length)) {
        PendingRead previous{};
        bool had = false;
        {
            std::lock_guard<std::mutex> guard(g_lock);
            auto it = g_lastRead.find(h);
            if (it != g_lastRead.end()) {
                previous = it->second;
                had = true;
            }
            g_lastRead[h] = {h, (BYTE*)buffer, length};
        }
        if (had) {
            BYTE first;
            // Skip replies that are all zero: those reads never completed.
            if (Peek(previous.buffer, previous.length, &first, 1)) {
                Log(h, "read (earlier)", previous.buffer, previous.length);
            }
        }
    }
    BOOL ok = ReadFile_Original(h, buffer, length, read, ov);
    KeepLastError keep;
    if (IsWatched(h)) {
        if (ok && !ov && read) {
            Log(h, "read", buffer, *read);
        } else if (ov) {
            // Completes later; logged when its result is collected, or when
            // the next write shows it has had time to land.
            std::lock_guard<std::mutex> guard(g_lock);
            g_pending[ov] = {h, (BYTE*)buffer, length};
        }
    }
    return ok;
}

void FlushPending(LPOVERLAPPED ov, BOOL ok) {
    PendingRead p;
    {
        std::lock_guard<std::mutex> guard(g_lock);
        auto it = g_pending.find(ov);
        if (it == g_pending.end()) {
            return;
        }
        p = it->second;
        g_pending.erase(it);
    }
    if (ok) {
        Log(p.handle, "read", p.buffer, p.length);
    }
}

using GetOverlappedResult_t = decltype(&GetOverlappedResult);
GetOverlappedResult_t GetOverlappedResult_Original;
BOOL WINAPI GetOverlappedResult_Hook(HANDLE h, LPOVERLAPPED ov, LPDWORD n, BOOL wait) {
    BOOL ok = GetOverlappedResult_Original(h, ov, n, wait);
    KeepLastError keep;
    FlushPending(ov, ok);
    return ok;
}

using GetOverlappedResultEx_t = decltype(&GetOverlappedResultEx);
GetOverlappedResultEx_t GetOverlappedResultEx_Original;
BOOL WINAPI GetOverlappedResultEx_Hook(HANDLE h, LPOVERLAPPED ov, LPDWORD n, DWORD ms, BOOL alertable) {
    BOOL ok = GetOverlappedResultEx_Original(h, ov, n, ms, alertable);
    KeepLastError keep;
    FlushPending(ov, ok);
    return ok;
}

// Reads completed through an I/O completion port (.NET) never pass the
// functions above; their buffers are logged when the port hands them back.
using GetQueuedCompletionStatus_t = decltype(&GetQueuedCompletionStatus);
GetQueuedCompletionStatus_t GetQueuedCompletionStatus_Original;
BOOL WINAPI GetQueuedCompletionStatus_Hook(HANDLE port, LPDWORD n, PULONG_PTR key, LPOVERLAPPED* ov, DWORD ms) {
    BOOL ok = GetQueuedCompletionStatus_Original(port, n, key, ov, ms);
    KeepLastError keep;
    if (ov && *ov) {
        FlushPending(*ov, ok);
    }
    return ok;
}

using GetQueuedCompletionStatusEx_t = decltype(&GetQueuedCompletionStatusEx);
GetQueuedCompletionStatusEx_t GetQueuedCompletionStatusEx_Original;
BOOL WINAPI GetQueuedCompletionStatusEx_Hook(HANDLE port, LPOVERLAPPED_ENTRY entries, ULONG count, PULONG removed,
                                             DWORD ms, BOOL alertable) {
    BOOL ok = GetQueuedCompletionStatusEx_Original(port, entries, count, removed, ms, alertable);
    KeepLastError keep;
    if (ok && removed) {
        for (ULONG i = 0; i < *removed; i++) {
            if (entries[i].lpOverlapped) {
                FlushPending(entries[i].lpOverlapped, TRUE);
            }
        }
    }
    return ok;
}

const char* IoctlName(DWORD code) {
    switch (code) {
        case 0xB0191: return "set-feature";
        case 0xB0192: return "get-feature";
        case 0xB0195: return "set-output";
        case 0xB01A2: return "get-input";
        case 0xB01A6: return "get-output";
        default: return nullptr;
    }
}

using DeviceIoControl_t = decltype(&DeviceIoControl);
DeviceIoControl_t DeviceIoControl_Original;
BOOL WINAPI DeviceIoControl_Hook(HANDLE h, DWORD code, LPVOID in, DWORD inLength, LPVOID out, DWORD outLength,
                                 LPDWORD returned, LPOVERLAPPED ov) {
    bool watched, ks;
    const char* name = IoctlName(code);
    {
        KeepLastError keep;
        // HID report requests are logged on any handle: the codes are HID-only.
        // So are kernel streaming property and method requests (audio drivers).
        ks = (code >> 16) == 0x2F && ((code >> 2) & 0xFFF) <= 3;
        watched = h != g_log && (IsWatched(h) || name != nullptr || ks);
        if (watched && name && code != 0xB0192 && code != 0xB01A2 && code != 0xB01A6) {
            // Set requests carry the report in the output buffer for HID.
            Log(h, name, out ? out : in, out ? outLength : inLength);
        } else if (watched && !name) {
            char label[48];
            snprintf(label, sizeof(label), "%s %08lx in", ks ? "ks" : "ioctl", code);
            Log(h, label, in, inLength);
        }
    }
    BOOL ok = DeviceIoControl_Original(h, code, in, inLength, out, outLength, returned, ov);
    KeepLastError keep;
    if (watched && ok && (code == 0xB0192 || code == 0xB01A2 || code == 0xB01A6)) {
        Log(h, name, out, outLength);
    } else if (watched && ok && ks && out && returned) {
        char label[48];
        snprintf(label, sizeof(label), "ks %08lx out", code);
        Log(h, label, out, *returned);
    }
    return ok;
}

BOOL Wh_ModInit() {
    WCHAR folder[MAX_PATH], exe[MAX_PATH], path[MAX_PATH + 64];
    if (Wh_GetModStoragePath(folder, ARRAYSIZE(folder))) {
        GetModuleFileNameW(nullptr, exe, ARRAYSIZE(exe));
        PCWSTR base = wcsrchr(exe, L'\\');
        swprintf_s(path, L"%ls\\%ls-%lu.txt", folder, base ? base + 1 : exe, GetCurrentProcessId());
        g_log = CreateFileW(path, FILE_APPEND_DATA, FILE_SHARE_READ, nullptr, OPEN_ALWAYS, 0, nullptr);
    }
    Write("-- watching\r\n");

    HMODULE kernelbase = GetModuleHandleW(L"kernelbase.dll");
    HMODULE kernel32 = GetModuleHandleW(L"kernel32.dll");
    auto hook = [&](HMODULE m, const char* name, void* detour, void** original) {
        void* target = m ? (void*)GetProcAddress(m, name) : nullptr;
        if (target) {
            Wh_SetFunctionHook(target, detour, original);
        }
    };
    hook(kernelbase, "CreateFileW", (void*)CreateFileW_Hook, (void**)&CreateFileW_Original);
    hook(kernelbase, "CloseHandle", (void*)CloseHandle_Hook, (void**)&CloseHandle_Original);
    hook(kernelbase, "WriteFile", (void*)WriteFile_Hook, (void**)&WriteFile_Original);
    hook(kernelbase, "ReadFile", (void*)ReadFile_Hook, (void**)&ReadFile_Original);
    hook(kernelbase, "GetOverlappedResult", (void*)GetOverlappedResult_Hook, (void**)&GetOverlappedResult_Original);
    hook(kernelbase, "GetOverlappedResultEx", (void*)GetOverlappedResultEx_Hook, (void**)&GetOverlappedResultEx_Original);
    hook(kernelbase, "GetQueuedCompletionStatus", (void*)GetQueuedCompletionStatus_Hook,
         (void**)&GetQueuedCompletionStatus_Original);
    hook(kernelbase, "GetQueuedCompletionStatusEx", (void*)GetQueuedCompletionStatusEx_Hook,
         (void**)&GetQueuedCompletionStatusEx_Original);
    hook(kernelbase, "DeviceIoControl", (void*)DeviceIoControl_Hook, (void**)&DeviceIoControl_Original);
    (void)kernel32;
    return TRUE;
}

void Wh_ModUninit() {
    Write("-- stopped\r\n");
    if (g_log != INVALID_HANDLE_VALUE) {
        CloseHandle_Original ? CloseHandle_Original(g_log) : CloseHandle(g_log);
    }
}
