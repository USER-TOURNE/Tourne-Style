#include "presets.h"

#include <algorithm>

namespace td {

namespace {

const Preset kPresets[] = {
#include "presets-data.inc"
};

}  // namespace

Roles Preset::AsRoles() const {
    Roles r;
    uint32_t* out[] = {&r.ground, &r.raised, &r.line, &r.edge, &r.text, &r.accent, &r.muted, &r.dim, &r.warn, &r.bad, &r.accent2, &r.good};
    for (int i = 0; i < 12; i++) *out[i] = roles[i];
    return r;
}

std::vector<std::pair<std::wstring, std::wstring>> Preset::Settings() const {
    std::vector<std::pair<std::wstring, std::wstring>> out;
    std::wstring all = settings;
    for (size_t at = 0; at < all.size();) {
        size_t nl = all.find(L'\n', at);
        if (nl == std::wstring::npos) nl = all.size();
        std::wstring line = all.substr(at, nl - at);
        if (size_t eq = line.find(L'='); eq != std::wstring::npos) out.push_back({line.substr(0, eq), line.substr(eq + 1)});
        at = nl + 1;
    }
    return out;
}

const std::vector<Preset>& Presets() {
    static const std::vector<Preset> all(std::begin(kPresets), std::end(kPresets));
    return all;
}

const Preset* FindPreset(const std::wstring& id) {
    for (auto const& p : Presets())
        if (id == p.id) return &p;
    return nullptr;
}

std::vector<std::pair<std::wstring, std::wstring>> PresetChoice(const Preset& p) {
    std::vector<std::pair<std::wstring, std::wstring>> out = {
        {L"theme.source", L"preset"},
        {L"theme.preset", p.id},
        {L"taskbar.scheme", L"tourneTheme"},
        {L"switcher.theme.style", L"scheme"},
        {L"systemIcons.scheme", L"tourneTheme"},  // (the switcher's "scheme" is System Icons')
    };
    for (auto& kv : p.Settings()) {  // (an identity's own theme source replaces "preset")
        auto same = std::find_if(out.begin(), out.end(), [&](auto const& o) { return o.first == kv.first; });
        if (same != out.end()) same->second = kv.second;
        else out.push_back(kv);
    }
    return out;
}

}  // namespace td
