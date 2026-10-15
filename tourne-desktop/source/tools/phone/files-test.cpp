// phone-files.cpp's sending against a fake LocalSend phone on this PC (plain HTTP on 127.0.0.1:53317): asks what
// it is (https refused, http answers), prepare-upload with two files (one with an accented name), then each file
// streamed, compared byte for byte with what arrived. Exit code = failures.
//
// Build (Windhawk's compiler):
//   clang++ -std=c++20 -municode -static -DPHONE_MAP_PROBE -DPHONE_FILES_PROBE -I../.. files-test.cpp -o files-test.exe
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

int g_failures = 0;
void Check(bool ok, char const* what) {
    printf("%s  %s\n", ok ? "ok  " : "FAIL", what);
    if (!ok) g_failures++;
}

std::mutex g_lock;
std::string g_prepared;
std::map<std::string, std::string> g_uploads;  // fileId: bytes

// One request on a connection: its line, headers and body; one answer; closed.
void Serve(SOCKET c) {
    std::string in;
    char buf[65536];
    size_t headerEnd;
    for (;;) {
        int n = recv(c, buf, sizeof(buf), 0);
        if (n <= 0) return closesocket(c), void();
        in.append(buf, n);
        if ((headerEnd = in.find("\r\n\r\n")) != std::string::npos) break;
        if (in.size() > 8 && (unsigned char)in[0] == 0x16) return closesocket(c), void();  // (a TLS hello: not here)
    }
    std::string head = in.substr(0, headerEnd), body = in.substr(headerEnd + 4);
    size_t length = 0;
    std::string lower = head;
    for (auto& ch : lower) ch = (char)tolower((unsigned char)ch);
    if (size_t at = lower.find("content-length:"); at != std::string::npos) length = strtoull(head.c_str() + at + 15, nullptr, 10);
    while (body.size() < length) {
        int n = recv(c, buf, sizeof(buf), 0);
        if (n <= 0) break;
        body.append(buf, n);
    }
    std::string path = head.substr(head.find(' ') + 1);
    path = path.substr(0, path.find(' '));
    std::string answer = "{}";
    int status = 200;
    if (path == "/api/localsend/v2/info") {
        answer = "{\"alias\":\"Fake iPhone\",\"version\":\"2.1\",\"deviceModel\":\"iPhone\",\"deviceType\":\"mobile\",\"fingerprint\":\"abc\",\"download\":false}";
    } else if (path == "/api/localsend/v2/prepare-upload") {
        std::lock_guard<std::mutex> lock(g_lock);
        g_prepared = body;
        answer = "{\"sessionId\":\"s1\",\"files\":{\"f0\":\"t0\",\"f1\":\"t1\"}}";
    } else if (path.starts_with("/api/localsend/v2/upload?sessionId=s1&fileId=")) {
        std::string id = path.substr(path.find("fileId=") + 7, 2);
        bool tokenOk = path.find("token=t" + id.substr(1)) != std::string::npos;
        std::lock_guard<std::mutex> lock(g_lock);
        if (tokenOk) g_uploads[id] = body;
        else status = 403;
    } else {
        status = 404;
    }
    std::string reply = "HTTP/1.1 " + std::to_string(status) + " X\r\nContent-Type: application/json\r\nContent-Length: " +
                        std::to_string(answer.size()) + "\r\nConnection: close\r\n\r\n" + answer;
    send(c, reply.data(), (int)reply.size(), 0);
    shutdown(c, SD_SEND);
    closesocket(c);
}

int wmain() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    SOCKET l = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in at{};
    at.sin_family = AF_INET;
    at.sin_port = htons(kPort);
    at.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    Check(!bind(l, (sockaddr*)&at, sizeof(at)) && !listen(l, 8), "a fake phone on 127.0.0.1:53317");
    std::thread([l] {
        for (SOCKET c; (c = accept(l, nullptr, nullptr)) != INVALID_SOCKET;) std::thread(Serve, c).detach();
    }).detach();

    // Two files: one of 300 KB (several pieces), one small with an accented name.
    WCHAR temp[MAX_PATH];
    GetTempPathW(MAX_PATH, temp);
    std::wstring big = std::wstring(temp) + L"tourne-files-test.bin", small = std::wstring(temp) + L"naïve résumé.txt";
    std::string bigBytes(300 * 1024, '\0'), smallBytes = "hello phone\n";
    for (size_t i = 0; i < bigBytes.size(); i++) bigBytes[i] = (char)(i * 7 + (i >> 9));
    for (auto [path, bytes] : {std::pair{big, bigBytes}, std::pair{small, smallBytes}}) {
        HANDLE h = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, 0, nullptr);
        DWORD wrote = 0;
        WriteFile(h, bytes.data(), (DWORD)bytes.size(), &wrote, nullptr);
        CloseHandle(h);
    }

    HINTERNET session = InternetOpenW(L"Tourne", INTERNET_OPEN_TYPE_DIRECT, nullptr, nullptr, 0);
    Device phone;
    Check(Ask(session, L"127.0.0.1", phone), "the fake phone answers what it is");
    Check(!phone.secure && phone.type == "mobile" && phone.model == "iPhone" && phone.alias == "Fake iPhone", "over http, as a mobile iPhone");
    Deliver(session, phone, {big, small});
    {
        std::lock_guard<std::mutex> lock(g_lock);
        Check(g_prepared.find("\"info\":{\"alias\":\"Tourne on ") != std::string::npos, "prepare-upload says who's sending");
        Check(g_prepared.find("\"fileName\":\"tourne-files-test.bin\",\"size\":307200") != std::string::npos, "the big file's name and size");
        Check(g_prepared.find("\"fileName\":\"na\xC3\xAFve r\xC3\xA9sum\xC3\xA9.txt\",\"size\":12,\"fileType\":\"text/plain\"") != std::string::npos,
              "the small one's accented name in UTF-8, its size and type");
        Check(g_uploads["f0"] == bigBytes, "the big file arrived byte for byte");
        Check(g_uploads["f1"] == smallBytes, "the small file arrived byte for byte");
    }
    InternetCloseHandle(session);
    DeleteFileW(big.c_str());
    DeleteFileW(small.c_str());
    printf("%d failed\n", g_failures);
    return g_failures;
}
