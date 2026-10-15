#include "config.h"

#include <cstdint>
#include <cwctype>
#include <cstdlib>

namespace td {

std::wstring Utf8ToWide(const std::string& s) {
    std::wstring out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size();) {
        unsigned char c = (unsigned char)s[i];
        uint32_t cp;
        int extra;
        if (c < 0x80) cp = c, extra = 0;
        else if ((c & 0xE0) == 0xC0) cp = c & 0x1F, extra = 1;
        else if ((c & 0xF0) == 0xE0) cp = c & 0x0F, extra = 2;
        else cp = c & 0x07, extra = 3;
        i++;
        for (int k = 0; k < extra && i < s.size(); k++, i++) cp = cp << 6 | ((unsigned char)s[i] & 0x3F);
        if (sizeof(wchar_t) == 2 && cp >= 0x10000) {
            cp -= 0x10000;
            out += (wchar_t)(0xD800 + (cp >> 10));
            out += (wchar_t)(0xDC00 + (cp & 0x3FF));
        } else {
            out += (wchar_t)cp;
        }
    }
    return out;
}

std::string WideToUtf8(const std::wstring& w) {
    std::string out;
    for (size_t i = 0; i < w.size(); i++) {
        uint32_t cp = (uint32_t)w[i];
        if (sizeof(wchar_t) == 2 && cp >= 0xD800 && cp < 0xDC00 && i + 1 < w.size())
            cp = 0x10000 + ((cp - 0xD800) << 10) + ((uint32_t)w[++i] - 0xDC00);
        if (cp < 0x80) out += (char)cp;
        else if (cp < 0x800) out += (char)(0xC0 | cp >> 6), out += (char)(0x80 | (cp & 0x3F));
        else if (cp < 0x10000)
            out += (char)(0xE0 | cp >> 12), out += (char)(0x80 | (cp >> 6 & 0x3F)), out += (char)(0x80 | (cp & 0x3F));
        else
            out += (char)(0xF0 | cp >> 18), out += (char)(0x80 | (cp >> 12 & 0x3F)), out += (char)(0x80 | (cp >> 6 & 0x3F)),
                out += (char)(0x80 | (cp & 0x3F));
    }
    return out;
}

static std::wstring Trim(const std::wstring& s) {
    size_t a = 0, b = s.size();
    while (a < b && iswspace(s[a])) a++;
    while (b > a && iswspace(s[b - 1])) b--;
    return s.substr(a, b - a);
}

// A YAML scalar as written in the schema: quotes taken off, true/false as
// 1/0, a comment after an unquoted value dropped.
static std::wstring Scalar(std::wstring v) {
    v = Trim(v);
    if (v.size() >= 2 && v[0] == L'\'' ) {
        std::wstring out;
        for (size_t i = 1; i < v.size(); i++) {
            if (v[i] == L'\'') {
                if (i + 1 < v.size() && v[i + 1] == L'\'') { out += L'\''; i++; continue; }
                break;
            }
            out += v[i];
        }
        return out;
    }
    if (v.size() >= 2 && v[0] == L'"') {
        std::wstring out;
        for (size_t i = 1; i < v.size(); i++) {
            if (v[i] == L'\\' && i + 1 < v.size()) {
                wchar_t n = v[++i];
                out += n == L'n' ? L'\n' : n == L't' ? L'\t' : n;
                continue;
            }
            if (v[i] == L'"') break;
            out += v[i];
        }
        return out;
    }
    size_t hash = v.find(L" #");
    if (hash != std::wstring::npos) v = Trim(v.substr(0, hash));
    if (v == L"true") return L"1";
    if (v == L"false") return L"0";
    return v;
}

static std::vector<std::wstring> FlowList(const std::wstring& v) {
    std::vector<std::wstring> items;
    std::wstring inner = v.substr(1, v.rfind(L']') == std::wstring::npos ? std::wstring::npos : v.rfind(L']') - 1);
    std::wstring cur;
    wchar_t quote = 0;
    for (wchar_t c : inner) {
        if (quote) {
            cur += c;
            if (c == quote) quote = 0;
        } else if (c == L'\'' || c == L'"') {
            quote = c;
            cur += c;
        } else if (c == L',') {
            if (!Trim(cur).empty()) items.push_back(Scalar(cur));
            cur.clear();
        } else {
            cur += c;
        }
    }
    if (!Trim(cur).empty()) items.push_back(Scalar(cur));
    return items;
}

struct SchemaLine {
    int indent;
    std::wstring text;  // without the indent
    // What the schema says about the entry above ($name, $description, an
    // $options item, a "#! $min"/"$max" hint): kept for Tourne Studio.
    bool meta = false;
    std::wstring metaKey, metaValue, optionValue;
};

// "key: value" -> key and value; false if it isn't one.
static bool SplitKey(const std::wstring& s, std::wstring* key, std::wstring* value) {
    size_t colon = s.find(L':');
    if (colon == std::wstring::npos || colon == 0) return false;
    std::wstring k = s.substr(0, colon);
    for (wchar_t c : k)
        if (!(iswalnum(c) || c == L'_' || c == L'$' || c == L'-')) return false;
    if (colon + 1 < s.size() && s[colon + 1] != L' ') return false;
    *key = k;
    *value = Trim(s.substr(colon + 1));
    return true;
}

void ParseSchemaDefaults(const std::wstring& prefix, const std::string& yaml, Settings& out, SchemaInfo* info) {
    // The lines that matter: no comments, and what "$name" and its kin say
    // (with the text they carry on following lines) kept apart as meta lines.
    std::vector<SchemaLine> lines;
    {
        std::wstring all = Utf8ToWide(yaml);
        size_t at = 0;
        int contIndent = -1, optionsIndent = -1;
        size_t cont = 0;  // the meta line a folded text (">-") goes on in
        while (at <= all.size()) {
            size_t nl = all.find(L'\n', at);
            std::wstring line = all.substr(at, nl == std::wstring::npos ? std::wstring::npos : nl - at);
            at = nl == std::wstring::npos ? all.size() + 1 : nl + 1;
            if (!line.empty() && line.back() == L'\r') line.pop_back();
            int indent = 0;
            while (indent < (int)line.size() && line[indent] == L' ') indent++;
            std::wstring text = Trim(line);
            if (text.empty()) continue;
            if (contIndent >= 0) {
                if (indent > contIndent) {
                    std::wstring& v = lines[cont].metaValue;
                    v += (v.empty() ? L"" : L" ") + text;
                    continue;
                }
                contIndent = -1;
            }
            if (optionsIndent >= 0) {
                if (indent >= optionsIndent && text.rfind(L"- ", 0) == 0) {
                    std::wstring k, v;
                    SchemaLine m{indent, L"", true, L"$option"};
                    if (SplitKey(text.substr(2), &k, &v)) {
                        m.optionValue = k;
                        if (v == L">-" || v == L">" || v == L"|") contIndent = indent, cont = lines.size();
                        else m.metaValue = Scalar(v);
                    } else {
                        m.optionValue = Scalar(text.substr(2));
                        m.metaValue = m.optionValue;
                    }
                    lines.push_back(m);
                    continue;
                }
                optionsIndent = -1;
            }
            if (text.rfind(L"#!", 0) == 0) {
                std::wstring k, v;
                if (SplitKey(Trim(text.substr(2)), &k, &v) && (k == L"$min" || k == L"$max"))
                    lines.push_back({indent, L"", true, k, v});
                continue;
            }
            if (text[0] == L'#') continue;
            if (text[0] == L'$') {
                std::wstring k, v;
                if (SplitKey(text, &k, &v)) {
                    SchemaLine m{indent, L"", true, k};
                    if (v.empty()) optionsIndent = indent;  // $options and its list
                    else if (v[0] == L'>' || v[0] == L'|') contIndent = indent, cont = lines.size();
                    else m.metaValue = Scalar(v);
                    lines.push_back(m);
                }
                continue;
            }
            lines.push_back({indent, text});
        }
    }

    struct Level {
        int indent;  // its entries' indent
        std::wstring path;
        bool list;
        int next = 0;
    };
    std::vector<Level> stack{{0, prefix, false}};
    std::map<int, std::wstring> entryAt;  // the latest entry at each indent ("- key"), for its meta lines
    std::wstring optionsOf;

    auto nextReal = [&](size_t i) {
        for (i++; i < lines.size() && lines[i].meta; i++) {
        }
        return i;
    };
    // What follows "key:" with nothing after it: a group, or a list.
    auto opensList = [&](size_t i) {
        size_t n = nextReal(i);
        if (n >= lines.size()) return false;
        std::wstring const& t = lines[n].text;
        if (t.rfind(L"- - ", 0) == 0) return true;
        if (t.rfind(L"- ", 0) != 0) return false;
        std::wstring k, v;
        return !SplitKey(t.substr(2), &k, &v);
    };

    for (size_t i = 0; i < lines.size(); i++) {
        int indent = lines[i].indent;
        if (lines[i].meta) {
            if (!info) continue;
            SchemaLine const& m = lines[i];
            if (m.metaKey == L"$option") {
                if (!optionsOf.empty()) (*info)[optionsOf].options.push_back({m.optionValue, m.metaValue});
                continue;
            }
            auto owner = entryAt.find(indent - 2);
            if (owner == entryAt.end()) continue;
            SettingInfo& si = (*info)[owner->second];
            if (m.metaKey == L"$name") si.name = m.metaValue;
            else if (m.metaKey == L"$description") si.description = m.metaValue;
            else if (m.metaKey == L"$options") optionsOf = owner->second;
            else if (m.metaKey == L"$min") si.hasMin = true, si.min = wcstod(m.metaValue.c_str(), nullptr);
            else if (m.metaKey == L"$max") si.hasMax = true, si.max = wcstod(m.metaValue.c_str(), nullptr);
            else if (m.metaKey == L"$format") si.format = m.metaValue;
            continue;
        }
        std::wstring text = lines[i].text;
        if (text.rfind(L"- ", 0) != 0) continue;
        while (stack.size() > 1 && stack.back().indent > indent) stack.pop_back();
        Level* top = &stack.back();
        text = text.substr(2);
        if (top->list) {
            int index = top->next++;
            std::wstring element = top->path + L"[" + std::to_wstring(index) + L"]";
            if (text.rfind(L"- ", 0) == 0) {
                // An element made of fields: its first on this line.
                stack.push_back({indent + 2, element, false});
                top = &stack.back();
                indent += 2;
                text = text.substr(2);
            } else {
                out[element] = Scalar(text);
                continue;
            }
        }
        std::wstring key, value;
        if (!SplitKey(text, &key, &value)) continue;
        std::wstring path = top->path + L"." + key;
        for (auto it = entryAt.lower_bound(indent); it != entryAt.end();) it = entryAt.erase(it);
        entryAt[indent] = path;
        if (info) {
            int& order = (*info)[path].order;
            if (!order) order = (int)info->size();
        }
        if (value.empty()) {
            size_t n = nextReal(i);
            int childIndent = n < lines.size() ? lines[n].indent : indent + 2;
            if (childIndent <= indent && !(opensList(i) && childIndent == indent)) childIndent = indent + 2;
            stack.push_back({childIndent, path, opensList(i)});
        } else if (value[0] == L'[') {
            auto items = FlowList(value);
            for (size_t k = 0; k < items.size(); k++) out[path + L"[" + std::to_wstring(k) + L"]"] = items[k];
        } else {
            out[path] = Scalar(value);
            if (info && (value == L"true" || value == L"false")) (*info)[path].isSwitch = true;
        }
    }
}

// Hyprland's animation lines, read as they are into Tourne's motion settings
// (motion.cpp): "animation = LEAF, ON, SPEED, CURVE[, STYLE]", "bezier = NAME,
// X0, Y0, X1, Y1", Tourne's "spring = NAME, MASS, STIFFNESS, DAMPING", and the
// Lua form, hl.animation({ ... }) and hl.curve(NAME, { ... }).

// The fields of a Lua table: name = value, values unquoted.
static std::vector<std::pair<std::wstring, std::wstring>> LuaFields(const std::wstring& s) {
    std::vector<std::pair<std::wstring, std::wstring>> fields;
    for (size_t i = 0; i < s.size(); i++) {
        if (!(iswalpha(s[i]) || s[i] == L'_') || (i && (iswalnum(s[i - 1]) || s[i - 1] == L'_' || s[i - 1] == L'.'))) continue;
        size_t end = i;
        while (end < s.size() && (iswalnum(s[end]) || s[end] == L'_')) end++;
        size_t eq = s.find_first_not_of(L" \t", end);
        if (eq == std::wstring::npos || s[eq] != L'=' || (eq + 1 < s.size() && s[eq + 1] == L'=')) continue;
        size_t v = s.find_first_not_of(L" \t", eq + 1);
        if (v == std::wstring::npos) break;
        std::wstring value;
        size_t after;
        if (s[v] == L'"' || s[v] == L'\'') {
            size_t close = s.find(s[v], v + 1);
            if (close == std::wstring::npos) break;
            value = s.substr(v + 1, close - v - 1);
            after = close + 1;
        } else {
            after = s.find_first_of(L",}", v);
            if (after == std::wstring::npos) after = s.size();
            value = Trim(s.substr(v, after - v));
        }
        fields.push_back({s.substr(i, end - i), value});
        i = after;
    }
    return fields;
}

static std::vector<std::wstring> NumbersIn(const std::wstring& s) {
    std::vector<std::wstring> numbers;
    for (size_t i = 0; i < s.size();) {
        if (iswdigit(s[i]) || ((s[i] == L'-' || s[i] == L'.') && i + 1 < s.size() && (iswdigit(s[i + 1]) || s[i + 1] == L'.'))) {
            size_t end = i + 1;
            while (end < s.size() && (iswdigit(s[end]) || s[end] == L'.')) end++;
            numbers.push_back(s.substr(i, end - i));
            i = end;
        } else {
            i++;
        }
    }
    return numbers;
}

static bool HyprlandLine(const std::wstring& line, const std::wstring& key, const std::wstring& value, Settings& out) {
    if (line.rfind(L"hl.animation(", 0) == 0) {
        std::wstring leaf, on = L"1", speed, curve, style;
        for (auto const& [k, v] : LuaFields(line)) {
            if (k == L"leaf") leaf = v;
            else if (k == L"enabled") on = v;
            else if (k == L"speed") speed = v;
            else if (k == L"bezier" || k == L"spring" || k == L"curve") curve = v;
            else if (k == L"style") style = v;
        }
        if (leaf.empty()) return false;
        std::wstring text = on;
        if (!speed.empty() || !curve.empty()) text += L", " + speed + L", " + curve;
        if (!style.empty()) text += L", " + style;
        out[L"motion." + leaf] = text;
        return true;
    }
    if (line.rfind(L"hl.curve(", 0) == 0) {
        size_t q = line.find_first_of(L"\"'");
        size_t close = q == std::wstring::npos ? q : line.find(line[q], q + 1);
        if (close == std::wstring::npos) return false;
        std::wstring name = line.substr(q + 1, close - q - 1), rest = line.substr(close + 1), type = L"bezier", mass = L"1", stiffness, damping;
        for (auto const& [k, v] : LuaFields(rest)) {
            if (k == L"type") type = v;
            else if (k == L"mass") mass = v;
            else if (k == L"stiffness") stiffness = v;
            else if (k == L"damping") damping = v;
        }
        if (type == L"spring") {
            if (stiffness.empty() || damping.empty()) return false;
            out[L"motion.spring." + name] = mass + L", " + stiffness + L", " + damping;
            return true;
        }
        size_t points = rest.find(L"points");
        auto n = NumbersIn(points == std::wstring::npos ? rest : rest.substr(points));
        if (n.size() < 4) return false;
        out[L"motion.bezier." + name] = n[0] + L", " + n[1] + L", " + n[2] + L", " + n[3];
        return true;
    }
    if (key != L"animation" && key != L"bezier" && key != L"spring") return false;
    size_t comma = value.find(L',');
    if (comma == std::wstring::npos) return false;
    std::wstring name = Trim(value.substr(0, comma)), rest = Trim(value.substr(comma + 1));
    if (name.empty()) return false;
    out[key == L"animation" ? L"motion." + name : L"motion." + key + L"." + name] = rest;
    return true;
}

void ParseConfig(const std::wstring& text, Settings& out, std::vector<std::wstring>* errors) {
    std::vector<std::wstring> groups;
    size_t at = 0;
    int number = 0;
    while (at <= text.size()) {
        size_t nl = text.find(L'\n', at);
        std::wstring line = text.substr(at, nl == std::wstring::npos ? std::wstring::npos : nl - at);
        at = nl == std::wstring::npos ? text.size() + 1 : nl + 1;
        number++;
        line = Trim(line);
        if (line.empty() || line[0] == L'#') continue;
        if (line == L"}") {
            if (!groups.empty()) groups.pop_back();
            else if (errors) errors->push_back(L"line " + std::to_wstring(number) + L": a } with no group open");
            continue;
        }
        if (line.back() == L'{') {
            std::wstring name = Trim(line.substr(0, line.size() - 1));
            if (name == L"animations") name = L"motion";  // (Hyprland's group)
            if (!name.empty()) groups.push_back(name);
            continue;
        }
        if (line.rfind(L"hl.", 0) == 0) {
            if (!HyprlandLine(line, L"", L"", out) && errors)
                errors->push_back(L"line " + std::to_wstring(number) + L": not an animation or curve Tourne reads");
            continue;
        }
        size_t eq = line.find(L'=');
        if (eq == std::wstring::npos) {
            if (errors) errors->push_back(L"line " + std::to_wstring(number) + L": no = in \"" + line + L"\"");
            continue;
        }
        std::wstring key = Trim(line.substr(0, eq)), value = Trim(line.substr(eq + 1));
        if (key.empty()) continue;
        // Quoted values keep everything; unquoted ones lose a trailing comment.
        if (!value.empty() && (value[0] == L'"' || value[0] == L'\'')) {
            value = Scalar(value);
        } else {
            size_t hash = value.find(L" #");
            if (hash != std::wstring::npos) value = Trim(value.substr(0, hash));
        }
        if ((groups.empty() || (groups.size() == 1 && groups[0] == L"motion")) && HyprlandLine(line, key, value, out)) continue;
        std::wstring full;
        for (auto const& g : groups) full += g + L".";
        full += key;
        out[full] = value;
    }
    if (errors && !groups.empty()) errors->push_back(L"a group was left open (a } is missing)");
}

int ToInt(const std::wstring& v) {
    std::wstring s = Trim(v);
    for (auto& c : s) c = (wchar_t)towlower(c);
    if (s == L"true" || s == L"on" || s == L"yes") return 1;
    if (s == L"false" || s == L"off" || s == L"no" || s.empty()) return 0;
    return (int)wcstol(s.c_str(), nullptr, 0);
}

std::wstring WriteConfig(const Settings& settings, const std::wstring& header) {
    std::wstring out = header;
    std::wstring last;
    for (auto const& [key, value] : settings) {
        std::wstring group = key.substr(0, key.find(L'.'));
        if (group != last) {
            if (!last.empty()) out += L"\n";
            last = group;
        }
        bool quote = value.empty() || value.find(L" #") != std::wstring::npos || value[0] == L'#' ||
                     iswspace(value.front()) || iswspace(value.back()) || value[0] == L'"' || value[0] == L'\'';
        std::wstring shown = value;
        if (quote) {
            shown = L"\"";
            for (wchar_t c : value) {
                if (c == L'"' || c == L'\\') shown += L'\\';
                if (c == L'\n') { shown += L"\\n"; continue; }
                shown += c;
            }
            shown += L"\"";
        }
        out += key + L" = " + shown + L"\n";
    }
    return out;
}

}  // namespace td
