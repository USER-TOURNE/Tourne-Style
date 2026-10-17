// Does Tourne's own MAP session (phone-map.cpp) work with the phone? Opens it as Tourne Desktop does, asks the
// phone to tell of new texts (its notification server registered in SDP), waits, and prints only what happened:
// connect results, whether the phone connected back, and per new text the lengths of what was read (never the
// text, the name or the number). Sends nothing. Phone Link must not be running (it holds the channel).
//
//   map-session-probe.exe [seconds to wait, default 60]
// Build (Windhawk's compiler):
//   clang++ -std=c++20 -municode -static -DPHONE_MAP_PROBE -I../.. map-session-probe.cpp -o map-session-probe.exe
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

using namespace phonemap;

int wmain(int argc, wchar_t** argv) {
    int seconds = argc > 1 ? _wtoi(argv[1]) : 60;
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
    g_stopEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    g_wakeEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    g_noticeEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);

    PhoneInfo who;
    ReadPhoneIdentity(who);
    BTH_ADDR address = who.linked ? ClassicAddress(who.name) : 0;
    wprintf(L"phone linked: %d; classic address found: %ls\n", who.linked, address ? L"yes" : L"no");
    if (!address) return 1;
    if (HANDLE pl = PhoneLinkRunning()) {
        wprintf(L"Phone Link is running: it holds the channel\n");
        CloseHandle(pl);
        return 1;
    }
    Session mas;
    DWORD t0 = GetTickCount();
    int error = mas.Open(address);
    wprintf(L"session: %d after %lu ms (0 = open); packets up to %zu bytes\n", error, GetTickCount() - t0, mas.most);
    if (error) return 1;
    wprintf(L"into telecom/msg: %d\n", mas.ToMessages());
    std::string xml;
    bool listed = mas.Listing(L"inbox", 50, xml);
    wprintf(L"inbox listing: %d, %zu messages\n", listed, XmlElements(FromUtf8(xml), L"msg").size());
    Notices notices;
    bool started = notices.Start();
    wprintf(L"notification server: %d (error %d)\n", started, started ? 0 : WSAGetLastError());
    bool told = started && mas.Notices(true);
    wprintf(L"asked for notices: %d\n", told);
    DWORD until = GetTickCount() + seconds * 1000;
    bool connectedBack = false;
    while ((int)(until - GetTickCount()) > 0) {
        DWORD r = WaitForSingleObject(g_noticeEvent, 500);
        if (!connectedBack && notices.client != INVALID_SOCKET) {
            connectedBack = true;
            wprintf(L"the phone connected to the notification server after %lu ms\n", GetTickCount() - t0);
        }
        if (r != WAIT_OBJECT_0) continue;
        std::deque<std::wstring> arrived;
        bool dropped;
        {
            std::lock_guard<std::mutex> lock(g.lock);
            arrived.swap(g.arrived);
            dropped = g.dropped;
        }
        for (auto const& handle : arrived) {
            std::string bmsg;
            bool ok = mas.Message(handle, bmsg);
            Parsed p = ParseBMessage(bmsg);
            wprintf(L"new text: read %d, %zu bytes; parsed %d; name %zu chars, address %zu chars, text %zu chars\n", ok,
                    bmsg.size(), p.ok, p.name.size(), p.address.size(), p.body.size());
        }
        if (dropped) {
            wprintf(L"the phone closed the notification connection\n");
            break;
        }
    }
    if (told) wprintf(L"notices off: %d\n", mas.Notices(false));
    notices.Stop();
    mas.Close();
    wprintf(L"closed\n");
    WSACleanup();
    return 0;
}
