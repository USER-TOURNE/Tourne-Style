////////////////////////////////////////////////////////////////////////////////
// One command language ("// @inline command.cpp", from binds.cpp): the text a
// key, a menu item, a bar click and tournectl all speak, so the same line means
// the same thing wherever it is written.
//
// A command is one or more steps, separated by semicolons:
//
//     workspace 2
//     togglefloating; centerwindow
//     workspace 2; wait 200; exec notepad
//
// A step is a dispatcher's name and its argument, the same names a bind line
// uses (the 39 of them Dispatch answers to). Two steps are the language's own:
//
//     wait 200      the next step waits that long (200, 200ms and 0.2s all read)
//     run <command> refused inside a command, so a command can't call itself
//
// A semicolon inside an argument is written \; and a backslash \\. Nothing else
// is escaped, so a path keeps its own backslashes.
//
// This file is plain text work with no Windows in it, so tools/binds/cmdtest.cpp
// builds and runs it anywhere; `tournectl check "<command>"` prints what a
// command would do without doing any of it.
//
// A fragment, like the other inlined files: whoever inlines it has already
// included <algorithm>, <string> and <vector>, and has Trimmed() and Lower().

constexpr int kCommandMostSteps = 32;      // a command longer than this is refused
constexpr int kCommandLongestWait = 60000;  // and so is a wait longer than a minute

struct CommandStep {
    std::wstring name, arg;
    int waitMs = 0;  // a wait step: how long, and name is "wait"
};

struct Command {
    std::vector<CommandStep> steps;
    std::wstring error;  // what's wrong with it, empty when it reads
    bool ok() const { return error.empty() && !steps.empty(); }
};

// "200", "200ms", "0.2s", "2 s": milliseconds, -1 if it isn't a length of time
// at all, -2 if it is one but longer than a command may wait. A bare number is
// milliseconds, as Hyprland's own waits are.
int CommandWaitMs(std::wstring const& text) {
    std::wstring t = Lower(Trimmed(text));
    if (t.empty()) return -1;
    bool seconds = false;
    if (t.size() > 2 && t.compare(t.size() - 2, 2, L"ms") == 0) {
        t = Trimmed(t.substr(0, t.size() - 2));
    } else if (t.back() == L's') {
        seconds = true;
        t = Trimmed(t.substr(0, t.size() - 1));
    }
    if (t.empty()) return -1;
    double value = 0, place = 0;
    for (wchar_t c : t) {
        if (c == L'.') {
            if (place) return -1;  // two dots
            place = 0.1;
        } else if (c >= L'0' && c <= L'9') {
            if (place) value += (c - L'0') * place, place /= 10;
            else value = value * 10 + (c - L'0');
        } else {
            return -1;
        }
    }
    double ms = seconds ? value * 1000 : value;
    if (ms > kCommandLongestWait) return -2;
    return (int)(ms + 0.5);
}

// The steps of a command line, or what's wrong with it. Empty steps (";;" or a
// trailing ";") are dropped rather than refused, so a line stays easy to edit.
Command ParseCommand(std::wstring const& text) {
    Command out;
    std::wstring step;
    bool escape = false;
    auto take = [&] {
        std::wstring one = Trimmed(step);
        step.clear();
        if (one.empty()) return;
        if ((int)out.steps.size() >= kCommandMostSteps) {
            if (out.error.empty()) out.error = L"more than " + std::to_wstring(kCommandMostSteps) + L" steps";
            return;
        }
        size_t space = one.find_first_of(L" \t");
        CommandStep s;
        s.name = Lower(one.substr(0, space));
        s.arg = space == std::wstring::npos ? L"" : Trimmed(one.substr(space + 1));
        if (s.name == L"wait" || s.name == L"pause") {
            s.name = L"wait";
            s.waitMs = CommandWaitMs(s.arg);
            if (s.waitMs < 0 && out.error.empty())
                out.error = s.waitMs == -2 ? L"a command can't wait longer than a minute"
                            : s.arg.empty() ? L"wait needs a length of time, as wait 200"
                                            : L"wait doesn't understand \"" + s.arg + L"\" (try 200, 200ms or 0.2s)";
        } else if (s.name == L"run") {
            if (out.error.empty()) out.error = L"run can't be a step of a command: write the steps themselves";
        }
        out.steps.push_back(std::move(s));
    };
    for (wchar_t c : text) {
        if (escape) {
            if (c != L';' && c != L'\\') step += L'\\';  // (any other backslash is the path's own)
            step += c;
            escape = false;
        } else if (c == L'\\') {
            escape = true;
        } else if (c == L';') {
            take();
        } else {
            step += c;
        }
    }
    if (escape) step += L'\\';
    take();
    if (out.error.empty() && out.steps.empty()) out.error = L"nothing to do";
    // A wait at the end waits for nothing.
    while (!out.steps.empty() && out.steps.back().name == L"wait") out.steps.pop_back();
    if (out.error.empty() && out.steps.empty()) out.error = L"only a wait: nothing to do after it";
    return out;
}

// What the command would do, a step per line, for `tournectl check`, the menu
// editor's help and the console check. Nothing is done.
std::wstring DescribeCommand(Command const& c) {
    if (!c.error.empty()) return L"error: " + c.error;
    std::wstring out;
    int at = 1;
    for (CommandStep const& s : c.steps) {
        out += std::to_wstring(at++) + L". ";
        if (s.name == L"wait") out += L"wait " + std::to_wstring(s.waitMs) + L" ms";
        else if (s.arg.empty()) out += s.name;
        else out += s.name + L" (" + s.arg + L")";
        out += L"\n";
    }
    return out.substr(0, out.size() - 1);
}
