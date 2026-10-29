// The command language (command.cpp) on its own, without Windows: what each
// command line parses to, and what it would do.
//
//   g++ -std=c++23 -o cmdtest cmdtest.cpp          (anywhere)
//   x86_64-w64-mingw32-g++ -std=c++23 -o cmdtest.exe cmdtest.cpp   (for the PC)
//
// With no arguments it runs the cases below. With an argument it prints what
// that command line would do, the same as `tournectl check "<command>"`.
#include <algorithm>
#include <cstdio>
#include <string>
#include <vector>

namespace {

// The two helpers the inlined file expects from whoever inlines it.
std::wstring Trimmed(std::wstring s) {
    size_t a = s.find_first_not_of(L" \t\r\n"), b = s.find_last_not_of(L" \t\r\n");
    return a == std::wstring::npos ? L"" : s.substr(a, b - a + 1);
}

std::wstring Lower(std::wstring s) {
    for (auto& c : s) c = (wchar_t)(c >= L'A' && c <= L'Z' ? c - L'A' + L'a' : c);
    return s;
}

#include "../../command.cpp"

int g_failures;

// steps: name/arg/wait of each, as "name=arg" with "wait=200" for a wait.
void Check(const wchar_t* line, const char* wanted) {
    Command c = ParseCommand(line);
    std::string got;
    if (!c.error.empty()) {
        got = "error";
    } else {
        for (CommandStep const& s : c.steps) {
            if (!got.empty()) got += " | ";
            for (wchar_t ch : s.name) got += (char)ch;
            std::wstring arg = s.name == L"wait" ? std::to_wstring(s.waitMs) : s.arg;
            if (!arg.empty()) {
                got += "=";
                for (wchar_t ch : arg) got += (char)ch;
            }
        }
    }
    bool ok = got == wanted;
    if (!ok) g_failures++;
    printf("  %s %-46ls -> %s%s\n", ok ? "ok  " : "FAIL", line, got.c_str(),
           ok ? "" : (std::string("   (wanted ") + wanted + ")").c_str());
    if (!c.error.empty() && !got.empty() && got == "error") {
        printf("       ");
        for (wchar_t ch : c.error) putchar((char)ch);
        putchar('\n');
    }
}

void Cases() {
    printf("one step\n");
    Check(L"killactive", "killactive");
    Check(L"workspace 2", "workspace=2");
    Check(L"  MoveFocus   l  ", "movefocus=l");
    Check(L"resizeactive 10, 0", "resizeactive=10, 0");

    printf("several steps\n");
    Check(L"togglefloating; centerwindow", "togglefloating | centerwindow");
    Check(L"workspace 2; wait 200; exec notepad", "workspace=2 | wait=200 | exec=notepad");
    Check(L"workspace 2;;  ; centerwindow", "workspace=2 | centerwindow");
    Check(L"workspace 2; centerwindow;", "workspace=2 | centerwindow");

    printf("waits\n");
    Check(L"a; wait 200ms; b", "a | wait=200 | b");
    Check(L"a; wait 0.2s; b", "a | wait=200 | b");
    Check(L"a; wait 2 s; b", "a | wait=2000 | b");
    Check(L"a; pause 50; b", "a | wait=50 | b");
    Check(L"a; wait; b", "error");
    Check(L"a; wait soon; b", "error");
    Check(L"a; wait 120000; b", "error");  // longer than a minute
    Check(L"a; wait 200", "a");            // a wait at the end waits for nothing

    printf("paths and semicolons\n");
    Check(L"exec C:\\Windows\\notepad.exe", "exec=C:\\Windows\\notepad.exe");
    Check(L"exec cmd /c echo a\\;b", "exec=cmd /c echo a;b");
    Check(L"exec \"C:\\Program Files\\x\\y.exe\" -q", "exec=\"C:\\Program Files\\x\\y.exe\" -q");

    printf("refused\n");
    Check(L"", "error");
    Check(L"   ;  ;  ", "error");
    Check(L"run workspace 2", "error");  // a command can't call itself
    Check(L"a; b; c; d; e; f; g; h; i; j; k; l; m; n; o; p; q; r; s; t; u; v; w; x; y; z; "
          L"a; b; c; d; e; f; g",
          "error");  // more steps than it allows
}

}  // namespace

int main(int argc, char** argv) {
    if (argc > 1) {
        std::string one;
        for (int i = 1; i < argc; i++) one += (i > 1 ? " " : "") + std::string(argv[i]);
        std::wstring line(one.begin(), one.end());
        std::wstring said = DescribeCommand(ParseCommand(line));
        printf("%ls\n", said.c_str());
        return 0;
    }
    Cases();
    printf("\n%s\n", g_failures ? "failures above" : "the command language reads every case as it should");
    return g_failures ? 1 : 0;
}
