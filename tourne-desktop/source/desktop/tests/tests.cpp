// Tourne Desktop's console tests: the plain C++ parts (no window, no Windows calls), run by Run-Tests.ps1.
// doctest (MIT, third_party/doctest) gives the main and the checks.
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../third_party/doctest/doctest.h"

#include <cmath>
#include <string>

#include "../host/css-gen.h"
#include "../host/json-lite.h"
#include "../host/launcher-math.h"
#include "../host/palette-gen.h"
#include "../host/toml-lite.h"

using namespace td;

TEST_CASE("json: values by path, in order") {
    json::Value v;
    REQUIRE(json::Parse(R"({"id": 7, "result": {"ok": true, "items": [1, "two", null], "name": "café"}})", &v));
    CHECK(v["id"].number == 7);
    CHECK(v["result"]["ok"].boolean);
    CHECK(v["result"]["items"][1].text == "two");
    CHECK(v["result"]["items"][2].IsNull());
    CHECK(v["result"]["name"].text == "caf\xc3\xa9");  // (UTF-8)
    CHECK(v["missing"]["deeper"].IsNull());
    CHECK(v["result"]["items"][9].IsNull());
    REQUIRE(v.fields.size() == 2);
    CHECK(v.fields[0].first == "id");
}

TEST_CASE("json: not JSON is refused") {
    json::Value v;
    CHECK_FALSE(json::Parse("", &v));
    CHECK_FALSE(json::Parse("{\"a\": }", &v));
    CHECK_FALSE(json::Parse("[1, 2", &v));
    CHECK_FALSE(json::Parse("{\"a\": 1} trailing", &v));
}

TEST_CASE("json: quoting round-trips") {
    std::string odd = "line\nbreak \"quoted\" back\\slash\ttab";
    json::Value v;
    REQUIRE(json::Parse(json::Quote(odd), &v));
    CHECK(v.text == odd);
}

TEST_CASE("toml: a plugin.toml") {
    const char* text = R"(name = "Clock"
version = 2
enabled = true
tags = ["time", "bar"]
size = { w = 120, h = 24 }

[author]
name = "Tourne"

[[widget]]
id = "a"
[[widget.setting]]
key = "format"
[[widget]]
id = "b"
)";
    json::Value v;
    std::string error;
    REQUIRE_MESSAGE(toml::Parse(text, &v, &error), error);
    CHECK(v["name"].text == "Clock");
    CHECK(v["version"].number == 2);
    CHECK(v["enabled"].boolean);
    CHECK(v["tags"][1].text == "bar");
    CHECK(v["size"]["h"].number == 24);
    CHECK(v["author"]["name"].text == "Tourne");
    CHECK(v["widget"].items.size() == 2);
    CHECK(v["widget"][size_t(0)]["setting"][size_t(0)]["key"].text == "format");
    CHECK(v["widget"][1]["id"].text == "b");
}

TEST_CASE("toml: broken text says where") {
    json::Value v;
    std::string error;
    CHECK_FALSE(toml::Parse("name = \"unclosed\n", &v, &error));
    CHECK_FALSE(error.empty());
}

TEST_CASE("palette: contrast is WCAG's") {
    CHECK(Contrast(0x000000, 0xFFFFFF) == doctest::Approx(21.0).epsilon(0.01));
    CHECK(Contrast(0x777777, 0x777777) == doctest::Approx(1.0));
    CHECK(Contrast(0x123456, 0xABCDEF) == doctest::Approx(Contrast(0xABCDEF, 0x123456)));
}

TEST_CASE("palette: mixing and stepping") {
    CHECK(Mixed(0x112233, 0xDDEEFF, 0) == 0x112233);
    CHECK(Mixed(0x112233, 0xDDEEFF, 1) == 0xDDEEFF);
    CHECK(Contrast(Stepped(0x336699, 2), 0x000000) > Contrast(0x336699, 0x000000));  // lighter
    CHECK(Contrast(Stepped(0x336699, -2), 0xFFFFFF) > Contrast(0x336699, 0xFFFFFF));  // darker
}

TEST_CASE("palette: every profile's text is readable on its ground") {
    for (auto profile : {Profile::Adaptive, Profile::Monochrome, Profile::Muted, Profile::Vivid, Profile::Pastel})
        for (bool light : {false, true})
            for (uint32_t seed : {0xE57474u, 0x64AA89u, 0x2244CCu, 0x808080u}) {
                PaletteRequest r;
                r.profile = profile, r.light = light;
                Roles roles = PaletteFromSeed(seed, r);
                INFO("profile " << (int)profile << " light " << light << " seed " << seed);
                CHECK(Contrast(roles.text, roles.ground) >= r.textContrast - 0.05);
                CHECK((Contrast(roles.ground, 0x000000) < Contrast(roles.ground, 0xFFFFFF)) == !light);
            }
}

TEST_CASE("css: templates take every role and state") {
    Roles roles{};
    roles.ground = 0x121C21, roles.text = 0x66EA97, roles.accent = 0x15FFA2, roles.raised = 0x1E2F36;
    CHECK(FillTemplate("a {{accent}} b", roles) == "a #15ffa2 b");
    CHECK(FillTemplate("{{ground.rgb}}", roles) == "18, 28, 33");
    CHECK(FillTemplate("{{ground.hex}}", roles) == "121c21");
    CHECK(FillTemplate("{{nothing}} stays", roles) == "{{nothing}} stays");
    CHECK(FillTemplate("{{raised-hover}}", roles).rfind("#", 0) == 0);
}

TEST_CASE("css: the sheets leave nothing unfilled") {
    PaletteRequest r;
    Roles roles = PaletteFromSeed(0x64AA89, r);
    for (bool on : {false, true}) {
        CssGlass glass;
        glass.on = on, glass.blur = 1, glass.tintAmount = 35, glass.shadow = 50;
        for (auto const& app : StyledApps()) {
            std::string css = AppCss(app, roles, glass);
            INFO(app << " glass " << on);
            CHECK(css.find("{{") == std::string::npos);
            CHECK(css.find("--tourne-glass-page") != std::string::npos);
            CHECK(css.find("border-radius: 0") != std::string::npos);
            // Braces balance, so one bad rule can't swallow the rest of the sheet.
            long depth = 0;
            for (char c : css) depth += c == '{' ? 1 : c == '}' ? -1 : 0;
            CHECK(depth == 0);
        }
    }
}

TEST_CASE("launcher: sums typed in the search line") {
    using namespace td::launchermath;
    double v = 0;
    CHECK((Evaluate(L"2+3*4", &v) && v == 14));
    CHECK((Evaluate(L"(2+3)*4", &v) && v == 20));
    CHECK((Evaluate(L"2^10", &v) && v == 1024));
    CHECK((Evaluate(L"= 17/4", &v) && v == 4.25));
    CHECK((Evaluate(L"=7", &v) && v == 7));
    CHECK((Evaluate(L"sqrt(16) + 1,000", &v) && v == 1004));
    CHECK((Evaluate(L"-3 * -3", &v) && v == 9));
    CHECK((Evaluate(L"10 % 4", &v) && v == 2));
    CHECK_FALSE(Evaluate(L"7", &v));         // a number alone still searches
    CHECK_FALSE(Evaluate(L"chrome", &v));
    CHECK_FALSE(Evaluate(L"2+", &v));
    CHECK_FALSE(Evaluate(L"1/0", &v));
    CHECK_FALSE(Evaluate(L"", &v));
    CHECK(Number(0.1 + 0.2) == L"0.3");
    CHECK(Number(1e20) == L"1e+20");
}

TEST_CASE("launcher: units") {
    using namespace td::launchermath;
    std::wstring a;
    CHECK((Convert(L"10 km to mi", &a) && a == L"6.213711922 mi"));
    CHECK((Convert(L"212 f in c", &a) && a == L"100 c"));
    CHECK((Convert(L"1 gib to mb", &a) && a == L"1073.741824 mb"));
    CHECK((Convert(L"2*3 kg to lb", &a) && a.rfind(L"13.22773", 0) == 0));
    CHECK((Convert(L"5ft to cm", &a) && a == L"152.4 cm"));
    CHECK((Convert(L"90 km/h to mph", &a) && a.rfind(L"55.92", 0) == 0));
    CHECK_FALSE(Convert(L"10 km to kg", &a));  // not the same kind
    CHECK_FALSE(Convert(L"music to go", &a));
    CHECK_FALSE(Convert(L"10 km", &a));
}
