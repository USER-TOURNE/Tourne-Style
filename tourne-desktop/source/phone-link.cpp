////////////////////////////////////////////////////////////////////////////////
// Your phone, as Phone Link knows it ("// @inline phone-link.cpp", in taskbar-host.cpp before start-menu.cpp,
// which shows it in the Start menu's phone pane, startMenu.phone).
//
// Read, never written, from what Windows and Phone Link already keep:
//   - which phone is linked, its name and system: Cross Device's DeviceMetadataStorage.json;
//   - its battery and whether it's connected: its Bluetooth device (as Settings, Bluetooth & devices shows
//     them), so they're right with Phone Link closed too;
//   - its notifications and how many are unread: Phone Link's toasts and badge in Windows' notification
//     database (wpndatabase.db, through Windows' own winsqlite3.dll, as System Icons reads it);
//   - each phone app's name and icon: Phone Link's phoneapps.db, so a notification shows as Discord or
//     Messages with its own icon, not as Phone Link.
// With Tourne holding the phone instead (phone-map.cpp, phone-ancs.cpp), only Tourne's own notifications count,
// and the phone is still known by name with Phone Link unlinked or gone (kept in HKCU\Software\Tourne\StartMenu
// while it's still paired).
// On a thread of its own when the Start menu opens. Idle cost: none.

constexpr wchar_t kPhoneLinkPackage[] = L"Microsoft.YourPhone_8wekyb3d8bbwe";
constexpr wchar_t kPhoneLinkApp[] = L"shell:AppsFolder\\Microsoft.YourPhone_8wekyb3d8bbwe!App";

constexpr GUID kPhoneWicFactory = {0xcacaf262, 0x9370, 0x4615, {0xa1, 0x3b, 0x9f, 0x55, 0x39, 0xda, 0x4c, 0x0a}};
constexpr GUID kPhoneWicPbgra = {0x6fddc324, 0x4e03, 0x4bfe, {0xb1, 0x85, 0x3d, 0x77, 0x76, 0x8d, 0xc9, 0x10}};

std::wstring PhoneLower(std::wstring s) {
    if (!s.empty()) CharLowerBuffW(s.data(), (DWORD)s.size());
    return s;
}

struct PhoneNote {
    std::wstring app, package, title, body;
    long long when = 0;  // FILETIME
    std::wstring aumid, tag, group;  // which of Windows' notifications it is (to clear it, as the centre does)
};

struct PhoneInfo {
    bool linked = false;
    std::wstring name, os, model;
    int battery = -1;  // percent; -1 unknown
    bool connected = false;
    int unread = 0;
    std::vector<PhoneNote> notes;
    bool read = false;  // (PhoneNow: it has been read at least once)
};

namespace phonedb {
// Just enough of SQLite, from winsqlite3.dll in System32.
struct Db;
struct Stmt;
using Open = int(__cdecl*)(char const*, Db**, int, char const*);
using Prepare = int(__cdecl*)(Db*, char const*, int, Stmt**, char const**);
using Step = int(__cdecl*)(Stmt*);
using ColumnText = unsigned char const*(__cdecl*)(Stmt*, int);
using ColumnInt64 = long long(__cdecl*)(Stmt*, int);
using ColumnBlob = void const*(__cdecl*)(Stmt*, int);
using ColumnBytes = int(__cdecl*)(Stmt*, int);
using BindText = int(__cdecl*)(Stmt*, int, char const*, int, void*);
using Finalize = int(__cdecl*)(Stmt*);
using Close = int(__cdecl*)(Db*);
constexpr int kReadOnly = 0x1, kUri = 0x40, kRow = 100;

struct Api {
    Open open = nullptr;
    Prepare prepare = nullptr;
    Step step = nullptr;
    ColumnText text = nullptr;
    ColumnInt64 int64 = nullptr;
    ColumnBlob blob = nullptr;
    ColumnBytes bytes = nullptr;
    BindText bindText = nullptr;
    Finalize finalize = nullptr;
    Close close = nullptr;
};

Api const* Load() {
    static Api api = [] {
        Api a;
        if (HMODULE lib = LoadLibraryExW(L"winsqlite3.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32)) {
            a.open = (Open)GetProcAddress(lib, "sqlite3_open_v2");
            a.prepare = (Prepare)GetProcAddress(lib, "sqlite3_prepare_v2");
            a.step = (Step)GetProcAddress(lib, "sqlite3_step");
            a.text = (ColumnText)GetProcAddress(lib, "sqlite3_column_text");
            a.int64 = (ColumnInt64)GetProcAddress(lib, "sqlite3_column_int64");
            a.blob = (ColumnBlob)GetProcAddress(lib, "sqlite3_column_blob");
            a.bytes = (ColumnBytes)GetProcAddress(lib, "sqlite3_column_bytes");
            a.bindText = (BindText)GetProcAddress(lib, "sqlite3_bind_text");
            a.finalize = (Finalize)GetProcAddress(lib, "sqlite3_finalize");
            a.close = (Close)GetProcAddress(lib, "sqlite3_close");
        }
        return a;
    }();
    return api.open && api.prepare && api.step && api.text && api.int64 && api.blob && api.bytes && api.bindText &&
                   api.finalize && api.close
               ? &api
               : nullptr;
}

std::string Utf8(std::wstring const& s) {
    int n = WideCharToMultiByte(CP_UTF8, 0, s.c_str(), (int)s.size(), nullptr, 0, nullptr, nullptr);
    std::string out(n, '\0');
    WideCharToMultiByte(CP_UTF8, 0, s.c_str(), (int)s.size(), out.data(), n, nullptr, nullptr);
    return out;
}

std::wstring Wide(unsigned char const* s) {
    if (!s) return {};
    int n = MultiByteToWideChar(CP_UTF8, 0, (char const*)s, -1, nullptr, 0);
    std::wstring out(n > 0 ? n - 1 : 0, L'\0');
    if (n > 1) MultiByteToWideChar(CP_UTF8, 0, (char const*)s, -1, out.data(), n);
    return out;
}

// Each row of a read-only query (the file left as it is: Phone Link and Windows keep writing it).
// bind: the text for the statement's ?1, if it has one (never put into the SQL itself).
template <typename Row>
bool Query(std::wstring const& file, char const* sql, Row row, std::string const* bind = nullptr) {
    auto* api = Load();
    if (!api || GetFileAttributesW(file.c_str()) == INVALID_FILE_ATTRIBUTES) return false;
    std::wstring uri = L"file:" + file;
    for (auto& c : uri)
        if (c == L'\\') c = L'/';
    Db* db = nullptr;
    if (api->open(Utf8(uri + L"?mode=ro").c_str(), &db, kReadOnly | kUri, nullptr)) {
        if (db) api->close(db);
        return false;
    }
    Stmt* stmt = nullptr;
    bool ok = api->prepare(db, sql, -1, &stmt, nullptr) == 0;
    if (ok && bind) ok = api->bindText(stmt, 1, bind->c_str(), (int)bind->size(), (void*)-1) == 0;  // (SQLITE_TRANSIENT)
    if (ok)
        while (api->step(stmt) == kRow) row(api, stmt);
    if (stmt) api->finalize(stmt);
    api->close(db);
    return ok;
}
}  // namespace phonedb

std::wstring PhoneLocalAppData() {
    WCHAR path[MAX_PATH] = L"";
    return SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, path)) ? path : L"";
}

std::wstring PhoneReadFile(std::wstring const& path) {
    HANDLE file = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
                              OPEN_EXISTING, 0, nullptr);
    if (file == INVALID_HANDLE_VALUE) return {};
    std::string bytes;
    LARGE_INTEGER size{};
    if (GetFileSizeEx(file, &size) && size.QuadPart > 0 && size.QuadPart < (1 << 22)) {
        bytes.resize((size_t)size.QuadPart);
        DWORD read = 0;
        if (!ReadFile(file, bytes.data(), (DWORD)bytes.size(), &read, nullptr)) read = 0;
        bytes.resize(read);
    }
    CloseHandle(file);
    return phonedb::Wide((unsigned char const*)bytes.c_str());
}

// A JSON string's value after "key": in text, from at (unescaped); empty when it isn't there.
std::wstring PhoneJsonText(std::wstring const& text, std::wstring const& key, size_t at, size_t end) {
    size_t k = text.find(L"\"" + key + L"\"", at);
    if (k == std::wstring::npos || k >= end) return {};
    size_t q = text.find(L'"', text.find(L':', k + key.size() + 2));
    if (q == std::wstring::npos || q >= end) return {};
    std::wstring out;
    for (size_t i = q + 1; i < text.size() && text[i] != L'"'; i++) {
        if (text[i] == L'\\' && i + 1 < text.size()) {
            i++;
            if (text[i] == L'n') out += L' ';
            else if (text[i] == L'u' && i + 4 < text.size()) {
                out += (wchar_t)wcstoul(text.substr(i + 1, 4).c_str(), nullptr, 16);
                i += 4;
            } else out += text[i];
        } else {
            out += text[i];
        }
    }
    return out;
}

// The phone's Bluetooth device (Classic or LE), by its name: whether Windows has it paired, and its battery and
// whether it's connected, as Settings, Bluetooth & devices shows them. False: none by that name.
bool PhoneBluetoothDevice(std::wstring const& name, int* battery, bool* connected) {
    if (name.empty()) return false;
    using GetListSize = CONFIGRET(WINAPI*)(PULONG, PCWSTR, ULONG);
    using GetList = CONFIGRET(WINAPI*)(PCWSTR, PZZWSTR, ULONG, ULONG);
    using Locate = CONFIGRET(WINAPI*)(PDEVINST, DEVINSTID_W, ULONG);
    using GetProperty = CONFIGRET(WINAPI*)(DEVINST, const DEVPROPKEY*, DEVPROPTYPE*, PBYTE, PULONG, ULONG);
    static HMODULE lib = LoadLibraryExW(L"cfgmgr32.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (!lib) return false;
    auto listSize = (GetListSize)(void*)GetProcAddress(lib, "CM_Get_Device_ID_List_SizeW");
    auto list = (GetList)(void*)GetProcAddress(lib, "CM_Get_Device_ID_ListW");
    auto locate = (Locate)(void*)GetProcAddress(lib, "CM_Locate_DevNodeW");
    auto property = (GetProperty)(void*)GetProcAddress(lib, "CM_Get_DevNode_PropertyW");
    if (!listSize || !list || !locate || !property) return false;
    constexpr DEVPROPKEY kName{{0xa45c254e, 0xdf1c, 0x4efd, {0x80, 0x20, 0x67, 0xd1, 0x46, 0xa8, 0x50, 0xe0}}, 14};  // FriendlyName
    constexpr DEVPROPKEY kBattery{{0x104ea319, 0x6ee2, 0x4701, {0xbd, 0x47, 0x8d, 0xdb, 0xf4, 0x25, 0xbb, 0xe5}}, 2};
    constexpr DEVPROPKEY kConnected{{0x83da6326, 0x97a6, 0x4088, {0x94, 0x53, 0xa1, 0x92, 0x3f, 0x57, 0x3b, 0x29}}, 15};
    bool found = false;
    // Every node by that name (an iPhone has a Classic and an LE one, and only some carry the battery): the
    // battery from whichever has it, connected if any is.
    for (PCWSTR bus : {L"BTHLE", L"BTHENUM"}) {
        ULONG size = 0;
        if (listSize(&size, bus, CM_GETIDLIST_FILTER_ENUMERATOR | CM_GETIDLIST_FILTER_PRESENT) != CR_SUCCESS || !size) continue;
        std::vector<WCHAR> ids(size);
        if (list(bus, ids.data(), size, CM_GETIDLIST_FILTER_ENUMERATOR | CM_GETIDLIST_FILTER_PRESENT) != CR_SUCCESS) continue;
        for (PCWSTR id = ids.data(); *id; id += wcslen(id) + 1) {
            DEVINST node = 0;
            if (locate(&node, (DEVINSTID_W)id, CM_LOCATE_DEVNODE_NORMAL) != CR_SUCCESS) continue;
            WCHAR friendly[256] = L"";
            ULONG bytes = sizeof(friendly);
            DEVPROPTYPE type = 0;
            if (property(node, &kName, &type, (PBYTE)friendly, &bytes, 0) != CR_SUCCESS || name != friendly) continue;
            found = true;
            BYTE level = 0;
            bytes = sizeof(level);
            if (battery && *battery < 0 && property(node, &kBattery, &type, &level, &bytes, 0) == CR_SUCCESS && type == DEVPROP_TYPE_BYTE)
                *battery = level;
            DEVPROP_BOOLEAN on = DEVPROP_FALSE;
            bytes = sizeof(on);
            if (connected && property(node, &kConnected, &type, (PBYTE)&on, &bytes, 0) == CR_SUCCESS && on == DEVPROP_TRUE)
                *connected = true;
        }
    }
    return found;
}

// The phone as Tourne last saw it linked (HKCU\Software\Tourne\StartMenu: PhoneName, PhoneOs, PhoneModel).
constexpr wchar_t kPhoneRecordKey[] = L"Software\\Tourne\\StartMenu";

std::wstring PhoneRecord(PCWSTR value) {
    WCHAR text[256] = L"";
    DWORD size = sizeof(text);
    if (RegGetValueW(HKEY_CURRENT_USER, kPhoneRecordKey, value, RRF_RT_REG_SZ, nullptr, text, &size) != ERROR_SUCCESS) return {};
    return text;
}

void KeepPhoneRecord(PhoneInfo const& info) {
    for (auto [value, text] : {std::pair{L"PhoneName", &info.name}, std::pair{L"PhoneOs", &info.os}, std::pair{L"PhoneModel", &info.model}})
        if (PhoneRecord(value) != *text)
            RegSetKeyValueW(HKEY_CURRENT_USER, kPhoneRecordKey, value, REG_SZ, text->c_str(), (DWORD)((text->size() + 1) * sizeof(WCHAR)));
}

// The linked phone: the device entry that isn't this PC (Cross Device's, else Phone Link's own copy). Without one
// (Phone Link unlinked or removed), the phone Tourne last saw linked, while Windows still has it paired.
void ReadPhoneIdentity(PhoneInfo& info) {
    std::wstring base = PhoneLocalAppData() + L"\\Packages\\";
    for (PCWSTR package : {L"MicrosoftWindows.CrossDevice_cw5n1h2txyewy", L"Microsoft.YourPhone_8wekyb3d8bbwe"}) {
        std::wstring text = PhoneReadFile(base + package + L"\\LocalCache\\DeviceMetadataStorage.json");
        for (size_t at = 0; (at = text.find(L"\"IsLinked\"", at)) != std::wstring::npos; at++) {
            size_t next = text.find(L"\"IsLinked\"", at + 1);
            if (next == std::wstring::npos) next = text.size();
            size_t value = text.find_first_not_of(L" \t\r\n", text.find(L':', at) + 1);
            bool linked = value != std::wstring::npos && text.compare(value, 4, L"true") == 0;
            std::wstring os = PhoneJsonText(text, L"OsName", at, next);
            if (!linked || os.empty() || !_wcsicmp(os.c_str(), L"Windows")) continue;
            info.linked = true;
            info.os = os + L" " + PhoneJsonText(text, L"OsVersion", at, next);
            info.name = PhoneJsonText(text, L"DisplayName", at, next);
            info.model = PhoneJsonText(text, L"ModelName", at, next);
            if (!info.name.empty()) KeepPhoneRecord(info);
            return;
        }
    }
    std::wstring name = PhoneRecord(L"PhoneName");
    if (name.empty() || !PhoneBluetoothDevice(name, nullptr, nullptr)) return;
    info.linked = true;
    info.name = name;
    info.os = PhoneRecord(L"PhoneOs");
    info.model = PhoneRecord(L"PhoneModel");
}

// Its battery and whether it's connected.
void ReadPhoneBluetooth(PhoneInfo& info) {
    PhoneBluetoothDevice(info.name, &info.battery, &info.connected);
}

// Phone Link's data folder for the phone: the one written last.
std::wstring PhoneDataFolder() {
    std::wstring root = PhoneLocalAppData() + L"\\Packages\\" + kPhoneLinkPackage + L"\\LocalCache\\Indexed\\";
    std::wstring best;
    ULONGLONG newest = 0;
    WIN32_FIND_DATAW found;
    HANDLE find = FindFirstFileExW((root + L"*").c_str(), FindExInfoBasic, &found, FindExSearchNameMatch, nullptr, 0);
    if (find == INVALID_HANDLE_VALUE) return {};
    do {
        if (!(found.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) || found.cFileName[0] == L'.') continue;
        std::wstring folder = root + found.cFileName + L"\\System\\Database\\";
        for (PCWSTR file : {L"phoneapps.db-wal", L"phoneapps.db"}) {
            WIN32_FILE_ATTRIBUTE_DATA data;
            if (!GetFileAttributesExW((folder + file).c_str(), GetFileExInfoStandard, &data)) continue;
            ULONGLONG when = ((ULONGLONG)data.ftLastWriteTime.dwHighDateTime << 32) | data.ftLastWriteTime.dwLowDateTime;
            if (when > newest) newest = when, best = folder;
        }
    } while (FindNextFileW(find, &found));
    FindClose(find);
    return best;
}

// The phone's apps that send notifications: package (lower case) to name.
std::vector<std::pair<std::wstring, std::wstring>> PhoneApps() {
    std::vector<std::pair<std::wstring, std::wstring>> apps;
    std::wstring folder = PhoneDataFolder();
    if (folder.empty()) return apps;
    phonedb::Query(folder + L"phoneapps.db", "SELECT package_name, app_name FROM phone_apps",
                   [&](phonedb::Api const* api, phonedb::Stmt* stmt) {
                       std::wstring package = phonedb::Wide(api->text(stmt, 0)), name = phonedb::Wide(api->text(stmt, 1));
                       if (!package.empty()) apps.push_back({PhoneLower(package), name});
                   });
    return apps;
}

// Emoji and fancy letters as the pixel fonts can draw them: a face or a heart as its plain symbol (":)", "<3"),
// fancy letters (maths bold or script, fullwidth) as plain ones, other emoji as "*"; the joiners and colour/skin marks between them
// dropped. Everything else as it was.
std::wstring PlainSymbols(std::wstring const& s) {
    struct Swap {
        char32_t from, to;  // a range
        PCWSTR as;
    };
    static Swap const swaps[] = {
        {0x2764, 0x2764, L"<3"}, {0x2665, 0x2665, L"<3"}, {0x1F493, 0x1F49F, L"<3"}, {0x1F9E1, 0x1F9E1, L"<3"},
        {0x1F49A, 0x1F49C, L"<3"}, {0x1F5A4, 0x1F5A4, L"<3"}, {0x1F90D, 0x1F90E, L"<3"}, {0x1FA75, 0x1FA77, L"<3"},
        {0x1F60D, 0x1F60D, L"<3"}, {0x1F970, 0x1F970, L"<3"}, {0x1F618, 0x1F618, L":*"}, {0x1F617, 0x1F61A, L":*"},
        {0x1F602, 0x1F602, L"xD"}, {0x1F923, 0x1F923, L"xD"}, {0x1F606, 0x1F606, L"xD"}, {0x1F605, 0x1F605, L"^^'"},
        {0x1F600, 0x1F601, L":D"}, {0x1F603, 0x1F604, L":D"}, {0x1F60A, 0x1F60A, L":)"}, {0x1F642, 0x1F642, L":)"},
        {0x263A, 0x263A, L":)"}, {0x1F607, 0x1F607, L":)"}, {0x1F609, 0x1F609, L";)"}, {0x1F60E, 0x1F60E, L"B)"},
        {0x1F61B, 0x1F61D, L":P"}, {0x1F60B, 0x1F60B, L":P"}, {0x1F622, 0x1F622, L":'("}, {0x1F62D, 0x1F62D, L":'("},
        {0x1F61E, 0x1F61F, L":("}, {0x1F641, 0x1F641, L":("}, {0x2639, 0x2639, L":("}, {0x1F614, 0x1F614, L":("},
        {0x1F620, 0x1F621, L">:("}, {0x1F62E, 0x1F62F, L":O"}, {0x1F632, 0x1F632, L":O"}, {0x1F610, 0x1F611, L":|"},
        {0x1F612, 0x1F612, L"-_-"}, {0x1F644, 0x1F644, L"-_-"}, {0x1F634, 0x1F634, L"zzz"}, {0x1F914, 0x1F914, L"hmm"},
        {0x1F937, 0x1F937, L"shrug"}, {0x1F44D, 0x1F44D, L"(y)"}, {0x1F44E, 0x1F44E, L"(n)"}, {0x1F44B, 0x1F44B, L"o/"},
        {0x1F64F, 0x1F64F, L"(pray)"}, {0x1F525, 0x1F525, L"(fire)"}, {0x1F480, 0x1F480, L"(skull)"},
        {0x2620, 0x2620, L"(skull)"}, {0x1F440, 0x1F440, L"(eyes)"}, {0x1F389, 0x1F38A, L"(party)"},
        {0x1F4AF, 0x1F4AF, L"100"}, {0x2705, 0x2705, L"[v]"}, {0x2714, 0x2714, L"v"}, {0x274C, 0x274C, L"x"},
        {0x2716, 0x2716, L"x"}, {0x2B50, 0x2B50, L"*"}, {0x1F31F, 0x1F31F, L"*"}, {0x2728, 0x2728, L"*"},
        {0x1F3B5, 0x1F3B6, L"(music)"}, {0x1F4F7, 0x1F4F8, L"(photo)"}, {0x26A0, 0x26A0, L"(!)"}, {0x2757, 0x2757, L"!"},
        {0x2753, 0x2753, L"?"}, {0x2190, 0x2190, L"<-"}, {0x2192, 0x2192, L"->"}, {0x2191, 0x2191, L"^"}, {0x2193, 0x2193, L"v"},
        {0x2194, 0x2194, L"<->"}, {0x21A9, 0x21AA, L"<-"}, {0x2022, 0x2022, L"\u00B7"}, {0x2023, 0x2023, L">"},
        {0x2018, 0x2019, L"'"}, {0x201C, 0x201D, L"\""}, {0x2013, 0x2014, L"-"}, {0x00A0, 0x00A0, L" "},
    };
    std::wstring out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); i++) {
        char32_t c = s[i];
        size_t n = 1;
        if (c >= 0xD800 && c <= 0xDBFF && i + 1 < s.size() && s[i + 1] >= 0xDC00 && s[i + 1] <= 0xDFFF) {
            c = 0x10000 + ((c - 0xD800) << 10) + (s[i + 1] - 0xDC00);
            n = 2;
        }
        i += n - 1;
        // Marks between and after emoji: colour (FE0F), joiner (200D), skin tones, keycap, tags.
        if (c == 0xFE0E || c == 0xFE0F || c == 0x200D || c == 0x20E3 || (c >= 0x1F3FB && c <= 0x1F3FF) || (c >= 0xE0020 && c <= 0xE007F))
            continue;
        PCWSTR swapped = nullptr;
        for (auto const& w : swaps)
            if (c >= w.from && c <= w.to) {
                swapped = w.as;
                break;
            }
        if (swapped) {
            // (a word, not glued to the next one: "thanks<3" reads as "thanks <3")
            if (wcslen(swapped) > 1 && !out.empty() && out.back() != L' ' && iswalnum(out.back())) out += L' ';
            out += swapped;
            continue;
        }
        if (c >= 0x1D400 && c <= 0x1D6A3) {  // fancy letters: bold, italic, script, fraktur, double-struck...
            int k = (int)((c - 0x1D400) % 52);
            out += (wchar_t)(k < 26 ? L'A' + k : L'a' + k - 26);
        } else if (c >= 0x1D7CE && c <= 0x1D7FF) {  // and their digits
            out += (wchar_t)(L'0' + (c - 0x1D7CE) % 10);
        } else if (c >= 0xFF01 && c <= 0xFF5E) {  // fullwidth
            out += (wchar_t)(c - 0xFEE0);
        } else if (c >= 0x1F1E6 && c <= 0x1F1FF) {  // a flag's letters
            continue;
        } else if ((c >= 0x1F000 && c <= 0x1FAFF) || (c >= 0x2600 && c <= 0x27BF) || (c >= 0x2B00 && c <= 0x2BFF) ||
                   (c >= 0x2190 && c <= 0x21FF) || (c >= 0x2300 && c <= 0x23FF) || (c >= 0xE000 && c <= 0xF8FF) || c > 0xFFFF) {
            if (out.empty() || out.back() != L'*') out += L'*';
        } else {
            out += (wchar_t)c;
        }
    }
    return out;
}

std::wstring PhoneXmlText(std::wstring_view xml) {
    std::wstring out;
    for (size_t i = 0; i < xml.size(); i++) {
        if (xml[i] != L'&') {
            out += xml[i];
            continue;
        }
        size_t end = xml.find(L';', i);
        if (end == std::wstring_view::npos || end - i > 10) {
            out += xml[i];
            continue;
        }
        std::wstring_view e = xml.substr(i + 1, end - i - 1);
        if (e == L"amp") out += L'&';
        else if (e == L"lt") out += L'<';
        else if (e == L"gt") out += L'>';
        else if (e == L"quot") out += L'"';
        else if (e == L"apos") out += L'\'';
        else if (!e.empty() && e[0] == L'#') out += (wchar_t)wcstoul(std::wstring(e.substr(e.size() > 1 && e[1] == L'x' ? 2 : 1)).c_str(), nullptr, e.size() > 1 && e[1] == L'x' ? 16 : 10);
        else out += L'&', out += std::wstring(e), out += L';';
        i = end;
    }
    for (auto& c : out)
        if (c == L'\n' || c == L'\r' || c == L'\t') c = L' ';
    return out;
}

// A phone app's name: as Phone Link lists it, else its package's last part ("com.google.Gmail": Gmail).
// Where a phone app's notification opens on this PC, so it can be answered there: Apple's notification service
// lets the PC only clear one or press its own one or two buttons, never reply. The app itself when it's installed
// here (its link scheme is registered), else its website; empty for an app with neither (the centre opens then).
std::wstring PhoneAppOnPc(std::wstring const& package, std::wstring const& name) {
    struct Place {
        std::initializer_list<PCWSTR> words;  // in its bundle id or name
        PCWSTR app, web;                      // its own link (scheme first), its site
    };
    static Place const places[] = {
        {{L"discord", L"hammerandchisel"}, L"discord://-/channels/@me", L"https://discord.com/channels/@me"},
        {{L"slack"}, L"slack://open", L"https://app.slack.com"},
        {{L"whatsapp"}, L"whatsapp://", L"https://web.whatsapp.com"},
        {{L"telegram", L"ph.telegra"}, L"tg://", L"https://web.telegram.org"},
        {{L"spotify"}, L"spotify:", L"https://open.spotify.com"},
        {{L"teams"}, L"msteams:", L"https://teams.microsoft.com"},
        {{L"gmail"}, nullptr, L"https://mail.google.com"},
        {{L"outlook"}, L"outlookmail:", L"https://outlook.live.com/mail"},
        {{L"reddit"}, nullptr, L"https://www.reddit.com/notifications"},
        {{L"instagram"}, nullptr, L"https://www.instagram.com/direct/inbox"},
        {{L"messenger", L"facebook"}, nullptr, L"https://www.messenger.com"},
        {{L"twitter", L"atebits"}, nullptr, L"https://x.com/notifications"},
        {{L"github"}, nullptr, L"https://github.com/notifications"},
        {{L"youtube"}, nullptr, L"https://www.youtube.com/feed/subscriptions"},
        {{L"twitch"}, nullptr, L"https://www.twitch.tv"},
        {{L"steam", L"valvesoftware"}, L"steam://open/friends", L"https://steamcommunity.com"},
    };
    std::wstring text = PhoneLower(package + L" " + name);
    for (auto const& place : places) {
        bool match = false;
        for (PCWSTR w : place.words) match = match || text.find(w) != std::wstring::npos;
        if (!match) continue;
        if (place.app) {
            std::wstring scheme(place.app, wcschr(place.app, L':') - place.app);
            HKEY key;
            if (RegOpenKeyExW(HKEY_CLASSES_ROOT, scheme.c_str(), 0, KEY_READ, &key) == ERROR_SUCCESS) {
                bool registered = RegQueryValueExW(key, L"URL Protocol", nullptr, nullptr, nullptr, nullptr) == ERROR_SUCCESS;
                RegCloseKey(key);
                if (registered) return place.app;
            }
        }
        return place.web;
    }
    return {};
}

std::wstring PhoneAppName(std::wstring const& package, std::vector<std::pair<std::wstring, std::wstring>> const& apps) {
    for (auto const& [p, name] : apps)
        if (p == package && !name.empty()) return name;
    std::wstring last = package.substr(package.find_last_of(L'.') + 1);
    if (!last.empty()) last[0] = towupper(last[0]);
    return last;
}

// A phone app's name as Tourne registered it for its notifications ("Tourne.Phone.<bundle>", phone-ancs.cpp).
std::wstring PhoneRegisteredName(std::wstring const& aumid) {
    WCHAR name[256] = L"";
    DWORD size = sizeof(name);
    std::wstring key = L"Software\\Classes\\AppUserModelId\\" + aumid;
    if (RegGetValueW(HKEY_CURRENT_USER, key.c_str(), L"DisplayName", RRF_RT_REG_SZ, nullptr, name, &size) != ERROR_SUCCESS) return {};
    return name;
}

// The phone's notifications, newest first: Phone Link's toasts and its badge (the unread count), and Tourne's own
// (phone-ancs.cpp; all of them count as new while they're in the centre). own: Tourne holds the phone, so only
// Tourne's (Phone Link's left over would show twice, and its old badge would stand for the count).
void ReadPhoneNotes(PhoneInfo& info, int most, bool own) {
    std::wstring db = PhoneLocalAppData() + L"\\Microsoft\\Windows\\Notifications\\wpndatabase.db";
    auto apps = PhoneApps();
    std::string sql =
        "SELECT n.Type, n.ArrivalTime, n.Payload, h.PrimaryId, n.Tag, n.[Group] FROM Notification n JOIN NotificationHandler h ON h.RecordId = n.HandlerId"
        + std::string(own ? " WHERE h.PrimaryId LIKE 'Tourne.Phone.%' AND n.Type = 'toast'"
                          : " WHERE (h.PrimaryId LIKE 'Microsoft.YourPhone%' OR h.PrimaryId LIKE 'Tourne.Phone.%') AND (n.Type = 'badge' OR n.Type = 'toast')") +
        " ORDER BY n.ArrivalTime DESC";
    bool badge = false;
    int ours = 0;
    phonedb::Query(db, sql.c_str(), [&](phonedb::Api const* api, phonedb::Stmt* stmt) {
        std::wstring type = phonedb::Wide(api->text(stmt, 0));
        std::wstring handler = phonedb::Wide(api->text(stmt, 3));
        if (handler.starts_with(L"Tourne.Phone.") && type == L"toast") ours++;
        if (type == L"badge") badge = true;
        int bytes = api->bytes(stmt, 2);
        auto* blob = (char const*)api->blob(stmt, 2);
        std::string raw(blob ? blob : "", blob ? (size_t)bytes : 0);
        std::wstring xml = phonedb::Wide((unsigned char const*)raw.c_str());
        if (type == L"badge") {
            size_t v = xml.find(L"value=\"");
            if (v != std::wstring::npos) info.unread = _wtoi(xml.c_str() + v + 7);
            return;
        }
        if ((int)info.notes.size() >= most) return;
        PhoneNote note;
        note.when = api->int64(stmt, 1);
        note.aumid = handler;
        note.tag = phonedb::Wide(api->text(stmt, 4));
        note.group = phonedb::Wide(api->text(stmt, 5));
        std::vector<std::wstring> texts;
        for (size_t at = 0; (at = xml.find(L"<text", at)) != std::wstring::npos;) {
            size_t open = xml.find(L'>', at), close = xml.find(L"</text>", at);
            if (open == std::wstring::npos) break;
            if (xml[open - 1] == L'/' || close == std::wstring::npos || close < open) {
                at = open + 1;
                continue;
            }
            std::wstring t = PhoneXmlText(std::wstring_view(xml).substr(open + 1, close - open - 1));
            if (!t.empty()) texts.push_back(t);
            at = close + 7;
        }
        // Which phone app: Phone Link posts each one's as its own ("...!YourPhoneNotifications_com.google.Gmail"),
        // as Tourne does ("Tourne.Phone.com.google.Gmail"), else its package named anywhere in the toast (its
        // launch arguments carry it).
        if (handler.starts_with(L"Tourne.Phone.")) {
            note.package = PhoneLower(handler.substr(13));
            note.app = PhoneRegisteredName(handler);
            if (note.app.empty()) note.app = PhoneAppName(note.package, apps);
        } else if (size_t at = handler.find(L"!YourPhoneNotifications_"); at != std::wstring::npos) {
            std::wstring package = PhoneLower(handler.substr(at + 24));
            if (package.find(L'-') == std::wstring::npos || package.find(L'.') != std::wstring::npos) {  // (not a device id)
                note.package = package;
                note.app = PhoneAppName(package, apps);
            }
        }
        std::wstring lower = PhoneLower(xml);
        for (auto const& [package, name] : apps)
            if (note.package.empty() && lower.find(package) != std::wstring::npos) {
                note.package = package;
                note.app = name;
                break;
            }
        if (!texts.empty() && !note.app.empty() && !_wcsicmp(texts[0].c_str(), note.app.c_str())) texts.erase(texts.begin());
        if (note.app.empty()) note.app = L"Phone Link";
        if (!texts.empty()) note.title = texts[0];
        for (size_t i = 1; i < texts.size(); i++) note.body += (note.body.empty() ? L"" : L" ") + texts[i];
        note.title = PlainSymbols(note.title);
        note.body = PlainSymbols(note.body);
        if (!note.title.empty() || !note.body.empty()) info.notes.push_back(note);
    });
    if (!badge) info.unread = ours;
}

PhoneInfo ReadPhoneInfo(int notes, bool own = false) {
    PhoneInfo info;
    ReadPhoneIdentity(info);
    if (!info.linked) return info;
    ReadPhoneBluetooth(info);
    ReadPhoneNotes(info, notes, own);
    return info;
}

// Phone apps' icons made so far, by package and size (null: Phone Link had none then, asked again in a minute).
struct PhoneMadeIcon {
    HICON icon = nullptr;
    ULONGLONG when = 0;
};
std::map<std::wstring, PhoneMadeIcon> g_phoneAppIcons;

// Let go of them all (the scale or the colours changed: they're made again as they're drawn).
void ForgetPhoneAppIcons() {
    for (auto& [key, made] : g_phoneAppIcons)
        if (made.icon) DestroyIcon(made.icon);
    g_phoneAppIcons.clear();
}

// A phone app's icon from Phone Link's copy (a PNG), at a size; kept for the next paint. Null: none.
HICON PhoneAppIcon(std::wstring const& package, int size) {
    std::wstring key = package + L"|" + std::to_wstring(size);
    auto it = g_phoneAppIcons.find(key);
    if (it != g_phoneAppIcons.end() && (it->second.icon || GetTickCount64() - it->second.when < 60000)) return it->second.icon;
    HICON icon = nullptr;
    std::string png;
    std::wstring folder = PhoneDataFolder();
    std::string lower = phonedb::Utf8(package);
    if (!folder.empty())
        phonedb::Query(
            folder + L"phoneapps.db", "SELECT blob FROM phone_apps WHERE lower(package_name) = ?1",
            [&](phonedb::Api const* api, phonedb::Stmt* stmt) {
                auto* b = (char const*)api->blob(stmt, 0);
                if (b && png.empty()) png.assign(b, (size_t)api->bytes(stmt, 0));
            },
            &lower);
    IWICImagingFactory* factory = nullptr;
    if (!png.empty() &&
        SUCCEEDED(CoCreateInstance(kPhoneWicFactory, nullptr, CLSCTX_INPROC_SERVER, __uuidof(IWICImagingFactory), (void**)&factory))) {
        IStream* stream = SHCreateMemStream((BYTE const*)png.data(), (UINT)png.size());
        IWICBitmapDecoder* decoder = nullptr;
        IWICBitmapFrameDecode* frame = nullptr;
        IWICBitmapScaler* scaler = nullptr;
        IWICFormatConverter* converter = nullptr;
        if (stream && SUCCEEDED(factory->CreateDecoderFromStream(stream, nullptr, WICDecodeMetadataCacheOnDemand, &decoder)) &&
            SUCCEEDED(decoder->GetFrame(0, &frame)) && SUCCEEDED(factory->CreateBitmapScaler(&scaler)) &&
            SUCCEEDED(scaler->Initialize(frame, size, size, WICBitmapInterpolationModeFant)) &&
            SUCCEEDED(factory->CreateFormatConverter(&converter)) &&
            SUCCEEDED(converter->Initialize(scaler, kPhoneWicPbgra, WICBitmapDitherTypeNone, nullptr, 0, WICBitmapPaletteTypeCustom))) {
            BITMAPINFO bi{};
            bi.bmiHeader = {sizeof(BITMAPINFOHEADER), size, -size, 1, 32, BI_RGB};
            void* bits = nullptr;
            HBITMAP color = CreateDIBSection(nullptr, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
            if (color && SUCCEEDED(converter->CopyPixels(nullptr, size * 4, size * size * 4, (BYTE*)bits))) {
                HBITMAP mask = CreateBitmap(size, size, 1, 1, nullptr);
                ICONINFO ii{TRUE, 0, 0, mask, color};
                icon = CreateIconIndirect(&ii);
                DeleteObject(mask);
            }
            if (color) DeleteObject(color);
        }
        if (converter) converter->Release();
        if (scaler) scaler->Release();
        if (frame) frame->Release();
        if (decoder) decoder->Release();
        if (stream) stream->Release();
        factory->Release();
    }
    g_phoneAppIcons[key] = {icon, GetTickCount64()};
    return icon;
}

// The latest read, and the one under way. ReadPhoneSoon reads on its own thread and posts done to wnd.
std::mutex g_phoneLock;
PhoneInfo g_phone;
bool g_phoneRead = false;      // g_phone has been read once
std::atomic<bool> g_phoneReading{false};
HANDLE g_phoneThread = nullptr;  // the last read's (waited for before this code goes: PhoneReadWait)

struct PhoneJob {
    HWND wnd;
    UINT done;
    int notes;
    bool own;
};

DWORD WINAPI PhoneThread(LPVOID param) {
    std::unique_ptr<PhoneJob> job((PhoneJob*)param);
    PhoneInfo info = ReadPhoneInfo(job->notes, job->own);
    {
        std::lock_guard<std::mutex> lock(g_phoneLock);
        g_phone = std::move(info);
        g_phoneRead = true;
    }
    g_phoneReading = false;
    PostMessageW(job->wnd, job->done, 0, 0);
    return 0;
}

// own: Tourne holds the phone (ReadPhoneNotes).
void ReadPhoneSoon(HWND wnd, UINT done, int notes, bool own) {
    if (g_phoneReading.exchange(true)) return;
    if (g_phoneThread) {
        WaitForSingleObject(g_phoneThread, INFINITE);  // (it has said it's done: only its last instructions left)
        CloseHandle(g_phoneThread);
        g_phoneThread = nullptr;
    }
    auto* job = new PhoneJob{wnd, done, notes, own};
    if ((g_phoneThread = CreateThread(nullptr, 0, PhoneThread, job, 0, nullptr))) {
        SetThreadPriority(g_phoneThread, THREAD_PRIORITY_BELOW_NORMAL);
    } else {
        delete job;
        g_phoneReading = false;
    }
}

// The read under way finished before this code goes (the Start menu's end).
void PhoneReadWait() {
    if (!g_phoneThread) return;
    WaitForSingleObject(g_phoneThread, 10000);
    CloseHandle(g_phoneThread);
    g_phoneThread = nullptr;
}

PhoneInfo PhoneNow() {
    std::lock_guard<std::mutex> lock(g_phoneLock);
    PhoneInfo info = g_phone;
    info.read = g_phoneRead;
    return info;
}

// Whether a phone is linked: the last read's answer, else (before the first read is back) the small file's,
// looked at again after half a minute so a phone linked since isn't missed.
bool PhoneLinked() {
    {
        std::lock_guard<std::mutex> lock(g_phoneLock);
        if (g_phoneRead) return g_phone.linked;
    }
    static bool known = false;
    static ULONGLONG lookedAt = 0;
    if (!lookedAt || GetTickCount64() - lookedAt > 30000) {
        PhoneInfo info;
        ReadPhoneIdentity(info);
        known = info.linked;
        lookedAt = GetTickCount64();
    }
    return known;
}

////////////////////////////////////////////////////////////////////////////////
// Phone notifications you cleared (middle click, Clear, Clear all): kept so they don't come back. The phone sends
// every one it still has again each time it connects (signing in, a restart, Tourne or Explorer starting again),
// and phone-ancs.cpp shows only those not listed here. Each is known by its content (PhoneNoteKey: the toast's
// tag), not by the phone's number for it, which starts again with every connection. The newest 400 are kept,
// in HKCU\Software\Tourne\Phone (ClearedNotes).

constexpr wchar_t kPhoneClearedKey[] = L"Software\\Tourne\\Phone";
constexpr size_t kPhoneClearedMost = 400;

// A notification's key from what it says: the app, its words and the phone's time for it ("p" + 16 hex digits).
std::wstring PhoneNoteKey(std::wstring const& app, std::wstring const& title, std::wstring const& subtitle,
                          std::wstring const& message, std::wstring const& when) {
    unsigned long long h = 1469598103934665603ull;
    for (auto const* part : {&app, &title, &subtitle, &message, &when}) {
        for (wchar_t c : *part) h = (h ^ (unsigned)c) * 1099511628211ull;
        h = (h ^ 0x1F) * 1099511628211ull;  // (between parts)
    }
    wchar_t out[24];
    swprintf(out, 24, L"p%016llx", h);
    return out;
}

std::vector<std::wstring> PhoneClearedNotes() {
    std::vector<std::wstring> out;
    DWORD bytes = 0;
    if (RegGetValueW(HKEY_CURRENT_USER, kPhoneClearedKey, L"ClearedNotes", RRF_RT_REG_MULTI_SZ, nullptr, nullptr, &bytes) != ERROR_SUCCESS || !bytes)
        return out;
    std::vector<wchar_t> list(bytes / sizeof(wchar_t) + 2, 0);
    bytes = (DWORD)(list.size() * sizeof(wchar_t));
    if (RegGetValueW(HKEY_CURRENT_USER, kPhoneClearedKey, L"ClearedNotes", RRF_RT_REG_MULTI_SZ, nullptr, list.data(), &bytes) != ERROR_SUCCESS)
        return out;
    for (const wchar_t* p = list.data(); *p; p += wcslen(p) + 1) out.push_back(p);
    return out;
}

bool PhoneNoteCleared(std::wstring const& key) {
    auto list = PhoneClearedNotes();
    return std::find(list.begin(), list.end(), key) != list.end();
}

// Cleared: noted (only Tourne's own phone notifications have such keys; anything else is left out).
void PhoneNoteClear(std::wstring const& aumid, std::wstring const& tag) {
    if (!aumid.starts_with(L"Tourne.Phone.") || tag.size() != 17 || tag[0] != L'p') return;
    auto list = PhoneClearedNotes();
    if (std::find(list.begin(), list.end(), tag) != list.end()) return;
    list.push_back(tag);
    if (list.size() > kPhoneClearedMost) list.erase(list.begin(), list.end() - kPhoneClearedMost);
    std::wstring multi;
    for (auto const& item : list) multi += item + L'\0';
    multi += L'\0';
    RegSetKeyValueW(HKEY_CURRENT_USER, kPhoneClearedKey, L"ClearedNotes", REG_MULTI_SZ, multi.data(), (DWORD)(multi.size() * sizeof(wchar_t)));
}

// Conversations cleared from the phone pane: hidden until a text newer than the last one then comes
// (HKCU\Software\Tourne\Phone, ClearedConvos: "key|time of its last text", newest 200).
std::map<std::wstring, long long> PhoneClearedConvos() {
    std::map<std::wstring, long long> out;
    DWORD bytes = 0;
    if (RegGetValueW(HKEY_CURRENT_USER, kPhoneClearedKey, L"ClearedConvos", RRF_RT_REG_MULTI_SZ, nullptr, nullptr, &bytes) != ERROR_SUCCESS || !bytes)
        return out;
    std::vector<wchar_t> list(bytes / sizeof(wchar_t) + 2, 0);
    bytes = (DWORD)(list.size() * sizeof(wchar_t));
    if (RegGetValueW(HKEY_CURRENT_USER, kPhoneClearedKey, L"ClearedConvos", RRF_RT_REG_MULTI_SZ, nullptr, list.data(), &bytes) != ERROR_SUCCESS)
        return out;
    for (const wchar_t* p = list.data(); *p; p += wcslen(p) + 1) {
        std::wstring item = p;
        size_t bar = item.rfind(L'|');
        if (bar != std::wstring::npos) out[item.substr(0, bar)] = _wtoi64(item.c_str() + bar + 1);
    }
    return out;
}

void PhoneClearConvo(std::wstring const& key, long long when) {
    auto cleared = PhoneClearedConvos();
    cleared[key] = when;
    std::vector<std::pair<long long, std::wstring>> newest;
    for (auto const& [k, w] : cleared) newest.push_back({w, k});
    std::sort(newest.rbegin(), newest.rend());
    if (newest.size() > 200) newest.resize(200);
    std::wstring multi;
    for (auto const& [w, k] : newest) multi += k + L"|" + std::to_wstring(w) + L'\0';
    multi += L'\0';
    RegSetKeyValueW(HKEY_CURRENT_USER, kPhoneClearedKey, L"ClearedConvos", REG_MULTI_SZ, multi.data(), (DWORD)(multi.size() * sizeof(wchar_t)));
}
