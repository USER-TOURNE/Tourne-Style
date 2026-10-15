// A small YAML reader for YASB's config.yaml: block maps and lists, flow lists and maps (also across lines), quoted strings
// with \n \" \\ \uXXXX \UXXXXXXXX escapes, comments. No anchors, tags or multi-document files.
#include <cstdint>
#include <cwctype>
#include <string>
#include <utility>
#include <vector>

namespace yaml {

struct Node {
    enum Kind { Null, Scalar, Map, Seq } kind = Null;
    std::wstring s;
    std::vector<std::pair<std::wstring, Node>> map;
    std::vector<Node> seq;

    Node const* Get(std::wstring const& key) const {
        for (auto const& [k, v] : map)
            if (k == key) return &v;
        return nullptr;
    }
    std::wstring Str(std::wstring const& key, std::wstring const& fallback = L"") const {
        Node const* n = Get(key);
        return n && n->kind == Scalar ? n->s : fallback;
    }
    double Num(std::wstring const& key, double fallback = 0) const {
        Node const* n = Get(key);
        return n && n->kind == Scalar ? wcstod(n->s.c_str(), nullptr) : fallback;
    }
    bool Bool(std::wstring const& key, bool fallback = false) const {
        Node const* n = Get(key);
        if (!n || n->kind != Scalar) return fallback;
        std::wstring v = n->s;
        for (auto& c : v) c = (wchar_t)towlower(c);
        return v == L"true" || v == L"yes" || v == L"on" || v == L"1";
    }
};

struct Line {
    int indent = 0;
    std::wstring text;
};

inline void AppendCodePoint(std::wstring& out, uint32_t cp) {
    if (cp >= 0x10000) {
        cp -= 0x10000;
        out += (wchar_t)(0xD800 + (cp >> 10));
        out += (wchar_t)(0xDC00 + (cp & 0x3FF));
    } else {
        out += (wchar_t)cp;
    }
}

// Removes a comment (a # outside quotes, at the start or after a space) and trailing space.
inline std::wstring StripComment(std::wstring const& s) {
    wchar_t quote = 0;
    for (size_t i = 0; i < s.size(); i++) {
        wchar_t c = s[i];
        if (quote) {
            if (c == L'\\' && quote == L'"') i++;
            else if (c == quote) quote = 0;
            continue;
        }
        if (c == L'"' || c == L'\'') quote = c;
        else if (c == L'#' && (i == 0 || iswspace(s[i - 1]))) return s.substr(0, i);
    }
    return s;
}

inline std::wstring RTrim(std::wstring s) {
    while (!s.empty() && iswspace(s.back())) s.pop_back();
    return s;
}

inline std::wstring LTrim(std::wstring s) {
    size_t a = 0;
    while (a < s.size() && iswspace(s[a])) a++;
    return s.substr(a);
}

// Bracket balance of a flow collection so far (quotes respected).
inline int FlowDepth(std::wstring const& s) {
    int depth = 0;
    wchar_t quote = 0;
    for (size_t i = 0; i < s.size(); i++) {
        wchar_t c = s[i];
        if (quote) {
            if (c == L'\\' && quote == L'"') i++;
            else if (c == quote) quote = 0;
            continue;
        }
        if (c == L'"' || c == L'\'') quote = c;
        else if (c == L'[' || c == L'{') depth++;
        else if (c == L']' || c == L'}') depth--;
    }
    return depth;
}

struct Flow {
    std::wstring const& t;
    size_t i = 0;
    explicit Flow(std::wstring const& text) : t(text) {}

    void Skip() {
        while (i < t.size() && iswspace(t[i])) i++;
    }

    std::wstring Quoted() {
        wchar_t q = t[i++];
        std::wstring out;
        while (i < t.size() && t[i] != q) {
            if (q == L'"' && t[i] == L'\\' && i + 1 < t.size()) {
                wchar_t e = t[++i];
                if (e == L'n') out += L'\n';
                else if (e == L't') out += L'\t';
                else if (e == L'u' || e == L'U') {
                    size_t digits = e == L'u' ? 4 : 8;
                    uint32_t cp = (uint32_t)wcstoul(t.substr(i + 1, digits).c_str(), nullptr, 16);
                    i += digits;
                    // A 󰐽 pair is two units of one character: each is kept as it is.
                    AppendCodePoint(out, cp);
                } else out += e;
                i++;
            } else if (q == L'\'' && t[i] == L'\'' && i + 1 < t.size() && t[i + 1] == L'\'') {
                out += L'\'';
                i += 2;
            } else {
                out += t[i++];
            }
        }
        if (i < t.size()) i++;
        return out;
    }

    Node Value() {
        Skip();
        Node n;
        if (i >= t.size()) return n;
        if (t[i] == L'[') {
            n.kind = Node::Seq;
            i++;
            for (;;) {
                Skip();
                if (i >= t.size() || t[i] == L']') { i++; break; }
                n.seq.push_back(Value());
                Skip();
                if (i < t.size() && t[i] == L',') i++;
            }
        } else if (t[i] == L'{') {
            n.kind = Node::Map;
            i++;
            for (;;) {
                Skip();
                if (i >= t.size() || t[i] == L'}') { i++; break; }
                std::wstring key;
                if (t[i] == L'"' || t[i] == L'\'') key = Quoted();
                else {
                    size_t start = i;
                    while (i < t.size() && t[i] != L':' && t[i] != L',' && t[i] != L'}') i++;
                    key = RTrim(t.substr(start, i - start));
                }
                Skip();
                Node v;
                if (i < t.size() && t[i] == L':') { i++; v = Value(); }
                n.map.push_back({key, v});
                Skip();
                if (i < t.size() && t[i] == L',') i++;
            }
        } else if (t[i] == L'"' || t[i] == L'\'') {
            n.kind = Node::Scalar;
            n.s = Quoted();
        } else {
            n.kind = Node::Scalar;
            size_t start = i;
            while (i < t.size() && t[i] != L',' && t[i] != L']' && t[i] != L'}') i++;
            n.s = RTrim(t.substr(start, i - start));
            if (n.s == L"~" || n.s == L"null") n.kind = Node::Null;
        }
        return n;
    }
};

inline Node ParseInline(std::wstring const& text) {
    Flow f(text);
    return f.Value();
}

struct Parser {
    std::vector<Line> lines;
    size_t at = 0;

    // Splits "key: rest" at the first colon outside quotes that is followed by space or the end.
    static bool SplitKey(std::wstring const& text, std::wstring* key, std::wstring* rest) {
        wchar_t quote = 0;
        for (size_t i = 0; i < text.size(); i++) {
            wchar_t c = text[i];
            if (quote) {
                if (c == L'\\' && quote == L'"') i++;
                else if (c == quote) quote = 0;
                continue;
            }
            if (c == L'"' || c == L'\'') { if (i == 0) quote = c; }
            else if (c == L'[' || c == L'{') return false;
            else if (c == L':' && (i + 1 == text.size() || iswspace(text[i + 1]))) {
                *key = RTrim(text.substr(0, i));
                if (key->size() >= 2 && ((key->front() == L'"' && key->back() == L'"') || (key->front() == L'\'' && key->back() == L'\'')))
                    *key = key->substr(1, key->size() - 2);
                *rest = LTrim(text.substr(i + 1));
                return true;
            }
        }
        return false;
    }

    Node Block(int indent) {
        Node n;
        if (at >= lines.size()) return n;
        if (lines[at].text.rfind(L"- ", 0) == 0 || lines[at].text == L"-") {
            n.kind = Node::Seq;
            while (at < lines.size() && lines[at].indent == indent &&
                   (lines[at].text.rfind(L"- ", 0) == 0 || lines[at].text == L"-")) {
                std::wstring rest = LTrim(lines[at].text.substr(1));
                int childIndent = indent + 1 + (int)(lines[at].text.size() - 1 - rest.size());  // where the item's own keys line up
                if (rest.empty()) {
                    at++;
                    n.seq.push_back(at < lines.size() && lines[at].indent > indent ? Block(lines[at].indent) : Node());
                } else {
                    std::wstring key, value;
                    if (SplitKey(rest, &key, &value)) {
                        // "- key: value" starts a map whose other keys line up with this one.
                        lines[at].indent = childIndent;
                        lines[at].text = rest;
                        n.seq.push_back(Block(childIndent));
                    } else {
                        n.seq.push_back(ParseInline(rest));
                        at++;
                    }
                }
            }
            return n;
        }
        n.kind = Node::Map;
        while (at < lines.size() && lines[at].indent == indent) {
            std::wstring key, rest;
            if (!SplitKey(lines[at].text, &key, &rest)) { at++; continue; }
            at++;
            Node v;
            if (rest.empty()) {
                if (at < lines.size() && (lines[at].indent > indent ||
                                          (lines[at].indent == indent && lines[at].text.rfind(L"- ", 0) == 0)))
                    v = Block(lines[at].indent);
            } else {
                v = ParseInline(rest);
            }
            n.map.push_back({key, v});
        }
        return n;
    }
};

inline Node Parse(std::wstring const& source) {
    Parser p;
    std::vector<std::wstring> raw;
    size_t start = 0;
    for (size_t i = 0; i <= source.size(); i++) {
        if (i == source.size() || source[i] == L'\n') {
            std::wstring l = source.substr(start, i - start);
            if (!l.empty() && l.back() == L'\r') l.pop_back();
            raw.push_back(l);
            start = i + 1;
        }
    }
    for (size_t i = 0; i < raw.size(); i++) {
        std::wstring l = RTrim(StripComment(raw[i]));
        size_t ind = 0;
        while (ind < l.size() && l[ind] == L' ') ind++;
        std::wstring text = l.substr(ind);
        if (text.empty() || text == L"---") continue;
        // "key:" followed by a flow collection on the next line: join it.
        std::wstring key, rest;
        if (Parser::SplitKey(text, &key, &rest) && rest.empty()) {
            size_t j = i + 1;
            while (j < raw.size() && RTrim(StripComment(raw[j])).find_first_not_of(L' ') == std::wstring::npos) j++;
            if (j < raw.size()) {
                std::wstring nxt = LTrim(RTrim(StripComment(raw[j])));
                if (!nxt.empty() && (nxt[0] == L'[' || nxt[0] == L'{')) {
                    text += L" " + nxt;
                    i = j;
                }
            }
        }
        // A flow collection left open at the end of a line goes on into the next ones.
        while (FlowDepth(text) > 0 && i + 1 < raw.size()) {
            text += L" " + LTrim(RTrim(StripComment(raw[++i])));
        }
        p.lines.push_back({(int)ind, text});
    }
    return p.Block(p.lines.empty() ? 0 : p.lines[0].indent);
}

}  // namespace yaml
