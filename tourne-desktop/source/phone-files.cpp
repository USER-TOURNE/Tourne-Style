////////////////////////////////////////////////////////////////////////////////
// Files to your phone over your own network, as LocalSend sends them ("// @inline phone-files.cpp" in taskbar-host.cpp
// after phone-ancs.cpp, Tourne Desktop only). An iPhone takes no files over Bluetooth, and Phone Link's way is
// Microsoft's own; LocalSend is free and open (its app on the phone, https://localsend.org), and needs no
// account or cloud: the PC and the phone talk directly on the same Wi-Fi.
//
// The phone is found by asking each address on this PC's networks for LocalSend's info (outgoing connections
// only, so no firewall prompt), the last one it was at first. Then LocalSend's upload: "prepare-upload" (the phone
// asks you to accept), and each file streamed. How it went comes back as a notification ("Phone files").
// LocalSend's own protocol, v2: https://github.com/localsend/protocol

#if defined(TD_PREFIX) || defined(PHONE_FILES_PROBE)
#include <wininet.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <functional>
#include <thread>

namespace phonefiles {

constexpr INTERNET_PORT kPort = 53317;
constexpr wchar_t kAumid[] = L"Tourne.PhoneFiles";
constexpr wchar_t kKey[] = L"Software\\Tourne\\StartMenu";

struct Device {
    std::wstring ip;
    bool secure = true;  // https (LocalSend's default), else http
    std::string alias, model, type, fingerprint;
};

std::string JsonEscape(std::string const& s) {
    std::string out;
    for (unsigned char c : s) {
        if (c == '"' || c == '\\') out += '\\', out += (char)c;
        else if (c < 0x20) {
            char hex[8];
            snprintf(hex, sizeof(hex), "\\u%04x", c);
            out += hex;
        } else out += (char)c;
    }
    return out;
}

// A string value's text in some JSON, by its key (the first one); empty when it isn't there.
std::string JsonText(std::string const& json, std::string const& key) {
    size_t k = json.find("\"" + key + "\"");
    if (k == std::string::npos) return {};
    size_t colon = json.find(':', k + key.size() + 2);
    size_t q = colon == std::string::npos ? std::string::npos : json.find_first_not_of(" \t\r\n", colon + 1);
    if (q == std::string::npos || json[q] != '"') return {};
    std::string out;
    for (size_t i = q + 1; i < json.size() && json[i] != '"'; i++) {
        if (json[i] == '\\' && i + 1 < json.size()) {
            char n = json[++i];
            if (n == 'u' && i + 4 < json.size()) {
                std::wstring w(1, (wchar_t)strtoul(json.substr(i + 1, 4).c_str(), nullptr, 16));
                out += phonemap::ToUtf8(w);
                i += 4;
            } else {
                out += n == 'n' ? '\n' : n == 't' ? '\t' : n;
            }
        } else {
            out += json[i];
        }
    }
    return out;
}

// This PC as LocalSend names devices.
std::string Me() {
    static std::string me = [] {
        WCHAR name[MAX_COMPUTERNAME_LENGTH + 1] = L"";
        DWORD size = ARRAYSIZE(name);
        GetComputerNameW(name, &size);
        // (a fingerprint of its own, random: Tourne only sends, so there's no certificate for it to stand for)
        std::string fingerprint;
        for (int i = 0; i < 2; i++) {
            GUID g{};
            CoCreateGuid(&g);
            for (size_t b = 0; b < sizeof(g); b++) {
                char hex[3];
                snprintf(hex, sizeof(hex), "%02x", ((unsigned char*)&g)[b]);
                fingerprint += hex;
            }
        }
        return "{\"alias\":\"" + JsonEscape(phonemap::ToUtf8(std::wstring(L"Tourne on ") + name)) +
               "\",\"version\":\"2.1\",\"deviceModel\":\"Windows\",\"deviceType\":\"desktop\",\"fingerprint\":\"" + fingerprint +
               "\",\"port\":53317,\"protocol\":\"https\",\"download\":false}";
    }();
    return me;
}

// One request to a device: status (0 when it didn't answer) and the answer's body. body is sent as is; or, with
// file, that file streamed (its size is known).
struct Reply {
    DWORD status = 0;
    std::string body;
};

Reply Request(HINTERNET session, Device const& d, PCWSTR verb, std::wstring const& path, std::string const& body, int waitMs,
              HANDLE file = INVALID_HANDLE_VALUE, ULONGLONG fileSize = 0) {
    Reply r;
    HINTERNET connect = InternetConnectW(session, d.ip.c_str(), kPort, nullptr, nullptr, INTERNET_SERVICE_HTTP, 0, 0);
    if (!connect) return r;
    DWORD flags = INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_UI | INTERNET_FLAG_NO_COOKIES;
    if (d.secure) flags |= INTERNET_FLAG_SECURE | INTERNET_FLAG_IGNORE_CERT_CN_INVALID | INTERNET_FLAG_IGNORE_CERT_DATE_INVALID;
    HINTERNET request = HttpOpenRequestW(connect, verb, path.c_str(), nullptr, nullptr, nullptr, flags, 0);
    if (request) {
        DWORD connectMs = 1500, wait = (DWORD)waitMs;
        InternetSetOptionW(request, INTERNET_OPTION_CONNECT_TIMEOUT, &connectMs, sizeof(connectMs));
        InternetSetOptionW(request, INTERNET_OPTION_SEND_TIMEOUT, &wait, sizeof(wait));
        InternetSetOptionW(request, INTERNET_OPTION_RECEIVE_TIMEOUT, &wait, sizeof(wait));
        if (d.secure) {
            // LocalSend's certificates are its own (self-signed): what stands for the phone is its fingerprint.
            DWORD security = 0, size = sizeof(security);
            InternetQueryOptionW(request, INTERNET_OPTION_SECURITY_FLAGS, &security, &size);
            security |= SECURITY_FLAG_IGNORE_UNKNOWN_CA | SECURITY_FLAG_IGNORE_REVOCATION | SECURITY_FLAG_IGNORE_WRONG_USAGE |
                        SECURITY_FLAG_IGNORE_CERT_CN_INVALID | SECURITY_FLAG_IGNORE_CERT_DATE_INVALID;
            InternetSetOptionW(request, INTERNET_OPTION_SECURITY_FLAGS, &security, sizeof(security));
        }
        bool sent;
        if (file != INVALID_HANDLE_VALUE) {
            // A file, streamed in pieces.
            std::wstring headers = L"Content-Type: application/octet-stream\r\n";
            INTERNET_BUFFERSW buffers{};
            buffers.dwStructSize = sizeof(buffers);
            buffers.lpcszHeader = headers.c_str();
            buffers.dwHeadersLength = (DWORD)headers.size();
            buffers.dwBufferTotal = (DWORD)fileSize;
            sent = HttpSendRequestExW(request, &buffers, nullptr, 0, 0);
            std::vector<char> chunk(1 << 16);
            for (ULONGLONG done = 0; sent && done < fileSize;) {
                DWORD read = 0, wrote = 0;
                if (!ReadFile(file, chunk.data(), (DWORD)chunk.size(), &read, nullptr) || !read) {
                    sent = false;
                    break;
                }
                sent = InternetWriteFile(request, chunk.data(), read, &wrote) && wrote == read;
                done += read;
            }
            if (sent) sent = HttpEndRequestW(request, nullptr, 0, 0);
        } else {
            std::wstring headers = L"Content-Type: application/json\r\n";
            sent = HttpSendRequestW(request, headers.c_str(), (DWORD)headers.size(), body.empty() ? nullptr : (LPVOID)body.data(), (DWORD)body.size());
        }
        if (sent) {
            DWORD status = 0, size = sizeof(status);
            HttpQueryInfoW(request, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &status, &size, nullptr);
            r.status = status;
            char part[4096];
            for (DWORD read = 0; InternetReadFile(request, part, sizeof(part), &read) && read && r.body.size() < (1 << 20);) r.body.append(part, read);
        }
        InternetCloseHandle(request);
    }
    InternetCloseHandle(connect);
    return r;
}
// Whether a LocalSend device answers at ip (https, else http): what it says it is.
bool Ask(HINTERNET session, std::wstring const& ip, Device& out) {
    for (bool secure : {true, false}) {
        Device d;
        d.ip = ip;
        d.secure = secure;
        Reply r = Request(session, d, L"GET", L"/api/localsend/v2/info", {}, 1500);
        if (r.status != 200) continue;
        d.alias = JsonText(r.body, "alias");
        d.model = JsonText(r.body, "deviceModel");
        d.type = JsonText(r.body, "deviceType");
        d.fingerprint = JsonText(r.body, "fingerprint");
        if (d.fingerprint.empty() && d.alias.empty()) continue;
        out = d;
        return true;
    }
    return false;
}

// The addresses to ask: each one on this PC's networks (at most a /24 around it).
std::vector<std::wstring> Neighbours() {
    std::vector<std::wstring> out;
    ULONG size = 16 << 10;
    std::vector<BYTE> buffer(size);
    auto* list = (IP_ADAPTER_ADDRESSES*)buffer.data();
    ULONG flags = GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER;
    if (GetAdaptersAddresses(AF_INET, flags, nullptr, list, &size) == ERROR_BUFFER_OVERFLOW) {
        buffer.resize(size);
        list = (IP_ADAPTER_ADDRESSES*)buffer.data();
    }
    if (GetAdaptersAddresses(AF_INET, flags, nullptr, list, &size) != NO_ERROR) return out;
    for (auto* a = list; a; a = a->Next) {
        if (a->OperStatus != IfOperStatusUp || a->IfType == IF_TYPE_SOFTWARE_LOOPBACK || a->IfType == IF_TYPE_TUNNEL) continue;
        for (auto* u = a->FirstUnicastAddress; u; u = u->Next) {
            auto* in = (sockaddr_in*)u->Address.lpSockaddr;
            uint32_t mine = ntohl(in->sin_addr.s_addr);
            if ((mine >> 16) == 0xA9FE) continue;  // (169.254: no network)
            uint32_t base = mine & 0xFFFFFF00;
            for (uint32_t host = 1; host < 255; host++) {
                uint32_t ip = base | host;
                if (ip == mine) continue;
                wchar_t text[32];
                swprintf(text, 32, L"%u.%u.%u.%u", ip >> 24, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, ip & 0xFF);
                out.push_back(text);
            }
        }
    }
    return out;
}

// Of these addresses, the ones taking a connection on LocalSend's port: all tried at once, for about a second.
std::vector<std::wstring> Listening(std::vector<std::wstring> const& ips, int ms) {
    std::vector<std::wstring> out;
    std::vector<WSAPOLLFD> polls;
    std::vector<size_t> which;
    for (size_t i = 0; i < ips.size(); i++) {
        sockaddr_in a{};
        a.sin_family = AF_INET;
        a.sin_port = htons(kPort);
        if (InetPtonW(AF_INET, ips[i].c_str(), &a.sin_addr) != 1) continue;
        SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (s == INVALID_SOCKET) continue;
        u_long on = 1;
        ioctlsocket(s, FIONBIO, &on);
        connect(s, (sockaddr*)&a, sizeof(a));
        polls.push_back({s, POLLWRNORM, 0});
        which.push_back(i);
    }
    for (DWORD until = GetTickCount() + ms;;) {
        int left = (int)(until - GetTickCount());
        if (left <= 0 || WSAPoll(polls.data(), (ULONG)polls.size(), left) <= 0) break;
        bool waiting = false;
        for (size_t i = 0; i < polls.size(); i++) {
            if (polls[i].fd == INVALID_SOCKET) continue;
            if (polls[i].revents & POLLWRNORM) out.push_back(ips[which[i]]);
            if (polls[i].revents) {
                closesocket(polls[i].fd);
                polls[i].fd = INVALID_SOCKET;  // (left out of the next poll)
            } else {
                waiting = true;
            }
        }
        if (!waiting) break;
    }
    for (auto& p : polls)
        if (p.fd != INVALID_SOCKET) closesocket(p.fd);
    return out;
}

// Every LocalSend device answering on this PC's networks; the one at first (the phone's last address) alone
// first, and if it's a phone that's enough.
std::vector<Device> Find(HINTERNET session, std::wstring const& first) {
    std::vector<Device> found;
    Device d;
    if (!first.empty() && Ask(session, first, d)) {
        found.push_back(d);
        if (d.type == "mobile") return found;
    }
    std::vector<std::wstring> all = Listening(Neighbours(), 1200);
    std::mutex lock;
    std::atomic<size_t> next{0};
    std::vector<std::thread> workers;
    for (int i = 0; i < (int)std::min<size_t>(all.size(), 16); i++)
        workers.emplace_back([&] {
            for (size_t at; (at = next++) < all.size();) {
                if (all[at] == first) continue;
                Device x;
                if (!Ask(session, all[at], x)) continue;
                std::lock_guard<std::mutex> guard(lock);
                found.push_back(x);
            }
        });
    for (auto& w : workers) w.join();
    return found;
}

std::wstring LastAddress() {
    WCHAR ip[64] = L"";
    DWORD size = sizeof(ip);
    if (RegGetValueW(HKEY_CURRENT_USER, kKey, L"PhoneLocalSend", RRF_RT_REG_SZ, nullptr, ip, &size) != ERROR_SUCCESS) return {};
    return ip;
}

void Tell(std::wstring const& title, std::wstring const& text) {
#ifdef PHONE_FILES_PROBE
    wprintf(L"%ls: %ls\n", title.c_str(), text.c_str());
#else
    static bool named = false;
    if (!named) {
        named = true;
        std::wstring key = std::wstring(L"Software\\Classes\\AppUserModelId\\") + kAumid;
        RegSetKeyValueW(HKEY_CURRENT_USER, key.c_str(), L"DisplayName", REG_SZ, L"Phone files", sizeof(L"Phone files"));
    }
    try {
        winrt::Windows::Data::Xml::Dom::XmlDocument doc;
        doc.LoadXml(L"<toast><visual><binding template=\"ToastGeneric\"><text>" + phonemap::XmlEscape(title) + L"</text><text>" +
                    phonemap::XmlEscape(text) + L"</text></binding></visual></toast>");
        winrt::Windows::UI::Notifications::ToastNotification toast(doc);
        toast.Tag(L"send");
        toast.Group(L"files");
        winrt::Windows::UI::Notifications::ToastNotificationManager::CreateToastNotifier(kAumid).Show(toast);
    } catch (...) {
    }
#endif
}

// A file's MIME type, as Windows knows its extension (LocalSend sorts by it: pictures to Photos...).
std::string FileType(std::wstring const& path) {
    WCHAR type[128] = L"";
    DWORD size = sizeof(type);
    size_t dot = path.find_last_of(L'.');
    if (dot != std::wstring::npos &&
        RegGetValueW(HKEY_CLASSES_ROOT, path.substr(dot).c_str(), L"Content Type", RRF_RT_REG_SZ, nullptr, type, &size) == ERROR_SUCCESS)
        return phonemap::ToUtf8(type);
    return "application/octet-stream";
}

std::wstring Words(std::string const& s) { return phonemap::FromUtf8(s); }

// The files to a device found: what's coming, then each one; how it went as a notification.
void Deliver(HINTERNET session, Device const& phone, std::vector<std::wstring> const& files) {
    std::wstring to = Words(phone.alias.empty() ? phone.model : phone.alias);
    // What's coming, each with an id; the phone asks you to accept (waited for up to 2 minutes).
    std::string list;
    std::vector<ULONGLONG> sizes;
    for (size_t i = 0; i < files.size(); i++) {
        WIN32_FILE_ATTRIBUTE_DATA a{};
        GetFileAttributesExW(files[i].c_str(), GetFileExInfoStandard, &a);
        ULONGLONG size = ((ULONGLONG)a.nFileSizeHigh << 32) | a.nFileSizeLow;
        sizes.push_back(size);
        std::wstring name = files[i].substr(files[i].find_last_of(L"\\/") + 1);
        list += (i ? "," : "") + std::string("\"f") + std::to_string(i) + "\":{\"id\":\"f" + std::to_string(i) + "\",\"fileName\":\"" +
                JsonEscape(phonemap::ToUtf8(name)) + "\",\"size\":" + std::to_string(size) + ",\"fileType\":\"" + FileType(files[i]) +
                "\"}";
    }
    Tell(L"Sending to " + to, L"Accept on your phone.");
    Reply prepared = Request(session, phone, L"POST", L"/api/localsend/v2/prepare-upload", "{\"info\":" + Me() + ",\"files\":{" + list + "}}", 120000);
    if (prepared.status == 204) {
        Tell(L"Nothing to send", to + L" already has them.");
        return;
    }
    if (prepared.status != 200) {
        Tell(prepared.status == 403 ? L"Not accepted" : L"Couldn't send",
             prepared.status == 403 ? to + L" said no."
             : prepared.status == 409 ? to + L" is busy with another transfer."
             : prepared.status == 0 ? L"No answer from " + to + L"."
                                    : L"LocalSend answered " + std::to_wstring(prepared.status) + L".");
        return;
    }
    std::string session2 = JsonText(prepared.body, "sessionId");
    int sent = 0;
    for (size_t i = 0; i < files.size(); i++) {
        std::string id = "f" + std::to_string(i), token = JsonText(prepared.body, id);
        if (token.empty()) continue;  // (one the phone didn't want)
        HANDLE file = CreateFileW(files[i].c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
        if (file == INVALID_HANDLE_VALUE) continue;
        std::wstring path = L"/api/localsend/v2/upload?sessionId=" + Words(session2) + L"&fileId=" + Words(id) + L"&token=" + Words(token);
        Reply up = Request(session, phone, L"POST", path, {}, 600000, file, sizes[i]);
        CloseHandle(file);
        if (up.status == 200) sent++;
    }
    Tell(sent == (int)files.size() ? L"Sent to " + to : L"Partly sent to " + to,
         std::to_wstring(sent) + (sent == 1 ? L" file" : L" files") + (sent == (int)files.size() ? L"." : L" of " + std::to_wstring(files.size()) + L"."));
    phonemap::Log(L"files: " + std::to_wstring(sent) + L" of " + std::to_wstring(files.size()) + L" sent");
}

std::atomic<bool> g_sending{false};

// Sends files to the phone (on a thread of its own; one sending at a time). pick: when several LocalSend devices
// answer, which one (called on that thread with their names; -1: none).
void SendFiles(std::vector<std::wstring> files, std::function<int(std::vector<std::wstring> const&)> pick) {
    if (files.empty()) return;
    if (g_sending.exchange(true)) {
        Tell(L"Already sending to your phone", L"Send these once the files on their way are done.");
        return;
    }
    // Over 4 GB a file can't go in one upload (its length doesn't fit): left out, and said so.
    std::vector<std::wstring> tooBig;
    for (auto it = files.begin(); it != files.end();) {
        WIN32_FILE_ATTRIBUTE_DATA a{};
        if (GetFileAttributesExW(it->c_str(), GetFileExInfoStandard, &a) && a.nFileSizeHigh) {
            tooBig.push_back(it->substr(it->find_last_of(L"\\/") + 1));
            it = files.erase(it);
        } else {
            ++it;
        }
    }
    if (!tooBig.empty())
        Tell(tooBig.size() == 1 ? L"Too big to send" : L"Some too big to send",
             tooBig[0] + (tooBig.size() > 1 ? L" and " + std::to_wstring(tooBig.size() - 1) + L" more are" : L" is") + L" over 4 GB.");
    if (files.empty()) {
        g_sending = false;
        return;
    }
    std::thread([files, pick] {
        WSADATA wsa;
        WSAStartup(MAKEWORD(2, 2), &wsa);  // (for the search's sockets)
        HINTERNET session = InternetOpenW(L"Tourne", INTERNET_OPEN_TYPE_DIRECT, nullptr, nullptr, 0);
        auto finish = [&] {
            if (session) InternetCloseHandle(session);
            WSACleanup();
            g_sending = false;
        };
        if (!session) return finish();
        auto found = Find(session, LastAddress());
        // Phones first (a desktop with LocalSend could answer too).
        std::stable_sort(found.begin(), found.end(), [](Device const& a, Device const& b) { return (a.type == "mobile") > (b.type == "mobile"); });
        if (found.empty()) {
            Tell(L"Your phone wasn't found", L"Open LocalSend on it, on the same Wi-Fi as this PC, and send again.");
            return finish();
        }
        int chosen = 0;
        if (found.size() > 1 && found[1].type == found[0].type) {
            std::vector<std::wstring> names;
            for (auto const& d : found) names.push_back(Words(d.alias) + L" (" + Words(d.model.empty() ? d.type : d.model) + L")");
            chosen = pick ? pick(names) : 0;
            if (chosen < 0 || chosen >= (int)found.size()) return finish();
        }
        Device phone = found[chosen];
        RegSetKeyValueW(HKEY_CURRENT_USER, kKey, L"PhoneLocalSend", REG_SZ, phone.ip.c_str(), (DWORD)((phone.ip.size() + 1) * sizeof(WCHAR)));
        phonemap::Log(L"files: " + std::to_wstring(files.size()) + L" to " + Words(phone.alias) + L" at " + phone.ip);

        Deliver(session, phone, files);
        finish();
    }).detach();
}

bool Sending() { return g_sending; }

}  // namespace phonefiles

#else
namespace phonefiles {
inline void SendFiles(std::vector<std::wstring>, std::function<int(std::vector<std::wstring> const&)>) {}
inline bool Sending() { return false; }
}  // namespace phonefiles
#endif
