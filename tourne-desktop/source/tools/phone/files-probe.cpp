// phone-files.cpp's search, alone: which addresses on this PC's networks take a connection on LocalSend's port,
// and what each LocalSend device there says it is. Sends nothing.
//
// Build (Windhawk's compiler):
//   clang++ -std=c++20 -municode -static -DPHONE_MAP_PROBE -DPHONE_FILES_PROBE -I../.. files-probe.cpp -o files-probe.exe
//           -lws2_32 -lole32 -loleaut32 -lshell32 -lwindowscodecs -lshlwapi -luser32 -lgdi32 -lruntimeobject -lwininet -liphlpapi
#include <winsock2.h>
#include <ws2bth.h>
#include <ws2tcpip.h>
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
#include "phone-files.cpp"

using namespace phonefiles;

int wmain() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    auto all = Neighbours();
    DWORD t0 = GetTickCount();
    auto open = Listening(all, 1200);
    wprintf(L"%zu addresses asked, %zu listening on %u, in %lu ms\n", all.size(), open.size(), kPort, GetTickCount() - t0);
    HINTERNET session = InternetOpenW(L"Tourne", INTERNET_OPEN_TYPE_DIRECT, nullptr, nullptr, 0);
    t0 = GetTickCount();
    auto found = Find(session, LastAddress());
    wprintf(L"%zu LocalSend devices in %lu ms\n", found.size(), GetTickCount() - t0);
    for (auto const& d : found)
        wprintf(L"  %ls (%ls): %ls, %ls %ls\n", d.ip.c_str(), d.secure ? L"https" : L"http", Words(d.alias).c_str(), Words(d.type).c_str(),
                Words(d.model).c_str());
    InternetCloseHandle(session);
    wprintf(L"this PC as LocalSend sees it: %hs\n", Me().c_str());
    return 0;
}
