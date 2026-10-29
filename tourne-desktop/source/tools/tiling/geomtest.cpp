// Every tiling layout's geometry, checked without a screen: tiling-geometry.cpp
// on its own, with a stand-in RECT where there's no Windows (so this builds and
// runs on any machine, including the ones that only hold the repo).
//
//   x86_64-w64-mingw32-g++ -std=c++23 -o geomtest.exe geomtest.cpp   (Windows)
//   g++ -std=c++23 -o geomtest geomtest.cpp                          (anywhere)
//
// What it checks, for one to twelve windows, on a wide screen and a tall one:
//   - every rectangle sits inside the screen and has room in it;
//   - with no gaps, the rectangles cover the screen exactly and never overlap;
//   - with gaps, facing edges are at least the gap apart and the outer edges
//     still reach the screen's.
// Monocle and Layers lay windows over each other on purpose, so they are only
// checked for sitting inside the screen. A case with a window under 2 px (BSP
// and Spiral halve, so a dozen windows outrun any screen) is counted as out of
// room and only checked the same way.
//
// Then where a window that has just opened lands in the order, each rule in
// turn.
#ifdef _WIN32
#include <windows.h>
#else
using LONG = long;
using HWND = void*;
struct RECT {
    LONG left, top, right, bottom;
};
#endif

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <utility>
#include <vector>

#include "../../tiling-geometry.cpp"

namespace {

int g_failures, g_tight;

struct Named {
    TileLayout layout;
    const char* name;
};

constexpr Named kAll[] = {
    {TileLayout::Columns, "columns"},
    {TileLayout::Rows, "rows"},
    {TileLayout::VerticalStack, "verticalStack"},
    {TileLayout::RightMainVerticalStack, "rightMainVerticalStack"},
    {TileLayout::HorizontalStack, "horizontalStack"},
    {TileLayout::UltrawideVerticalStack, "ultrawideVerticalStack"},
    {TileLayout::Grid, "grid"},
    {TileLayout::Bsp, "bsp"},
    {TileLayout::Spiral, "spiral"},
    {TileLayout::ThreeColumn, "threeColumn"},
    {TileLayout::Slice, "slice"},
    // Both of these need more than a count, so with only a count they stand in
    // for themselves: a grid for the regions, columns for a script. Checked
    // here so that a screen with neither written still tiles.
    {TileLayout::ScreenSplit, "screenSplit (no regions)"},
    {TileLayout::Script, "script (no file)"},
    {TileLayout::Monocle, "monocle"},
    {TileLayout::Layers, "layers"},
};

bool Overlapping(TileLayout layout) { return layout == TileLayout::Monocle || layout == TileLayout::Layers; }

void Fail(const char* name, RECT screen, int n, int gap, const char* what) {
    printf("  FAIL %-24s %2d window%s, gap %d, screen %ldx%ld: %s\n", name, n, n == 1 ? " " : "s", gap,
           screen.right - screen.left, screen.bottom - screen.top, what);
    g_failures++;
}

long long Area(RECT const& r) { return (long long)(r.right - r.left) * (r.bottom - r.top); }

// The two rectangles share a span across, so they face each other up and down.
bool FaceUpDown(RECT const& a, RECT const& b) { return a.left < b.right && b.left < a.right; }
bool FaceAcross(RECT const& a, RECT const& b) { return a.top < b.bottom && b.top < a.bottom; }

void Check(Named const& named, RECT screen, int n, int gap, int percent) {
    std::vector<RECT> rects = LayoutRects(named.layout, screen, n, gap, percent);
    if ((int)rects.size() != n) {
        Fail(named.name, screen, n, gap, "a rectangle for every window is what's wanted");
        return;
    }
    for (RECT const& r : rects) {
        if (r.left < screen.left || r.top < screen.top || r.right > screen.right || r.bottom > screen.bottom)
            return Fail(named.name, screen, n, gap, "a rectangle hangs off the screen");
        if (r.right - r.left < 1 || r.bottom - r.top < 1)
            return Fail(named.name, screen, n, gap, "a rectangle has no room in it");
    }
    if (Overlapping(named.layout)) return;
    // Out of room: the layout stacks what's left over itself, on purpose.
    for (RECT const& r : rects)
        if (r.right - r.left < 2 || r.bottom - r.top < 2) {
            g_tight++;
            return;
        }

    for (size_t i = 0; i < rects.size(); i++)
        for (size_t j = i + 1; j < rects.size(); j++) {
            RECT const& a = rects[i];
            RECT const& b = rects[j];
            if (FaceAcross(a, b) && FaceUpDown(a, b)) return Fail(named.name, screen, n, gap, "two rectangles overlap");
            if (gap <= 0) continue;
            // Side by side or one above the other: the space between them is the
            // gap. (Rectangles with a third between them are further apart, which
            // is why this is a floor and not an equality.)
            int apart = FaceAcross(a, b) ? std::max(b.left - a.right, a.left - b.right)
                        : FaceUpDown(a, b) ? std::max(b.top - a.bottom, a.top - b.bottom)
                                           : gap;
            if (apart < gap) return Fail(named.name, screen, n, gap, "two rectangles are closer than the gap");
        }

    LONG left = rects[0].left, top = rects[0].top, right = rects[0].right, bottom = rects[0].bottom;
    long long covered = 0;
    for (RECT const& r : rects) {
        left = std::min(left, r.left), top = std::min(top, r.top);
        right = std::max(right, r.right), bottom = std::max(bottom, r.bottom);
        covered += Area(r);
    }
    if (left != screen.left || top != screen.top || right != screen.right || bottom != screen.bottom)
        return Fail(named.name, screen, n, gap, "the windows don't reach all four edges of the screen");
    if (gap == 0 && covered != Area(screen))
        return Fail(named.name, screen, n, gap, "with no gaps the windows should cover the screen exactly");
}

// Slice over each edge, each strip size and each layout for the rest: the
// strip is first and on the edge it was asked for, and the rest still fill the
// screen between them.
void CheckSlice() {
    printf("slice over every edge and fallback\n");
    constexpr RECT screen{0, 0, 3440, 1440};
    const char* edges[] = {"left", "right", "top", "bottom"};
    for (int edge = kSliceLeft; edge <= kSliceBottom; edge++)
        for (int size : {0, 20, 35, 60})
            for (TileLayout rest : {TileLayout::Columns, TileLayout::Rows, TileLayout::VerticalStack, TileLayout::Grid,
                                    TileLayout::Bsp, TileLayout::Monocle, TileLayout::Slice})
                for (int n = 1; n <= 8; n++)
                    for (int gap : {0, 8}) {
                        SliceSpec spec{edge, size, rest};
                        std::vector<RECT> rects = LayoutRects(TileLayout::Slice, screen, n, gap, 50, spec);
                        auto fail = [&](const char* what) {
                            printf("  FAIL %-6s size %2d, %d window%s, gap %d: %s\n", edges[edge], size, n, n == 1 ? "" : "s",
                                   gap, what);
                            g_failures++;
                        };
                        if ((int)rects.size() != n) {
                            fail("a rectangle for every window is what's wanted");
                            continue;
                        }
                        if (n == 1) continue;  // one window fills the screen, strip and all
                        RECT const& strip = rects[0];
                        bool onEdge = edge == kSliceLeft     ? strip.left == screen.left && strip.right < screen.right
                                      : edge == kSliceRight  ? strip.right == screen.right && strip.left > screen.left
                                      : edge == kSliceTop    ? strip.top == screen.top && strip.bottom < screen.bottom
                                                             : strip.bottom == screen.bottom && strip.top > screen.top;
                        if (!onEdge) fail("the strip isn't on the edge it was asked for");
                        LONG left = rects[0].left, top = rects[0].top, right = rects[0].right, bottom = rects[0].bottom;
                        for (RECT const& r : rects) {
                            left = std::min(left, r.left), top = std::min(top, r.top);
                            right = std::max(right, r.right), bottom = std::max(bottom, r.bottom);
                            if (r.right - r.left < 1 || r.bottom - r.top < 1) fail("a rectangle has no room in it");
                        }
                        if (left != screen.left || top != screen.top || right != screen.right || bottom != screen.bottom)
                            fail("the windows don't reach all four edges of the screen");
                    }
}

// The Split screen regions: a screen cut into shares, as the tiling.split lines
// write them. Each region's box sits inside the screen, boxes beside each other
// are a gap apart and never overlap, and between them they reach all four
// edges.
void CheckRegions() {
    printf("split screen regions\n");
    struct Share {
        int l, t, w, h;
    };
    struct Case {
        const char* what;
        std::vector<Share> shares;
    } const cases[] = {
        {"a sidebar and the rest", {{0, 0, 25, 100}, {25, 0, 75, 100}}},
        {"three columns", {{0, 0, 34, 100}, {34, 0, 33, 100}, {67, 0, 33, 100}}},
        {"top and bottom", {{0, 0, 100, 40}, {0, 40, 100, 60}}},
        {"quarters", {{0, 0, 50, 50}, {50, 0, 50, 50}, {0, 50, 50, 50}, {50, 50, 50, 50}}},
        {"a strip down the middle", {{0, 0, 20, 100}, {20, 0, 60, 100}, {80, 0, 20, 100}}},
        {"the whole screen", {{0, 0, 100, 100}}},
    };
    constexpr RECT kScreens[] = {{0, 0, 3440, 1440}, {1920, 0, 3360, 2560}, {0, 0, 1280, 1024}};
    for (Case const& t : cases)
        for (RECT screen : kScreens)
            for (int gap : {0, 6, 24}) {
                auto fail = [&](const char* what) {
                    printf("  FAIL %-26s gap %2d, screen %ldx%ld: %s\n", t.what, gap, screen.right - screen.left,
                           screen.bottom - screen.top, what);
                    g_failures++;
                };
                std::vector<RECT> boxes;
                for (Share const& sh : t.shares) boxes.push_back(RegionArea(screen, sh.l, sh.t, sh.w, sh.h, gap));
                for (RECT const& r : boxes) {
                    if (r.left < screen.left || r.top < screen.top || r.right > screen.right || r.bottom > screen.bottom)
                        fail("a region hangs off the screen");
                    if (r.right - r.left < 1 || r.bottom - r.top < 1) fail("a region has no room in it");
                }
                for (size_t i = 0; i < boxes.size(); i++)
                    for (size_t j = i + 1; j < boxes.size(); j++) {
                        RECT const& a = boxes[i];
                        RECT const& b = boxes[j];
                        if (FaceAcross(a, b) && FaceUpDown(a, b)) fail("two regions overlap");
                        else if (gap > 0) {
                            int apart = FaceAcross(a, b) ? std::max(b.left - a.right, a.left - b.right)
                                        : FaceUpDown(a, b) ? std::max(b.top - a.bottom, a.top - b.bottom)
                                                           : gap;
                            if (apart < gap) fail("two regions are closer than the gap");
                        }
                    }
                LONG left = boxes[0].left, top = boxes[0].top, right = boxes[0].right, bottom = boxes[0].bottom;
                for (RECT const& r : boxes) {
                    left = std::min(left, r.left), top = std::min(top, r.top);
                    right = std::max(right, r.right), bottom = std::max(bottom, r.bottom);
                }
                if (left != screen.left || top != screen.top || right != screen.right || bottom != screen.bottom)
                    fail("the regions don't reach all four edges of the screen");
            }
    // Shares nobody should write: a region still has room in it and stays on
    // the screen, because a region with no room would put windows nowhere.
    constexpr RECT screen{0, 0, 1280, 1024};
    struct Silly {
        int l, t, w, h;
    } const silly[] = {{0, 0, 0, 0}, {0, 0, 1, 1}, {99, 99, 40, 40}, {-20, -20, 50, 50}, {0, 0, 400, 400}};
    for (Silly const& sh : silly) {
        RECT r = RegionArea(screen, sh.l, sh.t, sh.w, sh.h, 24);
        if (r.right - r.left < 1 || r.bottom - r.top < 1 || r.left < screen.left || r.top < screen.top ||
            r.right > screen.right || r.bottom > screen.bottom) {
            printf("  FAIL a share of %d %d %d %d leaves nowhere to put a window\n", sh.l, sh.t, sh.w, sh.h);
            g_failures++;
        }
    }
}

// Where a window that opens lands in the order. Handles stand in for windows:
// what matters is where in the list they end up.
void CheckOrder() {
    HWND a = (HWND)1, b = (HWND)2, c = (HWND)3, fresh = (HWND)9, stranger = (HWND)7;
    std::vector<HWND> order{a, b, c}, opened{fresh};
    struct Case {
        const char* what;
        int rule;
        HWND focused, before;
        size_t at;
    } const cases[] = {
        {"end, as it always was", kNewEnd, b, a, 3},
        {"first, the main window", kNewFirst, b, a, 0},
        {"after the focused one", kNewAfterFocused, b, a, 2},
        {"before the focused one", kNewBeforeFocused, b, a, 1},
        // The new window is in front by the time we look: the one before it stands in.
        {"after, the new window in front", kNewAfterFocused, fresh, c, 3},
        {"before, the new window in front", kNewBeforeFocused, fresh, c, 2},
        // Nothing to go beside (the focus was on another screen, or nowhere).
        {"after, nothing focused", kNewAfterFocused, nullptr, nullptr, 3},
        {"before, a stranger focused", kNewBeforeFocused, stranger, stranger, 3},
        {"after the first one", kNewAfterFocused, a, nullptr, 1},
    };
    printf("where a window that opens lands\n");
    for (Case const& t : cases) {
        size_t at = NewWindowAt(order, opened, t.rule, t.focused, t.before);
        if (at != t.at) {
            printf("  FAIL %-34s place %zu, wanted %zu\n", t.what, at, t.at);
            g_failures++;
        }
    }
}

}  // namespace

int main() {
    constexpr RECT kScreens[] = {{0, 0, 3440, 1440}, {1920, 0, 3360, 2560}, {0, 0, 1280, 1024}};
    for (Named const& named : kAll) {
        printf("%s\n", named.name);
        for (RECT screen : kScreens)
            for (int n = 1; n <= 12; n++)
                for (int gap : {0, 6, 24})
                    for (int percent : {30, 50, 65})
                        Check(named, screen, n, gap, percent);
    }
    CheckSlice();
    CheckRegions();
    CheckOrder();
    printf("\n%s", g_failures ? "failures above" : "everything checks out");
    if (g_tight) printf(" (%d cases had more windows than the screen has room for)", g_tight);
    printf("\n");
    return g_failures ? 1 : 0;
}
