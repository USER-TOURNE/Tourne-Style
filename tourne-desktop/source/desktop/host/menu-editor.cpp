// The Tourne Menu's settings, laid out like LiteMenu's own configurator, so a menu with submenus in
// submenus reads as the tree it is (the Studio could only list its rows flat: depth, name, command).
//
//   Menu tab      the menu as a tree; Add item / Add submenu / Add separator / Remove / Move up / Move
//                 down under it; drag an item onto a submenu to move it in, or above or below an item to
//                 move it there. Beside it, the picked item's Properties: its type, Display name, Command
//                 (with "..." for a program, a folder, one of Tourne's actions or a submenu that fills
//                 itself) and Parameters.
//   Settings tab  Activation (on or off, the mouse button, the bar's empty space too, the hotkey) and Look
//                 (Tourne's menu look, in place of LiteMenu's own colours).
//
// OK and Apply write taskbar.menu.* to tourne-gui.conf as the Studio does (the rows: depth, name, command,
// args), and the config watch hands them to the bar. With no rows yet it opens on LiteMenu's own menu (its
// config.xml), which OK then copies in; "Copy LiteMenu's menu" does that again later.
// It runs on a thread of its own, so Tab steps through the boxes (IsDialogMessage) whatever the host does.

#include "menu-editor.h"

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <commdlg.h>
#include <dwmapi.h>
#include <shlobj.h>
#include <shobjidl.h>

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

struct Node {
    int type = 1;  // 0 a submenu, 1 an item, 2 a separator (LiteMenu's MenuItem types)
    std::wstring name, command, args;
};

struct MenuRow {
    int depth = 0;
    std::wstring name, command, args;
};

constexpr wchar_t kClass[] = L"TourneMenuSettings";
constexpr wchar_t kItems[] = L"taskbar.menu.items[";
constexpr wchar_t kLiteMenuPackage[] = L"RedBaronDevelopment.PopEx_36fx7gy0frrbc";

enum : int {
    idTabs = 100, idTree, idAddItem, idAddSub, idAddSep, idRemove, idUp, idDown,
    idProps, idTypeLabel, idType, idNameLabel, idName, idCommandLabel, idCommand, idPickCommand, idArgsLabel, idArgs, idHelp,
    idActivation, idEnabled, idButtonLabel, idButton, idOnBar, idHotkeyLabel, idCtrl, idAlt, idShift, idWin, idKey, idHotkeyHelp,
    idLook, idLookText,
    idLiteMenu, idStatus, idOk, idCancel, idApply,
};

// Where each control sits at 96 DPI, and on which tab (-1: below them both).
struct Place {
    int id, x, y, w, h, tab;
};
constexpr int kClientW = 660, kClientH = 492;
constexpr RECT kPage = {8, 32, 652, 446};  // the tab page, white with a frame, under the controls
const Place kPlaces[] = {
    {idTabs, 8, 8, 644, 26, -1},  // (only the tabs: the page under them is painted, so group boxes show it)
    {idTree, 20, 40, 290, 318, 0},
    {idAddItem, 20, 366, 94, 26, 0}, {idAddSub, 118, 366, 94, 26, 0}, {idAddSep, 216, 366, 94, 26, 0},
    {idRemove, 20, 398, 94, 26, 0}, {idUp, 118, 398, 94, 26, 0}, {idDown, 216, 398, 94, 26, 0},
    {idProps, 322, 34, 318, 398, 0},
    {idTypeLabel, 336, 62, 96, 20, 0}, {idType, 436, 62, 190, 20, 0},
    {idNameLabel, 336, 92, 290, 18, 0}, {idName, 336, 112, 290, 24, 0},
    {idCommandLabel, 336, 146, 290, 18, 0}, {idCommand, 336, 166, 256, 24, 0}, {idPickCommand, 596, 166, 30, 24, 0},
    {idArgsLabel, 336, 200, 290, 18, 0}, {idArgs, 336, 220, 290, 24, 0},
    {idHelp, 336, 258, 290, 164, 0},
    {idActivation, 20, 34, 620, 238, 1},
    {idEnabled, 36, 60, 580, 22, 1},
    {idButtonLabel, 36, 98, 160, 20, 1}, {idButton, 200, 94, 240, 200, 1},
    {idOnBar, 36, 130, 580, 22, 1},
    {idHotkeyLabel, 36, 168, 160, 20, 1}, {idCtrl, 200, 166, 56, 22, 1}, {idAlt, 258, 166, 50, 22, 1},
    {idShift, 310, 166, 60, 22, 1}, {idWin, 372, 166, 52, 22, 1}, {idKey, 428, 164, 140, 300, 1},
    {idHotkeyHelp, 36, 200, 590, 64, 1},
    {idLook, 20, 282, 620, 150, 1},
    {idLookText, 36, 306, 590, 120, 1},
    {idLiteMenu, 8, 456, 180, 28, -1}, {idStatus, 196, 462, 210, 20, -1},
    {idOk, 412, 456, 76, 28, -1}, {idCancel, 494, 456, 76, 28, -1}, {idApply, 576, 456, 76, 28, -1},
};

HWND g_wnd, g_tree;
HFONT g_font, g_bold;
int g_dpi = 96;
int g_tab = 0;
bool g_dirty, g_filling, g_fromLiteMenu;
HTREEITEM g_drag;
std::atomic<bool> g_open{false};

HWND Ctrl(int id) { return GetDlgItem(g_wnd, id); }
int S(int v) { return MulDiv(v, g_dpi, 96); }

std::wstring TextOf(int id) {
    HWND c = Ctrl(id);
    int n = GetWindowTextLengthW(c);
    std::wstring s(n, L'\0');
    if (n) GetWindowTextW(c, s.data(), n + 1);
    return s;
}

////////////////////////////////////////////////////////////////////////////////
// What it starts from: the rows set now, else LiteMenu's own menu.

std::vector<MenuRow> SettingsRows(std::map<std::wstring, std::wstring> const& set) {
    std::vector<MenuRow> rows;
    auto get = [&](int i, const wchar_t* part) {
        auto it = set.find(std::wstring(kItems) + std::to_wstring(i) + L"]." + part);
        return it == set.end() ? std::wstring() : it->second;
    };
    for (int i = 0; i < 500; i++) {  // (as the bar reads them: up to the first row with no name and no command)
        MenuRow r;
        r.name = get(i, L"name");
        r.command = get(i, L"command");
        if (r.name.empty() && r.command.empty()) break;
        r.args = get(i, L"args");
        r.depth = std::clamp(_wtoi(get(i, L"depth").c_str()), 0, 16);
        rows.push_back(std::move(r));
    }
    return rows;
}

// Enough XML for LiteMenu's config.xml: elements, quoted attributes, text, the prolog and comments.
struct Xml {
    std::wstring name, text;
    std::vector<std::pair<std::wstring, std::wstring>> attrs;
    std::vector<Xml> children;
    Xml const* Child(const wchar_t* n) const {
        for (auto const& c : children)
            if (c.name == n) return &c;
        return nullptr;
    }
    std::wstring ChildText(const wchar_t* n) const {
        Xml const* c = Child(n);
        return c ? c->text : L"";
    }
    std::wstring Attr(const wchar_t* n) const {
        for (auto const& [k, v] : attrs)
            if (k == n) return v;
        return L"";
    }
};

std::wstring XmlDecode(std::wstring_view s) {
    std::wstring out;
    for (size_t i = 0; i < s.size(); i++) {
        size_t semi = s[i] == L'&' ? s.find(L';', i) : std::wstring_view::npos;
        if (semi == std::wstring_view::npos || semi - i > 10) {
            out += s[i];
            continue;
        }
        std::wstring e(s.substr(i + 1, semi - i - 1));
        if (e == L"amp") out += L'&';
        else if (e == L"lt") out += L'<';
        else if (e == L"gt") out += L'>';
        else if (e == L"quot") out += L'"';
        else if (e == L"apos") out += L'\'';
        else if (e.size() > 1 && e[0] == L'#') out += (wchar_t)(e[1] == L'x' ? wcstoul(e.c_str() + 2, nullptr, 16) : wcstoul(e.c_str() + 1, nullptr, 10));
        else {
            out += s[i];
            continue;
        }
        i = semi;
    }
    return out;
}

void XmlSkip(std::wstring_view s, size_t& at) {
    for (;;) {
        while (at < s.size() && iswspace(s[at])) at++;
        size_t close = std::wstring_view::npos;
        if (s.compare(at, 2, L"<?") == 0) close = s.find(L"?>", at), close = close == std::wstring_view::npos ? s.size() : close + 2;
        else if (s.compare(at, 4, L"<!--") == 0) close = s.find(L"-->", at), close = close == std::wstring_view::npos ? s.size() : close + 3;
        else return;
        at = close;
    }
}

bool XmlParse(std::wstring_view s, size_t& at, Xml& node, int depth = 0) {
    if (depth > 64) return false;
    XmlSkip(s, at);
    if (at >= s.size() || s[at] != L'<') return false;
    at++;
    size_t start = at;
    while (at < s.size() && !iswspace(s[at]) && s[at] != L'>' && s[at] != L'/') at++;
    node.name = s.substr(start, at - start);
    for (;;) {  // attributes
        while (at < s.size() && iswspace(s[at])) at++;
        if (at >= s.size()) return false;
        if (s[at] == L'/') {
            at = s.find(L'>', at);
            if (at == std::wstring_view::npos) return false;
            at++;
            return true;
        }
        if (s[at] == L'>') {
            at++;
            break;
        }
        size_t eq = s.find(L'=', at);
        if (eq == std::wstring_view::npos) return false;
        std::wstring key(s.substr(at, eq - at));
        while (!key.empty() && iswspace(key.back())) key.pop_back();
        size_t q = eq + 1;
        while (q < s.size() && iswspace(s[q])) q++;
        if (q >= s.size() || (s[q] != L'"' && s[q] != L'\'')) return false;
        size_t end = s.find(s[q], q + 1);
        if (end == std::wstring_view::npos) return false;
        node.attrs.push_back({key, XmlDecode(s.substr(q + 1, end - q - 1))});
        at = end + 1;
    }
    for (;;) {  // text and children
        size_t lt = s.find(L'<', at);
        if (lt == std::wstring_view::npos) return false;
        node.text += XmlDecode(s.substr(at, lt - at));
        at = lt;
        if (s.compare(at, 4, L"<!--") == 0) {
            XmlSkip(s, at);
            continue;
        }
        if (s.compare(at, 2, L"</") == 0) {
            at = s.find(L'>', at);
            if (at == std::wstring_view::npos) return false;
            at++;
            return true;
        }
        Xml child;
        if (!XmlParse(s, at, child, depth + 1)) return false;
        node.children.push_back(std::move(child));
    }
}

// LiteMenu's own commands as the Tourne Menu's (it reads them either way; these read better), its app ids
// as shell:AppsFolder.
std::wstring FromLiteMenu(std::wstring command, bool shellApp) {
    static const std::pair<const wchar_t*, const wchar_t*> kOwn[] = {
        {L"<setup-litemenu>", L"tourne:menu-settings"}, {L"<close-litemenu>", L"tourne:close-litemenu"},
        {L"<lock-computer>", L"tourne:lock"},           {L"<restart-computer>", L"tourne:restart"},
        {L"<shutdown-computer>", L"tourne:shutdown"},
    };
    for (auto const& [lite, own] : kOwn)
        if (command == lite) return own;
    if (shellApp) return command == L"Microsoft.Windows.Shell.RunDialog" ? L"tourne:run" : L"shell:AppsFolder\\" + command;
    return command;
}

void LiteMenuRows(Xml const& parent, int depth, std::vector<MenuRow>& out) {
    for (auto const& c : parent.children) {
        if (c.name != L"MenuItem") continue;
        std::wstring type = c.Attr(L"type");
        int t = type.empty() ? 1 : _wtoi(type.c_str());
        MenuRow r;
        r.depth = depth;
        r.name = t == 2 ? L"-" : c.ChildText(L"DisplayName");
        if (t == 1) {
            r.command = FromLiteMenu(c.ChildText(L"Command"), _wtoi(c.ChildText(L"IsShellApp").c_str()) != 0);
            r.args = c.ChildText(L"Parameters");
        }
        out.push_back(std::move(r));
        if (t == 0) LiteMenuRows(c, depth + 1, out);
    }
}

std::wstring LiteMenuConfigPath() {
    PWSTR local = nullptr;
    if (FAILED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &local))) return L"";
    std::wstring path = std::wstring(local) + L"\\Packages\\" + kLiteMenuPackage + L"\\LocalState\\config.xml";
    CoTaskMemFree(local);
    return path;
}

// LiteMenu's menu from its config.xml; false when there's none (LiteMenu was never set up here).
bool ReadLiteMenu(std::vector<MenuRow>& rows) {
    HANDLE file = CreateFileW(LiteMenuConfigPath().c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
                              OPEN_EXISTING, 0, nullptr);
    if (file == INVALID_HANDLE_VALUE) return false;
    std::string bytes;
    LARGE_INTEGER size{};
    if (GetFileSizeEx(file, &size) && size.QuadPart > 0 && size.QuadPart < (1 << 22)) {
        bytes.resize((size_t)size.QuadPart);
        DWORD got = 0;
        if (!ReadFile(file, bytes.data(), (DWORD)bytes.size(), &got, nullptr)) got = 0;
        bytes.resize(got);
    }
    CloseHandle(file);
    size_t skip = bytes.rfind("\xEF\xBB\xBF", 0) == 0 ? 3 : 0;
    int n = MultiByteToWideChar(CP_UTF8, 0, bytes.data() + skip, (int)(bytes.size() - skip), nullptr, 0);
    if (n <= 0) return false;
    std::wstring text(n, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, bytes.data() + skip, (int)(bytes.size() - skip), text.data(), n);
    Xml root;
    size_t at = 0;
    if (!XmlParse(text, at, root) || root.name != L"Settings") return false;
    Xml const* menu = root.Child(L"Menu");
    if (!menu) return false;
    rows.clear();
    LiteMenuRows(*menu, 0, rows);
    return true;
}

////////////////////////////////////////////////////////////////////////////////
// The tree: each item holds its Node (lParam), freed as the item goes (TVN_DELETEITEM).

Node* NodeOf(HTREEITEM item) {
    if (!item) return nullptr;
    TVITEMW tv{};
    tv.mask = TVIF_PARAM;
    tv.hItem = item;
    return TreeView_GetItem(g_tree, &tv) ? (Node*)tv.lParam : nullptr;
}

std::wstring TreeText(Node const& n) {
    if (n.type == 2) return L"- - - - - - - - - - - -";
    return n.name.empty() ? L"(no name)" : n.name;
}

void ShowNode(HTREEITEM item) {
    Node* n = NodeOf(item);
    if (!n) return;
    std::wstring text = TreeText(*n);
    TVITEMW tv{};
    tv.mask = TVIF_TEXT | TVIF_STATE;
    tv.hItem = item;
    tv.pszText = text.data();
    tv.stateMask = TVIS_BOLD;
    tv.state = n->type == 0 ? TVIS_BOLD : 0;
    TreeView_SetItem(g_tree, &tv);
}

HTREEITEM AddNode(HTREEITEM parent, HTREEITEM after, Node const& node) {
    std::wstring text = TreeText(node);
    TVINSERTSTRUCTW ins{};
    ins.hParent = parent ? parent : TVI_ROOT;
    ins.hInsertAfter = after ? after : TVI_LAST;
    ins.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_STATE;
    ins.item.pszText = text.data();
    ins.item.lParam = (LPARAM) new Node(node);
    ins.item.stateMask = TVIS_BOLD;
    ins.item.state = node.type == 0 ? TVIS_BOLD : 0;
    return TreeView_InsertItem(g_tree, &ins);
}

void LoadRows(std::vector<MenuRow> const& rows) {
    TreeView_DeleteAllItems(g_tree);
    std::vector<HTREEITEM> levels{nullptr};  // the submenu each depth goes into (null: the top)
    for (size_t i = 0; i < rows.size(); i++) {
        MenuRow const& r = rows[i];
        int depth = std::clamp(r.depth, 0, (int)levels.size() - 1);
        levels.resize(depth + 1);
        Node n;
        n.name = r.name;
        n.command = r.command;
        n.args = r.args;
        bool opens = i + 1 < rows.size() && rows[i + 1].depth > depth;  // (as the bar reads them)
        n.type = r.name == L"-" ? 2 : opens ? 0 : 1;
        if (n.type == 2) n.name.clear();
        HTREEITEM item = AddNode(levels[depth], nullptr, n);
        if (n.type == 0) levels.push_back(item);
    }
    for (HTREEITEM top = TreeView_GetRoot(g_tree); top; top = TreeView_GetNextSibling(g_tree, top)) TreeView_Expand(g_tree, top, TVE_EXPAND);
    TreeView_SelectItem(g_tree, TreeView_GetRoot(g_tree));
}

void Flatten(HTREEITEM first, int depth, std::vector<MenuRow>& out) {
    for (HTREEITEM item = first; item; item = TreeView_GetNextSibling(g_tree, item)) {
        Node* n = NodeOf(item);
        if (!n) continue;
        MenuRow r;
        r.depth = depth;
        if (n->type == 2) r.name = L"-";
        else {
            r.name = n->name;
            if (n->type == 1) r.command = n->command, r.args = n->args;
            if (r.name.empty() && r.command.empty()) r.name = L"(no name)";  // (an empty row would end the menu there)
        }
        out.push_back(std::move(r));
        if (n->type == 0) Flatten(TreeView_GetChild(g_tree, item), depth + 1, out);
    }
}

bool Under(HTREEITEM item, HTREEITEM ancestor) {
    for (HTREEITEM p = item; p; p = TreeView_GetParent(g_tree, p))
        if (p == ancestor) return true;
    return false;
}

HTREEITEM CopyInto(HTREEITEM from, HTREEITEM parent, HTREEITEM after) {
    Node* n = NodeOf(from);
    if (!n) return nullptr;
    HTREEITEM to = AddNode(parent, after, *n);
    HTREEITEM last = nullptr;
    for (HTREEITEM c = TreeView_GetChild(g_tree, from); c; c = TreeView_GetNextSibling(g_tree, c)) last = CopyInto(c, to, last ? last : TVI_FIRST);
    if (TreeView_GetItemState(g_tree, from, TVIS_EXPANDED) & TVIS_EXPANDED) TreeView_Expand(g_tree, to, TVE_EXPAND);
    return to;
}

void Changed();

// item moved under parent (null: the top), after another (TVI_FIRST: first).
void MoveTo(HTREEITEM item, HTREEITEM parent, HTREEITEM after) {
    if (!item || after == item || (parent && Under(parent, item))) return;
    HTREEITEM moved = CopyInto(item, parent, after);
    if (!moved) return;
    TreeView_DeleteItem(g_tree, item);
    if (parent) TreeView_Expand(g_tree, parent, TVE_EXPAND);
    TreeView_SelectItem(g_tree, moved);
    TreeView_EnsureVisible(g_tree, moved);
    Changed();
}

void MoveUp() {
    HTREEITEM item = TreeView_GetSelection(g_tree);
    if (!item) return;
    HTREEITEM parent = TreeView_GetParent(g_tree, item), prev = TreeView_GetPrevSibling(g_tree, item);
    if (prev) {
        HTREEITEM before = TreeView_GetPrevSibling(g_tree, prev);
        MoveTo(item, parent, before ? before : TVI_FIRST);
    } else if (parent) {  // (the first in its submenu: out, above the submenu)
        HTREEITEM before = TreeView_GetPrevSibling(g_tree, parent);
        MoveTo(item, TreeView_GetParent(g_tree, parent), before ? before : TVI_FIRST);
    }
}

void MoveDown() {
    HTREEITEM item = TreeView_GetSelection(g_tree);
    if (!item) return;
    HTREEITEM parent = TreeView_GetParent(g_tree, item), next = TreeView_GetNextSibling(g_tree, item);
    if (next) MoveTo(item, parent, next);
    else if (parent) MoveTo(item, TreeView_GetParent(g_tree, parent), parent);  // (the last: out, below it)
}

// A new one: into the picked submenu (open), else after the picked item.
void Add(int type) {
    Node n;
    n.type = type;
    n.name = type == 0 ? L"New submenu" : type == 1 ? L"New item" : L"";
    HTREEITEM sel = TreeView_GetSelection(g_tree), parent = nullptr, after = nullptr;
    if (Node* s = NodeOf(sel)) {
        if (s->type == 0 && (TreeView_GetItemState(g_tree, sel, TVIS_EXPANDED) & TVIS_EXPANDED || !TreeView_GetChild(g_tree, sel))) parent = sel;
        else parent = TreeView_GetParent(g_tree, sel), after = sel;
    }
    HTREEITEM item = AddNode(parent, after, n);
    if (parent) TreeView_Expand(g_tree, parent, TVE_EXPAND);
    TreeView_SelectItem(g_tree, item);
    TreeView_EnsureVisible(g_tree, item);
    Changed();
    if (type != 2) {
        SetFocus(g_tree);
        TreeView_EditLabel(g_tree, item);
    }
}

void Remove() {
    HTREEITEM sel = TreeView_GetSelection(g_tree);
    Node* n = NodeOf(sel);
    if (!n) return;
    if (TreeView_GetChild(g_tree, sel)) {
        std::wstring ask = L"Remove \"" + TreeText(*n) + L"\" and everything in it?";
        if (MessageBoxW(g_wnd, ask.c_str(), L"Tourne Menu", MB_YESNO | MB_ICONQUESTION) != IDYES) return;
    }
    HTREEITEM next = TreeView_GetNextSibling(g_tree, sel);
    if (!next) next = TreeView_GetPrevSibling(g_tree, sel);
    if (!next) next = TreeView_GetParent(g_tree, sel);
    TreeView_DeleteItem(g_tree, sel);
    if (next) TreeView_SelectItem(g_tree, next);
    Changed();
}

////////////////////////////////////////////////////////////////////////////////
// Properties

struct Action {
    const wchar_t* command;
    const wchar_t* label;
};
const Action kActions[] = {
    {L"tourne:start", L"Start menu"},          {L"tourne:run", L"Run..."},
    {L"tourne:show-desktop", L"Show the desktop"}, {L"tourne:task-manager", L"Task Manager"},
    {L"tourne:settings", L"Windows Settings"}, {L"tourne:studio", L"Tourne Studio"},
    {L"tourne:menu-settings", L"This menu's settings"}, {L"tourne:windhawk", L"Windhawk"},
    {L"tourne:launcher", L"Launcher"}, {L"tourne:desktop-next", L"Next desktop"}, {L"tourne:desktop-previous", L"Desktop before"},
    {L"tourne:desktop-new", L"New desktop"},   {L"tourne:desktop-close", L"Close this desktop"},
    {L"tourne:lock", L"Lock"},                 {L"tourne:sleep", L"Sleep"},
    {L"tourne:sign-out", L"Sign out"},         {L"tourne:restart", L"Restart"},
    {L"tourne:shutdown", L"Shut down"},        {L"tourne:close-litemenu", L"Close LiteMenu"},
};
const Action kLive[] = {
    {L"tourne:windows", L"Open windows"}, {L"tourne:desktops", L"Desktops"},
    {L"tourne:programs", L"All programs"}, {L"tourne:recent", L"Recent items"},
};

std::wstring HelpFor(Node const* n) {
    if (!n) return L"Pick an item in the menu to change it, or add one with the buttons under the menu.";
    if (n->type == 2) return L"A line between items. Move it like any item.";
    if (n->type == 0)
        return L"A submenu: what's under it in the tree opens beside it. Add to it with Add item while it's picked, or "
               L"drag an item onto it.";
    std::wstring c = n->command;
    if (c.empty()) return L"No command: a click does nothing (a heading). Type one, or pick one with the ... button.";
    for (auto const& a : kActions)
        if (c == a.command) return std::wstring(L"Tourne's own action: ") + a.label + L".";
    for (auto const& a : kLive)
        if (c == a.command) return std::wstring(L"A submenu that fills itself each time it opens: ") + a.label + L".";
    if (c.rfind(L"tourne:folder:", 0) == 0) return L"A submenu that fills itself with this folder's contents each time it opens.";
    if (c.rfind(L"tourne:desktop:", 0) == 0) return L"Goes to that virtual desktop.";
    if (c.rfind(L"shell:AppsFolder\\", 0) == 0) return L"Opens this app (its app id). Parameters aren't used.";
    if (c.find(L"://") != std::wstring::npos) return L"Opens this address in its program.";
    return L"Opens this program, shortcut, file or folder. Parameters go to it, like a shortcut's.";
}

void Fill() {
    g_filling = true;
    Node* n = NodeOf(TreeView_GetSelection(g_tree));
    SetWindowTextW(Ctrl(idType), !n ? L"" : n->type == 0 ? L"Submenu" : n->type == 1 ? L"Item" : L"Separator");
    SetWindowTextW(Ctrl(idName), n && n->type != 2 ? n->name.c_str() : L"");
    SetWindowTextW(Ctrl(idCommand), n && n->type == 1 ? n->command.c_str() : L"");
    SetWindowTextW(Ctrl(idArgs), n && n->type == 1 ? n->args.c_str() : L"");
    SetWindowTextW(Ctrl(idHelp), HelpFor(n).c_str());
    bool named = n && n->type != 2, item = n && n->type == 1;
    EnableWindow(Ctrl(idNameLabel), named);
    EnableWindow(Ctrl(idName), named);
    for (int id : {idCommandLabel, idCommand, idPickCommand, idArgsLabel, idArgs}) EnableWindow(Ctrl(id), item);
    for (int id : {idRemove, idUp, idDown}) EnableWindow(Ctrl(id), n != nullptr);
    g_filling = false;
}

void FieldChanged(int id) {
    if (g_filling) return;
    HTREEITEM sel = TreeView_GetSelection(g_tree);
    Node* n = NodeOf(sel);
    if (!n) return;
    if (id == idName) n->name = TextOf(idName), ShowNode(sel);
    else if (id == idCommand) n->command = TextOf(idCommand), SetWindowTextW(Ctrl(idHelp), HelpFor(n).c_str());
    else if (id == idArgs) n->args = TextOf(idArgs);
    Changed();
}

std::wstring BrowseFile() {
    WCHAR path[MAX_PATH * 4] = L"";
    OPENFILENAMEW ofn{sizeof(ofn)};
    ofn.hwndOwner = g_wnd;
    ofn.lpstrFilter = L"Programs and shortcuts\0*.exe;*.lnk;*.url;*.bat;*.cmd;*.ps1;*.msc;*.cpl\0Everything\0*.*\0";
    ofn.lpstrFile = path;
    ofn.nMaxFile = ARRAYSIZE(path);
    ofn.lpstrTitle = L"A program, shortcut or file for the menu";
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_NODEREFERENCELINKS | OFN_HIDEREADONLY | OFN_EXPLORER;
    return GetOpenFileNameW(&ofn) ? path : L"";
}

std::wstring BrowseFolder(const wchar_t* title) {
    std::wstring out;
    IFileOpenDialog* dlg = nullptr;
    if (FAILED(CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&dlg)))) return out;
    DWORD opts = 0;
    dlg->GetOptions(&opts);
    dlg->SetOptions(opts | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);
    dlg->SetTitle(title);
    IShellItem* item = nullptr;
    if (SUCCEEDED(dlg->Show(g_wnd)) && SUCCEEDED(dlg->GetResult(&item))) {
        PWSTR p = nullptr;
        if (SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &p))) out = p, CoTaskMemFree(p);
        item->Release();
    }
    dlg->Release();
    return out;
}

// The ... beside Command: what LiteMenu's Browse gives, and Tourne's own.
void PickCommand() {
    Node* n = NodeOf(TreeView_GetSelection(g_tree));
    if (!n || n->type != 1) return;
    enum : int { kFile = 1, kFolder, kLiveFolder, kNothing, kAction = 100, kLiveList = 200 };
    HMENU menu = CreatePopupMenu(), actions = CreatePopupMenu(), live = CreatePopupMenu();
    AppendMenuW(menu, MF_STRING, kFile, L"A program, shortcut or file...");
    AppendMenuW(menu, MF_STRING, kFolder, L"A folder...");
    for (size_t i = 0; i < ARRAYSIZE(kActions); i++)
        AppendMenuW(actions, MF_STRING | (n->command == kActions[i].command ? MF_CHECKED : 0), kAction + i, kActions[i].label);
    AppendMenuW(menu, MF_POPUP, (UINT_PTR)actions, L"One of Tourne's actions");
    for (size_t i = 0; i < ARRAYSIZE(kLive); i++)
        AppendMenuW(live, MF_STRING | (n->command == kLive[i].command ? MF_CHECKED : 0), kLiveList + i, kLive[i].label);
    AppendMenuW(live, MF_STRING, kLiveFolder, L"A folder's contents...");
    AppendMenuW(menu, MF_POPUP, (UINT_PTR)live, L"A submenu that fills itself");
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING, kNothing, L"Nothing (a heading)");
    RECT r;
    GetWindowRect(Ctrl(idPickCommand), &r);
    int id = TrackPopupMenuEx(menu, TPM_RETURNCMD | TPM_RIGHTALIGN | TPM_TOPALIGN, r.right, r.bottom, g_wnd, nullptr);
    DestroyMenu(menu);
    std::wstring command;
    bool set = true;
    if (id == kFile) command = BrowseFile(), set = !command.empty();
    else if (id == kFolder) command = BrowseFolder(L"A folder for the menu"), set = !command.empty();
    else if (id == kLiveFolder) {
        std::wstring dir = BrowseFolder(L"The folder whose contents the submenu shows");
        command = L"tourne:folder:" + dir, set = !dir.empty();
    } else if (id == kNothing) command.clear();
    else if (id >= kAction && id < kAction + (int)ARRAYSIZE(kActions)) command = kActions[id - kAction].command;
    else if (id >= kLiveList && id < kLiveList + (int)ARRAYSIZE(kLive)) command = kLive[id - kLiveList].command;
    else set = false;
    if (!set) return;
    SetWindowTextW(Ctrl(idCommand), command.c_str());  // (FieldChanged keeps it)
    if (n->name.empty() || n->name == L"New item") {  // (a new item takes the name of what it opens)
        std::wstring name;
        for (auto const& a : kActions)
            if (command == a.command) name = a.label;
        for (auto const& a : kLive)
            if (command == a.command) name = a.label;
        if (name.empty() && !command.empty() && command.rfind(L"tourne:", 0) != 0) {
            name = command.substr(command.find_last_of(L"\\/") + 1);
            if (size_t dot = name.rfind(L'.'); dot != std::wstring::npos && dot > 0) name.erase(dot);
        }
        if (!name.empty()) SetWindowTextW(Ctrl(idName), name.c_str());
    }
}

////////////////////////////////////////////////////////////////////////////////
// Activation (the Settings tab)

struct Key {
    UINT vk;
    std::wstring label;
};

std::vector<Key> Keys() {
    std::vector<Key> keys{{0, L"(none)"}};
    for (wchar_t c = L'A'; c <= L'Z'; c++) keys.push_back({(UINT)c, std::wstring(1, c)});
    for (wchar_t c = L'0'; c <= L'9'; c++) keys.push_back({(UINT)c, std::wstring(1, c)});
    for (int f = 1; f <= 12; f++) keys.push_back({(UINT)(VK_F1 + f - 1), L"F" + std::to_wstring(f)});
    keys.push_back({VK_SPACE, L"Space"});
    keys.push_back({VK_OEM_3, L"` (backquote)"});
    keys.push_back({VK_APPS, L"Menu key"});
    return keys;
}

void LoadHotkey(std::wstring const& value) {
    UINT mods = 0, vk = 0;
    size_t at = 0;
    while (at <= value.size()) {  // "alt+win+77", as the bar reads it
        size_t end = value.find(L'+', at);
        std::wstring part = value.substr(at, end == std::wstring::npos ? end : end - at);
        if (part == L"ctrl") mods |= MOD_CONTROL;
        else if (part == L"alt") mods |= MOD_ALT;
        else if (part == L"shift") mods |= MOD_SHIFT;
        else if (part == L"win") mods |= MOD_WIN;
        else if (!part.empty()) vk = (UINT)_wtoi(part.c_str());
        if (end == std::wstring::npos) break;
        at = end + 1;
    }
    Button_SetCheck(Ctrl(idCtrl), mods & MOD_CONTROL ? BST_CHECKED : BST_UNCHECKED);
    Button_SetCheck(Ctrl(idAlt), mods & MOD_ALT ? BST_CHECKED : BST_UNCHECKED);
    Button_SetCheck(Ctrl(idShift), mods & MOD_SHIFT ? BST_CHECKED : BST_UNCHECKED);
    Button_SetCheck(Ctrl(idWin), mods & MOD_WIN ? BST_CHECKED : BST_UNCHECKED);
    HWND key = Ctrl(idKey);
    int pick = 0;
    for (int i = 0, n = ComboBox_GetCount(key); i < n; i++)
        if ((UINT)ComboBox_GetItemData(key, i) == vk) pick = i;
    if (vk && !pick) {  // (one set by hand that isn't in the list)
        pick = ComboBox_AddString(key, (L"Key " + std::to_wstring(vk)).c_str());
        ComboBox_SetItemData(key, pick, vk);
    }
    ComboBox_SetCurSel(key, pick);
}

std::wstring HotkeyValue() {
    HWND key = Ctrl(idKey);
    int sel = ComboBox_GetCurSel(key);
    UINT vk = sel < 0 ? 0 : (UINT)ComboBox_GetItemData(key, sel);
    if (!vk) return L"";
    std::wstring out;
    if (Button_GetCheck(Ctrl(idCtrl)) == BST_CHECKED) out += L"ctrl+";
    if (Button_GetCheck(Ctrl(idAlt)) == BST_CHECKED) out += L"alt+";
    if (Button_GetCheck(Ctrl(idShift)) == BST_CHECKED) out += L"shift+";
    if (Button_GetCheck(Ctrl(idWin)) == BST_CHECKED) out += L"win+";
    return out + std::to_wstring(vk);
}

const std::pair<const wchar_t*, const wchar_t*> kButtons[] = {
    {L"middle", L"Middle Button"}, {L"right", L"Right Button"}, {L"none", L"None (the hotkey only)"}};

////////////////////////////////////////////////////////////////////////////////
// Reading and writing

void Changed() {
    g_dirty = true;
    EnableWindow(Ctrl(idApply), TRUE);
    SetWindowTextW(Ctrl(idStatus), g_fromLiteMenu ? L"LiteMenu's menu: OK copies it in." : L"Not saved yet.");
}

void Load() {
    std::map<std::wstring, std::wstring> set;
    for (auto const& [k, v] : SettingsWithPrefix(L"taskbar.menu.")) set[k] = v;
    auto get = [&](const wchar_t* k) {
        auto it = set.find(std::wstring(L"taskbar.menu.") + k);
        return it == set.end() ? std::wstring() : it->second;
    };
    g_filling = true;
    std::wstring on = get(L"enabled");
    Button_SetCheck(Ctrl(idEnabled), on == L"true" || _wtoi(on.c_str()) ? BST_CHECKED : BST_UNCHECKED);
    std::wstring onBar = get(L"onBar");
    Button_SetCheck(Ctrl(idOnBar), onBar == L"true" || _wtoi(onBar.c_str()) ? BST_CHECKED : BST_UNCHECKED);
    std::wstring button = get(L"button");
    int pick = 0;
    for (int i = 0; i < (int)ARRAYSIZE(kButtons); i++)
        if (button == kButtons[i].first) pick = i;
    ComboBox_SetCurSel(Ctrl(idButton), pick);
    LoadHotkey(get(L"hotkey"));
    std::vector<MenuRow> rows = SettingsRows(set);
    g_fromLiteMenu = rows.empty() && ReadLiteMenu(rows);
    LoadRows(rows);
    g_filling = false;
    g_dirty = g_fromLiteMenu;  // (LiteMenu's menu isn't Tourne's yet: OK copies it in)
    EnableWindow(Ctrl(idApply), g_dirty);
    SetWindowTextW(Ctrl(idStatus), g_fromLiteMenu ? L"LiteMenu's menu: OK copies it in." : L"");
    Fill();
}

bool Save() {
    std::vector<MenuRow> rows;
    Flatten(TreeView_GetRoot(g_tree), 0, rows);
    std::vector<std::pair<std::wstring, std::wstring>> set;
    set.push_back({L"taskbar.menu.enabled", Button_GetCheck(Ctrl(idEnabled)) == BST_CHECKED ? L"true" : L"false"});
    int b = std::clamp(ComboBox_GetCurSel(Ctrl(idButton)), 0, (int)ARRAYSIZE(kButtons) - 1);
    set.push_back({L"taskbar.menu.button", kButtons[b].first});
    set.push_back({L"taskbar.menu.onBar", Button_GetCheck(Ctrl(idOnBar)) == BST_CHECKED ? L"true" : L"false"});
    set.push_back({L"taskbar.menu.hotkey", HotkeyValue()});
    for (size_t i = 0; i <= rows.size(); i++) {  // (and an empty row after the last: rows past it in tourne.conf end there)
        std::wstring at = kItems + std::to_wstring(i) + L"].";
        MenuRow r = i < rows.size() ? rows[i] : MenuRow{};
        set.push_back({at + L"depth", std::to_wstring(r.depth)});
        set.push_back({at + L"name", r.name});
        set.push_back({at + L"command", r.command});
        set.push_back({at + L"args", r.args});
    }
    if (!WriteStudioSettings(set, L"taskbar.menu.", L"menu-settings")) {
        MessageBoxW(g_wnd, L"Couldn't write tourne-gui.conf, so nothing changed.", L"Tourne Menu", MB_OK | MB_ICONWARNING);
        return false;
    }
    g_dirty = g_fromLiteMenu = false;
    EnableWindow(Ctrl(idApply), FALSE);
    SetWindowTextW(Ctrl(idStatus), L"Saved.");
    return true;
}

// Closing with changes not saved: keep them? False: stay open.
bool MayClose() {
    if (!g_dirty || g_fromLiteMenu) return true;
    int answer = MessageBoxW(g_wnd, L"Save the changes to the Tourne Menu?", L"Tourne Menu", MB_YESNOCANCEL | MB_ICONQUESTION);
    return answer == IDNO || (answer == IDYES && Save());
}

////////////////////////////////////////////////////////////////////////////////
// The window

void ShowTab(int tab) {
    g_tab = tab;
    for (auto const& p : kPlaces)
        if (p.tab >= 0) ShowWindow(Ctrl(p.id), p.tab == tab ? SW_SHOWNA : SW_HIDE);
}

void MakeFonts() {
    if (g_font) DeleteObject(g_font);
    if (g_bold) DeleteObject(g_bold);
    NONCLIENTMETRICSW m{sizeof(m)};
    SystemParametersInfoForDpi(SPI_GETNONCLIENTMETRICS, sizeof(m), &m, 0, g_dpi);
    g_font = CreateFontIndirectW(&m.lfMessageFont);
    m.lfMessageFont.lfWeight = FW_SEMIBOLD;
    g_bold = CreateFontIndirectW(&m.lfMessageFont);
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

void Build() {
    HWND tabs = Make(WC_TABCONTROLW, L"", WS_TABSTOP, idTabs);
    TCITEMW t{};
    t.mask = TCIF_TEXT;
    t.pszText = (LPWSTR)L"Menu";
    TabCtrl_InsertItem(tabs, 0, &t);
    t.pszText = (LPWSTR)L"Settings";
    TabCtrl_InsertItem(tabs, 1, &t);
    Make(L"BUTTON", L"Properties", BS_GROUPBOX, idProps);
    Make(L"BUTTON", L"Activation", BS_GROUPBOX, idActivation);
    Make(L"BUTTON", L"Look", BS_GROUPBOX, idLook);
    g_tree = Make(WC_TREEVIEWW, L"", WS_TABSTOP | TVS_HASLINES | TVS_HASBUTTONS | TVS_LINESATROOT | TVS_SHOWSELALWAYS | TVS_EDITLABELS,
                  idTree, WS_EX_CLIENTEDGE);
    Make(L"BUTTON", L"Add item", WS_TABSTOP, idAddItem);
    Make(L"BUTTON", L"Add submenu", WS_TABSTOP, idAddSub);
    Make(L"BUTTON", L"Add separator", WS_TABSTOP, idAddSep);
    Make(L"BUTTON", L"Remove", WS_TABSTOP, idRemove);
    Make(L"BUTTON", L"Move up", WS_TABSTOP, idUp);
    Make(L"BUTTON", L"Move down", WS_TABSTOP, idDown);
    Make(L"STATIC", L"Type:", 0, idTypeLabel);
    Make(L"STATIC", L"", 0, idType);
    Make(L"STATIC", L"Display name", 0, idNameLabel);
    Make(L"EDIT", L"", WS_TABSTOP | ES_AUTOHSCROLL, idName, WS_EX_CLIENTEDGE);
    Make(L"STATIC", L"Command", 0, idCommandLabel);
    Make(L"EDIT", L"", WS_TABSTOP | ES_AUTOHSCROLL, idCommand, WS_EX_CLIENTEDGE);
    Make(L"BUTTON", L"...", WS_TABSTOP, idPickCommand);
    Make(L"STATIC", L"Parameters", 0, idArgsLabel);
    Make(L"EDIT", L"", WS_TABSTOP | ES_AUTOHSCROLL, idArgs, WS_EX_CLIENTEDGE);
    Make(L"STATIC", L"", 0, idHelp);
    Make(L"BUTTON", L"Turn on the Tourne Menu", WS_TABSTOP | BS_AUTOCHECKBOX, idEnabled);
    Make(L"STATIC", L"Mouse button:", 0, idButtonLabel);
    HWND button = Make(WC_COMBOBOXW, L"", WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWNLIST, idButton);
    for (auto const& [value, label] : kButtons) ComboBox_AddString(button, label);
    Make(L"BUTTON", L"Also on the bar's empty space (a click where there's nothing)", WS_TABSTOP | BS_AUTOCHECKBOX, idOnBar);
    Make(L"STATIC", L"Hotkey:", 0, idHotkeyLabel);
    Make(L"BUTTON", L"Ctrl", WS_TABSTOP | BS_AUTOCHECKBOX, idCtrl);
    Make(L"BUTTON", L"Alt", WS_TABSTOP | BS_AUTOCHECKBOX, idAlt);
    Make(L"BUTTON", L"Shift", WS_TABSTOP | BS_AUTOCHECKBOX, idShift);
    Make(L"BUTTON", L"Win", WS_TABSTOP | BS_AUTOCHECKBOX, idWin);
    HWND key = Make(WC_COMBOBOXW, L"", WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWNLIST, idKey);
    for (auto const& k : Keys()) {
        int at = ComboBox_AddString(key, k.label.c_str());
        ComboBox_SetItemData(key, at, k.vk);
    }
    Make(L"STATIC",
         L"The menu opens on the desktop's empty space with the mouse button, and anywhere at the pointer with the hotkey "
         L"(none until a key is picked). With the right button, Shift + right-click keeps Windows' own desktop menu.",
         0, idHotkeyHelp);
    Make(L"STATIC",
         L"The menu draws in Tourne's menu look, like every Tourne menu: the theme's colours, its frame and font, and "
         L"whether menus show icons. Change those in Tourne Studio (Taskbar, its menu settings), so LiteMenu's own theme "
         L"and colours aren't used here.",
         0, idLookText);
    Make(L"BUTTON", L"Copy LiteMenu's menu", WS_TABSTOP, idLiteMenu);
    Make(L"STATIC", L"", 0, idStatus);
    Make(L"BUTTON", L"OK", WS_TABSTOP | BS_DEFPUSHBUTTON, idOk);
    Make(L"BUTTON", L"Cancel", WS_TABSTOP, idCancel);
    Make(L"BUTTON", L"Apply", WS_TABSTOP, idApply);
    // A child made later goes under those made before it (and Tab follows that order), so the tabs and the
    // group boxes go to the bottom, under what sits on them.
    Layout();
    SetWindowPos(Ctrl(idProps), HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    SetWindowPos(Ctrl(idActivation), HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    SetWindowPos(Ctrl(idLook), HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    SetWindowPos(tabs, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    ShowTab(0);
}

// Dragging an item: onto a submenu's middle puts it in; the top or bottom of an item, before or after it.
struct Drop {
    HTREEITEM target = nullptr;
    int where = 0;  // -1 before, 0 into, 1 after
};

Drop DropAt(POINT screen) {
    Drop d;
    POINT pt = screen;
    ScreenToClient(g_tree, &pt);
    TVHITTESTINFO hit{};
    hit.pt = pt;
    d.target = TreeView_HitTest(g_tree, &hit);
    if (!d.target) {
        d.where = 1;
        for (HTREEITEM last = TreeView_GetRoot(g_tree); last; last = TreeView_GetNextSibling(g_tree, last)) d.target = last;
        return d;
    }
    RECT r;
    TreeView_GetItemRect(g_tree, d.target, &r, FALSE);
    int h = r.bottom - r.top, y = pt.y - r.top;
    Node* n = NodeOf(d.target);
    if (n && n->type == 0) d.where = y < h / 4 ? -1 : y >= h * 3 / 4 ? 1 : 0;
    else d.where = y < h / 2 ? -1 : 1;
    return d;
}

void ShowDrop(Drop const& d) {
    bool into = d.target && d.where == 0;
    TreeView_SelectDropTarget(g_tree, into ? d.target : nullptr);
    TreeView_SetInsertMark(g_tree, into ? nullptr : d.target, d.where > 0);
}

void EndDrag(bool drop) {
    HTREEITEM item = g_drag;
    g_drag = nullptr;
    ReleaseCapture();
    POINT pt;
    GetCursorPos(&pt);
    Drop d = DropAt(pt);
    TreeView_SelectDropTarget(g_tree, nullptr);
    TreeView_SetInsertMark(g_tree, nullptr, FALSE);
    if (!drop || !item || !d.target || Under(d.target, item)) return;
    if (d.where == 0) MoveTo(item, d.target, TVI_LAST);
    else {
        HTREEITEM parent = TreeView_GetParent(g_tree, d.target);
        HTREEITEM after = d.where > 0 ? d.target : TreeView_GetPrevSibling(g_tree, d.target);
        if (after == item) return;  // (where it is already)
        MoveTo(item, parent, after ? after : TVI_FIRST);
    }
}

LRESULT OnNotify(NMHDR* h) {
    if (h->idFrom == idTabs && h->code == TCN_SELCHANGE) {
        ShowTab(TabCtrl_GetCurSel(h->hwndFrom));
        return 0;
    }
    if (h->idFrom != idTree) return 0;
    switch (h->code) {
        case TVN_SELCHANGEDW: Fill(); break;
        case TVN_DELETEITEMW: delete (Node*)((NMTREEVIEWW*)h)->itemOld.lParam; break;
        case TVN_BEGINDRAGW: {
            auto* tv = (NMTREEVIEWW*)h;
            g_drag = tv->itemNew.hItem;
            TreeView_SelectItem(g_tree, g_drag);
            SetCapture(g_wnd);
            break;
        }
        case TVN_BEGINLABELEDITW: {
            Node* n = NodeOf(((NMTVDISPINFOW*)h)->item.hItem);
            if (!n || n->type == 2) return TRUE;  // (a separator has no name)
            if (HWND edit = TreeView_GetEditControl(g_tree)) SetWindowTextW(edit, n->name.c_str());  // (not "(no name)")
            return FALSE;
        }
        case TVN_ENDLABELEDITW: {
            auto* d = (NMTVDISPINFOW*)h;
            Node* n = NodeOf(d->item.hItem);
            if (!n || !d->item.pszText) return FALSE;
            n->name = d->item.pszText;
            ShowNode(d->item.hItem);
            Fill();
            Changed();
            return FALSE;  // (ShowNode set the text)
        }
        case TVN_KEYDOWN: {
            auto* k = (NMTVKEYDOWN*)h;
            if (k->wVKey == VK_DELETE) Remove();
            else if (k->wVKey == VK_F2) {
                if (HTREEITEM sel = TreeView_GetSelection(g_tree)) TreeView_EditLabel(g_tree, sel);
            }
            break;
        }
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
        case WM_COMMAND: {
            int id = LOWORD(wp), code = HIWORD(wp);
            if (code == EN_CHANGE && (id == idName || id == idCommand || id == idArgs)) FieldChanged(id);
            else if (code == BN_CLICKED || code == 0) switch (id) {
                    case idAddItem: Add(1); break;
                    case idAddSub: Add(0); break;
                    case idAddSep: Add(2); break;
                    case idRemove: Remove(); break;
                    case idUp: MoveUp(); break;
                    case idDown: MoveDown(); break;
                    case idPickCommand: PickCommand(); break;
                    case idEnabled:
                    case idOnBar:
                    case idCtrl:
                    case idAlt:
                    case idShift:
                    case idWin:
                        if (!g_filling) Changed();
                        break;
                    case idLiteMenu: {
                        std::vector<MenuRow> rows;
                        if (!ReadLiteMenu(rows)) {
                            MessageBoxW(wnd, L"LiteMenu's settings (its config.xml) weren't found on this PC.", L"Tourne Menu", MB_OK | MB_ICONINFORMATION);
                            break;
                        }
                        if (TreeView_GetCount(g_tree) &&
                            MessageBoxW(wnd, L"Replace the menu here with LiteMenu's? (Nothing is saved until OK or Apply.)", L"Tourne Menu",
                                        MB_YESNO | MB_ICONQUESTION) != IDYES)
                            break;
                        LoadRows(rows);
                        Changed();
                        break;
                    }
                    case idOk:
                    case IDOK:
                        if (!g_dirty || Save()) DestroyWindow(wnd);
                        break;
                    case idCancel:
                    case IDCANCEL: DestroyWindow(wnd); break;
                    case idApply: Save(); break;
                }
            else if (code == CBN_SELCHANGE && (id == idButton || id == idKey) && !g_filling) Changed();
            return 0;
        }
        case WM_NOTIFY: return OnNotify((NMHDR*)lp);
        case WM_MOUSEMOVE:
            if (g_drag) {
                POINT pt;
                GetCursorPos(&pt);
                ShowDrop(DropAt(pt));
            }
            return 0;
        case WM_LBUTTONUP:
            if (g_drag) EndDrag(true);
            return 0;
        case WM_CAPTURECHANGED:
            if (g_drag && (HWND)lp != wnd) {
                g_drag = nullptr;
                TreeView_SelectDropTarget(g_tree, nullptr);
                TreeView_SetInsertMark(g_tree, nullptr, FALSE);
            }
            return 0;
        case WM_CTLCOLORSTATIC: {  // (the labels on the tab pages: the page's own white, not the window's grey)
            HWND c = (HWND)lp;
            int id = GetDlgCtrlID(c);
            if (id == idStatus || id == idLiteMenu) break;
            HDC dc = (HDC)wp;
            SetBkColor(dc, GetSysColor(COLOR_WINDOW));
            SetTextColor(dc, GetSysColor(IsWindowEnabled(c) ? COLOR_WINDOWTEXT : COLOR_GRAYTEXT));
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
            g_wnd = g_tree = nullptr;
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(wnd, msg, wp, lp);
}

DWORD WINAPI EditorThread(void*) {
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    INITCOMMONCONTROLSEX icc{sizeof(icc), ICC_TREEVIEW_CLASSES | ICC_TAB_CLASSES | ICC_STANDARD_CLASSES};
    InitCommonControlsEx(&icc);
    WNDCLASSEXW wc{sizeof(wc)};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
    wc.hIcon = LoadIconW(GetModuleHandleW(nullptr), MAKEINTRESOURCEW(1));
    wc.lpszClassName = kClass;
    RegisterClassExW(&wc);
    DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;  // (not WS_CLIPCHILDREN: the page shows inside group boxes)
    POINT cursor;
    GetCursorPos(&cursor);
    HMONITOR mon = MonitorFromPoint(cursor, MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi{sizeof(mi)};
    GetMonitorInfoW(mon, &mi);
    UINT dpi = GetDpiForSystem();
    RECT r{0, 0, MulDiv(kClientW, dpi, 96), MulDiv(kClientH, dpi, 96)};
    AdjustWindowRectExForDpi(&r, style, FALSE, 0, dpi);
    int w = r.right - r.left, h = r.bottom - r.top;
    int x = (mi.rcWork.left + mi.rcWork.right - w) / 2, y = (mi.rcWork.top + mi.rcWork.bottom - h) / 2;
    HWND wnd = CreateWindowExW(0, kClass, L"Tourne Menu settings", style, x, y, w, h, nullptr, nullptr, wc.hInstance, nullptr);
    if (wnd) {
        // (its client size again at the screen's own DPI, if that isn't the system's)
        if (UINT own = GetDpiForWindow(wnd); own != dpi) {
            RECT c{0, 0, MulDiv(kClientW, own, 96), MulDiv(kClientH, own, 96)};
            AdjustWindowRectExForDpi(&c, style, FALSE, 0, own);
            SetWindowPos(wnd, nullptr, 0, 0, c.right - c.left, c.bottom - c.top, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
        }
        ShowWindow(wnd, SW_SHOWNORMAL);
        SetForegroundWindow(wnd);
        SetFocus(g_tree);
        MSG m;
        while (GetMessageW(&m, nullptr, 0, 0) > 0) {
            // (Enter and Esc go to a name being typed in the tree, not to the window)
            bool naming = g_tree && TreeView_GetEditControl(g_tree);
            if (!naming && g_wnd && m.message == WM_KEYDOWN && m.wParam == VK_ESCAPE) {
                SendMessageW(g_wnd, WM_CLOSE, 0, 0);
                continue;
            }
            if (naming || !g_wnd || !IsDialogMessageW(g_wnd, &m)) {
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

void OpenMenuEditor() {
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

void RunMenuEditor() {
    if (!g_open.exchange(true)) EditorThread(nullptr);
}

}  // namespace td
