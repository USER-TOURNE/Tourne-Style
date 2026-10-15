// phone-ancs.cpp, checked: "test" reads made-up ANCS packets (events, a notification's answer in parts, an app's
// name, the phone's dates); "live [seconds]" runs the real connection with the phone in dry run: it shows nothing,
// and prints per notification only its app, category, flags and lengths (never its text). Phone Link must not be
// running (it does this itself then).
//
// Build (Windhawk's compiler):
//   clang++ -std=c++20 -municode -static -DPHONE_MAP_PROBE -DPHONE_ANCS_PROBE -I../.. ancs-probe.cpp -o ancs-probe.exe
//           -lws2_32 -lole32 -loleaut32 -lshell32 -lwindowscodecs -lshlwapi -luser32 -lgdi32 -lruntimeobject
#include <winsock2.h>
#include <ws2bth.h>
#include <windows.h>
#include <cfgmgr32.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <tlhelp32.h>
#include <wincodec.h>

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <cstdio>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <string_view>
#include <vector>

#include "phone-link.cpp"
#include "phone-map.cpp"
#include "phone-ancs.cpp"

using namespace phoneancs;

int g_failures = 0;
void Check(bool ok, char const* what) {
    printf("%s  %s\n", ok ? "ok  " : "FAIL", what);
    if (!ok) g_failures++;
}

std::vector<uint8_t> Attr(uint8_t id, std::string const& v) {
    std::vector<uint8_t> a{id, (uint8_t)v.size(), (uint8_t)(v.size() >> 8)};
    a.insert(a.end(), v.begin(), v.end());
    return a;
}

int Test() {
    uint8_t raw[8] = {kAdded, kPreExisting | kNegativeAction, 4, 1, 0x78, 0x56, 0x34, 0x12};
    Event e;
    Check(ParseEvent(raw, 8, e) && e.id == kAdded && e.flags == (kPreExisting | kNegativeAction) && e.category == 4 && e.uid == 0x12345678,
          "an event: id, flags, category, its id little-endian");
    Check(!ParseEvent(raw, 7, e), "a short event refused");

    auto ask = AskNotification(0x12345678);
    Check(ask.size() == 18 && ask[0] == 0 && ask[1] == 0x78 && ask[4] == 0x12 && ask[5] == kApp && ask[6] == kTitle && ask[7] == 128 &&
              ask[12] == kMessage && ask[13] == 0 && ask[14] == 4,
          "the question for a notification: its id, then the attributes with Title 128 and Message 1024");

    std::vector<uint8_t> answer{0, 0x78, 0x56, 0x34, 0x12};
    for (auto const& a : {Attr(kApp, "com.hammerandchisel.discord"), Attr(kTitle, "Ana"), Attr(kSubtitle, ""),
                          Attr(kMessage, "Bring \xF0\x9F\x8D\x95"), Attr(kDate, "20261008T153000"), Attr(kPositiveLabel, ""),
                          Attr(kNegativeLabel, "Clear")})
        answer.insert(answer.end(), a.begin(), a.end());
    std::map<uint8_t, std::string> v;
    bool partsOk = true;
    for (size_t cut = 1; cut < answer.size(); cut++)
        if (ParseNotification(std::vector<uint8_t>(answer.begin(), answer.begin() + cut), 0x12345678, v) != 0) partsOk = false;
    Check(partsOk, "any part of the answer: more to come");
    Check(ParseNotification(answer, 0x12345678, v) == 1 && v[kApp] == "com.hammerandchisel.discord" && v[kTitle] == "Ana" &&
              v[kSubtitle].empty() && v[kNegativeLabel] == "Clear",
          "the whole answer: every attribute");
    Check(phonemap::FromUtf8(v[kMessage]) == L"Bring \U0001F355", "its message, UTF-8");
    Check(ParseNotification(answer, 0x12345679, v) == -1, "an answer for another notification: not this one");

    std::vector<uint8_t> app{1};
    std::string id = "com.apple.mobilephone";
    app.insert(app.end(), id.begin(), id.end());
    app.push_back(0);
    auto name = Attr(0, "Phone");
    app.insert(app.end(), name.begin(), name.end());
    std::string got;
    Check(ParseApp(app, id, got) == 1 && got == "Phone", "an app's name");
    Check(ParseApp(std::vector<uint8_t>(app.begin(), app.end() - 2), id, got) == 0, "an app's name in part: more to come");
    Check(ParseApp(app, "com.other", got) == -1, "another app's: not this one");
    Check(AskApp(id).size() == id.size() + 3 && AskApp(id)[0] == 1 && AskApp(id).back() == 0, "the question for an app");
    Check(Act(5, 1) == std::vector<uint8_t>{2, 5, 0, 0, 0, 1}, "an action: negative");

    std::wstring ts = Timestamp("20261008T153000");
    Check(ts.size() == 20 && ts.back() == L'Z' && ts.substr(0, 4) == L"2026", "the phone's date as UTC");
    Check(Timestamp("garbage").empty(), "an unreadable date: none");
    printf("%d failed\n", g_failures);
    return g_failures;
}

int Live(int seconds) {
    g_dryRun = true;
    phonemap::g_stopEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);  // (phone-map's, which this reads)
    Begin(nullptr, 0);
    for (int i = 0; i < seconds; i++) {
        Sleep(1000);
        static Link last = (Link)-1;
        Link now = LinkNow();
        if (now != last) {
            static PCWSTR names[] = {L"off", L"no paired phone", L"Phone Link has it", L"not connected", L"connected", L"the phone said no"};
            wprintf(L"[%ds] %ls\n", i + 1, names[(int)now]);
            last = now;
        }
    }
    End();
    wprintf(L"%d notifications in %d seconds\n", g_shown.load(), seconds);
    return 0;
}

int wmain(int argc, wchar_t** argv) {
    if (argc > 1 && !wcscmp(argv[1], L"live")) return Live(argc > 2 ? _wtoi(argv[2]) : 60);
    return Test();
}
