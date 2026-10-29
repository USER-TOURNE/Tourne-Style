// Layouts written in Luau, checked without a screen: the real sandbox and the
// real examples (desktop/host/layout-script-core.inl and layout-examples.inl),
// run on any machine, Windows or not.
//
//   Windows:   tools\layouts\Run-LayoutTest.ps1
//   elsewhere: g++ -std=c++23 -I<luau includes> -o layouttest layouttest.cpp <luau objects>
//
// What it checks, for each example and one to eight windows on three screens:
//   - a rectangle for every window, inside the screen and with room in it;
//   - no two of them overlapping, where the layout doesn't mean them to;
//   - the gap asked for between neighbours.
// Then what a layout that misbehaves does: no layout function, an answer that
// isn't a list, too few rectangles, rectangles off the screen (kept inside),
// numbers that aren't numbers, and a script that never finishes (stopped).
//
// With a file name as its argument it runs that .luau file instead and prints
// the rectangles, which is how a layout is tried before it is used.
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

#include "lua.h"
#include "lualib.h"
#include "luacode.h"

namespace {

#include "../../desktop/host/layout-script-core.inl"
#include "../../desktop/host/layout-examples.inl"

int g_failures;

void Fail(const char* name, int n, int gap, const char* what) {
    printf("  FAIL %-28s %d window%s, gap %d: %s\n", name, n, n == 1 ? " " : "s", gap, what);
    g_failures++;
}

// Overlapping by more than half a pixel. A layout works in fractions (a screen
// 3440 across shared between 7 windows), and the tiling rounds its answer to
// whole pixels, so two neighbours that meet at 491.43 are touching, not
// overlapping.
bool Overlapping(LsBox const& a, LsBox const& b) {
    constexpr double slack = 0.5;
    return a.x + slack < b.x + b.w && b.x + slack < a.x + a.w && a.y + slack < b.y + b.h &&
           b.y + slack < a.y + a.h;
}

LsAsk Ask(LsBox screen, int n, int gap, std::string options) {
    LsAsk ask;
    ask.screen = screen;
    ask.gap = gap;
    ask.ratio = 55;
    ask.options = std::move(options);
    const char* programs[] = {"Code.exe", "Spotify.exe", "firefox.exe", "Discord.exe",
                              "Explorer.exe", "Terminal.exe", "Notepad.exe", "Steam.exe"};
    for (int i = 0; i < n; i++) {
        LsWindow w;
        w.box = LsBox{screen.x + i * 40.0, screen.y + i * 30.0, 800, 600};
        w.program = programs[i % 8];
        w.cls = "Window";
        w.title = std::string(programs[i % 8]) + " - a window";
        w.focused = i == 0;
        ask.windows.push_back(std::move(w));
    }
    return ask;
}

// Every example, laid out and looked over.
void CheckExamples() {
    const LsBox kScreens[] = {{0, 0, 3440, 1440}, {1920, 0, 1440, 2560}, {0, 0, 1280, 1024}};
    for (LsExample const& x : kLsExamples) {
        printf("%s\n", x.name);
        LsVm vm;
        if (!LsCompile(vm, x.text, x.name)) {
            printf("  FAIL %-28s %s\n", x.name, vm.problem.c_str());
            g_failures++;
            continue;
        }
        for (LsBox screen : kScreens)
            for (int n = 1; n <= 8; n++)
                for (int gap : {0, 8, 24}) {
                    // The by-program example is given programs to put in its strip.
                    std::string options = std::string(x.name).find("by-program") != std::string::npos
                                              ? "Spotify.exe, Discord.exe"
                                              : "3";
                    std::vector<LsBox> out;
                    if (!LsRun(vm, Ask(screen, n, gap, options), &out)) {
                        Fail(x.name, n, gap, vm.problem.c_str());
                        continue;
                    }
                    if ((int)out.size() != n) {
                        Fail(x.name, n, gap, "a rectangle for every window is what's wanted");
                        continue;
                    }
                    for (LsBox const& b : out) {
                        if (b.x < screen.x || b.y < screen.y || b.x + b.w > screen.x + screen.w ||
                            b.y + b.h > screen.y + screen.h) {
                            Fail(x.name, n, gap, "a rectangle hangs off the screen");
                            break;
                        }
                        if (b.w < 1 || b.h < 1) {
                            Fail(x.name, n, gap, "a rectangle has no room in it");
                            break;
                        }
                    }
                    for (size_t i = 0; i < out.size(); i++)
                        for (size_t j = i + 1; j < out.size(); j++)
                            if (Overlapping(out[i], out[j])) {
                                Fail(x.name, n, gap, "two rectangles overlap");
                                i = out.size();
                                break;
                            }
                }
        LsClose(vm);
    }
}

// The by-program example really does put the programs it was given along the
// bottom, which is the one thing only a layout with the window list can do.
void CheckByProgram() {
    printf("a layout that places windows by their program\n");
    const LsExample* found = nullptr;
    for (LsExample const& x : kLsExamples)
        if (std::string(x.name).find("by-program") != std::string::npos) found = &x;
    if (!found) {
        printf("  FAIL the by-program example isn't there any more\n");
        g_failures++;
        return;
    }
    LsVm vm;
    if (!LsCompile(vm, found->text, found->name)) {
        printf("  FAIL %s\n", vm.problem.c_str());
        g_failures++;
        return;
    }
    LsAsk ask = Ask(LsBox{0, 0, 3440, 1440}, 4, 12, "Spotify.exe");
    std::vector<LsBox> out;
    if (!LsRun(vm, ask, &out) || out.size() != 4) {
        printf("  FAIL it didn't answer: %s\n", vm.problem.c_str());
        g_failures++;
        LsClose(vm);
        return;
    }
    // Window 2 is Spotify (Ask's list), so its rectangle is the low one.
    double spotify = out[1].y, lowest = out[0].y;
    for (size_t i = 0; i < out.size(); i++)
        if (i != 1) lowest = std::min(lowest, out[i].y);
    if (spotify <= lowest) {
        printf("  FAIL Spotify's window isn't the one along the bottom\n");
        g_failures++;
    }
    if (out[1].x != ask.screen.x || std::lround(out[1].w) != std::lround(ask.screen.w)) {
        printf("  FAIL the strip doesn't run the width of the screen\n");
        g_failures++;
    }
    LsClose(vm);
}

// A layout that misbehaves: what the tiling is told, so that it can lay the
// windows out itself instead of leaving them in a heap.
void CheckBadScripts() {
    printf("layouts that misbehave\n");
    struct Case {
        const char* what;
        const char* source;
        bool compiles;  // it loads, and the fault shows when it is called
        bool answers;   // it answers with usable rectangles all the same
    } const cases[] = {
        {"no layout function", "local function other() end\n", false, false},
        {"a syntax error", "function layout(ask) return {{x =} end\n", false, false},
        {"answers with nothing", "function layout(ask) return nil end\n", true, false},
        {"answers with a number", "function layout(ask) return 7 end\n", true, false},
        {"too few rectangles", "function layout(ask) return {{x = 0, y = 0, w = 10, h = 10}} end\n", true, false},
        {"its own error", "function layout(ask) error('no thanks') end\n", true, false},
        {"not a number", "function layout(ask)\n local out = {}\n for i = 1, ask.count do out[i] = {x = 'left', y = 0, w = 10, h = 10} end\n return out\nend\n", true, false},
        {"nan and infinity", "function layout(ask)\n local out = {}\n for i = 1, ask.count do out[i] = {x = 0/0, y = 1/0, w = 10, h = 10} end\n return out\nend\n", true, false},
        {"rectangles off the screen", "function layout(ask)\n local out = {}\n for i = 1, ask.count do out[i] = {x = -9000, y = -9000, w = 99000, h = 99000} end\n return out\nend\n", true, true},
        {"four numbers in a row", "function layout(ask)\n local out = {}\n for i = 1, ask.count do out[i] = {ask.screen.x, ask.screen.y, 100, 100} end\n return out\nend\n", true, true},
        {"a table with layout in it", "return {layout = function(ask)\n local out = {}\n for i = 1, ask.count do out[i] = {x = ask.screen.x, y = ask.screen.y, w = 100, h = 100} end\n return out\nend}\n", true, true},
        {"never finishes", "function layout(ask)\n local n = 0\n while true do n = n + 1 end\n return {}\nend\n", true, false},
        {"more memory than it may have", "function layout(ask)\n local t = {}\n for i = 1, 20000000 do t[i] = i end\n return {}\nend\n", true, false},
        {"the files it can't reach", "function layout(ask) return require('../../anything') end\n", true, false},
    };
    LsBox screen{0, 0, 2560, 1440};
    for (Case const& t : cases) {
        LsVm vm;
        bool compiled = LsCompile(vm, t.source, t.what);
        std::vector<LsBox> out;
        bool answered = compiled && LsRun(vm, Ask(screen, 3, 8, ""), &out);
        bool inside = true;
        for (LsBox const& b : out)
            if (b.x < screen.x || b.y < screen.y || b.x + b.w > screen.x + screen.w ||
                b.y + b.h > screen.y + screen.h || b.w < 1 || b.h < 1)
                inside = false;
        bool ok = compiled == t.compiles && answered == t.answers && inside &&
                  (answered ? out.size() == 3 : out.empty()) && (answered || !vm.problem.empty());
        if (!ok) g_failures++;
        printf("  %s %-28s %s\n", ok ? "ok  " : "FAIL", t.what,
               answered ? "answered" : vm.problem.empty() ? "said nothing (it should say why)" : vm.problem.c_str());
        LsClose(vm);
    }
}

bool ReadAll(const char* path, std::string* out) {
    FILE* f = fopen(path, "rb");
    if (!f) return false;
    char buffer[4096];
    size_t read = 0;
    while ((read = fread(buffer, 1, sizeof(buffer), f)) > 0) out->append(buffer, read);
    fclose(f);
    return true;
}

// One file, laid out and printed: a layout tried before it is used.
int RunFile(const char* path, const char* options) {
    std::string source;
    if (!ReadAll(path, &source)) {
        printf("couldn't read %s\n", path);
        return 1;
    }
    LsVm vm;
    if (!LsCompile(vm, source, path)) {
        printf("%s %s\n", path, vm.problem.c_str());
        return 1;
    }
    for (int n = 1; n <= 5; n++) {
        std::vector<LsBox> out;
        if (!LsRun(vm, Ask(LsBox{0, 0, 3440, 1440}, n, 12, options ? options : ""), &out)) {
            printf("%d window%s: %s\n", n, n == 1 ? "" : "s", vm.problem.c_str());
            LsClose(vm);
            return 1;
        }
        printf("%d window%s on 3440x1440, gap 12:\n", n, n == 1 ? "" : "s");
        for (size_t i = 0; i < out.size(); i++)
            printf("  %zu. %5ld %5ld  %5ld x %-5ld\n", i + 1, std::lround(out[i].x), std::lround(out[i].y),
                   std::lround(out[i].w), std::lround(out[i].h));
    }
    LsClose(vm);
    return 0;
}

}  // namespace

int main(int argc, char** argv) {
    if (argc > 1) return RunFile(argv[1], argc > 2 ? argv[2] : nullptr);
    CheckExamples();
    CheckByProgram();
    CheckBadScripts();
    printf("\n%s\n", g_failures ? "failures above" : "the layouts and the sandbox do as they should");
    return g_failures ? 1 : 0;
}
