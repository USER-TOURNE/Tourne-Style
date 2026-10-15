// A small CSS engine in the flavour YASB themes are written in: parse a stylesheet, match its rules against a tree of
// nodes (classes, states, descendants), and work out each node's style. No drawing here; see css-draw.cpp.
//
// What it understands (measured against the 68 themes of amnweb/yasb-themes, see research/):
//   selectors   *, .class, compound .a.b, descendant and child (>) combinators, lists, :hover :active :pressed :checked
//               :disabled :focus :selected :root and :last-child; rules with other pseudo-classes, pseudo-elements (::item,
//               ::groove...), attributes or @-rules are skipped and counted
//   values      colours (#rgb #rgba #rrggbb #rrggbbaa rgb() rgba() hsl() hsla() and names), px lengths, var() with fallbacks,
//               the shorthands padding margin border border-radius background
//   properties  color background(-color) linear-gradient border(-side/-color/-width) border-radius padding margin font-*
//               opacity width height min/max-* transition (kept for the animator)
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cwctype>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace css {

struct Rgba {
    int r = 0, g = 0, b = 0;
    double a = 0;  // 0 clear .. 1 opaque
    bool operator==(Rgba const& o) const { return r == o.r && g == o.g && b == o.b && a == o.a; }
};

struct Edges {
    double top = 0, right = 0, bottom = 0, left = 0;
};

struct GradientStop {
    Rgba color;
    double at = 0;  // 0..1
};

struct Gradient {
    double angle = 180;  // degrees, CSS style: 180 is top to bottom
    std::vector<GradientStop> stops;
    bool radial = false;
};

struct Transition {
    std::wstring property;  // "all" or a property name
    double seconds = 0;
    double delay = 0;
    std::wstring easing = L"ease";
};

struct Shadow {
    double dx = 0, dy = 0, blur = 0, spread = 0;
    Rgba color{0, 0, 0, 0.5};
};

struct Style {
    // Inherited.
    Rgba color{205, 214, 244, 1};
    double fontSize = 12;
    int fontWeight = 400;
    std::vector<std::wstring> fontFamily;
    // Not inherited.
    Rgba background;
    bool hasGradient = false;
    Gradient gradient;
    Edges margin, padding, borderWidth;
    Rgba borderColor[4];  // top, right, bottom, left
    double radius[4] = {0, 0, 0, 0};  // top-left, top-right, bottom-right, bottom-left
    double opacity = 1;
    double width = -1, height = -1, minWidth = -1, maxWidth = -1, minHeight = -1, maxHeight = -1;
    std::vector<Transition> transitions;
    std::vector<Shadow> shadows;
};

struct Decl {
    std::wstring name, value;
    bool important = false;
};

struct Compound {
    std::wstring tag;  // "" or "*" for any
    std::vector<std::wstring> classes;
    std::vector<std::wstring> pseudos;
    bool child = false;  // reached from its left neighbour by ">" rather than by descent
};

struct Selector {
    std::vector<Compound> parts;  // left to right; the last one is the subject
    int specificity = 0;
};

struct Rule {
    std::vector<Selector> selectors;
    std::vector<Decl> decls;
    int order = 0;
};

struct Report {
    int rules = 0, skippedRules = 0, decls = 0, unknownDecls = 0;
    std::map<std::wstring, int> unknownProps;      // properties we don't act on
    std::map<std::wstring, int> skippedSelectors;  // why a selector was skipped (its odd part)
};

struct Sheet {
    std::vector<Rule> rules;
    std::map<std::wstring, std::wstring> vars;  // custom properties from :root (and *)
    Report report;
    // Which rules can match a node with a given class: by the first class of each selector's subject (the rest are always tried).
    std::map<std::wstring, std::vector<int>> byClass;
    std::vector<int> always;
    bool indexed = false;
};

struct Node {
    std::vector<std::wstring> classes;
    std::set<std::wstring> states;  // hover, pressed, checked...
    std::wstring tag;
    Node* parent = nullptr;
    std::vector<std::unique_ptr<Node>> kids;
    Style style;
    Node* Add(std::initializer_list<std::wstring> cls, std::wstring text = L"") {
        auto n = std::make_unique<Node>();
        n->classes = cls;
        n->parent = this;
        n->text = std::move(text);
        kids.push_back(std::move(n));
        return kids.back().get();
    }
    bool Has(std::wstring const& c) const { return std::find(classes.begin(), classes.end(), c) != classes.end(); }
    // What the drawing side fills in.
    std::wstring text;
    double x = 0, y = 0, w = 0, h = 0;  // the border box, in bar pixels
    double iconSize = 0;                // above 0: an icon of this size instead of text
    int payload = 0;                    // free for the caller (an icon index, say)
    // Popups stack their children (a column) instead of setting them in a row, and may centre them across.
    bool vertical = false, centerKids = false;
    std::wstring key;                   // says which node this is from one build of the tree to the next (for fades)
};

// ---------------------------------------------------------------------------------------------------- text helpers

inline std::wstring Trim(std::wstring s) {
    size_t a = 0, b = s.size();
    while (a < b && iswspace(s[a])) a++;
    while (b > a && iswspace(s[b - 1])) b--;
    return s.substr(a, b - a);
}

inline std::wstring Lower(std::wstring s) {
    for (auto& c : s) c = (wchar_t)towlower(c);
    return s;
}

inline bool StartsWith(std::wstring const& s, std::wstring const& p) { return s.compare(0, p.size(), p) == 0; }

// Splits at a separator outside brackets and quotes.
inline std::vector<std::wstring> SplitTop(std::wstring const& s, wchar_t sep) {
    std::vector<std::wstring> out;
    int depth = 0;
    wchar_t quote = 0;
    std::wstring cur;
    for (wchar_t c : s) {
        if (quote) {
            cur += c;
            if (c == quote) quote = 0;
            continue;
        }
        if (c == L'"' || c == L'\'') quote = c;
        if (c == L'(' || c == L'[') depth++;
        if (c == L')' || c == L']') depth--;
        if (c == sep && depth == 0) {
            out.push_back(cur);
            cur.clear();
        } else {
            cur += c;
        }
    }
    out.push_back(cur);
    return out;
}

// The value split at whitespace outside brackets and quotes.
inline std::vector<std::wstring> Words(std::wstring const& s) {
    std::vector<std::wstring> out;
    int depth = 0;
    wchar_t quote = 0;
    std::wstring cur;
    for (wchar_t c : s) {
        if (quote) {
            cur += c;
            if (c == quote) quote = 0;
            continue;
        }
        if (c == L'"' || c == L'\'') quote = c;
        if (c == L'(') depth++;
        if (c == L')') depth--;
        if (iswspace(c) && depth == 0) {
            if (!cur.empty()) out.push_back(cur);
            cur.clear();
        } else {
            cur += c;
        }
    }
    if (!cur.empty()) out.push_back(cur);
    return out;
}

// ---------------------------------------------------------------------------------------------------- values

inline bool ParseNumber(std::wstring const& s, double* out, std::wstring* unit = nullptr) {
    if (s.empty()) return false;
    wchar_t* end = nullptr;
    double v = wcstod(s.c_str(), &end);
    if (end == s.c_str()) return false;
    if (unit) *unit = Lower(end);
    else if (*end) return false;
    *out = v;
    return true;
}

// Every length read is multiplied by this: the screen's scale, so a theme drawn in CSS px fills a 200% screen the same way.
inline double& LengthScale() {
    static double scale = 1;
    return scale;
}

// A length in px (unitless and px alike); false for anything else, "auto" among them.
inline bool ParseLength(std::wstring const& s, double* out) {
    std::wstring unit;
    double v;
    if (!ParseNumber(s, &v, &unit)) return false;
    if (unit.empty() || unit == L"px" || unit == L"pt") {
        *out = (unit == L"pt" ? v * 96 / 72 : v) * LengthScale();
        return true;
    }
    return false;
}

inline double HueToRgb(double p, double q, double t) {
    if (t < 0) t += 1;
    if (t > 1) t -= 1;
    if (t < 1.0 / 6) return p + (q - p) * 6 * t;
    if (t < 0.5) return q;
    if (t < 2.0 / 3) return p + (q - p) * (2.0 / 3 - t) * 6;
    return p;
}

inline bool ParseColor(std::wstring str, Rgba* out) {
    str = Lower(Trim(str));
    if (str.empty()) return false;
    if (str == L"transparent") {
        *out = {0, 0, 0, 0};
        return true;
    }
    if (str[0] == L'#') {
        std::wstring h = str.substr(1);
        auto hex = [&](size_t at, size_t n) { return (int)wcstol(h.substr(at, n).c_str(), nullptr, 16); };
        for (wchar_t c : h)
            if (!iswxdigit(c)) return false;
        if (h.size() == 3 || h.size() == 4) {
            *out = {hex(0, 1) * 17, hex(1, 1) * 17, hex(2, 1) * 17, h.size() == 4 ? hex(3, 1) * 17 / 255.0 : 1};
            return true;
        }
        if (h.size() == 6 || h.size() == 8) {
            *out = {hex(0, 2), hex(2, 2), hex(4, 2), h.size() == 8 ? hex(6, 2) / 255.0 : 1};
            return true;
        }
        return false;
    }
    size_t open = str.find(L'(');
    if (open != std::wstring::npos && str.back() == L')') {
        std::wstring fn = str.substr(0, open);
        std::wstring inner = str.substr(open + 1, str.size() - open - 2);
        for (auto& c : inner)
            if (c == L'/' || c == L',') c = L' ';
        auto args = Words(inner);
        auto comp = [&](std::wstring const& a, double scale) {
            double v = 0;
            std::wstring unit;
            ParseNumber(a, &v, &unit);
            return unit == L"%" ? v * scale / 100 : v;
        };
        if ((fn == L"rgb" || fn == L"rgba") && args.size() >= 3) {
            Rgba c;
            c.r = (int)std::clamp(std::lround(comp(args[0], 255)), 0L, 255L);
            c.g = (int)std::clamp(std::lround(comp(args[1], 255)), 0L, 255L);
            c.b = (int)std::clamp(std::lround(comp(args[2], 255)), 0L, 255L);
            c.a = args.size() >= 4 ? std::clamp(comp(args[3], 1), 0.0, 1.0) : 1;
            *out = c;
            return true;
        }
        if ((fn == L"hsl" || fn == L"hsla") && args.size() >= 3) {
            double h = comp(args[0], 360) / 360.0, s = comp(args[1], 1) / 1, l = comp(args[2], 1) / 1;
            h -= std::floor(h);
            // "50%" is 0.5 above; a bare 50 was meant as a percentage too.
            if (s > 1) s /= 100;
            if (l > 1) l /= 100;
            double q = l < 0.5 ? l * (1 + s) : l + s - l * s, p = 2 * l - q;
            Rgba c;
            c.r = (int)std::lround(255 * HueToRgb(p, q, h + 1.0 / 3));
            c.g = (int)std::lround(255 * HueToRgb(p, q, h));
            c.b = (int)std::lround(255 * HueToRgb(p, q, h - 1.0 / 3));
            c.a = args.size() >= 4 ? std::clamp(comp(args[3], 1), 0.0, 1.0) : 1;
            *out = c;
            return true;
        }
        return false;
    }
    static const std::map<std::wstring, Rgba> names = {
        {L"white", {255, 255, 255, 1}}, {L"black", {0, 0, 0, 1}},       {L"red", {255, 0, 0, 1}},
        {L"green", {0, 128, 0, 1}},     {L"blue", {0, 0, 255, 1}},      {L"yellow", {255, 255, 0, 1}},
        {L"gray", {128, 128, 128, 1}},  {L"grey", {128, 128, 128, 1}},  {L"orange", {255, 165, 0, 1}},
        {L"purple", {128, 0, 128, 1}},  {L"pink", {255, 192, 203, 1}},  {L"cyan", {0, 255, 255, 1}},
        {L"silver", {192, 192, 192, 1}}, {L"lightgray", {211, 211, 211, 1}}, {L"darkgray", {169, 169, 169, 1}},
    };
    auto it = names.find(str);
    if (it == names.end()) return false;
    *out = it->second;
    return true;
}

// linear-gradient(<angle>|to <side>, <stop>...) and the Qt qlineargradient form's plain stops; radial ones are read as their stops.
inline bool ParseGradient(std::wstring const& value, Gradient* out) {
    std::wstring v = Trim(value);
    bool linear = StartsWith(v, L"linear-gradient(");
    bool radial = StartsWith(v, L"radial-gradient(");
    if ((!linear && !radial) || v.back() != L')') return false;
    std::wstring inner = v.substr(v.find(L'(') + 1, v.size() - v.find(L'(') - 2);
    auto parts = SplitTop(inner, L',');
    Gradient g;
    g.radial = radial;
    size_t first = 0;
    std::wstring head = Lower(Trim(parts[0]));
    double angle;
    std::wstring unit;
    if (ParseNumber(head, &angle, &unit) && (unit == L"deg" || unit == L"turn")) {
        g.angle = unit == L"turn" ? angle * 360 : angle;
        first = 1;
    } else if (StartsWith(head, L"to ")) {
        if (head.find(L"right") != std::wstring::npos) g.angle = 90;
        else if (head.find(L"left") != std::wstring::npos) g.angle = 270;
        else if (head.find(L"top") != std::wstring::npos) g.angle = 0;
        else g.angle = 180;
        first = 1;
    }
    for (size_t i = first; i < parts.size(); i++) {
        auto w = Words(Trim(parts[i]));
        if (w.empty()) continue;
        GradientStop s;
        if (!ParseColor(w[0], &s.color)) return false;
        s.at = -1;
        if (w.size() > 1) {
            double p;
            std::wstring u;
            if (ParseNumber(w[1], &p, &u) && u == L"%") s.at = p / 100;
        }
        g.stops.push_back(s);
    }
    if (g.stops.size() < 2) return false;
    // Unplaced stops spread evenly between their neighbours.
    if (g.stops.front().at < 0) g.stops.front().at = 0;
    if (g.stops.back().at < 0) g.stops.back().at = 1;
    for (size_t i = 1; i + 1 < g.stops.size(); i++) {
        if (g.stops[i].at >= 0) continue;
        size_t j = i;
        while (g.stops[j].at < 0) j++;
        double a = g.stops[i - 1].at, b = g.stops[j].at;
        for (size_t k = i; k < j; k++) g.stops[k].at = a + (b - a) * (k - i + 1) / (double)(j - i + 1);
    }
    *out = g;
    return true;
}

// ---------------------------------------------------------------------------------------------------- parsing

inline std::wstring StripComments(std::wstring const& s) {
    std::wstring out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] == L'/' && i + 1 < s.size() && s[i + 1] == L'*') {
            size_t end = s.find(L"*/", i + 2);
            if (end == std::wstring::npos) break;
            i = end + 1;
        } else {
            out += s[i];
        }
    }
    return out;
}

inline std::vector<Decl> ParseDecls(std::wstring const& body) {
    std::vector<Decl> out;
    for (auto const& item : SplitTop(body, L';')) {
        size_t colon = item.find(L':');
        if (colon == std::wstring::npos) continue;
        Decl d;
        d.name = Trim(item.substr(0, colon));
        if (d.name.rfind(L"--", 0) != 0) d.name = Lower(d.name);
        d.value = Trim(item.substr(colon + 1));
        size_t bang = d.value.find(L'!');
        if (bang != std::wstring::npos) {
            if (Lower(Trim(d.value.substr(bang + 1))) == L"important") d.important = true;
            d.value = Trim(d.value.substr(0, bang));
        }
        if (!d.name.empty()) out.push_back(std::move(d));
    }
    return out;
}

// One selector; false with a reason when it uses something we don't do.
inline bool ParseSelector(std::wstring text, Selector* out, std::wstring* why) {
    text = Trim(text);
    Selector sel;
    Compound cur;
    bool have = false, nextChild = false;
    size_t i = 0;
    auto flush = [&] {
        if (!have) return;
        cur.child = nextChild;
        nextChild = false;
        sel.specificity += (int)(cur.classes.size() + cur.pseudos.size()) * 10 + (cur.tag.empty() || cur.tag == L"*" ? 0 : 1);
        sel.parts.push_back(cur);
        cur = Compound();
        have = false;
    };
    auto ident = [&] {
        size_t start = i;
        while (i < text.size() && (iswalnum(text[i]) || text[i] == L'-' || text[i] == L'_')) i++;
        return text.substr(start, i - start);
    };
    while (i < text.size()) {
        wchar_t c = text[i];
        if (iswspace(c)) {
            flush();
            i++;
        } else if (c == L'>') {
            flush();
            nextChild = true;
            i++;
        } else if (c == L'*') {
            cur.tag = L"*";
            have = true;
            i++;
        } else if (c == L'.') {
            i++;
            cur.classes.push_back(ident());
            have = true;
        } else if (c == L':') {
            i++;
            if (i < text.size() && text[i] == L':') {
                // A Qt sub-control (::item, ::handle...): the popups build nodes wearing it as a class, "pe-item".
                i++;
                cur.classes.push_back(L"pe-" + Lower(ident()));
                have = true;
                continue;
            }
            std::wstring name = Lower(ident());
            static const std::set<std::wstring> known = {L"hover", L"active", L"pressed", L"checked", L"disabled", L"focus", L"selected",
                                                          L"root", L"last-child", L"first-child", L"clicked", L"unchecked", L"hover"};
            if (!known.count(name)) {
                *why = L":" + name;
                return false;
            }
            cur.pseudos.push_back(name);
            have = true;
        } else if (iswalpha(c) || c == L'_') {
            cur.tag = ident();
            have = true;
        } else if (c == L'[' || c == L'#' || c == L'+' || c == L'~') {
            *why = std::wstring(1, c);
            return false;
        } else {
            *why = std::wstring(1, c);
            return false;
        }
    }
    flush();
    if (sel.parts.empty()) return false;
    *out = sel;
    return true;
}

inline void ParseSheetInto(Sheet& sheet, std::wstring const& source) {
    std::wstring text = StripComments(source);
    size_t i = 0;
    while (i < text.size()) {
        while (i < text.size() && iswspace(text[i])) i++;
        if (i >= text.size()) break;
        if (text[i] == L'@') {  // @import ...; or @media {...}: not acted on
            size_t semi = text.find(L';', i), brace = text.find(L'{', i);
            sheet.report.skippedRules++;
            if (brace != std::wstring::npos && (semi == std::wstring::npos || brace < semi)) {
                int depth = 0;
                size_t j = brace;
                for (; j < text.size(); j++) {
                    if (text[j] == L'{') depth++;
                    if (text[j] == L'}' && --depth == 0) break;
                }
                i = j + 1;
            } else {
                i = semi == std::wstring::npos ? text.size() : semi + 1;
            }
            continue;
        }
        size_t open = text.find(L'{', i);
        if (open == std::wstring::npos) break;
        // A block may nest ({ ... { } ... }); take up to the matching brace.
        int depth = 0;
        size_t close = open;
        for (; close < text.size(); close++) {
            if (text[close] == L'{') depth++;
            if (text[close] == L'}' && --depth == 0) break;
        }
        std::wstring selectorText = Trim(text.substr(i, open - i));
        std::wstring body = text.substr(open + 1, close > open ? close - open - 1 : 0);
        i = close + 1;
        sheet.report.rules++;
        Rule rule;
        rule.order = (int)sheet.rules.size();
        rule.decls = ParseDecls(body);
        bool root = false;
        int kept = 0;
        for (auto const& s : SplitTop(selectorText, L',')) {
            std::wstring t = Trim(s);
            if (t == L":root" || t == L"*") root = true;
            Selector sel;
            std::wstring why;
            if (ParseSelector(t, &sel, &why)) {
                rule.selectors.push_back(sel);
                kept++;
            } else {
                sheet.report.skippedSelectors[why]++;
            }
        }
        for (auto const& d : rule.decls) {
            sheet.report.decls++;
            if (root && d.name.rfind(L"--", 0) == 0) sheet.vars[d.name] = d.value;
        }
        if (!kept) {
            sheet.report.skippedRules++;
            continue;
        }
        sheet.rules.push_back(std::move(rule));
    }
}

inline Sheet ParseSheet(std::wstring const& source) {
    Sheet sheet;
    ParseSheetInto(sheet, source);
    return sheet;
}

// ---------------------------------------------------------------------------------------------------- var()

inline std::wstring ResolveVars(std::wstring value, std::map<std::wstring, std::wstring> const& vars, int depth = 0) {
    if (depth > 8) return value;
    for (int guard = 0; guard < 64; guard++) {
        size_t at = value.find(L"var(");
        if (at == std::wstring::npos) break;
        int level = 0;
        size_t end = at + 3;
        for (; end < value.size(); end++) {
            if (value[end] == L'(') level++;
            if (value[end] == L')' && --level == 0) break;
        }
        if (end >= value.size()) break;
        std::wstring inner = value.substr(at + 4, end - at - 4);
        size_t comma = inner.find(L',');
        std::wstring name = Trim(comma == std::wstring::npos ? inner : inner.substr(0, comma));
        std::wstring fallback = comma == std::wstring::npos ? L"" : Trim(inner.substr(comma + 1));
        auto it = vars.find(name);
        std::wstring replacement = it != vars.end() ? it->second : fallback;
        value = value.substr(0, at) + ResolveVars(replacement, vars, depth + 1) + value.substr(end + 1);
    }
    return value;
}

// ---------------------------------------------------------------------------------------------------- cascade

inline bool CompoundMatches(Compound const& c, Node const& n) {
    if (!c.tag.empty() && c.tag != L"*" && c.tag != n.tag) return false;
    for (auto const& cls : c.classes)
        if (!n.Has(cls)) return false;
    for (auto const& p : c.pseudos) {
        if (p == L"root") {
            if (n.parent) return false;
        } else if (p == L"last-child") {
            if (!n.parent || n.parent->kids.back().get() != &n) return false;
        } else if (p == L"first-child") {
            if (!n.parent || n.parent->kids.front().get() != &n) return false;
        } else if (!n.states.count(p)) {
            return false;
        }
    }
    return true;
}

inline bool SelectorMatches(Selector const& sel, size_t at, Node const& n) {
    // parts[at] must match n; the ones before it match n's ancestors.
    if (!CompoundMatches(sel.parts[at], n)) return false;
    if (at == 0) return true;
    bool child = sel.parts[at].child;
    for (Node const* up = n.parent; up; up = up->parent) {
        if (SelectorMatches(sel, at - 1, *up)) return true;
        if (child) break;
    }
    return false;
}

inline void ApplyBoxShorthand(Edges* e, std::wstring const& value, bool lengths = true) {
    auto w = Words(value);
    double v[4] = {0, 0, 0, 0};
    size_t n = std::min<size_t>(w.size(), 4);
    for (size_t i = 0; i < n; i++) ParseLength(w[i], &v[i]);
    if (n == 1) v[1] = v[2] = v[3] = v[0];
    else if (n == 2) { v[2] = v[0]; v[3] = v[1]; }
    else if (n == 3) v[3] = v[1];
    (void)lengths;
    e->top = v[0]; e->right = v[1]; e->bottom = v[2]; e->left = v[3];
}

// One declaration applied to a style; the properties we don't act on are counted.
inline void ApplyDecl(Style& st, std::wstring const& name, std::wstring value, Report* report) {
    value = Trim(value);
    double d;
    Rgba c;
    auto side = [&](std::wstring const& suffix) -> int {
        if (suffix == L"top") return 0;
        if (suffix == L"right") return 1;
        if (suffix == L"bottom") return 2;
        return 3;
    };
    if (name == L"color") {
        if (ParseColor(value, &c)) st.color = c;
    } else if (name == L"background-color") {
        if (ParseColor(value, &c)) { st.background = c; st.hasGradient = false; }
    } else if (name == L"background" || name == L"background-image") {
        Gradient g;
        if (ParseGradient(value, &g)) { st.gradient = g; st.hasGradient = true; }
        else if (ParseColor(value, &c)) { st.background = c; st.hasGradient = false; }
        else if (value == L"none") { st.background = {}; st.hasGradient = false; }
        else if (report) report->unknownProps[name + L" (value)"]++;
    } else if (name == L"font-size") {
        if (ParseLength(value, &d)) st.fontSize = d;
    } else if (name == L"font-weight") {
        double w;
        std::wstring lv = Lower(value);
        if (lv == L"bold") st.fontWeight = 700;
        else if (lv == L"normal") st.fontWeight = 400;
        else if (ParseNumber(value, &w)) st.fontWeight = (int)w;
    } else if (name == L"font-family") {
        st.fontFamily.clear();
        for (auto const& f : SplitTop(value, L',')) {
            std::wstring t = Trim(f);
            if (t.size() >= 2 && (t.front() == L'"' || t.front() == L'\'')) t = t.substr(1, t.size() - 2);
            if (!t.empty()) st.fontFamily.push_back(t);
        }
    } else if (name == L"padding") {
        ApplyBoxShorthand(&st.padding, value);
    } else if (name == L"margin") {
        ApplyBoxShorthand(&st.margin, value);
    } else if (name.rfind(L"padding-", 0) == 0 && ParseLength(value, &d)) {
        (&st.padding.top)[side(name.substr(8))] = d;
    } else if (name.rfind(L"margin-", 0) == 0 && ParseLength(value, &d)) {
        (&st.margin.top)[side(name.substr(7))] = d;
    } else if (name == L"border" || name == L"border-top" || name == L"border-right" || name == L"border-bottom" || name == L"border-left") {
        double width = 0;
        Rgba col{0, 0, 0, 1};
        bool none = false;
        for (auto const& w : Words(value)) {
            if (Lower(w) == L"none" || Lower(w) == L"hidden") none = true;
            else if (ParseLength(w, &d)) width = d;
            else if (ParseColor(w, &c)) col = c;
        }
        if (none) width = 0;
        for (int i = 0; i < 4; i++) {
            if (name != L"border" && i != side(name.substr(7))) continue;
            (&st.borderWidth.top)[i] = width;
            st.borderColor[i] = col;
        }
    } else if (name == L"border-color") {
        auto w = Words(value);
        if (!w.empty()) {
            Rgba v[4];
            size_t n = std::min<size_t>(w.size(), 4);
            for (size_t i = 0; i < n; i++) ParseColor(w[i], &v[i]);
            if (n == 1) v[1] = v[2] = v[3] = v[0];
            else if (n == 2) { v[2] = v[0]; v[3] = v[1]; }
            else if (n == 3) v[3] = v[1];
            for (int i = 0; i < 4; i++) st.borderColor[i] = v[i];
        }
    } else if (name.rfind(L"border-", 0) == 0 && name.size() > 13 && name.compare(name.size() - 6, 6, L"-color") == 0) {
        if (ParseColor(value, &c)) st.borderColor[side(name.substr(7, name.size() - 13))] = c;
    } else if (name == L"border-width") {
        ApplyBoxShorthand(&st.borderWidth, value);
    } else if (name == L"border-style") {
        if (Lower(value) == L"none") st.borderWidth = {};
    } else if (name == L"border-radius") {
        auto w = Words(value);
        double v[4] = {0, 0, 0, 0};
        size_t n = std::min<size_t>(w.size(), 4);
        for (size_t i = 0; i < n; i++) ParseLength(w[i], &v[i]);
        if (n == 1) v[1] = v[2] = v[3] = v[0];
        else if (n == 2) { v[2] = v[0]; v[3] = v[1]; }
        else if (n == 3) v[3] = v[1];
        for (int i = 0; i < 4; i++) st.radius[i] = v[i];
    } else if (name == L"border-top-left-radius" && ParseLength(value, &d)) st.radius[0] = d;
    else if (name == L"border-top-right-radius" && ParseLength(value, &d)) st.radius[1] = d;
    else if (name == L"border-bottom-right-radius" && ParseLength(value, &d)) st.radius[2] = d;
    else if (name == L"border-bottom-left-radius" && ParseLength(value, &d)) st.radius[3] = d;
    else if (name == L"opacity") {
        if (ParseNumber(value, &d)) st.opacity = std::clamp(d, 0.0, 1.0);
    } else if (name == L"width") { if (ParseLength(value, &d)) st.width = d; }
    else if (name == L"height") { if (ParseLength(value, &d)) st.height = d; }
    else if (name == L"min-width") { if (ParseLength(value, &d)) st.minWidth = d; }
    else if (name == L"max-width") { if (ParseLength(value, &d)) st.maxWidth = d; }
    else if (name == L"min-height") { if (ParseLength(value, &d)) st.minHeight = d; }
    else if (name == L"max-height") { if (ParseLength(value, &d)) st.maxHeight = d; }
    else if (name == L"transition") {
        st.transitions.clear();
        for (auto const& part : SplitTop(value, L',')) {
            auto w = Words(Trim(part));
            if (w.empty()) continue;
            Transition t;
            t.property = Lower(w[0]);
            int times = 0;
            for (size_t i = 1; i < w.size(); i++) {
                std::wstring unit;
                double v;
                if (ParseNumber(w[i], &v, &unit) && (unit == L"s" || unit == L"ms")) {
                    (times++ == 0 ? t.seconds : t.delay) = unit == L"ms" ? v / 1000 : v;
                } else {
                    t.easing = Lower(w[i]);
                }
            }
            st.transitions.push_back(t);
        }
    } else if (name == L"box-shadow") {
        st.shadows.clear();
        for (auto const& part : SplitTop(value, L',')) {
            Shadow sh;
            int lengths = 0;
            bool inset = false;
            for (auto const& w : Words(Trim(part))) {
                if (Lower(w) == L"inset") inset = true;
                else if (ParseLength(w, &d)) {
                    (lengths == 0 ? sh.dx : lengths == 1 ? sh.dy : lengths == 2 ? sh.blur : sh.spread) = d;
                    lengths++;
                } else if (ParseColor(w, &c)) sh.color = c;
            }
            if (!inset && lengths >= 2) st.shadows.push_back(sh);
        }
    } else if (name == L"cursor" || name == L"outline" || name == L"outline-offset" || name.rfind(L"--", 0) == 0) {
        // no effect on a bar
    } else if (report) {
        report->unknownProps[name]++;
    }
}

inline void IndexSheet(Sheet& sheet) {
    sheet.byClass.clear();
    sheet.always.clear();
    for (size_t i = 0; i < sheet.rules.size(); i++) {
        bool any = false;
        std::set<std::wstring> keys;
        for (auto const& sel : sheet.rules[i].selectors) {
            auto const& subject = sel.parts.back();
            if (subject.classes.empty()) any = true;
            else keys.insert(subject.classes[0]);
        }
        if (any) sheet.always.push_back((int)i);
        else
            for (auto const& k : keys) sheet.byClass[k].push_back((int)i);
    }
    sheet.indexed = true;
}

// Every node's style, from the sheet (and its parent's, for what inherits).
inline void ComputeStyle(Node& n, Sheet& sheet) {
    if (!sheet.indexed) IndexSheet(sheet);
    Style st;
    if (n.parent) {
        st.color = n.parent->style.color;
        st.fontSize = n.parent->style.fontSize;
        st.fontWeight = n.parent->style.fontWeight;
        st.fontFamily = n.parent->style.fontFamily;
    }
    struct Hit {
        int weight, order;
        Decl const* decl;
    };
    std::vector<Hit> hits;
    // The rules that could apply: those with no class to go by, and those for each of this node's classes.
    std::vector<int> candidates = sheet.always;
    for (auto const& cls : n.classes) {
        auto it = sheet.byClass.find(cls);
        if (it != sheet.byClass.end()) candidates.insert(candidates.end(), it->second.begin(), it->second.end());
    }
    std::sort(candidates.begin(), candidates.end());
    candidates.erase(std::unique(candidates.begin(), candidates.end()), candidates.end());
    for (int ri : candidates) {
        Rule const& rule = sheet.rules[ri];
        int best = -1;
        for (auto const& sel : rule.selectors)
            if (SelectorMatches(sel, sel.parts.size() - 1, n)) best = std::max(best, sel.specificity);
        if (best < 0) continue;
        for (auto const& d : rule.decls) hits.push_back({best + (d.important ? 100000 : 0), rule.order, &d});
    }
    std::stable_sort(hits.begin(), hits.end(), [](Hit const& a, Hit const& b) {
        return a.weight != b.weight ? a.weight < b.weight : a.order < b.order;
    });
    for (auto const& h : hits) ApplyDecl(st, h.decl->name, ResolveVars(h.decl->value, sheet.vars), &sheet.report);
    n.style = std::move(st);
    for (auto& k : n.kids) ComputeStyle(*k, sheet);
}

}  // namespace css
