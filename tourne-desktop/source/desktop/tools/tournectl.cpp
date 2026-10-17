// tournectl: Tourne's hyprctl. Sends one request to the running Tourne
// (Switcher part, with "Let scripts control Tourne" on) and prints the answer.
//
//     tournectl dispatch workspace 2
//     tournectl dispatch exec notepad
//     tournectl activewindow | clients | workspaces | activeworkspace | monitors | binds | submap
//     tournectl reload | version
//     tournectl listen          events as lines, until Ctrl+C (activewindow>>, openwindow>>, ...)
//
// Exit code 1 when Tourne isn't reachable or answers with an error.
#include <windows.h>

#include <cstdio>
#include <string>

static std::string Utf8(std::wstring const& s) {
    int n = WideCharToMultiByte(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0, nullptr, nullptr);
    std::string out(n, '\0');
    WideCharToMultiByte(CP_UTF8, 0, s.data(), (int)s.size(), out.data(), n, nullptr, nullptr);
    return out;
}

int wmain(int argc, wchar_t** argv) {
    if (argc < 2) {
        fwprintf(stderr, L"usage: tournectl <request> [arguments]   (try: tournectl dispatch workspace 2)\n");
        return 2;
    }
    std::wstring line;
    for (int i = 1; i < argc; i++) {
        if (i > 1) line += L' ';
        line += argv[i];
    }
    DWORD session = 0;
    ProcessIdToSessionId(GetCurrentProcessId(), &session);
    std::wstring name = L"\\\\.\\pipe\\tourne-ctl-" + std::to_wstring(session);
    HANDLE pipe = INVALID_HANDLE_VALUE;
    for (int tries = 0; tries < 20; tries++) {
        pipe = CreateFileW(name.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
        if (pipe != INVALID_HANDLE_VALUE || GetLastError() != ERROR_PIPE_BUSY) break;
        WaitNamedPipeW(name.c_str(), 250);
    }
    if (pipe == INVALID_HANDLE_VALUE) {
        fwprintf(stderr, L"tournectl: Tourne isn't listening (turn on Key binds and \"Let scripts control Tourne\")\n");
        return 1;
    }
    std::string request = Utf8(line) + "\n";
    DWORD n = 0;
    if (!WriteFile(pipe, request.data(), (DWORD)request.size(), &n, nullptr)) {
        fwprintf(stderr, L"tournectl: couldn't send (%lu)\n", GetLastError());
        return 1;
    }
    std::string answer;
    char buf[4096];
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    bool listening = _wcsicmp(argv[1], L"listen") == 0;
    while (ReadFile(pipe, buf, sizeof(buf), &n, nullptr) && n) {
        if (listening) {
            DWORD w = 0;
            WriteFile(out, buf, n, &w, nullptr);
        } else {
            answer.append(buf, n);
        }
    }
    CloseHandle(pipe);
    if (listening) return 0;
    DWORD w = 0;
    WriteFile(out, answer.data(), (DWORD)answer.size(), &w, nullptr);
    return answer.rfind("error", 0) == 0 ? 1 : 0;
}
