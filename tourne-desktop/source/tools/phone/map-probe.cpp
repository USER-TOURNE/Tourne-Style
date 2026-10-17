// Can Tourne open its own Bluetooth MAP session with the phone while Phone Link has one? Connects to the
// phone's Message Access Server (MAS, 0x1132) over RFCOMM, does the OBEX CONNECT with MAP's target, asks
// for the inbox listing's size only (MaxListCount = 0: no messages read), then disconnects. Prints only
// response codes and the count. Never sends or reads a message.
//
//   map-probe.exe 68EFDCAC8DDA
#include <winsock2.h>
#include <ws2bth.h>
#include <windows.h>

#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

static const GUID kMas = {0x00001132, 0x0000, 0x1000, {0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB}};
static const uint8_t kMapTarget[16] = {0xbb, 0x58, 0x2b, 0x40, 0x42, 0x0c, 0x11, 0xdb, 0xb0, 0xde, 0x08, 0x00, 0x20, 0x0c, 0x9a, 0x66};

static void Put16(std::vector<uint8_t>& b, size_t at, uint16_t v) { b[at] = v >> 8, b[at + 1] = v & 0xFF; }

static bool SendAll(SOCKET s, std::vector<uint8_t> const& b) {
    size_t sent = 0;
    while (sent < b.size()) {
        int n = send(s, (char const*)b.data() + sent, (int)(b.size() - sent), 0);
        if (n <= 0) return false;
        sent += n;
    }
    return true;
}

// One OBEX response: its code and headers.
static bool Receive(SOCKET s, std::vector<uint8_t>& out) {
    out.assign(3, 0);
    int got = 0;
    while (got < 3) {
        int n = recv(s, (char*)out.data() + got, 3 - got, 0);
        if (n <= 0) return false;
        got += n;
    }
    size_t length = (out[1] << 8) | out[2];
    out.resize(length);
    while ((size_t)got < length) {
        int n = recv(s, (char*)out.data() + got, (int)(length - got), 0);
        if (n <= 0) return false;
        got += n;
    }
    return true;
}

int wmain(int argc, wchar_t** argv) {
    if (argc < 2) {
        wprintf(L"usage: map-probe <phone's classic Bluetooth address, 12 hex digits>\n");
        return 2;
    }
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
    SOCKET s = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
    if (s == INVALID_SOCKET) {
        wprintf(L"socket: %d\n", WSAGetLastError());
        return 1;
    }
    SOCKADDR_BTH address{};
    address.addressFamily = AF_BTH;
    address.btAddr = wcstoull(argv[1], nullptr, 16);
    address.serviceClassId = kMas;  // (the channel is looked up by SDP)
    DWORD t0 = GetTickCount();
    if (connect(s, (sockaddr*)&address, sizeof(address)) == SOCKET_ERROR) {
        wprintf(L"rfcomm connect: failed, error %d after %lu ms\n", WSAGetLastError(), GetTickCount() - t0);
        closesocket(s);
        return 1;
    }
    wprintf(L"rfcomm connect: ok after %lu ms\n", GetTickCount() - t0);
    DWORD timeout = 8000;
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char const*)&timeout, sizeof(timeout));

    // OBEX CONNECT, version 1.0, max packet 0x2000, Target = MAP MAS.
    std::vector<uint8_t> connectPacket = {0x80, 0, 0, 0x10, 0x00, 0x20, 0x00, 0x46, 0, 19};
    connectPacket.insert(connectPacket.end(), kMapTarget, kMapTarget + 16);
    Put16(connectPacket, 1, (uint16_t)connectPacket.size());
    std::vector<uint8_t> reply;
    if (!SendAll(s, connectPacket) || !Receive(s, reply)) {
        wprintf(L"obex connect: no answer (%d)\n", WSAGetLastError());
        closesocket(s);
        return 1;
    }
    wprintf(L"obex connect: 0x%02X (0xA0 = success)\n", reply[0]);
    uint32_t connectionId = 0;
    bool haveId = false;
    for (size_t at = 7; at + 1 < reply.size();) {
        uint8_t h = reply[at];
        if (h == 0xCB && at + 5 <= reply.size()) {
            connectionId = (reply[at + 1] << 24) | (reply[at + 2] << 16) | (reply[at + 3] << 8) | reply[at + 4];
            haveId = true;
        }
        uint8_t kind = h & 0xC0;
        if (kind == 0x80) at += 2;
        else if (kind == 0xC0) at += 5;
        else at += (reply[at + 1] << 8) | reply[at + 2];
    }
    wprintf(L"connection id: %s\n", haveId ? L"given" : L"none");

    if (reply[0] == 0xA0) {
        // GET x-bt/MAP-msg-listing of "inbox" with MaxListCount 0: only the listing's size comes back.
        std::vector<uint8_t> get = {0x83, 0, 0};
        auto u32 = [&](uint32_t v) { get.push_back(v >> 24), get.push_back(v >> 16), get.push_back(v >> 8), get.push_back(v); };
        if (haveId) get.push_back(0xCB), u32(connectionId);
        const char* type = "x-bt/MAP-msg-listing";
        size_t typeLength = strlen(type) + 1;
        get.push_back(0x42), get.push_back(0), get.push_back((uint8_t)(3 + typeLength));
        get.insert(get.end(), type, type + typeLength);
        std::wstring name = L"inbox";
        size_t nameAt = get.size();
        get.push_back(0x01), get.push_back(0), get.push_back(0);
        for (wchar_t c : name) get.push_back(c >> 8), get.push_back(c & 0xFF);
        get.push_back(0), get.push_back(0);
        Put16(get, nameAt + 1, (uint16_t)(get.size() - nameAt));
        // Application parameters: MaxListCount (0x01, 2 bytes) = 0.
        get.push_back(0x4C), get.push_back(0), get.push_back(3 + 4);
        get.push_back(0x01), get.push_back(2), get.push_back(0), get.push_back(0);
        Put16(get, 1, (uint16_t)get.size());
        if (SendAll(s, get) && Receive(s, reply)) {
            wprintf(L"listing size: response 0x%02X\n", reply[0]);
            for (size_t at = 3; at + 2 < reply.size();) {
                uint8_t h = reply[at];
                uint8_t kind = h & 0xC0;
                size_t length = kind == 0x80 ? 2 : kind == 0xC0 ? 5 : ((reply[at + 1] << 8) | reply[at + 2]);
                if (h == 0x4C) {
                    for (size_t p = at + 3; p + 1 < at + length;) {
                        uint8_t tag = reply[p], len = reply[p + 1];
                        if (tag == 0x12 && len == 2) wprintf(L"messages in the inbox listing: %d\n", (reply[p + 2] << 8) | reply[p + 3]);
                        if (tag == 0x0D && len == 1) wprintf(L"new message flag: %d\n", reply[p + 2]);
                        p += 2 + len;
                    }
                }
                if (!length) break;
                at += length;
            }
        } else {
            wprintf(L"listing size: no answer (%d)\n", WSAGetLastError());
        }
        // OBEX DISCONNECT.
        std::vector<uint8_t> bye = {0x81, 0, 3};
        if (haveId) {
            bye.push_back(0xCB);
            bye.push_back(connectionId >> 24), bye.push_back(connectionId >> 16), bye.push_back(connectionId >> 8), bye.push_back(connectionId);
            Put16(bye, 1, (uint16_t)bye.size());
        }
        if (SendAll(s, bye) && Receive(s, reply)) wprintf(L"obex disconnect: 0x%02X\n", reply[0]);
    }
    closesocket(s);
    WSACleanup();
    return 0;
}
