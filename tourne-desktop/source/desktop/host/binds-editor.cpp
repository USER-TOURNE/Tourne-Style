// Key binds, window rules and idle actions, without writing a config file. The same lines the Switcher reads
// (switcher.binds.lines, written as Hyprland's), shown as lists with plain choices beside them:
//
//   Key binds     each bind as its keys and what it does; beside it the keys (Win, Ctrl, Alt, Shift and a key,
//                 or Press keys), what it does from a list in words, what with (suggestions per action), when
//                 it runs (pressed, held, released), in games too, and a description the Keybind Guide shows.
//   Window rules  per rule the windows it's for (a program, class or title, or Pick an open window) and what
//                 happens as they open: float, tile, centre, keep on top, size, place, desktop, screen...
//   When idle     after so long without input: screens off, lock, or run something.
//   As text       every line as written, for anything the lists don't cover ($names, key modes...).
//
// Lines the lists don't show (comments, $names, key modes) stay where they are. OK and Apply write
// switcher.binds.* to tourne-gui.conf as the Studio does, and the config watch hands them to the Switcher.
// It runs on a thread of its own, as the Tourne Menu's settings do.

#include "binds-editor.h"

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <commdlg.h>
#include <dwmapi.h>
#include <psapi.h>

#include <algorithm>
#include <atomic>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "standalone.h"
#include "td.h"

namespace td {
namespace {

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc23-extensions"
const char kStarter[] = {
#embed "../presets/binds-starter.conf"
    , 0};
#pragma clang diagnostic pop

constexpr wchar_t kClass[] = L"TourneBindsSettings";
constexpr wchar_t kLines[] = L"switcher.binds.lines[";

enum : int {
    idTabs = 100,
    // Key binds
    idBindList, idBindAdd, idBindRemove, idBindUp, idBindDown, idBindProps,
    idKeysLabel, idWin, idCtrl, idAlt, idShift, idKey, idPress,
    idActionLabel, idAction, idArgLabel, idArg, idArgBrowse, idArgHelp,
    idWhenLabel, idWhen, idInGames, idPassThrough, idDescLabel, idDesc, idClash,
    // Window rules
    idRuleList, idRuleAdd, idRuleRemove, idRuleUp, idRuleDown, idRuleProps,
    idMatchLabel, idMatchKind, idMatch, idPickWindow,
    idFloat, idTile, idCenter, idPin, idMaximize, idMinimize, idNoBorder,
    idSizeOn, idSizeW, idSizeH, idMoveOn, idMoveX, idMoveY, idDeskOn, idDesk, idDeskSilent, idScreenOn, idScreen,
    idOpacityOn, idOpacity, idRuleHelp, idKeep, idHereNow,
    // When idle
    idIdleList, idIdleAdd, idIdleRemove, idIdleProps, idAfterLabel, idMinutes, idMinutesLabel, idSeconds, idSecondsLabel,
    idIdleDoLabel, idIdleDo, idIdleArgLabel, idIdleArg, idIdleBrowse, idIdleHelp,
    // As text
    idText, idTextHelp,
    // Below
    idEnabled, idStarter, idStatus, idOk, idCancel, idApply,
};

struct Place {
    int id, x, y, w, h, tab;
};
constexpr int kClientW = 780, kClientH = 560;
constexpr RECT kPage = {8, 32, 772, 514};
const Place kPlaces[] = {
    {idTabs, 8, 8, 764, 26, -1},
    // Key binds
    {idBindList, 20, 42, 400, 418, 0},
    {idBindAdd, 20, 470, 94, 28, 0}, {idBindRemove, 120, 470, 94, 28, 0}, {idBindUp, 220, 470, 94, 28, 0}, {idBindDown, 320, 470, 100, 28, 0},
    {idBindProps, 432, 36, 328, 470, 0},
    {idKeysLabel, 446, 60, 300, 18, 0},
    {idWin, 446, 80, 52, 22, 0}, {idCtrl, 500, 80, 54, 22, 0}, {idAlt, 556, 80, 48, 22, 0}, {idShift, 606, 80, 60, 22, 0},
    {idKey, 446, 106, 190, 300, 0}, {idPress, 642, 105, 104, 26, 0},
    {idActionLabel, 446, 142, 300, 18, 0}, {idAction, 446, 162, 300, 400, 0},
    {idArgLabel, 446, 196, 300, 18, 0}, {idArg, 446, 216, 264, 300, 0}, {idArgBrowse, 714, 215, 32, 26, 0},
    {idArgHelp, 446, 246, 300, 46, 0},
    {idWhenLabel, 446, 298, 300, 18, 0}, {idWhen, 446, 318, 300, 200, 0},
    {idInGames, 446, 350, 300, 22, 0}, {idPassThrough, 446, 374, 300, 22, 0},
    {idDescLabel, 446, 404, 300, 18, 0}, {idDesc, 446, 424, 300, 24, 0},
    {idClash, 446, 454, 300, 46, 0},
    // Window rules
    {idRuleList, 20, 42, 400, 418, 1},
    {idRuleAdd, 20, 470, 94, 28, 1}, {idRuleRemove, 120, 470, 94, 28, 1}, {idRuleUp, 220, 470, 94, 28, 1}, {idRuleDown, 320, 470, 100, 28, 1},
    {idRuleProps, 432, 36, 328, 470, 1},
    {idMatchLabel, 446, 60, 300, 18, 1}, {idMatchKind, 446, 80, 110, 200, 1}, {idMatch, 560, 80, 186, 24, 1},
    {idPickWindow, 446, 110, 300, 26, 1},
    {idFloat, 446, 146, 145, 22, 1}, {idTile, 600, 146, 145, 22, 1},
    {idCenter, 446, 170, 145, 22, 1}, {idPin, 600, 170, 145, 22, 1},
    {idMaximize, 446, 194, 145, 22, 1}, {idMinimize, 600, 194, 145, 22, 1},
    {idNoBorder, 446, 218, 145, 22, 1}, {idKeep, 600, 218, 145, 22, 1},
    {idSizeOn, 446, 250, 110, 22, 1}, {idSizeW, 560, 249, 90, 24, 1}, {idSizeH, 656, 249, 90, 24, 1},
    {idMoveOn, 446, 280, 110, 22, 1}, {idMoveX, 560, 279, 90, 24, 1}, {idMoveY, 656, 279, 90, 24, 1},
    {idDeskOn, 446, 310, 110, 22, 1}, {idDesk, 560, 309, 50, 24, 1}, {idDeskSilent, 616, 310, 130, 22, 1},
    {idScreenOn, 446, 340, 110, 22, 1}, {idScreen, 560, 339, 50, 24, 1}, {idHereNow, 616, 338, 130, 26, 1},
    {idOpacityOn, 446, 370, 110, 22, 1}, {idOpacity, 560, 369, 90, 24, 1},
    {idRuleHelp, 446, 402, 300, 98, 1},
    // When idle
    {idIdleList, 20, 42, 400, 418, 2},
    {idIdleAdd, 20, 470, 94, 28, 2}, {idIdleRemove, 120, 470, 94, 28, 2},
    {idIdleProps, 432, 36, 328, 470, 2},
    {idAfterLabel, 446, 60, 300, 18, 2},
    {idMinutes, 446, 80, 60, 24, 2}, {idMinutesLabel, 510, 84, 60, 18, 2}, {idSeconds, 574, 80, 60, 24, 2}, {idSecondsLabel, 638, 84, 80, 18, 2},
    {idIdleDoLabel, 446, 116, 300, 18, 2}, {idIdleDo, 446, 136, 300, 200, 2},
    {idIdleArgLabel, 446, 170, 300, 18, 2}, {idIdleArg, 446, 190, 264, 24, 2}, {idIdleBrowse, 714, 189, 32, 26, 2},
    {idIdleHelp, 446, 226, 300, 120, 2},
    // As text
    {idText, 20, 42, 740, 410, 3},
    {idTextHelp, 20, 458, 740, 48, 3},
    // Below
    {idEnabled, 10, 524, 230, 24, -1}, {idStarter, 244, 522, 150, 28, -1}, {idStatus, 400, 528, 120, 20, -1},
    {idOk, 524, 522, 76, 28, -1}, {idCancel, 606, 522, 76, 28, -1}, {idApply, 688, 522, 76, 28, -1},
};

////////////////////////////////////////////////////////////////////////////////
// The lines

std::vector<std::wstring> g_lines;

std::wstring Trim(std::wstring s) {
    size_t a = s.find_first_not_of(L" \t\r\n");
    if (a == std::wstring::npos) return L"";
    size_t b = s.find_last_not_of(L" \t\r\n");
    return s.substr(a, b - a + 1);
}

std::wstring Lower(std::wstring s) {
    for (auto& c : s) c = towlower(c);
    return s;
}

// Split at the first commas (at most parts - 1: the last part keeps the rest), each trimmed (as binds.cpp).
std::vector<std::wstring> Split(std::wstring const& text, size_t parts) {
    std::vector<std::wstring> out;
    size_t at = 0;
    while (out.size() + 1 < parts) {
        size_t comma = text.find(L',', at);
        if (comma == std::wstring::npos) break;
        out.push_back(Trim(text.substr(at, comma - at)));
        at = comma + 1;
    }
    out.push_back(Trim(text.substr(std::min(at, text.size()))));
    return out;
}

// "keyword = rest" of a line, or false for a comment, a blank or something else.
bool Keyword(std::wstring const& line, std::wstring* keyword, std::wstring* rest) {
    std::wstring t = Trim(line);
    if (t.empty() || t[0] == L'#' || t[0] == L'$') return false;
    size_t eq = t.find(L'=');
    if (eq == std::wstring::npos) return false;
    *keyword = Lower(Trim(t.substr(0, eq)));
    *rest = Trim(t.substr(eq + 1));
    return true;
}

struct BindLine {
    std::wstring flags;  // e r l n (d is written when there's a description)
    std::wstring mods, key, desc, command, arg;
};

bool ParseBind(std::wstring const& line, BindLine* b) {
    std::wstring keyword, rest;
    if (!Keyword(line, &keyword, &rest) || keyword.rfind(L"bind", 0) != 0) return false;
    std::wstring flags = keyword.substr(4);
    if (flags.find(L'm') != std::wstring::npos) return false;  // (mouse binds: as text only)
    bool described = flags.find(L'd') != std::wstring::npos;
    auto parts = Split(rest, described ? 5 : 4);
    if (parts.size() < 2) return false;
    *b = {};
    for (wchar_t f : flags)
        if (f != L'd') b->flags += f;
    b->mods = parts[0];
    b->key = parts[1];
    size_t at = 2;
    if (described && parts.size() > 3) b->desc = parts[at++];
    if (parts.size() > at) b->command = Lower(parts[at++]);
    if (parts.size() > at) b->arg = parts[at];
    return true;
}

std::wstring BindText(BindLine const& b) {
    std::wstring s = L"bind" + b.flags + (b.desc.empty() ? L"" : L"d") + L" = " + b.mods + L", " + b.key;
    if (!b.desc.empty()) s += L", " + b.desc;
    s += L", " + b.command;
    if (!b.arg.empty()) s += L", " + b.arg;
    return s;
}

// "When it runs": 0 pressed, 1 pressed and again while held (e), 2 let go (r), 3 a double press (k), 4 and on
// held down for kHolds[i - 4] ms (o and the time).
constexpr int kHolds[] = {300, 500, 800, 1200, 2000};
int WhenOf(std::wstring const& flags) {
    if (flags.find(L'k') != std::wstring::npos) return 3;
    if (size_t o = flags.find(L'o'); o != std::wstring::npos) {
        int ms = _wtoi(flags.c_str() + o + 1);
        if (ms <= 0) ms = 500;
        int best = 0;
        for (int i = 0; i < (int)ARRAYSIZE(kHolds); i++)
            if (std::abs(kHolds[i] - ms) < std::abs(kHolds[best] - ms)) best = i;
        return 4 + best;
    }
    return flags.find(L'e') != std::wstring::npos ? 1 : flags.find(L'r') != std::wstring::npos ? 2 : 0;
}
std::wstring WhenFlags(int when) {
    if (when >= 4 && when < 4 + (int)ARRAYSIZE(kHolds)) return L"o" + std::to_wstring(kHolds[when - 4]);
    return when == 1 ? L"e" : when == 2 ? L"r" : when == 3 ? L"k" : L"";
}
std::wstring WhenWords(std::wstring const& flags) {
    int when = WhenOf(flags);
    if (when >= 4) {
        WCHAR t[32];
        swprintf(t, 32, L"held %.1f s", kHolds[when - 4] / 1000.0);
        return t;
    }
    return when == 1 ? L"held" : when == 2 ? L"released" : when == 3 ? L"twice" : L"";
}

// The $names set before a line, as the Switcher fills them in.
std::wstring WithNames(std::wstring text, size_t before) {
    std::map<std::wstring, std::wstring> set;  // (a name set again: the later value)
    for (size_t i = 0; i < before && i < g_lines.size(); i++) {
        std::wstring t = Trim(g_lines[i]);
        if (t.empty() || t[0] != L'$') continue;
        size_t eq = t.find(L'=');
        if (eq != std::wstring::npos) set[Trim(t.substr(0, eq))] = Trim(t.substr(eq + 1));
    }
    std::vector<std::pair<std::wstring, std::wstring>> names(set.begin(), set.end());
    std::sort(names.begin(), names.end(), [](auto const& a, auto const& b) { return a.first.size() > b.first.size(); });
    for (auto const& [name, value] : names)
        for (size_t at = text.find(name); at != std::wstring::npos; at = text.find(name, at + value.size())) text.replace(at, name.size(), value);
    return text;
}

// The key mode (submap) a line is in: "" for the usual keys.
std::wstring ModeAt(size_t line) {
    std::wstring mode;
    for (size_t i = 0; i < line && i < g_lines.size(); i++) {
        std::wstring keyword, rest;
        if (Keyword(g_lines[i], &keyword, &rest) && keyword == L"submap") mode = Lower(rest) == L"reset" ? L"" : rest;
    }
    return mode;
}

// The heading a line is under: the nearest "# Heading" comment above it with nothing but binds between.
std::wstring HeadingAt(size_t line) {
    for (size_t i = line; i-- > 0;) {
        std::wstring t = Trim(g_lines[i]);
        if (t.empty()) continue;
        if (t[0] == L'#') {
            t = Trim(t.substr(t.find_first_not_of(L"#")));
            return t.size() <= 40 ? t : L"";
        }
        std::wstring keyword, rest;
        if (!Keyword(t, &keyword, &rest) || keyword.rfind(L"bind", 0) != 0) return L"";
    }
    return L"";
}

// Where a new line goes: before the first key mode (lines after "submap = name" belong to it), else at the end.
size_t InsertPoint() {
    for (size_t i = 0; i < g_lines.size(); i++) {
        std::wstring keyword, rest;
        if (Keyword(g_lines[i], &keyword, &rest) && keyword == L"submap" && !rest.empty() && Lower(rest) != L"reset") return i;
    }
    return g_lines.size();
}

////////////////////////////////////////////////////////////////////////////////
// What a bind can do, in words

enum class ArgKind { None, Direction, Desktop, Program, Text, Layout, Number, Mode, Screen };

struct Action {
    const wchar_t* command;
    const wchar_t* label;
    ArgKind arg;
    const wchar_t* help;
};

const Action kActions[] = {
    {L"exec", L"Run a program or command", ArgKind::Program,
     L"A program, a file, a folder or a command line. [float; size 50% 50%] before it sets rules for the window it opens."},
    {L"killactive", L"Close the window in front", ArgKind::None, L""},
    {L"forcekillactive", L"Force the program in front to quit", ArgKind::None, L"Ends the program at once, without asking it to save."},
    {L"togglefloating", L"Float or tile the window in front", ArgKind::None, L""},
    {L"launcher", L"Open or close the launcher", ArgKind::None, L""},
    {L"fullscreen", L"Maximise or restore the window", ArgKind::None, L""},
    {L"minimize", L"Minimise the window", ArgKind::None, L""},
    {L"pin", L"Keep the window on top (or not)", ArgKind::None, L""},
    {L"centerwindow", L"Centre the window", ArgKind::None, L""},
    {L"movefocus", L"Focus the window to one side", ArgKind::Direction, L""},
    {L"movewindow", L"Move the window to one side", ArgKind::Direction,
     L"Tiled windows swap places; a floating one goes to the screen that way. mon:2 sends it to screen 2."},
    {L"swapwindow", L"Swap with the window to one side", ArgKind::Direction, L""},
    {L"resizeactive", L"Resize the window", ArgKind::Text, L"Width and height to add, like 40 0 or -40 0 (pixels or %)."},
    {L"moveactive", L"Move the window by", ArgKind::Text, L"Across and down, like 40 0 (pixels or %)."},
    {L"cyclenext", L"Next window", ArgKind::Text, L"Empty for the next, prev for the one before; tiled or floating for only those."},
    {L"focuscurrentorlast", L"Back to the window before", ArgKind::None, L""},
    {L"focusurgentorlast", L"The window that wants attention, or the one before", ArgKind::None, L""},
    {L"focusmonitor", L"Focus another screen", ArgKind::Screen, L"l, r, u or d for the screen that way, +1 or -1, or its number."},
    {L"workspace", L"Go to a desktop", ArgKind::Desktop, L"Its number, +1 or -1 for the next or the one before, or previous."},
    {L"movetoworkspace", L"Send the window to a desktop (and go along)", ArgKind::Desktop,
     L"special (or special:name) puts the window away in a scratchpad instead."},
    {L"movetoworkspacesilent", L"Send the window to a desktop (stay here)", ArgKind::Desktop, L""},
    {L"togglespecialworkspace", L"Show or hide a scratchpad", ArgKind::Text,
     L"Empty for the usual one, or the name used with movetoworkspace, special:name."},
    {L"layoutmsg", L"Tiling layout", ArgKind::Layout, L""},
    {L"splitratio", L"Change the split between tiles", ArgKind::Number, L"Like 0.05 (wider) or -0.05 (narrower)."},
    {L"togglegroup", L"Make a tab group, or break it up", ArgKind::None, L""},
    {L"changegroupactive", L"Next tab in the group", ArgKind::Text, L"f for the next, b for the one before, or its number."},
    {L"moveintogroup", L"Put the window into the group to one side", ArgKind::Direction, L""},
    {L"moveoutofgroup", L"Take the window out of its group", ArgKind::None, L""},
    {L"overview", L"Show every window (overview)", ArgKind::None, L""},
    {L"pickcolor", L"Copy the colour under the pointer", ArgKind::None, L"As #rrggbb, to the clipboard."},
    {L"dpms", L"Turn the screens off or on", ArgKind::Text, L"off, on or toggle."},
    {L"lockscreen", L"Lock the PC", ArgKind::None, L""},
    {L"bringactivetotop", L"Bring the window in front to the top", ArgKind::None, L""},
    {L"movecursortocorner", L"Move the pointer to a corner of the window", ArgKind::Text, L"0 bottom left, 1 bottom right, 2 top right, 3 top left."},
    {L"sendshortcut", L"Send keys to a window", ArgKind::Text, L"MODS, key, window: like CTRL, S, exe:notepad.exe (empty window: the one in front)."},
    {L"submap", L"Enter a key mode", ArgKind::Mode,
     L"The mode's name. Its keys are written under \"submap = name\" (As text); reset goes back to the usual keys."},
    {L"focuswindow", L"Focus a window by name", ArgKind::Text, L"exe:name.exe, class:..., title:..., with * for any text."},
    {L"closewindow", L"Close a window by name", ArgKind::Text, L"exe:name.exe, class:..., title:..."},
    {L"setfloating", L"Float the window", ArgKind::None, L""},
    {L"settiled", L"Tile the window", ArgKind::None, L""},
};

const Action* ActionOf(std::wstring const& command) {
    for (auto const& a : kActions)
        if (command == a.command) return &a;
    return nullptr;
}

std::vector<std::pair<std::wstring, std::wstring>> Suggestions(ArgKind kind) {  // (value, label)
    switch (kind) {
        case ArgKind::Direction: return {{L"l", L"l (left)"}, {L"r", L"r (right)"}, {L"u", L"u (up)"}, {L"d", L"d (down)"}};
        case ArgKind::Desktop: {
            std::vector<std::pair<std::wstring, std::wstring>> v;
            for (int i = 1; i <= 9; i++) v.push_back({std::to_wstring(i), std::to_wstring(i)});
            v.push_back({L"+1", L"+1 (next)"});
            v.push_back({L"-1", L"-1 (before)"});
            v.push_back({L"previous", L"previous"});
            v.push_back({L"special", L"special (scratchpad)"});
            return v;
        }
        case ArgKind::Layout:
            return {{L"cycle", L"cycle (the next layout)"}, {L"tile", L"tile (tile this desktop now)"}, {L"auto", L"auto (tiling on or off)"},
                    {L"mfact 0.6", L"mfact 0.6 (the main tile's share)"}};
        case ArgKind::Number: return {{L"0.05", L"0.05"}, {L"-0.05", L"-0.05"}};
        case ArgKind::Screen: return {{L"l", L"l"}, {L"r", L"r"}, {L"+1", L"+1"}, {L"-1", L"-1"}, {L"1", L"1"}, {L"2", L"2"}};
        case ArgKind::Mode: return {{L"resize", L"resize"}, {L"reset", L"reset"}};
        default: return {};
    }
}

std::wstring Doing(BindLine const& b) {
    const Action* a = ActionOf(b.command);
    std::wstring s = a ? a->label : b.command;
    if (!b.arg.empty()) s += L": " + b.arg;
    return s;
}

////////////////////////////////////////////////////////////////////////////////
// Keys, by the Switcher's names for them

struct KeyName {
    const wchar_t* name;  // as written in a bind
    const wchar_t* label;
};

std::vector<KeyName> const& KeyNames() {
    static std::vector<KeyName> keys = [] {
        std::vector<KeyName> k;
        static std::wstring letters[36];
        for (int i = 0; i < 26; i++) letters[i] = std::wstring(1, (wchar_t)(L'A' + i));
        for (int i = 0; i < 10; i++) letters[26 + i] = std::wstring(1, (wchar_t)(L'0' + i));
        for (auto const& l : letters) k.push_back({l.c_str(), l.c_str()});
        static std::wstring fs[24];
        for (int i = 0; i < 24; i++) fs[i] = L"F" + std::to_wstring(i + 1);
        for (auto const& f : fs) k.push_back({f.c_str(), f.c_str()});
        const KeyName more[] = {
            {L"space", L"Space"}, {L"return", L"Enter"}, {L"tab", L"Tab"}, {L"escape", L"Esc"}, {L"backspace", L"Backspace"},
            {L"delete", L"Delete"}, {L"insert", L"Insert"}, {L"home", L"Home"}, {L"end", L"End"}, {L"prior", L"Page Up"},
            {L"next", L"Page Down"}, {L"left", L"Left arrow"}, {L"right", L"Right arrow"}, {L"up", L"Up arrow"},
            {L"down", L"Down arrow"}, {L"print", L"Print Screen"}, {L"pause", L"Pause"}, {L"menu", L"Menu key"},
            {L"minus", L"- (minus)"}, {L"equal", L"= (equals)"}, {L"comma", L", (comma)"}, {L"period", L". (full stop)"},
            {L"slash", L"/"}, {L"semicolon", L";"}, {L"apostrophe", L"'"}, {L"grave", L"` (backquote)"},
            {L"bracketleft", L"["}, {L"bracketright", L"]"}, {L"backslash", L"\\"},
            {L"XF86AudioRaiseVolume", L"Volume up"}, {L"XF86AudioLowerVolume", L"Volume down"}, {L"XF86AudioMute", L"Mute"},
            {L"XF86AudioPlay", L"Play/Pause"}, {L"XF86AudioNext", L"Next track"}, {L"XF86AudioPrev", L"Previous track"},
            {L"XF86AudioStop", L"Stop"},
        };
        for (auto const& m : more) k.push_back(m);
        return k;
    }();
    return keys;
}

int KeyIndex(std::wstring const& name) {
    auto const& keys = KeyNames();
    std::wstring n = Lower(Trim(name));
    for (size_t i = 0; i < keys.size(); i++)
        if (Lower(keys[i].name) == n) return (int)i;
    // Other spellings the Switcher reads.
    const std::pair<const wchar_t*, const wchar_t*> same[] = {{L"enter", L"return"}, {L"esc", L"escape"}, {L"pageup", L"prior"},
                                                              {L"pagedown", L"next"}, {L"-", L"minus"}, {L"=", L"equal"},
                                                              {L",", L"comma"}, {L".", L"period"}, {L"/", L"slash"},
                                                              {L";", L"semicolon"}, {L"'", L"apostrophe"}, {L"`", L"grave"},
                                                              {L"[", L"bracketleft"}, {L"]", L"bracketright"}, {L"\\", L"backslash"}};
    for (auto const& [a, b] : same)
        if (n == a) return KeyIndex(b);
    return -1;
}

std::wstring KeyLabel(std::wstring const& name) {
    int i = KeyIndex(name);
    return i >= 0 ? KeyNames()[i].label : name;
}

// The Switcher's key name for a Windows key code ("" if it has none in the list).
std::wstring KeyNameOfVk(UINT vk) {
    if ((vk >= 'A' && vk <= 'Z') || (vk >= '0' && vk <= '9')) return std::wstring(1, (wchar_t)vk);
    if (vk >= VK_F1 && vk <= VK_F24) return L"F" + std::to_wstring(vk - VK_F1 + 1);
    const std::pair<UINT, const wchar_t*> map[] = {
        {VK_SPACE, L"space"}, {VK_RETURN, L"return"}, {VK_TAB, L"tab"}, {VK_ESCAPE, L"escape"}, {VK_BACK, L"backspace"},
        {VK_DELETE, L"delete"}, {VK_INSERT, L"insert"}, {VK_HOME, L"home"}, {VK_END, L"end"}, {VK_PRIOR, L"prior"},
        {VK_NEXT, L"next"}, {VK_LEFT, L"left"}, {VK_RIGHT, L"right"}, {VK_UP, L"up"}, {VK_DOWN, L"down"},
        {VK_SNAPSHOT, L"print"}, {VK_PAUSE, L"pause"}, {VK_APPS, L"menu"}, {VK_OEM_MINUS, L"minus"}, {VK_OEM_PLUS, L"equal"},
        {VK_OEM_COMMA, L"comma"}, {VK_OEM_PERIOD, L"period"}, {VK_OEM_2, L"slash"}, {VK_OEM_1, L"semicolon"},
        {VK_OEM_7, L"apostrophe"}, {VK_OEM_3, L"grave"}, {VK_OEM_4, L"bracketleft"}, {VK_OEM_6, L"bracketright"},
        {VK_OEM_5, L"backslash"}, {VK_VOLUME_UP, L"XF86AudioRaiseVolume"}, {VK_VOLUME_DOWN, L"XF86AudioLowerVolume"},
        {VK_VOLUME_MUTE, L"XF86AudioMute"}, {VK_MEDIA_PLAY_PAUSE, L"XF86AudioPlay"}, {VK_MEDIA_NEXT_TRACK, L"XF86AudioNext"},
        {VK_MEDIA_PREV_TRACK, L"XF86AudioPrev"}, {VK_MEDIA_STOP, L"XF86AudioStop"},
    };
    for (auto const& [v, n] : map)
        if (v == vk) return n;
    return vk ? L"code:" + std::to_wstring(vk) : L"";
}

UINT ModFlags(std::wstring const& mods) {
    UINT f = 0;
    std::wstring part;
    auto take = [&] {
        std::wstring p = Lower(part);
        part.clear();
        if (p == L"win" || p == L"super" || p == L"mod4" || p == L"logo" || p == L"meta") f |= MOD_WIN;
        else if (p == L"ctrl" || p == L"control") f |= MOD_CONTROL;
        else if (p == L"alt" || p == L"mod1") f |= MOD_ALT;
        else if (p == L"shift") f |= MOD_SHIFT;
    };
    for (wchar_t c : mods) {
        if (c == L' ' || c == L'_' || c == L'+' || c == L'\t') take();
        else part += c;
    }
    take();
    return f;
}

std::wstring ModsText(UINT f) {
    std::wstring s;
    for (auto [flag, name] : {std::pair{MOD_WIN, L"WIN"}, {MOD_CONTROL, L"CTRL"}, {MOD_ALT, L"ALT"}, {MOD_SHIFT, L"SHIFT"}})
        if (f & flag) s += (s.empty() ? L"" : L" ") + std::wstring(name);
    return s;
}

std::wstring KeysShown(BindLine const& b, size_t line) {
    UINT f = ModFlags(WithNames(b.mods, line));
    std::wstring s;
    for (auto [flag, name] : {std::pair{MOD_WIN, L"Win"}, {MOD_CONTROL, L"Ctrl"}, {MOD_ALT, L"Alt"}, {MOD_SHIFT, L"Shift"}})
        if (f & flag) s += std::wstring(name) + L" + ";
    return s + KeyLabel(WithNames(b.key, line));
}

////////////////////////////////////////////////////////////////////////////////
// Window rules: "windowrule = actions..., class:a, exe:b"

const wchar_t* const kMatchKinds[] = {L"exe", L"class", L"title"};
const wchar_t* const kMatchLabels[] = {L"Program", L"Class", L"Title"};

struct RuleLine {
    std::vector<std::wstring> actions;
    int matchKind = 0;
    std::wstring match;
    std::wstring otherMatches;  // more than one match: kept as written
};

bool IsMatch(std::wstring const& part) {
    std::wstring p = Lower(part);
    for (const wchar_t* k : {L"class:", L"title:", L"exe:", L"initialclass:", L"initialtitle:", L"pid:", L"address:"})
        if (p.rfind(k, 0) == 0) return true;
    return false;
}

bool ParseRule(std::wstring const& line, RuleLine* r) {
    std::wstring keyword, rest;
    if (!Keyword(line, &keyword, &rest) || (keyword != L"windowrule" && keyword != L"windowrulev2")) return false;
    *r = {};
    bool first = true;
    for (auto const& part : Split(rest, 64)) {
        if (part.empty()) continue;
        if (!IsMatch(part)) {
            r->actions.push_back(part);
            continue;
        }
        if (!first) {
            r->otherMatches += L", " + part;
            continue;
        }
        first = false;
        size_t colon = part.find(L':');
        std::wstring kind = Lower(part.substr(0, colon));
        r->matchKind = kind == L"class" || kind == L"initialclass" ? 1 : kind == L"title" || kind == L"initialtitle" ? 2 : 0;
        r->match = part.substr(colon + 1);
    }
    return true;
}

std::wstring RuleText(RuleLine const& r) {
    std::wstring s = L"windowrule = ";
    for (auto const& a : r.actions) s += a + L", ";
    s += std::wstring(kMatchKinds[r.matchKind]) + L":" + r.match + r.otherMatches;
    return s;
}

// A rule's first word and the rest ("size", "50% 60%").
std::pair<std::wstring, std::wstring> RuleWord(std::wstring const& action) {
    std::wstring a = Trim(action);
    size_t sp = a.find(L' ');
    return {Lower(a.substr(0, sp)), sp == std::wstring::npos ? L"" : Trim(a.substr(sp + 1))};
}

const std::pair<const wchar_t*, const wchar_t*> kRuleWords[] = {
    {L"float", L"Float"}, {L"tile", L"Tile"}, {L"center", L"Centre"}, {L"pin", L"Keep on top"}, {L"maximize", L"Maximise"},
    {L"fullscreen", L"Maximise"}, {L"minimize", L"Minimise"}, {L"noborder", L"No border"}, {L"size", L"Size"},
    {L"move", L"Place"}, {L"workspace", L"Desktop"}, {L"monitor", L"Screen"}, {L"opacity", L"Opacity"}, {L"keep", L"Kept there"},
};

std::wstring RuleShown(RuleLine const& r) {
    std::wstring s;
    for (auto const& a : r.actions) {
        auto [word, rest] = RuleWord(a);
        std::wstring label = a;
        for (auto const& [w, l] : kRuleWords)
            if (word == w) label = rest.empty() ? l : std::wstring(l) + L" " + rest;
        s += (s.empty() ? L"" : L", ") + label;
    }
    return s.empty() ? L"(nothing yet)" : s;
}

////////////////////////////////////////////////////////////////////////////////
// When idle: "idle = SECONDS, command, argument"

struct IdleLine {
    int seconds = 300;
    std::wstring command = L"dpms", arg = L"off";
};

bool ParseIdle(std::wstring const& line, IdleLine* i) {
    std::wstring keyword, rest;
    if (!Keyword(line, &keyword, &rest) || keyword != L"idle") return false;
    auto parts = Split(rest, 3);
    *i = {};
    i->seconds = std::max(1, _wtoi(parts[0].c_str()));
    i->command = parts.size() > 1 ? Lower(parts[1]) : L"";
    i->arg = parts.size() > 2 ? parts[2] : L"";
    return true;
}

std::wstring IdleText(IdleLine const& i) {
    return L"idle = " + std::to_wstring(i.seconds) + L", " + i.command + (i.arg.empty() ? L"" : L", " + i.arg);
}

struct IdleChoice {
    const wchar_t* label;
    const wchar_t* command;
    const wchar_t* arg;
};
const IdleChoice kIdleChoices[] = {
    {L"Turn the screens off", L"dpms", L"off"},
    {L"Lock the PC", L"lockscreen", L""},
    {L"Run a program or command", L"exec", nullptr},
    {L"Another command (as in a bind)", nullptr, nullptr},
};

int IdleChoiceOf(IdleLine const& i) {
    if (i.command == L"dpms" && Lower(i.arg) == L"off") return 0;
    if (i.command == L"lockscreen") return 1;
    if (i.command == L"exec") return 2;
    return 3;
}

std::wstring After(int seconds) {
    int m = seconds / 60, s = seconds % 60;
    std::wstring t;
    if (m) t = std::to_wstring(m) + L" min";
    if (s || !m) t += (t.empty() ? L"" : L" ") + std::to_wstring(s) + L" s";
    return t;
}

std::wstring IdleShown(IdleLine const& i) {
    int c = IdleChoiceOf(i);
    if (c == 0 || c == 1) return kIdleChoices[c].label;
    if (c == 2) return L"Run " + i.arg;
    return i.command + (i.arg.empty() ? L"" : L" " + i.arg);
}

////////////////////////////////////////////////////////////////////////////////
// The window

HWND g_wnd, g_bindList, g_ruleList, g_idleList;
HFONT g_font;
int g_dpi = 96, g_tab = 0;
bool g_dirty, g_filling;
std::atomic<bool> g_open{false};
HHOOK g_captureHook;

HWND Ctrl(int id) { return GetDlgItem(g_wnd, id); }
int S(int v) { return MulDiv(v, g_dpi, 96); }

std::wstring TextOf(int id) {
    HWND c = Ctrl(id);
    int n = GetWindowTextLengthW(c);
    std::wstring s(n, L'\0');
    if (n) GetWindowTextW(c, s.data(), n + 1);
    return s;
}

bool Checked(int id) { return Button_GetCheck(Ctrl(id)) == BST_CHECKED; }
void Check(int id, bool on) { Button_SetCheck(Ctrl(id), on ? BST_CHECKED : BST_UNCHECKED); }

void Changed() {
    if (g_filling) return;
    g_dirty = true;
    EnableWindow(Ctrl(idApply), TRUE);
    SetWindowTextW(Ctrl(idStatus), L"");
}

int Selected(HWND list) { return ListView_GetNextItem(list, -1, LVNI_SELECTED); }

int LineOfRow(HWND list, int row) {
    if (row < 0) return -1;
    LVITEMW it{};
    it.mask = LVIF_PARAM;
    it.iItem = row;
    ListView_GetItem(list, &it);
    return (int)it.lParam;
}

void AddColumn(HWND list, int at, const wchar_t* title, int width) {
    LVCOLUMNW c{};
    c.mask = LVCF_TEXT | LVCF_WIDTH;
    c.pszText = (LPWSTR)title;
    c.cx = S(width);
    ListView_InsertColumn(list, at, &c);
}

void SetRow(HWND list, int row, std::vector<std::wstring> const& cells, int line, int group = -1, bool insert = false) {
    LVITEMW it{};
    it.mask = LVIF_TEXT | LVIF_PARAM | (group >= 0 ? LVIF_GROUPID : 0);
    it.iItem = row;
    it.pszText = (LPWSTR)cells[0].c_str();
    it.lParam = line;
    it.iGroupId = group;
    if (insert) ListView_InsertItem(list, &it);
    else ListView_SetItem(list, &it);
    for (size_t c = 1; c < cells.size(); c++) ListView_SetItemText(list, row, (int)c, (LPWSTR)cells[c].c_str());
}

// The binds' list, grouped by their headings and key modes.
void FillBinds(int selectLine = -1) {
    g_filling = true;
    ListView_DeleteAllItems(g_bindList);
    ListView_RemoveAllGroups(g_bindList);
    std::vector<std::wstring> groups;
    int row = 0, select = -1;
    for (size_t i = 0; i < g_lines.size(); i++) {
        BindLine b;
        if (!ParseBind(g_lines[i], &b)) continue;
        std::wstring mode = ModeAt(i), heading = HeadingAt(i);
        std::wstring group = !mode.empty() ? L"Key mode: " + mode : !heading.empty() ? heading : L"Binds";
        auto at = std::find(groups.begin(), groups.end(), group);
        int id = (int)(at - groups.begin());
        if (at == groups.end()) {
            groups.push_back(group);
            LVGROUP g{sizeof(g)};
            g.mask = LVGF_HEADER | LVGF_GROUPID;
            g.pszHeader = (LPWSTR)group.c_str();
            g.iGroupId = id;
            ListView_InsertGroup(g_bindList, -1, &g);
        }
        std::wstring when = WhenWords(b.flags);
        SetRow(g_bindList, row, {KeysShown(b, i), b.desc.empty() ? Doing(b) : b.desc, when}, (int)i, id, true);
        if ((int)i == selectLine) select = row;
        row++;
    }
    ListView_EnableGroupView(g_bindList, groups.size() > 1);
    if (select >= 0) {
        ListView_SetItemState(g_bindList, select, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
        ListView_EnsureVisible(g_bindList, select, FALSE);
    }
    g_filling = false;
}

void FillRules(int selectLine = -1) {
    g_filling = true;
    ListView_DeleteAllItems(g_ruleList);
    int row = 0, select = -1;
    for (size_t i = 0; i < g_lines.size(); i++) {
        RuleLine r;
        if (!ParseRule(g_lines[i], &r)) continue;
        std::wstring who = std::wstring(kMatchLabels[r.matchKind]) + L": " + (r.match.empty() ? L"(any)" : r.match) + r.otherMatches;
        SetRow(g_ruleList, row, {who, RuleShown(r)}, (int)i, -1, true);
        if ((int)i == selectLine) select = row;
        row++;
    }
    if (select >= 0) ListView_SetItemState(g_ruleList, select, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
    g_filling = false;
}

void FillIdle(int selectLine = -1) {
    g_filling = true;
    ListView_DeleteAllItems(g_idleList);
    int row = 0, select = -1;
    for (size_t i = 0; i < g_lines.size(); i++) {
        IdleLine l;
        if (!ParseIdle(g_lines[i], &l)) continue;
        SetRow(g_idleList, row, {After(l.seconds), IdleShown(l)}, (int)i, -1, true);
        if ((int)i == selectLine) select = row;
        row++;
    }
    if (select >= 0) ListView_SetItemState(g_idleList, select, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
    g_filling = false;
}

void FillText() {
    std::wstring all;
    for (auto const& l : g_lines) all += l + L"\r\n";
    g_filling = true;
    SetWindowTextW(Ctrl(idText), all.c_str());
    g_filling = false;
}

void ReadText() {
    std::wstring all = TextOf(idText);
    g_lines.clear();
    size_t at = 0;
    while (at <= all.size()) {
        size_t end = all.find(L'\n', at);
        std::wstring line = all.substr(at, end == std::wstring::npos ? std::wstring::npos : end - at);
        if (!line.empty() && line.back() == L'\r') line.pop_back();
        g_lines.push_back(line);
        if (end == std::wstring::npos) break;
        at = end + 1;
    }
    while (!g_lines.empty() && Trim(g_lines.back()).empty()) g_lines.pop_back();
}

void EnableAll(std::initializer_list<int> ids, bool on) {
    for (int id : ids) EnableWindow(Ctrl(id), on);
}

////////////////////////////////////////////////////////////////////////////////
// Key binds: the picked one's properties

void FillArgChoices(const Action* a, std::wstring const& now) {
    HWND arg = Ctrl(idArg);
    ComboBox_ResetContent(arg);
    if (a)
        for (auto const& [value, label] : Suggestions(a->arg)) ComboBox_AddString(arg, label.c_str());
    SetWindowTextW(arg, now.c_str());
    bool takes = !a || a->arg != ArgKind::None;
    EnableWindow(arg, takes);
    EnableWindow(Ctrl(idArgLabel), takes);
    ShowWindow(Ctrl(idArgBrowse), a && a->arg == ArgKind::Program ? SW_SHOWNA : SW_HIDE);
    SetWindowTextW(Ctrl(idArgHelp), a ? a->help : L"A command the list doesn't name: as written in the line (see As text).");
}

// The other binds on the same keys (in the same key mode).
std::wstring Clashes(int line) {
    BindLine b;
    if (line < 0 || !ParseBind(g_lines[line], &b)) return L"";
    UINT mods = ModFlags(WithNames(b.mods, line));
    std::wstring key = Lower(WithNames(b.key, line)), mode = ModeAt(line), out;
    for (size_t i = 0; i < g_lines.size(); i++) {
        BindLine o;
        if ((int)i == line || !ParseBind(g_lines[i], &o) || ModeAt(i) != mode) continue;
        if (ModFlags(WithNames(o.mods, i)) == mods && Lower(WithNames(o.key, i)) == key) out += (out.empty() ? L"" : L"; ") + Doing(o);
    }
    if (!out.empty()) return L"These keys are also bound to: " + out;
    if (mods == 0 || mods == MOD_SHIFT) return L"Without Win, Ctrl or Alt, this key is taken from every program while Tourne runs.";
    if (mods == MOD_CONTROL) return L"Ctrl and one key is often a program's own shortcut (Ctrl+S saves): that program won't hear it.";
    return L"";
}

void ShowBind() {
    int line = LineOfRow(g_bindList, Selected(g_bindList));
    BindLine b;
    bool have = line >= 0 && ParseBind(g_lines[line], &b);
    g_filling = true;
    EnableAll({idWin, idCtrl, idAlt, idShift, idKey, idPress, idAction, idArg, idArgBrowse, idWhen, idInGames, idPassThrough, idDesc,
               idBindRemove, idBindUp, idBindDown},
              have);
    if (!have) b = {};
    UINT mods = ModFlags(WithNames(b.mods, have ? line : 0));
    Check(idWin, mods & MOD_WIN);
    Check(idCtrl, mods & MOD_CONTROL);
    Check(idAlt, mods & MOD_ALT);
    Check(idShift, mods & MOD_SHIFT);
    ComboBox_SetCurSel(Ctrl(idKey), KeyIndex(WithNames(b.key, have ? line : 0)));
    if (have && ComboBox_GetCurSel(Ctrl(idKey)) < 0) SetWindowTextW(Ctrl(idKey), b.key.c_str());
    const Action* a = ActionOf(b.command);
    int pick = -1;
    for (int i = 0; i < (int)ARRAYSIZE(kActions); i++)
        if (&kActions[i] == a) pick = i;
    ComboBox_SetCurSel(Ctrl(idAction), have && !a ? (int)ARRAYSIZE(kActions) : pick);
    FillArgChoices(have && !a ? nullptr : a, b.arg);
    ComboBox_SetCurSel(Ctrl(idWhen), WhenOf(b.flags));
    Check(idInGames, b.flags.find(L'l') != std::wstring::npos);
    Check(idPassThrough, b.flags.find(L'n') != std::wstring::npos);
    SetWindowTextW(Ctrl(idDesc), b.desc.c_str());
    SetWindowTextW(Ctrl(idClash), have ? Clashes(line).c_str() : L"");
    g_filling = false;
}

// The properties written back into the picked bind's line.
void StoreBind() {
    if (g_filling) return;
    int row = Selected(g_bindList), line = LineOfRow(g_bindList, row);
    BindLine b;
    if (line < 0 || !ParseBind(g_lines[line], &b)) return;
    UINT mods = (Checked(idWin) ? MOD_WIN : 0) | (Checked(idCtrl) ? MOD_CONTROL : 0) | (Checked(idAlt) ? MOD_ALT : 0) | (Checked(idShift) ? MOD_SHIFT : 0);
    // A $name kept while it still says the same.
    if (ModFlags(WithNames(b.mods, line)) != mods) b.mods = ModsText(mods);
    int k = ComboBox_GetCurSel(Ctrl(idKey));
    if (k >= 0 && Lower(WithNames(b.key, line)) != Lower(KeyNames()[k].name)) b.key = KeyNames()[k].name;
    int a = ComboBox_GetCurSel(Ctrl(idAction));
    if (a >= 0 && a < (int)ARRAYSIZE(kActions)) b.command = kActions[a].command;
    std::wstring arg = TextOf(idArg);
    // A suggestion's label ("l (left)") stands for its value.
    if (const Action* act = ActionOf(b.command))
        for (auto const& [value, label] : Suggestions(act->arg))
            if (arg == label) arg = value;
    b.arg = Trim(arg);
    if (const Action* act = ActionOf(b.command); act && act->arg == ArgKind::None) b.arg.clear();
    int when = ComboBox_GetCurSel(Ctrl(idWhen));
    b.flags = WhenFlags(when) + (Checked(idInGames) ? L"l" : L"") + (Checked(idPassThrough) ? L"n" : L"");
    b.desc = Trim(TextOf(idDesc));
    std::replace(b.desc.begin(), b.desc.end(), L',', L';');  // (a comma would end it)
    g_lines[line] = BindText(b);
    std::wstring when2 = WhenWords(b.flags);
    std::wstring keys = KeysShown(b, line);  // (kept: the macro reads the text a statement later)
    ListView_SetItemText(g_bindList, row, 0, (LPWSTR)keys.c_str());
    std::wstring doing = b.desc.empty() ? Doing(b) : b.desc;
    ListView_SetItemText(g_bindList, row, 1, (LPWSTR)doing.c_str());
    ListView_SetItemText(g_bindList, row, 2, (LPWSTR)when2.c_str());
    SetWindowTextW(Ctrl(idClash), Clashes(line).c_str());
    Changed();
}

void AddBind() {
    size_t at = InsertPoint();
    g_lines.insert(g_lines.begin() + at, L"bind = WIN ALT, K, exec, notepad.exe");
    FillBinds((int)at);
    ShowBind();
    Changed();
    SetFocus(Ctrl(idKey));
}

void RemoveLine(HWND list, void (*fill)(int)) {
    int line = LineOfRow(list, Selected(list));
    if (line < 0) return;
    g_lines.erase(g_lines.begin() + line);
    fill(-1);
    Changed();
}

// Moved past the next line of the same kind (comments and headings between go along with where it lands).
void MoveLine(HWND list, bool (*same)(std::wstring const&), int step, void (*fill)(int)) {
    int line = LineOfRow(list, Selected(list));
    if (line < 0) return;
    int other = line + step;
    while (other >= 0 && other < (int)g_lines.size() && !same(g_lines[other])) other += step;
    if (other < 0 || other >= (int)g_lines.size()) return;
    std::swap(g_lines[line], g_lines[other]);
    fill(other);
    Changed();
}

bool IsBindLine(std::wstring const& l) {
    BindLine b;
    return ParseBind(l, &b);
}
bool IsRuleLine(std::wstring const& l) {
    RuleLine r;
    return ParseRule(l, &r);
}

// Press keys: the keys pressed together with their modifiers, caught before anything else sees them (Win too);
// the first one let go ends it. Keys held with the last one are a combination (Esc+End: escape&end).
std::vector<UINT> g_capturedKeys;
UINT g_capturedMods = 0;
LRESULT CALLBACK CaptureProc(int code, WPARAM wp, LPARAM lp) {
    if (code == HC_ACTION && (wp == WM_KEYUP || wp == WM_SYSKEYUP) && !g_capturedKeys.empty()) {
        auto* k = (KBDLLHOOKSTRUCT*)lp;
        if (std::find(g_capturedKeys.begin(), g_capturedKeys.end(), k->vkCode) != g_capturedKeys.end()) {
            PostMessageW(g_wnd, WM_APP + 1, g_capturedMods, g_capturedKeys.back());
            return 1;
        }
    }
    if (code == HC_ACTION && (wp == WM_KEYDOWN || wp == WM_SYSKEYDOWN)) {
        auto* k = (KBDLLHOOKSTRUCT*)lp;
        switch (k->vkCode) {
            case VK_LWIN: case VK_RWIN: case VK_LCONTROL: case VK_RCONTROL: case VK_LMENU: case VK_RMENU: case VK_LSHIFT:
            case VK_RSHIFT: case VK_CONTROL: case VK_MENU: case VK_SHIFT:
                break;  // (waiting for the key itself)
            default: {
                UINT mods = 0;
                if ((GetAsyncKeyState(VK_LWIN) | GetAsyncKeyState(VK_RWIN)) & 0x8000) mods |= MOD_WIN;
                if (GetAsyncKeyState(VK_CONTROL) & 0x8000) mods |= MOD_CONTROL;
                if (GetAsyncKeyState(VK_MENU) & 0x8000) mods |= MOD_ALT;
                if (GetAsyncKeyState(VK_SHIFT) & 0x8000) mods |= MOD_SHIFT;
                if ((mods & (MOD_WIN | MOD_ALT))) {
                    // Win or Alt let go with nothing between would open Start or a menu bar: a key nothing uses first.
                    INPUT in[2]{};
                    in[0].type = in[1].type = INPUT_KEYBOARD;
                    in[0].ki.wVk = in[1].ki.wVk = 0xE8;
                    in[1].ki.dwFlags = KEYEVENTF_KEYUP;
                    SendInput(2, in, sizeof(INPUT));
                }
                g_capturedMods |= mods;
                if (std::find(g_capturedKeys.begin(), g_capturedKeys.end(), k->vkCode) == g_capturedKeys.end())
                    g_capturedKeys.push_back(k->vkCode);
                return 1;
            }
        }
    }
    return CallNextHookEx(nullptr, code, wp, lp);
}

void StopCapture() {
    if (g_captureHook) UnhookWindowsHookEx(g_captureHook);
    g_captureHook = nullptr;
    SetWindowTextW(Ctrl(idPress), L"Press keys...");
}

void StartCapture() {
    if (g_captureHook) return StopCapture();
    g_capturedKeys.clear();
    g_capturedMods = 0;
    g_captureHook = SetWindowsHookExW(WH_KEYBOARD_LL, CaptureProc, GetModuleHandleW(nullptr), 0);
    if (g_captureHook) SetWindowTextW(Ctrl(idPress), L"Press them now");
}

void Captured(UINT mods, UINT vk) {
    std::vector<UINT> held(g_capturedKeys.begin(), g_capturedKeys.end());
    if (!held.empty()) held.pop_back();  // (the others, held with the last one)
    StopCapture();
    g_capturedKeys.clear();
    if (vk == VK_ESCAPE && !mods && held.empty()) return;  // (Esc alone: as it was)
    g_filling = true;
    Check(idWin, mods & MOD_WIN);
    Check(idCtrl, mods & MOD_CONTROL);
    Check(idAlt, mods & MOD_ALT);
    Check(idShift, mods & MOD_SHIFT);
    std::wstring name = KeyNameOfVk(vk);
    for (auto it = held.rbegin(); it != held.rend(); ++it) name = KeyNameOfVk(*it) + L"&" + name;
    int k = held.empty() ? KeyIndex(name) : -1;
    ComboBox_SetCurSel(Ctrl(idKey), k);
    g_filling = false;
    if (k < 0) {  // (a key the list doesn't name: by its code)
        int line = LineOfRow(g_bindList, Selected(g_bindList));
        BindLine b;
        if (line >= 0 && ParseBind(g_lines[line], &b)) {
            b.key = name;
            b.mods = ModsText(mods);
            g_lines[line] = BindText(b);
        }
    }
    StoreBind();
}

std::wstring BrowseProgram() {
    WCHAR file[MAX_PATH] = L"";
    OPENFILENAMEW ofn{sizeof(ofn)};
    ofn.hwndOwner = g_wnd;
    ofn.lpstrFilter = L"Programs\0*.exe;*.lnk;*.bat;*.cmd;*.ps1\0Anything\0*.*\0";
    ofn.lpstrFile = file;
    ofn.nMaxFile = ARRAYSIZE(file);
    ofn.lpstrTitle = L"A program to run";
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
    if (!GetOpenFileNameW(&ofn)) return L"";
    std::wstring f = file;
    return f.find(L' ') != std::wstring::npos ? L"\"" + f + L"\"" : f;
}

////////////////////////////////////////////////////////////////////////////////
// Window rules: the picked one's properties

struct RuleField {
    int on, a, b;  // the checkbox and its boxes (0: none)
    const wchar_t* word;
};
const RuleField kRuleFields[] = {
    {idFloat, 0, 0, L"float"}, {idTile, 0, 0, L"tile"}, {idCenter, 0, 0, L"center"}, {idPin, 0, 0, L"pin"},
    {idMaximize, 0, 0, L"maximize"}, {idMinimize, 0, 0, L"minimize"}, {idNoBorder, 0, 0, L"noborder"},
    {idSizeOn, idSizeW, idSizeH, L"size"}, {idMoveOn, idMoveX, idMoveY, L"move"}, {idDeskOn, idDesk, 0, L"workspace"},
    {idScreenOn, idScreen, 0, L"monitor"}, {idOpacityOn, idOpacity, 0, L"opacity"}, {idKeep, 0, 0, L"keep"},
};

void ShowRule() {
    int line = LineOfRow(g_ruleList, Selected(g_ruleList));
    RuleLine r;
    bool have = line >= 0 && ParseRule(g_lines[line], &r);
    g_filling = true;
    std::vector<int> ids = {idMatchKind, idMatch, idPickWindow, idHereNow, idDeskSilent, idRuleRemove, idRuleUp, idRuleDown};
    for (auto const& f : kRuleFields) {
        ids.push_back(f.on);
        if (f.a) ids.push_back(f.a);
        if (f.b) ids.push_back(f.b);
    }
    for (int id : ids) EnableWindow(Ctrl(id), have);
    if (!have) r = {};
    ComboBox_SetCurSel(Ctrl(idMatchKind), r.matchKind);
    SetWindowTextW(Ctrl(idMatch), r.match.c_str());
    for (auto const& f : kRuleFields) {
        Check(f.on, false);
        if (f.a) SetWindowTextW(Ctrl(f.a), L"");
        if (f.b) SetWindowTextW(Ctrl(f.b), L"");
    }
    Check(idDeskSilent, false);
    for (auto const& a : r.actions) {
        auto [word, rest] = RuleWord(a);
        if (word == L"fullscreen") word = L"maximize";
        for (auto const& f : kRuleFields) {
            if (word != f.word) continue;
            Check(f.on, true);
            std::wstring one = rest, two;
            if (size_t sp = rest.find(L' '); sp != std::wstring::npos) one = rest.substr(0, sp), two = Trim(rest.substr(sp + 1));
            if (f.word == std::wstring(L"workspace") && Lower(two) == L"silent") Check(idDeskSilent, true), two.clear();
            if (f.a) SetWindowTextW(Ctrl(f.a), one.c_str());
            if (f.b) SetWindowTextW(Ctrl(f.b), two.c_str());
        }
    }
    g_filling = false;
}

void StoreRule() {
    if (g_filling) return;
    int row = Selected(g_ruleList), line = LineOfRow(g_ruleList, row);
    RuleLine r;
    if (line < 0 || !ParseRule(g_lines[line], &r)) return;
    // The rules the boxes don't cover stay; those they cover are written from them.
    std::vector<std::wstring> kept;
    for (auto const& a : r.actions) {
        auto [word, rest] = RuleWord(a);
        bool covered = word == L"fullscreen";
        for (auto const& f : kRuleFields) covered |= word == f.word;
        if (!covered) kept.push_back(a);
    }
    r.actions.clear();
    for (auto const& f : kRuleFields) {
        if (!Checked(f.on)) continue;
        std::wstring a = f.word;
        std::wstring one = f.a ? Trim(TextOf(f.a)) : L"", two = f.b ? Trim(TextOf(f.b)) : L"";
        if (f.a && one.empty()) continue;  // (a size with no size yet)
        if (!one.empty()) a += L" " + one;
        if (!two.empty()) a += L" " + two;
        if (f.on == idDeskOn && Checked(idDeskSilent)) a += L" silent";
        r.actions.push_back(a);
    }
    r.actions.insert(r.actions.end(), kept.begin(), kept.end());
    r.matchKind = std::clamp(ComboBox_GetCurSel(Ctrl(idMatchKind)), 0, 2);
    r.match = Trim(TextOf(idMatch));
    std::replace(r.match.begin(), r.match.end(), L',', L'?');
    g_lines[line] = RuleText(r);
    std::wstring who = std::wstring(kMatchLabels[r.matchKind]) + L": " + (r.match.empty() ? L"(any)" : r.match) + r.otherMatches;
    ListView_SetItemText(g_ruleList, row, 0, (LPWSTR)who.c_str());
    std::wstring shown = RuleShown(r);
    ListView_SetItemText(g_ruleList, row, 1, (LPWSTR)shown.c_str());
    Changed();
}

void AddRule() {
    size_t at = InsertPoint();
    g_lines.insert(g_lines.begin() + at, L"windowrule = float, center, exe:");
    FillRules((int)at);
    ShowRule();
    Changed();
    SetFocus(Ctrl(idMatch));
}

// Pick an open window: its program's file name (or class or title, as the match is set to).
HWND g_picked;  // the last window picked (for Where it is now)

bool PickWindow() {
    struct Found {
        std::wstring title, exe, cls;
        HWND h;
    };
    std::vector<Found> found;
    EnumWindows(
        [](HWND w, LPARAM p) -> BOOL {
            if (!IsWindowVisible(w) || GetWindow(w, GW_OWNER) || (GetWindowLongW(w, GWL_EXSTYLE) & WS_EX_TOOLWINDOW)) return TRUE;
            WCHAR title[200] = L"", cls[200] = L"", path[MAX_PATH] = L"";
            if (!GetWindowTextW(w, title, ARRAYSIZE(title))) return TRUE;
            GetClassNameW(w, cls, ARRAYSIZE(cls));
            DWORD pid = 0;
            GetWindowThreadProcessId(w, &pid);
            if (HANDLE h = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid)) {
                DWORD n = ARRAYSIZE(path);
                QueryFullProcessImageNameW(h, 0, path, &n);
                CloseHandle(h);
            }
            std::wstring exe = path;
            exe = exe.substr(exe.find_last_of(L"\\/") + 1);
            auto* list = (std::vector<Found>*)p;
            if (list->size() < 60) list->push_back({title, exe, cls, w});
            return TRUE;
        },
        (LPARAM)&found);
    HMENU menu = CreatePopupMenu();
    for (size_t i = 0; i < found.size(); i++) {
        std::wstring t = found[i].title.size() > 60 ? found[i].title.substr(0, 57) + L"..." : found[i].title;
        std::wstring label = t + L"\t" + found[i].exe;
        AppendMenuW(menu, MF_STRING, i + 1, label.c_str());
    }
    if (found.empty()) AppendMenuW(menu, MF_STRING | MF_GRAYED, 0, L"(no windows)");
    RECT r;
    GetWindowRect(Ctrl(idPickWindow), &r);
    int pick = (int)TrackPopupMenu(menu, TPM_RETURNCMD | TPM_NONOTIFY, r.left, r.bottom, 0, g_wnd, nullptr);
    DestroyMenu(menu);
    if (pick <= 0) return false;
    Found const& f = found[pick - 1];
    g_picked = f.h;
    int kind = ComboBox_GetCurSel(Ctrl(idMatchKind));
    std::wstring value = kind == 1 ? f.cls : kind == 2 ? f.title : f.exe;
    SetWindowTextW(Ctrl(idMatch), value.c_str());  // (StoreRule follows)
    return true;
}

// Screen, place and size from where the picked window is now (picked first if
// none is), floating and kept there: for a program that forgets its own.
void UseWhereItIs() {
    if (!IsWindow(g_picked) && !PickWindow()) return;
    HWND h = g_picked;
    if (IsZoomed(h) || IsIconic(h)) {
        MessageBoxW(g_wnd, L"Restore the window to the size you want first (not maximised or minimised).", L"Where it is now", MB_ICONINFORMATION);
        return;
    }
    RECT f{};
    if (FAILED(DwmGetWindowAttribute(h, DWMWA_EXTENDED_FRAME_BOUNDS, &f, sizeof(f)))) GetWindowRect(h, &f);
    HMONITOR on = MonitorFromWindow(h, MONITOR_DEFAULTTONEAREST);
    // Screens numbered left to right (as the Switcher's monitor N).
    std::vector<HMONITOR> all;
    EnumDisplayMonitors(nullptr, nullptr, [](HMONITOR m, HDC, LPRECT, LPARAM p) -> BOOL {
        ((std::vector<HMONITOR>*)p)->push_back(m);
        return TRUE;
    }, (LPARAM)&all);
    auto info = [](HMONITOR m) {
        MONITORINFO mi{sizeof(mi)};
        GetMonitorInfoW(m, &mi);
        return mi;
    };
    std::sort(all.begin(), all.end(), [&](HMONITOR a, HMONITOR b) {
        RECT ra = info(a).rcMonitor, rb = info(b).rcMonitor;
        return ra.left != rb.left ? ra.left < rb.left : ra.top < rb.top;
    });
    int screen = (int)(std::find(all.begin(), all.end(), on) - all.begin()) + 1;
    RECT work = info(on).rcWork;
    g_filling = true;
    Check(idFloat, true), Check(idTile, false), Check(idCenter, false), Check(idMaximize, false), Check(idMinimize, false);
    Check(idScreenOn, true), Check(idMoveOn, true), Check(idSizeOn, true), Check(idKeep, true);
    SetWindowTextW(Ctrl(idScreen), std::to_wstring(screen).c_str());
    SetWindowTextW(Ctrl(idMoveX), std::to_wstring(f.left - work.left).c_str());
    SetWindowTextW(Ctrl(idMoveY), std::to_wstring(f.top - work.top).c_str());
    SetWindowTextW(Ctrl(idSizeW), std::to_wstring(f.right - f.left).c_str());
    SetWindowTextW(Ctrl(idSizeH), std::to_wstring(f.bottom - f.top).c_str());
    g_filling = false;
    StoreRule();
}

////////////////////////////////////////////////////////////////////////////////
// When idle: the picked one's properties

void ShowIdle() {
    int line = LineOfRow(g_idleList, Selected(g_idleList));
    IdleLine l;
    bool have = line >= 0 && ParseIdle(g_lines[line], &l);
    g_filling = true;
    EnableAll({idMinutes, idSeconds, idIdleDo, idIdleArg, idIdleBrowse, idIdleRemove}, have);
    if (!have) l = {};
    SetWindowTextW(Ctrl(idMinutes), std::to_wstring(l.seconds / 60).c_str());
    SetWindowTextW(Ctrl(idSeconds), std::to_wstring(l.seconds % 60).c_str());
    int c = IdleChoiceOf(l);
    ComboBox_SetCurSel(Ctrl(idIdleDo), c);
    std::wstring arg = c == 3 ? l.command + (l.arg.empty() ? L"" : L", " + l.arg) : c == 2 ? l.arg : L"";
    SetWindowTextW(Ctrl(idIdleArg), arg.c_str());
    EnableWindow(Ctrl(idIdleArg), have && c >= 2);
    ShowWindow(Ctrl(idIdleBrowse), c == 2 ? SW_SHOWNA : SW_HIDE);
    SetWindowTextW(Ctrl(idIdleArgLabel), c == 3 ? L"Command, argument (as in a bind):" : L"Program or command:");
    g_filling = false;
}

void StoreIdle() {
    if (g_filling) return;
    int row = Selected(g_idleList), line = LineOfRow(g_idleList, row);
    IdleLine l;
    if (line < 0 || !ParseIdle(g_lines[line], &l)) return;
    l.seconds = std::max(1, _wtoi(TextOf(idMinutes).c_str()) * 60 + _wtoi(TextOf(idSeconds).c_str()));
    int c = std::clamp(ComboBox_GetCurSel(Ctrl(idIdleDo)), 0, 3);
    std::wstring arg = Trim(TextOf(idIdleArg));
    if (c <= 1) l.command = kIdleChoices[c].command, l.arg = kIdleChoices[c].arg;
    else if (c == 2) l.command = L"exec", l.arg = arg;
    else {
        auto parts = Split(arg, 2);
        l.command = Lower(parts[0]);
        l.arg = parts.size() > 1 ? parts[1] : L"";
    }
    g_lines[line] = IdleText(l);
    std::wstring after = After(l.seconds), shownIdle = IdleShown(l);  // (kept: the macro reads the text a statement later)
    ListView_SetItemText(g_idleList, row, 0, (LPWSTR)after.c_str());
    ListView_SetItemText(g_idleList, row, 1, (LPWSTR)shownIdle.c_str());
    EnableWindow(Ctrl(idIdleArg), c >= 2);
    ShowWindow(Ctrl(idIdleBrowse), c == 2 ? SW_SHOWNA : SW_HIDE);
    Changed();
}

void AddIdle() {
    size_t at = InsertPoint();
    g_lines.insert(g_lines.begin() + at, L"idle = 600, dpms, off");
    FillIdle((int)at);
    ShowIdle();
    Changed();
}

////////////////////////////////////////////////////////////////////////////////
// Reading and writing

std::wstring Setting(std::wstring const& name) {
    for (auto const& [k, v] : SettingsWithPrefix(name))
        if (k == name) return v;
    return L"";
}

void Load() {
    g_lines.clear();
    std::map<int, std::wstring> rows;
    for (auto const& [k, v] : SettingsWithPrefix(kLines)) {
        int i = _wtoi(k.c_str() + wcslen(kLines));
        if (k.size() > wcslen(kLines) && k.back() == L']') rows[i] = v;
    }
    int empty = 0;
    for (int i = 0; i < 1024 && empty < 8; i++) {  // (as the Switcher reads them)
        auto it = rows.find(i);
        std::wstring v = it == rows.end() ? L"" : it->second;
        empty = v.empty() ? empty + 1 : 0;
        g_lines.push_back(v);
    }
    std::erase_if(g_lines, [](std::wstring const& l) { return Trim(l).empty(); });
    std::wstring on = Setting(L"switcher.binds.enabled");
    g_filling = true;
    Check(idEnabled, on == L"true" || _wtoi(on.c_str()) != 0);
    g_filling = false;
    FillBinds();
    FillRules();
    FillIdle();
    FillText();
    ShowBind();
    ShowRule();
    ShowIdle();
    g_dirty = false;
    EnableWindow(Ctrl(idApply), FALSE);
    std::wstring file = Setting(L"switcher.binds.file");
    std::wstring status = file.empty() ? L"" : L"Also read: " + file;
    SetWindowTextW(Ctrl(idStatus), status.c_str());
}

bool Save() {
    if (g_tab == 3) ReadText();
    std::vector<std::pair<std::wstring, std::wstring>> set;
    size_t n = 0;
    for (auto const& l : g_lines)
        if (!Trim(l).empty()) set.push_back({kLines + std::to_wstring(n++) + L"]", l});
    if (!n) set.push_back({kLines + std::wstring(L"0]"), L""});
    bool ok = WriteStudioSettings(set, kLines, L"binds-settings") &&
              WriteStudioSettings({{L"switcher.binds.enabled", Checked(idEnabled) ? L"true" : L"false"}}, L"switcher.binds.enabled", L"binds-settings");
    if (!ok) {
        MessageBoxW(g_wnd, L"Couldn't write tourne-gui.conf, so nothing changed.", L"Keys and rules", MB_OK | MB_ICONWARNING);
        return false;
    }
    g_dirty = false;
    EnableWindow(Ctrl(idApply), FALSE);
    SetWindowTextW(Ctrl(idStatus), L"Saved.");
    return true;
}

bool MayClose() {
    if (!g_dirty) return true;
    int answer = MessageBoxW(g_wnd, L"Save the changes to your keys and rules?", L"Keys and rules", MB_YESNOCANCEL | MB_ICONQUESTION);
    return answer == IDNO || (answer == IDYES && Save());
}

void AddStarter() {
    if (MessageBoxW(g_wnd,
                    L"Add the starter set: about 40 binds, all on Win+Alt so everyday shortcuts are left alone (focus, move, "
                    L"desktops, tiling, groups, scratchpads). Binds on keys you already use are left out. Nothing is saved until OK "
                    L"or Apply.",
                    L"Keys and rules", MB_OKCANCEL | MB_ICONQUESTION) != IDOK)
        return;
    if (g_tab == 3) ReadText();
    int len = MultiByteToWideChar(CP_UTF8, 0, kStarter, -1, nullptr, 0);
    std::wstring text(len, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, kStarter, -1, text.data(), len);
    text.resize(wcslen(text.c_str()));
    std::vector<std::wstring> add;
    size_t at = 0;
    while (at <= text.size()) {
        size_t end = text.find(L'\n', at);
        std::wstring line = text.substr(at, end == std::wstring::npos ? std::wstring::npos : end - at);
        if (!line.empty() && line.back() == L'\r') line.pop_back();
        add.push_back(line);
        if (end == std::wstring::npos) break;
        at = end + 1;
    }
    // The file's opening comment (how to use it as a file) isn't wanted here.
    while (!add.empty() && (Trim(add.front()).empty() || Trim(add.front())[0] == L'#')) add.erase(add.begin());
    // In before the first key mode, each bind left out when the same keys are bound already.
    size_t at0 = InsertPoint(), put = at0;
    for (auto const& l : add) {
        g_lines.insert(g_lines.begin() + put, l);
        BindLine b;
        if (ParseBind(l, &b) && ModeAt(put).empty()) {
            UINT mods = ModFlags(WithNames(b.mods, put));
            std::wstring key = Lower(WithNames(b.key, put));
            bool clash = false;
            for (size_t i = 0; i < g_lines.size() && !clash; i++) {
                BindLine o;
                if (i >= at0 && i <= put) continue;  // (the set's own)
                clash = ParseBind(g_lines[i], &o) && ModeAt(i).empty() && ModFlags(WithNames(o.mods, i)) == mods &&
                        Lower(WithNames(o.key, i)) == key;
            }
            if (clash) {
                g_lines.erase(g_lines.begin() + put);
                continue;
            }
        }
        put++;
    }
    FillBinds();
    FillRules();
    FillIdle();
    FillText();
    ShowBind();
    Check(idEnabled, true);
    Changed();
}

////////////////////////////////////////////////////////////////////////////////
// Building it

void ShowTab(int tab) {
    if (g_tab == 3 && tab != 3) {  // (the text read back into the lists)
        ReadText();
        FillBinds();
        FillRules();
        FillIdle();
        ShowBind();
        ShowRule();
        ShowIdle();
    }
    if (tab == 3) FillText();
    g_tab = tab;
    for (auto const& p : kPlaces)
        if (p.tab >= 0) ShowWindow(Ctrl(p.id), p.tab == tab ? SW_SHOWNA : SW_HIDE);
    if (tab == 0) ShowBind();  // (the browse button's own showing)
    if (tab == 2) ShowIdle();
}

void MakeFonts() {
    if (g_font) DeleteObject(g_font);
    NONCLIENTMETRICSW m{sizeof(m)};
    SystemParametersInfoForDpi(SPI_GETNONCLIENTMETRICS, sizeof(m), &m, 0, g_dpi);
    g_font = CreateFontIndirectW(&m.lfMessageFont);
}

void Layout() {
    for (auto const& p : kPlaces) {
        HWND c = Ctrl(p.id);
        SetWindowPos(c, nullptr, S(p.x), S(p.y), S(p.w), S(p.h), SWP_NOZORDER | SWP_NOACTIVATE);
        SendMessageW(c, WM_SETFONT, (WPARAM)g_font, FALSE);
    }
    InvalidateRect(g_wnd, nullptr, TRUE);
}

HWND Make(const wchar_t* cls, const wchar_t* text, DWORD style, int id, DWORD exStyle = 0) {
    return CreateWindowExW(exStyle, cls, text, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | style, 0, 0, 0, 0, g_wnd, (HMENU)(INT_PTR)id,
                           GetModuleHandleW(nullptr), nullptr);
}

HWND MakeList(int id) {
    HWND l = Make(WC_LISTVIEWW, L"", WS_TABSTOP | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL, id, WS_EX_CLIENTEDGE);
    ListView_SetExtendedListViewStyle(l, LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
    return l;
}

void Build() {
    HWND tabs = Make(WC_TABCONTROLW, L"", WS_TABSTOP, idTabs);
    const wchar_t* names[] = {L"Key binds", L"Window rules", L"When idle", L"As text"};
    for (int i = 0; i < 4; i++) {
        TCITEMW t{};
        t.mask = TCIF_TEXT;
        t.pszText = (LPWSTR)names[i];
        TabCtrl_InsertItem(tabs, i, &t);
    }
    // Key binds
    g_bindList = MakeList(idBindList);
    AddColumn(g_bindList, 0, L"Keys", 140);
    AddColumn(g_bindList, 1, L"Does", 186);
    AddColumn(g_bindList, 2, L"When", 60);
    Make(L"BUTTON", L"Add", WS_TABSTOP, idBindAdd);
    Make(L"BUTTON", L"Remove", WS_TABSTOP, idBindRemove);
    Make(L"BUTTON", L"Move up", WS_TABSTOP, idBindUp);
    Make(L"BUTTON", L"Move down", WS_TABSTOP, idBindDown);
    Make(L"BUTTON", L"This bind", BS_GROUPBOX, idBindProps);
    Make(L"STATIC", L"Keys:", 0, idKeysLabel);
    Make(L"BUTTON", L"Win", WS_TABSTOP | BS_AUTOCHECKBOX, idWin);
    Make(L"BUTTON", L"Ctrl", WS_TABSTOP | BS_AUTOCHECKBOX, idCtrl);
    Make(L"BUTTON", L"Alt", WS_TABSTOP | BS_AUTOCHECKBOX, idAlt);
    Make(L"BUTTON", L"Shift", WS_TABSTOP | BS_AUTOCHECKBOX, idShift);
    HWND key = Make(WC_COMBOBOXW, L"", WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWNLIST, idKey);
    for (auto const& k : KeyNames()) ComboBox_AddString(key, k.label);
    Make(L"BUTTON", L"Press keys...", WS_TABSTOP, idPress);
    Make(L"STATIC", L"What it does:", 0, idActionLabel);
    HWND action = Make(WC_COMBOBOXW, L"", WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWNLIST, idAction);
    for (auto const& a : kActions) ComboBox_AddString(action, a.label);
    ComboBox_AddString(action, L"Another command (see As text)");
    Make(L"STATIC", L"With:", 0, idArgLabel);
    Make(WC_COMBOBOXW, L"", WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWN | CBS_AUTOHSCROLL, idArg);
    Make(L"BUTTON", L"...", WS_TABSTOP, idArgBrowse);
    Make(L"STATIC", L"", 0, idArgHelp);
    Make(L"STATIC", L"When it runs:", 0, idWhenLabel);
    HWND when = Make(WC_COMBOBOXW, L"", WS_TABSTOP | CBS_DROPDOWNLIST, idWhen);
    for (const wchar_t* w : {L"When pressed", L"When pressed, and again while held", L"When let go", L"Double press (twice quickly)",
                             L"Held down for 0.3 s", L"Held down for 0.5 s", L"Held down for 0.8 s", L"Held down for 1.2 s",
                             L"Held down for 2 s"})
        ComboBox_AddString(when, w);
    Make(L"BUTTON", L"Also in games and fullscreen programs", WS_TABSTOP | BS_AUTOCHECKBOX, idInGames);
    Make(L"BUTTON", L"The program in front hears the keys too", WS_TABSTOP | BS_AUTOCHECKBOX, idPassThrough);
    Make(L"STATIC", L"Description (the Keybind Guide shows it):", 0, idDescLabel);
    Make(L"EDIT", L"", WS_TABSTOP | ES_AUTOHSCROLL, idDesc, WS_EX_CLIENTEDGE);
    Make(L"STATIC", L"", 0, idClash);
    // Window rules
    g_ruleList = MakeList(idRuleList);
    AddColumn(g_ruleList, 0, L"Windows", 170);
    AddColumn(g_ruleList, 1, L"As they open", 216);
    Make(L"BUTTON", L"Add", WS_TABSTOP, idRuleAdd);
    Make(L"BUTTON", L"Remove", WS_TABSTOP, idRuleRemove);
    Make(L"BUTTON", L"Move up", WS_TABSTOP, idRuleUp);
    Make(L"BUTTON", L"Move down", WS_TABSTOP, idRuleDown);
    Make(L"BUTTON", L"This rule", BS_GROUPBOX, idRuleProps);
    Make(L"STATIC", L"For windows whose:", 0, idMatchLabel);
    HWND kind = Make(WC_COMBOBOXW, L"", WS_TABSTOP | CBS_DROPDOWNLIST, idMatchKind);
    for (const wchar_t* k : {L"Program is", L"Class is", L"Title is"}) ComboBox_AddString(kind, k);
    Make(L"EDIT", L"", WS_TABSTOP | ES_AUTOHSCROLL, idMatch, WS_EX_CLIENTEDGE);
    Make(L"BUTTON", L"Pick an open window...", WS_TABSTOP, idPickWindow);
    Make(L"BUTTON", L"Float", WS_TABSTOP | BS_AUTOCHECKBOX, idFloat);
    Make(L"BUTTON", L"Tile", WS_TABSTOP | BS_AUTOCHECKBOX, idTile);
    Make(L"BUTTON", L"Centre", WS_TABSTOP | BS_AUTOCHECKBOX, idCenter);
    Make(L"BUTTON", L"Keep on top", WS_TABSTOP | BS_AUTOCHECKBOX, idPin);
    Make(L"BUTTON", L"Maximise", WS_TABSTOP | BS_AUTOCHECKBOX, idMaximize);
    Make(L"BUTTON", L"Minimise", WS_TABSTOP | BS_AUTOCHECKBOX, idMinimize);
    Make(L"BUTTON", L"No border", WS_TABSTOP | BS_AUTOCHECKBOX, idNoBorder);
    Make(L"BUTTON", L"Size", WS_TABSTOP | BS_AUTOCHECKBOX, idSizeOn);
    Make(L"EDIT", L"", WS_TABSTOP | ES_AUTOHSCROLL, idSizeW, WS_EX_CLIENTEDGE);
    Make(L"EDIT", L"", WS_TABSTOP | ES_AUTOHSCROLL, idSizeH, WS_EX_CLIENTEDGE);
    Make(L"BUTTON", L"Place at", WS_TABSTOP | BS_AUTOCHECKBOX, idMoveOn);
    Make(L"EDIT", L"", WS_TABSTOP | ES_AUTOHSCROLL, idMoveX, WS_EX_CLIENTEDGE);
    Make(L"EDIT", L"", WS_TABSTOP | ES_AUTOHSCROLL, idMoveY, WS_EX_CLIENTEDGE);
    Make(L"BUTTON", L"On desktop", WS_TABSTOP | BS_AUTOCHECKBOX, idDeskOn);
    Make(L"EDIT", L"", WS_TABSTOP | ES_NUMBER, idDesk, WS_EX_CLIENTEDGE);
    Make(L"BUTTON", L"without going there", WS_TABSTOP | BS_AUTOCHECKBOX, idDeskSilent);
    Make(L"BUTTON", L"On screen", WS_TABSTOP | BS_AUTOCHECKBOX, idScreenOn);
    Make(L"EDIT", L"", WS_TABSTOP | ES_NUMBER, idScreen, WS_EX_CLIENTEDGE);
    Make(L"BUTTON", L"Where it is now", WS_TABSTOP, idHereNow);
    Make(L"BUTTON", L"Keep it there", WS_TABSTOP | BS_AUTOCHECKBOX, idKeep);
    Make(L"BUTTON", L"Opacity", WS_TABSTOP | BS_AUTOCHECKBOX, idOpacityOn);
    Make(L"EDIT", L"", WS_TABSTOP | ES_AUTOHSCROLL, idOpacity, WS_EX_CLIENTEDGE);
    Make(L"STATIC",
         L"Each rule is for a window as it opens. A program is its file name (notepad.exe); * matches any text. Size and place "
         L"take pixels or % of the screen (50% 60%). Opacity is 0 to 1 (0.9). Where it is now fills screen, place and size from the "
         L"window picked; Keep it there puts them back for 15 seconds if the program moves itself.",
         0, idRuleHelp);
    // When idle
    g_idleList = MakeList(idIdleList);
    AddColumn(g_idleList, 0, L"After", 110);
    AddColumn(g_idleList, 1, L"Does", 276);
    Make(L"BUTTON", L"Add", WS_TABSTOP, idIdleAdd);
    Make(L"BUTTON", L"Remove", WS_TABSTOP, idIdleRemove);
    Make(L"BUTTON", L"This one", BS_GROUPBOX, idIdleProps);
    Make(L"STATIC", L"After this long without a key or the mouse:", 0, idAfterLabel);
    Make(L"EDIT", L"", WS_TABSTOP | ES_NUMBER, idMinutes, WS_EX_CLIENTEDGE);
    Make(L"STATIC", L"minutes", 0, idMinutesLabel);
    Make(L"EDIT", L"", WS_TABSTOP | ES_NUMBER, idSeconds, WS_EX_CLIENTEDGE);
    Make(L"STATIC", L"seconds", 0, idSecondsLabel);
    Make(L"STATIC", L"Do:", 0, idIdleDoLabel);
    HWND doIt = Make(WC_COMBOBOXW, L"", WS_TABSTOP | CBS_DROPDOWNLIST, idIdleDo);
    for (auto const& c : kIdleChoices) ComboBox_AddString(doIt, c.label);
    Make(L"STATIC", L"Program or command:", 0, idIdleArgLabel);
    Make(L"EDIT", L"", WS_TABSTOP | ES_AUTOHSCROLL, idIdleArg, WS_EX_CLIENTEDGE);
    Make(L"BUTTON", L"...", WS_TABSTOP, idIdleBrowse);
    Make(L"STATIC",
         L"Each runs once you've been away that long (and again after the next time). Not while a fullscreen program "
         L"(a game, a video) is in front.",
         0, idIdleHelp);
    // As text
    Make(L"EDIT", L"", WS_TABSTOP | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | WS_VSCROLL | WS_HSCROLL | ES_WANTRETURN, idText,
         WS_EX_CLIENTEDGE);
    Make(L"STATIC",
         L"Every line as the Switcher reads it, written as Hyprland's: bind, windowrule, idle, $name = value, submap = name. "
         L"Lines the other tabs don't show stay as they are. A \"# Heading\" line above binds groups them in the Keybind Guide.",
         0, idTextHelp);
    // Below
    Make(L"BUTTON", L"Key binds on", WS_TABSTOP | BS_AUTOCHECKBOX, idEnabled);
    Make(L"BUTTON", L"Add the starter set...", WS_TABSTOP, idStarter);
    Make(L"STATIC", L"", SS_ENDELLIPSIS, idStatus);
    Make(L"BUTTON", L"OK", WS_TABSTOP | BS_DEFPUSHBUTTON, idOk);
    Make(L"BUTTON", L"Cancel", WS_TABSTOP, idCancel);
    Make(L"BUTTON", L"Apply", WS_TABSTOP, idApply);
    Layout();
    for (int id : {idBindProps, idRuleProps, idIdleProps, idTabs})
        SetWindowPos(Ctrl(id), HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    // (a fixed-width text: lines line up)
    static HFONT mono = CreateFontW(-S(13), 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, FIXED_PITCH, L"Consolas");
    SendMessageW(Ctrl(idText), WM_SETFONT, (WPARAM)mono, FALSE);
    ShowTab(0);
}

LRESULT OnNotify(NMHDR* h) {
    if (h->idFrom == idTabs && h->code == TCN_SELCHANGE) {
        ShowTab(TabCtrl_GetCurSel(h->hwndFrom));
        return 0;
    }
    if (h->code == LVN_ITEMCHANGED && !g_filling) {
        auto* v = (NMLISTVIEW*)h;
        if (!(v->uChanged & LVIF_STATE) || !((v->uNewState ^ v->uOldState) & LVIS_SELECTED)) return 0;
        if (h->idFrom == idBindList) ShowBind();
        else if (h->idFrom == idRuleList) ShowRule();
        else if (h->idFrom == idIdleList) ShowIdle();
    }
    if (h->code == LVN_KEYDOWN && ((NMLVKEYDOWN*)h)->wVKey == VK_DELETE) {
        if (h->idFrom == idBindList) RemoveLine(g_bindList, FillBinds), ShowBind();
        else if (h->idFrom == idRuleList) RemoveLine(g_ruleList, FillRules), ShowRule();
        else if (h->idFrom == idIdleList) RemoveLine(g_idleList, FillIdle), ShowIdle();
    }
    return 0;
}

LRESULT CALLBACK WndProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_CREATE:
            g_wnd = wnd;
            g_dpi = GetDpiForWindow(wnd);
            MakeFonts();
            Build();
            Load();
            return 0;
        case WM_APP + 1:
            Captured((UINT)wp, (UINT)lp);
            return 0;
        case WM_COMMAND: {
            int id = LOWORD(wp), code = HIWORD(wp);
            if (code == EN_CHANGE) {
                if (id == idDesc) StoreBind();
                else if (id == idMatch || id == idSizeW || id == idSizeH || id == idMoveX || id == idMoveY || id == idDesk || id == idScreen ||
                         id == idOpacity)
                    StoreRule();
                else if (id == idMinutes || id == idSeconds || id == idIdleArg) StoreIdle();
                else if (id == idText && !g_filling) Changed();
                return 0;
            }
            if (code == CBN_SELCHANGE || code == CBN_EDITCHANGE) {
                if (id == idAction && code == CBN_SELCHANGE) {
                    int a = ComboBox_GetCurSel(Ctrl(idAction));
                    const Action* act = a >= 0 && a < (int)ARRAYSIZE(kActions) ? &kActions[a] : nullptr;
                    auto sug = act ? Suggestions(act->arg) : std::vector<std::pair<std::wstring, std::wstring>>{};
                    g_filling = true;
                    FillArgChoices(act, sug.empty() ? L"" : sug[0].first);
                    g_filling = false;
                    StoreBind();
                } else if (id == idArg && code == CBN_SELCHANGE) {
                    // (the edit shows the picked text only after this message: set it now)
                    int i = ComboBox_GetCurSel(Ctrl(idArg));
                    int a = ComboBox_GetCurSel(Ctrl(idAction));
                    if (i >= 0 && a >= 0 && a < (int)ARRAYSIZE(kActions)) {
                        auto sug = Suggestions(kActions[a].arg);
                        if (i < (int)sug.size()) {
                            g_filling = true;
                            SetWindowTextW(Ctrl(idArg), sug[i].first.c_str());
                            g_filling = false;
                        }
                    }
                    StoreBind();
                } else if (id == idArg || id == idKey || id == idWhen) StoreBind();
                else if (id == idMatchKind) StoreRule();
                else if (id == idIdleDo) StoreIdle();
                return 0;
            }
            if (code != BN_CLICKED && code != 0) return 0;
            switch (id) {
                case idBindAdd: AddBind(); break;
                case idBindRemove: RemoveLine(g_bindList, FillBinds), ShowBind(); break;
                case idBindUp: MoveLine(g_bindList, IsBindLine, -1, FillBinds); break;
                case idBindDown: MoveLine(g_bindList, IsBindLine, 1, FillBinds); break;
                case idPress: StartCapture(); break;
                case idWin: case idCtrl: case idAlt: case idShift: case idInGames: case idPassThrough: StoreBind(); break;
                case idArgBrowse:
                    if (std::wstring p = BrowseProgram(); !p.empty()) SetWindowTextW(Ctrl(idArg), p.c_str()), StoreBind();
                    break;
                case idRuleAdd: AddRule(); break;
                case idRuleRemove: RemoveLine(g_ruleList, FillRules), ShowRule(); break;
                case idRuleUp: MoveLine(g_ruleList, IsRuleLine, -1, FillRules); break;
                case idRuleDown: MoveLine(g_ruleList, IsRuleLine, 1, FillRules); break;
                case idPickWindow: PickWindow(); break;
                case idFloat: case idTile: case idCenter: case idPin: case idMaximize: case idMinimize: case idNoBorder: case idSizeOn:
                case idMoveOn: case idDeskOn: case idDeskSilent: case idScreenOn: case idOpacityOn: case idKeep:
                    StoreRule();
                    break;
                case idHereNow: UseWhereItIs(); break;
                case idIdleAdd: AddIdle(); break;
                case idIdleRemove: RemoveLine(g_idleList, FillIdle), ShowIdle(); break;
                case idIdleBrowse:
                    if (std::wstring p = BrowseProgram(); !p.empty()) SetWindowTextW(Ctrl(idIdleArg), p.c_str());
                    break;
                case idEnabled: Changed(); break;
                case idStarter: AddStarter(); break;
                case idOk:
                case IDOK:
                    if (!g_dirty || Save()) DestroyWindow(wnd);
                    break;
                case idCancel:
                case IDCANCEL: DestroyWindow(wnd); break;
                case idApply: Save(); break;
            }
            return 0;
        }
        case WM_NOTIFY: return OnNotify((NMHDR*)lp);
        case WM_CTLCOLORSTATIC: {  // (labels on the tab pages: the page's white)
            HWND c = (HWND)lp;
            int id = GetDlgCtrlID(c);
            if (id == idStatus || id == idEnabled) break;
            HDC dc = (HDC)wp;
            SetBkColor(dc, GetSysColor(COLOR_WINDOW));
            SetTextColor(dc, id == idClash ? RGB(0xA0, 0x40, 0x10) : GetSysColor(IsWindowEnabled(c) ? COLOR_WINDOWTEXT : COLOR_GRAYTEXT));
            return (LRESULT)GetSysColorBrush(COLOR_WINDOW);
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC dc = BeginPaint(wnd, &ps);
            RECT page{S(kPage.left), S(kPage.top), S(kPage.right), S(kPage.bottom)};
            FillRect(dc, &page, GetSysColorBrush(COLOR_WINDOW));
            FrameRect(dc, &page, GetSysColorBrush(COLOR_3DSHADOW));
            EndPaint(wnd, &ps);
            return 0;
        }
        case WM_DPICHANGED: {
            g_dpi = HIWORD(wp);
            MakeFonts();
            RECT* r = (RECT*)lp;
            SetWindowPos(wnd, nullptr, r->left, r->top, r->right - r->left, r->bottom - r->top, SWP_NOZORDER | SWP_NOACTIVATE);
            Layout();
            return 0;
        }
        case WM_CLOSE:
            if (MayClose()) DestroyWindow(wnd);
            return 0;
        case WM_DESTROY:
            StopCapture();
            g_wnd = g_bindList = g_ruleList = g_idleList = nullptr;
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(wnd, msg, wp, lp);
}

DWORD WINAPI EditorThread(void*) {
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    INITCOMMONCONTROLSEX icc{sizeof(icc), ICC_LISTVIEW_CLASSES | ICC_TAB_CLASSES | ICC_STANDARD_CLASSES};
    InitCommonControlsEx(&icc);
    WNDCLASSEXW wc{sizeof(wc)};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
    wc.hIcon = LoadIconW(GetModuleHandleW(nullptr), MAKEINTRESOURCEW(1));
    wc.lpszClassName = kClass;
    RegisterClassExW(&wc);
    DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    POINT cursor;
    GetCursorPos(&cursor);
    MONITORINFO mi{sizeof(mi)};
    GetMonitorInfoW(MonitorFromPoint(cursor, MONITOR_DEFAULTTONEAREST), &mi);
    UINT dpi = GetDpiForSystem();
    RECT r{0, 0, MulDiv(kClientW, dpi, 96), MulDiv(kClientH, dpi, 96)};
    AdjustWindowRectExForDpi(&r, style, FALSE, 0, dpi);
    int w = r.right - r.left, h = r.bottom - r.top;
    int x = (mi.rcWork.left + mi.rcWork.right - w) / 2, y = (mi.rcWork.top + mi.rcWork.bottom - h) / 2;
    HWND wnd = CreateWindowExW(0, kClass, L"Keys and rules", style, x, y, w, h, nullptr, nullptr, wc.hInstance, nullptr);
    if (wnd) {
        if (UINT own = GetDpiForWindow(wnd); own != dpi) {
            RECT c{0, 0, MulDiv(kClientW, own, 96), MulDiv(kClientH, own, 96)};
            AdjustWindowRectExForDpi(&c, style, FALSE, 0, own);
            SetWindowPos(wnd, nullptr, 0, 0, c.right - c.left, c.bottom - c.top, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
        }
        ShowWindow(wnd, SW_SHOWNORMAL);
        SetForegroundWindow(wnd);
        SetFocus(g_bindList);
        MSG m;
        while (GetMessageW(&m, nullptr, 0, 0) > 0) {
            if (g_wnd && m.message == WM_KEYDOWN && m.wParam == VK_ESCAPE && !g_captureHook) {
                SendMessageW(g_wnd, WM_CLOSE, 0, 0);
                continue;
            }
            // (Enter and Tab stay in the text page)
            bool text = g_wnd && m.hwnd == GetDlgItem(g_wnd, idText);
            if (text || !g_wnd || !IsDialogMessageW(g_wnd, &m)) {
                TranslateMessage(&m);
                DispatchMessageW(&m);
            }
        }
    }
    UnregisterClassW(kClass, GetModuleHandleW(nullptr));
    CoUninitialize();
    g_open = false;
    return 0;
}

}  // namespace

void OpenBindsEditor() {
    if (g_open.exchange(true)) {
        if (HWND w = FindWindowW(kClass, nullptr)) {
            if (IsIconic(w)) ShowWindow(w, SW_RESTORE);
            SetForegroundWindow(w);
        }
        return;
    }
    if (HANDLE t = CreateThread(nullptr, 0, EditorThread, nullptr, 0, nullptr)) CloseHandle(t);
    else g_open = false;
}

void RunBindsEditor() {
    if (!g_open.exchange(true)) EditorThread(nullptr);
}

}  // namespace td
