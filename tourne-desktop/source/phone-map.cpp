////////////////////////////////////////////////////////////////////////////////
// Your phone's texts, over Bluetooth MAP ("// @inline phone-map.cpp" in taskbar-host.cpp after phone-link.cpp,
// Tourne Desktop only): Tourne talks to the phone's Message Access Server itself, as a car kit does, so the phone
// pane lists texts as they come and sends your replies. Only one program on the PC can hold that channel: while
// Phone Link runs it has it, and Tourne waits for Phone Link to end.
//
// An iPhone shares only the texts that arrive while it's connected (none from before), so what came is kept in
// %APPDATA%\Tourne\Desktop\phone-messages.tsv (the latest 500), on this PC only.
//
// One thread holds the session: it connects, reads a text when the phone says one came, and sends what's queued.
// A second answers the phone's notices (MAP's notification server, which the phone connects back to). Idle cost:
// both wait on events; with the phone away, connecting is tried again after 1, 2, then every 5 minutes.

#if defined(TD_PREFIX) || defined(PHONE_MAP_PROBE)
#include <winsock2.h>
#include <ws2bth.h>
#include <deque>
#include <initializer_list>
#include <winrt/Windows.Data.Xml.Dom.h>
#include <winrt/Windows.UI.Notifications.h>

namespace phonemap {

constexpr GUID kMasClass = {0x00001132, 0x0000, 0x1000, {0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB}};
constexpr uint8_t kMasTarget[16] = {0xbb, 0x58, 0x2b, 0x40, 0x42, 0x0c, 0x11, 0xdb, 0xb0, 0xde, 0x08, 0x00, 0x20, 0x0c, 0x9a, 0x66};
constexpr uint8_t kMnsTarget[16] = {0xbb, 0x58, 0x2b, 0x41, 0x42, 0x0c, 0x11, 0xdb, 0xb0, 0xde, 0x08, 0x00, 0x20, 0x0c, 0x9a, 0x66};

// OBEX's operations, answers and headers, and MAP's application parameters.
enum : uint8_t {
    kConnect = 0x80, kDisconnect = 0x81, kPut = 0x02, kPutFinal = 0x82, kGetFinal = 0x83, kSetPath = 0x85,
    kContinue = 0x90, kOk = 0xA0, kBadRequest = 0xC0,
};
enum : uint8_t {
    kHName = 0x01, kHType = 0x42, kHTarget = 0x46, kHBody = 0x48, kHEndOfBody = 0x49, kHWho = 0x4A, kHParams = 0x4C,
    kHConnectionId = 0xCB,
};
enum : uint8_t {
    kPMaxListCount = 0x01, kPAttachment = 0x0A, kPTransparent = 0x0B, kPRetry = 0x0C, kPNotificationStatus = 0x0E,
    kPSubjectLength = 0x13, kPCharset = 0x14,
};

// One OBEX packet, built up: its code, then its length (filled in by Done), then headers.
struct Packet {
    std::vector<uint8_t> b;
    explicit Packet(uint8_t op) : b{op, 0, 0} {}
    Packet& Byte(uint8_t v) {
        b.push_back(v);
        return *this;
    }
    Packet& U16(uint16_t v) { return Byte(v >> 8).Byte(v & 0xFF); }
    Packet& U32(uint32_t v) { return U16(v >> 16).U16(v & 0xFFFF); }
    Packet& Bytes(void const* p, size_t n) {
        if (n) b.insert(b.end(), (uint8_t const*)p, (uint8_t const*)p + n);
        return *this;
    }
    // A header with a two-byte length (its own three bytes counted).
    Packet& Header(uint8_t id, void const* p, size_t n) { return Byte(id).U16((uint16_t)(n + 3)).Bytes(p, n); }
    Packet& Id(uint32_t id) { return Byte(kHConnectionId).U32(id); }
    Packet& Type(char const* type) { return Header(kHType, type, strlen(type) + 1); }
    // UTF-16, big-endian, ending in a null (an empty name has none: the root folder).
    Packet& Name(std::wstring const& name) {
        std::vector<uint8_t> u;
        for (wchar_t c : name) u.push_back(c >> 8), u.push_back(c & 0xFF);
        if (!name.empty()) u.push_back(0), u.push_back(0);
        return Header(kHName, u.data(), u.size());
    }
    Packet& Params(std::initializer_list<uint8_t> tlv) { return Header(kHParams, tlv.begin(), tlv.size()); }
    std::vector<uint8_t> Done() {
        b[1] = (uint8_t)(b.size() >> 8);
        b[2] = (uint8_t)b.size();
        return b;
    }
};

bool SendAll(SOCKET s, std::vector<uint8_t> const& b) {
    for (size_t sent = 0; sent < b.size();) {
        int n = send(s, (char const*)b.data() + sent, (int)(b.size() - sent), 0);
        if (n <= 0) return false;
        sent += n;
    }
    return true;
}

// One whole packet: its code, length and headers.
bool Receive(SOCKET s, std::vector<uint8_t>& out) {
    out.assign(3, 0);
    for (int got = 0; got < 3;) {
        int n = recv(s, (char*)out.data() + got, 3 - got, 0);
        if (n <= 0) return false;
        got += n;
    }
    size_t length = (out[1] << 8) | out[2];
    if (length < 3) return false;
    out.resize(length);
    for (size_t got = 3; got < length;) {
        int n = recv(s, (char*)out.data() + got, (int)(length - got), 0);
        if (n <= 0) return false;
        got += n;
    }
    return true;
}

// Each header of a packet from at: its id, data and size (the one-byte and four-byte kinds too).
template <typename Each>
void Headers(std::vector<uint8_t> const& p, size_t at, Each each) {
    while (at < p.size()) {
        uint8_t h = p[at];
        size_t length = 0;
        switch (h & 0xC0) {
            case 0x80:
                length = 2;
                if (at + length > p.size()) return;
                each(h, &p[at + 1], 1);
                break;
            case 0xC0:
                length = 5;
                if (at + length > p.size()) return;
                each(h, &p[at + 1], 4);
                break;
            default:
                if (at + 3 > p.size()) return;
                length = (p[at + 1] << 8) | p[at + 2];
                if (length < 3 || at + length > p.size()) return;
                each(h, &p[at + 3], length - 3);
        }
        at += length;
    }
}

std::wstring FromUtf8(std::string const& s) {
    int n = MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0);
    std::wstring out(n, L'\0');
    if (n) MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), out.data(), n);
    return out;
}

std::string ToUtf8(std::wstring const& s) {
    int n = WideCharToMultiByte(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0, nullptr, nullptr);
    std::string out(n, '\0');
    if (n) WideCharToMultiByte(CP_UTF8, 0, s.data(), (int)s.size(), out.data(), n, nullptr, nullptr);
    return out;
}

HANDLE g_stopEvent, g_wakeEvent, g_noticeEvent;
std::atomic<SOCKET> g_live{INVALID_SOCKET};  // the session's socket, so ending can cut a wait on the phone short

// The session with the phone's Message Access Server, on an RFCOMM socket.
struct Session {
    SOCKET s = INVALID_SOCKET;
    uint32_t id = 0;
    bool haveId = false;
    size_t most = 255;  // the largest packet either side takes

    // 0 when open, else Winsock's error (10048: another program has the channel; 10060: the phone isn't in
    // reach) or -1 (the phone said no, or ending).
    int Open(BTH_ADDR address) {
        s = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
        if (s == INVALID_SOCKET) return WSAGetLastError();
        g_live = s;
        SOCKADDR_BTH a{};
        a.addressFamily = AF_BTH;
        a.btAddr = address;
        a.serviceClassId = kMasClass;  // (the channel is looked up by SDP)
        // Connecting can take the phone's whole page timeout: waited for in steps, so ending needn't.
        u_long on = 1, off = 0;
        ioctlsocket(s, FIONBIO, &on);
        int error = 0;
        if (connect(s, (sockaddr*)&a, sizeof(a)) == SOCKET_ERROR && (error = WSAGetLastError()) == WSAEWOULDBLOCK) {
            error = WSAETIMEDOUT;
            for (int step = 0; step < 120; step++) {  // (30 seconds)
                if (g_stopEvent && WaitForSingleObject(g_stopEvent, 0) == WAIT_OBJECT_0) {
                    error = -1;
                    break;
                }
                fd_set writable, failed;
                FD_ZERO(&writable);
                FD_ZERO(&failed);
                FD_SET(s, &writable);
                FD_SET(s, &failed);
                timeval wait{0, 250000};
                if (select(0, nullptr, &writable, &failed, &wait) <= 0) continue;
                if (FD_ISSET(s, &failed)) {
                    int size = sizeof(error);
                    getsockopt(s, SOL_SOCKET, SO_ERROR, (char*)&error, &size);
                    if (!error) error = WSAECONNREFUSED;
                } else {
                    error = 0;
                }
                break;
            }
        }
        if (error) {
            Close();
            return error;
        }
        ioctlsocket(s, FIONBIO, &off);
        DWORD timeout = 15000;
        setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char const*)&timeout, sizeof(timeout));
        setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, (char const*)&timeout, sizeof(timeout));
        Packet c(kConnect);
        c.Byte(0x10).Byte(0).U16(0x2000).Header(kHTarget, kMasTarget, 16);
        std::vector<uint8_t> reply;
        if (!SendAll(s, c.Done()) || !Receive(s, reply) || reply[0] != kOk || reply.size() < 7) {
            Close();
            return -1;
        }
        most = std::clamp<size_t>((reply[5] << 8) | reply[6], 255, 0x2000);
        Headers(reply, 7, [&](uint8_t h, uint8_t const* d, size_t n) {
            if (h == kHConnectionId && n == 4) id = (d[0] << 24) | (d[1] << 16) | (d[2] << 8) | d[3], haveId = true;
        });
        return 0;
    }

    void Close() {
        if (s == INVALID_SOCKET) return;
        if (haveId) {
            std::vector<uint8_t> reply;
            if (SendAll(s, Packet(kDisconnect).Id(id).Done())) Receive(s, reply);
        }
        g_live = INVALID_SOCKET;
        closesocket(s);
        s = INVALID_SOCKET;
        haveId = false;
    }

    // A request's first packet, with the connection's id first.
    Packet Start(uint8_t op) {
        Packet p(op);
        if (op == kSetPath) p.Byte(0x02).Byte(0);  // (flags: never create a folder; constants)
        if (haveId) p.Id(id);
        return p;
    }

    bool Ask(std::vector<uint8_t> const& p, std::vector<uint8_t>& reply) { return SendAll(s, p) && Receive(s, reply); }

    bool SetPath(std::wstring const& name) {
        std::vector<uint8_t> reply;
        return Ask(Start(kSetPath).Name(name).Done(), reply) && reply[0] == kOk;
    }

    // Into telecom/msg, where inbox, outbox and sent are.
    bool ToMessages() { return SetPath(L"") && SetPath(L"telecom") && SetPath(L"msg"); }

    // A GET, as many packets as the answer takes.
    bool Get(Packet first, std::string& body) {
        std::vector<uint8_t> packet = first.Done(), reply;
        for (int round = 0; round < 4096; round++) {
            if (!Ask(packet, reply)) return false;
            Headers(reply, 3, [&](uint8_t h, uint8_t const* d, size_t n) {
                if (h == kHBody || h == kHEndOfBody) body.append((char const*)d, n);
            });
            if (reply[0] == kOk) return true;
            if (reply[0] != kContinue) return false;
            packet = Packet(kGetFinal).Done();
        }
        return false;
    }

    // A PUT of body, in as many packets as it takes. name: the Name the phone answers with (a sent text's handle).
    // 0 when taken, the phone's answer when refused, -1 when the session broke.
    int Put(Packet first, std::string const& body, std::wstring* name = nullptr) {
        Packet p = first;
        size_t at = 0;
        for (;;) {
            size_t room = most > p.b.size() + 3 ? most - p.b.size() - 3 : 0;
            size_t n = std::min(body.size() - at, room);
            bool last = at + n == body.size();
            p.Header(last ? kHEndOfBody : kHBody, body.data() + at, n);
            if (last) p.b[0] |= 0x80;
            std::vector<uint8_t> reply;
            if (!Ask(p.Done(), reply)) return -1;
            if (last) {
                if (name)
                    Headers(reply, 3, [&](uint8_t h, uint8_t const* d, size_t size) {
                        if (h != kHName) return;
                        for (size_t i = 0; i + 1 < size; i += 2)
                            if (wchar_t c = (wchar_t)((d[i] << 8) | d[i + 1])) *name += c;
                    });
                return reply[0] == kOk ? 0 : reply[0];
            }
            if (reply[0] != kContinue) return reply[0];
            at += n;
            p = Packet(kPut);
        }
    }

    // The phone tells the notification server about new texts (on), or stops (off).
    bool Notices(bool on) {
        return Put(Start(kPut).Type("x-bt/MAP-NotificationRegistration").Params({kPNotificationStatus, 1, (uint8_t)on}), "0") == 0;
    }

    // A folder's listing (XML): its latest, at most most.
    bool Listing(std::wstring const& folder, int count, std::string& xml) {
        return Get(Start(kGetFinal).Type("x-bt/MAP-msg-listing").Name(folder).Params(
                       {kPMaxListCount, 2, (uint8_t)(count >> 8), (uint8_t)count, kPSubjectLength, 1, 255}),
                   xml);
    }

    // One text, as a bMessage (UTF-8, no attachments).
    bool Message(std::wstring const& handle, std::string& bmsg) {
        return Get(Start(kGetFinal).Type("x-bt/message").Name(handle).Params({kPAttachment, 1, 0, kPCharset, 1, 1}), bmsg);
    }

    // A text to send, into the outbox (the phone sends it, and keeps it in Sent).
    int Push(std::string const& bmsg, std::wstring* handle) {
        return Put(Start(kPut).Type("x-bt/message").Name(L"outbox").Params({kPTransparent, 1, 0, kPRetry, 1, 1, kPCharset, 1, 1}),
                   bmsg, handle);
    }
};

////////////////////////////////////////////////////////////////////////////////
// Texts as the phone writes them (bMessage: a vCard for who, then the text) and lists them (XML).

// A vCard or bMessage line's value, unescaped (\, \; \n), and quoted-printable decoded when it says so.
std::string CardValue(std::string const& line) {
    size_t colon = line.find(':');
    if (colon == std::string::npos) return {};
    std::string head = line.substr(0, colon), v = line.substr(colon + 1), out;
    for (auto& c : head) c = (char)toupper((unsigned char)c);
    if (head.find("QUOTED-PRINTABLE") != std::string::npos) {
        for (size_t i = 0; i < v.size(); i++) {
            if (v[i] == '=' && i + 2 < v.size() && isxdigit((unsigned char)v[i + 1]) && isxdigit((unsigned char)v[i + 2])) {
                out += (char)strtol(v.substr(i + 1, 2).c_str(), nullptr, 16);
                i += 2;
            } else {
                out += v[i];
            }
        }
        v = out;
        out.clear();
    }
    for (size_t i = 0; i < v.size(); i++) {
        if (v[i] == '\\' && i + 1 < v.size()) {
            char n = v[++i];
            out += n == 'n' || n == 'N' ? '\n' : n;
        } else {
            out += v[i];
        }
    }
    return out;
}

// A line's name, upper case, without its parameters ("TEL;TYPE=CELL:..." is TEL).
std::string CardName(std::string const& line) {
    std::string name = line.substr(0, std::min(line.find(':'), line.find(';')));
    for (auto& c : name) c = (char)toupper((unsigned char)c);
    return name;
}

struct Parsed {
    std::wstring name, address, body;
    bool ok = false;
};

// Who sent it (the first vCard, the originator) and its text.
Parsed ParseBMessage(std::string const& bmsg) {
    Parsed p;
    std::vector<std::string> lines;
    for (size_t at = 0; at < bmsg.size();) {
        size_t end = bmsg.find('\n', at);
        if (end == std::string::npos) end = bmsg.size();
        std::string line = bmsg.substr(at, end - at);
        if (!line.empty() && line.back() == '\r') line.pop_back();
        lines.push_back(line);
        at = end + 1;
    }
    int cards = 0;
    bool inCard = false, inBody = false, inEnvelope = false;
    std::string fn, n, tel, email, body;
    for (auto const& line : lines) {
        if (inBody) {
            if (line == "END:MSG") {
                inBody = false;
                p.ok = true;
                continue;
            }
            body += (body.empty() ? "" : "\n") + line;
            continue;
        }
        std::string name = CardName(line);
        if (line == "BEGIN:MSG" && !p.ok) inBody = true;
        else if (name == "BEGIN" && line.size() > 6 && !_stricmp(line.c_str() + 6, "VCARD")) inCard = true, cards++;
        else if (name == "END" && line.size() > 4 && !_stricmp(line.c_str() + 4, "VCARD")) inCard = false;
        else if (name == "BEGIN" && line.size() > 6 && !_stricmp(line.c_str() + 6, "BENV")) inEnvelope = true;
        else if (inCard && cards == 1 && !inEnvelope) {
            // (the originator's card comes before the envelope; the recipients' are inside it)
            if (name == "FN") fn = CardValue(line);
            else if (name == "N") n = CardValue(line);
            else if (name == "TEL" && tel.empty()) tel = CardValue(line);
            else if (name == "EMAIL" && email.empty()) email = CardValue(line);
        }
    }
    if (fn.empty() && !n.empty()) {
        // N is Last;First;Middle;Prefix;Suffix.
        std::string last = n.substr(0, n.find(';')), first;
        if (size_t semi = n.find(';'); semi != std::string::npos) first = n.substr(semi + 1, n.find(';', semi + 1) - semi - 1);
        fn = first.empty() ? last : last.empty() ? first : first + " " + last;
    }
    p.name = FromUtf8(fn);
    p.address = FromUtf8(!tel.empty() ? tel : email);
    p.body = FromUtf8(body);
    return p;
}

// A text to send, as a bMessage: to an address (a number, or an email for iMessage).
std::string MakeBMessage(std::wstring const& address, std::wstring const& name, std::wstring const& text) {
    auto escape = [](std::string s) {
        std::string out;
        for (char c : s) {
            if (c == '\r') continue;
            if (c == '\\' || c == ';' || c == ',') out += '\\';
            if (c == '\n') {
                out += "\\n";
                continue;
            }
            out += c;
        }
        return out;
    };
    std::string body;
    for (char c : ToUtf8(text)) {  // (lines end CRLF in a bMessage)
        if (c == '\r') continue;
        if (c == '\n') body += '\r';
        body += c;
    }
    std::string msg = "BEGIN:MSG\r\n" + body + "\r\nEND:MSG\r\n";
    bool email = address.find(L'@') != std::wstring::npos;
    std::string m =
        "BEGIN:BMSG\r\nVERSION:1.0\r\nSTATUS:READ\r\nTYPE:SMS_GSM\r\nFOLDER:TELECOM/MSG/OUTBOX\r\n"
        "BEGIN:BENV\r\nBEGIN:VCARD\r\nVERSION:2.1\r\n";
    m += "N:" + escape(ToUtf8(name)) + "\r\n";
    m += std::string(email ? "EMAIL:" : "TEL:") + ToUtf8(address) + "\r\n";
    m += "END:VCARD\r\nBEGIN:BBODY\r\nCHARSET:UTF-8\r\nLENGTH:" + std::to_string(msg.size()) + "\r\n";
    m += msg + "END:BBODY\r\nEND:BENV\r\nEND:BMSG\r\n";
    return m;
}

// An XML attribute's value in an element (entities decoded).
std::wstring XmlAttribute(std::wstring const& element, std::wstring const& name) {
    for (size_t at = 0; (at = element.find(name + L"=\"", at)) != std::wstring::npos; at++) {
        if (at && !iswspace(element[at - 1])) continue;
        size_t from = at + name.size() + 2, to = element.find(L'"', from);
        if (to == std::wstring::npos) return {};
        std::wstring raw = element.substr(from, to - from), out;
        for (size_t i = 0; i < raw.size(); i++) {
            if (raw[i] != L'&') {
                out += raw[i];
                continue;
            }
            size_t end = raw.find(L';', i);
            if (end == std::wstring::npos) {
                out += raw[i];
                continue;
            }
            std::wstring e = raw.substr(i + 1, end - i - 1);
            if (e == L"amp") out += L'&';
            else if (e == L"lt") out += L'<';
            else if (e == L"gt") out += L'>';
            else if (e == L"quot") out += L'"';
            else if (e == L"apos") out += L'\'';
            else if (!e.empty() && e[0] == L'#') out += (wchar_t)wcstoul(e.c_str() + (e.size() > 1 && e[1] == L'x' ? 2 : 1), nullptr, e.size() > 1 && e[1] == L'x' ? 16 : 10);
            i = end;
        }
        return out;
    }
    return {};
}

// Each element named tag in some XML: its text, from < to >.
std::vector<std::wstring> XmlElements(std::wstring const& xml, std::wstring const& tag) {
    std::vector<std::wstring> out;
    for (size_t at = 0; (at = xml.find(L"<" + tag, at)) != std::wstring::npos;) {
        size_t end = xml.find(L'>', at);
        if (end == std::wstring::npos) break;
        wchar_t after = xml[at + 1 + tag.size()];
        if (iswspace(after) || after == L'/' || after == L'>') out.push_back(xml.substr(at, end - at + 1));
        at = end;
    }
    return out;
}

////////////////////////////////////////////////////////////////////////////////
// The texts kept, and how the link stands.

struct Text {
    long long when = 0;  // FILETIME (UTC)
    std::wstring address, name, body;
    wchar_t kind = L'i';  // i came in; s sent; p sending; f not sent
    bool read = false;
};

enum class Link { Off, NoPhone, PhoneLink, Away, Connecting, Ready };

struct State {
    std::mutex lock;
    std::vector<Text> texts;  // oldest first
    Link link = Link::Off;
    int error = 0;            // the last connect's (Away)
    bool loaded = false;
    HWND listener = nullptr;
    UINT listenerMessage = 0;
    // To send: the texts' places in texts (kind p).
    std::deque<size_t> outbox;
    // Notices from the phone: new texts' handles.
    std::deque<std::wstring> arrived;
    bool dropped = false;     // the phone closed the notification server's connection
    std::wstring sentHandle;  // the last sent text's handle, and its place, for SendingSuccess/Failure
    size_t sentAt = SIZE_MAX;
    bool pushing = false;     // a text is being handed to the phone (its place is held outside the lock: no Trim)
};
State g;
HANDLE g_thread;
constexpr size_t kKept = 500;

long long Now() {
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    return ((long long)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
}

std::wstring StoreFile() {
    WCHAR path[MAX_PATH] = L"";
    if (FAILED(SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, path))) return {};
    std::wstring dir = std::wstring(path) + L"\\Tourne";
    CreateDirectoryW(dir.c_str(), nullptr);
    dir += L"\\Desktop";
    CreateDirectoryW(dir.c_str(), nullptr);
    return dir + L"\\phone-messages.tsv";
}

std::wstring Escape(std::wstring const& s) {
    std::wstring out;
    for (wchar_t c : s) {
        if (c == L'\\') out += L"\\\\";
        else if (c == L'\t') out += L"\\t";
        else if (c == L'\n') out += L"\\n";
        else if (c == L'\r') out += L"\\r";
        else out += c;
    }
    return out;
}

std::wstring Unescape(std::wstring const& s) {
    std::wstring out;
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] != L'\\' || i + 1 >= s.size()) {
            out += s[i];
            continue;
        }
        wchar_t n = s[++i];
        out += n == L't' ? L'\t' : n == L'n' ? L'\n' : n == L'r' ? L'\r' : n;
    }
    return out;
}

// Under g.lock: the oldest let go once there are well over kKept (as the file keeps them), the places of the
// ones waiting to go moved with them.
void Trim() {
    if (g.pushing || g.texts.size() <= kKept + 100) return;
    size_t drop = g.texts.size() - kKept;
    g.texts.erase(g.texts.begin(), g.texts.begin() + drop);
    std::deque<size_t> outbox;
    for (size_t at : g.outbox)
        if (at >= drop) outbox.push_back(at - drop);
    g.outbox = std::move(outbox);
    g.sentAt = g.sentAt != SIZE_MAX && g.sentAt >= drop ? g.sentAt - drop : SIZE_MAX;
}

// Under g.lock.
void Save() {
    std::wstring file = StoreFile();
    if (file.empty()) return;
    std::wstring all;
    size_t from = g.texts.size() > kKept ? g.texts.size() - kKept : 0;
    for (size_t i = from; i < g.texts.size(); i++) {
        auto const& t = g.texts[i];
        wchar_t kind = t.kind == L'p' ? L'f' : t.kind;  // (one still sending when Tourne ends wasn't sent)
        all += std::to_wstring(t.when) + L"\t" + kind + L"\t" + (t.read ? L"1" : L"0") + L"\t" + Escape(t.address) + L"\t" +
               Escape(t.name) + L"\t" + Escape(t.body) + L"\n";
    }
    std::string bytes = ToUtf8(all);
    std::wstring temp = file + L".new";
    HANDLE h = CreateFileW(temp.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) return;
    DWORD wrote = 0;
    bool ok = WriteFile(h, bytes.data(), (DWORD)bytes.size(), &wrote, nullptr) && wrote == bytes.size();
    CloseHandle(h);
    if (ok) MoveFileExW(temp.c_str(), file.c_str(), MOVEFILE_REPLACE_EXISTING);
    else DeleteFileW(temp.c_str());
}

// Under g.lock.
void Load() {
    g.loaded = true;
    std::wstring file = StoreFile();
    HANDLE h = CreateFileW(file.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    if (h == INVALID_HANDLE_VALUE) return;
    std::string bytes;
    LARGE_INTEGER size{};
    if (GetFileSizeEx(h, &size) && size.QuadPart > 0 && size.QuadPart < (16 << 20)) {
        bytes.resize((size_t)size.QuadPart);
        DWORD read = 0;
        if (!ReadFile(h, bytes.data(), (DWORD)bytes.size(), &read, nullptr)) read = 0;
        bytes.resize(read);
    }
    CloseHandle(h);
    std::wstring text = FromUtf8(bytes);
    for (size_t at = 0; at < text.size();) {
        size_t end = text.find(L'\n', at);
        if (end == std::wstring::npos) end = text.size();
        std::vector<std::wstring> f;
        for (size_t p = at; p <= end;) {
            size_t tab = text.find(L'\t', p);
            if (tab == std::wstring::npos || tab > end) tab = end;
            f.push_back(text.substr(p, tab - p));
            p = tab + 1;
        }
        at = end + 1;
        if (f.size() < 6 || f[1].empty()) continue;
        Text t;
        t.when = _wtoi64(f[0].c_str());
        t.kind = f[1][0];
        t.read = f[2] == L"1";
        t.address = Unescape(f[3]);
        t.name = Unescape(f[4]);
        if (t.address.empty()) t.address = !t.name.empty() ? t.name : L"Unknown sender";
        t.body = Unescape(f[5]);
        g.texts.push_back(std::move(t));
    }
}

void Changed() {
    HWND to;
    UINT message;
    {
        std::lock_guard<std::mutex> lock(g.lock);
        to = g.listener;
        message = g.listenerMessage;
    }
    if (to) PostMessageW(to, message, 0, 0);
}

void Log(std::wstring const& text);

void SetLink(Link link, int error = 0) {
    {
        std::lock_guard<std::mutex> lock(g.lock);
        if (g.link == link && g.error == error) return;
        g.link = link;
        g.error = error;
    }
    static PCWSTR const names[] = {L"off", L"no paired phone", L"Phone Link has them", L"not connected", L"connecting", L"connected"};
    Log(std::wstring(L"texts: ") + names[(int)link] + (error ? L" (" + std::to_wstring(error) + L")" : L""));
    Changed();
}

// The same correspondent however the number is written ("+1 (555) 010-0199" and "+15550100199").
std::wstring Key(std::wstring const& address) {
    if (address.find(L'@') != std::wstring::npos) return PhoneLower(address);
    std::wstring digits;
    for (wchar_t c : address)
        if (iswdigit(c)) digits += c;
    return digits.size() > 10 ? digits.substr(digits.size() - 10) : digits.empty() ? address : digits;
}

////////////////////////////////////////////////////////////////////////////////
// The notification server: the phone connects to it once asked to (Session::Notices) and tells of new texts.

struct Notices {
    SOCKET listener = INVALID_SOCKET;
    std::atomic<SOCKET> client{INVALID_SOCKET};
    HANDLE thread = nullptr;
    std::vector<uint8_t> record;  // its SDP record, as registered (BTH_SET_SERVICE)
    HANDLE recordHandle = nullptr;
    ULONG sdpVersion = BTH_SDP_VERSION;
    bool registered = false;

    // Its SDP record: MAP's notification server (0x1133) on an RFCOMM channel, over OBEX, MAP 1.0.
    static std::vector<uint8_t> Record(uint8_t channel) {
        auto seq = [](std::vector<uint8_t> inner) {
            inner.insert(inner.begin(), {0x35, (uint8_t)inner.size()});
            return inner;
        };
        auto cat = [](std::initializer_list<std::vector<uint8_t>> parts) {
            std::vector<uint8_t> out;
            for (auto const& p : parts) out.insert(out.end(), p.begin(), p.end());
            return out;
        };
        auto id = [](uint16_t v) { return std::vector<uint8_t>{0x09, (uint8_t)(v >> 8), (uint8_t)v}; };
        auto uuid = [](uint16_t v) { return std::vector<uint8_t>{0x19, (uint8_t)(v >> 8), (uint8_t)v}; };
        std::string name = "MAP MNS-Tourne";
        std::vector<uint8_t> text{0x25, (uint8_t)name.size()};
        text.insert(text.end(), name.begin(), name.end());
        return seq(cat({
            id(0x0001), seq(uuid(0x1133)),
            id(0x0004), seq(cat({seq(uuid(0x0100)), seq(cat({uuid(0x0003), {0x08, channel}})), seq(uuid(0x0008))})),
            id(0x0005), seq(uuid(0x1002)),
            id(0x0009), seq(seq(cat({uuid(0x1134), id(0x0100)}))),
            id(0x0100), text,
        }));
    }

    bool Start() {
        listener = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
        if (listener == INVALID_SOCKET) return false;
        SOCKADDR_BTH a{};
        a.addressFamily = AF_BTH;
        a.port = BT_PORT_ANY;
        int size = sizeof(a);
        if (bind(listener, (sockaddr*)&a, sizeof(a)) || listen(listener, 1) || getsockname(listener, (sockaddr*)&a, &size)) {
            Stop();
            return false;
        }
        std::vector<uint8_t> sdp = Record((uint8_t)a.port);
        record.assign(sizeof(BTH_SET_SERVICE) + sdp.size(), 0);
        auto* set = (BTH_SET_SERVICE*)record.data();
        set->pSdpVersion = &sdpVersion;
        set->pRecordHandle = &recordHandle;
        set->ulRecordLength = (ULONG)sdp.size();
        memcpy(set->pRecord, sdp.data(), sdp.size());
        BLOB blob{(ULONG)record.size(), record.data()};
        WSAQUERYSETW q{};
        q.dwSize = sizeof(q);
        q.dwNameSpace = NS_BTH;
        q.lpBlob = &blob;
        registered = WSASetServiceW(&q, RNRSERVICE_REGISTER, 0) == 0;
        if (!registered) {
            Stop();
            return false;
        }
        thread = CreateThread(nullptr, 0, Answer, this, 0, nullptr);
        return thread != nullptr;
    }

    void Stop() {
        if (registered) {
            BLOB blob{(ULONG)record.size(), record.data()};
            WSAQUERYSETW q{};
            q.dwSize = sizeof(q);
            q.dwNameSpace = NS_BTH;
            q.lpBlob = &blob;
            WSASetServiceW(&q, RNRSERVICE_DELETE, 0);
            registered = false;
        }
        if (listener != INVALID_SOCKET) closesocket(listener), listener = INVALID_SOCKET;
        if (SOCKET c = client.exchange(INVALID_SOCKET); c != INVALID_SOCKET) closesocket(c);
        if (thread) {
            WaitForSingleObject(thread, 5000);
            CloseHandle(thread);
            thread = nullptr;
        }
    }

    // An event report: new texts in the inbox, and whether the last one sent went.
    static void Report(std::string const& body) {
        std::wstring xml = FromUtf8(body);
        bool any = false;
        std::lock_guard<std::mutex> lock(g.lock);
        for (auto const& e : XmlElements(xml, L"event")) {
            std::wstring type = XmlAttribute(e, L"type"), handle = XmlAttribute(e, L"handle"), folder = PhoneLower(XmlAttribute(e, L"folder"));
            if (type == L"NewMessage" && !handle.empty() && folder.find(L"inbox") != std::wstring::npos) {
                g.arrived.push_back(handle);
                any = true;
            } else if ((type == L"SendingSuccess" || type == L"SendingFailure") && handle == g.sentHandle && g.sentAt < g.texts.size()) {
                g.texts[g.sentAt].kind = type == L"SendingSuccess" ? L's' : L'f';
                Save();
                any = true;
            }
        }
        if (any) SetEvent(g_noticeEvent);
    }

    // One connection from the phone, answered as an OBEX server: connect, its PUTs, disconnect.
    void Serve(SOCKET c) {
        std::vector<uint8_t> p;
        std::string body;
        uint32_t connection = 1;
        while (Receive(c, p)) {
            uint8_t op = p[0];
            std::vector<uint8_t> answer;
            if (op == kConnect) {
                answer = Packet(kOk).Byte(0x10).Byte(0).U16(0x2000).Id(connection).Header(kHWho, kMnsTarget, 16).Done();
            } else if (op == kPut || op == kPutFinal) {
                Headers(p, 3, [&](uint8_t h, uint8_t const* d, size_t n) {
                    if (h == kHBody || h == kHEndOfBody) body.append((char const*)d, n);
                });
                if (op == kPutFinal) {
                    Report(body);
                    body.clear();
                }
                answer = Packet(op == kPutFinal ? kOk : kContinue).Done();
            } else if (op == kDisconnect) {
                SendAll(c, Packet(kOk).Done());
                return;
            } else {
                answer = Packet(kBadRequest).Done();
            }
            if (!SendAll(c, answer)) return;
        }
    }

    static DWORD WINAPI Answer(LPVOID param) {
        auto* self = (Notices*)param;
        for (;;) {
            SOCKET c = accept(self->listener, nullptr, nullptr);
            if (c == INVALID_SOCKET) return 0;  // (stopped)
            self->client = c;
            self->Serve(c);
            if (self->client.exchange(INVALID_SOCKET) == INVALID_SOCKET) return 0;  // (stopped while serving)
            closesocket(c);
            {
                std::lock_guard<std::mutex> lock(g.lock);
                g.dropped = true;  // the phone went (or stopped telling): the session is checked again
            }
            SetEvent(g_noticeEvent);
        }
    }
};

////////////////////////////////////////////////////////////////////////////////
// The session's thread.

// The phone's classic Bluetooth address, from its device by name (BTHENUM\DEV_<address>); 0 when not paired.
BTH_ADDR ClassicAddress(std::wstring const& name) {
    if (name.empty()) return 0;
    using GetListSize = CONFIGRET(WINAPI*)(PULONG, PCWSTR, ULONG);
    using GetList = CONFIGRET(WINAPI*)(PCWSTR, PZZWSTR, ULONG, ULONG);
    using Locate = CONFIGRET(WINAPI*)(PDEVINST, DEVINSTID_W, ULONG);
    using GetProperty = CONFIGRET(WINAPI*)(DEVINST, const DEVPROPKEY*, DEVPROPTYPE*, PBYTE, PULONG, ULONG);
    static HMODULE lib = LoadLibraryExW(L"cfgmgr32.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (!lib) return 0;
    auto listSize = (GetListSize)(void*)GetProcAddress(lib, "CM_Get_Device_ID_List_SizeW");
    auto list = (GetList)(void*)GetProcAddress(lib, "CM_Get_Device_ID_ListW");
    auto locate = (Locate)(void*)GetProcAddress(lib, "CM_Locate_DevNodeW");
    auto property = (GetProperty)(void*)GetProcAddress(lib, "CM_Get_DevNode_PropertyW");
    if (!listSize || !list || !locate || !property) return 0;
    constexpr DEVPROPKEY kName{{0xa45c254e, 0xdf1c, 0x4efd, {0x80, 0x20, 0x67, 0xd1, 0x46, 0xa8, 0x50, 0xe0}}, 14};
    ULONG size = 0;
    if (listSize(&size, L"BTHENUM", CM_GETIDLIST_FILTER_ENUMERATOR) != CR_SUCCESS || !size) return 0;
    std::vector<WCHAR> ids(size);
    if (list(L"BTHENUM", ids.data(), size, CM_GETIDLIST_FILTER_ENUMERATOR) != CR_SUCCESS) return 0;
    for (PCWSTR id = ids.data(); *id; id += wcslen(id) + 1) {
        if (_wcsnicmp(id, L"BTHENUM\\DEV_", 12) || wcslen(id) < 24) continue;
        DEVINST node = 0;
        if (locate(&node, (DEVINSTID_W)id, CM_LOCATE_DEVNODE_PHANTOM) != CR_SUCCESS) continue;
        WCHAR found[256] = L"";
        ULONG bytes = sizeof(found);
        DEVPROPTYPE type = 0;
        if (property(node, &kName, &type, (PBYTE)found, &bytes, 0) != CR_SUCCESS || name != found) continue;
        return wcstoull(std::wstring(id + 12, 12).c_str(), nullptr, 16);
    }
    return 0;
}

// Phone Link, while it runs (it holds the phone's message channel then): a handle to wait on.
HANDLE PhoneLinkRunning() {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return nullptr;
    PROCESSENTRY32W e{sizeof(e)};
    HANDLE found = nullptr;
    for (BOOL ok = Process32FirstW(snap, &e); ok && !found; ok = Process32NextW(snap, &e))
        if (!_wcsicmp(e.szExeFile, L"PhoneExperienceHost.exe")) found = OpenProcess(SYNCHRONIZE, FALSE, e.th32ProcessID);
    CloseHandle(snap);
    return found;
}

// Waits for stop (true), or wake, or ms.
bool Pause(DWORD ms, HANDLE also = nullptr) {
    HANDLE w[3] = {g_stopEvent, g_wakeEvent, also};
    return WaitForMultipleObjects(also ? 3 : 2, w, FALSE, ms) == WAIT_OBJECT_0;
}

void Log(std::wstring const& text) {
#ifdef TD_PREFIX
    Wh_Log(L"phone: %s", text.c_str());
#else
    wprintf(L"%ls\n", text.c_str());
#endif
}

// A new text, read and kept.
bool Fetch(Session& mas, std::wstring const& handle) {
    std::string bmsg;
    if (!mas.Message(handle, bmsg)) return false;
    Parsed p = ParseBMessage(bmsg);
    if (!p.ok) return true;  // (not one with text: a picture alone)
    Text t;
    t.when = Now();
    // (one whose sender has no number or email is kept under their name, so its conversation still opens)
    t.address = !p.address.empty() ? p.address : !p.name.empty() ? p.name : L"Unknown sender";
    t.name = p.name;
    t.body = p.body;
    {
        std::lock_guard<std::mutex> lock(g.lock);
        g.texts.push_back(std::move(t));
        Trim();
        Save();
    }
    Changed();
    return true;
}

// The texts waiting to go, sent; false when the session broke.
bool SendQueued(Session& mas) {
    for (;;) {
        std::string bmsg;
        size_t at;
        {
            std::lock_guard<std::mutex> lock(g.lock);
            if (g.outbox.empty()) return true;
            at = g.outbox.front();
            g.outbox.pop_front();
            if (at >= g.texts.size()) continue;
            auto const& t = g.texts[at];
            bmsg = MakeBMessage(t.address, t.name, t.body);
            g.pushing = true;
        }
        std::wstring handle;
        int result = mas.Push(bmsg, &handle);
        {
            std::lock_guard<std::mutex> lock(g.lock);
            // Taken: sent (the phone may still say SendingFailure for it); refused: not sent.
            g.pushing = false;
            g.texts[at].kind = result == 0 ? L's' : L'f';
            g.sentHandle = handle;
            g.sentAt = at;
            Save();
        }
        Log(result == 0 ? L"texts: sent" : L"texts: not sent (" + std::to_wstring(result) + L")");
        Changed();
        if (result < 0) return false;
    }
}

// The texts still waiting when there's no session: not sent.
void FailQueued() {
    bool any = false;
    {
        std::lock_guard<std::mutex> lock(g.lock);
        for (size_t at : g.outbox)
            if (at < g.texts.size()) g.texts[at].kind = L'f', any = true;
        g.outbox.clear();
        if (any) Save();
    }
    if (any) Changed();
}

DWORD WINAPI Run(LPVOID) {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
    int misses = 0;
    while (WaitForSingleObject(g_stopEvent, 0) != WAIT_OBJECT_0) {
        PhoneInfo who;
        ReadPhoneIdentity(who);
        BTH_ADDR address = who.linked ? ClassicAddress(who.name) : 0;
        if (!address) {
            SetLink(Link::NoPhone);
            FailQueued();
            if (Pause(5 * 60000)) break;
            continue;
        }
        if (HANDLE phoneLink = PhoneLinkRunning()) {
            SetLink(Link::PhoneLink);
            FailQueued();
            HANDLE w[2] = {g_stopEvent, phoneLink};
            DWORD r = WaitForMultipleObjects(2, w, FALSE, 10 * 60000);
            CloseHandle(phoneLink);
            if (r == WAIT_OBJECT_0) break;
            if (Pause(3000)) break;  // (Phone Link let go of the channel just now)
            continue;
        }
        SetLink(Link::Connecting);
        Session mas;
        int error = mas.Open(address);
        if (!error && !mas.ToMessages()) {
            mas.Close();
            error = -1;
        }
        if (error) {
            SetLink(Link::Away, error);
            FailQueued();

            DWORD wait = misses == 0 ? 60000 : misses == 1 ? 120000 : 300000;
            misses++;
            if (Pause(wait)) break;
            continue;
        }
        misses = 0;
        {
            std::lock_guard<std::mutex> lock(g.lock);
            g.arrived.clear();
            g.dropped = false;
        }
        ResetEvent(g_noticeEvent);
        Notices notices;
        bool told = notices.Start() && mas.Notices(true);
        Log(told ? L"texts: the phone tells of new ones" : L"texts: asking for new ones every 20 seconds");
        SetLink(Link::Ready);
        // Without notices: the inbox looked at now and then, its new handles fetched.
        std::set<std::wstring> seen;
        auto look = [&](bool fetch) {
            std::string xml;
            if (!mas.Listing(L"inbox", 50, xml)) return false;
            for (auto const& e : XmlElements(FromUtf8(xml), L"msg")) {
                std::wstring handle = XmlAttribute(e, L"handle");
                if (handle.empty() || !seen.insert(handle).second || !fetch) continue;
                if (!Fetch(mas, handle)) return false;
            }
            return true;
        };
        bool alive = told || look(false);
        while (alive) {
            alive = SendQueued(mas);
            if (!alive) break;
            HANDLE w[3] = {g_stopEvent, g_wakeEvent, g_noticeEvent};
            DWORD r = WaitForMultipleObjects(3, w, FALSE, told ? INFINITE : 20000);
            if (r == WAIT_OBJECT_0) break;
            if (r == WAIT_TIMEOUT) {
                alive = look(true);
                continue;
            }
            std::deque<std::wstring> arrived;
            bool dropped;
            {
                std::lock_guard<std::mutex> lock(g.lock);
                arrived.swap(g.arrived);
                dropped = g.dropped;
            }
            for (auto const& handle : arrived)
                if (!(alive = Fetch(mas, handle))) break;
            if (dropped) alive = false;
        }
        bool stopping = WaitForSingleObject(g_stopEvent, 0) == WAIT_OBJECT_0;
        if (told && alive) mas.Notices(false);
        notices.Stop();
        mas.Close();
        Log(stopping ? L"texts: closed" : L"texts: the phone went");
        SetLink(Link::Away);
        if (stopping || Pause(5000)) break;
    }
    FailQueued();
    SetLink(Link::Off);
    WSACleanup();
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// For the phone pane.

// New texts as notifications of their own ("Messages"), so they pop up and wait in the notification centre as
// any app's do (Tourne's or Windows', as chosen).
constexpr wchar_t kAumid[] = L"Tourne.PhoneTexts";

void NameForNotifications() {
    std::wstring key = std::wstring(L"Software\\Classes\\AppUserModelId\\") + kAumid;
    RegSetKeyValueW(HKEY_CURRENT_USER, key.c_str(), L"DisplayName", REG_SZ, L"Messages", sizeof(L"Messages"));
    // Its place in Windows' notification settings made before the first, so a Tourne that draws the pop-ups
    // itself turns Windows' off for it (desktop/host/notifications.cpp) before one shows twice.
    HKEY settings;
    std::wstring at = std::wstring(L"Software\\Microsoft\\Windows\\CurrentVersion\\Notifications\\Settings\\") + kAumid;
    if (RegCreateKeyExW(HKEY_CURRENT_USER, at.c_str(), 0, nullptr, 0, KEY_READ, nullptr, &settings, nullptr) == ERROR_SUCCESS)
        RegCloseKey(settings);
}

std::wstring XmlEscape(std::wstring const& s) {
    std::wstring out;
    for (wchar_t c : s) {
        if (c == L'&') out += L"&amp;";
        else if (c == L'<') out += L"&lt;";
        else if (c == L'>') out += L"&gt;";
        else if (c == L'"') out += L"&quot;";
        else if (c >= 0x20 || c == L'\n') out += c;
    }
    return out;
}

void Toast(std::wstring const& who, std::wstring const& text) {
    try {
        winrt::Windows::Data::Xml::Dom::XmlDocument doc;
        doc.LoadXml(L"<toast><visual><binding template=\"ToastGeneric\"><text>" + XmlEscape(who) + L"</text><text>" +
                    XmlEscape(text) + L"</text></binding></visual><audio src=\"ms-winsoundevent:Notification.SMS\"/></toast>");
        winrt::Windows::UI::Notifications::ToastNotificationManager::CreateToastNotifier(kAumid).Show(
            winrt::Windows::UI::Notifications::ToastNotification(doc));
    } catch (...) {
        Log(L"couldn't show a new text's notification");
    }
}

// Starts holding the phone's texts (once); changes are posted to listener as message.
void Begin(HWND listener, UINT message) {
    {
        std::lock_guard<std::mutex> lock(g.lock);
        g.listener = listener;
        g.listenerMessage = message;
        if (!g.loaded) Load();
    }
    if (g_thread) return;
    NameForNotifications();
    g_stopEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    g_wakeEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    g_noticeEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    g_thread = CreateThread(nullptr, 0, Run, nullptr, 0, nullptr);
    if (g_thread) SetThreadPriority(g_thread, THREAD_PRIORITY_BELOW_NORMAL);
}

void End() {
    if (!g_thread) return;
    SetEvent(g_stopEvent);
    if (SOCKET s = g_live.load(); s != INVALID_SOCKET) shutdown(s, SD_BOTH);  // (a wait on the phone ends now)
    bool ended = WaitForSingleObject(g_thread, 10000) == WAIT_OBJECT_0;
    CloseHandle(g_thread);
    g_thread = nullptr;
    if (ended)  // (else it still has them: left open)
        for (HANDLE* h : {&g_stopEvent, &g_wakeEvent, &g_noticeEvent}) CloseHandle(*h), *h = nullptr;
    std::lock_guard<std::mutex> lock(g.lock);
    g.listener = nullptr;
}

bool Running() { return g_thread != nullptr; }

// Try connecting now (when the pane is opened and the phone may be back).
void Wake() {
    if (g_wakeEvent) SetEvent(g_wakeEvent);
}

Link LinkNow(int* error = nullptr) {
    std::lock_guard<std::mutex> lock(g.lock);
    if (error) *error = g.error;
    return g.link;
}

std::vector<Text> TextsNow() {
    std::lock_guard<std::mutex> lock(g.lock);
    return g.texts;
}

// A reply, queued: sent as soon as the session has it (shown as sending till then).
bool Send(std::wstring const& address, std::wstring const& name, std::wstring const& body) {
    if (address.empty() || body.empty() || !g_thread) return false;
    {
        std::lock_guard<std::mutex> lock(g.lock);
        Text t;
        t.when = Now();
        t.address = address;
        t.name = name;
        t.body = body;
        t.kind = g.link == Link::Ready ? L'p' : L'f';
        t.read = true;
        g.texts.push_back(t);
        if (t.kind == L'p') g.outbox.push_back(g.texts.size() - 1);
        Trim();
        Save();
    }
    Changed();
    Wake();
    return true;
}

// A conversation read: its texts aren't new any more.
void MarkRead(std::wstring const& key) {
    bool any = false;
    {
        std::lock_guard<std::mutex> lock(g.lock);
        for (auto& t : g.texts)
            if (!t.read && Key(t.address) == key) t.read = true, any = true;
        if (any) Save();
    }
    if (any) Changed();
}

}  // namespace phonemap
#else
// Stand-ins where Tourne doesn't hold the phone's texts (the suite's build, in Explorer): Phone Link has them.
namespace phonemap {
struct Text {
    long long when = 0;
    std::wstring address, name, body;
    wchar_t kind = L'i';
    bool read = false;
};
enum class Link { Off, NoPhone, PhoneLink, Away, Connecting, Ready };
inline void Begin(HWND, UINT) {}
inline void End() {}
inline void Wake() {}
inline Link LinkNow(int* = nullptr) { return Link::Off; }
inline std::vector<Text> TextsNow() { return {}; }
inline bool Send(std::wstring const&, std::wstring const&, std::wstring const&) { return false; }
inline void MarkRead(std::wstring const&) {}
inline std::wstring Key(std::wstring const& address) { return address; }
inline void Toast(std::wstring const&, std::wstring const&) {}
}  // namespace phonemap
#endif
