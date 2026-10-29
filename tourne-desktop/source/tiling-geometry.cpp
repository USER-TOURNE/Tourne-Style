////////////////////////////////////////////////////////////////////////////////
// The layouts themselves ("// @inline tiling-geometry.cpp", from tiling.cpp):
// the list of layouts and the plain geometry that turns one into rectangles.
// Nothing here touches a window, a screen or a setting, so it builds and runs
// on its own (tools/tiling/geomtest.cpp checks every layout this way).
//
// A fragment, like the other inlined files: whoever inlines it has already
// included <algorithm>, <cmath>, <utility> and <vector>.

enum class TileLayout {
    Off,
    Same,  // other screens: as the main screen
    Columns,
    Rows,
    VerticalStack,
    RightMainVerticalStack,
    HorizontalStack,
    UltrawideVerticalStack,
    Grid,
    Bsp,
    Spiral,       // each window takes a share off one side of what's left, turning a quarter each time
    ThreeColumn,  // the main window down the middle, the rest in a column each side
    Slice,        // one window keeps a strip along one edge, another layout takes the rest
    ScreenSplit,  // the screen cut into named regions, each with its own layout and its own windows
    Script,       // a layout written in Luau, in the Layouts folder (desktop/host/layout-scripts.cpp)
    Monocle,
    Layers,  // threes laid over each other: 1 fills, 2 halves, 3 main and two; the 4th starts a layer on top
};

constexpr TileLayout kLayoutCycle[] = {
    TileLayout::Columns,        TileLayout::Rows,
    TileLayout::VerticalStack,  TileLayout::RightMainVerticalStack,
    TileLayout::HorizontalStack, TileLayout::UltrawideVerticalStack,
    TileLayout::Grid,           TileLayout::Bsp,
    TileLayout::Spiral,         TileLayout::ThreeColumn,
    TileLayout::Slice,          TileLayout::ScreenSplit,
    TileLayout::Script,         TileLayout::Monocle,
    TileLayout::Layers,
};


// Slice: one window keeps a strip along an edge of the screen and the rest are
// laid out by another layout in what's left (qtile's Slice; the Spotify and
// Discord strip as a layout instead of a rule each). The window with the strip
// is the first in the screen's order.
enum { kSliceLeft, kSliceRight, kSliceTop, kSliceBottom };

struct SliceSpec {
    int edge = kSliceLeft;
    int percent = 0;                            // of the screen; 0 means the main window's share
    TileLayout fallback = TileLayout::Columns;  // the rest of the windows
};

// ScreenSplit: a region's share of the screen, written as percentages of each
// side, in pixels. Half the gap comes off each side that isn't the screen's own
// edge, so two regions side by side leave exactly one gap between them.
RECT RegionArea(RECT screen, int l, int t, int w, int h, int gap) {
    int sw = screen.right - screen.left, sh = screen.bottom - screen.top;
    l = std::clamp(l, 0, 100), t = std::clamp(t, 0, 100);
    w = std::clamp(w, 1, 100 - l), h = std::clamp(h, 1, 100 - t);
    RECT out{screen.left + sw * l / 100, screen.top + sh * t / 100, screen.left + sw * (l + w) / 100,
             screen.top + sh * (t + h) / 100};
    int half = gap / 2, rest = gap - half;
    if (l > 0) out.left += half;
    if (t > 0) out.top += half;
    if (l + w < 100) out.right -= rest;
    if (t + h < 100) out.bottom -= rest;
    // A share so small the gap eats it still gets a pixel, taken from inside the
    // screen: a region with no room would put its windows nowhere.
    auto room = [](LONG* near_, LONG* far_, LONG edge) {
        if (*far_ > *near_) return;
        *far_ = *near_ + 1;
        if (*far_ > edge) *far_ = edge, *near_ = edge - 1;
    };
    room(&out.left, &out.right, screen.right);
    room(&out.top, &out.bottom, screen.bottom);
    return out;
}

// Where a window that has just opened goes in a screen's order (qtile's
// new_client_position).
enum { kNewEnd, kNewFirst, kNewAfterFocused, kNewBeforeFocused };

// The place in the order for the windows that have just opened: the end, where
// they always went, first (the main window in the layouts that have one), or
// either side of the window that was focused when they opened. A new window is
// usually in front by now, so the one focused before it stands in; with neither
// in the order, the end.
size_t NewWindowAt(std::vector<HWND> const& ordered, std::vector<HWND> const& fresh, int rule, HWND focused,
                   HWND focusedBefore) {
    if (rule == kNewFirst) return 0;
    if (rule != kNewAfterFocused && rule != kNewBeforeFocused) return ordered.size();
    auto known = [&](HWND h) {
        return h && std::find(fresh.begin(), fresh.end(), h) == fresh.end() &&
               std::find(ordered.begin(), ordered.end(), h) != ordered.end();
    };
    HWND anchor = known(focused) ? focused : known(focusedBefore) ? focusedBefore : nullptr;
    if (!anchor) return ordered.size();
    size_t at = std::find(ordered.begin(), ordered.end(), anchor) - ordered.begin();
    return rule == kNewAfterFocused ? at + 1 : at;
}

// A length cut into pieces with gaps between; the odd pixels go to the first.
// Every piece keeps at least a pixel: on a length too short for the gaps asked
// for, the gaps give way, so a layout never hands out an empty window.
std::vector<std::pair<int, int>> Cut(int start, int length, int count, int gap) {
    std::vector<std::pair<int, int>> pieces;
    if (count <= 0) return pieces;
    if (count > 1) gap = std::clamp(gap, 0, std::max(0, (length - count) / (count - 1)));
    int room = std::max(0, length - gap * (count - 1));
    int each = room / count, extra = room % count;
    for (int i = 0, at = start; i < count; i++) {
        int size = each + (i < extra ? 1 : 0);
        pieces.push_back({at, at + size});
        at += size + gap;
    }
    return pieces;
}

// A length cut in two by a share (percent) of it for the first part. Neither
// part is ever empty: on a short length the gap gives way first, then the share.
std::pair<std::pair<int, int>, std::pair<int, int>> CutShare(int start, int length, int percent, int gap) {
    if (length < 2) return {{start, start + length}, {start + length, start + length}};  // no room to cut
    gap = std::clamp(gap, 0, length - 2);
    int room = length - gap;
    int first = std::clamp(room * percent / 100, 1, room - 1);
    return {{start, start + first}, {start + first + gap, start + length}};
}

// Out of room: the windows still waiting all get what's left, over each other,
// the way Monocle does. Only reachable with far more windows than a screen can
// hold (halving a wide screen a dozen times).
void NoRoomLeft(RECT area, int count, std::vector<RECT>* out) {
    for (int i = 0; i < count; i++) out->push_back(area);
}

void Bsp(RECT area, int count, bool across, int percent, int gap, std::vector<RECT>* out) {
    if (count == 1) {
        out->push_back(area);
        return;
    }
    if ((across ? area.right - area.left : area.bottom - area.top) < 2) return NoRoomLeft(area, count, out);
    RECT a = area, b = area;
    if (across) {
        auto [l, r] = CutShare(area.left, area.right - area.left, percent, gap);
        a.left = l.first, a.right = l.second, b.left = r.first, b.right = r.second;
    } else {
        auto [t, u] = CutShare(area.top, area.bottom - area.top, percent, gap);
        a.top = t.first, a.bottom = t.second, b.top = u.first, b.bottom = u.second;
    }
    out->push_back(a);
    Bsp(b, count - 1, !across, 50, gap, out);
}

// A spiral: each window takes a share off one side of what's left and the side
// turns a quarter each time (left, top, right, bottom), so the windows wind
// inwards. The first window's share is the ratio, the rest halve what's left.
void Spiral(RECT area, int count, int turn, int percent, int gap, std::vector<RECT>* out) {
    if (count <= 1) {
        out->push_back(area);
        return;
    }
    bool across = turn % 2 == 0;  // 0 and 2 cut left to right, 1 and 3 top to bottom
    if ((across ? area.right - area.left : area.bottom - area.top) < 2) return NoRoomLeft(area, count, out);
    bool front = turn < 2;        // 0 and 1 take the near side of the cut, 2 and 3 the far side
    RECT mine = area, rest = area;
    if (across) {
        auto [l, r] = CutShare(area.left, area.right - area.left, front ? percent : 100 - percent, gap);
        auto [take, leave] = front ? std::pair{l, r} : std::pair{r, l};
        mine.left = take.first, mine.right = take.second;
        rest.left = leave.first, rest.right = leave.second;
    } else {
        auto [t, b] = CutShare(area.top, area.bottom - area.top, front ? percent : 100 - percent, gap);
        auto [take, leave] = front ? std::pair{t, b} : std::pair{b, t};
        mine.top = take.first, mine.bottom = take.second;
        rest.top = leave.first, rest.bottom = leave.second;
    }
    out->push_back(mine);
    Spiral(rest, count - 1, (turn + 1) % 4, 50, gap, out);
}

std::vector<RECT> LayoutRects(TileLayout layout, RECT area, int n, int gap, int percent, SliceSpec const& slice = {}) {
    std::vector<RECT> rects;
    if (n <= 0) return rects;
    int w = area.right - area.left, h = area.bottom - area.top;
    auto column = [&](int left, int right, int count) {
        for (auto [t, b] : Cut(area.top, h, count, gap)) rects.push_back({left, t, right, b});
    };
    auto row = [&](int top, int bottom, int count) {
        for (auto [l, r] : Cut(area.left, w, count, gap)) rects.push_back({l, top, r, bottom});
    };
    if (n == 1 || layout == TileLayout::Monocle) {
        rects.assign(n, area);
        return rects;
    }
    switch (layout) {
        case TileLayout::Rows:
            column(area.left, area.right, n);
            break;
        case TileLayout::VerticalStack: {
            auto [main, stack] = CutShare(area.left, w, percent, gap);
            rects.push_back({main.first, area.top, main.second, area.bottom});
            column(stack.first, stack.second, n - 1);
            break;
        }
        case TileLayout::RightMainVerticalStack: {
            auto [stack, main] = CutShare(area.left, w, 100 - percent, gap);
            rects.push_back({main.first, area.top, main.second, area.bottom});
            column(stack.first, stack.second, n - 1);
            break;
        }
        case TileLayout::HorizontalStack: {
            auto [main, stack] = CutShare(area.top, h, percent, gap);
            rects.push_back({area.left, main.first, area.right, main.second});
            row(stack.first, stack.second, n - 1);
            break;
        }
        case TileLayout::UltrawideVerticalStack: {
            if (n == 2) return LayoutRects(TileLayout::VerticalStack, area, n, gap, percent);
            // The main window in the middle, one to its left, the rest stacked
            // to its right.
            int mainW = std::max(0, w - 2 * gap) * percent / 100;
            int side = std::max(0, w - 2 * gap - mainW) / 2;
            int mainLeft = area.left + side + gap;
            rects.push_back({mainLeft, area.top, mainLeft + mainW, area.bottom});
            rects.push_back({area.left, area.top, area.left + side, area.bottom});
            column(mainLeft + mainW + gap, area.right, n - 2);
            break;
        }
        case TileLayout::Grid: {
            int columns = (int)std::ceil(std::sqrt((double)n));
            auto spans = Cut(area.left, w, columns, gap);
            for (int c = 0; c < columns; c++) {
                int count = n / columns + (c < n % columns ? 1 : 0);
                if (count) column(spans[c].first, spans[c].second, count);
            }
            break;
        }
        case TileLayout::Bsp:
            Bsp(area, n, true, percent, gap, &rects);
            break;
        case TileLayout::Spiral:
            Spiral(area, n, 0, percent, gap, &rects);
            break;
        case TileLayout::Slice: {
            int share = slice.percent ? std::clamp(slice.percent, 5, 90) : percent;
            bool across = slice.edge == kSliceLeft || slice.edge == kSliceRight;
            bool first = slice.edge == kSliceLeft || slice.edge == kSliceTop;
            RECT strip = area, rest = area;
            if (across) {
                auto [l, r] = CutShare(area.left, w, first ? share : 100 - share, gap);
                auto [mine, theirs] = first ? std::pair{l, r} : std::pair{r, l};
                strip.left = mine.first, strip.right = mine.second;
                rest.left = theirs.first, rest.right = theirs.second;
            } else {
                auto [t, b] = CutShare(area.top, h, first ? share : 100 - share, gap);
                auto [mine, theirs] = first ? std::pair{t, b} : std::pair{b, t};
                strip.top = mine.first, strip.bottom = mine.second;
                rest.top = theirs.first, rest.bottom = theirs.second;
            }
            rects.push_back(strip);
            // Another Slice in the rest would want a second window pinned, which
            // there's no way to name, so it falls back to columns.
            TileLayout rest_layout = slice.fallback;
            if (rest_layout == TileLayout::Slice || rest_layout == TileLayout::Off || rest_layout == TileLayout::Same)
                rest_layout = TileLayout::Columns;
            for (RECT const& r : LayoutRects(rest_layout, rest, n - 1, gap, percent)) rects.push_back(r);
            break;
        }
        case TileLayout::ThreeColumn: {
            if (n == 2) return LayoutRects(TileLayout::VerticalStack, area, n, gap, percent);
            // The main window down the middle, the rest shared between a column
            // each side (the odd one goes right).
            int mainW = std::max(0, w - 2 * gap) * percent / 100;
            int side = std::max(0, w - 2 * gap - mainW) / 2;
            int mainLeft = area.left + side + gap;
            rects.push_back({mainLeft, area.top, mainLeft + mainW, area.bottom});
            int left = (n - 1) / 2;
            column(area.left, area.left + side, left);
            column(mainLeft + mainW + gap, area.right, n - 1 - left);
            break;
        }
        case TileLayout::Layers: {
            // The first three as the vertical stack grows (one fills, two halves, then the main and two
            // stacked); each three after is a layer over them in the three places, the 4th in the main.
            for (int at = 0; at < n; at += 3) {
                int count = at == 0 ? std::min(3, n) : 3;
                auto layer = LayoutRects(TileLayout::VerticalStack, area, count, gap, percent);
                for (int i = 0; i < count && at + i < n; i++) rects.push_back(layer[i]);
            }
            break;
        }
        case TileLayout::ScreenSplit:
            // The regions and which windows go in them are the tiling's
            // (SplitGroups in tiling.cpp); with only a count to go on, a grid
            // is the nearest thing.
            return LayoutRects(TileLayout::Grid, area, n, gap, percent);
        case TileLayout::Script:
            // A script is handed the windows themselves, so it is asked for in
            // tiling.cpp (RectsIn); columns stand in wherever it can't be.
            row(area.top, area.bottom, n);
            break;
        default:  // Columns
            row(area.top, area.bottom, n);
            break;
    }
    return rects;
}
