// Peek, as the taskbar's thumbnails do ("// @inline live-peek.cpp"): every
// other window faded away, the peeked one shown where it is, and the window
// passed as "above" (a preview popup, the switcher) kept over it. DWM's own
// private call (dwmapi ordinal 113), whose fifth argument came with Windows
// 10.

using DwmpActivateLivePreview_t = HRESULT(WINAPI*)(BOOL, HWND, HWND, UINT, UINT_PTR);
[[maybe_unused]] constexpr UINT kPeekTaskbar = 1;
[[maybe_unused]] constexpr UINT kPeekAltTab = 3;

HWND g_peeking;  // the window being peeked at, or null

// Stays in view through any peek, as Windows' own taskbar does.
[[maybe_unused]] void KeepOutOfPeek(HWND wnd) {
    BOOL exclude = TRUE;
    if (wnd) DwmSetWindowAttribute(wnd, DWMWA_EXCLUDED_FROM_PEEK, &exclude, sizeof(exclude));
}

// Peeks at a window, or ends the peek (target null).
[[maybe_unused]] void LivePeek(HWND target, HWND above, UINT type) {
    static auto activate =
        (DwmpActivateLivePreview_t)GetProcAddress(LoadLibraryW(L"dwmapi.dll"), MAKEINTRESOURCEA(113));
    if (!activate || target == g_peeking) return;
    if (!target) {
        g_peeking = nullptr;
        activate(FALSE, nullptr, nullptr, type, 0);
        return;
    }
    g_peeking = target;
    activate(TRUE, target, above, type, 0);
}
