#include "motion-settings.h"

#include <windows.h>

#include "motion.cpp"
#include "td.h"

namespace td {

namespace {

motion::Anim HostMotion(const wchar_t* leaf) {
    return motion::Resolve(leaf, [](const std::wstring& key) {
        const wchar_t* v = GetString(L"desktop", key);  // (motion.* is a full name)
        std::wstring s = v ? v : L"";
        FreeString(v);
        return s;
    });
}

int g_windowsBefore = -1;  // Windows' window animation before Tourne changed it (-1: it hasn't)

void SetWindowsAnimation(bool on) {
    ANIMATIONINFO ai{sizeof(ai)};
    if (!SystemParametersInfoW(SPI_GETANIMATION, sizeof(ai), &ai, 0) || (ai.iMinAnimate != 0) == on) return;
    if (g_windowsBefore < 0) g_windowsBefore = ai.iMinAnimate != 0;
    ai.iMinAnimate = on;
    SystemParametersInfoW(SPI_SETANIMATION, sizeof(ai), &ai, SPIF_SENDCHANGE);  // (not SPIF_UPDATEINIFILE: not kept)
}

}  // namespace

// Tourne doesn't draw other programs' windows, so their opening and closing are Windows' own: these lines turn
// that on or off ("Animate windows when minimizing and maximizing"); its speed and curve stay Windows'.
void ApplyWindowsMotion() {
    bool set = false, on = false;
    for (auto leaf : {L"windowsIn", L"windowsOut", L"fadeIn", L"fadeOut"}) {
        motion::Anim a = HostMotion(leaf);
        if (a.set) set = true, on = on || a.on;
    }
    if (set) SetWindowsAnimation(on);
    else RestoreWindowsMotion();
}

void RestoreWindowsMotion() {
    if (g_windowsBefore < 0) return;
    bool before = g_windowsBefore != 0;
    g_windowsBefore = -1;
    ANIMATIONINFO ai{sizeof(ai)};
    if (SystemParametersInfoW(SPI_GETANIMATION, sizeof(ai), &ai, 0) && (ai.iMinAnimate != 0) != before) {
        ai.iMinAnimate = before;
        SystemParametersInfoW(SPI_SETANIMATION, sizeof(ai), &ai, SPIF_SENDCHANGE);
    }
}

void MotionDefaults(Settings& out) {
    out[L"motion.enabled"] = L"1";
    for (auto const& leaf : motion::Tree()) out[std::wstring(L"motion.") + leaf.name] = L"";
}

void MotionDescribe(SchemaInfo& info) {
    auto add = [&](const std::wstring& key, const std::wstring& name, const std::wstring& description) -> SettingInfo& {
        SettingInfo& si = info[key];
        si.name = name;
        si.description = description;
        if (!si.order) si.order = (int)info.size();
        return si;
    };
    add(L"motion", L"Motion",
        L"Pick a ready-made set under the curve (Subtle, Smooth, Snappy, Lively, Springy, komorebi's, or Hyprland's own) and change "
        L"any line after. How things move, written as Hyprland writes it: on, speed in tenths of a second, curve, and a style, such as "
        L"1, 4, overshoot, slide. A line left empty takes its parent's (the name under each). Curves: default, linear, ease, "
        L"overshoot, the easings.net set (easeOutQuint, easeInOutBack...), and the springs rubber, gentle, snappy, bouncy and "
        L"wobbly. Your own go in tourne.conf as bezier = NAME, X0, Y0, X1, Y1 or spring = NAME, MASS, STIFFNESS, DAMPING; "
        L"a Hyprland animations block pastes in as it is. Following it: the bar's hiding (bar) and a new screen's bar "
        L"(monitorAdded), CSS bar transitions (by curve name), Start (start), menus, flyouts and previews (popups, fadePopups), "
        L"notifications and plugin panels (layers, fadeLayers), the switcher's views (switcher), desktop switches and the pager's mark "
        L"(workspaces), tiling's moves (windowsMove), borders, their glow and shadow (border, fadeSwitch, fadeGlow, fadeShadow, "
        L"borderangle, glowangle), windows opening (windowsIn, fadeIn: popin N%, slide, gnomed, fade), and Windows' own minimise "
        L"and close animations on or off (windowsIn, windowsOut, fadeIn, fadeOut). "
        L"Nothing set: each part keeps its own.");
    add(L"motion.enabled", L"Animations", L"Off: everything that follows these lines moves at once.").isSwitch = true;
    // Leaves with nothing in Tourne Desktop to move yet (kept so a Hyprland block pastes in whole).
    static const std::pair<const wchar_t*, const wchar_t*> kUnused[] = {
        {L"specialWorkspace", L"Tourne has no scratchpad desktop"}, {L"specialWorkspaceIn", L"Tourne has no scratchpad desktop"},
        {L"specialWorkspaceOut", L"Tourne has no scratchpad desktop"}, {L"fadeDim", L"Tourne doesn't dim inactive windows"},
        {L"fadeDpms", L"Tourne doesn't turn screens off"}, {L"zoomFactor", L"Tourne has no screen zoom"},
        {L"shadowangle", L"borders' shadows have no gradient to turn"},
    };
    for (auto const& leaf : motion::Tree()) {
        std::wstring about = leaf.about;
        if (leaf.parent) about += std::wstring(L". Empty: as ") + leaf.parent;
        for (auto const& [name, why] : kUnused)
            if (std::wstring(leaf.name) == name) about += std::wstring(L". Not used yet: ") + why;
        if (std::wstring(leaf.name) == L"windows" || std::wstring(leaf.name).rfind(L"windowsIn", 0) == 0 ||
            std::wstring(leaf.name) == L"windowsOut" || std::wstring(leaf.name) == L"fadeIn" || std::wstring(leaf.name) == L"fadeOut")
            about += L". Other programs' windows: Windows' own animation, on or off (its speed is Windows')";
        add(std::wstring(L"motion.") + leaf.name, leaf.name, about);
    }
}

}  // namespace td
