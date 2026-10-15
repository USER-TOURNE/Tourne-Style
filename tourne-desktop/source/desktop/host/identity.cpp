#include "identity.h"

#include <algorithm>
#include <cwchar>
#include <string>

namespace td {

namespace {

std::wstring Get(const Settings& values, const Settings& defaults, const wchar_t* key) {
    if (auto it = values.find(key); it != values.end()) return it->second;
    if (auto it = defaults.find(key); it != defaults.end()) return it->second;
    return L"";
}

int Number(const std::wstring& text, int fallback) {
    return text.empty() ? fallback : ToInt(text);
}

}  // namespace

void IdentityDefaults(Settings& out) {
    out[L"identity.shape"] = L"inherit";
    out[L"identity.frame"] = L"0";
    out[L"identity.material"] = L"inherit";
    out[L"identity.opacity"] = L"0";
    out[L"identity.motion"] = L"inherit";
    out[L"identity.speed"] = L"100";
}

void IdentityDescribe(SchemaInfo& info) {
    auto add = [&](const std::wstring& key, const wchar_t* name, const wchar_t* description) -> SettingInfo& {
        SettingInfo& si = info[key];
        si.name = name;
        si.description = description;
        if (!si.order) si.order = (int)info.size();
        return si;
    };
    add(L"identity", L"Identity",
        L"How the desktop is cut, what it is made of and how it moves, said once for every piece (the colours are in Tourne theme). "
        L"Each piece's own setting takes these values until you set that one yourself.");
    add(L"identity.shape", L"Shape", L"Corners on the switcher, borders, glass and previews.").options = {
        {L"inherit", L"Leave as each piece has it"}, {L"square", L"Square"}, {L"subtle", L"Subtle"}, {L"rounded", L"Rounded"}, {L"pill", L"Pill"}};
    SettingInfo& frame = add(L"identity.frame", L"Frame weight", L"Drawn borders, in pixels. 0 leaves them as they are.");
    frame.hasMin = frame.hasMax = true, frame.min = 0, frame.max = 8;
    add(L"identity.material", L"Material", L"What shows through see-through parts.").options = {
        {L"inherit", L"Leave as each piece has it"}, {L"none", L"Solid"}, {L"blur", L"Blur"}, {L"acrylic", L"Acrylic"}, {L"mica", L"Mica"}};
    SettingInfo& opacity = add(L"identity.opacity", L"Opacity", L"How solid see-through parts are, in percent. 0 leaves them as they are.");
    opacity.hasMin = opacity.hasMax = true, opacity.min = 0, opacity.max = 100;
    add(L"identity.motion", L"Motion", L"How things come and go: Off (at once), Snap (hard and short), Smooth or Lively.").options = {
        {L"inherit", L"Leave as each piece has it"}, {L"off", L"Off"}, {L"snap", L"Snap"}, {L"smooth", L"Smooth"}, {L"lively", L"Lively"}};
    SettingInfo& speed = add(L"identity.speed", L"Motion speed", L"Percent: 100 as designed, 200 twice as fast.");
    speed.hasMin = speed.hasMax = true, speed.min = 50, speed.max = 300;
}

Settings IdentityDerive(const Settings& values, const Settings& defaults) {
    Settings out;
    std::wstring shape = Get(values, defaults, L"identity.shape");
    std::wstring material = Get(values, defaults, L"identity.material");
    std::wstring motion = Get(values, defaults, L"identity.motion");
    int frame = std::clamp(Number(Get(values, defaults, L"identity.frame"), 0), 0, 8);
    int opacity = std::clamp(Number(Get(values, defaults, L"identity.opacity"), 0), 0, 100);
    int speed = std::clamp(Number(Get(values, defaults, L"identity.speed"), 100), 50, 300);

    int radius = -1;
    if (shape == L"square") radius = 0;
    else if (shape == L"subtle") radius = 4;
    else if (shape == L"rounded") radius = 10;
    else if (shape == L"pill") radius = 18;
    if (radius >= 0) {
        out[L"switcher.corners.radius"] = std::to_wstring(radius);
        out[L"switcher.borders.radius"] = radius ? L"custom" : L"square";
        out[L"switcher.borders.radiusCustom"] = std::to_wstring(radius);
        out[L"glass.radius"] = std::to_wstring(radius);
        out[L"glass.rims.radius"] = std::to_wstring(std::min(radius, 24));
        out[L"taskbar.previewCorners"] = radius == 0 ? L"square" : radius <= 4 ? L"small" : L"round";
    }
    if (frame > 0) out[L"switcher.borders.width"] = std::to_wstring(frame);

    if (material == L"none" || material == L"blur" || material == L"acrylic" || material == L"mica") {
        out[L"taskbar.previewBackdrop"] = material;
        out[L"taskbar.coloringBlur"] = material == L"none" ? L"none" : material == L"acrylic" ? L"acrylic" : L"blur";
        out[L"switcher.theme.backdrop"] = material == L"none" ? L"none" : L"acrylic";  // (the switcher has the two)
    }
    if (opacity >= 10) {
        out[L"taskbar.previewOpacity"] = std::to_wstring(opacity);
        out[L"switcher.theme.opacity"] = std::to_wstring(opacity);
    }

    if (motion == L"off" || motion == L"snap" || motion == L"smooth" || motion == L"lively") {
        // Milliseconds as designed for each personality, then the speed.
        int hide = motion == L"off" ? 1 : motion == L"snap" ? 60 : motion == L"smooth" ? 220 : 320;
        int fade = motion == L"off" ? 1 : motion == L"snap" ? 150 : motion == L"smooth" ? 1500 : 2200;
        int spin = motion == L"off" ? 1 : motion == L"snap" ? 800 : motion == L"smooth" ? 2500 : 1600;
        auto scaled = [&](int ms) { return std::to_wstring(std::max(1, ms * 100 / speed)); };
        out[L"taskbar.shyAnimation"] = motion == L"off" || motion == L"snap" ? L"none" : motion == L"smooth" ? L"slide" : L"slidefade";
        out[L"taskbar.shyDuration"] = scaled(hide);
        out[L"taskbar.idleFadeTime"] = scaled(fade * 2 / 5);
        out[L"switcher.borders.fadeDuration"] = scaled(fade);
        out[L"switcher.borders.spinDuration"] = scaled(spin);
        // The same as motion lines (motion.cpp), which the parts read first:
        // speeds in tenths of a second, as Hyprland's.
        auto tenths = [&](int ms) {
            wchar_t text[32];
            swprintf(text, 32, L"%g", std::max(1, ms * 100 / speed) / 100.0);
            return std::wstring(text);
        };
        if (motion == L"off") {
            out[L"motion.global"] = L"0";
        } else {
            std::wstring curve = motion == L"snap" ? L"easeOutQuint" : motion == L"smooth" ? L"easeInOutQuint" : L"overshot";
            int all = motion == L"snap" ? 150 : motion == L"smooth" ? 400 : 500;
            out[L"motion.global"] = L"1, " + tenths(all) + L", " + curve;
            out[L"motion.bar"] = motion == L"snap" ? L"0" : L"1, " + tenths(hide) + L", " + curve + (motion == L"smooth" ? L", slide" : L", slidefade");
            out[L"motion.border"] = L"1, " + tenths(fade) + L", " + curve;
            out[L"motion.borderangle"] = L"1, " + tenths(spin) + L", linear, loop";
        }
    }
    return out;
}

}  // namespace td
