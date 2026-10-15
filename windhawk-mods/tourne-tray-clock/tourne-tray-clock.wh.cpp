// ==WindhawkMod==
// @id              tourne-tray-clock
// @name            Tourne'Tray [Clock]
// @description     Restyle the classic taskbar clock (StartAllBack and the Windows 10 taskbar): lines, font, color, position and what each click does
// @version         1.1.0
// @author          USER-TOURNE
// @github          https://github.com/USER-TOURNE
// @include         explorer.exe
// @architecture    x86-64
// @compilerOptions -luxtheme -lgdi32 -lcomctl32 -lshell32 -lole32
// @license         MIT
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# `Tourne'Tray` **[Clock]**

Takes over the drawing, size and clicks of the classic taskbar clock, the one
StartAllBack shows on every taskbar. **Keep that clock turned on in StartAllBack**:
this mod redraws it, and StartAllBack removes the clock's window when it's off. The time itself still comes from Windows:
the clock reads the system time and your region's formats the same way the
built-in clock does, and follows time zone and format changes.

## Lines

Up to three lines, each a mix of text and these tokens:

| Token | Shows | Example |
|---|---|---|
| `{time}` | The short time, as the built-in clock shows it | 9:41 PM |
| `{time:long}` | The long time, with seconds | 9:41:07 PM |
| `{date}` | The short date | 9/24/2026 |
| `{date:long}` | The long date | Thursday, September 24, 2026 |
| `{weekday}` / `{weekday:short}` | The day of the week | Thursday / Thu |
| `{t:...}` | Any time picture: `h H hh HH m mm s ss t tt` | `{t:HH:mm}` 21:41 |
| `{d:...}` | Any date picture: `d dd ddd dddd M MM MMM MMMM yy yyyy` | `{d:dd MMM}` 24 Sep |

The clock ticks every second only when a line shows seconds.

## Clicks

Left, right and middle click each pick one action: the Tourne notification
centre and calendar (System Icons' flyout), what Windows does, nothing,
the clock's own flyout, the taskbar menu, notifications, date and time
settings, Alarms & Clock, copying the shown text, or a command of your own.

## Position

The native spot, after the tray icons, or left of the tray icons.
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- position: native
  $name: Position
  $description: >-
    Keep the taskbar clock turned on in StartAllBack (Tray icons, the clock
    button): this mod redraws that clock, so with it off there's nothing to
    draw into.
  $options:
  - native: Native spot, after the tray icons
  - leftOfIcons: Left of the tray icons
- lines:
  - line1: "{time}"
    $name: Line 1
  - line2: "{date}"
    $name: Line 2
  - line3: ""
    $name: Line 3
  $name: Lines
  $description: >-
    Text and tokens such as {time}, {date}, {weekday} or {t:HH:mm}; see the
    readme for all of them. Empty lines are left out.
- font:
  - face: ""
    $name: Font
    $description: Empty uses the taskbar clock's own font.
  - size: 0
    $name: Size
    $description: In pixels at 100% scaling. 0 keeps the clock's own size.
  - secondarySize: 0
    $name: Size of lines 2 and 3
    $description: 0 uses the size above.
  - bold: false
    $name: Bold first line
  - smoothing: default
    $name: Smoothing
    $options:
    - default: Like the taskbar
    - crisp: None, for pixel fonts
    - smooth: Smooth
  $name: Font
- color:
  - primary: ""
    $name: First line
    $description: "#RRGGBB. Empty uses the taskbar clock's own color."
  - secondary: ""
    $name: Lines 2 and 3
    $description: "#RRGGBB. Empty uses the first line's color."
  $name: Color
- layout:
  - align: center
    $name: Alignment
    $options:
    - center: Center
    - left: Left
    - right: Right
  - padding: 6
    $name: Side padding
    $description: In pixels at 100% scaling, on each side.
  - lineSpacing: 0
    $name: Line spacing
    $description: Extra pixels between lines at 100% scaling. Can be negative.
  $name: Layout
- clicks:
  - left: notificationCenter
    $name: Left click
    $options:
    - notificationCenter: Tourne notifications & calendar
    - default: What Windows does
    - nothing: Nothing
    - flyout: The clock's flyout
    - taskbarMenu: Taskbar menu
    - notifications: Notifications
    - settings: Date and time settings
    - alarms: Alarms & Clock
    - copy: Copy the shown text
    - command: Run the command below
  - leftCommand: ""
    $name: Left click command
  - right: default
    $name: Right click
    $options:
    - notificationCenter: Tourne notifications & calendar
    - default: What Windows does
    - nothing: Nothing
    - flyout: The clock's flyout
    - taskbarMenu: Taskbar menu
    - notifications: Notifications
    - settings: Date and time settings
    - alarms: Alarms & Clock
    - copy: Copy the shown text
    - command: Run the command below
  - rightCommand: ""
    $name: Right click command
  - middle: default
    $name: Middle click
    $options:
    - notificationCenter: Tourne notifications & calendar
    - default: What Windows does
    - nothing: Nothing
    - flyout: The clock's flyout
    - taskbarMenu: Taskbar menu
    - notifications: Notifications
    - settings: Date and time settings
    - alarms: Alarms & Clock
    - copy: Copy the shown text
    - command: Run the command below
  - middleCommand: ""
    $name: Middle click command
  $name: Clicks
  $description: >-
    A command is a program or file path, a URI such as ms-settings:display,
    or a program in quotes followed by its arguments.
*/
// ==/WindhawkModSettings==

#include <windhawk_utils.h>

#include <windows.h>
#include <commctrl.h>
#include <shellapi.h>
#include <uxtheme.h>
#include <vssym32.h>
#include <windowsx.h>

#include <algorithm>
#include <atomic>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
// Settings

enum class Position { kNative, kLeftOfIcons };
enum class Smoothing { kDefault, kCrisp, kSmooth };
enum class Align { kCenter, kLeft, kRight };
enum class Action {
    kDefault,
    kNothing,
    kFlyout,
    kTaskbarMenu,
    kNotifications,
    kSettings,
    kAlarms,
    kCopy,
    kCommand,
    kNotificationCenter,
};

struct ClickSetting {
    Action action = Action::kDefault;
    std::wstring command;
};

struct Settings {
    Position position = Position::kNative;
    std::vector<std::wstring> lines;
    std::wstring face;
    int size = 0;
    int secondarySize = 0;
    bool bold = false;
    Smoothing smoothing = Smoothing::kDefault;
    bool hasPrimary = false;
    COLORREF primary = 0;
    bool hasSecondary = false;
    COLORREF secondary = 0;
    Align align = Align::kCenter;
    int padding = 6;
    int lineSpacing = 0;
    ClickSetting clicks[3];  // Left, right, middle.
    bool seconds = false;    // A line shows seconds.
};

// The name goes through "%s" so the suite build, which prefixes literal
// setting names, gives this part's reads its prefix too.
std::wstring ReadString(PCWSTR name) {
    PCWSTR value = Wh_GetStringSetting(L"%s", name);
    std::wstring text = value ? value : L"";
    Wh_FreeStringSetting(value);
    return text;
}

bool ParseColor(std::wstring text, COLORREF* color) {
    text.erase(std::remove_if(text.begin(), text.end(), iswspace), text.end());
    if (!text.empty() && text[0] == L'#') {
        text.erase(0, 1);
    }
    if (text.size() != 6 && text.size() != 8) {
        return false;
    }
    wchar_t* end = nullptr;
    unsigned long value = wcstoul(text.c_str(), &end, 16);
    if (!end || *end) {
        return false;
    }
    // #AARRGGBB, alpha first like the other Tourne mods; the alpha is ignored.
    *color = RGB((value >> 16) & 0xFF, (value >> 8) & 0xFF, value & 0xFF);
    return true;
}

Action ParseAction(std::wstring_view text) {
    if (text == L"nothing") return Action::kNothing;
    if (text == L"flyout") return Action::kFlyout;
    if (text == L"taskbarMenu") return Action::kTaskbarMenu;
    if (text == L"notifications") return Action::kNotifications;
    if (text == L"settings") return Action::kSettings;
    if (text == L"alarms") return Action::kAlarms;
    if (text == L"copy") return Action::kCopy;
    if (text == L"command") return Action::kCommand;
    if (text == L"notificationCenter") return Action::kNotificationCenter;
    return Action::kDefault;
}

std::shared_ptr<const Settings> LoadSettings() {
    auto s = std::make_shared<Settings>();
    s->position = ReadString(L"position") == L"leftOfIcons" ? Position::kLeftOfIcons
                                                             : Position::kNative;
    for (PCWSTR key : {L"lines.line1", L"lines.line2", L"lines.line3"}) {
        std::wstring line = ReadString(key);
        if (!line.empty()) {
            s->lines.push_back(line);
        }
    }
    s->face = ReadString(L"font.face");
    s->size = std::clamp(Wh_GetIntSetting(L"font.size"), 0, 200);
    s->secondarySize = std::clamp(Wh_GetIntSetting(L"font.secondarySize"), 0, 200);
    s->bold = Wh_GetIntSetting(L"font.bold");
    std::wstring smoothing = ReadString(L"font.smoothing");
    s->smoothing = smoothing == L"crisp"    ? Smoothing::kCrisp
                   : smoothing == L"smooth" ? Smoothing::kSmooth
                                            : Smoothing::kDefault;
    s->hasPrimary = ParseColor(ReadString(L"color.primary"), &s->primary);
    s->hasSecondary = ParseColor(ReadString(L"color.secondary"), &s->secondary);
    std::wstring align = ReadString(L"layout.align");
    s->align = align == L"left" ? Align::kLeft : align == L"right" ? Align::kRight : Align::kCenter;
    s->padding = std::clamp(Wh_GetIntSetting(L"layout.padding"), 0, 100);
    s->lineSpacing = std::clamp(Wh_GetIntSetting(L"layout.lineSpacing"), -50, 50);
    PCWSTR names[3] = {L"left", L"right", L"middle"};
    for (int i = 0; i < 3; i++) {
        s->clicks[i].action = ParseAction(ReadString((std::wstring(L"clicks.") + names[i]).c_str()));
        s->clicks[i].command =
            ReadString((std::wstring(L"clicks.") + names[i] + L"Command").c_str());
    }
    for (auto const& line : s->lines) {
        if (line.find(L"{time:long}") != std::wstring::npos) {
            s->seconds = true;
        }
        for (size_t at = line.find(L"{t:"); at != std::wstring::npos; at = line.find(L"{t:", at + 1)) {
            size_t end = line.find(L'}', at);
            if (line.substr(at, end == std::wstring::npos ? std::wstring::npos : end - at)
                    .find(L's') != std::wstring::npos) {
                s->seconds = true;
            }
        }
    }
    return s;
}

SRWLOCK g_settingsLock = SRWLOCK_INIT;
[[clang::no_destroy]] std::shared_ptr<const Settings> g_settings;

std::shared_ptr<const Settings> CurrentSettings() {
    AcquireSRWLockShared(&g_settingsLock);
    auto settings = g_settings;
    ReleaseSRWLockShared(&g_settingsLock);
    return settings;
}

////////////////////////////////////////////////////////////////////////////////
// Telling the time, the way the built-in clock does: the system time in the
// user's region formats.

std::wstring LocaleString(LCTYPE type) {
    WCHAR buffer[128];
    return GetLocaleInfoEx(LOCALE_NAME_USER_DEFAULT, type, buffer, ARRAYSIZE(buffer))
               ? buffer
               : L"";
}

std::wstring FormatTime(SYSTEMTIME const& time, PCWSTR picture, DWORD flags = 0) {
    WCHAR buffer[128];
    int n = GetTimeFormatEx(LOCALE_NAME_USER_DEFAULT, flags, &time, picture, buffer,
                            ARRAYSIZE(buffer));
    return n ? std::wstring(buffer) : L"";
}

std::wstring FormatDate(SYSTEMTIME const& time, PCWSTR picture, DWORD flags = 0) {
    WCHAR buffer[128];
    int n = GetDateFormatEx(LOCALE_NAME_USER_DEFAULT, flags, &time, picture, buffer,
                            ARRAYSIZE(buffer), nullptr);
    return n ? std::wstring(buffer) : L"";
}

std::wstring ExpandToken(std::wstring_view token, SYSTEMTIME const& time) {
    if (token == L"time") {
        std::wstring shortTime = LocaleString(LOCALE_SSHORTTIME);
        return FormatTime(time, shortTime.empty() ? nullptr : shortTime.c_str(),
                          shortTime.empty() ? TIME_NOSECONDS : 0);
    }
    if (token == L"time:long") return FormatTime(time, nullptr);
    if (token == L"date") return FormatDate(time, nullptr, DATE_SHORTDATE);
    if (token == L"date:long") return FormatDate(time, nullptr, DATE_LONGDATE);
    if (token == L"weekday") return FormatDate(time, L"dddd");
    if (token == L"weekday:short") return FormatDate(time, L"ddd");
    if (token.starts_with(L"t:")) return FormatTime(time, std::wstring(token.substr(2)).c_str());
    if (token.starts_with(L"d:")) return FormatDate(time, std::wstring(token.substr(2)).c_str());
    return L"{" + std::wstring(token) + L"}";
}

std::wstring ExpandLine(std::wstring const& line, SYSTEMTIME const& time) {
    std::wstring out;
    size_t at = 0;
    while (at < line.size()) {
        size_t open = line.find(L'{', at);
        size_t close = open == std::wstring::npos ? open : line.find(L'}', open);
        if (close == std::wstring::npos) {
            out += line.substr(at);
            break;
        }
        out += line.substr(at, open - at);
        out += ExpandToken(std::wstring_view(line).substr(open + 1, close - open - 1), time);
        at = close + 1;
    }
    return out;
}

std::vector<std::wstring> ShownLines(Settings const& settings) {
    SYSTEMTIME now;
    GetLocalTime(&now);
    std::vector<std::wstring> lines;
    for (auto const& line : settings.lines) {
        lines.push_back(ExpandLine(line, now));
    }
    return lines;
}

// Times and dates that come out widest (8 is the widest digit in most fonts,
// and the 28th of every month plus a full week covers every month and day
// name), so the clock keeps one width instead of shifting the taskbar.
std::vector<SYSTEMTIME> WidthSamples() {
    std::vector<SYSTEMTIME> samples;
    auto add = [&](WORD year, WORD month, WORD day) {
        for (WORD hour : {10, 12, 23}) {
            SYSTEMTIME t{};
            t.wYear = year;
            t.wMonth = month;
            t.wDay = day;
            t.wHour = hour;
            t.wMinute = 58;
            t.wSecond = 58;
            // The day of the week comes from the date.
            FILETIME ft;
            SystemTimeToFileTime(&t, &ft);
            FileTimeToSystemTime(&ft, &t);
            samples.push_back(t);
        }
    };
    for (WORD month = 1; month <= 12; month++) {
        add(2028, month, 28);
    }
    for (WORD day = 18; day <= 24; day++) {
        add(2028, 9, day);
    }
    return samples;
}

////////////////////////////////////////////////////////////////////////////////
// The clock windows

// Asks the clock for its size: returns MAKELONG(width, height).
constexpr UINT kCalcMinSize = WM_USER + 100;
constexpr UINT_PTR kTickTimer = 0x7C1C;

UINT g_msgApply;      // Sent to a clock: reload settings, resize, repaint.
UINT g_msgRearrange;  // Posted to a clock: put it where the settings say.
UINT g_msgDetach;     // Sent to a clock: hand it back to the built-in clock.

struct ClockFonts {
    HFONT primary = nullptr;
    HFONT secondary = nullptr;
    UINT dpi = 0;
    std::shared_ptr<const Settings> settings;
};

void FreeFonts(ClockFonts& fonts) {
    if (fonts.primary) {
        DeleteObject(fonts.primary);
    }
    if (fonts.secondary && fonts.secondary != fonts.primary) {
        DeleteObject(fonts.secondary);
    }
    fonts.primary = fonts.secondary = nullptr;
}

// Per-clock state, owned by the clock's thread through the subclass.
struct ClockState {
    ClockFonts fonts;
    int width = 0;
    int height = 0;
    bool arranging = false;
};

LOGFONTW BaseFont(HWND hwnd, HDC dc, UINT dpi) {
    LOGFONTW font{};
    bool found = false;
    if (HTHEME theme = OpenThemeData(hwnd, L"Clock")) {
        found = SUCCEEDED(GetThemeFont(theme, dc, CLP_TIME, 0, TMT_FONT, &font));
        CloseThemeData(theme);
    }
    if (!found) {
        NONCLIENTMETRICSW metrics{sizeof(metrics)};
        SystemParametersInfoForDpi(SPI_GETNONCLIENTMETRICS, sizeof(metrics), &metrics, 0, dpi);
        font = metrics.lfStatusFont;
    }
    return font;
}

HFONT MakeFont(HWND hwnd, HDC dc, Settings const& s, UINT dpi, int size, bool bold) {
    LOGFONTW font = BaseFont(hwnd, dc, dpi);
    if (!s.face.empty()) {
        wcsncpy_s(font.lfFaceName, s.face.c_str(), _TRUNCATE);
    }
    if (size > 0) {
        font.lfHeight = -MulDiv(size, dpi, USER_DEFAULT_SCREEN_DPI);
        font.lfWidth = 0;
    }
    if (bold) {
        font.lfWeight = FW_BOLD;
    }
    if (s.smoothing == Smoothing::kCrisp) {
        font.lfQuality = NONANTIALIASED_QUALITY;
    } else if (s.smoothing == Smoothing::kSmooth) {
        font.lfQuality = ANTIALIASED_QUALITY;
    }
    return CreateFontIndirectW(&font);
}

void EnsureFonts(HWND hwnd, HDC dc, ClockState& state,
                 std::shared_ptr<const Settings> const& settings) {
    UINT dpi = GetDpiForWindow(hwnd);
    if (state.fonts.primary && state.fonts.dpi == dpi && state.fonts.settings == settings) {
        return;
    }
    FreeFonts(state.fonts);
    Settings const& s = *settings;
    state.fonts.primary = MakeFont(hwnd, dc, s, dpi, s.size, s.bold);
    state.fonts.secondary = s.secondarySize > 0 || s.bold
                                ? MakeFont(hwnd, dc, s, dpi, s.secondarySize ? s.secondarySize : s.size, false)
                                : state.fonts.primary;
    state.fonts.dpi = dpi;
    state.fonts.settings = settings;
}

int Scaled(HWND hwnd, int value) {
    return MulDiv(value, GetDpiForWindow(hwnd), USER_DEFAULT_SCREEN_DPI);
}

SIZE MeasureText(HDC dc, HFONT font, std::wstring const& text) {
    HGDIOBJ old = SelectObject(dc, font);
    SIZE size{};
    GetTextExtentPoint32W(dc, text.empty() ? L" " : text.c_str(),
                          text.empty() ? 1 : (int)text.size(), &size);
    if (text.empty()) {
        size.cx = 0;
    }
    SelectObject(dc, old);
    return size;
}

// The clock's size for the current settings and DPI, from the widest samples.
void Measure(HWND hwnd, ClockState& state) {
    auto settings = CurrentSettings();
    HDC dc = GetDC(hwnd);
    EnsureFonts(hwnd, dc, state, settings);
    Settings const& s = *settings;
    int width = 0;
    int height = 0;
    auto samples = WidthSamples();
    for (size_t i = 0; i < s.lines.size(); i++) {
        HFONT font = i == 0 ? state.fonts.primary : state.fonts.secondary;
        int lineWidth = 0;
        int lineHeight = 0;
        for (auto const& sample : samples) {
            SIZE size = MeasureText(dc, font, ExpandLine(s.lines[i], sample));
            lineWidth = std::max(lineWidth, (int)size.cx);
            lineHeight = std::max(lineHeight, (int)size.cy);
        }
        width = std::max(width, lineWidth);
        height += lineHeight + (i ? Scaled(hwnd, s.lineSpacing) : 0);
    }
    ReleaseDC(hwnd, dc);
    state.width = width + 2 * Scaled(hwnd, s.padding);
    state.height = std::max(height, 1);
}

COLORREF ThemeTextColor(HWND hwnd) {
    COLORREF color = GetSysColor(COLOR_BTNTEXT);
    if (HTHEME theme = OpenThemeData(hwnd, L"Clock")) {
        GetThemeColor(theme, CLP_TIME, 0, TMT_TEXTCOLOR, &color);
        CloseThemeData(theme);
    }
    return color;
}

void Paint(HWND hwnd, ClockState& state) {
    PAINTSTRUCT ps;
    HDC dc = BeginPaint(hwnd, &ps);
    RECT client;
    GetClientRect(hwnd, &client);

    auto settings = CurrentSettings();
    Settings const& s = *settings;
    EnsureFonts(hwnd, dc, state, settings);

    HDC buffered = nullptr;
    HPAINTBUFFER buffer =
        BeginBufferedPaint(dc, &client, BPBF_TOPDOWNDIB, nullptr, &buffered);
    HDC target = buffer ? buffered : dc;
    DrawThemeParentBackground(hwnd, target, &client);

    HTHEME theme = OpenThemeData(hwnd, L"Clock");
    COLORREF themeColor = ThemeTextColor(hwnd);
    COLORREF primary = s.hasPrimary ? s.primary : themeColor;
    COLORREF secondary = s.hasSecondary ? s.secondary : primary;

    std::vector<std::wstring> lines = ShownLines(s);
    std::vector<SIZE> sizes;
    int total = 0;
    for (size_t i = 0; i < lines.size(); i++) {
        sizes.push_back(MeasureText(target, i == 0 ? state.fonts.primary : state.fonts.secondary,
                                    lines[i]));
        total += sizes.back().cy + (i ? Scaled(hwnd, s.lineSpacing) : 0);
    }

    int padding = Scaled(hwnd, s.padding);
    int y = client.top + (client.bottom - client.top - total) / 2;
    for (size_t i = 0; i < lines.size(); i++) {
        if (i) {
            y += Scaled(hwnd, s.lineSpacing);
        }
        RECT line{client.left + padding, y, client.right - padding, y + sizes[i].cy};
        UINT format = DT_SINGLELINE | DT_NOPREFIX | DT_VCENTER |
                      (s.align == Align::kLeft    ? DT_LEFT
                       : s.align == Align::kRight ? DT_RIGHT
                                                  : DT_CENTER);
        HGDIOBJ old = SelectObject(target, i == 0 ? state.fonts.primary : state.fonts.secondary);
        COLORREF color = i == 0 ? primary : secondary;
        if (theme && buffer) {
            // Composited text keeps its alpha over a translucent taskbar.
            DTTOPTS options{sizeof(options)};
            options.dwFlags = DTT_COMPOSITED | DTT_TEXTCOLOR;
            options.crText = color;
            DrawThemeTextEx(theme, target, CLP_TIME, 0, lines[i].c_str(), -1, format, &line,
                            &options);
        } else {
            SetBkMode(target, TRANSPARENT);
            SetTextColor(target, color);
            DrawTextW(target, lines[i].c_str(), -1, &line, format);
        }
        SelectObject(target, old);
        y += sizes[i].cy;
    }

    if (theme) {
        CloseThemeData(theme);
    }
    if (buffer) {
        EndBufferedPaint(buffer, TRUE);
    }
    EndPaint(hwnd, &ps);
}

void ScheduleTick(HWND hwnd) {
    auto settings = CurrentSettings();
    SYSTEMTIME now;
    GetLocalTime(&now);
    UINT wait = 1000 - now.wMilliseconds;
    if (!settings->seconds) {
        wait += (59 - now.wSecond) * 1000;
    }
    SetTimer(hwnd, kTickTimer, wait + 15, nullptr);
}

// Makes the tray lay out again, which asks every clock for its size.
void RelayoutTray(HWND clock) {
    HWND notify = GetParent(clock);
    RECT rc;
    if (notify && GetClientRect(notify, &rc)) {
        SendMessageW(notify, WM_SIZE, SIZE_RESTORED, MAKELPARAM(rc.right, rc.bottom));
    }
    if (HWND taskbar = GetAncestor(clock, GA_ROOT)) {
        GetClientRect(taskbar, &rc);
        SendMessageW(taskbar, WM_SIZE, SIZE_RESTORED, MAKELPARAM(rc.right, rc.bottom));
    }
}

// Left of the tray icons: the clock moves to where the first of its left-hand
// neighbors starts, and they move right by the clock's width. Runs after the
// tray's own layout, on every move it makes.
void Rearrange(HWND clock, ClockState& state) {
    auto settings = CurrentSettings();
    HWND notify = GetParent(clock);
    RECT clockRect;
    if (settings->position != Position::kLeftOfIcons || !notify || state.arranging ||
        !GetWindowRect(clock, &clockRect)) {
        return;
    }
    RECT notifyRect;
    GetWindowRect(notify, &notifyRect);
    if (notifyRect.right - notifyRect.left < notifyRect.bottom - notifyRect.top) {
        return;  // Vertical taskbar: left as it is.
    }

    struct Child {
        HWND hwnd;
        RECT rc;
    };
    std::vector<Child> before;
    int start = clockRect.left;
    for (HWND child = GetWindow(notify, GW_CHILD); child; child = GetWindow(child, GW_HWNDNEXT)) {
        RECT rc;
        if (child == clock || !IsWindowVisible(child) || !GetWindowRect(child, &rc) ||
            rc.left >= clockRect.left) {
            continue;
        }
        before.push_back({child, rc});
        start = std::min(start, (int)rc.left);
    }
    if (before.empty()) {
        return;  // Already first.
    }

    int width = clockRect.right - clockRect.left;
    state.arranging = true;
    HDWP defer = BeginDeferWindowPos((int)before.size() + 1);
    auto move = [&](HWND hwnd, RECT const& rc, int left) {
        POINT pt{left, rc.top};
        ScreenToClient(notify, &pt);
        if (defer) {
            defer = DeferWindowPos(defer, hwnd, nullptr, pt.x, pt.y, 0, 0,
                                   SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
        }
    };
    move(clock, clockRect, start);
    for (auto const& child : before) {
        move(child.hwnd, child.rc, child.rc.left + width);
    }
    if (defer) {
        EndDeferWindowPos(defer);
    }
    state.arranging = false;
}

void CopyShownText() {
    auto lines = ShownLines(*CurrentSettings());
    std::wstring text;
    for (size_t i = 0; i < lines.size(); i++) {
        text += (i ? L" " : L"") + lines[i];
    }
    if (!OpenClipboard(nullptr)) {
        return;
    }
    EmptyClipboard();
    if (HGLOBAL memory = GlobalAlloc(GMEM_MOVEABLE, (text.size() + 1) * sizeof(WCHAR))) {
        memcpy(GlobalLock(memory), text.c_str(), (text.size() + 1) * sizeof(WCHAR));
        GlobalUnlock(memory);
        if (!SetClipboardData(CF_UNICODETEXT, memory)) {
            GlobalFree(memory);
        }
    }
    CloseClipboard();
}

void RunCommand(std::wstring const& command) {
    std::wstring file = command;
    std::wstring arguments;
    size_t start = file.find_first_not_of(L" \t");
    if (start == std::wstring::npos) {
        return;
    }
    file.erase(0, start);
    if (file[0] == L'"') {
        size_t close = file.find(L'"', 1);
        if (close != std::wstring::npos) {
            arguments = file.substr(close + 1);
            file = file.substr(1, close - 1);
        }
    }
    ShellExecuteW(nullptr, L"open", file.c_str(), arguments.empty() ? nullptr : arguments.c_str(),
                  nullptr, SW_SHOWNORMAL);
}

void PressKeys(std::initializer_list<WORD> keys) {
    std::vector<INPUT> inputs;
    for (WORD key : keys) {
        INPUT input{INPUT_KEYBOARD};
        input.ki.wVk = key;
        inputs.push_back(input);
    }
    for (auto it = std::rbegin(keys); it != std::rend(keys); ++it) {
        INPUT input{INPUT_KEYBOARD};
        input.ki.wVk = *it;
        input.ki.dwFlags = KEYEVENTF_KEYUP;
        inputs.push_back(input);
    }
    SendInput((UINT)inputs.size(), inputs.data(), sizeof(INPUT));
}

// System Icons' notification centre, in its own process or in Explorer. The
// click came to Explorer, which may hand the foreground on.
bool ShowTourneNotificationCenter() {
    HWND host = nullptr;
    EnumWindows(
        [](HWND hwnd, LPARAM lParam) -> BOOL {
            WCHAR name[96];
            if (GetClassNameW(hwnd, name, ARRAYSIZE(name)) &&
                wcsncmp(name, L"TourneTraySystemIcons_", 22) == 0) {
                *(HWND*)lParam = hwnd;
                return FALSE;
            }
            return TRUE;
        },
        (LPARAM)&host);
    if (!host) {
        return false;
    }
    DWORD pid = 0;
    GetWindowThreadProcessId(host, &pid);
    AllowSetForegroundWindow(pid);
    PostMessageW(host, RegisterWindowMessageW(L"TourneTray_NotificationCenter"), 0, 0);
    return true;
}

// Runs a click's action. The clock's own behavior for another button is
// replayed through the window procedure below the subclass.
void RunAction(HWND hwnd, ClickSetting const& click, LPARAM point) {
    switch (click.action) {
        case Action::kDefault:
        case Action::kNothing:
            break;
        case Action::kFlyout:
            DefSubclassProc(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, point);
            DefSubclassProc(hwnd, WM_LBUTTONUP, 0, point);
            break;
        case Action::kTaskbarMenu: {
            POINT pt{GET_X_LPARAM(point), GET_Y_LPARAM(point)};
            ClientToScreen(hwnd, &pt);
            DefSubclassProc(hwnd, WM_CONTEXTMENU, (WPARAM)hwnd, MAKELPARAM(pt.x, pt.y));
            break;
        }
        case Action::kNotifications:
            PressKeys({VK_LWIN, 'N'});
            break;
        case Action::kSettings:
            ShellExecuteW(nullptr, L"open", L"ms-settings:dateandtime", nullptr, nullptr, SW_SHOWNORMAL);
            break;
        case Action::kAlarms:
            ShellExecuteW(nullptr, L"open", L"ms-clock:", nullptr, nullptr, SW_SHOWNORMAL);
            break;
        case Action::kCopy:
            CopyShownText();
            break;
        case Action::kCommand:
            RunCommand(click.command);
            break;
        case Action::kNotificationCenter:
            if (!ShowTourneNotificationCenter()) {
                PressKeys({VK_LWIN, 'N'});
            }
            break;
    }
}

int ButtonOf(UINT msg) {
    switch (msg) {
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_LBUTTONDBLCLK:
            return 0;
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_RBUTTONDBLCLK:
            return 1;
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
        case WM_MBUTTONDBLCLK:
            return 2;
    }
    return -1;
}

LRESULT CALLBACK ClockSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam,
                                   DWORD_PTR refData) {
    auto* state = (ClockState*)refData;

    if (msg == g_msgApply && g_msgApply) {
        state->fonts.settings.reset();
        Measure(hwnd, *state);
        ScheduleTick(hwnd);
        RelayoutTray(hwnd);
        PostMessageW(hwnd, g_msgRearrange, 0, 0);
        InvalidateRect(hwnd, nullptr, TRUE);
        return 0;
    }
    if (msg == g_msgRearrange && g_msgRearrange) {
        Rearrange(hwnd, *state);
        return 0;
    }
    if (msg == g_msgDetach && g_msgDetach) {
        // On the clock's own thread, so no message can reach the state after
        // it's freed. The tray then asks the built-in clock for its size.
        KillTimer(hwnd, kTickTimer);
        WindhawkUtils::RemoveWindowSubclassFromAnyThread(hwnd, ClockSubclassProc);
        FreeFonts(state->fonts);
        delete state;
        RelayoutTray(hwnd);
        InvalidateRect(hwnd, nullptr, TRUE);
        return 0;
    }

    switch (msg) {
        case kCalcMinSize:
            if (!state->width) {
                Measure(hwnd, *state);
            }
            return MAKELONG(state->width, state->height);

        case WM_PAINT:
            Paint(hwnd, *state);
            return 0;

        case WM_ERASEBKGND:
            return 1;

        case WM_TIMER:
            if (wParam == kTickTimer) {
                InvalidateRect(hwnd, nullptr, FALSE);
                ScheduleTick(hwnd);
                return 0;
            }
            break;

        case WM_TIMECHANGE:
        case WM_SETTINGCHANGE:
        case WM_DPICHANGED_AFTERPARENT:
            // Time zone, region formats or scaling: measure again.
            state->fonts.settings.reset();
            state->width = 0;
            PostMessageW(hwnd, g_msgApply, 0, 0);
            break;

        case WM_WINDOWPOSCHANGED:
            if (!state->arranging && !(((WINDOWPOS*)lParam)->flags & SWP_NOMOVE)) {
                PostMessageW(hwnd, g_msgRearrange, 0, 0);
            }
            break;

        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_LBUTTONDBLCLK:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_RBUTTONDBLCLK:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
        case WM_MBUTTONDBLCLK: {
            auto settings = CurrentSettings();
            ClickSetting const& click = settings->clicks[ButtonOf(msg)];
            if (click.action == Action::kDefault) {
                break;
            }
            if (msg == WM_LBUTTONUP || msg == WM_RBUTTONUP || msg == WM_MBUTTONUP) {
                RunAction(hwnd, click, lParam);
            }
            return 0;
        }

        case WM_CONTEXTMENU:
            // Only a right click that was taken over reaches here as its own
            // message; the keyboard's menu key still shows the taskbar menu.
            if (CurrentSettings()->clicks[1].action != Action::kDefault && lParam != -1) {
                return 0;
            }
            break;

        case WM_NCDESTROY:
            KillTimer(hwnd, kTickTimer);
            FreeFonts(state->fonts);
            delete state;
            break;
    }
    return DefSubclassProc(hwnd, msg, wParam, lParam);
}

////////////////////////////////////////////////////////////////////////////////
// Finding the clocks: one on each taskbar, found again when taskbars come and
// go.

SRWLOCK g_clocksLock = SRWLOCK_INIT;
[[clang::no_destroy]] std::vector<HWND> g_clocks;
HANDLE g_thread;
HANDLE g_stop;

bool IsOwnProcessWindow(HWND hwnd) {
    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    return pid == GetCurrentProcessId();
}

std::vector<HWND> FindClocks() {
    std::vector<HWND> clocks;
    for (PCWSTR taskbarClass : {L"Shell_TrayWnd", L"Shell_SecondaryTrayWnd"}) {
        for (HWND taskbar = FindWindowExW(nullptr, nullptr, taskbarClass, nullptr); taskbar;
             taskbar = FindWindowExW(nullptr, taskbar, taskbarClass, nullptr)) {
            if (!IsOwnProcessWindow(taskbar)) {
                continue;
            }
            HWND notify = FindWindowExW(taskbar, nullptr, L"TrayNotifyWnd", nullptr);
            HWND clock = notify ? FindWindowExW(notify, nullptr, L"TrayClockWClass", nullptr) : nullptr;
            if (clock) {
                clocks.push_back(clock);
            }
        }
    }
    return clocks;
}

void HookClocks() {
    AcquireSRWLockExclusive(&g_clocksLock);
    std::erase_if(g_clocks, [](HWND hwnd) { return !IsWindow(hwnd); });
    for (HWND clock : FindClocks()) {
        if (std::find(g_clocks.begin(), g_clocks.end(), clock) != g_clocks.end()) {
            continue;
        }
        auto* state = new ClockState();
        if (WindhawkUtils::SetWindowSubclassFromAnyThread(clock, ClockSubclassProc,
                                                          (DWORD_PTR)state)) {
            g_clocks.push_back(clock);
            SendMessageW(clock, g_msgApply, 0, 0);
        } else {
            delete state;
        }
    }
    ReleaseSRWLockExclusive(&g_clocksLock);
}

void UnhookClocks() {
    AcquireSRWLockExclusive(&g_clocksLock);
    std::vector<HWND> clocks;
    clocks.swap(g_clocks);
    ReleaseSRWLockExclusive(&g_clocksLock);
    for (HWND clock : clocks) {
        if (IsWindow(clock)) {
            DWORD_PTR result;
            SendMessageTimeoutW(clock, g_msgDetach, 0, 0, SMTO_ABORTIFHUNG, 2000, &result);
        }
    }
}

void ApplyToClocks() {
    AcquireSRWLockShared(&g_clocksLock);
    std::vector<HWND> clocks = g_clocks;
    ReleaseSRWLockShared(&g_clocksLock);
    for (HWND clock : clocks) {
        DWORD_PTR result;
        SendMessageTimeoutW(clock, g_msgApply, 0, 0, SMTO_ABORTIFHUNG, 1000, &result);
    }
}

DWORD WINAPI ThreadProc(LPVOID) {
    // Quick while Explorer starts, then a check every few seconds for
    // taskbars that appeared (a monitor plugged in, Explorer restarted).
    DWORD wait = 500;
    do {
        HookClocks();
        wait = FindClocks().empty() ? 500 : 3000;
    } while (WaitForSingleObject(g_stop, wait) == WAIT_TIMEOUT);
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Mod lifecycle

BOOL Wh_ModInit() {
    Wh_Log(L">");
    g_msgApply = RegisterWindowMessageW(L"TourneTrayClock_Apply");
    g_msgRearrange = RegisterWindowMessageW(L"TourneTrayClock_Rearrange");
    g_msgDetach = RegisterWindowMessageW(L"TourneTrayClock_Detach");
    auto settings = LoadSettings();
    AcquireSRWLockExclusive(&g_settingsLock);
    g_settings = settings;
    ReleaseSRWLockExclusive(&g_settingsLock);

    g_stop = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    g_thread = CreateThread(nullptr, 0, ThreadProc, nullptr, 0, nullptr);
    return g_thread != nullptr;
}

void Wh_ModUninit() {
    Wh_Log(L">");
    SetEvent(g_stop);
    if (g_thread) {
        WaitForSingleObject(g_thread, 5000);
        CloseHandle(g_thread);
        g_thread = nullptr;
    }
    CloseHandle(g_stop);
    g_stop = nullptr;
    UnhookClocks();
}

void Wh_ModSettingsChanged() {
    Wh_Log(L">");
    auto settings = LoadSettings();
    AcquireSRWLockExclusive(&g_settingsLock);
    g_settings = settings;
    ReleaseSRWLockExclusive(&g_settingsLock);
    ApplyToClocks();
}
