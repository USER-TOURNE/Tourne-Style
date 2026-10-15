#include "toml-lite.h"

#include <cstdlib>
#include <cstring>

namespace td::toml {

namespace {

using json::Value;

// A field of an object, made (null) when missing.
Value& Field(Value& object, const std::string& name) {
    for (auto& [key, value] : object.fields)
        if (key == name) return value;
    object.fields.push_back({name, Value{}});
    return object.fields.back().second;
}

struct Reader {
    const std::string& s;
    size_t at = 0;
    std::string error;

    bool Fail(const char* why) {
        if (error.empty()) {
            int line = 1;
            for (size_t i = 0; i < at && i < s.size(); i++) line += s[i] == '\n';
            error = std::string(why) + " on line " + std::to_string(line);
        }
        return false;
    }
    bool End() const { return at >= s.size(); }
    // Spaces and tabs (and, inside arrays, newlines and comments).
    void Blank(bool lines) {
        while (!End()) {
            char c = s[at];
            if (c == ' ' || c == '\t' || c == '\r' || (lines && c == '\n')) at++;
            else if (c == '#') while (!End() && s[at] != '\n') at++;
            else break;
        }
    }
    static void Utf8(std::string& out, unsigned cp) {
        if (cp < 0x80) out += (char)cp;
        else if (cp < 0x800) out += (char)(0xC0 | cp >> 6), out += (char)(0x80 | (cp & 0x3F));
        else if (cp < 0x10000) out += (char)(0xE0 | cp >> 12), out += (char)(0x80 | (cp >> 6 & 0x3F)), out += (char)(0x80 | (cp & 0x3F));
        else
            out += (char)(0xF0 | cp >> 18), out += (char)(0x80 | (cp >> 12 & 0x3F)), out += (char)(0x80 | (cp >> 6 & 0x3F)),
                out += (char)(0x80 | (cp & 0x3F));
    }
    // "basic", 'literal', and their """ / ''' many-line forms.
    bool String(std::string* out) {
        char q = s[at];
        bool many = s.compare(at, 3, std::string(3, q)) == 0;
        at += many ? 3 : 1;
        if (many && !End() && s[at] == '\n') at++;  // a newline right after the quotes isn't part of it
        else if (many && s.compare(at, 2, "\r\n") == 0) at += 2;
        for (;;) {
            if (End()) return Fail("unclosed string");
            char c = s[at];
            if (many ? s.compare(at, 3, std::string(3, q)) == 0 : c == q) {
                at += many ? 3 : 1;
                return true;
            }
            if (!many && c == '\n') return Fail("string runs past the line");
            at++;
            if (q == '\'' || c != '\\') {
                *out += c;
                continue;
            }
            if (End()) return Fail("unclosed string");
            char e = s[at++];
            switch (e) {
                case 'n': *out += '\n'; break;
                case 't': *out += '\t'; break;
                case 'r': *out += '\r'; break;
                case 'b': *out += '\b'; break;
                case 'f': *out += '\f'; break;
                case 'u':
                case 'U': {
                    size_t n = e == 'u' ? 4 : 8;
                    if (at + n > s.size()) return Fail("short \\u escape");
                    Utf8(*out, (unsigned)strtoul(s.substr(at, n).c_str(), nullptr, 16));
                    at += n;
                    break;
                }
                case '\r':
                case '\n':  // a line ending in \ joins the next, without its leading blanks
                    while (!End() && strchr(" \t\r\n", s[at])) at++;
                    break;
                default: *out += e;
            }
        }
    }
    bool Key(std::vector<std::string>* path) {
        for (;;) {
            Blank(false);
            if (End()) return Fail("missing key");
            std::string part;
            if (s[at] == '"' || s[at] == '\'') {
                if (!String(&part)) return false;
            } else {
                while (!End() && (isalnum((unsigned char)s[at]) || s[at] == '_' || s[at] == '-')) part += s[at++];
                if (part.empty()) return Fail("bad key");
            }
            path->push_back(part);
            Blank(false);
            if (End() || s[at] != '.') return true;
            at++;
        }
    }
    bool Read(Value* v, int depth) {
        if (depth > 32) return Fail("nested too deep");
        Blank(false);
        if (End()) return Fail("missing value");
        char c = s[at];
        if (c == '"' || c == '\'') {
            v->kind = Value::String;
            return String(&v->text);
        }
        if (c == '[') {
            v->kind = Value::Array;
            at++;
            for (;;) {
                Blank(true);
                if (End()) return Fail("unclosed array");
                if (s[at] == ']') return at++, true;
                Value item;
                if (!Read(&item, depth + 1)) return false;
                v->items.push_back(std::move(item));
                Blank(true);
                if (!End() && s[at] == ',') at++;
            }
        }
        if (c == '{') {
            v->kind = Value::Object;
            at++;
            for (;;) {
                Blank(false);
                if (End()) return Fail("unclosed inline table");
                if (s[at] == '}') return at++, true;
                if (!Pair(*v, depth + 1)) return false;
                Blank(false);
                if (!End() && s[at] == ',') at++;
            }
        }
        if (s.compare(at, 4, "true") == 0) return at += 4, v->kind = Value::Bool, v->boolean = true, true;
        if (s.compare(at, 5, "false") == 0) return at += 5, v->kind = Value::Bool, v->boolean = false, true;
        // A number (dates and times are kept as their text).
        size_t start = at;
        while (!End() && !strchr(" \t\r\n,]}#", s[at])) at++;
        std::string word = s.substr(start, at - start);
        std::string digits;
        for (char d : word)
            if (d != '_') digits += d;
        char* stop = nullptr;
        double n = digits.compare(0, 2, "0x") == 0 ? (double)strtoll(digits.c_str() + 2, &stop, 16) : strtod(digits.c_str(), &stop);
        if (!digits.empty() && stop && !*stop) {
            v->kind = Value::Number;
            v->number = n;
        } else if (!word.empty()) {
            v->kind = Value::String;
            v->text = word;
        } else {
            return Fail("bad value");
        }
        return true;
    }
    // key = value, into `table`.
    bool Pair(Value& table, int depth) {
        std::vector<std::string> path;
        if (!Key(&path)) return false;
        if (End() || s[at] != '=') return Fail("expected =");
        at++;
        Value* target = &table;
        for (size_t i = 0; i + 1 < path.size(); i++) {
            target = &Field(*target, path[i]);
            if (target->kind != Value::Object) *target = Value{}, target->kind = Value::Object;
        }
        Value value;
        if (!Read(&value, depth)) return false;
        Field(*target, path.back()) = std::move(value);
        return true;
    }
};

}  // namespace

bool Parse(const std::string& text, json::Value* out, std::string* error) {
    Reader r{text};
    *out = Value{};
    out->kind = Value::Object;
    Value* table = out;
    while (true) {
        r.Blank(true);
        if (r.End()) break;
        if (text[r.at] == '[') {
            bool many = text.compare(r.at, 2, "[[") == 0;
            r.at += many ? 2 : 1;
            std::vector<std::string> path;
            if (!r.Key(&path)) break;
            if (text.compare(r.at, many ? 2 : 1, many ? "]]" : "]") != 0) {
                r.Fail("unclosed table header");
                break;
            }
            r.at += many ? 2 : 1;
            // Every part but the last: the table, or the last of an array of tables.
            table = out;
            for (size_t i = 0; i + 1 < path.size(); i++) {
                Value& next = Field(*table, path[i]);
                if (next.kind == Value::Array && !next.items.empty()) table = &next.items.back();
                else {
                    if (next.kind != Value::Object) next = Value{}, next.kind = Value::Object;
                    table = &next;
                }
            }
            Value& last = Field(*table, path.back());
            if (many) {
                if (last.kind != Value::Array) last = Value{}, last.kind = Value::Array;
                last.items.push_back(Value{});
                last.items.back().kind = Value::Object;
                table = &last.items.back();
            } else {
                if (last.kind != Value::Object) last = Value{}, last.kind = Value::Object;
                table = &last;
            }
            continue;
        }
        if (!r.Pair(*table, 0)) break;
        r.Blank(false);
        if (!r.End() && text[r.at] != '\n') {
            r.Fail("more after a value");
            break;
        }
    }
    if (error) *error = r.error;
    return r.error.empty();
}

}  // namespace td::toml
