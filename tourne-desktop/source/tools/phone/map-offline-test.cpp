// phone-map.cpp without a phone: its bMessage reading and writing, and its OBEX framing against a fake phone on
// a local TCP socket (the session's GET over several packets, a PUT split to the phone's packet size, the
// notification server taking an event report in two parts). Prints each check; exit code = failures.
//
// Build (Windhawk's compiler):
//   clang++ -std=c++20 -municode -static -DPHONE_MAP_PROBE -I../.. map-offline-test.cpp -o map-offline-test.exe
//           -lws2_32 -lole32 -lshell32 -lwindowscodecs -lshlwapi -luser32 -lgdi32 -lruntimeobject
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
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include "phone-link.cpp"
#include "phone-map.cpp"

using namespace phonemap;

int g_failures = 0;
void Check(bool ok, char const* what) {
    printf("%s  %s\n", ok ? "ok  " : "FAIL", what);
    if (!ok) g_failures++;
}

// A connected pair of TCP sockets on loopback.
bool Pair(SOCKET& a, SOCKET& b) {
    SOCKET l = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in at{};
    at.sin_family = AF_INET;
    at.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    int size = sizeof(at);
    if (bind(l, (sockaddr*)&at, sizeof(at)) || listen(l, 1) || getsockname(l, (sockaddr*)&at, &size)) return false;
    a = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (connect(a, (sockaddr*)&at, sizeof(at))) return false;
    b = accept(l, nullptr, nullptr);
    closesocket(l);
    return b != INVALID_SOCKET;
}

std::string Utf8Of(std::vector<uint8_t> const& p, uint8_t header) {
    std::string out;
    Headers(p, 3, [&](uint8_t h, uint8_t const* d, size_t n) {
        if (h == header) out.append((char const*)d, n);
    });
    return out;
}

int wmain() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
    g_noticeEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);

    // An iPhone-like text: the sender's card first (a quoted-printable name), the recipient's inside the envelope.
    std::string in =
        "BEGIN:BMSG\r\nVERSION:1.0\r\nSTATUS:UNREAD\r\nTYPE:SMS_GSM\r\nFOLDER:TELECOM/MSG/INBOX\r\n"
        "BEGIN:VCARD\r\nVERSION:2.1\r\nN;CHARSET=UTF-8;ENCODING=QUOTED-PRINTABLE:Bj=C3=B6rk;Ana;;;\r\nTEL;TYPE=CELL:+1 (555) 010-0199\r\nEND:VCARD\r\n"
        "BEGIN:BENV\r\nBEGIN:VCARD\r\nVERSION:2.1\r\nFN:Me\r\nTEL:+15550100100\r\nEND:VCARD\r\n"
        "BEGIN:BBODY\r\nCHARSET:UTF-8\r\nLENGTH:40\r\nBEGIN:MSG\r\nSee you at 6?\r\nBring \xF0\x9F\x8D\x95\r\nEND:MSG\r\nEND:BBODY\r\nEND:BENV\r\nEND:BMSG\r\n";
    Parsed p = ParseBMessage(in);
    Check(p.ok, "a text is read");
    Check(p.name == L"Ana Björk", "the sender's name, from N in quoted-printable UTF-8");
    Check(p.address == L"+1 (555) 010-0199", "the sender's number, not the recipient's");
    Check(p.body == L"See you at 6?\nBring \U0001F355", "the text, its two lines and an emoji");
    Check(Key(L"+1 (555) 010-0199") == Key(L"+15550100199") && Key(L"5550100199") == Key(L"+1 555 010 0199"), "one key however the number is written");
    Check(Key(L"Ana@Example.com") == L"ana@example.com", "an email's key");

    std::string out = MakeBMessage(L"+15550100199", L"Ana; B", L"On my way\nnow");
    size_t begin = out.find("BEGIN:MSG\r\n"), end = out.find("END:MSG\r\n");
    size_t length = std::stoul(out.substr(out.find("LENGTH:") + 7));
    Check(begin != std::string::npos && end != std::string::npos && length == end + 9 - begin, "LENGTH counts BEGIN:MSG to END:MSG");
    Check(out.find("On my way\r\nnow") != std::string::npos, "its lines end CRLF");
    Check(out.find("N:Ana\\; B\r\n") != std::string::npos && out.find("TEL:+15550100199\r\n") != std::string::npos, "the recipient's card, escaped");
    Check(MakeBMessage(L"ana@example.com", L"", L"hi").find("EMAIL:ana@example.com") != std::string::npos, "an email address as EMAIL");

    std::wstring listing = FromUtf8(
        "<MAP-msg-listing version=\"1.0\"><msg handle=\"20000100001\" subject=\"Tom &amp; Jerry\" sender_name=\"A\"/>"
        "<msg handle=\"20000100002\" subject=\"x\"/></MAP-msg-listing>");
    auto msgs = XmlElements(listing, L"msg");
    Check(msgs.size() == 2 && XmlAttribute(msgs[0], L"handle") == L"20000100001" && XmlAttribute(msgs[0], L"subject") == L"Tom & Jerry",
          "a listing's handles and attributes");
    Check(Unescape(Escape(L"a\tb\\c\nd")) == L"a\tb\\c\nd", "the store's escaping");

    // The session against a fake phone: SETPATH, a GET answered in three packets, a PUT split to 64-byte packets.
    SOCKET mine, phone;
    Check(Pair(mine, phone), "a local socket pair");
    std::string pushed;
    int putPackets = 0;
    std::thread fake([&] {
        std::vector<uint8_t> q;
        int gets = 0;
        while (Receive(phone, q)) {
            uint8_t op = q[0];
            if (op == kSetPath) SendAll(phone, Packet(kOk).Done());
            else if (op == kGetFinal) {
                gets++;
                std::string part = gets == 1 ? "<MAP-msg-listing><msg handle=\"1\"/>" : gets == 2 ? "<msg handle=\"2\"/>" : "</MAP-msg-listing>";
                Packet a(gets < 3 ? kContinue : kOk);
                a.Header(gets < 3 ? kHBody : kHEndOfBody, part.data(), part.size());
                SendAll(phone, a.Done());
            } else if (op == kPut || op == kPutFinal) {
                putPackets++;
                Check(q.size() <= 64, "each PUT packet within the phone's size");
                pushed += Utf8Of(q, kHBody) + Utf8Of(q, kHEndOfBody);
                if (op == kPut) SendAll(phone, Packet(kContinue).Done());
                else {
                    std::wstring handle = L"4000000001";
                    SendAll(phone, Packet(kOk).Name(handle).Done());
                }
            } else if (op == kDisconnect) {
                SendAll(phone, Packet(kOk).Done());
                break;
            }
        }
    });
    Session s;
    s.s = mine;
    s.haveId = true;
    s.id = 7;
    s.most = 64;
    Check(s.ToMessages(), "into telecom/msg");
    std::string xml;
    Check(s.Listing(L"inbox", 50, xml) && XmlElements(FromUtf8(xml), L"msg").size() == 2, "a listing over three packets");
    std::string bmsg = MakeBMessage(L"+15550100199", L"Ana", L"A reply long enough to need more than one packet of sixty-four bytes.");
    std::wstring handle;
    Check(s.Push(bmsg, &handle) == 0, "a text pushed");
    Check(pushed == bmsg && putPackets > 2, "its bMessage arrived whole, in several packets");
    Check(handle == L"4000000001", "the sent text's handle, from the answer");
    s.Close();
    fake.join();
    closesocket(phone);

    // The notification server: the phone connects, sends an event report in two PUTs, disconnects.
    SOCKET server, client;
    Check(Pair(server, client), "a second socket pair");
    Notices notices;
    std::thread serving([&] { notices.Serve(server); });
    std::vector<uint8_t> a;
    Packet c(kConnect);
    c.Byte(0x10).Byte(0).U16(0x400).Header(kHTarget, kMnsTarget, 16);
    Check(SendAll(client, c.Done()) && Receive(client, a) && a[0] == kOk, "the phone's CONNECT taken");
    std::string who;
    Headers(a, 7, [&](uint8_t h, uint8_t const* d, size_t n) {
        if (h == kHWho) who.assign((char const*)d, n);
    });
    Check(who == std::string((char const*)kMnsTarget, 16), "answered as MAP's notification server");
    std::string report =
        "<MAP-event-report version=\"1.0\"><event type=\"NewMessage\" handle=\"20000100003\" folder=\"TELECOM/MSG/INBOX\" "
        "msg_type=\"SMS_GSM\"/><event type=\"NewMessage\" handle=\"9\" folder=\"TELECOM/MSG/SENT\" msg_type=\"SMS_GSM\"/></MAP-event-report>";
    Packet first(kPut);
    first.Id(1).Type("x-bt/MAP-event-report").Header(kHBody, report.data(), 60);
    Check(SendAll(client, first.Done()) && Receive(client, a) && a[0] == kContinue, "its first part: continue");
    Packet last(kPutFinal);
    last.Header(kHEndOfBody, report.data() + 60, report.size() - 60);
    Check(SendAll(client, last.Done()) && Receive(client, a) && a[0] == kOk, "its last part: taken");
    Check(SendAll(client, Packet(kDisconnect).Done()) && Receive(client, a) && a[0] == kOk, "its DISCONNECT");
    serving.join();
    {
        std::lock_guard<std::mutex> lock(g.lock);
        Check(g.arrived.size() == 1 && g.arrived[0] == L"20000100003", "the inbox's new text noted, the sent one not");
    }
    Check(WaitForSingleObject(g_noticeEvent, 0) == WAIT_OBJECT_0, "the session woken for it");
    closesocket(client);
    closesocket(server);

    printf("%d failed\n", g_failures);
    return g_failures;
}
