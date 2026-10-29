// Screenshot annotation (included in capture.cpp after Picture): a window over the screen with the picture in
// it, and tools to draw on it before it's copied and kept. Pen, arrow, box, highlighter, text, numbered markers
// and redact (squares of colour over what's private, the pixels themselves replaced, so nothing can be undone
// out of the file). Keys: 1 to 7 pick a tool, C the next colour, Ctrl+Z takes the last mark back, Enter or Done
// keeps it, Esc keeps the plain picture. Drawn with Direct2D; the toolbar in the menus' look.

namespace annotate {

enum Tool { Pen, Arrow, Box, Highlight, Text, Marker, Redact, kTools };
const wchar_t* const kToolNames[kTools] = {L"Pen", L"Arrow", L"Box", L"Highlight", L"Text", L"Marker", L"Redact"};
const uint32_t kColours[] = {0xE57474, 0x15FFA2, 0xF2C94C, 0x5DA9F6, 0xFFFFFF, 0x101010};

struct Mark {
    Tool tool;
    uint32_t colour;
    float width;
    std::vector<D2D1_POINT_2F> points;  // in the picture's pixels
    std::wstring text;
    int number = 0;
};

struct State {
    Picture* pic = nullptr;
    std::vector<uint32_t> shown;  // the picture with the redactions in
    std::vector<Mark> marks;
    Tool tool = Arrow;
    int colour = 0;
    bool drawing = false, done = false, keep = false;
    float scale = 1, ox = 0, oy = 0;  // picture to window: x * scale + ox
    HWND wnd = nullptr, edit = nullptr;
    D2D1_POINT_2F editAt{};
    int nextNumber = 1;
    ID2D1Factory* d2d = nullptr;
    IDWriteFactory* dw = nullptr;
    ID2D1HwndRenderTarget* rt = nullptr;
    ID2D1Bitmap* bitmap = nullptr;  // `shown`, on rt
    std::vector<RECT> toolRects;    // the toolbar's buttons: tools, then the colour, Undo, Done
};
State* g_state = nullptr;

int ToolbarH() { return 40; }

// The redactions put into the pixels: each rectangle made of 12 px squares of its average colour.
void Redraw(State& s) {
    s.shown = s.pic->px;
    for (auto const& m : s.marks) {
        if (m.tool != Redact || m.points.size() < 2) continue;
        int x0 = (int)std::min(m.points[0].x, m.points.back().x), x1 = (int)std::max(m.points[0].x, m.points.back().x);
        int y0 = (int)std::min(m.points[0].y, m.points.back().y), y1 = (int)std::max(m.points[0].y, m.points.back().y);
        x0 = std::clamp(x0, 0, s.pic->w), x1 = std::clamp(x1, 0, s.pic->w), y0 = std::clamp(y0, 0, s.pic->h), y1 = std::clamp(y1, 0, s.pic->h);
        const int k = 12;
        for (int by = y0; by < y1; by += k)
            for (int bx = x0; bx < x1; bx += k) {
                uint64_t r = 0, g = 0, b = 0, n = 0;
                for (int y = by; y < std::min(by + k, y1); y++)
                    for (int x = bx; x < std::min(bx + k, x1); x++) {
                        uint32_t c = s.pic->px[(size_t)y * s.pic->w + x];
                        r += c >> 16 & 255, g += c >> 8 & 255, b += c & 255, n++;
                    }
                if (!n) continue;
                uint32_t avg = 0xFF000000u | (uint32_t)(r / n) << 16 | (uint32_t)(g / n) << 8 | (uint32_t)(b / n);
                for (int y = by; y < std::min(by + k, y1); y++)
                    for (int x = bx; x < std::min(bx + k, x1); x++) s.shown[(size_t)y * s.pic->w + x] = avg;
            }
    }
    if (s.bitmap) s.bitmap->Release(), s.bitmap = nullptr;
}

D2D1_COLOR_F Colour(uint32_t c, float a = 1) { return D2D1::ColorF((c >> 16 & 255) / 255.f, (c >> 8 & 255) / 255.f, (c & 255) / 255.f, a); }

// The marks, in the picture's pixels (the target's transform places them).
void DrawMarks(State& s, ID2D1RenderTarget* rt) {
    ID2D1SolidColorBrush* brush = nullptr;
    rt->CreateSolidColorBrush(Colour(0), &brush);
    if (!brush) return;
    ID2D1StrokeStyle* round = nullptr;
    D2D1_STROKE_STYLE_PROPERTIES props = D2D1::StrokeStyleProperties(D2D1_CAP_STYLE_ROUND, D2D1_CAP_STYLE_ROUND, D2D1_CAP_STYLE_ROUND,
                                                                     D2D1_LINE_JOIN_ROUND);
    s.d2d->CreateStrokeStyle(props, nullptr, 0, &round);
    for (auto const& m : s.marks) {
        if (m.points.empty() || m.tool == Redact) continue;
        D2D1_POINT_2F a = m.points[0], b = m.points.back();
        switch (m.tool) {
            case Pen:
            case Highlight:
                brush->SetColor(Colour(m.colour, m.tool == Highlight ? 0.4f : 1));
                for (size_t i = 1; i < m.points.size(); i++) rt->DrawLine(m.points[i - 1], m.points[i], brush, m.width, round);
                if (m.points.size() == 1) rt->FillEllipse(D2D1::Ellipse(a, m.width / 2, m.width / 2), brush);
                break;
            case Arrow: {
                brush->SetColor(Colour(m.colour));
                rt->DrawLine(a, b, brush, m.width, round);
                float dx = b.x - a.x, dy = b.y - a.y, len = std::sqrt(dx * dx + dy * dy);
                if (len > 1) {
                    float ux = dx / len, uy = dy / len, head = std::max(12.f, m.width * 4);
                    D2D1_POINT_2F l{b.x - ux * head - uy * head * 0.55f, b.y - uy * head + ux * head * 0.55f};
                    D2D1_POINT_2F r{b.x - ux * head + uy * head * 0.55f, b.y - uy * head - ux * head * 0.55f};
                    rt->DrawLine(b, l, brush, m.width, round);
                    rt->DrawLine(b, r, brush, m.width, round);
                }
                break;
            }
            case Box:
                brush->SetColor(Colour(m.colour));
                rt->DrawRectangle(D2D1::RectF(std::min(a.x, b.x), std::min(a.y, b.y), std::max(a.x, b.x), std::max(a.y, b.y)), brush, m.width);
                break;
            case Text: {
                IDWriteTextFormat* f = nullptr;
                s.dw->CreateTextFormat(L"Segoe UI", nullptr, DWRITE_FONT_WEIGHT_SEMI_BOLD, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
                                       m.width * 6, L"", &f);
                if (f) {
                    brush->SetColor(Colour(m.colour));
                    rt->DrawText(m.text.c_str(), (UINT32)m.text.size(), f, D2D1::RectF(a.x, a.y, a.x + 4000, a.y + 2000), brush);
                    f->Release();
                }
                break;
            }
            case Marker: {
                float r = std::max(12.f, m.width * 4);
                brush->SetColor(Colour(m.colour));
                rt->FillEllipse(D2D1::Ellipse(a, r, r), brush);
                IDWriteTextFormat* f = nullptr;
                s.dw->CreateTextFormat(L"Segoe UI", nullptr, DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, r * 1.2f,
                                       L"", &f);
                if (f) {
                    f->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
                    f->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
                    uint32_t c = m.colour;
                    bool lightDot = ((c >> 16 & 255) * 299 + (c >> 8 & 255) * 587 + (c & 255) * 114) / 1000 > 140;
                    brush->SetColor(Colour(lightDot ? 0x101010 : 0xFFFFFF));
                    std::wstring n = std::to_wstring(m.number);
                    rt->DrawText(n.c_str(), (UINT32)n.size(), f, D2D1::RectF(a.x - r, a.y - r, a.x + r, a.y + r), brush);
                    f->Release();
                }
                break;
            }
            default:
                break;
        }
    }
    if (round) round->Release();
    brush->Release();
}

// The toolbar: the tools as "[ NAME ]", the one in use in the accent; the colour; Undo; Done.
void DrawToolbar(State& s, HDC dc, RECT rc) {
    struct LookC {
        uint32_t ground, line, edge, text, accent, dim, hover, hoverText, band;
        HFONT font;
        int scale, frame, caps;
        uint32_t frameLine;
    } look{0x121C21, 0x1E2F36, 0x4F8F6A, 0x66EA97, 0x15FFA2, 0x5D8D6F, 0x1F3D34, 0xFFFFFFFF, 0x151F24, nullptr, 1, 0, 1, 0xE57474};
    using LookFn = BOOL (*)(LookC*, int);
    static auto lookFor = (LookFn)GetProcAddress(GetModuleHandleW(nullptr), "TourneMenuLookFor");
    if (lookFor) lookFor(&look, (int)sizeof(look));
    auto rgb = [](uint32_t c) { return RGB(c >> 16 & 255, c >> 8 & 255, c & 255); };
    RECT bar{0, 0, rc.right, ToolbarH()};
    SetDCBrushColor(dc, rgb(look.ground));
    FillRect(dc, &bar, (HBRUSH)GetStockObject(DC_BRUSH));
    RECT edge{0, bar.bottom - 1, rc.right, bar.bottom};
    SetDCBrushColor(dc, rgb(look.frameLine != 0xFFFFFFFF ? look.frameLine : look.edge));
    FillRect(dc, &edge, (HBRUSH)GetStockObject(DC_BRUSH));
    HGDIOBJ oldFont = SelectObject(dc, look.font ? look.font : GetStockObject(DEFAULT_GUI_FONT));
    SetBkMode(dc, TRANSPARENT);
    s.toolRects.clear();
    int x = 12;
    auto button = [&](std::wstring label, bool on) {
        if (look.caps)
            for (auto& c : label) c = towupper(c);
        std::wstring t = L"[ " + label + L" ]";
        SIZE size{};
        GetTextExtentPoint32W(dc, t.c_str(), (int)t.size(), &size);
        RECT r{x, 0, x + size.cx + 8, ToolbarH() - 1};
        SetTextColor(dc, rgb(on ? look.accent : look.text));
        DrawTextW(dc, t.c_str(), -1, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
        s.toolRects.push_back(r);
        x = r.right + 6;
    };
    for (int i = 0; i < kTools; i++) button(std::to_wstring(i + 1) + L" " + kToolNames[i], s.tool == i);
    x += 12;
    RECT swatch{x, 12, x + 16, ToolbarH() - 13};
    SetDCBrushColor(dc, rgb(kColours[s.colour]));
    FillRect(dc, &swatch, (HBRUSH)GetStockObject(DC_BRUSH));
    s.toolRects.push_back({x - 4, 0, x + 20, ToolbarH() - 1});
    x += 30;
    button(L"Undo", false);
    button(L"Done", false);
    RECT hint{x + 10, 0, rc.right - 12, ToolbarH() - 1};
    SetTextColor(dc, rgb(look.dim));
    DrawTextW(dc, L"C colour   Ctrl+Z undo   Enter keep   Esc plain", -1, &hint, DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    SelectObject(dc, oldFont);
}

void Paint(State& s) {
    RECT rc;
    GetClientRect(s.wnd, &rc);
    if (!s.rt) {
        D2D1_RENDER_TARGET_PROPERTIES p = D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT,
                                                                        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE));
        p.usage = D2D1_RENDER_TARGET_USAGE_GDI_COMPATIBLE;
        s.d2d->CreateHwndRenderTarget(p, D2D1::HwndRenderTargetProperties(s.wnd, D2D1::SizeU(rc.right, rc.bottom)), &s.rt);
        if (!s.rt) return;
    }
    if (!s.bitmap) {
        D2D1_BITMAP_PROPERTIES bp = D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE));
        s.rt->CreateBitmap(D2D1::SizeU(s.pic->w, s.pic->h), s.shown.data(), s.pic->w * 4, bp, &s.bitmap);
    }
    s.rt->BeginDraw();
    s.rt->SetTransform(D2D1::Matrix3x2F::Identity());
    s.rt->Clear(D2D1::ColorF(0.05f, 0.06f, 0.07f));
    s.rt->SetTransform(D2D1::Matrix3x2F::Scale(s.scale, s.scale) * D2D1::Matrix3x2F::Translation(s.ox, s.oy));
    if (s.bitmap) s.rt->DrawBitmap(s.bitmap, D2D1::RectF(0, 0, (float)s.pic->w, (float)s.pic->h));
    DrawMarks(s, s.rt);
    // The toolbar, with GDI in the menus' font.
    s.rt->SetTransform(D2D1::Matrix3x2F::Identity());
    ID2D1GdiInteropRenderTarget* gdi = nullptr;
    if (SUCCEEDED(s.rt->QueryInterface(__uuidof(ID2D1GdiInteropRenderTarget), (void**)&gdi))) {
        HDC dc = nullptr;
        if (SUCCEEDED(gdi->GetDC(D2D1_DC_INITIALIZE_MODE_COPY, &dc))) {
            DrawToolbar(s, dc, rc);
            gdi->ReleaseDC(nullptr);
        }
        gdi->Release();
    }
    if (s.rt->EndDraw() == (HRESULT)D2DERR_RECREATE_TARGET) {
        if (s.bitmap) s.bitmap->Release(), s.bitmap = nullptr;
        s.rt->Release(), s.rt = nullptr;
    }
}

D2D1_POINT_2F ToPicture(State const& s, int x, int y) { return {(x - s.ox) / s.scale, (y - s.oy) / s.scale}; }

float Width(State const& s, Tool t) {
    float base = std::max(2.f, std::max(s.pic->w, s.pic->h) / 400.f);
    return t == Highlight ? base * 6 : base;
}

void CommitText(State& s) {
    if (!s.edit) return;
    int n = GetWindowTextLengthW(s.edit);
    std::wstring t(n, L'\0');
    GetWindowTextW(s.edit, t.data(), n + 1);
    DestroyWindow(s.edit);
    s.edit = nullptr;
    if (!t.empty()) s.marks.push_back({Text, kColours[s.colour], Width(s, Text), {s.editAt}, t});
    InvalidateRect(s.wnd, nullptr, FALSE);
}

WNDPROC g_editProc = nullptr;
LRESULT CALLBACK EditProc(HWND w, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_KEYDOWN && (wp == VK_RETURN || wp == VK_ESCAPE)) {
        if (wp == VK_ESCAPE) SetWindowTextW(w, L"");
        CommitText(*g_state);
        SetFocus(g_state->wnd);
        return 0;
    }
    if (msg == WM_CHAR && (wp == VK_RETURN || wp == VK_ESCAPE)) return 0;
    return CallWindowProcW(g_editProc, w, msg, wp, lp);
}

void Undo(State& s) {
    if (s.marks.empty()) return;
    if (s.marks.back().tool == Marker) s.nextNumber = std::max(1, s.nextNumber - 1);
    bool redact = s.marks.back().tool == Redact;
    s.marks.pop_back();
    if (redact) Redraw(s);
    InvalidateRect(s.wnd, nullptr, FALSE);
}

void Finish(State& s, bool keep) {
    CommitText(s);
    s.keep = keep;
    s.done = true;
}

LRESULT CALLBACK Proc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) {
    State* s = g_state;
    if (!s) return DefWindowProcW(wnd, msg, wp, lp);
    switch (msg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            BeginPaint(wnd, &ps);
            Paint(*s);
            EndPaint(wnd, &ps);
            return 0;
        }
        case WM_ERASEBKGND:
            return 1;
        case WM_KEYDOWN:
            if (wp == VK_ESCAPE) Finish(*s, false);
            else if (wp == VK_RETURN) Finish(*s, true);
            else if (wp == 'Z' && GetKeyState(VK_CONTROL) < 0) Undo(*s);
            else if (wp == 'C') s->colour = (s->colour + 1) % (int)std::size(kColours);
            else if (wp >= '1' && wp < '1' + kTools) s->tool = (Tool)(wp - '1');
            InvalidateRect(wnd, nullptr, FALSE);
            return 0;
        case WM_LBUTTONDOWN: {
            int x = GET_X_LPARAM(lp), y = GET_Y_LPARAM(lp);
            CommitText(*s);
            if (y < ToolbarH()) {
                for (size_t i = 0; i < s->toolRects.size(); i++) {
                    RECT r = s->toolRects[i];
                    if (x < r.left || x >= r.right) continue;
                    if (i < kTools) s->tool = (Tool)i;
                    else if (i == kTools) s->colour = (s->colour + 1) % (int)std::size(kColours);
                    else if (i == kTools + 1) Undo(*s);
                    else Finish(*s, true);
                }
                InvalidateRect(wnd, nullptr, FALSE);
                return 0;
            }
            D2D1_POINT_2F p = ToPicture(*s, x, y);
            if (s->tool == Text) {
                s->editAt = p;
                int fontPx = (int)(Width(*s, Text) * 6 * s->scale);
                s->edit = CreateWindowExW(0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, x, y, 400, fontPx + 8, wnd, nullptr,
                                          GetModuleHandleW(nullptr), nullptr);
                HFONT f = CreateFontW(-fontPx, 0, 0, 0, FW_SEMIBOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");
                SendMessageW(s->edit, WM_SETFONT, (WPARAM)f, TRUE);
                g_editProc = (WNDPROC)SetWindowLongPtrW(s->edit, GWLP_WNDPROC, (LONG_PTR)EditProc);
                SetFocus(s->edit);
                return 0;
            }
            if (s->tool == Marker) {
                Mark m{Marker, kColours[s->colour], Width(*s, Marker), {p}};
                m.number = s->nextNumber++;
                s->marks.push_back(m);
                InvalidateRect(wnd, nullptr, FALSE);
                return 0;
            }
            s->marks.push_back({s->tool, kColours[s->colour], Width(*s, s->tool), {p, p}});
            if (s->tool == Pen || s->tool == Highlight) s->marks.back().points.pop_back();
            s->drawing = true;
            SetCapture(wnd);
            return 0;
        }
        case WM_MOUSEMOVE:
            if (s->drawing && !s->marks.empty()) {
                Mark& m = s->marks.back();
                D2D1_POINT_2F p = ToPicture(*s, GET_X_LPARAM(lp), GET_Y_LPARAM(lp));
                if (m.tool == Pen || m.tool == Highlight) m.points.push_back(p);
                else m.points.back() = p;
                InvalidateRect(wnd, nullptr, FALSE);
            }
            return 0;
        case WM_LBUTTONUP:
            if (s->drawing) {
                s->drawing = false;
                ReleaseCapture();
                if (!s->marks.empty() && s->marks.back().tool == Redact) Redraw(*s);
                InvalidateRect(wnd, nullptr, FALSE);
            }
            return 0;
        case WM_SETCURSOR:
            if (LOWORD(lp) == HTCLIENT) {
                POINT pt;
                GetCursorPos(&pt);
                ScreenToClient(wnd, &pt);
                SetCursor(LoadCursorW(nullptr, pt.y < ToolbarH() ? IDC_HAND : s->tool == Text ? IDC_IBEAM : IDC_CROSS));
                return TRUE;
            }
            break;
        case WM_CLOSE:
            Finish(*s, false);
            return 0;
        case WM_ACTIVATE:
            if (LOWORD(wp) == WA_INACTIVE && !s->edit) SetWindowPos(wnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
            return 0;
    }
    return DefWindowProcW(wnd, msg, wp, lp);
}

// The marks drawn into the picture itself, at its own size.
void Flatten(State& s) {
    Picture& p = *s.pic;
    BITMAPINFO bi{};
    bi.bmiHeader = {sizeof(BITMAPINFOHEADER), p.w, -p.h, 1, 32, BI_RGB};
    void* bits = nullptr;
    HDC screen = GetDC(nullptr);
    HDC mem = CreateCompatibleDC(screen);
    HBITMAP dib = CreateDIBSection(screen, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
    ReleaseDC(nullptr, screen);
    if (!dib) return DeleteDC(mem), void();
    HGDIOBJ old = SelectObject(mem, dib);
    memcpy(bits, s.shown.data(), s.shown.size() * 4);
    ID2D1DCRenderTarget* rt = nullptr;
    D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_SOFTWARE,
                                                                        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE));
    if (SUCCEEDED(s.d2d->CreateDCRenderTarget(&props, &rt))) {
        RECT all{0, 0, p.w, p.h};
        rt->BindDC(mem, &all);
        rt->BeginDraw();
        DrawMarks(s, rt);
        rt->EndDraw();
        rt->Release();
    }
    GdiFlush();
    p.px.assign((uint32_t*)bits, (uint32_t*)bits + (size_t)p.w * p.h);
    for (auto& v : p.px) v |= 0xFF000000;
    SelectObject(mem, old);
    DeleteObject(dib);
    DeleteDC(mem);
}

// The window, until a choice; true with the marks drawn in (Enter, Done), false for the plain one (Esc).
bool Run(Picture& pic) {
    if (pic.px.empty()) return false;
    State s;
    s.pic = &pic;
    D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory), nullptr, (void**)&s.d2d);
    DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)&s.dw);
    if (!s.d2d || !s.dw) return false;
    Redraw(s);
    static bool registered = false;
    if (!registered) {
        WNDCLASSW wc{};
        wc.lpfnWndProc = Proc;
        wc.hInstance = GetModuleHandleW(nullptr);
        wc.lpszClassName = L"TourneAnnotate";
        wc.hCursor = LoadCursorW(nullptr, IDC_CROSS);
        registered = RegisterClassW(&wc) != 0;
    }
    POINT pt;
    GetCursorPos(&pt);
    MONITORINFO mi{sizeof(mi)};
    GetMonitorInfoW(MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST), &mi);
    RECT m = mi.rcMonitor;
    int w = m.right - m.left, h = m.bottom - m.top;
    // The picture as big as fits under the toolbar, never larger than itself.
    float room = (float)(h - ToolbarH() - 24);
    s.scale = std::min({1.f, (w - 24.f) / pic.w, room / pic.h});
    s.ox = (w - pic.w * s.scale) / 2, s.oy = ToolbarH() + (h - ToolbarH() - pic.h * s.scale) / 2;
    g_state = &s;
    s.wnd = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, L"TourneAnnotate", L"Tourne capture: mark it up", WS_POPUP, m.left, m.top, w, h, nullptr,
                            nullptr, GetModuleHandleW(nullptr), nullptr);
    if (!s.wnd) {
        g_state = nullptr;
        return false;
    }
    ShowWindow(s.wnd, SW_SHOW);
    SetForegroundWindow(s.wnd);
    SetFocus(s.wnd);
    MSG msg;
    BOOL got = 1;
    while (!s.done && (got = GetMessageW(&msg, nullptr, 0, 0)) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    if (got == 0) PostQuitMessage((int)msg.wParam);  // (the capture thread asked to end meanwhile: it still does)
    if (s.keep && (!s.marks.empty())) Flatten(s);
    DestroyWindow(s.wnd);
    g_state = nullptr;
    if (s.bitmap) s.bitmap->Release();
    if (s.rt) s.rt->Release();
    s.dw->Release();
    s.d2d->Release();
    return s.keep;
}

}  // namespace annotate
