// Plugin items on the bar, shared by the taskbar ("// @inline plugin-bar-shared.cpp")
// and Tourne Desktop's plugin runtime (desktop/host/plugin-runtime.cpp includes it).
//
// The runtime keeps a message-only window of class kPluginHostClass. Bars only
// ever post to it, so a bar never waits on a plugin that is busy:
// - kPluginBarAskMessage (wParam: the bar) asks for the items. The runtime
//   answers by sending the bar WM_COPYDATA kPluginBarItemsId with them packed,
//   behind the list's version.
// - kPluginBarEventMessage (wParam: that version, lParam: MakePluginBarEvent)
//   is a click, scroll or hover on one; one from an older list is dropped.
// When the items change, the runtime posts kPluginBarChangedMessage to every
// bar, which asks again. Outside Tourne Desktop (the Windhawk suite) there is
// no such window, so no items.
// System Icons asks the same way (with SendMessageTimeout, so the answer comes
// while it waits), to show the items the user moved into the overflow flyout
// among its hidden icons, and posts their clicks back as a bar does.
//
// Plugin items are placed like tray icons, by the key "plugin:<key>" in the
// tray's places and order (tray-shared.cpp): in the tray (the default), in the
// overflow, or hidden. Each can have an icon of the user's own in place of its
// glyph or picture (kPluginIconsValue); the runtime draws it into the item's
// picture, so every bar and the flyout show it alike.

[[maybe_unused]] constexpr wchar_t kPluginHostClass[] = L"TourneDesktopPlugins";
[[maybe_unused]] constexpr wchar_t kPluginBarChangedMessage[] = L"TourneSuite_PluginBarChanged";
[[maybe_unused]] constexpr wchar_t kPluginBarAskMessage[] = L"TourneSuite_PluginBarAsk";
[[maybe_unused]] constexpr wchar_t kPluginBarEventMessage[] = L"TourneSuite_PluginBarEvent";
[[maybe_unused]] constexpr ULONG_PTR kPluginBarItemsId = 0x54504C49;  // "TPLI"
[[maybe_unused]] constexpr wchar_t kPluginGlyphFont[] = L"tabler-icons";
[[maybe_unused]] constexpr LPARAM kPluginAskIconsChanged = 1;  // an ask's lParam: the user's icons changed, read them again
[[maybe_unused]] constexpr wchar_t kPluginIconsKey[] = L"Software\\TourneTray";  // (beside the tray's places)
[[maybe_unused]] constexpr wchar_t kPluginIconsValue[] = L"PluginIcons";       // "plugin:<key><tab>file" (or "file,index")

enum PluginBarEvent : int { kPluginClick, kPluginRightClick, kPluginMiddleClick, kPluginScrollUp, kPluginScrollDown, kPluginEnter, kPluginLeave };
[[maybe_unused]] static LPARAM MakePluginBarEvent(int index, PluginBarEvent event) { return (LPARAM)((index & 0xFFFF) | (event << 16)); }

struct PluginBarItem {
    std::wstring key;    // "<plugin id>:<entry id>"
    std::wstring glyph;  // the glyph's character in kPluginGlyphFont, or empty
    std::wstring text;
    std::wstring tip;
    COLORREF color = CLR_INVALID;  // the text's colour when the plugin set one, else the bar's
    std::wstring image;            // a picture in place of the glyph: a file of its pixels ("TBI1", width, height,
                                   // premultiplied BGRA rows top-down, at the screen's scale), or empty
    COLORREF glyphColor = CLR_INVALID;  // the glyph's own colour, when the plugin set one
};

// Packed: the version, \x1D, then items split by \x1E and their fields by \x1F.
[[maybe_unused]] static std::wstring PackPluginBarItems(std::vector<PluginBarItem> const& items, unsigned version) {
    std::wstring out = std::to_wstring(version) + L'\x1D';
    for (size_t i = 0; i < items.size(); i++) {
        auto const& item = items[i];
        if (i) out += L'\x1E';
        out += item.key + L'\x1F' + item.glyph + L'\x1F' + item.text + L'\x1F' + item.tip + L'\x1F' +
               (item.color == CLR_INVALID ? L"" : std::to_wstring(item.color)) + L'\x1F' + item.image + L'\x1F' +
               (item.glyphColor == CLR_INVALID ? L"" : std::to_wstring(item.glyphColor));
    }
    return out;
}

[[maybe_unused]] static std::vector<PluginBarItem> UnpackPluginBarItems(std::wstring_view packed, unsigned* version) {
    std::vector<PluginBarItem> items;
    size_t head = packed.find(L'\x1D');
    if (head == std::wstring_view::npos) return items;
    *version = (unsigned)wcstoul(std::wstring(packed.substr(0, head)).c_str(), nullptr, 10);
    packed = packed.substr(head + 1);
    while (!packed.empty()) {
        size_t end = packed.find(L'\x1E');
        std::wstring_view one = packed.substr(0, end);
        packed = end == std::wstring_view::npos ? std::wstring_view() : packed.substr(end + 1);
        std::wstring fields[7];
        for (int i = 0; i < 7; i++) {
            size_t cut = one.find(L'\x1F');
            fields[i] = std::wstring(one.substr(0, cut));
            one = cut == std::wstring_view::npos ? std::wstring_view() : one.substr(cut + 1);
        }
        PluginBarItem item{fields[0], fields[1], fields[2], fields[3]};
        if (!fields[4].empty()) item.color = (COLORREF)wcstoul(fields[4].c_str(), nullptr, 10);
        item.image = fields[5];
        if (!fields[6].empty()) item.glyphColor = (COLORREF)wcstoul(fields[6].c_str(), nullptr, 10);
        items.push_back(std::move(item));  // (kept even if empty, so indexes match the runtime's)
    }
    return items;
}

// The user's own icons for plugin items: (item's place key "plugin:<key>", its file).
[[maybe_unused]] static std::vector<std::pair<std::wstring, std::wstring>> LoadPluginIcons() {
    std::vector<std::pair<std::wstring, std::wstring>> rows;
    DWORD size = 0;
    if (RegGetValueW(HKEY_CURRENT_USER, kPluginIconsKey, kPluginIconsValue, RRF_RT_REG_MULTI_SZ, nullptr, nullptr, &size) != ERROR_SUCCESS ||
        size < sizeof(wchar_t))
        return rows;
    std::wstring buffer(size / sizeof(wchar_t) + 2, L'\0');
    size = (DWORD)(buffer.size() * sizeof(wchar_t));
    if (RegGetValueW(HKEY_CURRENT_USER, kPluginIconsKey, kPluginIconsValue, RRF_RT_REG_MULTI_SZ, nullptr, buffer.data(), &size) != ERROR_SUCCESS)
        return rows;
    for (const wchar_t* p = buffer.data(); p < buffer.data() + buffer.size() && *p; p += wcslen(p) + 1) {
        std::wstring row = p;
        size_t tab = row.find(L'\t');
        if (tab != std::wstring::npos && tab + 1 < row.size()) rows.push_back({row.substr(0, tab), row.substr(tab + 1)});
    }
    return rows;
}

// An item's own icon set (an empty file puts its own back).
[[maybe_unused]] static void SavePluginIcon(std::wstring const& key, std::wstring const& file) {
    auto rows = LoadPluginIcons();
    std::erase_if(rows, [&](auto const& row) { return row.first == key; });
    if (!file.empty()) rows.push_back({key, file});
    std::wstring data;
    for (auto const& [k, f] : rows) data += k + L'\t' + f + L'\0';
    data += L'\0';
    HKEY reg;
    if (RegCreateKeyExW(HKEY_CURRENT_USER, kPluginIconsKey, 0, nullptr, 0, KEY_SET_VALUE, nullptr, &reg, nullptr) != ERROR_SUCCESS) return;
    if (rows.empty()) RegDeleteValueW(reg, kPluginIconsValue);
    else RegSetValueExW(reg, kPluginIconsValue, 0, REG_MULTI_SZ, (const BYTE*)data.data(), (DWORD)(data.size() * sizeof(wchar_t)));
    RegCloseKey(reg);
}

// An item's picture, from the file of its pixels the runtime made (see PluginBarItem::image).
[[maybe_unused]] static bool ReadPluginImageFile(std::wstring const& path, int* w, int* h, std::vector<uint32_t>* pixels) {
    if (path.empty()) return false;
    HANDLE f = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, 0, nullptr);
    if (f == INVALID_HANDLE_VALUE) return false;
    char head[12] = {};
    DWORD got = 0;
    bool ok = false;
    if (ReadFile(f, head, 12, &got, nullptr) && got == 12 && !memcmp(head, "TBI1", 4)) {
        uint32_t pw = 0, ph = 0;
        memcpy(&pw, head + 4, 4), memcpy(&ph, head + 8, 4);
        if (pw && ph && pw <= 1024 && ph <= 256) {
            pixels->resize((size_t)pw * ph);
            DWORD bytes = (DWORD)(pixels->size() * 4);
            ok = ReadFile(f, pixels->data(), bytes, &got, nullptr) && got == bytes;
            if (ok) *w = (int)pw, *h = (int)ph;
            else pixels->clear();
        }
    }
    CloseHandle(f);
    return ok;
}

// An item as a tray icon's pixels (premultiplied BGRA, size by size): its picture fitted in, else its glyph,
// else the first letter of its text; in `color` unless the plugin chose its glyph's colour.
[[maybe_unused]] static std::vector<uint32_t> PluginItemPixels(PluginBarItem const& item, int size, COLORREF color) {
    std::vector<uint32_t> out((size_t)size * size, 0);
    if (size <= 0) return out;
    int pw = 0, ph = 0;
    std::vector<uint32_t> picture;
    if (ReadPluginImageFile(item.image, &pw, &ph, &picture)) {
        // Fitted in, its shape kept (an area average when it's smaller), centred.
        double k = std::min((double)size / pw, (double)size / ph);
        int ow = std::max(1, (int)(pw * k + 0.5)), oh = std::max(1, (int)(ph * k + 0.5));
        int left = (size - ow) / 2, top = (size - oh) / 2;
        for (int y = 0; y < oh; y++)
            for (int x = 0; x < ow; x++) {
                int x0 = x * pw / ow, x1 = std::max(x0 + 1, (x + 1) * pw / ow), y0 = y * ph / oh, y1 = std::max(y0 + 1, (y + 1) * ph / oh);
                uint32_t sum[4] = {}, n = 0;
                for (int sy = y0; sy < y1 && sy < ph; sy++)
                    for (int sx = x0; sx < x1 && sx < pw; sx++, n++)
                        for (int c = 0; c < 4; c++) sum[c] += picture[(size_t)sy * pw + sx] >> (c * 8) & 0xFF;
                uint32_t v = 0;
                for (int c = 0; c < 4; c++) v |= (n ? sum[c] / n : 0) << (c * 8);
                out[(size_t)(top + y) * size + left + x] = v;
            }
        return out;
    }
    bool glyph = !item.glyph.empty();
    std::wstring text = glyph ? item.glyph : item.text.substr(0, 1);
    if (text.empty()) return out;
    if (glyph && item.glyphColor != CLR_INVALID) color = item.glyphColor;
    else if (item.color != CLR_INVALID) color = item.color;
    // Drawn white on black; how bright each pixel came out is how much of the colour it takes.
    BITMAPINFO bi{};
    bi.bmiHeader = {sizeof(BITMAPINFOHEADER), size, -size, 1, 32, BI_RGB};
    void* bits = nullptr;
    HDC dc = CreateCompatibleDC(nullptr);
    HBITMAP bitmap = CreateDIBSection(dc, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
    if (!bitmap) return DeleteDC(dc), out;
    HGDIOBJ oldBitmap = SelectObject(dc, bitmap);
    HFONT font = CreateFontW(-(glyph ? size : size * 3 / 4), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                             CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, glyph ? kPluginGlyphFont : L"Segoe UI");
    HGDIOBJ oldFont = SelectObject(dc, font);
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, RGB(255, 255, 255));
    RECT rc{0, 0, size, size};
    DrawTextW(dc, text.c_str(), (int)text.size(), &rc, DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOPREFIX | DT_NOCLIP);
    GdiFlush();
    auto const* drawn = (uint32_t const*)bits;
    for (size_t i = 0; i < out.size(); i++) {
        uint32_t p = drawn[i], a = std::max({p & 0xFF, p >> 8 & 0xFF, p >> 16 & 0xFF});
        if (a) out[i] = a << 24 | (GetRValue(color) * a / 255) << 16 | (GetGValue(color) * a / 255) << 8 | GetBValue(color) * a / 255;
    }
    SelectObject(dc, oldFont);
    SelectObject(dc, oldBitmap);
    DeleteObject(font);
    DeleteObject(bitmap);
    DeleteDC(dc);
    return out;
}
