#include "json-lite.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace td::json {

namespace {

const Value kNull;

struct Reader {
    const std::string& s;
    size_t at = 0;

    void Space() {
        while (at < s.size() && (s[at] == ' ' || s[at] == '\t' || s[at] == '\n' || s[at] == '\r')) at++;
    }
    bool Word(const char* w) {
        size_t n = strlen(w);
        if (s.compare(at, n, w) != 0) return false;
        at += n;
        return true;
    }
    static void Utf8(std::string& out, unsigned cp) {
        if (cp < 0x80) out += (char)cp;
        else if (cp < 0x800) out += (char)(0xC0 | cp >> 6), out += (char)(0x80 | (cp & 0x3F));
        else if (cp < 0x10000) out += (char)(0xE0 | cp >> 12), out += (char)(0x80 | (cp >> 6 & 0x3F)), out += (char)(0x80 | (cp & 0x3F));
        else
            out += (char)(0xF0 | cp >> 18), out += (char)(0x80 | (cp >> 12 & 0x3F)), out += (char)(0x80 | (cp >> 6 & 0x3F)),
                out += (char)(0x80 | (cp & 0x3F));
    }
    bool String(std::string* out) {
        if (at >= s.size() || s[at] != '"') return false;
        at++;
        while (at < s.size() && s[at] != '"') {
            char c = s[at++];
            if (c != '\\') {
                *out += c;
                continue;
            }
            if (at >= s.size()) return false;
            char e = s[at++];
            switch (e) {
                case 'n': *out += '\n'; break;
                case 't': *out += '\t'; break;
                case 'r': *out += '\r'; break;
                case 'b': *out += '\b'; break;
                case 'f': *out += '\f'; break;
                case 'u': {
                    if (at + 4 > s.size()) return false;
                    unsigned cp = (unsigned)strtoul(s.substr(at, 4).c_str(), nullptr, 16);
                    at += 4;
                    if (cp >= 0xD800 && cp < 0xDC00 && at + 6 <= s.size() && s[at] == '\\' && s[at + 1] == 'u') {
                        unsigned low = (unsigned)strtoul(s.substr(at + 2, 4).c_str(), nullptr, 16);
                        at += 6;
                        cp = 0x10000 + ((cp - 0xD800) << 10) + (low - 0xDC00);
                    }
                    Utf8(*out, cp);
                    break;
                }
                default: *out += e;
            }
        }
        if (at >= s.size()) return false;
        at++;
        return true;
    }
    bool Read(Value* v, int depth) {
        if (depth > 64) return false;
        Space();
        if (at >= s.size()) return false;
        char c = s[at];
        if (c == '{') {
            v->kind = Value::Object;
            at++;
            Space();
            if (at < s.size() && s[at] == '}') return at++, true;
            for (;;) {
                Space();
                std::string name;
                if (!String(&name)) return false;
                Space();
                if (at >= s.size() || s[at++] != ':') return false;
                Value item;
                if (!Read(&item, depth + 1)) return false;
                v->fields.emplace_back(std::move(name), std::move(item));
                Space();
                if (at < s.size() && s[at] == ',') { at++; continue; }
                if (at < s.size() && s[at] == '}') return at++, true;
                return false;
            }
        }
        if (c == '[') {
            v->kind = Value::Array;
            at++;
            Space();
            if (at < s.size() && s[at] == ']') return at++, true;
            for (;;) {
                Value item;
                if (!Read(&item, depth + 1)) return false;
                v->items.push_back(std::move(item));
                Space();
                if (at < s.size() && s[at] == ',') { at++; continue; }
                if (at < s.size() && s[at] == ']') return at++, true;
                return false;
            }
        }
        if (c == '"') return v->kind = Value::String, String(&v->text);
        if (Word("true")) return v->kind = Value::Bool, v->boolean = true, true;
        if (Word("false")) return v->kind = Value::Bool, true;
        if (Word("null")) return true;
        char* end = nullptr;
        v->number = strtod(s.c_str() + at, &end);
        if (end == s.c_str() + at) return false;
        v->kind = Value::Number;
        at = end - s.c_str();
        return true;
    }
};

}  // namespace

const Value& Value::operator[](const char* name) const {
    if (kind == Object)
        for (auto const& f : fields)
            if (f.first == name) return f.second;
    return kNull;
}

const Value& Value::operator[](size_t index) const { return kind == Array && index < items.size() ? items[index] : kNull; }

bool Parse(const std::string& text, Value* out) {
    Reader r{text};
    *out = Value{};
    if (!r.Read(out, 0)) return false;
    r.Space();
    return r.at == text.size();
}

std::string Quote(const std::string& text) {
    std::string out = "\"";
    for (unsigned char c : text) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (c < 0x20) {
                    char hex[8];
                    snprintf(hex, sizeof(hex), "\\u%04x", c);
                    out += hex;
                } else {
                    out += (char)c;
                }
        }
    }
    return out + "\"";
}

}  // namespace td::json
