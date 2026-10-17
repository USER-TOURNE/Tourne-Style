#include "studio.h"

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <commdlg.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <shellapi.h>
#include <d2d1.h>
#include <dwmapi.h>
#include <dwrite.h>
#include <wrl/client.h>

#include <algorithm>
#include <cmath>
#include <iterator>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "glass.h"
#include "plugin-runtime.h"
#include "plugins.h"
#include "extensions.h"
#include "dots.h"
#include "presets.h"
#include "menu-editor.h"
#include "binds-editor.h"
#include "td.h"

#include "motion.cpp"

// Which build this is (commit and date), in the window's title.
#if __has_include("build-stamp.h")
#include "build-stamp.h"
#endif
#ifndef TD_BUILD
#define TD_BUILD L"unknown"
#endif

using Microsoft::WRL::ComPtr;

namespace td {

namespace {

////////////////////////////////////////////////////////////////////////////////
// What's shown: the settings, what you've changed and not saved, and the
// steps to undo.

enum class Kind { Switch, Choice, Slider, Number, Colour, Text, Pick };

struct Change {
    bool remove = false;  // take it out of tourne-gui.conf (yours or the default again)
    std::wstring value;
    bool operator==(const Change&) const = default;
};

struct Edit {
    std::wstring key;
    std::optional<Change> before, after;
};
using Step = std::vector<Edit>;  // one undo: usually one setting, a whole look at once

struct Model {
    Settings user, gui;                     // tourne.conf, tourne-gui.conf as saved
    std::map<std::wstring, Change> changes;  // not saved yet
    std::vector<Step> undo, redo;
} g_model;

const std::wstring* Find(const Settings& s, const std::wstring& key) {
    auto it = s.find(key);
    return it == s.end() ? nullptr : &it->second;
}

std::wstring ValueOf(const std::wstring& key);

// A glass setting's default is its look's.
std::wstring DefaultOf(const std::wstring& key) {
    if (GlassIsLookSetting(key)) {
        std::wstring v = GlassLookValue(ValueOf(L"glass.look"), key);
        if (!v.empty()) return v;
    }
    auto v = Find(DefaultSettings(), key);
    return v ? *v : L"";
}

// What it is with nothing changed: the Studio's, else yours, else the default.
std::wstring SavedValue(const std::wstring& key) {
    if (auto v = Find(g_model.gui, key)) return *v;
    if (auto v = Find(g_model.user, key)) return *v;
    return DefaultOf(key);
}

std::wstring ValueOf(const std::wstring& key) {
    auto c = g_model.changes.find(key);
    if (c == g_model.changes.end()) return SavedValue(key);
    if (!c->second.remove) return c->second.value;
    if (auto v = Find(g_model.user, key)) return *v;
    return DefaultOf(key);
}

const SettingInfo* InfoOf(const std::wstring& key) {
    auto const& info = SettingsSchema();
    auto it = info.find(key);
    return it == info.end() ? nullptr : &it->second;
}

bool IsHex6(std::wstring v) {
    if (!v.empty() && v[0] == L'#') v.erase(0, 1);
    return v.size() == 6 && v.find_first_not_of(L"0123456789abcdefABCDEF") == std::wstring::npos;
}

bool IsInt(const std::wstring& v) {
    size_t i = !v.empty() && v[0] == L'-';
    return i < v.size() && v.find_first_not_of(L"0123456789", i) == std::wstring::npos;
}

bool IsNumber(const std::wstring& v) {
    wchar_t* end = nullptr;
    wcstod(v.c_str(), &end);
    return !v.empty() && end && !*end && v.find_first_not_of(L"-0123456789.") == std::wstring::npos;
}

std::wstring Lower(std::wstring s) {
    for (auto& c : s) c = towlower(c);
    return s;
}

std::wstring LastPart(const std::wstring& key) {
    size_t dot = key.rfind(L'.');
    return dot == std::wstring::npos ? key : key.substr(dot + 1);
}

std::wstring Parent(const std::wstring& key) {
    size_t dot = key.rfind(L'.');
    return dot == std::wstring::npos ? L"" : key.substr(0, dot);
}

////////////////////////////////////////////////////////////////////////////////
// Settings with a set of answers that aren't a fixed list (programs, icons, keys...): a menu of what's there
// to pick from, with typing it out kept as the last item, rather than a bare text box.

enum class Pick { None, Programs, Program, Titles, Classes, Icon, Folder, PlaceTarget, File, Hotkey, Font, StyleSheets, Command, Motion, Gradient, MenuTree, BindsEditor };

// The Tourne Menu's rows (taskbar.menu.items[i]: depth, name, command, args) show as one row that opens its own
// window, laid out like LiteMenu's (menu-editor.cpp): listed flat, submenus in submenus were hard to follow.
constexpr wchar_t kMenuRows[] = L"taskbar.menu.items[";
constexpr wchar_t kMenuTreeKey[] = L"taskbar.menu.items[0].name";  // (the row that stands for them all)
bool MenuRowKey(const std::wstring& key) { return key.rfind(kMenuRows, 0) == 0; }
// The Switcher's bind lines (binds, window rules, idle) likewise: one row that opens "Keys and rules" (binds-editor.cpp).
constexpr wchar_t kBindRows[] = L"switcher.binds.lines[";
constexpr wchar_t kBindsKey[] = L"switcher.binds.lines[0]";
bool BindRowKey(const std::wstring& key) { return key.rfind(kBindRows, 0) == 0; }

struct PickInfo {
    Pick type = Pick::None;
    std::wstring separator = L", ";  // a list's, as the setting reads it
};

bool KeyMatches(const std::wstring& key, const wchar_t* pattern) {  // * for any run, like taskbar.buttonIcons[*].icon
    std::wstring_view k = key, p = pattern;
    size_t star = p.find(L'*');
    if (star == std::wstring_view::npos) return k == p;
    std::wstring_view head = p.substr(0, star), tail = p.substr(star + 1);
    return k.size() >= head.size() + tail.size() && k.substr(0, head.size()) == head && k.substr(k.size() - tail.size()) == tail;
}

PickInfo PickOf(const std::wstring& key) {
    const SettingInfo* info = InfoOf(key);
    if (info && info->format == L"hotkey") return {Pick::Hotkey};
    struct Entry {
        const wchar_t* pattern;
        Pick type;
        const wchar_t* separator;
    };
    static const Entry kEntries[] = {
        {L"shell.exclude", Pick::Programs, L"|"},
        {L"shell.excludeKeys", Pick::Programs, L"|"},
        {L"switcher.borders.skipApps", Pick::Programs, L", "},
        {L"switcher.tiling.floatApps", Pick::Programs, L", "},
        {L"switcher.windows.excludeApps", Pick::Programs, L", "},
        {L"systemIcons.doubleClickApps", Pick::Programs, L", "},
        {L"switcher.borders.skipTitles", Pick::Titles, L", "},
        {L"switcher.tiling.floatTitles", Pick::Titles, L", "},
        {L"switcher.windows.excludeTitles", Pick::Titles, L", "},
        {L"switcher.borders.skipClasses", Pick::Classes, L", "},
        {L"switcher.tiling.floatClasses", Pick::Classes, L", "},
        {L"glass.app.*.exe", Pick::Program, L""},
        {L"inject.*.exe", Pick::Program, L""},
        {L"switcher.customHeaders[*].program", Pick::Program, L""},
        {L"taskbar.buttonIcons[*].program", Pick::Program, L""},
        {L"taskbar.fullscreenPrograms[*]", Pick::Program, L""},
        {L"taskbar.buttonIcons[*].icon", Pick::Icon, L""},
        {L"switcher.customHeaders[*].icon", Pick::Icon, L""},
        {L"taskbar.startMenu.*Icon", Pick::Icon, L""},
        {L"shell.wallpaper.folder", Pick::Folder, L""},
        {L"plugins.*notes_dir", Pick::Folder, L""},
        {L"taskbar.startMenu.rightSide.customFolders[*].path", Pick::PlaceTarget, L""},
        {L"systemIcons.calendarIcs", Pick::File, L""},
        {L"switcher.tiling.leader", Pick::Hotkey, L""},
        {L"taskbar.menu.hotkey", Pick::Hotkey, L""},
        {L"taskbar.hideHotkey", Pick::Hotkey, L""},
        {L"taskbar.desktopKeys.*", Pick::Hotkey, L""},
        {L"switcher.font.family", Pick::Font, L""},
        {L"inject.*.css", Pick::StyleSheets, L", "},
        {L"taskbar.menu.items[0].name", Pick::MenuTree, L""},
        {L"switcher.binds.lines[0]", Pick::BindsEditor, L""},
        {L"taskbar.menu.items[*].command", Pick::Command, L""},
        {L"switcher.borders.activeColor", Pick::Gradient, L""},
        {L"switcher.borders.inactiveColor", Pick::Gradient, L""},
    };
    for (auto const& e : kEntries)
        if (KeyMatches(key, e.pattern)) return {e.type, e.separator};
    if (key.rfind(L"motion.", 0) == 0)  // (the motion tree's lines)
        for (auto const& leaf : motion::Tree())
            if (key.compare(7, std::wstring::npos, leaf.name) == 0) return {Pick::Motion};
    return {};
}

// Colours that don't say so in their name (the previews', the desktop icons' look).
bool ColourKey(const std::wstring& key) {
    static const wchar_t* const kColours[] = {
        L"taskbar.previewBackground", L"taskbar.previewText",   L"taskbar.previewHover",      L"taskbar.previewHoverBorder",
        L"taskbar.previewActive",     L"taskbar.previewFlashing", L"taskbar.previewCloseHover", L"taskbar.previewCloseGlyph",
        L"shell.iconLook.pickedFill", L"shell.iconLook.pickedEdge", L"shell.iconLook.hoverFill",  L"shell.iconLook.hoverEdge",
    };
    for (auto k : kColours)
        if (key == k) return true;
    return false;
}

Kind KindOf(const std::wstring& key) {
    const SettingInfo* info = InfoOf(key);
    std::wstring def = DefaultOf(key);
    if (info && info->isSwitch) return Kind::Switch;
    if (info && !info->options.empty()) return Kind::Choice;
    if (info && info->hasMin && info->hasMax && IsInt(def)) return Kind::Slider;
    if (PickOf(key).type != Pick::None) return Kind::Pick;
    std::wstring name = Lower(LastPart(key)), group = Lower(LastPart(Parent(key))), shown = info ? Lower(info->name) : L"";
    bool colourName = name.find(L"color") != std::wstring::npos || name.find(L"colour") != std::wstring::npos ||
                      shown.find(L"colour") != std::wstring::npos || shown.find(L"color") != std::wstring::npos ||
                      group == L"colors" || group == L"lightcolors" || name.find(L"seed") != std::wstring::npos;
    if (ColourKey(key)) return Kind::Colour;
    if (IsHex6(def) || (colourName && def.empty())) return Kind::Colour;
    if (IsNumber(def) || (info && (info->hasMin || info->hasMax))) return Kind::Number;
    return Kind::Text;
}

bool IsOn(const std::wstring& v) { return ToInt(v) != 0; }

void Live() {
    Settings set;
    std::set<std::wstring> removed;
    for (auto const& [k, c] : g_model.changes)
        if (c.remove) removed.insert(k);
        else set[k] = c.value;
    SetLiveLayer(set, removed);
}

std::optional<Change> ChangeOf(const std::wstring& key) {
    auto it = g_model.changes.find(key);
    if (it == g_model.changes.end()) return std::nullopt;
    return it->second;
}

void Put(const std::wstring& key, const std::optional<Change>& c) {
    if (c) g_model.changes[key] = *c;
    else g_model.changes.erase(key);
}

bool Same(const std::wstring& key, const std::wstring& a, const std::wstring& b) {
    if (a == b) return true;
    return KindOf(key) == Kind::Switch && IsOn(a) == IsOn(b);
}

// Changes as one step to undo.
void RecordMany(const std::vector<std::pair<std::wstring, std::optional<Change>>>& changes) {
    Step step;
    for (auto const& [key, after] : changes) {
        std::optional<Change> before = ChangeOf(key);
        if (before == after) continue;
        step.push_back({key, before, after});
        Put(key, after);
    }
    if (step.empty()) return;
    g_model.undo.push_back(step);
    g_model.redo.clear();
    Live();
}

void Record(const std::wstring& key, std::optional<Change> after) { RecordMany({{key, after}}); }

void SetValue(const std::wstring& key, const std::wstring& value) {
    if (Same(key, value, SavedValue(key))) Record(key, std::nullopt);  // back to what's saved
    else Record(key, Change{false, value});
}

// Out of the Studio's file (the line in tourne.conf, or the default, again).
void Reset(const std::wstring& key) {
    if (Find(g_model.gui, key)) Record(key, Change{true, L""});
    else Record(key, std::nullopt);
}

void Undo() {
    if (g_model.undo.empty()) return;
    Step s = g_model.undo.back();
    g_model.undo.pop_back();
    for (auto it = s.rbegin(); it != s.rend(); ++it) Put(it->key, it->before);
    g_model.redo.push_back(s);
    Live();
}

void Redo() {
    if (g_model.redo.empty()) return;
    Step s = g_model.redo.back();
    g_model.redo.pop_back();
    for (auto const& e : s) Put(e.key, e.after);
    g_model.undo.push_back(s);
    Live();
}

// A glass look: the look, and every look setting the Studio set taken back
// out, so the look's own values show (one step to undo).
void SetLook(const std::wstring& look) {
    std::vector<std::pair<std::wstring, std::optional<Change>>> changes;
    std::wstring saved = SavedValue(L"glass.look");
    changes.push_back({L"glass.look", look == saved ? std::nullopt : std::optional<Change>(Change{false, look})});
    for (auto const& [key, value] : DefaultSettings()) {
        if (!GlassIsLookSetting(key)) continue;
        if (Find(g_model.gui, key)) changes.push_back({key, Change{true, L""}});
        else if (g_model.changes.count(key)) changes.push_back({key, std::nullopt});
    }
    RecordMany(changes);
}

void Discard() {
    g_model.changes.clear();
    g_model.undo.clear();
    g_model.redo.clear();
    Live();
}

bool Save() {
    Settings gui = g_model.gui;
    for (auto const& [k, c] : g_model.changes)
        if (c.remove) gui.erase(k);
        else gui[k] = c.value;
    if (!SaveGuiLayer(gui)) return false;
    ConfigLayers(&g_model.user, &g_model.gui);
    g_model.changes.clear();
    g_model.undo.clear();
    g_model.redo.clear();
    return true;
}

////////////////////////////////////////////////////////////////////////////////
// Pages and rows

struct Page {
    std::wstring prefix;  // "taskbar." (empty: the changes not saved)
    std::wstring title, description;
};

struct Row {
    bool section = false;
    std::wstring key;             // a setting, or a section's group
    std::wstring title, detail;   // a section's
    Kind kind = Kind::Text;
    // Laid out:
    float y = 0, h = 0;
    ComPtr<IDWriteTextLayout> name, description;
    bool first = false, last = false;  // in its card
};

std::vector<Page> g_pages;
int g_page = 0;
std::wstring g_query;
std::vector<Row> g_rows;

void MakePages() {
    g_pages.clear();
    g_pages.push_back({L"presets.", L"Presets", L""});  // (no settings of its own: the presets' grid)
    g_pages.push_back({L"extensions.", L"Extensions", L""});  // (the same: its plugins' cards)
    g_pages.push_back({L"plugins.", L"Plugins", L"The settings of the plugins you installed. A change restarts them."});
    for (int i = 0; i < kModuleCount; i++) {  // (a part beside the suite is set in Windhawk: no page here)
        if (kModules[i].besideSuite) continue;
        g_pages.push_back({std::wstring(kModules[i].prefix) + L".", kModules[i].title, L""});
        // The switcher's views and its window tiling have pages of their own, after the switcher's.
        if (std::wstring(kModules[i].prefix) == L"switcher")
            g_pages.push_back({L"switcher.view.", L"Views",
                               L"How Alt+Tab (and Win+Tab) shows your windows. Pick a view: each flies your windows in from "
                               L"where they are, over a dimmed, blurred copy of the screen, and moves on Motion's switcher line."});
        if (std::wstring(kModules[i].prefix) == L"switcher")
            g_pages.push_back({L"switcher.tiling.", L"Tiling",
                               L"Windows laid out side by side, in the manner of komorebi and Hyprland. Pick a ready-made set "
                               L"(the layout, the gaps, the focus border and how windows move to their places) and change "
                               L"any line after; the set turns tiling on. The Tiling key's shortcuts are under Tiling key."});
    }
    auto described = [](const wchar_t* group, const wchar_t* fallback) {
        const SettingInfo* info = InfoOf(group);
        return Page{std::wstring(group) + L".", info && !info->name.empty() ? info->name : fallback,
                    info ? info->description : L""};
    };
    g_pages.push_back(described(L"theme", L"Tourne theme"));
    g_pages.push_back(described(L"identity", L"Identity"));
    g_pages.push_back(described(L"motion", L"Motion"));
    g_pages.push_back(described(L"glass", L"Glass"));
    g_pages.push_back(described(L"shader", L"Screen shader"));
    g_pages.push_back(described(L"capture", L"Capture"));
    g_pages.push_back(described(L"launcher", L"Launcher"));
    g_pages.push_back(described(L"sounds", L"Sounds"));
    g_pages.push_back(described(L"inject", L"Style apps"));
    g_pages.push_back(described(L"shell", L"Shell mode"));
    g_pages.push_back(described(L"notify", L"Notifications"));
    g_pages.push_back(described(L"killswitch", L"Windows' taskbar off"));
    g_pages.push_back(described(L"keystrokes", L"Keystrokes"));
    g_pages.push_back(described(L"desktop", L"Tourne Desktop"));
}

int PendingPage() { return (int)g_pages.size(); }  // the sidebar's last item, when there are changes

int OrderOf(const std::wstring& key) {
    const SettingInfo* info = InfoOf(key);
    return info && info->order ? info->order : 1 << 30;
}

// A setting's group, as a section title: its own name, else its key's last part.
std::wstring GroupTitle(const std::wstring& group) {
    std::wstring element;
    std::wstring g = group;
    if (size_t b = g.rfind(L'['); b != std::wstring::npos && g.back() == L']') {
        element = L" " + std::to_wstring(_wtoi(g.c_str() + b + 1) + 1);
        g = g.substr(0, b);
    }
    const SettingInfo* info = InfoOf(g);
    std::wstring name = info && !info->name.empty() ? info->name : LastPart(g);
    return name + element;
}

std::wstring NameOf(const std::wstring& key) {
    if (key == kMenuTreeKey) return L"Menu items";
    if (key == kBindsKey) return L"Binds, window rules and idle actions";
    const SettingInfo* info = InfoOf(key);
    return info && !info->name.empty() ? info->name : LastPart(key);
}

bool Matches(const std::wstring& key, const std::wstring& query) {
    const SettingInfo* info = InfoOf(key);
    if (Lower(key).find(query) != std::wstring::npos) return true;
    if (!info) return false;
    if (Lower(info->name).find(query) != std::wstring::npos || Lower(info->description).find(query) != std::wstring::npos) return true;
    for (auto const& o : info->options)
        if (Lower(o.second).find(query) != std::wstring::npos) return true;
    return false;
}

// System Icons' scheme has no page: it shows with the switcher's style, which reads it.
constexpr wchar_t kIconsScheme[] = L"systemIcons.scheme";

std::wstring GroupOf(const std::wstring& key) {
    if (key == kMenuTreeKey) return L"taskbar.menu";
    return key == kIconsScheme ? L"switcher.theme" : Parent(key);
}

int PageOf(const std::wstring& key) {
    if (key == kIconsScheme) return PageOf(L"switcher.theme.style");
    // The longest page that holds it (switcher.tiling. before switcher.).
    int best = -1;
    for (size_t i = 0; i < g_pages.size(); i++)
        if (key.rfind(g_pages[i].prefix, 0) == 0 && (best < 0 || g_pages[i].prefix.size() > g_pages[best].prefix.size())) best = (int)i;
    return best;
}

void BuildTiles();
bool PresetSearchFoundNothing();  // (and the search goes to the settings then)
bool PresetsPage();
bool ExtensionsPage();
bool MotionPage();
bool ViewsPage();  // (Alt+Tab's views as cards, then their settings)
constexpr int kViewCount = 7;
constexpr float kViewH = 168, kViewMinW = 210;
float g_viewsTop = 0, g_viewW = 220;
int g_viewColumns = 1;
D2D1_RECT_F ViewRect(int n);

void BuildRows() {
    g_rows.clear();
    if (g_page == PendingPage() && g_model.changes.empty()) g_page = 0;
    if (PresetsPage() || ExtensionsPage()) {
        BuildTiles();
        // A search begun on the presets that no preset matches searches the settings instead (it used to say
        // "0 presets match" while settings did).
        if (!PresetSearchFoundNothing()) return;
    }
    std::vector<std::wstring> keys;
    std::wstring query = Lower(g_query);
    bool pending = query.empty() && g_page == PendingPage();
    for (auto const& [key, value] : DefaultSettings()) {
        int page = PageOf(key);
        if (page < 0 || (MenuRowKey(key) && key != kMenuTreeKey) || (BindRowKey(key) && key != kBindsKey)) continue;
        if (!query.empty() ? !Matches(key, query) : pending ? !g_model.changes.count(key) : page != g_page) continue;
        keys.push_back(key);
    }
    // The schema's order; a module's own switch first.
    std::stable_sort(keys.begin(), keys.end(), [](const std::wstring& a, const std::wstring& b) {
        int pa = PageOf(a), pb = PageOf(b);
        if (pa != pb) return pa < pb;
        bool ea = LastPart(a) == L"enabled" && Parent(a) + L"." == g_pages[pa].prefix;
        bool eb = LastPart(b) == L"enabled" && Parent(b) + L"." == g_pages[pb].prefix;
        if (ea != eb) return ea;
        return OrderOf(a) < OrderOf(b);
    });
    std::wstring group = L"\x1";
    int page = -1;
    for (auto const& key : keys) {
        int p = PageOf(key);
        std::wstring g = GroupOf(key);
        bool top = g + L"." == g_pages[p].prefix;
        if (p != page || g != group) {
            Row s;
            s.section = true;
            s.key = g;
            if (!query.empty() || pending) {
                s.title = g_pages[p].title;
                if (!top) s.title += L"  \x203A  " + GroupTitle(g);
            } else if (!top) {
                s.title = GroupTitle(g);
                if (const SettingInfo* info = InfoOf(g)) s.detail = info->description;
            }
            g_rows.push_back(s);
            page = p, group = g;
        }
        Row r;
        r.key = key;
        r.kind = KindOf(key);
        g_rows.push_back(r);
    }
}

////////////////////////////////////////////////////////////////////////////////
// The window

constexpr wchar_t kClass[] = L"TourneStudio";
constexpr float kSidebar = 240, kPad = 32, kControl = 248, kRowPad = 14, kItem = 36, kBar = 60, kMaxWidth = 880;
constexpr UINT_PTR kTimerLive = 1, kTimerMotion = 2;
constexpr UINT kMsgPluginsChanged = WM_APP + 1;  // a plugin finished installing (or failed)
constexpr int kEditId = 10, kSearchId = 11;

HWND g_wnd, g_host, g_search, g_edit;
HFONT g_editFont;
HBRUSH g_editBrush;
float g_scale = 1;
float g_scroll = 0, g_contentHeight = 0;
std::wstring g_editing;  // the setting the text box is open on
bool g_editingHex = false;
UINT g_themeChanged;

struct Colours {
    uint32_t ground, raised, hover, line, text, muted, dim, accent, onAccent;
    bool light;
} g_c;

ComPtr<ID2D1Factory> g_d2d;
ComPtr<IDWriteFactory> g_dw;
ComPtr<ID2D1HwndRenderTarget> g_rt;
ComPtr<ID2D1SolidColorBrush> g_brush;
ComPtr<IDWriteTextFormat> g_fTitle, g_fSection, g_fName, g_fBody, g_fSmall, g_fItem, g_fApp;

// Where the pointer is, and what it's on.
enum class Part { None, Item, Control, Reset, Track, ScrollThumb, Undo, Redo, Discard, Save, TestWindow, Compare, Filter, Tile, TileButton, ChipSet, ViewCard, AddExtension, ImportLook, ExportLook };
struct Hit {
    Part part = Part::None;
    int index = -1;  // a row, or a sidebar item
    bool operator==(const Hit&) const = default;
};
Hit g_hover, g_pressed;
float g_dragFrom = 0, g_scrollFrom = 0;

uint32_t Mix(uint32_t a, uint32_t b, float t) {
    auto ch = [&](int s) { return (uint32_t)std::lround(((a >> s) & 255) * (1 - t) + ((b >> s) & 255) * t) << s; };
    return ch(16) | ch(8) | ch(0);
}

double Luma(uint32_t c) { return (0.2126 * ((c >> 16) & 255) + 0.7152 * ((c >> 8) & 255) + 0.0722 * (c & 255)) / 255; }

bool WindowsLight() {
    DWORD v = 0, size = sizeof(v);
    RegGetValueW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", L"AppsUseLightTheme",
                 RRF_RT_REG_DWORD, nullptr, &v, &size);
    return v != 0;
}

// The Studio wears the Tourne theme when there is one.
void LoadColours() {
    auto role = [](const wchar_t* token, uint32_t fallback) {
        for (int i = 0; i < kThemeRoleCount; i++)
            if (!wcscmp(kThemeRoles[i].token, token)) {
                uint32_t c;
                if (ThemeRoleColor(kThemeRoles[i], &c)) return c;
            }
        return fallback;
    };
    uint32_t ground = 0;
    bool themed = ThemeRoleColor(kThemeRoles[0], &ground);
    bool light = themed ? Luma(ground) > 0.5 : WindowsLight();
    if (light) g_c = {0xF3F4F6, 0xFFFFFF, 0xF7F8FA, 0xE1E4E8, 0x1B1F24, 0x57606A, 0x8C959F, 0x2F7D5B, 0xFFFFFF, true};
    else g_c = {0x15181C, 0x1E2227, 0x252A30, 0x2C3138, 0xE8EAED, 0xA4ABB5, 0x6E7681, 0x64AA89, 0x0B0F0D, false};
    if (themed) {
        g_c.ground = ground;
        g_c.raised = role(L"@raised", g_c.raised);
        g_c.line = role(L"@line", g_c.line);
        g_c.text = role(L"@text", g_c.text);
        g_c.muted = role(L"@muted", g_c.muted);
        g_c.dim = role(L"@dim", g_c.dim);
        g_c.accent = role(L"@accent", g_c.accent);
        g_c.hover = Mix(g_c.raised, g_c.text, 0.05f);
        g_c.onAccent = Luma(g_c.accent) > 0.55 ? 0x0B0F0D : 0xFFFFFF;
    }
    if (g_editBrush) DeleteObject(g_editBrush);
    g_editBrush = CreateSolidBrush(RGB(g_c.raised >> 16, (g_c.raised >> 8) & 255, g_c.raised & 255));
    if (g_wnd) {
        RedrawWindow(g_wnd, nullptr, nullptr, RDW_INVALIDATE | RDW_ALLCHILDREN);
        BOOL dark = !g_c.light;
        DwmSetWindowAttribute(g_wnd, 20 /* DWMWA_USE_IMMERSIVE_DARK_MODE */, &dark, sizeof(dark));
        COLORREF caption = RGB(g_c.ground >> 16, (g_c.ground >> 8) & 255, g_c.ground & 255);
        COLORREF text = RGB(g_c.text >> 16, (g_c.text >> 8) & 255, g_c.text & 255);
        DwmSetWindowAttribute(g_wnd, 35 /* DWMWA_CAPTION_COLOR */, &caption, sizeof(caption));
        DwmSetWindowAttribute(g_wnd, 36 /* DWMWA_TEXT_COLOR */, &text, sizeof(text));
    }
}

// Windows' own menus dark when its apps are (uxtheme's unnamed export 135,
// as Notepad++ and others use it; left alone if it isn't there).
void MenusFollowWindows() {
    static bool done;
    if (done) return;
    done = true;
    if (HMODULE ux = LoadLibraryExW(L"uxtheme.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32)) {
        using SetPreferredAppMode = int(WINAPI*)(int);
        if (auto set = (SetPreferredAppMode)(void*)GetProcAddress(ux, MAKEINTRESOURCEA(135))) set(1 /* AllowDark */);
    }
}

D2D1_COLOR_F Color(uint32_t c, float a = 1) {
    return D2D1::ColorF(((c >> 16) & 255) / 255.f, ((c >> 8) & 255) / 255.f, (c & 255) / 255.f, a);
}

ID2D1SolidColorBrush* Brush(uint32_t c, float a = 1) {
    g_brush->SetColor(Color(c, a));
    return g_brush.Get();
}

ComPtr<IDWriteTextFormat> Format(float size, DWRITE_FONT_WEIGHT weight, bool wrap = false) {
    ComPtr<IDWriteTextFormat> f;
    g_dw->CreateTextFormat(L"Segoe UI", nullptr, weight, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, size, L"", &f);
    if (!f) return f;
    f->SetWordWrapping(wrap ? DWRITE_WORD_WRAPPING_WRAP : DWRITE_WORD_WRAPPING_NO_WRAP);
    f->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
    if (!wrap) {
        DWRITE_TRIMMING trim{DWRITE_TRIMMING_GRANULARITY_CHARACTER, 0, 0};
        ComPtr<IDWriteInlineObject> dots;
        g_dw->CreateEllipsisTrimmingSign(f.Get(), &dots);
        f->SetTrimming(&trim, dots.Get());
    }
    return f;
}

void MakeFormats() {
    g_fApp = Format(17, DWRITE_FONT_WEIGHT_SEMI_BOLD);
    g_fTitle = Format(26, DWRITE_FONT_WEIGHT_SEMI_BOLD);
    g_fSection = Format(15, DWRITE_FONT_WEIGHT_SEMI_BOLD);
    g_fName = Format(14, DWRITE_FONT_WEIGHT_NORMAL, true);
    g_fBody = Format(12.5f, DWRITE_FONT_WEIGHT_NORMAL, true);
    g_fSmall = Format(11.5f, DWRITE_FONT_WEIGHT_SEMI_BOLD);
    g_fItem = Format(14, DWRITE_FONT_WEIGHT_NORMAL);
}

ComPtr<IDWriteTextLayout> Layout(const std::wstring& text, IDWriteTextFormat* f, float width) {
    ComPtr<IDWriteTextLayout> l;
    g_dw->CreateTextLayout(text.c_str(), (UINT32)text.size(), f, std::max(width, 1.f), 10000, &l);
    return l;
}

float TextHeight(IDWriteTextLayout* l) {
    DWRITE_TEXT_METRICS m{};
    if (l) l->GetMetrics(&m);
    return m.height;
}

float TextWidth(IDWriteTextLayout* l) {
    DWRITE_TEXT_METRICS m{};
    if (l) l->GetMetrics(&m);
    return m.widthIncludingTrailingWhitespace;
}

D2D1_SIZE_F ClientSize() {
    RECT rc;
    GetClientRect(g_wnd, &rc);
    return {rc.right / g_scale, rc.bottom / g_scale};
}

bool Pending() { return !g_model.changes.empty(); }

// The content column: left edge and width.
float ContentLeft() { return kSidebar + kPad; }
float ContentWidth() { return std::min(ClientSize().width - kSidebar - 2 * kPad, kMaxWidth); }
float ViewBottom() { return ClientSize().height - (Pending() ? kBar : 0); }
float g_header = 104;  // the page's title and what it's about
float HeaderHeight() { return g_header; }

////////////////////////////////////////////////////////////////////////////////
// Presets: a grid of the built-in palettes and looks. Choosing one makes it
// the Tourne theme and sets the pieces to follow it (one step to undo, shown
// at once, kept when you save).

bool g_queryPresets = false;  // the search box filters the presets (it was started on their page)
int g_filter = 0;             // (each tile page starts on its first: set back when the page changes)
const wchar_t* const kPresetFilters[] = {L"All", L"Identities", L"Picks", L"Dark", L"Light", L"Full looks"};
const wchar_t* const kPluginFilters[] = {L"All", L"Installed", L"Bar", L"Panel", L"Desktop", L"Launcher"};
constexpr int kFilterCount = 6;   // (both have six)
std::vector<int> g_tiles;     // the presets or plugins shown, by index

bool PresetSearchFoundNothing() {
    if (!g_queryPresets || g_query.empty() || !g_tiles.empty()) return false;
    g_queryPresets = false;
    return true;
}
float g_tilesTop = 0, g_tileW = 220;
int g_columns = 1;
constexpr float kTileH = 104, kPluginTileH = 168, kTileGap = 12, kFilterH = 30;

bool PresetsPage() {
    return g_page < (int)g_pages.size() && g_pages[g_page].prefix == L"presets." && (g_query.empty() || g_queryPresets);
}

// Extensions: for now its one section, Noctalia's plugins that run on Windows.
bool ExtensionsPage() {
    return g_page < (int)g_pages.size() && g_pages[g_page].prefix == L"extensions." && g_query.empty();
}

bool TilesPage() { return PresetsPage() || ExtensionsPage(); }
const wchar_t* FilterName(int i) { return ExtensionsPage() ? kPluginFilters[i] : kPresetFilters[i]; }
float TileH() { return ExtensionsPage() ? kPluginTileH : kTileH; }

// An extension (extensions.h) is on when every setting it sets is as it says.
bool ExtensionOn(const Extension& e) {
    for (auto const& [key, value] : e.settings)
        if (!Same(key, ValueOf(key), value)) return false;
    return true;
}

void BuildPluginTiles() {
    g_tiles.clear();
    // Tourne's extensions first (as -1 - their place), under All, and under Installed when on.
    auto const& looks = ExtensionList();
    for (int i = 0; i < (int)looks.size(); i++)
        if (g_filter == 0 || (g_filter == 1 && ExtensionOn(looks[i]))) g_tiles.push_back(-1 - i);
    auto const& all = PluginCatalogue();
    for (int i = 0; i < (int)all.size(); i++) {
        PluginInfo const& p = all[i];
        auto shows = [&](const wchar_t* entry) { return std::find(p.entries.begin(), p.entries.end(), entry) != p.entries.end(); };
        bool keep = g_filter == 1   ? PluginStateOf(p) == PluginState::Installed
                    : g_filter == 2 ? shows(L"widget")
                    : g_filter == 3 ? shows(L"panel")
                    : g_filter == 4 ? shows(L"desktop_widget")
                    : g_filter == 5 ? shows(L"launcher_provider")
                                    : true;
        if (keep) g_tiles.push_back(i);
    }
}

bool PresetMatches(const Preset& p, const std::wstring& query) {
    if (query.empty()) return true;
    for (const wchar_t* text : {p.name, p.family, p.source, p.id})
        if (Lower(text).find(query) != std::wstring::npos) return true;
    return false;
}

void BuildTiles() {
    if (ExtensionsPage()) return BuildPluginTiles();
    g_tiles.clear();
    std::wstring query = Lower(g_query);
    auto const& all = Presets();
    for (int i = 0; i < (int)all.size(); i++) {
        Preset const& p = all[i];
        bool keep = g_filter == 1   ? p.family == std::wstring(L"Identity")
                    : g_filter == 2 ? p.pick > 0
                    : g_filter == 3 ? !p.light
                    : g_filter == 4 ? p.light
                    : g_filter == 5 ? *p.settings != 0
                                    : true;
        if (keep && PresetMatches(p, query)) g_tiles.push_back(i);
    }
}

bool PresetChosen(const Preset& p) {
    if (ValueOf(L"theme.preset") != p.id) return false;
    for (auto const& [key, value] : PresetChoice(p))
        if (key == L"theme.source") return ValueOf(key) == value;
    return false;
}

// Every setting some preset's look sets: a preset without it takes back the
// last one's (so trying presets one after another doesn't pile them up).
const std::vector<std::wstring>& LookKeys() {
    static std::vector<std::wstring> keys = [] {
        std::vector<std::wstring> out;
        for (auto const& p : Presets())
            for (auto const& kv : p.Settings())
                if (std::find(out.begin(), out.end(), kv.first) == out.end()) out.push_back(kv.first);
        return out;
    }();
    return keys;
}

void ChoosePreset(const Preset& p) {
    std::vector<std::pair<std::wstring, std::optional<Change>>> changes;
    auto choice = PresetChoice(p);
    for (auto const& [key, value] : choice)
        changes.push_back({key, Same(key, value, SavedValue(key)) ? std::nullopt : std::optional<Change>(Change{false, value})});
    for (auto const& key : LookKeys()) {
        bool set = false;
        for (auto const& kv : choice) set = set || kv.first == key;
        if (!set && g_model.changes.count(key)) changes.push_back({key, std::nullopt});
    }
    RecordMany(changes);
}

std::wstring PageTitle() {
    if (PresetsPage()) return L"Presets";
    if (ExtensionsPage()) return L"Extensions";
    if (!g_query.empty()) return L"Search";
    if (g_page == PendingPage()) return L"Changes not saved";
    return g_page < (int)g_pages.size() ? g_pages[g_page].title : L"";
}

std::wstring PageDescription() {
    if (ExtensionsPage())
        return L"Looks first: Tourne's shaders, motion and views as data, turned on like a preset (Add an extension takes a folder's "
               L"extension.json; an extension can only change how things look). Then plugins: " + std::to_wstring(PluginCatalogue().size()) +
               L" plugins from Noctalia's catalogues and Tourne's own that run on Windows. Each card says where it shows and "
               L"what it needs; you're asked before it installs, and it's allowed only that. " + std::to_wstring(PluginsLeftOut()) +
               L" more need Linux programs or paths and are left out. Installed plugins start right away; their settings "
               L"are under Plugins.";
    if (PresetsPage()) {
        if (!g_query.empty()) return std::to_wstring(g_tiles.size()) + L" presets match “" + g_query + L"”.";
        return std::to_wstring(Presets().size()) +
               L" palettes and looks from the ricing world (base16, Catppuccin, Tokyo Night, Omarchy, HyDE, YASB and more). "
               L"Choose one and it becomes your Tourne theme: the taskbar, the switcher and every colour set to a theme role "
               L"take it at once. Identities set the whole look (shape, material and motion too); full looks also set gaps, borders and corners. "
               L"Save keeps it.";
    }
    if (!g_query.empty()) return g_rows.empty() ? L"Nothing matches “" + g_query + L"”." : L"Settings matching “" + g_query + L"”.";
    if (g_page == PendingPage())
        return L"Showing already, kept when you save. Save writes them to tourne-gui.conf; your tourne.conf is never written.";
    return g_page < (int)g_pages.size() ? g_pages[g_page].description : L"";
}

// The Motion page: a curve drawn large for the line you point at, with a dot
// running along it and a block moved by it, and each line's curve small.
constexpr float kCurveH = 196;
float g_curveTop = 0;
std::wstring g_motionLeaf = L"global";
ULONGLONG g_motionStart = GetTickCount64();
bool g_motionTimer = false;

bool IsMotionLeaf(const std::wstring& key) { return key.rfind(L"motion.", 0) == 0 && key != L"motion.enabled"; }

// A leaf as the Studio shows it now (changes not saved included).
motion::Anim MotionOf(const std::wstring& leaf) {
    return motion::Resolve(leaf, [](const std::wstring& key) { return ValueOf(key); });
}

// Where the preview is in its loop: 0 to 1 over the animation, then a rest.
float MotionPhase(const motion::Anim& a) {
    double ms = std::max(120.0, a.set && a.on ? motion::Ms(a) : 800.0), cycle = ms + 700;
    double at = std::fmod((double)(GetTickCount64() - g_motionStart), cycle);
    return (float)std::min(1.0, at / ms);
}

// Ready-made sets of chips at the top of a page (Motion's under the curve,
// Tiling's under the page's words). Each set writes its lines; a line another
// set of the page wrote and this one doesn't goes back to empty, so trying
// sets one after another doesn't pile them up.
struct ChipSet {
    std::wstring name;
    std::vector<std::pair<std::wstring, std::wstring>> lines;  // (whole keys)
};

std::vector<ChipSet> WithPrefix(const wchar_t* prefix,
                                std::vector<std::pair<const wchar_t*, std::vector<std::pair<const wchar_t*, const wchar_t*>>>> sets) {
    std::vector<ChipSet> out;
    for (auto const& [name, lines] : sets) {
        ChipSet set{name};
        for (auto const& [leaf, line] : lines) set.lines.push_back({std::wstring(prefix) + leaf, line});
        out.push_back(set);
    }
    return out;
}

// Motion's: each writes global, bar, border and borderangle (the lines an
// identity sets, so the set wins over them) and whatever else it shapes.
const std::vector<ChipSet>& MotionSets() {
    static const std::vector<ChipSet> sets = WithPrefix(L"motion.", {
        {L"Off", {{L"enabled", L"false"}}},
        {L"Subtle",
         {{L"enabled", L"true"}, {L"global", L"1, 2.5, easeOutCubic"}, {L"bar", L"1, 2.5, easeOutCubic, fade"},
          {L"switcher", L"1, 2, easeOutCubic"}, {L"fade", L"1, 2, easeOutSine"}, {L"border", L"1, 3, easeOutCubic"},
          {L"borderangle", L"1, 30, linear, loop"}}},
        {L"Smooth",
         {{L"enabled", L"true"}, {L"global", L"1, 4, easeInOutQuint"}, {L"bar", L"1, 3.5, easeInOutQuint, slide"},
          {L"switcher", L"1, 3, easeOutQuint"}, {L"popups", L"1, 3, easeOutQuint, popin 90%"},
          {L"border", L"1, 6, easeInOutCubic"}, {L"borderangle", L"1, 25, linear, loop"}}},
        {L"Snappy",
         {{L"enabled", L"true"}, {L"global", L"1, 1.5, easeOutQuint"}, {L"bar", L"1, 1.2, easeOutExpo, slide"},
          {L"switcher", L"1, 1.2, easeOutExpo"}, {L"border", L"1, 1.5, easeOutQuint"}, {L"borderangle", L"1, 8, linear, loop"}}},
        {L"Lively",
         {{L"enabled", L"true"}, {L"global", L"1, 5, overshot"}, {L"bar", L"1, 3.2, overshot, slidefade"},
          {L"switcher", L"1, 4, easeOutBack"}, {L"popups", L"1, 3, easeOutBack, popin 80%"}, {L"border", L"1, 8, overshot"},
          {L"borderangle", L"1, 16, linear, loop"}}},
        {L"Springy",
         {{L"enabled", L"true"}, {L"global", L"1, 4, gentle"}, {L"bar", L"1, 4, bouncy, slide"}, {L"switcher", L"1, 4, snappy"},
          {L"popups", L"1, 4, rubber, popin 85%"}, {L"border", L"1, 4, gentle"}, {L"borderangle", L"1, 16, linear, loop"}}},
        // komorebi's animation block (200 ms, EaseOutBack, on every move); its borders don't turn.
        {L"komorebi",
         {{L"enabled", L"true"}, {L"global", L"1, 2, easeOutBack"}, {L"windowsMove", L"1, 2, easeOutBack"},
          {L"bar", L"1, 2, easeOutBack, slide"}, {L"border", L"1, 2, easeOutBack"}, {L"borderangle", L"0"}}},
        // Hyprland's own, from the hyprland.conf it ships.
        {L"Hyprland",
         {{L"enabled", L"true"}, {L"bezier.almostLinear", L"0.5, 0.5, 0.75, 1"}, {L"bezier.quick", L"0.15, 0, 0.1, 1"},
          {L"global", L"1, 10, default"}, {L"border", L"1, 5.39, easeOutQuint"}, {L"windows", L"1, 4.79, easeOutQuint"},
          {L"windowsIn", L"1, 4.1, easeOutQuint, popin 87%"}, {L"windowsOut", L"1, 1.49, linear, popin 87%"},
          {L"fadeIn", L"1, 1.73, almostLinear"}, {L"fadeOut", L"1, 1.46, almostLinear"}, {L"fade", L"1, 3.03, quick"},
          {L"layers", L"1, 3.81, easeOutQuint"}, {L"layersIn", L"1, 4, easeOutQuint, fade"},
          {L"layersOut", L"1, 1.5, linear, fade"}, {L"fadeLayersIn", L"1, 1.79, almostLinear"},
          {L"fadeLayersOut", L"1, 1.39, almostLinear"}, {L"bar", L"1, 3.81, easeOutQuint, slide"},
          {L"workspaces", L"1, 1.94, almostLinear, fade"}, {L"workspacesIn", L"1, 1.21, almostLinear, fade"},
          {L"workspacesOut", L"1, 1.94, almostLinear, fade"}, {L"borderangle", L"1, 30, linear, loop"}}},
    });
    return sets;
}

// Tiling's: how windows are laid out, the gaps, the focus border and how they
// move to their places, after the tilers people know. Picking one turns
// automatic tiling on (it's in every set).
const std::vector<ChipSet>& TilingSets() {
    static const std::vector<ChipSet> sets = [] {
        auto tiling = [](const wchar_t* name, const wchar_t* layout, const wchar_t* rules, const wchar_t* ratio, const wchar_t* inner,
                         const wchar_t* outer, const wchar_t* border, const wchar_t* move) {
            ChipSet set{name};
            auto line = [&](const wchar_t* key, const wchar_t* value) { set.lines.push_back({key, value}); };
            line(L"switcher.tiling.auto", L"true");
            line(L"switcher.tiling.layout", layout);
            line(L"switcher.tiling.layoutRules", rules);
            line(L"switcher.tiling.mainRatio", ratio);
            line(L"switcher.tiling.gapInner", inner);
            for (auto side : {L"switcher.tiling.gapTop", L"switcher.tiling.gapBottom", L"switcher.tiling.gapLeft", L"switcher.tiling.gapRight"})
                line(side, outer);
            line(L"switcher.tiling.focusBorder", border);
            line(L"motion.windowsMove", move);
            if (std::wstring(move) != L"0") line(L"motion.enabled", L"true");  // (moves need motion on)
            return set;
        };
        return std::vector<ChipSet>{
            // komorebi as Tourne set it up: BSP, Grid from four windows, 8 px workspace and
            // container padding (8 + 8 at the edges, 8 + 8 between two windows), no
            // border, every move 200 ms EaseOutBack.
            tiling(L"komorebi", L"bsp", L"1: bsp, 4: grid", L"50", L"16", L"16", L"none", L"1, 2, easeOutBack"),
            // Hyprland's dwindle with its own gaps (gaps_in 5 a side, gaps_out 20) and the
            // windows line of the hyprland.conf it ships.
            tiling(L"Hyprland", L"bsp", L"", L"50", L"10", L"20", L"tiled", L"1, 4.79, easeOutQuint"),
            // GlazeWM: side by side, 20 px gaps, its focus border, no animation.
            tiling(L"GlazeWM", L"columns", L"", L"50", L"20", L"20", L"tiled", L"0"),
            // dwm: a main window on the left at 55%, the rest stacked, no gaps, no animation.
            tiling(L"dwm", L"verticalStack", L"", L"55", L"0", L"0", L"tiled", L"0"),
            // i3: side by side, no gaps, no animation.
            tiling(L"i3", L"columns", L"", L"50", L"0", L"0", L"tiled", L"0"),
            // One window at a time, each filling the screen.
            tiling(L"Monocle", L"monocle", L"", L"50", L"0", L"0", L"none", L"1, 2.5, easeOutCubic"),
        };
    }();
    return sets;
}

bool TilingPage();

// The sets the page shows (none on most pages).
const std::vector<ChipSet>* PageSets() {
    if (MotionPage()) return &MotionSets();
    if (TilingPage()) return &TilingSets();
    return nullptr;
}

// Every line some set of these writes.
std::vector<std::wstring> SetKeys(const std::vector<ChipSet>& sets) {
    std::vector<std::wstring> out;
    for (auto const& set : sets)
        for (auto const& [key, line] : set.lines)
            if (std::find(out.begin(), out.end(), key) == out.end()) out.push_back(key);
    return out;
}

// What this set wants on a line ("" for none).
std::wstring SetWants(const ChipSet& set, const std::wstring& key) {
    for (auto const& [k, line] : set.lines)
        if (k == key) return line;
    return L"";
}

// A switch (motion.enabled, tiling.auto) a set leaves alone isn't one it asks to be off.
bool SetLeavesAlone(const ChipSet& set, const std::wstring& key) {
    return (key == L"motion.enabled" || key == L"switcher.tiling.auto") && SetWants(set, key).empty();
}

bool SetChosen(const std::vector<ChipSet>& sets, const ChipSet& set) {
    for (auto const& key : SetKeys(sets)) {
        if (SetLeavesAlone(set, key)) continue;
        if (!Same(key, ValueOf(key), SetWants(set, key))) return false;
    }
    return true;
}

void ChooseSet(const std::vector<ChipSet>& sets, const ChipSet& set) {
    std::vector<std::pair<std::wstring, std::optional<Change>>> changes;
    for (auto const& key : SetKeys(sets)) {
        if (SetLeavesAlone(set, key)) continue;
        std::wstring want = SetWants(set, key);
        changes.push_back({key, Same(key, want, SavedValue(key)) ? std::nullopt : std::optional<Change>(Change{false, want})});
    }
    RecordMany(changes);
}

// The sets' chips, in a row (wrapping when the window is narrow).
float g_setsTop = 0;

D2D1_RECT_F SetRect(int i) {
    float left = ContentLeft(), right = left + ContentWidth(), x = left, top = g_setsTop - g_scroll;
    for (int j = 0;; j++) {
        float w = 28 + 8 * (float)(*PageSets())[j].name.size();
        if (x > left && x + w > right) x = left, top += kFilterH + 8;
        if (j == i) return {x, top, x + w, top + kFilterH};
        x += w + 8;
    }
}

void LayoutRows() {
    float width = ContentWidth();
    std::wstring about = PageDescription();
    g_header = 72 + (about.empty() ? 0 : TextHeight(Layout(about, g_fBody.Get(), width).Get())) + 16;
    float y = g_header;
    if (MotionPage()) {
        g_curveTop = y;
        y += kCurveH + 16;
    }
    if (auto sets = PageSets()) {
        g_setsTop = y;
        y = SetRect((int)sets->size() - 1).bottom + g_scroll + 16;
    }
    if (ViewsPage()) {
        g_viewsTop = y;
        g_viewColumns = std::max(1, (int)((width + kTileGap) / (kViewMinW + kTileGap)));
        g_viewW = (width - (g_viewColumns - 1) * kTileGap) / g_viewColumns;
        y += ((kViewCount + g_viewColumns - 1) / g_viewColumns) * (kViewH + kTileGap) + 8;
    }
    if (TilesPage()) {
        g_tilesTop = y + kFilterH + 16;
        float least = ExtensionsPage() ? 300 : 200;  // (a plugin's card has words to fit)
        g_columns = std::max(1, (int)((width + kTileGap) / (least + kTileGap)));
        g_tileW = (width - (g_columns - 1) * kTileGap) / g_columns;
        int rows = ((int)g_tiles.size() + g_columns - 1) / g_columns;
        g_contentHeight = g_tilesTop + rows * (TileH() + kTileGap) + 32;
        return;
    }
    float textWidth = width - 2 * kRowPad - kControl - 24;
    for (size_t i = 0; i < g_rows.size(); i++) {
        Row& r = g_rows[i];
        if (r.section) {
            if (r.title.empty()) {  // (the page's own settings: no title)
                r.y = y, r.h = 0;
                continue;
            }
            y += 18;
            r.name = Layout(r.title, g_fSection.Get(), width);
            r.description = r.detail.empty() ? nullptr : Layout(r.detail, g_fBody.Get(), width);
            r.y = y;
            r.h = TextHeight(r.name.Get()) + (r.description ? 4 + TextHeight(r.description.Get()) : 0) + 10;
            y += r.h;
            continue;
        }
        const SettingInfo* info = InfoOf(r.key);
        float tw = r.kind == Kind::Switch ? textWidth : textWidth - 64;  // (room for Reset left of the control)
        if (IsMotionLeaf(r.key)) tw -= 60;                               // (and for its little curve)
        r.name = Layout(NameOf(r.key), g_fName.Get(), tw - 96);
        std::wstring d = info ? info->description : L"";
        if (r.key == kMenuTreeKey)
            d = L"The menu and its submenus as a tree, in a window laid out like LiteMenu's own: add, remove, move and drag "
                L"items, and set each one's name, command and parameters. With none yet, it opens on LiteMenu's menu.";
        if (r.key == kBindsKey)
            d = L"Your keys, window rules and idle actions as lists with plain choices: pick the keys (or press them), what they "
                L"do and with what, which windows float or open where, and what happens when you're away. No config file needed; "
                L"the lines are written as Hyprland's, and an As text page shows them all.";
        r.description = d.empty() ? nullptr : Layout(d, g_fBody.Get(), tw);
        float h = TextHeight(r.name.Get()) + (r.description ? 3 + TextHeight(r.description.Get()) : 0);
        r.h = std::max(h + 2 * kRowPad, 60.f);
        r.y = y;
        r.first = i == 0 || g_rows[i - 1].section;
        r.last = i + 1 == g_rows.size() || g_rows[i + 1].section;
        y += r.h + (r.last ? 0 : 1);
    }
    g_contentHeight = y + 32;
}

float MaxScroll() { return std::max(0.f, g_contentHeight - ViewBottom()); }

void ScrollTo(float y) { g_scroll = std::clamp(y, 0.f, MaxScroll()); }

// A row's control, in window units (DIPs).
D2D1_RECT_F ControlRect(const Row& r) {
    float right = ContentLeft() + ContentWidth() - kRowPad, mid = r.y - g_scroll + r.h / 2;
    switch (r.kind) {
        case Kind::Switch: return {right - 44, mid - 12, right, mid + 12};
        default: return {right - kControl, mid - 16, right, mid + 16};
    }
}

D2D1_RECT_F TrackRect(const Row& r) {  // a slider's track (the box is its value)
    D2D1_RECT_F c = ControlRect(r);
    return {c.left + 6, c.top, c.right - 72, c.bottom};
}

D2D1_RECT_F ValueBox(const Row& r) {
    D2D1_RECT_F c = ControlRect(r);
    if (r.kind == Kind::Slider) return {c.right - 60, c.top, c.right, c.bottom};
    if (r.kind == Kind::Colour) return {c.left + 40, c.top, c.right, c.bottom};
    return c;
}

bool CanReset(const Row& r) { return r.key != kMenuTreeKey && r.key != kBindsKey && (g_model.changes.count(r.key) || Find(g_model.gui, r.key)); }

D2D1_RECT_F ResetRect(const Row& r) {
    D2D1_RECT_F c = ControlRect(r);
    float left = r.kind == Kind::Switch ? c.left - 64 : c.left - 64;
    return {left, c.top + (c.bottom - c.top) / 2 - 12, left + 52, c.top + (c.bottom - c.top) / 2 + 12};
}

D2D1_RECT_F ItemRect(int i) { return {12, 104 + i * (kItem + 2), kSidebar - 12, 104 + i * (kItem + 2) + kItem}; }

D2D1_RECT_F BarButton(Part p) {
    D2D1_SIZE_F s = ClientSize();
    float right = s.width - 24, top = s.height - kBar + 14, bottom = s.height - 14;
    float save = 84, discard = 84, step = 72;
    switch (p) {
        case Part::Save: return {right - save, top, right, bottom};
        case Part::Discard: return {right - save - 8 - discard, top, right - save - 8, bottom};
        case Part::Redo: return {right - save - discard - 24 - step, top, right - save - discard - 24, bottom};
        case Part::Undo: return {right - save - discard - 32 - 2 * step, top, right - save - discard - 32 - step, bottom};
        default: return {};
    }
}

D2D1_RECT_F ScrollThumb() {
    float view = ViewBottom();
    if (g_contentHeight <= view) return {};
    float x = ClientSize().width - 10, h = std::max(32.f, view * view / g_contentHeight);
    float y = (view - h) * (g_scroll / MaxScroll());
    return {x, y + 4, x + 6, y + h - 4};
}

bool In(const D2D1_RECT_F& r, float x, float y) { return x >= r.left && x < r.right && y >= r.top && y < r.bottom; }

// The Glass page's own buttons, beside its title: the Tuner's test window
// and "hold to compare".
bool GlassPage() { return g_query.empty() && g_page < (int)g_pages.size() && g_pages[g_page].prefix == L"glass."; }
bool MotionPage() { return g_query.empty() && g_page < (int)g_pages.size() && g_pages[g_page].prefix == L"motion."; }
bool TilingPage() { return g_query.empty() && g_page < (int)g_pages.size() && g_pages[g_page].prefix == L"switcher.tiling."; }
bool ViewsPage() { return g_query.empty() && g_page < (int)g_pages.size() && g_pages[g_page].prefix == L"switcher.view."; }

D2D1_RECT_F HeaderButton(Part p) {
    float right = ContentLeft() + ContentWidth(), top = 30 - g_scroll;
    if (p == Part::Compare) return {right - 150, top, right, top + 34};
    return {right - 150 - 10 - 170, top, right - 160, top + 34};
}

D2D1_RECT_F FilterRect(int i) {
    float x = ContentLeft(), top = g_header - g_scroll;
    for (int j = 0; j < i; j++) x += 20 + 8 * (float)wcslen(FilterName(j)) + 8;
    return {x, top, x + 20 + 8 * (float)wcslen(FilterName(i)), top + kFilterH};
}

D2D1_RECT_F TileRect(int n) {
    int row = n / g_columns, column = n % g_columns;
    float x = ContentLeft() + column * (g_tileW + kTileGap), y = g_tilesTop + row * (TileH() + kTileGap) - g_scroll;
    return {x, y, x + g_tileW, y + TileH()};
}

// A plugin card's button (Install, Remove...), top right.
D2D1_RECT_F TileButtonRect(int n) {
    D2D1_RECT_F r = TileRect(n);
    return {r.right - 14 - 96, r.top + 14, r.right - 14, r.top + 14 + 30};
}

int SidebarCount() { return (int)g_pages.size() + (Pending() ? 1 : 0); }

Hit HitTest(float x, float y) {
    if (x < kSidebar) {
        for (int i = 0; i < SidebarCount(); i++)
            if (In(ItemRect(i), x, y)) return {Part::Item, i};
        return {};
    }
    if (Pending() && y >= ViewBottom()) {
        for (Part p : {Part::Undo, Part::Redo, Part::Discard, Part::Save})
            if (In(BarButton(p), x, y)) return {p};
        return {};
    }
    if (GlassPage())
        for (Part p : {Part::TestWindow, Part::Compare})
            if (In(HeaderButton(p), x, y)) return {p};
    if (ExtensionsPage() && In(HeaderButton(Part::Compare), x, y)) return {Part::AddExtension};
    if (PresetsPage() && In(HeaderButton(Part::Compare), x, y)) return {Part::ImportLook};
    if (PresetsPage() && In(HeaderButton(Part::TestWindow), x, y)) return {Part::ExportLook};
    D2D1_RECT_F thumb = ScrollThumb();
    if (thumb.right > 0 && x >= thumb.left - 4 && y >= thumb.top - 4 && y < thumb.bottom + 4) return {Part::ScrollThumb};
    if (auto sets = PageSets())
        for (int i = 0; i < (int)sets->size(); i++)
            if (In(SetRect(i), x, y)) return {Part::ChipSet, i};
    if (ViewsPage())
        for (int i = 0; i < kViewCount; i++)
            if (In(ViewRect(i), x, y)) return {Part::ViewCard, i};
    if (TilesPage()) {
        for (int i = 0; i < kFilterCount; i++)
            if (In(FilterRect(i), x, y)) return {Part::Filter, i};
        float top = g_tilesTop - g_scroll;
        if (y < top || x < ContentLeft()) return {};
        int column = (int)((x - ContentLeft()) / (g_tileW + kTileGap)), row = (int)((y - top) / (TileH() + kTileGap));
        int n = row * g_columns + column;
        if (column < g_columns && n < (int)g_tiles.size() && In(TileRect(n), x, y)) {
            if (ExtensionsPage()) return In(TileButtonRect(n), x, y) ? Hit{Part::TileButton, n} : Hit{};  // (only its button acts)
            return {Part::Tile, n};
        }
        return {};
    }
    for (size_t i = 0; i < g_rows.size(); i++) {
        Row const& r = g_rows[i];
        if (r.section || y < r.y - g_scroll || y >= r.y - g_scroll + r.h) continue;
        if (CanReset(r) && In(ResetRect(r), x, y)) return {Part::Reset, (int)i};
        if (r.kind == Kind::Slider && In(TrackRect(r), x, y)) return {Part::Track, (int)i};
        D2D1_RECT_F c = ControlRect(r);
        if (In(c, x, y)) return {Part::Control, (int)i};
        // A switch's whole row turns it.
        if (r.kind == Kind::Switch && x >= ContentLeft() && x < c.right) return {Part::Control, (int)i};
        return {};
    }
    return {};
}

////////////////////////////////////////////////////////////////////////////////
// Drawing

void RoundRect(const D2D1_RECT_F& r, float radius, uint32_t c, float a = 1) {
    g_rt->FillRoundedRectangle(D2D1::RoundedRect(r, radius, radius), Brush(c, a));
}

void Text(const std::wstring& s, IDWriteTextFormat* f, const D2D1_RECT_F& r, uint32_t c, float a = 1) {
    g_rt->DrawText(s.c_str(), (UINT32)s.size(), f, r, Brush(c, a), D2D1_DRAW_TEXT_OPTIONS_CLIP);
}

void TextCentred(const std::wstring& s, IDWriteTextFormat* f, const D2D1_RECT_F& r, uint32_t c) {
    auto l = Layout(s, f, r.right - r.left);
    if (!l) return;
    l->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    g_rt->DrawTextLayout({r.left, (r.top + r.bottom) / 2 - TextHeight(l.Get()) / 2}, l.Get(), Brush(c));
}

void TextMiddle(const std::wstring& s, IDWriteTextFormat* f, const D2D1_RECT_F& r, uint32_t c, float a = 1) {
    auto l = Layout(s, f, r.right - r.left);
    if (!l) return;
    g_rt->PushAxisAlignedClip(r, D2D1_ANTIALIAS_MODE_ALIASED);
    g_rt->DrawTextLayout({r.left, (r.top + r.bottom) / 2 - TextHeight(l.Get()) / 2}, l.Get(), Brush(c, a));
    g_rt->PopAxisAlignedClip();
}

void Chevron(float x, float y, uint32_t c) {
    g_rt->DrawLine({x - 4, y - 2}, {x, y + 2}, Brush(c), 1.5f);
    g_rt->DrawLine({x, y + 2}, {x + 4, y - 2}, Brush(c), 1.5f);
}

bool ParseColour(std::wstring v, uint32_t* out) {
    if (v.empty()) return false;
    if (v[0] == L'@') {
        for (int i = 0; i < kThemeRoleCount; i++)
            if (!_wcsicmp(kThemeRoles[i].token, v.c_str())) return ThemeRoleColor(kThemeRoles[i], out);
        return false;
    }
    if (!IsHex6(v)) return false;
    if (v[0] == L'#') v.erase(0, 1);
    *out = (uint32_t)wcstoul(v.c_str(), nullptr, 16);
    return true;
}

const ThemeRole* RoleOf(const std::wstring& v) {
    for (int i = 0; i < kThemeRoleCount; i++)
        if (!_wcsicmp(kThemeRoles[i].token, v.c_str())) return &kThemeRoles[i];
    return nullptr;
}

std::wstring ChoiceLabel(const std::wstring& key, const std::wstring& value) {
    if (const SettingInfo* info = InfoOf(key))
        for (auto const& o : info->options)
            if (o.first == value) return o.second;
    return value;
}

// A list setting's items.
std::vector<std::wstring> SplitList(const std::wstring& value, const std::wstring& separator) {
    std::vector<std::wstring> out;
    wchar_t sep = separator.empty() ? L',' : separator[0];
    size_t at = 0;
    while (at <= value.size()) {
        size_t end = value.find(sep, at);
        std::wstring item = value.substr(at, end == std::wstring::npos ? end : end - at);
        item.erase(0, item.find_first_not_of(L" \t"));
        item.erase(item.find_last_not_of(L" \t") + 1);
        if (!item.empty()) out.push_back(item);
        if (end == std::wstring::npos) break;
        at = end + 1;
    }
    return out;
}

std::wstring JoinList(const std::vector<std::wstring>& items, const std::wstring& separator) {
    std::wstring out;
    for (auto const& i : items) out += (out.empty() ? L"" : separator) + i;
    return out;
}

std::wstring FileNameOf(const std::wstring& path) {
    size_t slash = path.find_last_of(L"\\/");
    return slash == std::wstring::npos ? path : path.substr(slash + 1);
}

// "ctrl+alt+77" as Ctrl+Alt+M; typed names ("Alt+Insert") stay as they are.
std::wstring HotkeyLabel(const std::wstring& full) {
    // (how it's pressed, after a space: "x2", "hold 800")
    std::wstring value = full, how;
    if (size_t sp = full.find(L' '); sp != std::wstring::npos) value = full.substr(0, sp), how = full.substr(sp + 1);
    std::wstring out;
    size_t at = 0;
    while (at <= value.size()) {
        size_t end = value.find(L'+', at);
        std::wstring part = value.substr(at, end == std::wstring::npos ? end : end - at);
        std::wstring shown;
        if (!_wcsicmp(part.c_str(), L"ctrl")) shown = L"Ctrl";
        else if (!_wcsicmp(part.c_str(), L"alt")) shown = L"Alt";
        else if (!_wcsicmp(part.c_str(), L"shift")) shown = L"Shift";
        else if (!_wcsicmp(part.c_str(), L"win")) shown = L"Win";
        else if (!part.empty() && !iswdigit(part[0])) shown = part;
        else if (!part.empty()) {
            UINT vk = (UINT)_wtoi(part.c_str());
            WCHAR name[64] = L"";
            UINT scan = MapVirtualKeyW(vk, MAPVK_VK_TO_VSC);
            bool extended = (vk >= VK_PRIOR && vk <= VK_HELP) || vk == VK_DIVIDE || vk == VK_NUMLOCK;
            if (!scan || !GetKeyNameTextW((LONG)((scan << 16) | (extended ? 1 << 24 : 0)), name, ARRAYSIZE(name)))
                swprintf(name, ARRAYSIZE(name), L"key %u", vk);
            shown = name;
        }
        if (!shown.empty()) out += (out.empty() ? L"" : L"+") + shown;
        if (end == std::wstring::npos) break;
        at = end + 1;
    }
    if (how.find(L"x2") != std::wstring::npos) out += L", double press";
    else if (how.rfind(L"hold", 0) == 0) {
        double ms = _wtof(how.c_str() + 4);
        if (ms <= 0) ms = 500;
        WCHAR t[48];
        swprintf(t, 48, L", held %.1f s", ms / 1000);
        out += t;
    }
    return out;
}

// A motion line ("1, 4, overshoot, slide") in words.
std::wstring MotionLabel(const std::wstring& key, const std::wstring& value) {
    std::vector<std::wstring> parts = SplitList(value, L",");
    if (parts.empty()) {
        const wchar_t* parent = motion::ParentOf(key.substr(7));
        return parent ? std::wstring(L"As ") + parent : L"Each part's own";
    }
    if (parts[0] == L"0") return L"Off";
    std::wstring out = parts.size() > 3 ? parts[3] : L"";
    if (parts.size() > 2) out += (out.empty() ? L"" : L", ") + parts[2];
    if (parts.size() > 1) {
        double s = _wtof(parts[1].c_str()) / 10;
        WCHAR t[32];
        swprintf(t, ARRAYSIZE(t), L"%.1f s", s);
        out += (out.empty() ? L"" : L", ") + std::wstring(t);
    }
    return out.empty() ? value : out;
}

// What a pick setting shows in its box.
std::wstring PickLabel(const std::wstring& key, const std::wstring& value) {
    PickInfo p = PickOf(key);
    std::wstring def = DefaultOf(key);
    switch (p.type) {
        case Pick::Programs:
        case Pick::Titles:
        case Pick::Classes:
        case Pick::StyleSheets: {
            auto items = SplitList(value, p.separator);
            if (items.empty()) return L"None";
            std::wstring out = p.type == Pick::StyleSheets ? FileNameOf(items[0]) : items[0];
            if (items.size() > 1) out += L" and " + std::to_wstring(items.size() - 1) + L" more";
            return out;
        }
        case Pick::Program:
            if (value.empty()) return key.rfind(L"inject.", 0) == 0 ? L"Found by itself" : L"Not set";
            return value.find(L'!') != std::wstring::npos ? L"App: " + value.substr(0, value.find(L'_')) : FileNameOf(value);
        case Pick::Icon:
            if (value.empty()) return L"Default";
            if (value.rfind(L"tourne:", 0) == 0) return L"Design: " + value.substr(7);
            return FileNameOf(value);
        case Pick::Folder:
        case Pick::PlaceTarget:
        case Pick::File:
            if (value.empty()) return def.empty() ? L"Not set" : FileNameOf(def);
            if (value.rfind(L"http", 0) == 0) return L"Web address";
            return value.rfind(L"shell:", 0) == 0 ? value : FileNameOf(value);
        case Pick::Hotkey: return value.empty() ? L"None" : HotkeyLabel(value);
        case Pick::Font: return value.empty() ? L"Default" : value;
        case Pick::Command:
            if (value.empty()) return L"Nothing";
            if (value.rfind(L"tourne:", 0) == 0) return L"Tourne: " + value.substr(7);
            return value.rfind(L"shell:", 0) == 0 || value.find(L"://") != std::wstring::npos ? value : FileNameOf(value);
        case Pick::Motion: return MotionLabel(key, value);
        case Pick::BindsEditor: {
            int binds = 0, rules = 0, idle = 0;
            for (int i = 0, empty = 0; i < 1024 && empty < 8; i++) {
                std::wstring line = ValueOf(kBindRows + std::to_wstring(i) + L"]");
                empty = line.empty() ? empty + 1 : 0;
                size_t a = line.find_first_not_of(L" \t");
                if (a == std::wstring::npos) continue;
                std::wstring head = Lower(line.substr(a, 10));
                if (head.rfind(L"bind", 0) == 0) binds++;
                else if (head.rfind(L"windowrule", 0) == 0) rules++;
                else if (head.rfind(L"idle", 0) == 0) idle++;
            }
            if (!binds && !rules && !idle) return L"Edit (none yet)";
            return L"Edit (" + std::to_wstring(binds) + L" binds, " + std::to_wstring(rules) + L" rules, " + std::to_wstring(idle) + L" idle)";
        }
        case Pick::MenuTree: {
            int rows = 0;
            while (rows < 500 && !(ValueOf(kMenuRows + std::to_wstring(rows) + L"].name").empty() &&
                                   ValueOf(kMenuRows + std::to_wstring(rows) + L"].command").empty()))
                rows++;
            return rows ? L"Edit the menu (" + std::to_wstring(rows) + L" rows)" : L"Edit the menu (LiteMenu's)";
        }
        case Pick::Gradient: {
            if (value.empty()) return L"No border";
            std::vector<std::wstring> colours;
            for (auto const& word : SplitList(value, L" "))
                if (word.find(L',') == std::wstring::npos && word.find(L"deg") == std::wstring::npos) colours.push_back(word);
            if (colours.size() == 1) return colours[0] == L"accent" ? L"Windows' accent" : colours[0] == L"accentInactive" ? L"Windows' inactive accent" : colours[0];
            return L"Gradient: " + JoinList(colours, L" to ");
        }
        case Pick::None: break;
    }
    return value;
}

void Box(const D2D1_RECT_F& r, bool hover) {
    RoundRect(r, 6, hover ? Mix(g_c.ground, g_c.text, 0.10f) : Mix(g_c.ground, g_c.text, 0.05f));
    g_rt->DrawRoundedRectangle(D2D1::RoundedRect({r.left + .5f, r.top + .5f, r.right - .5f, r.bottom - .5f}, 6, 6),
                               Brush(g_c.line), 1);
}

// The box a text box is open in: the colour the text box has, an accent edge.
void EditingBox(const D2D1_RECT_F& r) {
    RoundRect(r, 6, Mix(g_c.ground, g_c.text, 0.05f));
    g_rt->DrawRoundedRectangle(D2D1::RoundedRect({r.left + .5f, r.top + .5f, r.right - .5f, r.bottom - .5f}, 6, 6), Brush(g_c.accent), 1.5f);
}

void DrawControl(const Row& r, int index) {
    std::wstring v = ValueOf(r.key);
    if (g_editing == r.key) EditingBox(ValueBox(r));
    bool hover = g_hover.index == index && (g_hover.part == Part::Control || g_hover.part == Part::Track);
    D2D1_RECT_F c = ControlRect(r);
    switch (r.kind) {
        case Kind::Switch: {
            bool on = IsOn(v);
            RoundRect(c, 12, on ? g_c.accent : Mix(g_c.raised, g_c.text, hover ? 0.28f : 0.2f));
            float cx = on ? c.right - 12 : c.left + 12, cy = (c.top + c.bottom) / 2;
            g_rt->FillEllipse(D2D1::Ellipse({cx, cy}, 8, 8), Brush(on ? g_c.onAccent : g_c.text));
            break;
        }
        case Kind::Choice: {
            Box(c, hover);
            TextMiddle(ChoiceLabel(r.key, v), g_fItem.Get(), {c.left + 12, c.top, c.right - 30, c.bottom}, g_c.text);
            Chevron(c.right - 16, (c.top + c.bottom) / 2, g_c.muted);
            break;
        }
        case Kind::Pick: {
            if (g_editing == r.key) break;  // (typed out: the edit box shows)
            Box(c, hover);
            bool unset = v.empty();
            float x = c.left + 12;
            if (PickOf(r.key).type == Pick::Gradient)  // (its colours as swatches first)
                for (size_t at = v.find(L'#'); at != std::wstring::npos && x < c.left + 80; at = v.find(L'#', at + 1)) {
                    uint32_t col;
                    if (!ParseColour(v.substr(at, 7), &col)) continue;
                    float cy = (c.top + c.bottom) / 2;
                    RoundRect({x, cy - 7, x + 14, cy + 7}, 4, col);
                    x += 18;
                }
            TextMiddle(PickLabel(r.key, v), g_fItem.Get(), {x, c.top, c.right - 30, c.bottom}, unset ? g_c.muted : g_c.text);
            Chevron(c.right - 16, (c.top + c.bottom) / 2, g_c.muted);
            break;
        }
        case Kind::Slider: {
            const SettingInfo* info = InfoOf(r.key);
            D2D1_RECT_F t = TrackRect(r);
            float cy = (t.top + t.bottom) / 2;
            double lo = info->min, hi = info->max, at = std::clamp((double)_wtoi(v.c_str()), lo, hi);
            float x = hi > lo ? t.left + (float)((at - lo) / (hi - lo)) * (t.right - t.left) : t.left;
            RoundRect({t.left, cy - 2, t.right, cy + 2}, 2, Mix(g_c.raised, g_c.text, 0.18f));
            RoundRect({t.left, cy - 2, x, cy + 2}, 2, g_c.accent);
            g_rt->FillEllipse(D2D1::Ellipse({x, cy}, hover ? 9.f : 8.f, hover ? 9.f : 8.f), Brush(g_c.accent));
            g_rt->FillEllipse(D2D1::Ellipse({x, cy}, 3.5f, 3.5f), Brush(g_c.onAccent));
            if (g_editing != r.key) {
                D2D1_RECT_F b = ValueBox(r);
                Box(b, false);
                TextCentred(v, g_fItem.Get(), b, g_c.text);
            }
            break;
        }
        case Kind::Colour: {
            uint32_t col;
            bool has = ParseColour(v, &col);
            if (!has && (v.empty() || RoleOf(v))) has = ParseColour(DefaultOf(r.key), &col);  // (a role with no theme yet: the default shows)
            D2D1_RECT_F sw{c.left, c.top, c.left + 32, c.bottom};
            if (has) RoundRect(sw, 6, col);
            else {
                Box(sw, false);
                g_rt->DrawLine({sw.left + 8, sw.bottom - 8}, {sw.right - 8, sw.top + 8}, Brush(g_c.dim), 1.5f);
            }
            g_rt->DrawRoundedRectangle(D2D1::RoundedRect({sw.left + .5f, sw.top + .5f, sw.right - .5f, sw.bottom - .5f}, 6, 6),
                                       Brush(g_c.text, 0.18f), 1);
            if (g_editing != r.key) {
                D2D1_RECT_F b = ValueBox(r);
                Box(b, hover);
                const ThemeRole* role = RoleOf(v);
                std::wstring label = role ? std::wstring(role->label) + L" (theme)" : v.empty() ? L"Default" : v;
                TextMiddle(label, g_fItem.Get(), {b.left + 12, b.top, b.right - 30, b.bottom}, v.empty() ? g_c.muted : g_c.text);
                Chevron(b.right - 16, (b.top + b.bottom) / 2, g_c.muted);
            }
            break;
        }
        case Kind::Number:
        case Kind::Text: {
            if (g_editing == r.key) break;
            Box(c, hover);
            // Colours written in a value (a gradient's): shown as swatches.
            float x = c.left + 10;
            if (r.kind == Kind::Text && v.find(L'#') != std::wstring::npos) {
                for (size_t at = v.find(L'#'); at != std::wstring::npos && x < c.left + 90; at = v.find(L'#', at + 1)) {
                    uint32_t col;
                    if (!ParseColour(v.substr(at, 7), &col)) continue;
                    float cy = (c.top + c.bottom) / 2;
                    RoundRect({x, cy - 7, x + 14, cy + 7}, 4, col);
                    x += 18;
                }
                if (x > c.left + 10) x += 4;
            }
            std::wstring shown = v.empty() ? L"(empty)" : v;
            TextMiddle(shown, g_fItem.Get(), {x + 2, c.top, c.right - 10, c.bottom}, v.empty() ? g_c.dim : g_c.text);
            break;
        }
    }
}

// What the value is, beside the name: not saved, the Studio's, or yours.
void DrawBadge(const Row& r, float x, float y) {
    std::wstring label;
    uint32_t fill = g_c.muted;
    float a = 0.16f;
    auto c = g_model.changes.find(r.key);
    if (c != g_model.changes.end()) label = c->second.remove ? L"Reset, not saved" : L"Not saved", fill = g_c.accent, a = 0.22f;
    else if (Find(g_model.gui, r.key)) label = Find(g_model.user, r.key) ? L"Studio, over tourne.conf" : L"Studio";
    else if (Find(g_model.user, r.key)) label = L"tourne.conf";
    if (label.empty()) return;
    auto l = Layout(label, g_fSmall.Get(), 400);
    float w = TextWidth(l.Get()) + 14, h = TextHeight(l.Get()) + 2;
    RoundRect({x, y, x + w, y + h}, h / 2, fill, a);
    g_rt->DrawTextLayout({x + 7, y + 1}, l.Get(), Brush(c != g_model.changes.end() ? g_c.accent : g_c.muted));
}

// The Studio's own glass, drawn again when the window moves or the glass changes.
ComPtr<ID2D1Bitmap> g_glass;
bool g_glassValid = false;

bool GlassBackground() {
    if (!GlassStudioOn()) {
        g_glass.Reset();
        return false;
    }
    if (!g_glassValid) {
        g_glassValid = true;
        g_glass.Reset();
        std::vector<uint32_t> px;
        int w = 0, h = 0;
        if (GlassForWindow(g_wnd, &px, &w, &h) && w > 0 && h > 0) {
            D2D1_BITMAP_PROPERTIES props =
                D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED), 96 * g_scale, 96 * g_scale);
            g_rt->CreateBitmap(D2D1::SizeU(w, h), px.data(), w * 4, props, &g_glass);
        }
    }
    return g_glass != nullptr;
}

// A curve in a box: 0 and 1 marked, the curve, and a dot at t.
void DrawCurve(const motion::Anim& a, const D2D1_RECT_F& box, float t, float alpha, float stroke) {
    motion::Anim shown = a;
    if (!a.set) shown.curve = motion::Curve();  // (unset: Hyprland's default, dimmed)
    constexpr int kSteps = 64;
    float values[kSteps + 1];
    float lo = -0.1f, hi = 1.1f;
    for (int i = 0; i <= kSteps; i++) {
        values[i] = (float)motion::Ease(shown, (double)i / kSteps);
        lo = std::min(lo, values[i]), hi = std::max(hi, values[i]);
    }
    auto at = [&](float x, float v) {
        return D2D1::Point2F(box.left + x * (box.right - box.left), box.bottom - (v - lo) / (hi - lo) * (box.bottom - box.top));
    };
    for (float mark : {0.f, 1.f}) g_rt->DrawLine(at(0, mark), at(1, mark), Brush(g_c.line), 1);
    if (a.set && !a.on) {
        g_rt->DrawLine(at(0, 1), at(1, 1), Brush(g_c.muted, alpha), stroke);  // (off: there at once)
        return;
    }
    for (int i = 0; i < kSteps; i++)
        g_rt->DrawLine(at((float)i / kSteps, values[i]), at((float)(i + 1) / kSteps, values[i + 1]), Brush(g_c.accent, alpha), stroke);
    float v = (float)motion::Ease(shown, t);
    D2D1_POINT_2F dot = at(t, v);
    g_rt->FillEllipse(D2D1::Ellipse(dot, stroke + 2.5f, stroke + 2.5f), Brush(g_c.text, alpha));
}

void DrawMotionHeader(float cardA) {
    float left = ContentLeft(), width = ContentWidth(), top = g_curveTop - g_scroll;
    D2D1_RECT_F card{left, top, left + width, top + kCurveH};
    RoundRect(card, 10, g_c.raised, cardA);
    motion::Anim a = MotionOf(g_motionLeaf);
    float t = MotionPhase(a);
    D2D1_RECT_F plot{left + 24, top + 22, left + 24 + 200, top + kCurveH - 22};
    DrawCurve(a, plot, t, 1, 2.5f);
    float x = plot.right + 32, right = card.right - 24;
    Text(g_motionLeaf, g_fSection.Get(), {x, top + 20, right, top + 46}, g_c.text);
    std::wstring line, what;
    if (!a.set) {
        line = L"Not set here or above it.";
        what = L"Each part keeps its own animation. The curve shown is Hyprland's default.";
    } else if (!a.on) {
        line = a.from == L"enabled" ? L"Animations are off." : L"Off (" + a.from + L"'s line).";
        what = L"Things that follow it move at once.";
    } else {
        std::wstring own = ValueOf(L"motion." + g_motionLeaf);
        line = (a.from == g_motionLeaf ? L"Its own line: " : L"From " + a.from + L": ") + ValueOf(L"motion." + a.from);
        wchar_t ms[64];
        swprintf(ms, 64, L"%.0f ms, ", motion::Ms(a));
        what = ms + a.curveName + (a.curve.spring ? L" (a spring)" : L"") + (a.style.empty() ? L"" : L", " + a.style);
    }
    Text(line, g_fBody.Get(), {x, top + 50, right, top + 72}, g_c.muted);
    Text(what, g_fBody.Get(), {x, top + 72, right, top + 94}, g_c.muted);
    // A block moved by it, as the style says: sliding, fading or popping in.
    float track = top + kCurveH - 58, size = 36;
    g_rt->FillRectangle({x, track + size / 2, right, track + size / 2 + 1}, Brush(g_c.line));
    double v = a.set && !a.on ? 1.0 : motion::Ease(a.set ? a : motion::Anim{}, t);
    std::wstring style = motion::StyleWord(a);
    bool fade = style.find(L"fade") != std::wstring::npos, popin = style == L"popin";
    float bx = popin || (fade && style == L"fade") ? right - size : x + (float)v * (right - x - size);
    float scale = popin ? (float)(motion::StylePercent(a, 0.8) + (1 - motion::StylePercent(a, 0.8)) * v) : 1;
    float alpha = fade ? std::clamp((float)v, 0.f, 1.f) : 1;
    float half = size * scale / 2, cx = bx + size / 2, cy = track + size / 2;
    RoundRect({cx - half, cy - half, cx + half, cy + half}, 8, g_c.accent, alpha);
}

void DrawSets(float cardA) {
    auto sets = PageSets();
    if (!sets) return;
    for (int i = 0; i < (int)sets->size(); i++) {
        D2D1_RECT_F r = SetRect(i);
        bool on = SetChosen(*sets, (*sets)[i]), hot = g_hover == Hit{Part::ChipSet, i};
        RoundRect(r, kFilterH / 2, on ? g_c.accent : g_c.raised, on ? 1 : cardA);
        if (hot && !on) RoundRect(r, kFilterH / 2, g_c.text, 0.08f);
        TextCentred((*sets)[i].name, g_fItem.Get(), r, on ? g_c.onAccent : g_c.text);
    }
}

////////////////////////////////////////////////////////////////////////////////
// Views: Alt+Tab's views as cards, each with a small moving picture of it; a
// click picks it (switcher.view.style). The view's own settings follow as rows.

struct ViewCard {
    const wchar_t* id;
    const wchar_t* name;
    const wchar_t* about;
};
const ViewCard kViewCards[] = {
    {L"list", L"List", L"Windows' own Alt+Tab, restyled"},
    {L"flip3d", L"Flip 3D", L"A slanted stack, the chosen in front"},
    {L"coverflow", L"Cover Flow", L"One facing you, the rest turned"},
    {L"grid", L"Grid", L"Every window at once"},
    {L"mission", L"Mission Control", L"Spread out where they sit"},
    {L"ring", L"Ring", L"A turning circle of windows"},
    {L"shelf", L"Shelf", L"One row, magnified like a dock"},
};
static_assert(ARRAYSIZE(kViewCards) == kViewCount);


D2D1_RECT_F ViewRect(int n) {
    int row = n / g_viewColumns, column = n % g_viewColumns;
    float x = ContentLeft() + column * (g_viewW + kTileGap), y = g_viewsTop + row * (kViewH + kTileGap) - g_scroll;
    return {x, y, x + g_viewW, y + kViewH};
}

// One window in a preview: a rounded card, a title strip, lit when chosen.
void PreviewWindow(D2D1_RECT_F r, bool lit, float a = 1) {
    if (r.right - r.left < 2 || r.bottom - r.top < 2) return;
    float rad = std::min(4.f, (r.right - r.left) / 6);
    RoundRect(r, rad, lit ? g_c.accent : g_c.hover, a);
    float strip = std::min(5.f, (r.bottom - r.top) / 5);
    g_rt->FillRectangle({r.left + 2, r.top + 2, r.right - 2, r.top + 2 + strip}, Brush(lit ? g_c.onAccent : g_c.line, a * (lit ? 0.35f : 1)));
}

// The view drawn small in r, at time t (seconds), as it moves.
void DrawViewPreview(int view, D2D1_RECT_F r, float t) {
    float w = r.right - r.left, h = r.bottom - r.top, cx = (r.left + r.right) / 2, cy = (r.top + r.bottom) / 2;
    g_rt->PushAxisAlignedClip(r, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
    RoundRect(r, 6, g_c.ground, 0.9f);
    auto smooth = [](float x) { x = std::clamp(x, 0.f, 1.f); return x * x * (3 - 2 * x); };
    float step = std::fmod(t, 1.2f) / 1.2f, ease = smooth(step * 2.2f);  // (a move, then a rest)
    int at = (int)(t / 1.2f);
    D2D1_MATRIX_3X2_F before;
    g_rt->GetTransform(&before);
    switch (view) {
        case 0: {  // the list: a panel of tiles, the light moving along
            float pw = w * 0.82f, ph = h * 0.42f, px = cx - pw / 2, py = cy - ph / 2;
            RoundRect({px, py, px + pw, py + ph}, 6, g_c.raised);
            float tw = (pw - 6 * 6) / 5;
            int from = at % 5, to = (at + 1) % 5;
            float lx = px + 6 + (from + ease * (to - from)) * (tw + 6);
            RoundRect({lx - 3, py + 3, lx + tw + 3, py + ph - 3}, 5, g_c.accent, 0.35f);
            for (int i = 0; i < 5; i++) PreviewWindow({px + 6 + i * (tw + 6), py + 8, px + 6 + i * (tw + 6) + tw, py + ph - 8}, false);
            break;
        }
        case 1: {  // Flip 3D: a slanted stack; the front one goes to the back
            float ww = w * 0.42f, wh = h * 0.5f;
            for (int i = 4; i >= 0; i--) {
                float k = i - ease;  // (each moves one place forward)
                if (k < 0) k += 5;
                float x = cx - ww / 2 + 16 * k - 24, y = cy - wh / 2 - 9 * k + 14;
                g_rt->SetTransform(D2D1::Matrix3x2F::Skew(0, -14, {x, y + wh / 2}) * before);
                PreviewWindow({x, y, x + ww * 0.8f, y + wh}, k < 0.5f, std::clamp(1.1f - k * 0.17f, 0.f, 1.f));
                g_rt->SetTransform(before);
            }
            break;
        }
        case 2: {  // Cover Flow: the middle one facing you, the rest turned, sliding along
            float ww = w * 0.3f, wh = h * 0.55f;
            for (int i = -3; i <= 3; i++) {
                float k = i - ease + 1;
                float x = cx +(std::abs(k) <= 1 ? k * ww * 0.85f : (k > 0 ? 1 : -1) * (ww * 0.85f + (std::abs(k) - 1) * ww * 0.3f));
                float squeeze = std::clamp(1 - std::abs(k) * 0.6f, 0.4f, 1.f);
                float y = cy - wh / 2;
                g_rt->SetTransform(D2D1::Matrix3x2F::Skew(0, k > 0 ? 10.f * std::min(1.f, k) : -10.f * std::min(1.f, -k), {x, cy}) * before);
                PreviewWindow({x - ww * squeeze / 2, y, x + ww * squeeze / 2, y + wh}, std::abs(k) < 0.5f);
                g_rt->SetTransform(before);
            }
            break;
        }
        case 3: {  // Grid: three by two, the light moving
            float gw = w * 0.8f, gh = h * 0.7f, gx = cx - gw / 2, gy = cy - gh / 2, tw = (gw - 2 * 8) / 3, th = (gh - 8) / 2;
            int lit = at % 6;
            for (int i = 0; i < 6; i++) {
                D2D1_RECT_F c{gx + (i % 3) * (tw + 8), gy + (i / 3) * (th + 8), 0, 0};
                c.right = c.left + tw, c.bottom = c.top + th;
                PreviewWindow(c, i == lit);
            }
            break;
        }
        case 4: {  // Mission Control: from on top of each other to spread out, the desktops above
            for (int i = 0; i < 3; i++) {
                float dw = w * 0.16f, dx = cx - (3 * dw + 16) / 2 + i * (dw + 8);
                RoundRect({dx, r.top + 8, dx + dw, r.top + 8 + h * 0.14f}, 3, i == 0 ? g_c.accent : g_c.raised, i == 0 ? 0.7f : 1);
            }
            float spread = smooth(std::fmod(t, 2.4f) < 1.2f ? std::fmod(t, 2.4f) / 0.9f : 1 - (std::fmod(t, 2.4f) - 1.2f) / 0.9f);
            const float home[4][4] = {{0.30f, 0.38f, 0.66f, 0.80f}, {0.38f, 0.42f, 0.74f, 0.86f}, {0.26f, 0.46f, 0.58f, 0.84f}, {0.42f, 0.36f, 0.78f, 0.76f}};
            const float out[4][4] = {{0.08f, 0.32f, 0.46f, 0.60f}, {0.54f, 0.32f, 0.92f, 0.60f}, {0.08f, 0.66f, 0.46f, 0.94f}, {0.54f, 0.66f, 0.92f, 0.94f}};
            for (int i = 0; i < 4; i++) {
                float c[4];
                for (int j = 0; j < 4; j++) c[j] = home[i][j] + (out[i][j] - home[i][j]) * spread;
                PreviewWindow({r.left + c[0] * w, r.top + c[1] * h, r.left + c[2] * w, r.top + c[3] * h}, i == 0 && spread > 0.9f);
            }
            break;
        }
        case 5: {  // Ring: windows on a turning circle, the front one biggest
            struct Placed {
                float depth;
                D2D1_RECT_F rc;
                bool front;
            };
            std::vector<Placed> placed;
            float turn = (at + ease) * 6.2831853f / 6;
            for (int i = 0; i < 6; i++) {
                float a = i * 6.2831853f / 6 - turn;
                float depth = (std::cos(a) + 1) / 2, s = 0.45f + 0.55f * depth;
                float x = cx + std::sin(a) * w * 0.32f, y = cy + depth * h * 0.12f - h * 0.04f;
                float ww = w * 0.24f * s, wh = h * 0.36f * s;
                placed.push_back({depth, {x - ww / 2, y - wh / 2, x + ww / 2, y + wh / 2}, depth > 0.97f});
            }
            std::sort(placed.begin(), placed.end(), [](Placed const& a, Placed const& b) { return a.depth < b.depth; });
            for (auto const& p : placed) PreviewWindow(p.rc, p.front, 0.45f + 0.55f * p.depth);
            break;
        }
        default: {  // Shelf: one row, magnified under a pointer that wanders along it
            float px = cx + std::sin(t * 1.7f) * w * 0.34f, base = r.bottom - h * 0.22f;
            float x = r.left + w * 0.08f;
            float unit = w * 0.84f / 7;
            for (int pass = 0; pass < 2; pass++)  // (the one under the pointer last, on top)
                for (int i = 0; i < 7; i++) {
                    float mid = x + unit * (i + 0.5f), d = std::abs(mid - px) / (unit * 1.6f);
                    if ((d < 0.35f) != (pass == 1)) continue;
                    float s = 1 + 0.9f * std::max(0.f, 1 - d * d);
                    float ww = unit * 0.78f * s, wh = unit * 0.7f * s;
                    PreviewWindow({mid - ww / 2, base - wh, mid + ww / 2, base}, d < 0.35f);
                }
            g_rt->FillEllipse(D2D1::Ellipse({px, base + 8}, 2.5f, 2.5f), Brush(g_c.text, 0.8f));
            break;
        }
    }
    g_rt->SetTransform(before);
    g_rt->PopAxisAlignedClip();
}

void DrawViews(float cardA) {
    std::wstring chosen = ValueOf(L"switcher.view.style");
    float t = (float)((GetTickCount64() - g_motionStart) / 1000.0);
    for (int i = 0; i < kViewCount; i++) {
        D2D1_RECT_F c = ViewRect(i);
        if (c.bottom < 0 || c.top > ViewBottom()) continue;
        bool on = chosen == kViewCards[i].id || (chosen.empty() && i == 0), hot = g_hover == Hit{Part::ViewCard, i};
        RoundRect(c, 10, g_c.raised, cardA);
        if (hot) RoundRect(c, 10, g_c.text, 0.05f);
        if (on) g_rt->DrawRoundedRectangle(D2D1::RoundedRect({c.left + 1, c.top + 1, c.right - 1, c.bottom - 1}, 10, 10), Brush(g_c.accent), 2);
        DrawViewPreview(i, {c.left + 10, c.top + 10, c.right - 10, c.top + 112}, t + i * 0.37f);
        Text(kViewCards[i].name, g_fItem.Get(), {c.left + 12, c.top + 120, c.right - 12, c.top + 140}, on ? g_c.accent : g_c.text);
        Text(kViewCards[i].about, g_fSmall.Get(), {c.left + 12, c.top + 140, c.right - 12, c.top + 160}, g_c.muted);
    }
}

void DrawFilters(float cardA) {
    for (int i = 0; i < kFilterCount; i++) {
        D2D1_RECT_F r = FilterRect(i);
        bool on = i == g_filter, hot = g_hover == Hit{Part::Filter, i};
        RoundRect(r, kFilterH / 2, on ? g_c.accent : g_c.raised, on ? 1 : cardA);
        if (hot && !on) RoundRect(r, kFilterH / 2, g_c.text, 0.08f);
        TextCentred(FilterName(i), g_fItem.Get(), r, on ? g_c.onAccent : g_c.text);
    }
}

// An extension's card: its name and switch-like button, who made it, what it does, what kind it is and what it sets.
void DrawExtension(int n, const Extension& e, D2D1_RECT_F r, float cardA) {
    bool on = ExtensionOn(e);
    float left = r.left + 14, right = r.right - 14;
    RoundRect(r, 10, g_c.raised, cardA);
    if (on) g_rt->DrawRoundedRectangle(D2D1::RoundedRect({r.left + 1, r.top + 1, r.right - 1, r.bottom - 1}, 9, 9), Brush(g_c.accent), 1.5f);
    D2D1_RECT_F b = TileButtonRect(n);
    bool hot = g_hover == Hit{Part::TileButton, n};
    if (on) {
        Box(b, hot);
        TextCentred(L"Turn off", g_fItem.Get(), b, g_c.text);
    } else {
        RoundRect(b, 6, g_c.accent, hot ? 0.85f : 1);
        TextCentred(L"Turn on", g_fItem.Get(), b, g_c.onAccent);
    }
    Text(e.name, g_fSection.Get(), {left, r.top + 14, b.left - 10, r.top + 34}, g_c.text);
    std::wstring by = e.author + L" · " + (e.licence.empty() ? L"no licence given" : e.licence) + L" · " + e.version;
    Text(by, g_fSmall.Get(), {left, r.top + 36, b.left - 10, r.top + 52}, g_c.muted);
    D2D1_RECT_F about{left, r.top + 60, right, r.top + 60 + 36};
    if (auto l = Layout(e.description, g_fBody.Get(), right - left)) {
        g_rt->PushAxisAlignedClip(about, D2D1_ANTIALIAS_MODE_ALIASED);
        g_rt->DrawTextLayout({about.left, about.top}, l.Get(), Brush(g_c.text, 0.9f));
        g_rt->PopAxisAlignedClip();
    }
    float x = left, y = r.top + 104;
    std::vector<std::wstring> chips = {e.kind, e.builtIn ? L"Built in" : L"Added"};
    if (on) chips.insert(chips.begin(), L"On");
    for (size_t i = 0; i < chips.size(); i++) {
        auto l = Layout(chips[i], g_fSmall.Get(), 200);
        float w = TextWidth(l.Get()) + 14;
        bool lit = on && i == 0;
        RoundRect({x, y, x + w, y + 20}, 10, lit ? g_c.accent : g_c.text, lit ? 1 : 0.08f);
        TextCentred(chips[i], g_fSmall.Get(), {x, y, x + w, y + 20}, lit ? g_c.onAccent : g_c.text);
        x += w + 6;
    }
    std::wstring sets;
    for (auto const& [key, value] : e.settings) sets += (sets.empty() ? L"Sets " : L", ") + NameOf(key);
    Text(sets, g_fSmall.Get(), {left, r.bottom - 34, right, r.bottom - 14}, g_c.muted);
}

// A plugin's card: its name and button, who made it, what it does, where it shows, and what it needs.
void DrawPlugins(float view, float cardA) {
    DrawFilters(cardA);
    {
        D2D1_RECT_F b = HeaderButton(Part::Compare);
        Box(b, g_hover.part == Part::AddExtension);
        TextCentred(L"Add an extension…", g_fItem.Get(), b, g_c.text);
    }
    auto const& all = PluginCatalogue();
    int first = std::max(0, (int)((g_scroll - g_tilesTop) / (kPluginTileH + kTileGap)) * g_columns);
    for (int n = first; n < (int)g_tiles.size(); n++) {
        D2D1_RECT_F r = TileRect(n);
        if (r.top > view) break;
        if (g_tiles[n] < 0) {
            DrawExtension(n, ExtensionList()[-1 - g_tiles[n]], r, cardA);
            continue;
        }
        PluginInfo const& p = all[g_tiles[n]];
        PluginState state = PluginStateOf(p);
        float left = r.left + 14, right = r.right - 14;
        RoundRect(r, 10, g_c.raised, cardA);
        if (state == PluginState::Installed)
            g_rt->DrawRoundedRectangle(D2D1::RoundedRect({r.left + 1, r.top + 1, r.right - 1, r.bottom - 1}, 9, 9), Brush(g_c.accent), 1.5f);

        D2D1_RECT_F b = TileButtonRect(n);
        bool hot = g_hover == Hit{Part::TileButton, n};
        const wchar_t* label = state == PluginState::Installing ? L"Installing…"
                               : state == PluginState::Installed ? L"Remove"
                               : state == PluginState::Failed    ? L"Try again"
                                                                 : L"Install";
        if (state == PluginState::NotInstalled || state == PluginState::Failed) {
            RoundRect(b, 6, g_c.accent, hot ? 0.85f : 1);
            TextCentred(label, g_fItem.Get(), b, g_c.onAccent);
        } else {
            Box(b, hot && state == PluginState::Installed);
            TextCentred(label, g_fItem.Get(), b, state == PluginState::Installing ? g_c.muted : g_c.text);
        }

        Text(p.name, g_fSection.Get(), {left, r.top + 14, b.left - 10, r.top + 34}, g_c.text);
        std::wstring by = p.author + L" · " + (p.license.empty() ? L"no licence given" : p.license) + L" · " + p.version;
        Text(by, g_fSmall.Get(), {left, r.top + 36, b.left - 10, r.top + 52}, g_c.muted);
        D2D1_RECT_F about{left, r.top + 60, right, r.top + 60 + 36};
        if (auto l = Layout(p.description, g_fBody.Get(), right - left)) {
            g_rt->PushAxisAlignedClip(about, D2D1_ANTIALIAS_MODE_ALIASED);
            g_rt->DrawTextLayout({about.left, about.top}, l.Get(), Brush(g_c.text, 0.9f));
            g_rt->PopAxisAlignedClip();
        }

        // Where it shows, as chips; "Installed" first when it is.
        float x = left, y = r.top + 104;
        std::vector<std::wstring> chips = PluginSurfaces(p);
        if (state == PluginState::Installed) chips.insert(chips.begin(), L"Installed");
        for (size_t i = 0; i < chips.size(); i++) {
            auto l = Layout(chips[i], g_fSmall.Get(), 200);
            float w = TextWidth(l.Get()) + 14;
            if (x + w > right) break;
            bool installed = state == PluginState::Installed && i == 0;
            RoundRect({x, y, x + w, y + 20}, 10, installed ? g_c.accent : g_c.text, installed ? 1 : 0.08f);
            TextCentred(chips[i], g_fSmall.Get(), {x, y, x + w, y + 20}, installed ? g_c.onAccent : g_c.text);
            x += w + 6;
        }

        std::wstring needs = PluginNeedsLine(p);
        if (state == PluginState::Failed) needs = L"Didn't install: " + PluginError(p);
        Text(needs, g_fSmall.Get(), {left, r.bottom - 34, right, r.bottom - 14}, state == PluginState::Failed ? g_c.accent : g_c.muted);
    }
}

void DrawPresets(float view, float cardA) {
    DrawFilters(cardA);
    for (auto [part, label] : {std::pair{Part::ImportLook, L"Import a look…"}, std::pair{Part::ExportLook, L"Export my look…"}}) {
        D2D1_RECT_F b = HeaderButton(part == Part::ImportLook ? Part::Compare : Part::TestWindow);
        Box(b, g_hover.part == part);
        TextCentred(label, g_fItem.Get(), b, g_c.text);
    }
    if (g_tiles.empty()) return;
    int first = std::max(0, (int)((g_scroll - g_tilesTop) / (kTileH + kTileGap)) * g_columns);
    auto const& all = Presets();
    for (int n = first; n < (int)g_tiles.size(); n++) {
        D2D1_RECT_F r = TileRect(n);
        if (r.top > view) break;
        Preset const& p = all[g_tiles[n]];
        enum { Ground, Raised, Line, Edge, TextRole, Accent, Muted, Dim, Warn, Bad, Accent2, Good };
        bool chosen = PresetChosen(p), hot = g_hover == Hit{Part::Tile, n};
        RoundRect(r, 10, p.roles[Ground]);
        D2D1_RECT_F inner{r.left + .5f, r.top + .5f, r.right - .5f, r.bottom - .5f};
        if (chosen) {
            g_rt->DrawRoundedRectangle(D2D1::RoundedRect({r.left + 1, r.top + 1, r.right - 1, r.bottom - 1}, 9, 9), Brush(g_c.accent), 2.5f);
        } else {
            g_rt->DrawRoundedRectangle(D2D1::RoundedRect(inner, 10, 10), Brush(hot ? p.roles[Accent] : p.roles[Line]), hot ? 1.5f : 1);
        }
        // A bar of its raised colour, like a panel, with its accent as the "active" mark.
        RoundRect({r.left + 12, r.top + 12, r.right - 12, r.top + 22}, 4, p.roles[Raised]);
        RoundRect({r.left + 16, r.top + 15, r.left + 40, r.top + 19}, 2, p.roles[Accent]);
        Text(p.name, g_fItem.Get(), {r.left + 12, r.top + 30, r.right - 12, r.top + 50}, p.roles[TextRole]);
        Text(p.family, g_fSmall.Get(), {r.left + 12, r.top + 51, r.right - 12, r.top + 66}, p.roles[Muted]);
        float sx = r.left + 12, sy = r.bottom - 28;
        for (int role : {Accent, Accent2, Good, Warn, Bad, TextRole, Edge}) {
            RoundRect({sx, sy, sx + 18, sy + 14}, 4, p.roles[role]);
            sx += 22;
        }
        std::wstring tag = p.pick ? L"Pick" : *p.settings ? L"Look" : L"";
        if (chosen) tag = L"In use";
        if (!tag.empty()) {
            auto l = Layout(tag, g_fSmall.Get(), 100);
            float w = TextWidth(l.Get()) + 12;
            D2D1_RECT_F t{r.right - 12 - w, r.bottom - 30, r.right - 12, r.bottom - 12};
            RoundRect(t, 9, chosen ? g_c.accent : p.roles[Accent], chosen ? 1 : 0.22f);
            TextCentred(tag, g_fSmall.Get(), t, chosen ? g_c.onAccent : p.roles[TextRole]);
        }
    }
}

void Paint() {
    D2D1_SIZE_F s = ClientSize();
    g_rt->BeginDraw();
    g_rt->SetTransform(D2D1::Matrix3x2F::Identity());
    g_rt->Clear(Color(g_c.ground));
    // In glass, the panels let it show through.
    bool glassy = GlassBackground();
    float cardA = 1, sideA = 1, barA = 1;
    if (glassy) {
        g_rt->DrawBitmap(g_glass.Get(), {0, 0, s.width, s.height});
        g_rt->FillRectangle({0, 0, s.width, s.height}, Brush(g_c.ground, 0.22f));
        cardA = 0.62f, sideA = 0.5f, barA = 0.9f;
    }

    // Settings
    float left = ContentLeft(), width = ContentWidth(), view = ViewBottom();
    g_rt->PushAxisAlignedClip({kSidebar, 0, s.width, view}, D2D1_ANTIALIAS_MODE_ALIASED);
    Text(PageTitle(), g_fTitle.Get(), {left, 28 - g_scroll, left + width, 70 - g_scroll}, g_c.text);
    std::wstring about = PageDescription();
    if (!about.empty()) {
        auto l = Layout(about, g_fBody.Get(), width);
        g_rt->DrawTextLayout({left, 68 - g_scroll}, l.Get(), Brush(g_c.muted));
    }
    if (GlassPage()) {
        auto button = [&](Part p, const std::wstring& label, bool on) {
            D2D1_RECT_F b = HeaderButton(p);
            if (on) RoundRect(b, 6, g_c.accent);
            else Box(b, g_hover.part == p);
            TextCentred(label, g_fItem.Get(), b, on ? g_c.onAccent : g_c.text);
        };
        button(Part::TestWindow, GlassTestWindowShown() ? L"Close test window" : L"Open test window", GlassTestWindowShown());
        button(Part::Compare, L"Hold to compare", g_pressed.part == Part::Compare);
    }
    if (PresetsPage()) DrawPresets(view, cardA);
    if (ExtensionsPage()) DrawPlugins(view, cardA);
    if (MotionPage()) DrawMotionHeader(cardA);
    if (ViewsPage()) DrawViews(cardA);
    DrawSets(cardA);
    // The rows of a section share one card.
    for (size_t i = 0; i < g_rows.size(); i++) {
        if (g_rows[i].section || !g_rows[i].first) continue;
        size_t last = i;
        while (last + 1 < g_rows.size() && !g_rows[last].last) last++;
        float top = g_rows[i].y - g_scroll, bottom = g_rows[last].y - g_scroll + g_rows[last].h;
        if (bottom >= 0 && top <= view) RoundRect({left, top, left + width, bottom}, 10, g_c.raised, cardA);
    }
    for (size_t i = 0; i < g_rows.size(); i++) {
        Row const& r = g_rows[i];
        float top = r.y - g_scroll;
        if (top > view || top + r.h < -40) continue;
        if (r.section) {
            if (!r.name) continue;
            g_rt->DrawTextLayout({left + 2, top}, r.name.Get(), Brush(g_c.text));
            if (r.description) g_rt->DrawTextLayout({left + 2, top + TextHeight(r.name.Get()) + 4}, r.description.Get(), Brush(g_c.muted));
            continue;
        }
        D2D1_RECT_F card{left, top, left + width, top + r.h};
        if (!r.last) g_rt->FillRectangle({card.left + kRowPad, card.bottom, card.right - kRowPad, card.bottom + 1}, Brush(g_c.line));
        float th = TextHeight(r.name.Get()) + (r.description ? 3 + TextHeight(r.description.Get()) : 0);
        float ty = top + (r.h - th) / 2;
        g_rt->DrawTextLayout({left + kRowPad, ty}, r.name.Get(), Brush(g_c.text));
        DrawBadge(r, left + kRowPad + TextWidth(r.name.Get()) + 8, ty + 2);
        if (r.description) g_rt->DrawTextLayout({left + kRowPad, ty + TextHeight(r.name.Get()) + 3}, r.description.Get(), Brush(g_c.muted));
        if (IsMotionLeaf(r.key)) {
            D2D1_RECT_F rr = ResetRect(r);
            motion::Anim a = MotionOf(r.key.substr(7));
            DrawCurve(a, {rr.left - 56, rr.top - 2, rr.left - 10, rr.bottom + 2}, MotionPhase(a), a.set ? 1.f : 0.35f, 1.5f);
        }
        if (CanReset(r)) {
            D2D1_RECT_F rr = ResetRect(r);
            bool hot = g_hover == Hit{Part::Reset, (int)i};
            if (hot) RoundRect(rr, 6, g_c.text, 0.08f);
            TextCentred(g_model.changes.count(r.key) ? L"Revert" : L"Reset", g_fSmall.Get(), rr, hot ? g_c.text : g_c.accent);
        }
        DrawControl(r, (int)i);
    }
    D2D1_RECT_F thumb = ScrollThumb();
    if (thumb.right > 0)
        RoundRect(thumb, 3, g_c.text, g_hover.part == Part::ScrollThumb || g_pressed.part == Part::ScrollThumb ? 0.35f : 0.18f);
    g_rt->PopAxisAlignedClip();

    // Sidebar
    g_rt->FillRectangle({0, 0, kSidebar, s.height}, Brush(Mix(g_c.ground, g_c.raised, 0.6f), sideA));
    g_rt->FillRectangle({kSidebar - 1, 0, kSidebar, s.height}, Brush(g_c.line));
    Text(L"Tourne Studio", g_fApp.Get(), {24, 20, kSidebar - 12, 48}, g_c.text);
    D2D1_RECT_F sr{12, 56, kSidebar - 12, 88};
    RoundRect(sr, 6, g_c.raised);
    g_rt->DrawRoundedRectangle(D2D1::RoundedRect({sr.left + .5f, sr.top + .5f, sr.right - .5f, sr.bottom - .5f}, 6, 6),
                               Brush(GetFocus() == g_search ? g_c.accent : g_c.line), 1);
    bool searching = !g_query.empty() && !g_queryPresets;
    for (int i = 0; i < SidebarCount(); i++) {
        D2D1_RECT_F ir = ItemRect(i);
        bool selected = !searching && i == g_page, hot = g_hover == Hit{Part::Item, i};
        if (selected) RoundRect(ir, 8, g_c.accent, 0.16f);
        else if (hot) RoundRect(ir, 8, g_c.text, 0.06f);
        if (selected) RoundRect({ir.left, ir.top + 9, ir.left + 3, ir.bottom - 9}, 1.5f, g_c.accent);
        std::wstring label = i < (int)g_pages.size() ? g_pages[i].title : L"Changes not saved (" + std::to_wstring(g_model.changes.size()) + L")";
        TextMiddle(label, g_fItem.Get(), {ir.left + 14, ir.top, ir.right - 8, ir.bottom},
                   i == PendingPage() ? g_c.accent : selected ? g_c.text : g_c.muted);
    }

    // What's not saved
    if (Pending()) {
        D2D1_RECT_F bar{kSidebar, view, s.width, s.height};
        g_rt->FillRectangle(bar, Brush(g_c.raised, barA));
        g_rt->FillRectangle({bar.left, bar.top, bar.right, bar.top + 1}, Brush(g_c.line));
        size_t n = g_model.changes.size();
        std::wstring what = std::to_wstring(n) + (n == 1 ? L" change" : L" changes") + L" not saved. Showing already.";
        TextMiddle(what, g_fItem.Get(), {bar.left + kPad, bar.top, BarButton(Part::Undo).left - 12, bar.bottom}, g_c.text);
        auto button = [&](Part p, const wchar_t* label, bool primary, bool enabled) {
            D2D1_RECT_F b = BarButton(p);
            bool hot = enabled && g_hover.part == p;
            if (primary) RoundRect(b, 6, hot ? Mix(g_c.accent, g_c.text, 0.12f) : g_c.accent);
            else Box(b, hot);
            TextCentred(label, g_fItem.Get(), b, primary ? g_c.onAccent : enabled ? g_c.text : g_c.dim);
        };
        button(Part::Undo, L"Undo", false, !g_model.undo.empty());
        button(Part::Redo, L"Redo", false, !g_model.redo.empty());
        button(Part::Discard, L"Discard", false, true);
        button(Part::Save, L"Save", true, true);
    }

    if ((MotionPage() || ViewsPage()) && !g_motionTimer) g_motionTimer = SetTimer(g_wnd, kTimerMotion, 16, nullptr) != 0;  // (the previews play)
    if (g_rt->EndDraw() == (HRESULT)D2DERR_RECREATE_TARGET) {
        g_rt.Reset();
        g_glass.Reset(), g_glassValid = false;
        g_brush.Reset();
    }
}

////////////////////////////////////////////////////////////////////////////////
// The text boxes: search, and the one that opens on a value

void PlaceSearch() {
    int x = (int)std::lround(24 * g_scale), y = (int)std::lround(63 * g_scale);
    int w = (int)std::lround((kSidebar - 48) * g_scale), h = (int)std::lround(20 * g_scale);
    SetWindowPos(g_search, nullptr, x, y, w, h, SWP_NOZORDER | SWP_NOACTIVATE);
}

void MakeEditFont() {
    if (g_editFont) DeleteObject(g_editFont);
    g_editFont = CreateFontW(-(int)std::lround(14 * g_scale), 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                             CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
    SendMessageW(g_search, WM_SETFONT, (WPARAM)g_editFont, TRUE);
    SendMessageW(g_edit, WM_SETFONT, (WPARAM)g_editFont, TRUE);
}

void Relayout() {
    LayoutRows();
    ScrollTo(g_scroll);
    InvalidateRect(g_wnd, nullptr, FALSE);
}

void Rebuild(bool toTop) {
    BuildRows();
    if (toTop) g_scroll = 0;
    Relayout();
}

int RowOf(const std::wstring& key) {
    for (size_t i = 0; i < g_rows.size(); i++)
        if (!g_rows[i].section && g_rows[i].key == key) return (int)i;
    return -1;
}

void CloseEditor(bool keep) {
    if (g_editing.empty()) return;
    std::wstring key = g_editing;
    g_editing.clear();
    if (keep) {
        int n = GetWindowTextLengthW(g_edit);
        std::wstring text(n, L'\0');
        GetWindowTextW(g_edit, text.data(), n + 1);
        text.erase(0, text.find_first_not_of(L" \t"));
        text.erase(text.find_last_not_of(L" \t") + 1);
        Kind kind = KindOf(key);
        const SettingInfo* info = InfoOf(key);
        bool ok = true;
        if (kind == Kind::Slider || kind == Kind::Number) {
            ok = IsNumber(text);
            if (ok && info && (info->hasMin || info->hasMax)) {
                double v = wcstod(text.c_str(), nullptr);
                if (info->hasMin) v = std::max(v, info->min);
                if (info->hasMax) v = std::min(v, info->max);
                wchar_t buf[32];
                _snwprintf(buf, 32, IsInt(DefaultOf(key)) ? L"%.0f" : L"%g", v);
                text = buf;
            }
        } else if (kind == Kind::Colour && text.empty()) {
            text = DefaultOf(key);
        } else if (kind == Kind::Colour) {
            if (text[0] == L'@') ok = RoleOf(text) != nullptr;
            else if (IsHex6(text)) {
                // Written as its default is: with the # or without.
                std::wstring bare = text[0] == L'#' ? text.substr(1) : text;
                std::wstring def = DefaultOf(key);
                text = (!def.empty() && def[0] != L'#' ? L"" : L"#") + bare;
                for (auto& c : text) c = towupper(c);
            }
        }
        if (ok) SetValue(key, text);
        else MessageBeep(MB_ICONWARNING);
    }
    ShowWindow(g_edit, SW_HIDE);
    if (GetFocus() == g_edit) SetFocus(g_wnd);
    InvalidateRect(g_wnd, nullptr, FALSE);
}

void OpenEditor(int index) {
    CloseEditor(true);
    Row const& r = g_rows[index];
    D2D1_RECT_F b = ValueBox(r);
    float inset = r.kind == Kind::Slider ? 6.f : 12.f;
    g_editing = r.key;
    SetWindowTextW(g_edit, ValueOf(r.key).c_str());
    SetWindowLongPtrW(g_edit, GWL_STYLE, (GetWindowLongPtrW(g_edit, GWL_STYLE) & ~(ES_CENTER | ES_LEFT)) |
                                              (r.kind == Kind::Slider ? ES_CENTER : ES_LEFT));
    int h = (int)std::lround(20 * g_scale);
    int y = (int)std::lround(((b.top + b.bottom) / 2) * g_scale) - h / 2;
    SetWindowPos(g_edit, HWND_TOP, (int)std::lround((b.left + inset) * g_scale), y, (int)std::lround((b.right - b.left - 2 * inset) * g_scale),
                 h, SWP_SHOWWINDOW);
    SetFocus(g_edit);
    SendMessageW(g_edit, EM_SETSEL, 0, -1);
    InvalidateRect(g_wnd, nullptr, FALSE);
}

void Click(Hit h, float x);

LRESULT CALLBACK EditProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR id, DWORD_PTR) {
    if (msg == WM_KEYDOWN || msg == WM_CHAR) {
        bool ctrl = GetKeyState(VK_CONTROL) < 0;
        if (msg == WM_KEYDOWN && ctrl && wp == 'S') {
            if (Pending() || !g_editing.empty()) Click({Part::Save}, 0);
            return 0;
        }
        if (id == kEditId && wp == VK_RETURN) return msg == WM_KEYDOWN ? (CloseEditor(true), 0) : 0;
        if (id == kEditId && wp == VK_ESCAPE) return msg == WM_KEYDOWN ? (CloseEditor(false), 0) : 0;
        if (id == kEditId && wp == VK_TAB) return 0;
        if (id == kSearchId && wp == VK_ESCAPE) return msg == WM_KEYDOWN ? (SetWindowTextW(wnd, L""), SetFocus(g_wnd), 0) : 0;
        if (id == kSearchId && (wp == VK_RETURN || (ctrl && wp == 0x7F))) return 0;  // (no beep)
        if (id == kSearchId && msg == WM_CHAR && wp < 32 && wp != 8 && wp != 22 && wp != 3 && wp != 24 && wp != 1) return 0;
    }
    if (msg == WM_KILLFOCUS && id == kEditId) {
        LRESULT r = DefSubclassProc(wnd, msg, wp, lp);
        CloseEditor(true);
        return r;
    }
    if ((msg == WM_SETFOCUS || msg == WM_KILLFOCUS) && id == kSearchId) InvalidateRect(g_wnd, nullptr, FALSE);
    return DefSubclassProc(wnd, msg, wp, lp);
}

////////////////////////////////////////////////////////////////////////////////
// What a click does

void ChooseFromMenu(int index) {
    Row const& r = g_rows[index];
    const SettingInfo* info = InfoOf(r.key);
    if (!info) return;
    HMENU menu = CreatePopupMenu();
    std::wstring now = ValueOf(r.key);
    for (size_t i = 0; i < info->options.size(); i++)
        AppendMenuW(menu, MF_STRING | (info->options[i].first == now ? MF_CHECKED : 0), i + 1, info->options[i].second.c_str());
    D2D1_RECT_F c = ControlRect(r);
    POINT pt{(LONG)std::lround(c.left * g_scale), (LONG)std::lround(c.bottom * g_scale) + 2};
    ClientToScreen(g_wnd, &pt);
    int id = TrackPopupMenuEx(menu, TPM_RETURNCMD | TPM_LEFTALIGN | TPM_TOPALIGN, pt.x, pt.y, g_wnd, nullptr);
    DestroyMenu(menu);
    if (id <= 0) return;
    if (r.key == L"glass.look") SetLook(info->options[id - 1].first);
    else SetValue(r.key, info->options[id - 1].first);
}

std::wstring Hex(uint32_t c, bool hash) {
    wchar_t text[8];
    _snwprintf(text, 8, hash ? L"#%06X" : L"%06X", c & 0xFFFFFF);
    return text;
}

void ChooseColour(int index) {
    Row const& r = g_rows[index];
    std::wstring def = DefaultOf(r.key);
    bool hash = def.empty() || def[0] == L'#';
    HMENU menu = CreatePopupMenu();
    AppendMenuW(menu, MF_STRING, 1, L"Pick a colour…");
    AppendMenuW(menu, MF_STRING, 2, L"Type one…");
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    uint32_t sample;
    bool themed = ThemeRoleColor(kThemeRoles[0], &sample);
    std::wstring now = ValueOf(r.key);
    for (int i = 0; i < kThemeRoleCount; i++) {
        uint32_t c;
        std::wstring label = std::wstring(L"Theme: ") + kThemeRoles[i].label;
        if (ThemeRoleColor(kThemeRoles[i], &c)) label += L"\t" + Hex(c, true);
        AppendMenuW(menu, MF_STRING | (!_wcsicmp(now.c_str(), kThemeRoles[i].token) ? MF_CHECKED : 0), 100 + i, label.c_str());
    }
    if (!themed) AppendMenuW(menu, MF_STRING | MF_GRAYED, 0, L"(no theme yet: the default shows until there is)");
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING, 3, L"Use the default");
    D2D1_RECT_F c = ControlRect(r);
    POINT pt{(LONG)std::lround(c.left * g_scale), (LONG)std::lround(c.bottom * g_scale) + 2};
    ClientToScreen(g_wnd, &pt);
    int id = TrackPopupMenuEx(menu, TPM_RETURNCMD | TPM_LEFTALIGN | TPM_TOPALIGN, pt.x, pt.y, g_wnd, nullptr);
    DestroyMenu(menu);
    if (id == 1) {
        static COLORREF custom[16];
        for (int i = 0; i < kThemeRoleCount && i < 16; i++) {
            uint32_t t;
            if (ThemeRoleColor(kThemeRoles[i], &t)) custom[i] = RGB(t >> 16, (t >> 8) & 255, t & 255);
        }
        uint32_t start = 0;
        if (!ParseColour(now, &start)) ParseColour(def, &start);
        CHOOSECOLORW cc{sizeof(cc)};
        cc.hwndOwner = g_wnd;
        cc.rgbResult = RGB(start >> 16, (start >> 8) & 255, start & 255);
        cc.lpCustColors = custom;
        cc.Flags = CC_FULLOPEN | CC_RGBINIT | CC_ANYCOLOR;
        if (ChooseColorW(&cc))
            SetValue(r.key, Hex(GetRValue(cc.rgbResult) << 16 | GetGValue(cc.rgbResult) << 8 | GetBValue(cc.rgbResult), hash));
    } else if (id == 2) {
        OpenEditor(index);
    } else if (id == 3) {
        SetValue(r.key, def);
    } else if (id >= 100) {
        SetValue(r.key, kThemeRoles[id - 100].token);
    }
}

////////////////////////////////////////////////////////////////////////////////
// The pick settings' menus: what's there to choose from, then typing it out.

int PopMenuUnder(HMENU menu, const Row& r) {
    D2D1_RECT_F c = ControlRect(r);
    POINT pt{(LONG)std::lround(c.left * g_scale), (LONG)std::lround(c.bottom * g_scale) + 2};
    ClientToScreen(g_wnd, &pt);
    return TrackPopupMenuEx(menu, TPM_RETURNCMD | TPM_LEFTALIGN | TPM_TOPALIGN, pt.x, pt.y, g_wnd, nullptr);
}

// The open windows (shown, titled, not tool windows): their program's file name, title and class.
struct OpenWindow {
    std::wstring program, title, cls;
};
std::vector<OpenWindow> OpenWindows() {
    std::vector<OpenWindow> out;
    EnumWindows(
        [](HWND h, LPARAM param) -> BOOL {
            if (!IsWindowVisible(h) || GetWindow(h, GW_OWNER) || (GetWindowLongPtrW(h, GWL_EXSTYLE) & WS_EX_TOOLWINDOW)) return TRUE;
            WCHAR title[256] = L"", cls[128] = L"";
            if (!GetWindowTextW(h, title, ARRAYSIZE(title))) return TRUE;
            GetClassNameW(h, cls, ARRAYSIZE(cls));
            DWORD pid = 0;
            GetWindowThreadProcessId(h, &pid);
            std::wstring program;
            if (HANDLE p = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid)) {
                WCHAR path[MAX_PATH];
                DWORD size = ARRAYSIZE(path);
                if (QueryFullProcessImageNameW(p, 0, path, &size)) program = FileNameOf(path);
                CloseHandle(p);
            }
            ((std::vector<OpenWindow>*)param)->push_back({program, title, cls});
            return TRUE;
        },
        (LPARAM)&out);
    return out;
}

// The running programs with a window, by file name, each once, sorted.
std::vector<std::wstring> RunningPrograms() {
    std::vector<std::wstring> out;
    for (auto const& w : OpenWindows())
        if (!w.program.empty() && std::none_of(out.begin(), out.end(), [&](auto const& o) { return !_wcsicmp(o.c_str(), w.program.c_str()); }))
            out.push_back(w.program);
    std::sort(out.begin(), out.end(), [](auto const& a, auto const& b) { return _wcsicmp(a.c_str(), b.c_str()) < 0; });
    return out;
}

std::wstring BrowseForFile(const wchar_t* filter, const wchar_t* title, const std::wstring& start = L"") {
    WCHAR file[MAX_PATH] = L"";
    if (!start.empty() && start.size() < MAX_PATH) wcscpy(file, start.c_str());
    OPENFILENAMEW ofn{sizeof(ofn)};
    ofn.hwndOwner = g_wnd;
    ofn.lpstrFilter = filter;
    ofn.lpstrFile = file;
    ofn.nMaxFile = ARRAYSIZE(file);
    ofn.lpstrTitle = title;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
    return GetOpenFileNameW(&ofn) ? file : L"";
}

std::wstring BrowseForFolder(const wchar_t* title) {
    std::wstring out;
    ComPtr<IFileOpenDialog> dialog;
    if (FAILED(CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&dialog)))) return out;
    FILEOPENDIALOGOPTIONS options = 0;
    dialog->GetOptions(&options);
    dialog->SetOptions(options | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);
    dialog->SetTitle(title);
    ComPtr<IShellItem> item;
    PWSTR path = nullptr;
    if (SUCCEEDED(dialog->Show(g_wnd)) && SUCCEEDED(dialog->GetResult(&item)) && SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &path))) {
        out = path;
        CoTaskMemFree(path);
    }
    return out;
}

// Windows' own icon picker (any .ico, .exe or .dll), from what's set now: "file" or "file,index".
std::wstring PickIconFile(const std::wstring& now) {
    WCHAR file[MAX_PATH] = L"%SystemRoot%\\System32\\shell32.dll";
    int index = 0;
    if (!now.empty() && now.rfind(L"tourne:", 0) != 0) {
        std::wstring path = now;
        size_t comma = path.rfind(L',');
        if (comma != std::wstring::npos && comma + 1 < path.size() && (iswdigit(path[comma + 1]) || path[comma + 1] == L'-')) {
            index = _wtoi(path.c_str() + comma + 1);
            path.resize(comma);
        }
        if (path.size() < MAX_PATH) wcscpy(file, path.c_str());
    }
    if (!PickIconDlg(g_wnd, file, ARRAYSIZE(file), &index)) return L"";
    WCHAR expanded[MAX_PATH];
    if (!ExpandEnvironmentStringsW(file, expanded, ARRAYSIZE(expanded))) wcscpy(expanded, file);
    std::wstring chosen = expanded;
    size_t dot = chosen.rfind(L'.');
    bool ico = dot != std::wstring::npos && !_wcsicmp(chosen.c_str() + dot, L".ico");
    return ico ? chosen : chosen + L"," + std::to_wstring(index);
}

// Asks for a key press (with its modifiers): "ctrl+alt+77"; Backspace alone clears it, Esc keeps it as it was.
// Asks for keys in a small window of its own. Plain (one key with modifiers, as Windows' hotkeys take) unless
// rich: then any keys held together (Esc+End: the last one pressed is the one that runs it), and a menu after
// for how they're pressed: once, twice ("x2"), or held ("hold 800").
std::optional<std::wstring> CaptureHotkey(const std::wstring& what, bool rich) {
    static bool registered;
    static std::optional<std::wstring> result;
    static bool done, richNow;
    static std::wstring prompt;
    static std::vector<UINT> pressed;  // the keys other than modifiers, in the order they went down
    static UINT heldMods;
    if (!registered) {
        WNDCLASSW wc{};
        wc.lpfnWndProc = [](HWND w, UINT m, WPARAM wp, LPARAM lp) -> LRESULT {
            if (m == WM_PAINT) {
                PAINTSTRUCT ps;
                HDC dc = BeginPaint(w, &ps);
                RECT rc;
                GetClientRect(w, &rc);
                FillRect(dc, &rc, GetSysColorBrush(COLOR_WINDOW));
                InflateRect(&rc, -16, -16);
                SetBkMode(dc, TRANSPARENT);
                HGDIOBJ old = SelectObject(dc, GetStockObject(DEFAULT_GUI_FONT));
                DrawTextW(dc, prompt.c_str(), -1, &rc, DT_WORDBREAK);
                SelectObject(dc, old);
                EndPaint(w, &ps);
                return 0;
            }
            if (m == WM_CLOSE) {
                done = true;
                return 0;
            }
            auto modsNow = [] {
                UINT mods = 0;
                if (GetKeyState(VK_CONTROL) & 0x8000) mods |= MOD_CONTROL;
                if (GetKeyState(VK_MENU) & 0x8000) mods |= MOD_ALT;
                if (GetKeyState(VK_SHIFT) & 0x8000) mods |= MOD_SHIFT;
                if ((GetKeyState(VK_LWIN) & 0x8000) || (GetKeyState(VK_RWIN) & 0x8000)) mods |= MOD_WIN;
                return mods;
            };
            auto isMod = [](UINT vk) {
                switch (vk) {
                    case VK_CONTROL: case VK_LCONTROL: case VK_RCONTROL: case VK_MENU: case VK_LMENU: case VK_RMENU:
                    case VK_SHIFT: case VK_LSHIFT: case VK_RSHIFT: case VK_LWIN: case VK_RWIN:
                        return true;
                }
                return false;
            };
            auto finish = [](UINT mods, std::vector<UINT> const& keys) {
                bool plain = !mods && keys.size() == 1;
                if (plain && keys[0] == VK_ESCAPE) result.reset();
                else if (plain && keys[0] == VK_BACK) result = L"";
                else {
                    std::wstring s;
                    if (mods & MOD_CONTROL) s += L"ctrl+";
                    if (mods & MOD_ALT) s += L"alt+";
                    if (mods & MOD_SHIFT) s += L"shift+";
                    if (mods & MOD_WIN) s += L"win+";
                    for (size_t i = 0; i < keys.size(); i++) s += (i ? L"+" : L"") + std::to_wstring(keys[i]);
                    result = s;
                }
                done = true;
            };
            if (m == WM_KEYDOWN || m == WM_SYSKEYDOWN) {
                UINT vk = (UINT)wp;
                if (isMod(vk)) {
                    heldMods |= modsNow();
                    return 0;  // (waiting for the key itself)
                }
                if (!richNow) {
                    finish(modsNow(), {vk});
                    return 0;
                }
                heldMods |= modsNow();
                if (std::find(pressed.begin(), pressed.end(), vk) == pressed.end()) pressed.push_back(vk);
                return 0;
            }
            if ((m == WM_KEYUP || m == WM_SYSKEYUP) && richNow && !pressed.empty()) {
                // The first key let go ends it: what was held then is the combination.
                finish(heldMods, pressed);
                return 0;
            }
            return DefWindowProcW(w, m, wp, lp);
        };
        wc.hInstance = GetModuleHandleW(nullptr);
        wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        wc.lpszClassName = L"TourneStudioKeys";
        registered = RegisterClassW(&wc) != 0;
    }
    result.reset();
    done = false;
    richNow = rich;
    pressed.clear();
    heldMods = 0;
    prompt = rich ? L"Press the keys for " + what + L", all held together (like Ctrl+Alt+M, or Esc+End: hold Esc, press End).\n\n"
                    L"Next you choose a single press, a double press or a hold.\nBackspace on its own: none.   Esc on its own: keep it as it is."
                  : L"Press the keys for " + what + L", all at once (like Ctrl+Alt+M).\n\nBackspace on its own: none.   Esc: keep it as it is.";
    RECT owner;
    GetWindowRect(g_wnd, &owner);
    int w = (int)(440 * g_scale), h = (int)((rich ? 190 : 150) * g_scale);
    HWND box = CreateWindowExW(WS_EX_DLGMODALFRAME | WS_EX_TOPMOST, L"TourneStudioKeys", L"Press the keys", WS_POPUP | WS_CAPTION | WS_SYSMENU,
                               (owner.left + owner.right - w) / 2, (owner.top + owner.bottom - h) / 2, w, h, g_wnd, nullptr,
                               GetModuleHandleW(nullptr), nullptr);
    if (!box) return std::nullopt;
    EnableWindow(g_wnd, FALSE);
    ShowWindow(box, SW_SHOW);
    SetForegroundWindow(box);
    SetFocus(box);
    MSG msg;
    while (!done && GetMessageW(&msg, nullptr, 0, 0) > 0) {
        bool key = msg.message == WM_KEYDOWN || msg.message == WM_SYSKEYDOWN || msg.message == WM_KEYUP || msg.message == WM_SYSKEYUP;
        if (key && msg.hwnd != box) msg.hwnd = box;
        if (msg.message == WM_SYSKEYDOWN || msg.message == WM_SYSKEYUP || msg.message == WM_SYSCHAR) {  // (Alt+key: no menu beep, ours)
            if (msg.message != WM_SYSCHAR) SendMessageW(box, msg.message, msg.wParam, msg.lParam);
            continue;
        }
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    POINT at;
    GetCursorPos(&at);
    RECT shown;
    GetWindowRect(box, &shown);
    EnableWindow(g_wnd, TRUE);
    DestroyWindow(box);
    SetForegroundWindow(g_wnd);
    // How they're pressed.
    if (rich && result && !result->empty()) {
        HMENU menu = CreatePopupMenu();
        AppendMenuW(menu, MF_STRING | MF_DISABLED, 0, (HotkeyLabel(*result) + L": pressed how?").c_str());
        AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
        AppendMenuW(menu, MF_STRING, 1, L"Pressed once");
        AppendMenuW(menu, MF_STRING, 2, L"Double press");
        const int holds[] = {300, 500, 800, 1200, 2000};
        for (int i = 0; i < 5; i++) {
            WCHAR t[48];
            swprintf(t, 48, L"Held for %.1f s", holds[i] / 1000.0);
            AppendMenuW(menu, MF_STRING, 10 + i, t);
        }
        int pick = (int)TrackPopupMenu(menu, TPM_RETURNCMD | TPM_NONOTIFY, (shown.left + shown.right) / 2, (shown.top + shown.bottom) / 2, 0, g_wnd, nullptr);
        DestroyMenu(menu);
        if (!pick) return std::nullopt;
        if (pick == 2) *result += L" x2";
        else if (pick >= 10) *result += L" hold " + std::to_wstring(holds[pick - 10]);
    }
    (void)at;
    return result;
}

// A motion leaf's styles, from its own line or the nearest above that names them ("Windows: slide, popin, gnomed").
std::vector<std::wstring> MotionStyles(std::wstring leaf) {
    for (int guard = 0; guard < 8 && !leaf.empty(); guard++) {
        for (auto const& l : motion::Tree()) {
            if (leaf != l.name) continue;
            std::wstring about = l.about;
            size_t colon = about.find(L':');
            if (colon != std::wstring::npos) return SplitList(about.substr(colon + 1), L",");
            leaf = l.parent ? l.parent : L"";
            break;
        }
    }
    return {};
}

void PickFromMenu(int index) {
    Row const r = g_rows[index];
    PickInfo p = PickOf(r.key);
    if (p.type == Pick::MenuTree) {
        OpenMenuEditor();
        return;
    }
    if (p.type == Pick::BindsEditor) {
        OpenBindsEditor();
        return;
    }
    std::wstring now = ValueOf(r.key), def = DefaultOf(r.key), name = NameOf(r.key);
    HMENU menu = CreatePopupMenu();
    std::vector<std::wstring> answers;  // id - 1000: the value it sets
    auto answer = [&](HMENU m, const std::wstring& label, const std::wstring& value, bool checked = false, bool grayed = false) {
        answers.push_back(value);
        AppendMenuW(m, MF_STRING | (checked ? MF_CHECKED : 0) | (grayed ? MF_GRAYED : 0), 1000 + answers.size() - 1, label.c_str());
    };
    enum : int { kType = 1, kBrowse, kBrowse2, kFolder, kIcon, kFont, kKeys, kOpenFolder, kLiveFolder, kGradientOne, kGradientTwo };
    auto typeItem = [&](HMENU m, const wchar_t* label = L"Type it out…") {
        AppendMenuW(m, MF_SEPARATOR, 0, nullptr);
        AppendMenuW(m, MF_STRING, kType, label);
    };
    std::vector<std::wstring> items = SplitList(now, p.separator);
    auto without = [&](size_t i) {
        auto copy = items;
        copy.erase(copy.begin() + i);
        return JoinList(copy, p.separator);
    };
    auto with = [&](const std::wstring& item) {
        auto copy = items;
        copy.push_back(item);
        return JoinList(copy, p.separator);
    };
    auto listItems = [&](const wchar_t* empty) {
        if (items.empty()) AppendMenuW(menu, MF_STRING | MF_GRAYED, 0, empty);
        for (size_t i = 0; i < items.size(); i++) answer(menu, items[i] + L"\tTake out", without(i), true);
        AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    };
    switch (p.type) {
        case Pick::Programs: {
            listItems(L"(none in the list)");
            HMENU running = CreatePopupMenu();
            for (auto const& program : RunningPrograms())
                if (std::none_of(items.begin(), items.end(), [&](auto const& i) { return !_wcsicmp(i.c_str(), program.c_str()); }))
                    answer(running, program, with(program));
            AppendMenuW(menu, MF_POPUP, (UINT_PTR)running, L"Add a running program");
            AppendMenuW(menu, MF_STRING, kBrowse, L"Add one by finding its file…");
            if (!items.empty()) answer(menu, L"Clear the list", L"");
            typeItem(menu, L"Type the list out…");
            break;
        }
        case Pick::Titles:
        case Pick::Classes: {
            bool titles = p.type == Pick::Titles;
            listItems(titles ? L"(no titles in the list)" : L"(no classes in the list)");
            HMENU open = CreatePopupMenu();
            std::vector<std::wstring> seen;
            for (auto const& w : OpenWindows()) {
                std::wstring item = titles ? w.title : w.cls;
                if (item.empty() || std::find(seen.begin(), seen.end(), item) != seen.end()) continue;
                seen.push_back(item);
                std::wstring label = item.size() > 60 ? item.substr(0, 59) + L"…" : item;
                if (!titles) label += L"\t" + (w.title.size() > 30 ? w.title.substr(0, 29) + L"…" : w.title);
                answer(open, label, with(item));
            }
            AppendMenuW(menu, MF_POPUP, (UINT_PTR)open, titles ? L"Add an open window's title" : L"Add an open window's class");
            if (!items.empty()) answer(menu, L"Clear the list", L"");
            typeItem(menu, L"Type the list out… (* and ? match any text)");
            break;
        }
        case Pick::Program: {
            HMENU running = CreatePopupMenu();
            for (auto const& program : RunningPrograms()) answer(running, program, program, !_wcsicmp(program.c_str(), now.c_str()));
            AppendMenuW(menu, MF_POPUP, (UINT_PTR)running, L"A running program");
            AppendMenuW(menu, MF_STRING, kBrowse, L"Find its file…");
            AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
            answer(menu, r.key.rfind(L"inject.", 0) == 0 ? L"Found by itself (the default)" : L"None", def.empty() ? L"" : def, now == def);
            typeItem(menu, L"Type it out… (a file name, a full path or an app id)");
            break;
        }
        case Pick::Icon: {
            AppendMenuW(menu, MF_STRING, kIcon, L"Pick an icon… (from an .ico, .exe or .dll)");
            AppendMenuW(menu, MF_STRING, kBrowse, L"An .ico or picture file…");
            HMENU designs = CreatePopupMenu();
            static const wchar_t* const kDesigns[] = {
                L"phone", L"chat", L"bell", L"cloud-sync", L"system", L"search", L"wifi-4", L"ethernet", L"vpn", L"battery-full",
                L"audio-high", L"mic", L"brightness", L"night-light", L"bluetooth", L"usb", L"printer", L"keyboard", L"power",
                L"security-ok", L"firewall-on", L"update", L"gpu", L"sensor", L"airplane", L"focus", L"accessibility",
                L"app-bin", L"app-clipboard", L"app-capture", L"app-audio", L"app-cloud", L"app-system", L"app-claude",
                L"app-spotify", L"app-fan", L"app-tiling", L"app-screenshot", L"app-wallpaper", L"app-game"};
            for (auto d : kDesigns) answer(designs, d, std::wstring(L"tourne:") + d, now == std::wstring(L"tourne:") + d);
            AppendMenuW(menu, MF_POPUP, (UINT_PTR)designs, L"One of System Icons' designs (in your colours)");
            AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
            answer(menu, L"The default", def, now == def);
            typeItem(menu);
            break;
        }
        case Pick::Folder:
            AppendMenuW(menu, MF_STRING, kFolder, L"Choose a folder…");
            answer(menu, def.empty() ? L"None (the default)" : L"The default", def, now == def);
            typeItem(menu);
            break;
        case Pick::PlaceTarget:
            AppendMenuW(menu, MF_STRING, kFolder, L"A folder…");
            AppendMenuW(menu, MF_STRING, kBrowse, L"A program, shortcut or file…");
            answer(menu, L"Nothing", L"", now.empty());
            typeItem(menu, L"Type it out… (or a shell: name)");
            break;
        case Pick::File:
            AppendMenuW(menu, MF_STRING, kBrowse, L"A calendar file (.ics)…");
            answer(menu, L"None", L"", now.empty());
            typeItem(menu, L"Type a web address…");
            break;
        case Pick::Hotkey:
            AppendMenuW(menu, MF_STRING, kKeys, L"Press the keys…");
            answer(menu, L"None", L"", now.empty());
            typeItem(menu);
            break;
        case Pick::Font: {
            AppendMenuW(menu, MF_STRING, kFont, L"Choose a font…");
            HMENU pixel = CreatePopupMenu();
            for (auto f : {L"Scientifica", L"ProggyCleanTT", L"CozetteVector", L"Cozette", L"Segoe UI", L"Segoe UI Variable", L"Consolas"})
                answer(pixel, f, f, !_wcsicmp(now.c_str(), f));
            AppendMenuW(menu, MF_POPUP, (UINT_PTR)pixel, L"Suggested");
            answer(menu, L"The default", def, now == def);
            typeItem(menu);
            break;
        }
        case Pick::StyleSheets: {
            PWSTR roaming = nullptr;
            std::wstring folder;
            if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &roaming))) folder = std::wstring(roaming) + L"\\Tourne\\Desktop\\css";
            CoTaskMemFree(roaming);
            std::vector<std::wstring> sheets;
            for (auto sub : {L"", L"tourne\\"}) {
                WIN32_FIND_DATAW fd;
                HANDLE find = FindFirstFileW((folder + L"\\" + sub + L"*.css").c_str(), &fd);
                if (find == INVALID_HANDLE_VALUE) continue;
                do {
                    std::wstring sheet = std::wstring(sub) + fd.cFileName;
                    for (auto& c : sheet)
                        if (c == L'\\') c = L'/';
                    sheets.push_back(sheet);
                } while (FindNextFileW(find, &fd));
                FindClose(find);
            }
            for (auto const& sheet : sheets) {
                auto at = std::find_if(items.begin(), items.end(), [&](auto const& i) { return !_wcsicmp(i.c_str(), sheet.c_str()); });
                bool on = at != items.end();
                answer(menu, sheet, on ? without(at - items.begin()) : with(sheet), on);
            }
            if (sheets.empty()) AppendMenuW(menu, MF_STRING | MF_GRAYED, 0, L"(no style sheets in the folder yet)");
            AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
            AppendMenuW(menu, MF_STRING, kOpenFolder, L"Open the style sheets folder");
            answer(menu, L"The default", def, now == def);
            typeItem(menu);
            break;
        }
        case Pick::Command: {
            AppendMenuW(menu, MF_STRING, kBrowse, L"A program, shortcut or file…");
            AppendMenuW(menu, MF_STRING, kFolder, L"A folder…");
            HMENU actions = CreatePopupMenu();
            static const std::pair<const wchar_t*, const wchar_t*> kActions[] = {
                {L"tourne:start", L"Start menu"},          {L"tourne:run", L"Run…"},
                {L"tourne:show-desktop", L"Show the desktop"}, {L"tourne:task-manager", L"Task Manager"},
                {L"tourne:settings", L"Windows Settings"}, {L"tourne:studio", L"Tourne Studio"},
                {L"tourne:menu-settings", L"This menu's settings"}, {L"tourne:windhawk", L"Windhawk"},
                {L"tourne:launcher", L"Launcher"}, {L"tourne:desktop-next", L"Next desktop"}, {L"tourne:desktop-previous", L"Desktop before"},
                {L"tourne:desktop-new", L"New desktop"},   {L"tourne:desktop-close", L"Close this desktop"},
                {L"tourne:lock", L"Lock"},                 {L"tourne:sleep", L"Sleep"},
                {L"tourne:sign-out", L"Sign out"},         {L"tourne:restart", L"Restart"},
                {L"tourne:shutdown", L"Shut down"},        {L"tourne:close-litemenu", L"Close LiteMenu"}};
            for (auto const& [value, label] : kActions) answer(actions, label, value, now == value);
            AppendMenuW(menu, MF_POPUP, (UINT_PTR)actions, L"One of Tourne's actions");
            HMENU live = CreatePopupMenu();
            answer(live, L"Open windows", L"tourne:windows", now == L"tourne:windows");
            answer(live, L"Desktops", L"tourne:desktops", now == L"tourne:desktops");
            answer(live, L"All programs", L"tourne:programs", now == L"tourne:programs");
            answer(live, L"Recent items", L"tourne:recent", now == L"tourne:recent");
            AppendMenuW(live, MF_STRING, kLiveFolder, L"A folder's contents…");
            AppendMenuW(menu, MF_POPUP, (UINT_PTR)live, L"A submenu that fills itself");
            answer(menu, L"Nothing (a heading)", L"", now.empty());
            typeItem(menu, L"Type it out… (a web address, shell: name, app id)");
            break;
        }
        case Pick::Motion: {
            std::wstring leaf = r.key.substr(7);
            std::vector<std::wstring> parts = SplitList(now, L",");
            std::wstring speed = parts.size() > 1 ? parts[1] : L"5", curve = parts.size() > 2 ? parts[2] : L"default",
                         style = parts.size() > 3 ? parts[3] : L"";
            auto line = [](const std::wstring& sp, const std::wstring& cu, const std::wstring& st) {
                return L"1, " + sp + L", " + cu + (st.empty() ? L"" : L", " + st);
            };
            const wchar_t* parent = motion::ParentOf(leaf);
            answer(menu, parent ? std::wstring(L"As ") + parent + L" (the line above it)" : std::wstring(L"Each part's own"), L"", now.empty());
            answer(menu, L"Off (no animation)", L"0, 1, default", !parts.empty() && parts[0] == L"0");
            AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
            HMENU styles = CreatePopupMenu();
            answer(styles, L"None (just its curve)", line(speed, curve, L""), !parts.empty() && style.empty());
            for (auto const& s : MotionStyles(leaf)) answer(styles, s, line(speed, curve, s), style == s);
            AppendMenuW(menu, MF_POPUP, (UINT_PTR)styles, L"Style");
            HMENU curves = CreatePopupMenu();
            for (auto const& c : motion::BuiltIn()) answer(curves, c.name, line(speed, c.name, style), !parts.empty() && curve == c.name);
            AppendMenuW(menu, MF_POPUP, (UINT_PTR)curves, L"Curve");
            HMENU speeds = CreatePopupMenu();
            for (auto const& [tenths, label] : std::initializer_list<std::pair<const wchar_t*, const wchar_t*>>{
                     {L"2", L"Very quick (0.2 s)"}, {L"3", L"Quick (0.3 s)"}, {L"4", L"Brisk (0.4 s)"}, {L"5", L"Normal (0.5 s)"},
                     {L"7", L"Relaxed (0.7 s)"}, {L"10", L"Slow (1 s)"}, {L"15", L"Very slow (1.5 s)"}})
                answer(speeds, label, line(tenths, curve, style), !parts.empty() && speed == tenths);
            AppendMenuW(menu, MF_POPUP, (UINT_PTR)speeds, L"Speed");
            typeItem(menu, L"Type it out… (Hyprland's form: 1, 4, overshoot, slide)");
            break;
        }
        case Pick::Gradient: {
            AppendMenuW(menu, MF_STRING, kGradientOne, L"One colour…");
            AppendMenuW(menu, MF_STRING, kGradientTwo, L"A gradient of two colours…");
            answer(menu, L"Windows' accent colour", L"accent", now == L"accent");
            answer(menu, L"Windows' inactive accent colour", L"accentInactive", now == L"accentInactive");
            HMENU direction = CreatePopupMenu();
            std::vector<std::wstring> colours;
            for (auto const& word : SplitList(now, L" "))
                if (word.find(L',') == std::wstring::npos && word.find(L"deg") == std::wstring::npos) colours.push_back(word);
            for (auto const& [angle, label] : std::initializer_list<std::pair<const wchar_t*, const wchar_t*>>{
                     {L"0deg", L"Left to right"}, {L"90deg", L"Top to bottom"}, {L"45deg", L"Diagonal (top left to bottom right)"},
                     {L"135deg", L"Diagonal (top right to bottom left)"}})
                answer(direction, label, JoinList(colours, L" ") + L" " + angle, now.find(angle) != std::wstring::npos);
            AppendMenuW(menu, MF_POPUP | (colours.size() > 1 ? 0 : MF_GRAYED), (UINT_PTR)direction, L"The gradient's direction");
            if (r.key.find(L"inactive") != std::wstring::npos) answer(menu, L"No border on them", L"", now.empty());
            answer(menu, L"The default", def, now == def);
            typeItem(menu);
            break;
        }
        case Pick::MenuTree:  // (its own window, above)
        case Pick::BindsEditor:
        case Pick::None: break;
    }
    int id = PopMenuUnder(menu, r);
    DestroyMenu(menu);
    if (id >= 1000 && id - 1000 < (int)answers.size()) return SetValue(r.key, answers[id - 1000]);
    switch (id) {
        case kType: OpenEditor(index); break;
        case kBrowse: {
            const wchar_t* filter = p.type == Pick::File ? L"Calendars (*.ics)\0*.ics\0All files\0*.*\0"
                                    : p.type == Pick::Icon ? L"Icons and pictures\0*.ico;*.png;*.bmp\0All files\0*.*\0"
                                    : p.type == Pick::Programs || p.type == Pick::Program ? L"Programs (*.exe)\0*.exe\0All files\0*.*\0"
                                                                                         : L"All files\0*.*\0";
            std::wstring file = BrowseForFile(filter, (L"Tourne Studio: " + name).c_str());
            if (file.empty()) break;
            if (p.type == Pick::Programs) SetValue(r.key, with(FileNameOf(file)));
            else SetValue(r.key, file);
            break;
        }
        case kFolder: {
            std::wstring folder = BrowseForFolder((L"Tourne Studio: " + name).c_str());
            if (!folder.empty()) SetValue(r.key, folder);
            break;
        }
        case kLiveFolder: {
            std::wstring folder = BrowseForFolder(L"Tourne Studio: a folder whose contents the menu shows");
            if (!folder.empty()) SetValue(r.key, L"tourne:folder:" + folder);
            break;
        }
        case kIcon: {
            std::wstring icon = PickIconFile(now);
            if (!icon.empty()) SetValue(r.key, icon);
            break;
        }
        case kKeys: {
            // (a plugin's hotkey goes through Keys and rules: any keys, pressed twice or held)
            bool rich = r.key.rfind(L"plugins.", 0) == 0;
            if (auto keys = CaptureHotkey(name, rich)) SetValue(r.key, *keys);
            break;
        }
        case kFont: {
            LOGFONTW lf{};
            if (now.size() < LF_FACESIZE) wcscpy(lf.lfFaceName, now.c_str());
            lf.lfHeight = -(LONG)std::lround(14 * g_scale);
            CHOOSEFONTW cf{sizeof(cf)};
            cf.hwndOwner = g_wnd;
            cf.lpLogFont = &lf;
            cf.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT | CF_NOSIZESEL | CF_NOSTYLESEL | CF_NOSCRIPTSEL;
            if (ChooseFontW(&cf)) SetValue(r.key, lf.lfFaceName);
            break;
        }
        case kGradientOne:
        case kGradientTwo: {
            // Windows' colour picker, once or twice; the theme's colours are offered as its custom ones.
            static COLORREF custom[16];
            for (int i = 0; i < kThemeRoleCount && i < 16; i++) {
                uint32_t t;
                if (ThemeRoleColor(kThemeRoles[i], &t)) custom[i] = RGB(t >> 16, (t >> 8) & 255, t & 255);
            }
            std::vector<std::wstring> picked;
            for (int n = 0; n < (id == kGradientTwo ? 2 : 1); n++) {
                CHOOSECOLORW cc{sizeof(cc)};
                cc.hwndOwner = g_wnd;
                cc.lpCustColors = custom;
                cc.Flags = CC_FULLOPEN | CC_ANYCOLOR;
                if (!ChooseColorW(&cc)) return;
                picked.push_back(Hex(GetRValue(cc.rgbResult) << 16 | GetGValue(cc.rgbResult) << 8 | GetBValue(cc.rgbResult), true));
            }
            SetValue(r.key, JoinList(picked, L" ") + (picked.size() > 1 ? L" 45deg" : L""));
            break;
        }
        case kOpenFolder: {
            PWSTR roaming = nullptr;
            if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &roaming))) {
                std::wstring folder = std::wstring(roaming) + L"\\Tourne\\Desktop\\css";
                SHCreateDirectoryExW(nullptr, folder.c_str(), nullptr);
                ShellExecuteW(nullptr, nullptr, folder.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
            }
            CoTaskMemFree(roaming);
            break;
        }
    }
}

void SliderTo(int index, float x, bool live) {
    Row const& r = g_rows[index];
    const SettingInfo* info = InfoOf(r.key);
    D2D1_RECT_F t = TrackRect(r);
    double f = std::clamp((x - t.left) / (t.right - t.left), 0.f, 1.f);
    int v = (int)std::lround(info->min + f * (info->max - info->min));
    std::wstring text = std::to_wstring(v);
    if (text == ValueOf(r.key)) return;
    // While dragging, shown here at once and in the module a moment later
    // (one step for the whole drag on undo).
    if (live) {
        g_model.changes[r.key] = Change{false, text};
        SetTimer(g_wnd, kTimerLive, 60, nullptr);
    } else {
        SetValue(r.key, text);
    }
    InvalidateRect(g_wnd, nullptr, FALSE);
}

std::optional<Change> g_dragBefore;

void Click(Hit h, float x) {
    if (h.part == Part::Item) {
        CloseEditor(true);
        if (!g_query.empty()) SetWindowTextW(g_search, L"");
        if (g_page != h.index) g_filter = 0;
        g_page = h.index;
        Rebuild(true);
        return;
    }
    if (h.part == Part::Filter) {
        g_filter = h.index;
        return Rebuild(true);
    }
    if (h.part == Part::ExportLook) {
        WCHAR file[MAX_PATH] = L"my-look.tourne";
        OPENFILENAMEW ofn{sizeof(ofn)};
        ofn.hwndOwner = g_wnd;
        ofn.lpstrFilter = L"A Tourne look (*.tourne)\0*.tourne\0";
        ofn.lpstrDefExt = L"tourne";
        ofn.lpstrFile = file;
        ofn.nMaxFile = ARRAYSIZE(file);
        ofn.lpstrTitle = L"Export my look";
        ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
        if (!GetSaveFileNameW(&ofn)) return;
        std::wstring name = file, error;
        name = name.substr(name.find_last_of(L"\\/") + 1);
        name = name.substr(0, name.rfind(L'.'));
        if (Pending()) MessageBoxW(g_wnd, L"Changes not saved yet aren't in it: it's your look as saved.", L"Tourne Studio", MB_ICONINFORMATION);
        if (!DotsExport(file, name, &error)) MessageBoxW(g_wnd, (L"Not exported: " + error + L".").c_str(), L"Tourne Studio", MB_ICONWARNING);
        return;
    }
    if (h.part == Part::ImportLook) {
        WCHAR file[MAX_PATH] = L"";
        OPENFILENAMEW ofn{sizeof(ofn)};
        ofn.hwndOwner = g_wnd;
        ofn.lpstrFilter = L"A look (*.tourne, hyprland.conf)\0*.tourne;*.conf\0Anything\0*.*\0";
        ofn.lpstrFile = file;
        ofn.nMaxFile = ARRAYSIZE(file);
        ofn.lpstrTitle = L"Import a look: a .tourne file or a hyprland.conf";
        ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
        if (!GetOpenFileNameW(&ofn)) return;
        DotsLook look;
        std::wstring error;
        if (!DotsRead(file, &look, &error)) {
            MessageBoxW(g_wnd, (L"Nothing imported: " + error + L".").c_str(), L"Tourne Studio", MB_ICONWARNING);
            return;
        }
        // What it changes, by page, before anything does.
        std::map<std::wstring, int> byPage;
        int changes = 0;
        for (auto const& [key, value] : look.settings)
            if (!Same(key, value, ValueOf(key))) {
                int page = PageOf(key);
                byPage[page >= 0 ? g_pages[page].title : key.substr(0, key.find(L'.'))]++;
                changes++;
            }
        std::wstring ask = L"Import " + look.name + (look.author.empty() ? L"" : L" by " + look.author) + L" (from " + look.from + L")?\n\n";
        ask += std::to_wstring(changes) + L" settings change";
        for (auto const& [page, n] : byPage) ask += L"\n   " + page + L": " + std::to_wstring(n);
        if (!look.extensions.empty()) ask += L"\n" + std::to_wstring(look.extensions.size()) + L" extensions are added (looks only).";
        if (!look.skipped.empty()) {
            ask += L"\n\nLeft out (only looks come in):";
            for (size_t i = 0; i < look.skipped.size() && i < 6; i++) ask += L"\n   " + look.skipped[i];
            if (look.skipped.size() > 6) ask += L"\n   and " + std::to_wstring(look.skipped.size() - 6) + L" more";
        }
        ask += L"\n\nNothing is saved until you press Save, and Undo takes it back. A copy of your settings as they are now is kept "
               L"as tourne-gui.conf.before-import.";
        if (MessageBoxW(g_wnd, ask.c_str(), L"Tourne Studio: import a look", MB_OKCANCEL | MB_ICONQUESTION) != IDOK) return;
        CopyFileW((ConfigFolder() + L"\\tourne-gui.conf").c_str(), (ConfigFolder() + L"\\tourne-gui.conf.before-import").c_str(), FALSE);
        for (size_t i = 0; i < look.extensions.size(); i++) {
            WCHAR temp[MAX_PATH];
            GetTempPathW(MAX_PATH, temp);
            std::wstring dir = std::wstring(temp) + L"tourne-dots-" + std::to_wstring(GetCurrentProcessId()) + L"-" + std::to_wstring(i);
            CreateDirectoryW(dir.c_str(), nullptr);
            HANDLE f = CreateFileW((dir + L"\\extension.json").c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, 0, nullptr);
            if (f != INVALID_HANDLE_VALUE) {
                DWORD wrote;
                WriteFile(f, look.extensions[i].data(), (DWORD)look.extensions[i].size(), &wrote, nullptr);
                CloseHandle(f);
                std::wstring why;
                ExtensionAdd(dir, &why);
                DeleteFileW((dir + L"\\extension.json").c_str());
            }
            RemoveDirectoryW(dir.c_str());
        }
        std::vector<std::pair<std::wstring, std::optional<Change>>> list;
        for (auto const& [key, value] : look.settings)
            list.push_back({key, Same(key, value, SavedValue(key)) ? std::nullopt : std::optional<Change>(Change{false, value})});
        RecordMany(list);
        return Rebuild(false);
    }
    if (h.part == Part::AddExtension) {
        WCHAR file[MAX_PATH] = L"";
        OPENFILENAMEW ofn{sizeof(ofn)};
        ofn.hwndOwner = g_wnd;
        ofn.lpstrFilter = L"An extension (extension.json)\0extension.json\0";
        ofn.lpstrFile = file;
        ofn.nMaxFile = ARRAYSIZE(file);
        ofn.lpstrTitle = L"Add an extension: its extension.json";
        ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
        if (!GetOpenFileNameW(&ofn)) return;
        std::wstring error;
        if (!ExtensionAdd(file, &error))
            MessageBoxW(g_wnd, (L"That extension wasn't added: " + error + L".").c_str(), L"Tourne Studio", MB_ICONWARNING);
        return Rebuild(false);
    }
    if (h.part == Part::TileButton && h.index >= 0 && h.index < (int)g_tiles.size() && g_tiles[h.index] < 0) {
        // An extension: on writes its settings, off puts them back to Tourne's own (both wait for Save).
        Extension const& e = ExtensionList()[-1 - g_tiles[h.index]];
        bool on = ExtensionOn(e);
        std::vector<std::pair<std::wstring, std::optional<Change>>> changes;
        for (auto const& [key, value] : e.settings) {
            if (on) changes.push_back({key, Find(g_model.gui, key) ? std::optional<Change>(Change{true, L""}) : std::nullopt});
            else changes.push_back({key, Same(key, value, SavedValue(key)) ? std::nullopt : std::optional<Change>(Change{false, value})});
        }
        RecordMany(changes);
        return Rebuild(false);
    }
    if (h.part == Part::TileButton) {
        if (h.index < 0 || h.index >= (int)g_tiles.size()) return;
        PluginInfo const& p = PluginCatalogue()[g_tiles[h.index]];
        PluginState state = PluginStateOf(p);
        if (state == PluginState::Installing) return;
        if (state == PluginState::Installed) {
            std::wstring ask = L"Remove " + p.name + L"?\n\nIts folder goes, with anything it saved:\n" + PluginFolder(p);
            if (MessageBoxW(g_wnd, ask.c_str(), L"Tourne Studio", MB_OKCANCEL | MB_ICONQUESTION | MB_DEFBUTTON2) == IDOK) {
                PluginRemove(p);
                PluginRuntimeReload();  // (it stops running)
                RefreshPluginSettings();
            }
            return Rebuild(false);
        }
        // Asked first, with everything it will be allowed: that's all the runtime lets it do.
        std::wstring ask = L"Install " + p.name + L"?\n\n" + p.author + L" · " + (p.license.empty() ? L"no licence given" : p.license) +
                           L" · " + p.version + L"\n" + p.description + L"\n\n";
        std::wstring shows;
        for (auto const& s : PluginSurfaces(p)) shows += (shows.empty() ? L"" : L", ") + s;
        if (!shows.empty()) ask += L"Shows in: " + shows + L"\n\n";
        auto needs = PluginNeedsList(p);
        if (needs.empty()) {
            ask += L"It needs nothing: no internet, no files, no programs.\n";
        } else {
            ask += L"It will be allowed to:\n";
            for (auto const& n : needs) ask += L"  •  " + n + L"\n";
        }
        ask += L"\nIts " + std::to_wstring(p.files.size()) + L" files come from github.com/" + p.repo + L" at " + p.rev.substr(0, 7) +
               L", exactly as Tourne checked them, into\n" + PluginFolder(p);
        if (MessageBoxW(g_wnd, ask.c_str(), L"Tourne Studio: install a plugin", MB_OKCANCEL | MB_ICONQUESTION) != IDOK) return;
        PluginInstall(p, g_wnd, kMsgPluginsChanged);
        return Rebuild(false);
    }
    if (h.part == Part::Tile) {
        if (h.index >= 0 && h.index < (int)g_tiles.size()) ChoosePreset(Presets()[g_tiles[h.index]]);
        return Relayout();
    }
    if (h.part == Part::ViewCard) {
        if (h.index >= 0 && h.index < kViewCount) {
            std::wstring want = kViewCards[h.index].id, key = L"switcher.view.style";
            Record(key, Same(key, want, SavedValue(key)) ? std::nullopt : std::optional<Change>(Change{false, want}));
        }
        return Relayout();
    }
    if (h.part == Part::ChipSet) {
        if (auto sets = PageSets(); sets && h.index >= 0 && h.index < (int)sets->size()) ChooseSet(*sets, (*sets)[h.index]);
        return Relayout();
    }
    if (h.part == Part::TestWindow) return GlassTestWindow(!GlassTestWindowShown()), InvalidateRect(g_wnd, nullptr, FALSE), void();
    if (h.part == Part::Undo) return Undo(), Rebuild(false);
    if (h.part == Part::Redo) return Redo(), Rebuild(false);
    if (h.part == Part::Discard) return Discard(), Rebuild(false);
    if (h.part == Part::Save) {
        CloseEditor(true);
        if (!Pending()) return;
        if (!Save()) MessageBoxW(g_wnd, L"Tourne Studio couldn't write tourne-gui.conf.", L"Tourne Studio", MB_ICONWARNING);
        if (g_page == PendingPage()) g_page = 0;
        return Rebuild(false);
    }
    if (h.index < 0 || h.index >= (int)g_rows.size()) return;
    Row const& r = g_rows[h.index];
    std::wstring key = r.key;
    if (h.part == Part::Reset) {
        if (g_model.changes.count(key)) Record(key, std::nullopt);
        else Reset(key);
        return Relayout();
    }
    if (h.part == Part::Track) return;  // (handled as a drag)
    switch (r.kind) {
        case Kind::Switch: SetValue(key, IsOn(ValueOf(key)) ? L"false" : L"true"); break;
        case Kind::Choice: ChooseFromMenu(h.index); break;
        case Kind::Pick: PickFromMenu(h.index); break;
        case Kind::Colour: ChooseColour(h.index); break;
        case Kind::Slider:
        case Kind::Number:
        case Kind::Text: OpenEditor(h.index); return;
    }
    // A change can be the last one not saved, or the first: the sidebar and
    // the bar come and go.
    if (g_page == PendingPage() && g_query.empty()) Rebuild(false);
    else Relayout();
}

bool AskToSave() {
    if (!Pending()) return true;
    size_t n = g_model.changes.size();
    std::wstring text = L"Save your " + std::to_wstring(n) + (n == 1 ? L" change" : L" changes") +
                        L"?\n\nNot saving puts everything back as it was saved.";
    int answer = MessageBoxW(g_wnd, text.c_str(), L"Tourne Studio", MB_YESNOCANCEL | MB_ICONQUESTION);
    if (answer == IDCANCEL) return false;
    if (answer == IDYES && !Save()) {
        MessageBoxW(g_wnd, L"Tourne Studio couldn't write tourne-gui.conf.", L"Tourne Studio", MB_ICONWARNING);
        return false;
    }
    if (answer == IDNO) Discard();
    return true;
}

////////////////////////////////////////////////////////////////////////////////

bool MakeTarget() {
    if (g_rt) return true;
    RECT rc;
    GetClientRect(g_wnd, &rc);
    D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties();
    props.dpiX = props.dpiY = 96 * g_scale;
    if (FAILED(g_d2d->CreateHwndRenderTarget(props, D2D1::HwndRenderTargetProperties(g_wnd, D2D1::SizeU(rc.right, rc.bottom)), &g_rt)))
        return false;
    g_rt->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
    g_rt->CreateSolidColorBrush(Color(0), &g_brush);
    return true;
}

LRESULT CALLBACK StudioProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == g_themeChanged && g_themeChanged) {
        LoadColours();
        InvalidateRect(wnd, nullptr, TRUE);
        return 0;
    }
    switch (msg) {
        case WM_CREATE: {
            g_wnd = wnd;
            g_scale = GetDpiForWindow(wnd) / 96.f;
            g_search = CreateWindowExW(0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 0, 0, 0, 0, wnd, (HMENU)(INT_PTR)kSearchId,
                                       nullptr, nullptr);
            g_edit = CreateWindowExW(0, L"EDIT", L"", WS_CHILD | ES_AUTOHSCROLL, 0, 0, 0, 0, wnd, (HMENU)(INT_PTR)kEditId, nullptr, nullptr);
            SendMessageW(g_search, EM_SETCUEBANNER, TRUE, (LPARAM)L"Search every setting");
            SetWindowSubclass(g_search, EditProc, kSearchId, 0);
            SetWindowSubclass(g_edit, EditProc, kEditId, 0);
            MakeEditFont();
            PlaceSearch();
            LoadColours();
            return 0;
        }
        case WM_SIZE:
            if (g_rt) g_rt->Resize(D2D1::SizeU(LOWORD(lp), HIWORD(lp)));
            CloseEditor(true);
            g_glassValid = false;
            Relayout();
            return 0;
        case WM_MOVE:
            // What's behind its glass changed.
            if (g_glass) StudioGlassChanged();
            return 0;
        case WM_GETMINMAXINFO: {
            auto* mm = (MINMAXINFO*)lp;
            float s = GetDpiForWindow(wnd) / 96.f;
            mm->ptMinTrackSize = {(LONG)(760 * s), (LONG)(480 * s)};
            return 0;
        }
        case WM_DPICHANGED: {
            g_scale = HIWORD(wp) / 96.f;
            auto* r = (RECT*)lp;
            g_rt.Reset();
            g_glass.Reset(), g_glassValid = false;
            g_brush.Reset();
            SetWindowPos(wnd, nullptr, r->left, r->top, r->right - r->left, r->bottom - r->top, SWP_NOZORDER | SWP_NOACTIVATE);
            MakeEditFont();
            PlaceSearch();
            Relayout();
            return 0;
        }
        case WM_ERASEBKGND: return 1;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            BeginPaint(wnd, &ps);
            if (MakeTarget()) Paint();
            EndPaint(wnd, &ps);
            return 0;
        }
        case WM_CTLCOLOREDIT: {
            HDC dc = (HDC)wp;
            SetTextColor(dc, RGB(g_c.text >> 16, (g_c.text >> 8) & 255, g_c.text & 255));
            uint32_t bg = (HWND)lp == g_edit ? Mix(g_c.ground, g_c.text, 0.05f) : g_c.raised;
            SetBkColor(dc, RGB(bg >> 16, (bg >> 8) & 255, bg & 255));
            static HBRUSH editBg;
            if ((HWND)lp == g_edit) {
                if (editBg) DeleteObject(editBg);
                editBg = CreateSolidBrush(RGB(bg >> 16, (bg >> 8) & 255, bg & 255));
                return (LRESULT)editBg;
            }
            return (LRESULT)g_editBrush;
        }
        case WM_COMMAND:
            if (LOWORD(wp) == kSearchId && HIWORD(wp) == EN_CHANGE) {
                int n = GetWindowTextLengthW(g_search);
                std::wstring q(n, L'\0');
                GetWindowTextW(g_search, q.data(), n + 1);
                q.erase(0, q.find_first_not_of(L" "));
                q.erase(q.find_last_not_of(L" ") + 1);
                if (q != g_query) {
                    CloseEditor(true);
                    if (g_query.empty()) g_queryPresets = PresetsPage();  // (started on the presets: it filters them)
                    if (q.empty()) g_queryPresets = false;
                    g_query = q;
                    if (g_query.empty() && g_page > PendingPage()) g_page = 0;
                    Rebuild(true);
                }
            }
            return 0;
        case WM_MOUSEWHEEL: {
            CloseEditor(true);
            ScrollTo(g_scroll - GET_WHEEL_DELTA_WPARAM(wp) / 120.f * 64);
            InvalidateRect(wnd, nullptr, FALSE);
            return 0;
        }
        case WM_MOUSEMOVE: {
            float x = GET_X_LPARAM(lp) / g_scale, y = GET_Y_LPARAM(lp) / g_scale;
            if (g_pressed.part == Part::ScrollThumb) {
                float view = ViewBottom(), h = std::max(32.f, view * view / g_contentHeight);
                if (view > h) ScrollTo(g_scrollFrom + (y - g_dragFrom) * MaxScroll() / (view - h));
                InvalidateRect(wnd, nullptr, FALSE);
                return 0;
            }
            if (g_pressed.part == Part::Track) return SliderTo(g_pressed.index, x, true), 0;
            Hit h = HitTest(x, y);
            if (MotionPage()) {  // the large curve follows the line pointed at
                for (auto const& r : g_rows) {
                    float top = r.y - g_scroll;
                    if (!r.section && IsMotionLeaf(r.key) && y >= top && y < top + r.h && r.key.substr(7) != g_motionLeaf) {
                        g_motionLeaf = r.key.substr(7);
                        g_motionStart = GetTickCount64();
                    }
                }
            }
            if (!(h == g_hover)) {
                g_hover = h;
                InvalidateRect(wnd, nullptr, FALSE);
            }
            TRACKMOUSEEVENT tme{sizeof(tme), TME_LEAVE, wnd};
            TrackMouseEvent(&tme);
            SetCursor(LoadCursorW(nullptr, h.part != Part::None ? IDC_HAND : IDC_ARROW));
            return 0;
        }
        case WM_SETCURSOR:
            if (LOWORD(lp) == HTCLIENT && g_hover.part != Part::None) return SetCursor(LoadCursorW(nullptr, IDC_HAND)), TRUE;
            break;
        case WM_MOUSELEAVE:
            g_hover = {};
            InvalidateRect(wnd, nullptr, FALSE);
            return 0;
        case WM_LBUTTONDOWN: {
            float x = GET_X_LPARAM(lp) / g_scale, y = GET_Y_LPARAM(lp) / g_scale;
            Hit h = HitTest(x, y);
            if (GetFocus() != g_search || x >= kSidebar) SetFocus(wnd);
            if (h.part == Part::Compare) {
                g_pressed = h;
                SetCapture(wnd);
                GlassCompare(true);
                InvalidateRect(wnd, nullptr, FALSE);
                return 0;
            }
            if (h.part == Part::ScrollThumb || h.part == Part::Track) {
                CloseEditor(true);
                g_pressed = h;
                g_dragFrom = y, g_scrollFrom = g_scroll;
                SetCapture(wnd);
                if (h.part == Part::Track) {
                    g_dragBefore = ChangeOf(g_rows[h.index].key);
                    SliderTo(h.index, x, true);
                }
                return 0;
            }
            if (x < kSidebar && y >= 56 && y < 88) return SetFocus(g_search), 0;
            Click(h, x);
            return 0;
        }
        case WM_LBUTTONUP:
        case WM_CAPTURECHANGED: {
            Hit pressed = g_pressed;
            g_pressed = {};
            if (pressed.part == Part::Compare) {
                GlassCompare(false);
                InvalidateRect(wnd, nullptr, FALSE);
            }
            if (pressed.part == Part::Track && pressed.index < (int)g_rows.size()) {
                // The drag as one step.
                std::wstring key = g_rows[pressed.index].key, v = ValueOf(key);
                KillTimer(wnd, kTimerLive);
                Put(key, g_dragBefore);
                SetValue(key, v);
                Rebuild(false);
            }
            if (msg == WM_LBUTTONUP && pressed.part != Part::None) ReleaseCapture();
            return 0;
        }
        case kMsgPluginsChanged:
            PluginRuntimeReload();  // a plugin just installed starts running
            RefreshPluginSettings();
            Rebuild(false);
            return 0;
        case WM_TIMER:
            if (wp == kTimerLive) {
                KillTimer(wnd, kTimerLive);
                Live();
            }
            if (wp == kTimerMotion) {
                if ((!MotionPage() && !ViewsPage()) || IsIconic(wnd)) {
                    KillTimer(wnd, kTimerMotion);
                    g_motionTimer = false;
                } else {
                    InvalidateRect(wnd, nullptr, FALSE);
                }
            }
            return 0;
        case WM_KEYDOWN: {
            bool ctrl = GetKeyState(VK_CONTROL) < 0, shift = GetKeyState(VK_SHIFT) < 0;
            if (ctrl && wp == 'S' && Pending()) Click({Part::Save}, 0);
            else if (ctrl && (wp == 'Y' || (wp == 'Z' && shift))) Redo(), Rebuild(false);
            else if (ctrl && wp == 'Z') Undo(), Rebuild(false);
            else if (ctrl && wp == 'F') SetFocus(g_search), SendMessageW(g_search, EM_SETSEL, 0, -1);
            else if (wp == VK_NEXT || wp == VK_PRIOR) ScrollTo(g_scroll + (wp == VK_NEXT ? 1 : -1) * (ViewBottom() - 80));
            else if (wp == VK_HOME) ScrollTo(0);
            else if (wp == VK_END) ScrollTo(MaxScroll());
            else if (wp == VK_DOWN || wp == VK_UP) ScrollTo(g_scroll + (wp == VK_DOWN ? 48 : -48));
            InvalidateRect(wnd, nullptr, FALSE);
            return 0;
        }
        case WM_SETTINGCHANGE:
            if (lp && !wcscmp((const wchar_t*)lp, L"ImmersiveColorSet")) {
                LoadColours();
                InvalidateRect(wnd, nullptr, FALSE);
            }
            return 0;
        case WM_CLOSE:
            CloseEditor(true);
            if (AskToSave()) DestroyWindow(wnd);
            return 0;
        case WM_DESTROY:
            KillTimer(wnd, kTimerLive);
            KillTimer(wnd, kTimerMotion);
            g_motionTimer = false;
            g_rt.Reset();
            g_glass.Reset(), g_glassValid = false;
            g_brush.Reset();
            g_wnd = g_search = g_edit = nullptr;
            g_editing.clear();
            g_rows.clear();
            g_hover = g_pressed = {};
            return 0;
    }
    return DefWindowProcW(wnd, msg, wp, lp);
}

}  // namespace

void StudioGlassChanged() {
    g_glassValid = false;
    if (g_wnd) InvalidateRect(g_wnd, nullptr, FALSE);
}

void StudioOpen(HWND host) {
    if (g_wnd) {
        if (IsIconic(g_wnd)) ShowWindow(g_wnd, SW_RESTORE);
        SetForegroundWindow(g_wnd);
        return;
    }
    g_host = host;
    RefreshDynamicOptions();
    RefreshPluginChoices();
    if (!g_d2d && FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, g_d2d.GetAddressOf()))) return;
    if (!g_dw && FAILED(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)g_dw.GetAddressOf())))
        return;
    MakeFormats();
    MenusFollowWindows();
    g_themeChanged = RegisterWindowMessageW(L"TourneThemeChanged");
    static bool registered;
    if (!registered) {
        WNDCLASSW wc{};
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = StudioProc;
        wc.hInstance = GetModuleHandleW(nullptr);
        wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        wc.hIcon = LoadIconW(wc.hInstance, MAKEINTRESOURCEW(1));
        wc.lpszClassName = kClass;
        registered = RegisterClassW(&wc) != 0;
    }
    ConfigLayers(&g_model.user, &g_model.gui);
    MakePages();
    g_page = std::min(g_page, (int)g_pages.size() - 1);
    g_query.clear();
    g_scroll = 0;
    BuildRows();
    // A size that suits the screen it opens on.
    POINT pt;
    GetCursorPos(&pt);
    HMONITOR mon = MonitorFromPoint(pt, MONITOR_DEFAULTTOPRIMARY);
    MONITORINFO mi{sizeof(mi)};
    GetMonitorInfoW(mon, &mi);
    UINT dpiX = 96, dpiY = 96;
    using GetDpiForMonitorFn = HRESULT(WINAPI*)(HMONITOR, int, UINT*, UINT*);
    if (HMODULE shcore = LoadLibraryExW(L"shcore.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32))
        if (auto get = (GetDpiForMonitorFn)(void*)GetProcAddress(shcore, "GetDpiForMonitor")) get(mon, 0, &dpiX, &dpiY);
    float s = dpiX / 96.f;
    RECT work = mi.rcWork;
    int w = std::min((int)(1080 * s), (int)(work.right - work.left) - 40), h = std::min((int)(760 * s), (int)(work.bottom - work.top) - 40);
    int x = work.left + (work.right - work.left - w) / 2, y = work.top + (work.bottom - work.top - h) / 2;
    HWND wnd = CreateWindowExW(0, kClass, L"Tourne Studio (build " TD_BUILD L")", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, x, y, w, h, nullptr, nullptr,
                               GetModuleHandleW(nullptr), nullptr);
    if (!wnd) return;
    LayoutRows();
    ShowWindow(wnd, SW_SHOW);
    SetForegroundWindow(wnd);
}

// Every setting with the control the Studio gives it, its name, default and description, tab-separated, for
// checking that nothing with a set of answers shows a text box (TourneDesktop.exe --studio-kinds <file>).
bool StudioWriteKinds(const std::wstring& path) {
    static const wchar_t* const kNames[] = {L"switch", L"choice", L"slider", L"number", L"colour", L"text", L"pick"};  // (Kind's order)
    std::wstring out = L"key\tkind\tname\tdefault\toptions\tdescription\n";
    for (auto const& [key, def] : DefaultSettings()) {
        const SettingInfo* info = InfoOf(key);
        std::wstring options;
        if (info)
            for (auto const& o : info->options) options += (options.empty() ? L"" : L"|") + o.first;
        std::wstring description = info ? info->description : L"";
        for (auto& c : description)
            if (c == L'\t' || c == L'\n' || c == L'\r') c = L' ';
        out += key + L"\t" + kNames[(int)KindOf(key)] + L"\t" + (info ? info->name : L"") + L"\t" + def + L"\t" + options + L"\t" +
               description + L"\n";
    }
    int n = WideCharToMultiByte(CP_UTF8, 0, out.c_str(), (int)out.size(), nullptr, 0, nullptr, nullptr);
    std::string bytes(n, '\0');
    WideCharToMultiByte(CP_UTF8, 0, out.c_str(), (int)out.size(), bytes.data(), n, nullptr, nullptr);
    HANDLE file = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, 0, nullptr);
    if (file == INVALID_HANDLE_VALUE) return false;
    DWORD written = 0;
    bool ok = WriteFile(file, bytes.data(), (DWORD)bytes.size(), &written, nullptr);
    CloseHandle(file);
    return ok;
}

// Open at a page ("taskbar."), or with the search box set to what's asked for ("start menu") when that's given.
void StudioOpenAt(HWND host, const std::wstring& page, const std::wstring& query) {
    StudioOpen(host);
    if (!g_wnd) return;
    CloseEditor(true);
    if (!query.empty()) {
        SetWindowTextW(g_search, query.c_str());  // (its change notice filters the rows)
        return;
    }
    if (!g_query.empty()) SetWindowTextW(g_search, L"");
    for (int i = 0; i < (int)g_pages.size(); i++)
        if (g_pages[i].prefix == page) {
            if (g_page != i) g_filter = 0;
            g_page = i;
        }
    Rebuild(true);
}

void StudioConfigReloaded() {
    if (!g_wnd) return;
    ConfigLayers(&g_model.user, &g_model.gui);
    // A change that's now what the files say isn't one any more.
    for (auto it = g_model.changes.begin(); it != g_model.changes.end();) {
        bool saved = it->second.remove ? !Find(g_model.gui, it->first) : Same(it->first, it->second.value, SavedValue(it->first));
        it = saved ? g_model.changes.erase(it) : std::next(it);
    }
    Rebuild(false);
}

bool StudioClose() {
    if (!g_wnd) return true;
    CloseEditor(true);
    if (!AskToSave()) return false;
    DestroyWindow(g_wnd);
    return true;
}

}  // namespace td
