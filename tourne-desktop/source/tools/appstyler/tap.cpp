// Tourne App Styler, the part that goes into an app (Settings, Windows Security, Calculator...): Windows' XAML
// diagnostics (InitializeXamlDiagnosticsEx, what Visual Studio's live inspector uses) loads this DLL into the app
// and hands it the app's XAML. It sets the app's theme colours once, in the app's own theme dictionaries, and has
// each window take its theme again so everything already drawn follows; nothing is watched per element after
// that, so the app runs as fast as it did. The colours come as the attach's initialization text:
// "ground=#121C21;raised=#1E2F36;text=#66EA97;muted=#5D8D6F;accent=#15FFA2;line=#4F8F6A" (any left out stay).

#include <windows.h>
#include <ocidl.h>
#include <xamlom.h>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <set>
#include <string>
#include <thread>
#include <vector>

#undef GetCurrentTime  // (winbase.h's macro, which the XAML headers name a method)

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.UI.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Media.h>
#include <winrt/Windows.UI.Xaml.Media.Imaging.h>
#include <winrt/Windows.UI.Xaml.Shapes.h>

namespace wux = winrt::Windows::UI::Xaml;

namespace {

// {6F3C2B71-3D44-4E7B-9C1A-5B8E2F0D7A41}
constexpr CLSID kClsid = {0x6f3c2b71, 0x3d44, 0x4e7b, {0x9c, 0x1a, 0x5b, 0x8e, 0x2f, 0x0d, 0x7a, 0x41}};

std::atomic<long> g_objects{0};

// What happened, as debug output and in appstyler.log beside this DLL (while it's being tried out).
void Log(std::wstring const& line) {
    OutputDebugStringW((L"[tourne-appstyler] " + line + L"\n").c_str());
    HMODULE self = nullptr;
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCWSTR)&Log, &self);
    wchar_t path[MAX_PATH];
    if (!GetModuleFileNameW(self, path, MAX_PATH)) return;
    std::wstring file = path;
    file = file.substr(0, file.find_last_of(L'\\') + 1) + L"appstyler.log";
    HANDLE f = CreateFileW(file.c_str(), FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_ALWAYS, 0, nullptr);
    if (f == INVALID_HANDLE_VALUE) return;
    wchar_t exe[MAX_PATH] = L"";
    GetModuleFileNameW(nullptr, exe, MAX_PATH);
    std::wstring text = std::wstring(wcsrchr(exe, L'\\') ? wcsrchr(exe, L'\\') + 1 : exe) + L" " +
                        std::to_wstring(GetCurrentProcessId()) + L": " + line + L"\r\n";
    int n = WideCharToMultiByte(CP_UTF8, 0, text.c_str(), (int)text.size(), nullptr, 0, nullptr, nullptr);
    std::string utf8(n, '\0');
    WideCharToMultiByte(CP_UTF8, 0, text.c_str(), (int)text.size(), utf8.data(), n, nullptr, nullptr);
    DWORD wrote = 0;
    WriteFile(f, utf8.data(), (DWORD)utf8.size(), &wrote, nullptr);
    CloseHandle(f);
}

// The roles, and the app's resources that take each (the Fluent ones WinUI 2 and the system's styles use).
enum Role { Ground, Raised, Text, Muted, Accent, Line, kRoles };
constexpr const wchar_t* kRoleNames[kRoles] = {L"ground", L"raised", L"text", L"muted", L"accent", L"line"};

struct Colours {
    uint32_t rgb[kRoles] = {};
    bool set[kRoles] = {};
    bool square = false;  // "square=1": every corner square (the corner radius resources at 0)
    std::wstring font;    // "font=Name": the text's font (icons keep theirs)
    bool hideIcons = false;  // "icons=hide": the app's own pictures (its coloured icons, device pictures) gone
    double fontGrid = 0;  // "fontGrid=N": a pixel font's own size (13 for Cozette); text sizes snap to its multiples
};

Colours Parse(std::wstring const& text) {
    Colours c;
    size_t at = 0;
    while (at < text.size()) {
        size_t end = text.find(L';', at);
        std::wstring item = text.substr(at, end == std::wstring::npos ? std::wstring::npos : end - at);
        at = end == std::wstring::npos ? text.size() : end + 1;
        size_t eq = item.find(L'='), hash = item.find(L'#');
        if (eq != std::wstring::npos && item.substr(0, eq) == L"square") c.square = item.substr(eq + 1) == L"1";
        if (eq != std::wstring::npos && item.substr(0, eq) == L"font") c.font = item.substr(eq + 1);
        if (eq != std::wstring::npos && item.substr(0, eq) == L"icons") c.hideIcons = item.substr(eq + 1) == L"hide";
        if (eq != std::wstring::npos && item.substr(0, eq) == L"fontGrid") c.fontGrid = _wtof(item.substr(eq + 1).c_str());
        if (eq == std::wstring::npos || hash == std::wstring::npos || hash + 7 > item.size()) continue;
        std::wstring name = item.substr(0, eq);
        for (int r = 0; r < kRoles; r++)
            if (name == kRoleNames[r]) c.rgb[r] = wcstoul(item.substr(hash + 1, 6).c_str(), nullptr, 16), c.set[r] = true;
    }
    return c;
}

struct Key {
    const wchar_t* name;
    Role role;
    bool color;  // a Color, not a brush
};
constexpr Key kKeys[] = {
    {L"ApplicationPageBackgroundThemeBrush", Ground, false},
    {L"SolidBackgroundFillColorBaseBrush", Ground, false},
    {L"SolidBackgroundFillColorBaseAltBrush", Ground, false},
    {L"SolidBackgroundFillColorBase", Ground, true},
    {L"NavigationViewDefaultPaneBackground", Ground, false},
    {L"NavigationViewExpandedPaneBackground", Ground, false},
    {L"NavigationViewTopPaneBackground", Ground, false},
    {L"TextOnAccentFillColorPrimaryBrush", Ground, false},
    {L"SolidBackgroundFillColorSecondaryBrush", Raised, false},
    {L"SolidBackgroundFillColorTertiaryBrush", Raised, false},
    {L"NavigationViewContentBackground", Ground, false},  // (the page area: a see-through layer over the ground)
    {L"LayerFillColorDefaultBrush", Ground, false},
    {L"LayerFillColorAltBrush", Ground, false},
    {L"LayerOnMicaBaseAltFillColorDefaultBrush", Ground, false},
    {L"LayerOnMicaBaseAltFillColorSecondaryBrush", Ground, false},
    {L"LayerOnAcrylicFillColorDefaultBrush", Ground, false},
    {L"CardBackgroundFillColorDefaultBrush", Raised, false},
    {L"CardBackgroundFillColorSecondaryBrush", Raised, false},
    {L"ExpanderHeaderBackground", Raised, false},
    {L"ExpanderContentBackground", Raised, false},
    {L"ControlFillColorDefaultBrush", Raised, false},
    {L"ControlFillColorSecondaryBrush", Raised, false},
    {L"TextFillColorPrimaryBrush", Text, false},
    {L"TextFillColorPrimary", Text, true},
    {L"ApplicationForegroundThemeBrush", Text, false},
    {L"DefaultTextForegroundThemeBrush", Text, false},
    {L"SystemControlForegroundBaseHighBrush", Text, false},
    {L"SystemControlPageTextBaseHighBrush", Text, false},
    {L"SystemControlBackgroundBaseHighBrush", Text, false},
    {L"SystemBaseHighColor", Text, true},
    {L"NavigationViewItemForeground", Text, false},
    {L"NavigationViewItemForegroundPointerOver", Text, false},
    {L"NavigationViewItemForegroundPressed", Text, false},
    {L"NavigationViewItemForegroundSelected", Text, false},
    {L"NavigationViewItemForegroundSelectedPointerOver", Text, false},
    {L"NavigationViewItemForegroundSelectedPressed", Text, false},
    {L"NavigationViewItemHeaderForeground", Text, false},
    {L"TextControlForeground", Text, false},
    {L"ListViewItemForeground", Text, false},
    {L"ListViewItemForegroundPointerOver", Text, false},
    {L"ListViewItemForegroundSelected", Text, false},
    {L"GridViewItemForeground", Text, false},
    {L"GridViewItemForegroundPointerOver", Text, false},
    {L"GridViewItemForegroundSelected", Text, false},
    {L"ButtonForeground", Text, false},
    {L"HyperlinkButtonForeground", Accent, false},
    {L"NavigationViewSelectionIndicatorForeground", Accent, false},
    {L"TextFillColorSecondaryBrush", Muted, false},
    {L"TextFillColorTertiaryBrush", Muted, false},
    {L"SystemControlForegroundBaseMediumBrush", Muted, false},
    {L"SystemControlPageTextBaseMediumBrush", Muted, false},
    {L"SystemBaseMediumColor", Muted, true},
    {L"AccentFillColorDefaultBrush", Accent, false},
    {L"AccentFillColorSecondaryBrush", Accent, false},
    {L"AccentFillColorTertiaryBrush", Accent, false},
    {L"AccentTextFillColorPrimaryBrush", Accent, false},
    {L"AccentTextFillColorSecondaryBrush", Accent, false},
    {L"SystemControlHighlightAccentBrush", Accent, false},
    {L"SystemControlForegroundAccentBrush", Accent, false},
    {L"SystemAccentColor", Accent, true},
    {L"SystemAccentColorLight1", Accent, true},
    {L"SystemAccentColorLight2", Accent, true},
    {L"SystemAccentColorLight3", Accent, true},
    {L"SystemAccentColorDark1", Accent, true},
    {L"SystemAccentColorDark2", Accent, true},
    {L"SystemAccentColorDark3", Accent, true},
    {L"SettingsCardBackground", Raised, false},
    {L"SettingsCardBackgroundPointerOver", Raised, false},
    {L"SettingsCardBorderBrush", Line, false},
    {L"SettingsCardForeground", Text, false},
    // (each state too: hovering or pressing puts the state's brush on, which wins over anything set directly)
    {L"SettingsCardForegroundPointerOver", Text, false},
    {L"SettingsCardForegroundPressed", Text, false},
    {L"SettingsCardForegroundDisabled", Muted, false},
    {L"SettingsCardBackgroundPressed", Raised, false},
    {L"SettingsCardBorderBrushPointerOver", Line, false},
    {L"SettingsCardBorderBrushPressed", Line, false},
    {L"ExpanderHeaderForeground", Text, false},
    {L"ExpanderHeaderForegroundPointerOver", Text, false},
    {L"ExpanderHeaderForegroundPressed", Text, false},
    {L"ExpanderHeaderBackgroundPressed", Raised, false},
    {L"ExpanderChevronForeground", Text, false},
    {L"ExpanderChevronPointerOverForeground", Text, false},
    {L"ButtonForegroundPointerOver", Text, false},
    {L"ButtonForegroundPressed", Text, false},
    {L"ButtonBackground", Raised, false},
    {L"ButtonBackgroundPointerOver", Raised, false},
    {L"ButtonBackgroundPressed", Ground, false},
    {L"ButtonBorderBrush", Line, false},
    {L"AccentButtonBackground", Accent, false},  // (an accent button: the accent's fill, the ground's text)
    {L"AccentButtonBackgroundPointerOver", Accent, false},
    {L"AccentButtonBackgroundPressed", Accent, false},
    {L"AccentButtonForeground", Ground, false},
    {L"AccentButtonForegroundPointerOver", Ground, false},
    {L"AccentButtonForegroundPressed", Ground, false},
    {L"AccentButtonBorderBrush", Accent, false},
    {L"HyperlinkButtonForegroundPointerOver", Accent, false},
    {L"HyperlinkButtonForegroundPressed", Accent, false},
    {L"TextFillColorSecondary", Muted, true},
    {L"SubtleFillColorSecondaryBrush", Raised, false},
    {L"SubtleFillColorTertiaryBrush", Raised, false},
    {L"ControlFillColorTertiaryBrush", Raised, false},
    {L"ExpanderHeaderBackgroundPointerOver", Raised, false},
    {L"ExpanderHeaderBorderBrush", Line, false},
    {L"ExpanderHeaderBorderPointerOverBrush", Line, false},
    {L"ExpanderHeaderBorderPressedBrush", Line, false},
    {L"ExpanderContentBorderBrush", Raised, false},  // (an open section as one card: its header has no outline)
    {L"ToggleSwitchStrokeOff", Line, false},
    {L"ToggleSwitchStrokeOffPointerOver", Line, false},
    {L"ToggleSwitchKnobFillOff", Muted, false},
    {L"ToggleSwitchKnobFillOffPointerOver", Text, false},
    {L"ToggleSwitchFillOn", Accent, false},
    {L"ToggleSwitchFillOnPointerOver", Accent, false},
    {L"ToggleSwitchKnobFillOn", Ground, false},
    {L"ToggleSwitchContentForeground", Text, false},
    {L"ToggleSwitchHeaderForeground", Text, false},
    {L"SystemControlFocusVisualPrimaryBrush", Accent, false},  // (the focus ring: the accent outside, the ground inside)
    {L"SystemControlFocusVisualSecondaryBrush", Ground, false},
    {L"FocusStrokeColorOuterBrush", Accent, false},
    {L"FocusStrokeColorInnerBrush", Ground, false},
    {L"FocusStrokeColorOuter", Accent, true},
    {L"FocusStrokeColorInner", Ground, true},
    {L"CardStrokeColorDefaultBrush", Line, false},
    {L"ControlStrokeColorDefaultBrush", Line, false},
    {L"ControlStrokeColorSecondaryBrush", Line, false},
    {L"DividerStrokeColorDefaultBrush", Line, false},
    {L"SurfaceStrokeColorDefaultBrush", Line, false},
};

winrt::Windows::UI::Color ColorOf(uint32_t rgb) { return {255, (uint8_t)(rgb >> 16), (uint8_t)(rgb >> 8), (uint8_t)rgb}; }

// The colours into the app's theme dictionaries (Dark, Light and Default, made if the app has none): the app's own
// top level, which wins over the styles it merges in.
void SetResources(Colours const& c) {
    auto resources = wux::Application::Current().Resources();
    // Square corners: the radius resources the styles take their corners from (not per theme).
    if (c.square)
        for (const wchar_t* name : {L"ControlCornerRadius", L"OverlayCornerRadius", L"CardCornerRadius", L"PopupCornerRadius",
                                    L"FlyoutCornerRadius", L"ContentDialogCornerRadius", L"ToolTipCornerRadius",
                                    L"NavigationViewContentGridCornerRadius", L"ComboBoxItemCornerRadius", L"ListViewItemCornerRadius"})
            resources.Insert(winrt::box_value(name), winrt::box_value(wux::CornerRadius{0, 0, 0, 0}));
    // An expander as one frame: its header outlined as its content is (the content's top edge is the header's).
    if (c.set[Line]) resources.Insert(winrt::box_value(L"ExpanderHeaderBorderThickness"), winrt::box_value(wux::Thickness{1, 1, 1, 1}));
    // The colours as a dictionary of their own (Dark, Light and Default themes), merged in last: the app's own
    // theme lists aren't read again once it has started, a dictionary merged after the rest is.
    // And into the app's own theme lists where it has them: what it defines itself wins over anything merged in.
    auto fill = [&](wux::ResourceDictionary const& d) {
        for (auto const& k : kKeys) {
            if (!c.set[k.role]) continue;
            auto color = ColorOf(c.rgb[k.role]);
            d.Insert(winrt::box_value(k.name), k.color ? winrt::box_value(color) : winrt::Windows::Foundation::IInspectable(wux::Media::SolidColorBrush(color)));
        }
    };
    wux::ResourceDictionary ours;
    // The font the styles take (the text's; the symbol font for icons is left alone).
    if (!c.font.empty()) resources.Insert(winrt::box_value(L"ContentControlThemeFontFamily"), wux::Media::FontFamily(c.font));
    auto own = resources.ThemeDictionaries();
    for (const wchar_t* theme : {L"Dark", L"Light", L"Default"}) {
        wux::ResourceDictionary d;
        fill(d);
        ours.ThemeDictionaries().Insert(winrt::box_value(theme), d);
        auto key = winrt::box_value(theme);
        if (own.HasKey(key))
            if (auto theirs = own.Lookup(key).try_as<wux::ResourceDictionary>()) fill(theirs);
    }
    resources.MergedDictionaries().Append(ours);
}

// Text with no colour of its own takes it from the control above it, back to Windows' default, which no
// resource changes: the text colour set on the first controls under the window's root (found breadth first),
// so everything under them takes it.
void SetForeground(wux::DependencyObject const& root, uint32_t rgb) {
    using winrt::Windows::UI::Xaml::Media::VisualTreeHelper;
    std::vector<wux::DependencyObject> level{root};
    for (int depth = 0; depth < 8 && !level.empty(); depth++) {
        std::vector<wux::DependencyObject> next;
        bool found = false;
        for (auto const& o : level) {
            if (depth > 0)
                if (auto control = o.try_as<wux::Controls::Control>()) {
                    control.Foreground(wux::Media::SolidColorBrush(ColorOf(rgb)));
                    found = true;
                    continue;
                }
            int n = VisualTreeHelper::GetChildrenCount(o);
            for (int i = 0; i < n; i++) next.push_back(VisualTreeHelper::GetChild(o, i));
        }
        if (found) return;
        level = std::move(next);
    }
}

// A window's content takes its theme again (to the other one and back), so what's drawn already looks the
// colours up again.
void Refresh(wux::FrameworkElement const& root) {
    auto was = root.RequestedTheme();
    auto other = root.ActualTheme() == wux::ElementTheme::Dark ? wux::ElementTheme::Light : wux::ElementTheme::Dark;
    root.RequestedTheme(other);
    root.RequestedTheme(was);
}

// Stock white (or its see-through greys) on text or an icon the app coloured itself becomes ours: now, and whenever
// the app gives it a colour later (a list filling in, a template applied after it was added). Only a change costs.
void WatchForeground(wux::DependencyObject const& o, int kind, Colours const& c) {
    auto property = kind == 1 ? wux::Controls::TextBlock::ForegroundProperty()
                    : kind == 2 ? wux::Controls::ContentPresenter::ForegroundProperty()
                                : wux::Controls::IconElement::ForegroundProperty();
    auto check = [c, property](wux::DependencyObject const& o) {
        try {
            // A text block the app set to Windows' own text font: ours (Segoe's symbol and emoji fonts stay).
            if (!c.font.empty())
                if (auto tb = o.try_as<wux::Controls::TextBlock>()) {
                    std::wstring face = tb.FontFamily().Source().c_str();
                    if (face.rfind(L"Segoe UI", 0) == 0 && face.find(L"Symbol") == std::wstring::npos && face.find(L"Emoji") == std::wstring::npos) {
                        tb.FontFamily(wux::Media::FontFamily(c.font));
                        // (a pixel font is only sharp at its own size and whole multiples of it)
                        if (c.fontGrid > 0) tb.FontSize(std::max(c.fontGrid, std::round(tb.FontSize() / c.fontGrid) * c.fontGrid));
                    }
                }
            auto value = o.GetValue(property);
            auto brush = value.try_as<wux::Media::SolidColorBrush>();
            if (!brush) return;
            auto k = brush.Color();
            if (k.R != 255 || k.G != 255 || k.B != 255) return;  // (its own colour, or ours: left alone)
            Role role = k.A >= 0xE0 ? Text : Muted;
            if (c.set[role]) o.SetValue(property, wux::Media::SolidColorBrush(ColorOf(c.rgb[role])));
        } catch (...) {
        }
    };
    o.RegisterPropertyChangedCallback(property, [check](wux::DependencyObject const& o, wux::DependencyProperty const&) { check(o); });
    o.Dispatcher().RunAsync(winrt::Windows::UI::Core::CoreDispatcherPriority::Low, [o, check] { check(o); });
}

// An image from the app's own package (ms-appx:, ms-resource:): its coloured icons and device pictures, hidden.
// A person's picture is drawn another way (an image brush), so it stays. Looked at again when its source changes
// (lists reuse their items).
void WatchImage(wux::Controls::Image const& image) {
    auto check = [](wux::Controls::Image const& image) {
        try {
            winrt::Windows::Foundation::Uri uri{nullptr};
            auto source = image.Source();
            if (auto bitmap = source.try_as<wux::Media::Imaging::BitmapImage>()) uri = bitmap.UriSource();
            else if (auto svg = source.try_as<wux::Media::Imaging::SvgImageSource>()) uri = svg.UriSource();
            std::wstring scheme = uri ? std::wstring(uri.SchemeName()) : L"";
            bool hide = scheme == L"ms-appx" || scheme == L"ms-resource";
            auto shown = hide ? wux::Visibility::Collapsed : wux::Visibility::Visible;
            image.Visibility(shown);
            // Its own frame too (a border, a viewbox, a panel holding only it), which keeps its width otherwise.
            auto parent = winrt::Windows::UI::Xaml::Media::VisualTreeHelper::GetParent(image);
            bool alone = parent && (parent.try_as<wux::Controls::Border>() || parent.try_as<wux::Controls::Viewbox>() ||
                                    (parent.try_as<wux::Controls::Panel>() &&
                                     winrt::Windows::UI::Xaml::Media::VisualTreeHelper::GetChildrenCount(parent) == 1));
            if (alone)
                if (auto frame = parent.try_as<wux::UIElement>()) frame.Visibility(shown);
            // And the place kept for an icon (a presenter named for it, a fixed width and margin of its own) a few
            // levels up, which holds the text off to the side otherwise.
            wux::DependencyObject at = parent;
            for (int i = 0; i < 3 && at; i++, at = winrt::Windows::UI::Xaml::Media::VisualTreeHelper::GetParent(at))
                if (auto fe = at.try_as<wux::FrameworkElement>(); fe && std::wstring(fe.Name()).find(L"Icon") != std::wstring::npos) {
                    fe.Visibility(shown);
                    break;
                }
        } catch (...) {
        }
    };
    image.RegisterPropertyChangedCallback(wux::Controls::Image::SourceProperty(), [check](wux::DependencyObject const& o, wux::DependencyProperty const&) {
        if (auto i = o.try_as<wux::Controls::Image>()) check(i);
    });
    image.Dispatcher().RunAsync(winrt::Windows::UI::Core::CoreDispatcherPriority::Low, [image, check] { check(image); });
}

// A corner the app's templates round themselves, not through the radius resources (a card's border, a button's
// presenter, the toggle's pill): square, now and whenever the template sets it again.
void WatchCorners(wux::DependencyObject const& o) {
    using winrt::Windows::UI::Xaml::Shapes::Rectangle;
    wux::DependencyProperty property{nullptr};
    if (o.try_as<wux::Controls::Border>()) property = wux::Controls::Border::CornerRadiusProperty();
    else if (o.try_as<wux::Controls::Grid>()) property = wux::Controls::Grid::CornerRadiusProperty();
    else if (o.try_as<wux::Controls::ContentPresenter>()) property = wux::Controls::ContentPresenter::CornerRadiusProperty();
    if (property) {
        auto square = [property](wux::DependencyObject const& o) {
            try {
                auto r = winrt::unbox_value<wux::CornerRadius>(o.GetValue(property));
                if (r.TopLeft || r.TopRight || r.BottomRight || r.BottomLeft) o.SetValue(property, winrt::box_value(wux::CornerRadius{0, 0, 0, 0}));
            } catch (...) {
            }
        };
        o.RegisterPropertyChangedCallback(property, [square](wux::DependencyObject const& o, wux::DependencyProperty const&) { square(o); });
        square(o);
        return;
    }
    if (auto rect = o.try_as<Rectangle>()) {
        auto square = [](Rectangle const& r) {
            if (r.RadiusX() != 0) r.RadiusX(0);
            if (r.RadiusY() != 0) r.RadiusY(0);
        };
        rect.RegisterPropertyChangedCallback(Rectangle::RadiusXProperty(), [square](wux::DependencyObject const& o, wux::DependencyProperty const&) {
            if (auto r = o.try_as<Rectangle>()) square(r);
        });
        rect.Dispatcher().RunAsync(winrt::Windows::UI::Core::CoreDispatcherPriority::Low, [rect, square] { square(rect); });
    }
}

struct Watcher : winrt::implements<Watcher, IVisualTreeServiceCallback2, winrt::non_agile> {
    winrt::com_ptr<IXamlDiagnostics> diag;
    Colours colours;
    bool set = false;
    std::set<void*> roots;  // windows done

    HRESULT STDMETHODCALLTYPE OnVisualTreeChange(ParentChildRelation relation, VisualElement element, VisualMutationType mutation) override try {
        // Only a window's root: one compare for everything else, so watching costs nothing.
        if (mutation != Add) return S_OK;
        // Text the app gives Windows' own white or grey itself (a page's title): checked once as it's laid out, and
        // only if it's still that stock colour does it take ours. One string compare for every other element.
        if (relation.Parent != 0) {
            // Text and icons: a text block, a content presenter showing a string, an icon (one compare each).
            const wchar_t* t = element.Type ? element.Type : L"";
            int kind = !wcscmp(t, L"Windows.UI.Xaml.Controls.TextBlock")          ? 1
                       : !wcscmp(t, L"Windows.UI.Xaml.Controls.ContentPresenter") ? 2
                       : !wcscmp(t, L"Windows.UI.Xaml.Controls.FontIcon") || !wcscmp(t, L"Windows.UI.Xaml.Controls.SymbolIcon") ||
                               !wcscmp(t, L"Windows.UI.Xaml.Controls.PathIcon")
                           ? 3
                           : 0;
            if (colours.square && (!wcscmp(t, L"Windows.UI.Xaml.Controls.Border") || !wcscmp(t, L"Windows.UI.Xaml.Controls.Grid") ||
                                   !wcscmp(t, L"Windows.UI.Xaml.Controls.ContentPresenter") || !wcscmp(t, L"Windows.UI.Xaml.Shapes.Rectangle"))) {
                winrt::Windows::Foundation::IInspectable object;
                if (SUCCEEDED(diag->GetIInspectableFromHandle(element.Handle, (::IInspectable**)winrt::put_abi(object))) && object)
                    WatchCorners(object.as<wux::DependencyObject>());
            }
            if (colours.hideIcons && !wcscmp(t, L"Windows.UI.Xaml.Controls.Image")) {
                winrt::Windows::Foundation::IInspectable object;
                if (SUCCEEDED(diag->GetIInspectableFromHandle(element.Handle, (::IInspectable**)winrt::put_abi(object))) && object)
                    if (auto image = object.try_as<wux::Controls::Image>()) WatchImage(image);
            }
            if (kind && (colours.set[Text] || colours.set[Muted])) {
                winrt::Windows::Foundation::IInspectable object;
                if (SUCCEEDED(diag->GetIInspectableFromHandle(element.Handle, (::IInspectable**)winrt::put_abi(object))) && object)
                    WatchForeground(object.as<wux::DependencyObject>(), kind, colours);
            }
            return S_OK;
        }
        winrt::Windows::Foundation::IInspectable object;
        if (FAILED(diag->GetIInspectableFromHandle(element.Handle, (::IInspectable**)winrt::put_abi(object))) || !object) return S_OK;
        // A window (a CoreWindow app's Window, or an island's root element): the colours set once, then its content
        // takes its theme again once the app has laid it out (asked for at low priority, after what's queued).
        auto window = object.try_as<wux::Window>();
        auto fe = object.try_as<wux::FrameworkElement>();
        if (!window && !fe) return S_OK;
        if (!set) {
            SetResources(colours);
            set = true;
            Log(L"colours set");
        }
        auto dispatcher = window ? window.Dispatcher() : fe.Dispatcher();
        auto self = get_strong();
        dispatcher.RunAsync(winrt::Windows::UI::Core::CoreDispatcherPriority::Low, [self, window, fe] {
            try {
                wux::FrameworkElement content = fe;
                if (window) content = window.Content().try_as<wux::FrameworkElement>();
                else if (auto xr = fe.XamlRoot())
                    if (auto c = xr.Content().try_as<wux::FrameworkElement>()) content = c;
                if (content && self->roots.insert(winrt::get_abi(content)).second) {
                    // The window's own ground, over Windows' backdrop (Mica) that shows through a see-through page.
                    if (self->colours.set[Ground]) {
                        if (auto panel = content.try_as<wux::Controls::Panel>())
                            panel.Background(wux::Media::SolidColorBrush(ColorOf(self->colours.rgb[Ground])));
                        else if (auto control = content.try_as<wux::Controls::Control>())
                            control.Background(wux::Media::SolidColorBrush(ColorOf(self->colours.rgb[Ground])));
                    }
                    if (self->colours.set[Text]) SetForeground(content, self->colours.rgb[Text]);
                    Refresh(content);
                    Log(L"a window's theme taken again");
                }
            } catch (...) {
                Log(L"a window couldn't take its theme again: " + std::wstring(winrt::to_message()));
            }
        });
        return S_OK;
    } catch (...) {
        Log(L"a window couldn't be styled: " + std::wstring(winrt::to_message()));
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE OnElementStateChanged(InstanceHandle, VisualElementState, LPCWSTR) override { return S_OK; }
};

struct Tap : winrt::implements<Tap, IObjectWithSite, winrt::non_agile> {
    winrt::com_ptr<IUnknown> site;

    Tap() { g_objects++; }
    ~Tap() { g_objects--; }

    HRESULT STDMETHODCALLTYPE SetSite(IUnknown* unknown) override try {
        site.copy_from(unknown);
        if (!unknown) return S_OK;
        Log(L"loaded");
        auto diag = site.as<IXamlDiagnostics>();
        BSTR init = nullptr;
        diag->GetInitializationData(&init);
        auto watcher = winrt::make_self<Watcher>();
        watcher->diag = diag;
        watcher->colours = Parse(init ? init : L"");
        SysFreeString(init);
        // Asked from another thread: asked on this one, the app waits on itself.
        auto service = site.as<IVisualTreeService3>();
        std::thread([service, watcher] {
            HRESULT hr = service->AdviseVisualTreeChange(watcher.get());
            Log(FAILED(hr) ? L"couldn't watch the app's windows: " + std::to_wstring((unsigned)hr) : std::wstring(L"watching the app's windows"));
        }).detach();
        return S_OK;
    } catch (...) {
        return winrt::to_hresult();
    }

    HRESULT STDMETHODCALLTYPE GetSite(REFIID riid, void** out) override {
        if (!site) return *out = nullptr, E_FAIL;
        return site->QueryInterface(riid, out);
    }
};

struct Factory : winrt::implements<Factory, IClassFactory, winrt::non_agile> {
    HRESULT STDMETHODCALLTYPE CreateInstance(IUnknown* outer, REFIID riid, void** out) override try {
        *out = nullptr;
        if (outer) return CLASS_E_NOAGGREGATION;
        return winrt::make<Tap>().as<IUnknown>()->QueryInterface(riid, out);
    } catch (...) {
        return winrt::to_hresult();
    }
    HRESULT STDMETHODCALLTYPE LockServer(BOOL) override { return S_OK; }
};

}  // namespace

extern "C" __declspec(dllexport) HRESULT STDAPICALLTYPE DllGetClassObject(REFCLSID clsid, REFIID riid, void** out) try {
    *out = nullptr;
    if (clsid != kClsid) return CLASS_E_CLASSNOTAVAILABLE;
    return winrt::make<Factory>().as<IUnknown>()->QueryInterface(riid, out);
} catch (...) {
    return winrt::to_hresult();
}

extern "C" __declspec(dllexport) HRESULT STDAPICALLTYPE DllCanUnloadNow() { return g_objects ? S_FALSE : S_FALSE; }  // (stays: the app holds it)
