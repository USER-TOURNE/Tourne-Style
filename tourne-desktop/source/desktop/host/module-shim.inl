// Windhawk's API for one module, inside its namespace (the build includes this
// right after "namespace mod_<name> {", with TD_PREFIX set to its name). The
// module's code calls these exactly as it calls Windhawk's, unchanged.
// Hooks are here only for the modules the build lets use them (besideSuite:
// Icon Redirect), and only ever in Tourne Desktop's own process (hooks.cpp). A
// module that hooks other programs belongs in the Windhawk suite.

inline int Wh_GetIntSetting(PCWSTR name, ...) {
    va_list args;
    va_start(args, name);
    std::wstring key = td::FormatKey(name, args);
    va_end(args);
    return td::GetInt(TD_PREFIX, key);
}

inline PCWSTR Wh_GetStringSetting(PCWSTR name, ...) {
    va_list args;
    va_start(args, name);
    std::wstring key = td::FormatKey(name, args);
    va_end(args);
    return td::GetString(TD_PREFIX, key);
}

inline void Wh_FreeStringSetting(PCWSTR text) { td::FreeString(text); }

inline void Wh_Log(PCWSTR format, ...) {
    va_list args;
    va_start(args, format);
    td::LogV(TD_PREFIX, format, args);
    va_end(args);
}

inline size_t Wh_GetStringValue(PCWSTR name, PWSTR buffer, size_t chars) {
    return td::GetStringValue(TD_PREFIX, name, buffer, chars);
}
inline BOOL Wh_SetStringValue(PCWSTR name, PCWSTR value) { return td::SetStringValue(TD_PREFIX, name, value); }
inline int Wh_GetIntValue(PCWSTR name, int fallback) { return td::GetIntValue(TD_PREFIX, name, fallback); }
inline BOOL Wh_SetIntValue(PCWSTR name, int value) { return td::SetIntValue(TD_PREFIX, name, value); }
inline size_t Wh_GetBinaryValue(PCWSTR name, void* buffer, size_t size) {
    return td::GetBinaryValue(TD_PREFIX, name, buffer, size);
}
inline BOOL Wh_SetBinaryValue(PCWSTR name, const void* buffer, size_t size) {
    return td::SetBinaryValue(TD_PREFIX, name, buffer, size);
}
inline BOOL Wh_DeleteValue(PCWSTR name) { return td::DeleteValue(TD_PREFIX, name); }
inline size_t Wh_GetModStoragePath(PWSTR buffer, size_t chars) { return td::GetStoragePath(TD_PREFIX, buffer, chars); }

inline BOOL Wh_SetFunctionHook(void* target, void* hook, void** original) {
    return td::SetFunctionHook(TD_PREFIX, target, hook, original);
}

// Nothing is downloaded from here: Windhawk's own process does a module's
// downloads (Icon Redirect's themes), and this one reads what it put down.
struct WH_GET_URL_CONTENT_OPTIONS {
    size_t optionsSize;
    PCWSTR targetFilePath;
};
struct WH_URL_CONTENT {
    const char* data;
    size_t length;
    int statusCode;
};
inline const WH_URL_CONTENT* Wh_GetUrlContent(PCWSTR, const WH_GET_URL_CONTENT_OPTIONS*) { return nullptr; }
inline void Wh_FreeUrlContent(const WH_URL_CONTENT*) {}

// The few Windhawk helpers that need nothing of Windhawk's.
namespace WindhawkUtils {

template <typename Prototype>
inline BOOL SetFunctionHook(Prototype* target, Prototype* hook, Prototype** original) {
    return Wh_SetFunctionHook((void*)target, (void*)hook, (void**)original);
}

// A string setting, freed when it goes.
class StringSetting {
   public:
    StringSetting() = default;
    explicit StringSetting(PCWSTR text) : text_(text) {}
    StringSetting(StringSetting&& other) noexcept : text_(other.text_) { other.text_ = nullptr; }
    StringSetting& operator=(StringSetting&& other) noexcept {
        if (this != &other) {
            if (text_) Wh_FreeStringSetting(text_);
            text_ = other.text_;
            other.text_ = nullptr;
        }
        return *this;
    }
    StringSetting(const StringSetting&) = delete;
    StringSetting& operator=(const StringSetting&) = delete;
    ~StringSetting() {
        if (text_) Wh_FreeStringSetting(text_);
    }
    static StringSetting make(PCWSTR name, ...) {
        va_list args;
        va_start(args, name);
        std::wstring key = td::FormatKey(name, args);
        va_end(args);
        return StringSetting(td::GetString(TD_PREFIX, key));
    }
    PCWSTR get() const { return text_; }
    operator PCWSTR() const { return text_; }

   private:
    PCWSTR text_ = nullptr;
};

// Subclassing a window of another thread in this process: the subclass is
// set on the window's own thread (a message hook there runs it), as
// Windhawk's helper does. The window procedure has no subclass id; the
// procedure itself is used as one.
using WH_SUBCLASSPROC = LRESULT(CALLBACK*)(HWND, UINT, WPARAM, LPARAM, DWORD_PTR);

struct SubclassCall {
    bool set;
    HWND wnd;
    WH_SUBCLASSPROC proc;
    DWORD_PTR data;
    BOOL result = FALSE;
};
constexpr WPARAM kSubclassCallTag = 0x54445343;  // "TDSC"

inline LRESULT CALLBACK SubclassTrampoline(HWND wnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR id, DWORD_PTR data) {
    return ((WH_SUBCLASSPROC)id)(wnd, msg, wp, lp, data);
}
inline UINT SubclassMessage() {
    static UINT message = RegisterWindowMessageW(L"TourneDesktop_SubclassCall");
    return message;
}
inline void RunSubclassCall(SubclassCall& c) {
    c.result = c.set ? SetWindowSubclass(c.wnd, SubclassTrampoline, (UINT_PTR)c.proc, c.data)
                     : RemoveWindowSubclass(c.wnd, SubclassTrampoline, (UINT_PTR)c.proc);
}
inline LRESULT CALLBACK SubclassHook(int code, WPARAM wp, LPARAM lp) {
    if (code == HC_ACTION) {
        auto* sent = (CWPSTRUCT*)lp;
        if (sent->message == SubclassMessage() && sent->wParam == kSubclassCallTag) RunSubclassCall(*(SubclassCall*)sent->lParam);
    }
    return CallNextHookEx(nullptr, code, wp, lp);
}
inline BOOL SubclassOnItsThread(SubclassCall c) {
    DWORD pid = 0, thread = GetWindowThreadProcessId(c.wnd, &pid);
    if (!thread || pid != GetCurrentProcessId()) return FALSE;  // (another program's windows: not from here)
    if (thread == GetCurrentThreadId()) {
        RunSubclassCall(c);
        return c.result;
    }
    HHOOK hook = SetWindowsHookExW(WH_CALLWNDPROC, SubclassHook, nullptr, thread);
    if (!hook) return FALSE;
    DWORD_PTR ignored = 0;
    SendMessageTimeoutW(c.wnd, SubclassMessage(), kSubclassCallTag, (LPARAM)&c, SMTO_NORMAL, 2000, &ignored);
    UnhookWindowsHookEx(hook);
    return c.result;
}
inline BOOL SetWindowSubclassFromAnyThread(HWND wnd, WH_SUBCLASSPROC proc, void* data) {
    return SubclassOnItsThread({true, wnd, proc, (DWORD_PTR)data});
}
inline BOOL RemoveWindowSubclassFromAnyThread(HWND wnd, WH_SUBCLASSPROC proc) {
    return SubclassOnItsThread({false, wnd, proc, 0});
}

}  // namespace WindhawkUtils
