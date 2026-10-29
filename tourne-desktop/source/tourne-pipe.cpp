////////////////////////////////////////////////////////////////////////////////
// Asking the part that runs commands ("// @inline tourne-pipe.cpp"): one line
// to tournectl's pipe for this session, \\.\pipe\tourne-ctl-<session>, and its
// answer back.
//
// The commands live with the key binds (binds.cpp, in the Switcher) because
// that is where the window and desktop work already is. Everything else that
// speaks the command language (the Tourne Menu's items, a bar click, the menu
// editor's help) asks through this pipe, which works whether the Switcher is a
// thread of the same program or, in the Windhawk suite, another process.
//
// An empty answer means nobody was listening: Key binds off, or "Let scripts
// control Tourne" off. Callers say so rather than failing silently.
//
// A fragment, like the other inlined files: whoever inlines it has already
// included <string> and windows.h.

inline std::string TournePipeUtf8(std::wstring const& s) {
    if (s.empty()) return {};
    int n = WideCharToMultiByte(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0, nullptr, nullptr);
    std::string out((size_t)n, '\0');
    WideCharToMultiByte(CP_UTF8, 0, s.data(), (int)s.size(), out.data(), n, nullptr, nullptr);
    return out;
}

inline std::wstring TournePipeWide(std::string const& s) {
    if (s.empty()) return {};
    int n = MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0);
    std::wstring out((size_t)n, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), out.data(), n);
    return out;
}

// The answer, or empty if nothing is listening. Waits at most waitMs for a busy
// pipe (it serves one caller at a time), so a click never hangs the menu.
inline std::wstring AskTourne(std::wstring const& line, DWORD waitMs = 250) {
    DWORD session = 0;
    ProcessIdToSessionId(GetCurrentProcessId(), &session);
    std::wstring name = L"\\\\.\\pipe\\tourne-ctl-" + std::to_wstring(session);
    HANDLE pipe = CreateFileW(name.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
    if (pipe == INVALID_HANDLE_VALUE && GetLastError() == ERROR_PIPE_BUSY && WaitNamedPipeW(name.c_str(), waitMs))
        pipe = CreateFileW(name.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
    if (pipe == INVALID_HANDLE_VALUE) return L"";
    std::string request = TournePipeUtf8(line) + "\n";
    DWORD moved = 0;
    std::string answer;
    if (WriteFile(pipe, request.data(), (DWORD)request.size(), &moved, nullptr)) {
        char buf[4096];
        while (ReadFile(pipe, buf, sizeof(buf), &moved, nullptr) && moved) answer.append(buf, moved);
    }
    CloseHandle(pipe);
    return answer.empty() ? L" " : TournePipeWide(answer);  // (a space: listening, but it said nothing)
}
