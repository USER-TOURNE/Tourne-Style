#include "launcher-math.h"

#include <cmath>
#include <cstdio>
#include <cwctype>
#include <string>

namespace td::launchermath {

namespace {

constexpr double kPi = 3.14159265358979323846;

// A recursive descent over the line: expression = term (+|- term)*, term = power (*|/|% power)*,
// power = unary (^ power)?, unary = (+|-)? atom, atom = number | name | name(expression) | (expression).
struct Parser {
    const std::wstring& s;
    size_t at = 0;
    bool bad = false;
    int operators = 0;  // (a plain number alone isn't a sum)

    void Space() {
        while (at < s.size() && iswspace(s[at])) at++;
    }
    bool Take(wchar_t c) {
        Space();
        if (at < s.size() && s[at] == c) return at++, true;
        return false;
    }
    double Expression() {
        double v = Term();
        for (;;) {
            if (Take(L'+')) v += Term(), operators++;
            else if (Take(L'-')) v -= Term(), operators++;
            else return v;
        }
    }
    double Term() {
        double v = Power();
        for (;;) {
            if (Take(L'*') || Take(L'x') || Take(L'×')) v *= Power(), operators++;
            else if (Take(L'/') || Take(L'÷')) v /= Power(), operators++;
            else if (Take(L'%')) v = std::fmod(v, Power()), operators++;
            else return v;
        }
    }
    double Power() {
        double base = Unary();
        if (Take(L'^')) return operators++, std::pow(base, Power());
        if (at + 1 < s.size() && s[at] == L'*' && s[at + 1] == L'*') return at += 2, operators++, std::pow(base, Power());
        return base;
    }
    double Unary() {
        if (Take(L'-')) return -Unary();
        if (Take(L'+')) return Unary();
        return Atom();
    }
    double Atom() {
        Space();
        if (Take(L'(')) {
            double v = Expression();
            if (!Take(L')')) bad = true;
            return v;
        }
        if (at < s.size() && (iswdigit(s[at]) || s[at] == L'.')) {
            size_t from = at;
            while (at < s.size() && (iswdigit(s[at]) || s[at] == L'.' || s[at] == L',')) at++;
            std::wstring n;
            for (size_t i = from; i < at; i++)
                if (s[i] != L',') n += s[i];  // (thousands written with commas)
            wchar_t* end = nullptr;
            double v = wcstod(n.c_str(), &end);
            if (!end || *end) bad = true;
            return v;
        }
        size_t from = at;
        while (at < s.size() && iswalpha(s[at])) at++;
        std::wstring name = s.substr(from, at - from);
        for (auto& c : name) c = towlower(c);
        if (name == L"pi") return kPi;
        if (name == L"e") return std::exp(1.0);
        static const struct {
            const wchar_t* name;
            double (*f)(double);
        } kFunctions[] = {
            {L"sqrt", [](double x) { return std::sqrt(x); }}, {L"abs", [](double x) { return std::fabs(x); }},
            {L"round", [](double x) { return std::round(x); }}, {L"floor", [](double x) { return std::floor(x); }},
            {L"ceil", [](double x) { return std::ceil(x); }}, {L"sin", [](double x) { return std::sin(x); }},
            {L"cos", [](double x) { return std::cos(x); }}, {L"tan", [](double x) { return std::tan(x); }},
            {L"log", [](double x) { return std::log10(x); }}, {L"ln", [](double x) { return std::log(x); }},
        };
        for (auto const& f : kFunctions)
            if (name == f.name && Take(L'(')) {
                double v = Expression();
                if (!Take(L')')) bad = true;
                operators++;
                return f.f(v);
            }
        bad = true;
        return 0;
    }
};

struct Unit {
    const wchar_t* names;  // space separated, lower case
    int kind;              // 0 length 1 mass 2 volume 3 data 4 time 5 speed 6 temperature 7 area
    double factor;         // to the kind's base (metre, gram, litre, byte, second, m/s, -, m2)
};

const Unit kUnits[] = {
    {L"mm millimetre millimeter millimetres millimeters", 0, 0.001}, {L"cm centimetre centimeter centimetres centimeters", 0, 0.01},
    {L"m metre meter metres meters", 0, 1}, {L"km kilometre kilometer kilometres kilometers", 0, 1000},
    {L"in inch inches \"", 0, 0.0254}, {L"ft foot feet '", 0, 0.3048}, {L"yd yard yards", 0, 0.9144},
    {L"mi mile miles", 0, 1609.344}, {L"nmi nauticalmile", 0, 1852},
    {L"mg milligram milligrams", 1, 0.001}, {L"g gram grams", 1, 1}, {L"kg kilogram kilograms kilo kilos", 1, 1000},
    {L"t tonne tonnes", 1, 1e6}, {L"oz ounce ounces", 1, 28.349523125}, {L"lb lbs pound pounds", 1, 453.59237},
    {L"st stone stones", 1, 6350.29318},
    {L"ml millilitre milliliter millilitres milliliters", 2, 0.001}, {L"cl", 2, 0.01}, {L"dl", 2, 0.1},
    {L"l litre liter litres liters", 2, 1}, {L"tsp teaspoon teaspoons", 2, 0.00492892159375},
    {L"tbsp tablespoon tablespoons", 2, 0.01478676478125}, {L"floz", 2, 0.0295735295625}, {L"cup cups", 2, 0.2365882365},
    {L"pt pint pints", 2, 0.473176473}, {L"qt quart quarts", 2, 0.946352946}, {L"gal gallon gallons", 2, 3.785411784},
    {L"b byte bytes", 3, 1}, {L"kb kilobyte kilobytes", 3, 1e3}, {L"mb megabyte megabytes", 3, 1e6},
    {L"gb gigabyte gigabytes", 3, 1e9}, {L"tb terabyte terabytes", 3, 1e12}, {L"kib", 3, 1024.0},
    {L"mib", 3, 1048576.0}, {L"gib", 3, 1073741824.0}, {L"tib", 3, 1099511627776.0}, {L"bit bits", 3, 0.125},
    {L"ms millisecond milliseconds", 4, 0.001}, {L"s sec secs second seconds", 4, 1}, {L"min mins minute minutes", 4, 60},
    {L"h hr hrs hour hours", 4, 3600}, {L"d day days", 4, 86400}, {L"wk week weeks", 4, 604800},
    {L"yr year years", 4, 31557600},
    {L"m/s mps", 5, 1}, {L"km/h kmh kph", 5, 1 / 3.6}, {L"mph", 5, 0.44704}, {L"kn kt knot knots", 5, 1852.0 / 3600},
    {L"c celsius °c", 6, 0}, {L"f fahrenheit °f", 6, 1}, {L"k kelvin", 6, 2},
    {L"m2 sqm", 7, 1}, {L"km2", 7, 1e6}, {L"ha hectare hectares", 7, 1e4}, {L"acre acres", 7, 4046.8564224},
    {L"ft2 sqft", 7, 0.09290304},
};

const Unit* UnitNamed(std::wstring n) {
    for (auto& c : n) c = towlower(c);
    if (n.empty()) return nullptr;
    for (auto const& u : kUnits) {
        const wchar_t* p = u.names;
        while (*p) {
            const wchar_t* end = wcschr(p, L' ');
            size_t len = end ? (size_t)(end - p) : wcslen(p);
            if (n.size() == len && !n.compare(0, len, p, len)) return &u;
            if (!end) break;
            p = end + 1;
        }
    }
    return nullptr;
}

double ToKelvin(double v, double scale) { return scale == 0 ? v + 273.15 : scale == 1 ? (v - 32) * 5 / 9 + 273.15 : v; }
double FromKelvin(double k, double scale) { return scale == 0 ? k - 273.15 : scale == 1 ? (k - 273.15) * 9 / 5 + 32 : k; }

}  // namespace

bool Evaluate(const std::wstring& text, double* out) {
    std::wstring t = text;
    size_t start = t.find_first_not_of(L" \t");
    if (start == std::wstring::npos) return false;
    bool asked = t[start] == L'=';  // ("= 7" is a sum asked for, even a plain number)
    if (asked) t = t.substr(start + 1);
    Parser p{t};
    double v = p.Expression();
    p.Space();
    if (p.bad || p.at != t.size() || (!asked && !p.operators) || std::isnan(v) || std::isinf(v)) return false;
    *out = v;
    return true;
}

bool Convert(const std::wstring& text, std::wstring* answer) {
    // N unit (to|in|as) unit, the number first (it may be a sum).
    std::wstring t;
    for (wchar_t c : text) t += towlower(c);
    size_t sep = std::wstring::npos, sepLen = 0;
    for (const wchar_t* word : {L" to ", L" in ", L" as ", L" -> "}) {
        size_t at = t.rfind(word);
        if (at != std::wstring::npos && (sep == std::wstring::npos || at > sep)) sep = at, sepLen = wcslen(word);
    }
    if (sep == std::wstring::npos) return false;
    std::wstring from = t.substr(0, sep), to = t.substr(sep + sepLen);
    auto trim = [](std::wstring& s) {
        size_t a = s.find_first_not_of(L" \t"), b = s.find_last_not_of(L" \t");
        s = a == std::wstring::npos ? L"" : s.substr(a, b - a + 1);
    };
    trim(from), trim(to);
    // The unit is what follows the number: the last letters (or a degree sign, ", ').
    size_t u = from.size();
    while (u > 0 && (iswalpha(from[u - 1]) || from[u - 1] == L'/' || from[u - 1] == L'°' || from[u - 1] == L'"' ||
                     from[u - 1] == L'\'' || (iswdigit(from[u - 1]) && u >= 2 && iswalpha(from[u - 2]))))
        u--;
    std::wstring amount = from.substr(0, u), unitFrom = from.substr(u);
    trim(amount), trim(unitFrom);
    const Unit* a = UnitNamed(unitFrom);
    const Unit* b = UnitNamed(to);
    if (!a || !b || a->kind != b->kind || amount.empty()) return false;
    double v;
    if (!Evaluate(L"=" + amount, &v)) return false;
    double r = a->kind == 6 ? FromKelvin(ToKelvin(v, a->factor), b->factor) : v * a->factor / b->factor;
    std::wstring name = to;
    *answer = Number(r) + L" " + name;
    return true;
}

std::wstring Number(double v) {
    if (v == 0) return L"0";
    wchar_t b[64];
    double mag = std::fabs(v);
    if (mag >= 1e15 || mag < 1e-6) swprintf(b, 64, L"%.6g", v);
    else swprintf(b, 64, L"%.10g", v);
    return b;
}

}  // namespace td::launchermath
