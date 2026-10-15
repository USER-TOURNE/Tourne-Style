// Tourne Calls: your phone's calls through this PC, as Phone Link makes them. Windows does the calling itself (the PC
// is the phone's Bluetooth hands-free: its "Hands-Free HF Audio" speaker and microphone appear during a call), but
// lets only a program with a package identity and the phoneLineTransportManagement / phoneCallSystem
// capabilities drive it. So this is a program of its own, registered as a local package beside its files
// (AppxManifest.xml, Register-TourneCalls.ps1; Developer Mode on), and Tourne Desktop asks it.
//
//   TourneCalls.exe status       what Windows allows it, the phone's line and its calls (to calls.log)
//   TourneCalls.exe register     registers it for the phone's hands-free and connects it
//   TourneCalls.exe unregister   gives the phone's hands-free back (to Phone Link, if it's run again)
//   TourneCalls.exe run          stays: follows the phone's calls, does Tourne Desktop's commands, and while a
//                                call's sound is on the PC carries it to and from the PC's own speakers and mic
//
// Commands (run): the registered message "TourneCallsCommand" to its window (class "TourneCalls_Window"), wParam
// 1 answer, 2 decline or hang up, 3 mute or unmute, 4 sound to the PC, 5 sound to the phone; WM_COPYDATA 6 with
// a number dials it; 9: other devices chosen (HKCU\Software\Tourne\Calls Output / Mic); 10: tell Tourne Desktop what's on again. Its log:
// %LOCALAPPDATA%\Tourne\Desktop\calls.log. While a call is on, Tourne Desktop shows it (TellTourne), else its own pop-up (UpdatePopup).
#define _WIN32_WINNT 0x0A00  // (per-monitor dpi)
#include <windows.h>
#include <shlobj.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <functiondiscoverykeys_devpkey.h>
#include <dwmapi.h>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdio>
#include <map>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <vector>

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.ApplicationModel.h>
#include <winrt/Windows.ApplicationModel.Calls.h>
#include <winrt/Windows.Devices.Enumeration.h>

using namespace winrt;
using namespace winrt::Windows::ApplicationModel::Calls;
using namespace winrt::Windows::Devices::Enumeration;

std::wstring LogFile() {
    WCHAR path[MAX_PATH] = L"";
    SHGetFolderPathW(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, path);
    return std::wstring(path) + L"\\Tourne\\Desktop\\calls.log";
}

void Log(wchar_t const* format, ...) {
    WCHAR text[1024];
    va_list args;
    va_start(args, format);
    vswprintf(text, ARRAYSIZE(text), format, args);
    va_end(args);
    SYSTEMTIME t;
    GetLocalTime(&t);
    WCHAR line[1100];
    swprintf(line, ARRAYSIZE(line), L"%02u:%02u:%02u.%03u %ls\r\n", t.wHour, t.wMinute, t.wSecond, t.wMilliseconds, text);
    int n = WideCharToMultiByte(CP_UTF8, 0, line, -1, nullptr, 0, nullptr, nullptr);
    std::string utf8(n > 0 ? n - 1 : 0, '\0');
    WideCharToMultiByte(CP_UTF8, 0, line, -1, utf8.data(), n, nullptr, nullptr);
    HANDLE h = CreateFileW(LogFile().c_str(), FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_ALWAYS, 0, nullptr);
    if (h != INVALID_HANDLE_VALUE) {
        DWORD wrote = 0;
        WriteFile(h, utf8.data(), (DWORD)utf8.size(), &wrote, nullptr);
        CloseHandle(h);
    }
}

template <typename F>
void Try(wchar_t const* what, F f) {
    try {
        f();
    } catch (hresult_error const& e) {
        Log(L"  %ls: error 0x%08X %ls", what, (unsigned)e.code(), e.message().c_str());
    } catch (...) {
        Log(L"  %ls: failed", what);
    }
}

// The phone's hands-free transport (the first Bluetooth one).
PhoneLineTransportDevice Transport() {
    for (auto const& d : DeviceInformation::FindAllAsync(PhoneLineTransportDevice::GetDeviceSelector(PhoneLineTransport::Bluetooth)).get()) {
        Log(L"transport: %ls", d.Name().c_str());
        return PhoneLineTransportDevice::FromId(d.Id());
    }
    return nullptr;
}

void Status() {
    Try(L"identity", [] { Log(L"identity: %ls", winrt::Windows::ApplicationModel::Package::Current().Id().FullName().c_str()); });
    Try(L"transport", [] {
        auto t = Transport();
        if (!t) return Log(L"  none");
        Log(L"  registered: %d, audio routing: %d, in-band ringing: %d", t.IsRegistered(), (int)t.AudioRoutingStatus(), t.InBandRingingEnabled());
        Log(L"  access: %d (1 allowed, 2 denied by user, 3 by system)", (int)t.RequestAccessAsync().get());
    });
    Try(L"lines", [] {
        auto store = PhoneCallManager::RequestStoreAsync().get();
        Log(L"lines: store ok");
        auto watcher = store.RequestLineWatcher();
        watcher.LineAdded([](auto&&, PhoneLineWatcherEventArgs const& a) {
            Try(L"line", [&] {
                auto line = PhoneLine::FromIdAsync(a.LineId()).get();
                Log(L"  line %ls: transport %d, can dial %d, network %d", line.DisplayName().c_str(), (int)line.Transport(), line.CanDial(),
                    (int)line.NetworkState());
                Try(L"active calls", [&] {
                    auto calls = line.GetAllActivePhoneCallsAsync().get();
                    Log(L"    active calls: status %d, %u", (int)calls.OperationStatus(), calls.AllActivePhoneCalls().Size());
                });
            });
        });
        watcher.Start();
        Sleep(4000);
        watcher.Stop();
    });
}

void Register(bool on) {
    Try(on ? L"register" : L"unregister", [&] {
        auto t = Transport();
        if (!t) return Log(L"  no transport");
        Log(L"  access: %d", (int)t.RequestAccessAsync().get());
        if (!on) {
            t.UnregisterApp();
            Log(L"  unregistered: %d", !t.IsRegistered());
            return;
        }
        if (!t.IsRegistered()) t.RegisterApp();
        Log(L"  registered: %d", t.IsRegistered());
        Log(L"  connected: %d", t.ConnectAsync().get());
        Log(L"  audio routing: %d", (int)t.AudioRoutingStatus());
    });
}

////////////////////////////////////////////////////////////////////////////////
// The call's sound on the PC: Windows gives the phone's side as the hands-free "Microphone" (what the caller
// says) and takes yours as its "Speakers"; nothing plays them by themselves. So, while a call's sound is on the
// PC, the caller goes to the PC's own output for calls (its default communications device, else the default) and
// the PC's own mic to the caller. Both as communications streams, so a device's echo cancelling applies.

struct Endpoints {
    IMMDevice* phoneVoice = nullptr;  // the caller (capture)
    IMMDevice* phoneEar = nullptr;    // to the caller (render)
    IMMDevice* speakers = nullptr;    // the PC's own
    IMMDevice* mic = nullptr;
    void Release() {
        for (IMMDevice** d : {&phoneVoice, &phoneEar, &speakers, &mic})
            if (*d) (*d)->Release(), *d = nullptr;
    }
};

std::wstring DeviceName(IMMDevice* d) {
    std::wstring name;
    IPropertyStore* props = nullptr;
    if (d && SUCCEEDED(d->OpenPropertyStore(STGM_READ, &props))) {
        PROPVARIANT v;
        PropVariantInit(&v);
        if (SUCCEEDED(props->GetValue(PKEY_Device_FriendlyName, &v)) && v.vt == VT_LPWSTR) name = v.pwszVal;
        PropVariantClear(&v);
        props->Release();
    }
    return name;
}

bool IsHandsFree(IMMDevice* d) { return DeviceName(d).find(L"Hands-Free") != std::wstring::npos; }

// The device chosen in Tourne Desktop's settings (Calls: sound out, Calls: microphone; HKCU\Software\Tourne\Calls
// Output / Mic, an endpoint's id), if it's there and working.
IMMDevice* ChosenDevice(IMMDeviceEnumerator* e, wchar_t const* value) {
    WCHAR id[512] = L"";
    DWORD size = sizeof(id);
    if (RegGetValueW(HKEY_CURRENT_USER, L"Software\\Tourne\\Calls", value, RRF_RT_REG_SZ, nullptr, id, &size) != ERROR_SUCCESS || !id[0])
        return nullptr;
    IMMDevice* d = nullptr;
    DWORD state = 0;
    if (FAILED(e->GetDevice(id, &d)) || !d) return nullptr;
    if (FAILED(d->GetState(&state)) || state != DEVICE_STATE_ACTIVE) {
        Log(L"sound: the chosen %ls isn't there now, Windows' default instead", value);
        d->Release();
        return nullptr;
    }
    return d;
}

// The PC's own device of a kind: the one chosen, else the default for calls, else the default, else the first that
// isn't a phone's.
IMMDevice* OwnDevice(IMMDeviceEnumerator* e, EDataFlow flow) {
    if (IMMDevice* chosen = ChosenDevice(e, flow == eRender ? L"Output" : L"Mic")) return chosen;
    for (ERole role : {eCommunications, eConsole}) {
        IMMDevice* d = nullptr;
        if (SUCCEEDED(e->GetDefaultAudioEndpoint(flow, role, &d)) && d) {
            if (!IsHandsFree(d)) return d;
            d->Release();
        }
    }
    IMMDeviceCollection* all = nullptr;
    IMMDevice* found = nullptr;
    UINT count = 0;
    if (SUCCEEDED(e->EnumAudioEndpoints(flow, DEVICE_STATE_ACTIVE, &all)) && SUCCEEDED(all->GetCount(&count)))
        for (UINT i = 0; i < count && !found; i++) {
            IMMDevice* d = nullptr;
            if (SUCCEEDED(all->Item(i, &d)) && !IsHandsFree(d)) found = d;
            else if (d) d->Release();
        }
    if (all) all->Release();
    return found;
}

IMMDevice* HandsFree(IMMDeviceEnumerator* e, EDataFlow flow) {
    IMMDeviceCollection* all = nullptr;
    IMMDevice* found = nullptr;
    UINT count = 0;
    if (SUCCEEDED(e->EnumAudioEndpoints(flow, DEVICE_STATE_ACTIVE, &all)) && SUCCEEDED(all->GetCount(&count)))
        for (UINT i = 0; i < count && !found; i++) {
            IMMDevice* d = nullptr;
            if (SUCCEEDED(all->Item(i, &d)) && IsHandsFree(d)) found = d;
            else if (d) d->Release();
        }
    if (all) all->Release();
    return found;
}

// One direction, from a capture device to a render device, until stop: 48 kHz stereo 16-bit (Windows converts
// to and from each device's own), with at most about 80 ms kept waiting so the call doesn't lag. Its loudness goes
// to level (the pop-up's meter); while silence is set it sends quiet instead (the mic, muted).
void Pump(IMMDevice* from, IMMDevice* to, std::atomic<bool>* stop, wchar_t const* what, std::atomic<int>* level,
          std::atomic<bool>* silence) {
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    IAudioClient *in = nullptr, *out = nullptr;
    IAudioCaptureClient* capture = nullptr;
    IAudioRenderClient* render = nullptr;
    WAVEFORMATEX f{WAVE_FORMAT_PCM, 2, 48000, 48000 * 4, 4, 16, 0};
    DWORD flags = AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM | AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY;
    UINT32 outFrames = 0;
    auto communications = [](IAudioClient* c) {
        IAudioClient2* c2 = nullptr;
        if (SUCCEEDED(c->QueryInterface(__uuidof(IAudioClient2), (void**)&c2))) {
            AudioClientProperties p{sizeof(p), FALSE, AudioCategory_Communications, AUDCLNT_STREAMOPTIONS_NONE};
            c2->SetClientProperties(&p);
            c2->Release();
        }
    };
    bool ok = SUCCEEDED(from->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, (void**)&in)) &&
              SUCCEEDED(to->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, (void**)&out));
    if (ok) {
        communications(in);
        communications(out);
        ok = SUCCEEDED(in->Initialize(AUDCLNT_SHAREMODE_SHARED, flags, 2000000, 0, &f, nullptr)) &&
             SUCCEEDED(out->Initialize(AUDCLNT_SHAREMODE_SHARED, flags, 2000000, 0, &f, nullptr)) &&
             SUCCEEDED(in->GetService(__uuidof(IAudioCaptureClient), (void**)&capture)) &&
             SUCCEEDED(out->GetService(__uuidof(IAudioRenderClient), (void**)&render)) && SUCCEEDED(out->GetBufferSize(&outFrames)) &&
             SUCCEEDED(in->Start()) && SUCCEEDED(out->Start());
    }
    Log(L"sound %ls: %ls -> %ls%ls", what, DeviceName(from).c_str(), DeviceName(to).c_str(), ok ? L"" : L" (couldn't start)");
    std::vector<int16_t> waiting;  // frames * 2
    UINT64 moved = 0;
    while (ok && !*stop) {
        Sleep(10);
        UINT32 packet = 0;
        int peak = -1;
        while (SUCCEEDED(capture->GetNextPacketSize(&packet)) && packet) {
            BYTE* data = nullptr;
            UINT32 frames = 0;
            DWORD bufferFlags = 0;
            if (FAILED(capture->GetBuffer(&data, &frames, &bufferFlags, nullptr, nullptr))) break;
            size_t at = waiting.size();
            waiting.resize(at + frames * 2);
            if ((bufferFlags & AUDCLNT_BUFFERFLAGS_SILENT) || (silence && *silence)) std::fill(waiting.begin() + at, waiting.end(), 0);
            else memcpy(waiting.data() + at, data, frames * 4);
            capture->ReleaseBuffer(frames);
            for (size_t i = at; i < waiting.size(); i++) peak = std::max(peak, std::abs((int)waiting[i]));
        }
        if (peak >= 0) *level = std::max(level->load(), peak);
        size_t most = 48000 * 80 / 1000 * 2;
        if (waiting.size() > most) waiting.erase(waiting.begin(), waiting.end() - most);
        UINT32 padding = 0;
        if (FAILED(out->GetCurrentPadding(&padding))) break;
        UINT32 room = outFrames - padding, frames = std::min<UINT32>(room, (UINT32)(waiting.size() / 2));
        BYTE* data = nullptr;
        if (frames && SUCCEEDED(render->GetBuffer(frames, &data))) {
            memcpy(data, waiting.data(), frames * 4);
            render->ReleaseBuffer(frames, 0);
            waiting.erase(waiting.begin(), waiting.begin() + frames * 2);
            moved += frames;
        }
    }
    Log(L"sound %ls: stopped after %llu ms", what, moved * 1000 / 48000);
    if (in) in->Stop();
    if (out) out->Stop();
    for (IUnknown* u : {(IUnknown*)capture, (IUnknown*)render, (IUnknown*)in, (IUnknown*)out})
        if (u) u->Release();
    CoUninitialize();
}

std::atomic<bool> g_micOff{false};             // muted: the mic's pump sends quiet
std::atomic<int> g_levelMic{0}, g_levelCaller{0};  // the loudest since the pop-up last looked (0 to 32767)
std::wstring g_micName, g_outName;             // the bridge's devices while it runs

struct Bridge {
    std::atomic<bool> stop{false};
    std::thread toCaller, fromCaller;
    Endpoints e;
    bool on = false;

    void Start() {
        if (on) return;
        IMMDeviceEnumerator* devices = nullptr;
        if (FAILED(CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&devices))) return;
        e.phoneVoice = HandsFree(devices, eCapture);
        e.phoneEar = HandsFree(devices, eRender);
        e.speakers = OwnDevice(devices, eRender);
        e.mic = OwnDevice(devices, eCapture);
        devices->Release();
        if (!e.phoneVoice || !e.phoneEar || !e.speakers || !e.mic) {
            Log(L"sound: missing a device (phone's voice %d, phone's ear %d, speakers %d, mic %d)", !!e.phoneVoice, !!e.phoneEar, !!e.speakers, !!e.mic);
            e.Release();
            return;
        }
        stop = false;
        on = true;
        g_micName = DeviceName(e.mic);
        g_outName = DeviceName(e.speakers);
        fromCaller = std::thread(Pump, e.phoneVoice, e.speakers, &stop, L"from the caller", &g_levelCaller, nullptr);
        toCaller = std::thread(Pump, e.mic, e.phoneEar, &stop, L"to the caller", &g_levelMic, &g_micOff);
    }

    void Stop() {
        if (!on) return;
        stop = true;
        fromCaller.join();
        toCaller.join();
        e.Release();
        on = false;
        g_micName.clear();
        g_outName.clear();
        g_levelMic = g_levelCaller = 0;
    }
};

////////////////////////////////////////////////////////////////////////////////
// The calls, followed.

std::mutex g_lock;
PhoneLine g_line{nullptr};
std::map<std::wstring, PhoneCall> g_calls;  // by CallId
std::map<std::wstring, PhoneCall> g_heard;  // the copies whose events come to the window (kept, or the events stop)
std::wstring g_seen;                        // the calls' state at the last Follow
std::set<std::wstring> g_answered;          // answered here, their sound still to come to the PC
std::map<std::wstring, std::wstring> g_names;  // who each call is with
std::map<std::wstring, ULONGLONG> g_since;     // when each call began to talk
void Command(int what);
void UpdatePopup();
Bridge g_bridge;
HWND g_window;
constexpr UINT WM_APP_CHANGED = WM_APP + 1;

wchar_t const* StatusName(PhoneCallStatus s) {
    static wchar_t const* names[] = {L"lost", L"incoming", L"dialing", L"talking", L"held", L"ended"};
    return (int)s >= 0 && (int)s < 6 ? names[(int)s] : L"?";
}

// The calls as Windows has them now: new ones followed (their changes come to the window), ended ones let go.
void Refresh(wchar_t const* why) {
    if (!g_line) return;
    try {
        auto result = g_line.GetAllActivePhoneCallsAsync().get();
        std::lock_guard<std::mutex> lock(g_lock);
        std::map<std::wstring, PhoneCall> now;
        for (auto const& call : result.AllActivePhoneCalls()) {
            std::wstring id(call.CallId());
            if (!g_calls.count(id)) {
                auto info = call.GetPhoneCallInfo();
                g_names[id] = !info.DisplayName().empty() ? std::wstring(info.DisplayName()) : std::wstring(info.PhoneNumber());
                Log(L"call %ls (%ls): %ls, %ls, sound on the %ls", id.c_str(), why, info.CallDirection() == PhoneCallDirection::Incoming ? L"in" : L"out",
                    StatusName(call.Status()), call.AudioDevice() == PhoneCallAudioDevice::LocalDevice ? L"PC" : L"phone");
                auto changed = [](auto&&, auto&&) { PostMessageW(g_window, WM_APP_CHANGED, 0, 0); };
                call.StatusChanged(changed);
                call.AudioDeviceChanged(changed);
                call.IsMutedChanged(changed);
                g_heard.emplace(id, call);
            }
            now.emplace(id, call);
        }
        for (auto const& [id, call] : g_calls)
            if (!now.count(id)) {
                Log(L"call %ls: gone", id.c_str());
                g_names.erase(id);
                g_since.erase(id);
            }
        for (auto i = g_heard.begin(); i != g_heard.end();) i = now.count(i->first) ? std::next(i) : g_heard.erase(i);
        g_calls = std::move(now);
    } catch (hresult_error const& e) {
        Log(L"calls: error 0x%08X", (unsigned)e.code());
    }
}

// The calls' state in short, to tell when it changed without an event.
std::wstring Seen() {
    std::lock_guard<std::mutex> lock(g_lock);
    std::wstring s;
    for (auto const& [id, call] : g_calls)
        s += id + L":" + std::to_wstring((int)call.Status()) + std::to_wstring((int)call.AudioDevice()) + std::to_wstring(call.IsMuted()) + L";";
    return s;
}

// What a change means for the sound: on the PC while a call talks there, else not.
void Follow() {
    bool talkingHere = false;
    g_seen = Seen();
    {
        std::lock_guard<std::mutex> lock(g_lock);
        for (auto const& [id, call] : g_calls) {
            Log(L"call %ls: %ls, sound on the %ls%ls", id.c_str(), StatusName(call.Status()),
                call.AudioDevice() == PhoneCallAudioDevice::LocalDevice ? L"PC" : L"phone", call.IsMuted() ? L", muted" : L"");
            if (call.Status() == PhoneCallStatus::Talking && call.AudioDevice() == PhoneCallAudioDevice::LocalDevice) talkingHere = true;
        }
    }
    // A call answered here gets its sound on the PC once it talks (moved while still ringing, the first one dropped).
    for (auto i = g_answered.begin(); i != g_answered.end();) {
        PhoneCall call{nullptr};
        {
            std::lock_guard<std::mutex> lock(g_lock);
            if (auto c = g_calls.find(*i); c != g_calls.end()) call = c->second;
        }
        if (!call || call.Status() == PhoneCallStatus::Ended) {
            i = g_answered.erase(i);
            continue;
        }
        if (call.Status() != PhoneCallStatus::Talking) {
            ++i;
            continue;
        }
        i = g_answered.erase(i);
        if (call.AudioDevice() == PhoneCallAudioDevice::LocalDevice) continue;
        try {
            call.ChangeAudioDeviceAsync(PhoneCallAudioDevice::LocalDevice).get();
            Log(L"call %ls: sound moved to the PC", std::wstring(call.CallId()).c_str());
            PostMessageW(g_window, WM_APP_CHANGED, 0, 0);
        } catch (hresult_error const& e) {
            Log(L"call: moving the sound, error 0x%08X", (unsigned)e.code());
        }
    }
    if (talkingHere) g_bridge.Start();
    else g_bridge.Stop();
    UpdatePopup();
}

// The call a command is for: the ringing one, else the one talking or held.
PhoneCall Current() {
    std::lock_guard<std::mutex> lock(g_lock);
    for (PhoneCallStatus want : {PhoneCallStatus::Incoming, PhoneCallStatus::Talking, PhoneCallStatus::Dialing, PhoneCallStatus::Held})
        for (auto const& [id, call] : g_calls)
            if (call.Status() == want) return call;
    return nullptr;
}

void Command(int what) {
    Refresh(L"a command");
    if (what == 10) {  // Tourne Desktop (re)started or its look changed: what's on, again
        UpdatePopup();
        return;
    }
    PhoneCall call = Current();
    Log(L"command %d%ls", what, call ? L"" : L" (no call)");
    if (!call) return;  // (9, other devices chosen: read at the next call's sound)
    try {
        switch (what) {
            case 1:  // answer, with its sound on the PC (moved by Follow once it talks)
                g_answered.insert(std::wstring(call.CallId()));
                call.AcceptIncomingAsync().get();
                break;
            case 2:
                if (call.Status() == PhoneCallStatus::Incoming) call.RejectIncomingAsync().get();
                else call.EndAsync().get();
                break;
            case 3:  // the mic: quiet from the bridge (what counts while the sound is on the PC), and the call's own mute
                g_micOff = !g_micOff;
                Log(L"mic %ls", g_micOff ? L"off" : L"on");
                if (g_micOff) call.MuteAsync().get();
                else call.UnmuteAsync().get();
                break;
            case 4: call.ChangeAudioDeviceAsync(PhoneCallAudioDevice::LocalDevice).get(); break;
            case 9: g_bridge.Stop(); break;  // (other devices chosen: started again with them, below)
            case 5: call.ChangeAudioDeviceAsync(PhoneCallAudioDevice::RemoteDevice).get(); break;
        }
    } catch (hresult_error const& e) {
        Log(L"command %d: error 0x%08X %ls", what, (unsigned)e.code(), e.message().c_str());
    }
    Follow();
}

////////////////////////////////////////////////////////////////////////////////
// The pop-up while a call is on (not while it rings: Tourne's notification has Answer and Decline then): who, how
// long, where its sound is and, on the PC, the mic and output by name with their levels as they go, so you can see
// the caller hears this mic. Mute, Sound to the phone (or the PC), Hang up. Bottom right, over everything, never
// taking the focus.

HWND g_popup;
struct Shown {
    bool on = false;
    std::wstring id, name;
    PhoneCallStatus status{};
    bool here = false;
} g_shown;
struct PopupButton {
    RECT r;
    int command;
    wchar_t const* text;
    bool danger, lit;
};
std::vector<PopupButton> g_popupButtons;
int g_popupHover = -1;
float g_meterMic = 0, g_meterCaller = 0;

constexpr int kPopupW = 360, kPopupH = 168;  // at 96 dpi

bool LightTheme() {
    DWORD light = 0, size = sizeof(light);
    RegGetValueW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", L"SystemUsesLightTheme",
                 RRF_RT_REG_DWORD, nullptr, &light, &size);
    return light != 0;
}

void PlacePopup() {
    MONITORINFO mi{sizeof(mi)};
    GetMonitorInfoW(MonitorFromPoint({0, 0}, MONITOR_DEFAULTTOPRIMARY), &mi);
    for (int pass = 0; pass < 2; pass++) {  // (the second with the size at the monitor's dpi)
        int dpi = GetDpiForWindow(g_popup), w = MulDiv(kPopupW, dpi, 96), h = MulDiv(kPopupH, dpi, 96), m = MulDiv(12, dpi, 96);
        SetWindowPos(g_popup, HWND_TOPMOST, mi.rcWork.right - w - m, mi.rcWork.bottom - h - m, w, h, SWP_NOACTIVATE);
    }
    BOOL dark = !LightTheme();
    DwmSetWindowAttribute(g_popup, 20 /* DWMWA_USE_IMMERSIVE_DARK_MODE */, &dark, sizeof(dark));
}

void PaintPopup(HDC target) {
    RECT client;
    GetClientRect(g_popup, &client);
    int dpi = GetDpiForWindow(g_popup);
    auto S = [dpi](int v) { return MulDiv(v, dpi, 96); };
    bool light = LightTheme();
    COLORREF back = light ? RGB(249, 249, 249) : RGB(32, 32, 32), text = light ? RGB(20, 20, 20) : RGB(255, 255, 255),
             sub = light ? RGB(96, 96, 96) : RGB(170, 170, 170), button = light ? RGB(230, 230, 230) : RGB(55, 55, 55),
             hover = light ? RGB(218, 218, 218) : RGB(70, 70, 70), track = light ? RGB(214, 214, 214) : RGB(66, 66, 66),
             accent = RGB(232, 128, 136), danger = RGB(196, 43, 28), dangerHover = RGB(220, 62, 46);
    HDC dc = CreateCompatibleDC(target);
    HBITMAP bitmap = CreateCompatibleBitmap(target, client.right, client.bottom), oldBitmap = (HBITMAP)SelectObject(dc, bitmap);
    HBRUSH fill = CreateSolidBrush(back);
    FillRect(dc, &client, fill);
    DeleteObject(fill);
    SetBkMode(dc, TRANSPARENT);
    auto font = [&](int size, int weight) {
        return CreateFontW(-S(size), 0, 0, 0, weight, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI Variable Text");
    };
    HFONT big = font(15, FW_SEMIBOLD), small = font(12, FW_NORMAL), oldFont = (HFONT)SelectObject(dc, big);
    auto write = [&](std::wstring const& s, RECT r, COLORREF c, HFONT f, UINT how) {
        SelectObject(dc, f);
        SetTextColor(dc, c);
        DrawTextW(dc, s.c_str(), -1, &r, how | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX);
    };
    auto box = [&](RECT r, COLORREF c, int round) {
        HBRUSH b = CreateSolidBrush(c);
        HPEN p = CreatePen(PS_SOLID, 1, c);
        HGDIOBJ ob = SelectObject(dc, b), op = SelectObject(dc, p);
        RoundRect(dc, r.left, r.top, r.right, r.bottom, round, round);
        SelectObject(dc, ob);
        SelectObject(dc, op);
        DeleteObject(b);
        DeleteObject(p);
    };
    int pad = S(16), right = client.right - pad;

    // Who, and how long.
    std::wstring time;
    if (auto since = g_since.find(g_shown.id); since != g_since.end() && g_shown.status != PhoneCallStatus::Dialing) {
        ULONGLONG s = (GetTickCount64() - since->second) / 1000;
        WCHAR t[32];
        swprintf(t, ARRAYSIZE(t), s >= 3600 ? L"%llu:%02llu:%02llu" : L"%llu:%02llu", s >= 3600 ? s / 3600 : s / 60, s >= 3600 ? s / 60 % 60 : s % 60,
                 s % 60);
        time = t;
    }
    write(time, {right - S(70), S(14), right, S(36)}, sub, small, DT_RIGHT);
    write(g_shown.name, {pad, S(14), right - S(76), S(36)}, text, big, DT_LEFT);
    std::wstring state = g_shown.status == PhoneCallStatus::Dialing ? L"Calling"
                         : g_shown.status == PhoneCallStatus::Held  ? L"On hold"
                         : g_shown.here                              ? L"On this PC"
                                                                     : L"On the phone";
    if (g_micOff && g_shown.here) state += L", mic off";
    write(state, {pad, S(36), right, S(56)}, g_micOff && g_shown.here ? accent : sub, small, DT_LEFT);

    // Where the sound is.
    if (g_shown.here && g_bridge.on) {
        auto row = [&](int y, wchar_t const* label, std::wstring const& name, float level) {
            write(label, {pad, y, pad + S(36), y + S(20)}, sub, small, DT_LEFT);
            write(name, {pad + S(36), y, right - S(84), y + S(20)}, text, small, DT_LEFT);
            RECT bar{right - S(72), y + S(8), right, y + S(12)};
            box(bar, track, S(4));
            if (level > 0.02f) box({bar.left, bar.top, bar.left + std::max<LONG>(S(4), (LONG)((bar.right - bar.left) * level)), bar.bottom}, accent, S(4));
        };
        row(S(62), L"Mic", g_micName, g_micOff ? 0.f : g_meterMic);
        row(S(86), L"Out", g_outName, g_meterCaller);
    } else {
        write(!g_shown.here ? L"To this PC: talk with its mic and speakers" : g_shown.status == PhoneCallStatus::Talking ? L"The sound couldn't start here (see calls.log)" : L"Its sound comes here when it starts", {pad, S(62), right, S(82)}, sub, small, DT_LEFT);
    }

    // The buttons.
    int top = S(120), height = S(32), gap = S(8), w = (client.right - 2 * pad - 2 * gap) / 3;
    g_popupButtons = {
        {{pad, top, pad + w, top + height}, 3, g_micOff ? L"Unmute" : L"Mute", false, g_micOff.load()},
        {{pad + w + gap, top, pad + 2 * w + gap, top + height}, g_shown.here ? 5 : 4, g_shown.here ? L"To the phone" : L"To this PC", false, false},
        {{pad + 2 * (w + gap), top, right, top + height}, 2, L"Hang up", true, false},
    };
    for (size_t i = 0; i < g_popupButtons.size(); i++) {
        auto const& b = g_popupButtons[i];
        bool over = (int)i == g_popupHover;
        box(b.r, b.danger ? (over ? dangerHover : danger) : b.lit ? accent : over ? hover : button, S(8));
        write(b.text, b.r, b.danger || b.lit ? RGB(255, 255, 255) : text, small, DT_CENTER);
    }

    BitBlt(target, 0, 0, client.right, client.bottom, dc, 0, 0, SRCCOPY);
    SelectObject(dc, oldFont);
    DeleteObject(big);
    DeleteObject(small);
    SelectObject(dc, oldBitmap);
    DeleteObject(bitmap);
    DeleteDC(dc);
}

LRESULT CALLBACK PopupProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_MOUSEACTIVATE:
            return MA_NOACTIVATE;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC dc = BeginPaint(wnd, &ps);
            PaintPopup(dc);
            EndPaint(wnd, &ps);
            return 0;
        }
        case WM_ERASEBKGND:
            return 1;
        case WM_TIMER: {  // the meters (fall back slowly, so a word shows) and the time
            auto meter = [](std::atomic<int>& level, float& shown) {
                float now = std::sqrt(level.exchange(0) / 32767.f);
                shown = std::max(now, shown * 0.8f);
            };
            meter(g_levelMic, g_meterMic);
            meter(g_levelCaller, g_meterCaller);
            InvalidateRect(wnd, nullptr, FALSE);
            return 0;
        }
        case WM_MOUSEMOVE: {
            POINT p{(short)LOWORD(lParam), (short)HIWORD(lParam)};
            int over = -1;
            for (size_t i = 0; i < g_popupButtons.size(); i++)
                if (PtInRect(&g_popupButtons[i].r, p)) over = (int)i;
            if (over != g_popupHover) g_popupHover = over, InvalidateRect(wnd, nullptr, FALSE);
            TRACKMOUSEEVENT t{sizeof(t), TME_LEAVE, wnd, 0};
            TrackMouseEvent(&t);
            return 0;
        }
        case WM_MOUSELEAVE:
            g_popupHover = -1;
            InvalidateRect(wnd, nullptr, FALSE);
            return 0;
        case WM_LBUTTONUP: {
            POINT p{(short)LOWORD(lParam), (short)HIWORD(lParam)};
            for (auto const& b : g_popupButtons)
                if (PtInRect(&b.r, p)) {
                    Command(b.command);
                    break;
                }
            return 0;
        }
        case WM_DPICHANGED:
        case WM_SETTINGCHANGE:
            if (g_shown.on) PlacePopup();
            InvalidateRect(wnd, nullptr, FALSE);
            return 0;
    }
    return DefWindowProcW(wnd, msg, wParam, lParam);
}

// The call as shown, to Tourne Desktop's Start window (start-menu.cpp, CallChanged; the same layout there). False
// when it isn't running.
constexpr wchar_t kTourneStart[] = L"TourneStartMenu_tourne-desktop";
constexpr ULONG_PTR kCallData = 0x4C414354;  // 'TCAL'
struct CallData {
    uint32_t on, status, here, muted, sound, seconds;
    wchar_t name[128], mic[128], out[128];
};

bool TellTourne() {
    HWND tourne = FindWindowW(kTourneStart, nullptr);
    if (!tourne) return false;
    CallData d{};
    d.on = g_shown.on;
    d.status = (uint32_t)g_shown.status;
    d.here = g_shown.here;
    d.muted = g_micOff;
    d.sound = g_bridge.on;
    auto since = g_since.find(g_shown.id);
    d.seconds = since != g_since.end() ? (uint32_t)((GetTickCount64() - since->second) / 1000) : ~0u;
    wcsncpy(d.name, g_shown.name.c_str(), 127);
    wcsncpy(d.mic, g_micName.c_str(), 127);
    wcsncpy(d.out, g_outName.c_str(), 127);
    COPYDATASTRUCT copy{kCallData, sizeof(d), &d};
    DWORD_PTR result = 0;
    return SendMessageTimeoutW(tourne, WM_COPYDATA, (WPARAM)g_window, (LPARAM)&copy, SMTO_ABORTIFHUNG, 1000, &result) && result;
}

// Shows, changes or hides the pop-up for the calls as they are now.
void UpdatePopup() {
    Shown s;
    {
        std::lock_guard<std::mutex> lock(g_lock);
        for (PhoneCallStatus want : {PhoneCallStatus::Talking, PhoneCallStatus::Held, PhoneCallStatus::Dialing})
            for (auto const& [id, call] : g_calls)
                if (!s.on && call.Status() == want) {
                    s.on = true;
                    s.id = id;
                    s.status = want;
                    s.here = call.AudioDevice() == PhoneCallAudioDevice::LocalDevice;
                }
    }
    if (s.on) {
        auto name = g_names.find(s.id);
        s.name = name != g_names.end() && !name->second.empty() ? name->second : L"Call";
        if (s.status == PhoneCallStatus::Talking && !g_since.count(s.id)) g_since[s.id] = GetTickCount64();
    } else {
        g_micOff = false;
    }
    bool was = g_shown.on;
    g_shown = s;
    // Tourne Desktop shows it, in its own look, when it's running; this pop-up is for when it isn't.
    if (TellTourne()) {
        if (g_popup && IsWindowVisible(g_popup)) {
            KillTimer(g_popup, 1);
            ShowWindow(g_popup, SW_HIDE);
        }
        if (s.on && s.here && g_bridge.on) SetTimer(g_window, 2, 50, nullptr);
        else KillTimer(g_window, 2);
        return;
    }
    KillTimer(g_window, 2);
    if (!s.on) {
        if (g_popup && was) {
            KillTimer(g_popup, 1);
            ShowWindow(g_popup, SW_HIDE);
            Log(L"pop-up: hidden");
        }
        return;
    }
    if (!g_popup) {
        WNDCLASSW wc{};
        wc.lpfnWndProc = PopupProc;
        wc.hInstance = GetModuleHandleW(nullptr);
        wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        wc.lpszClassName = L"TourneCalls_Popup";
        RegisterClassW(&wc);
        g_popup = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE, wc.lpszClassName, L"Call", WS_POPUP, 0, 0, kPopupW, kPopupH,
                                  nullptr, nullptr, wc.hInstance, nullptr);
        DWORD round = 2;  // DWMWCP_ROUND
        DwmSetWindowAttribute(g_popup, 33 /* DWMWA_WINDOW_CORNER_PREFERENCE */, &round, sizeof(round));
    }
    if (!was) {
        PlacePopup();
        ShowWindow(g_popup, SW_SHOWNOACTIVATE);
        SetTimer(g_popup, 1, 50, nullptr);
        Log(L"pop-up: shown");
    }
    InvalidateRect(g_popup, nullptr, FALSE);
}

// TourneCalls.exe preview <folder>: the pop-up drawn for a made-up call into popup-pc.bmp and popup-phone.bmp
// there, to see how it looks without a call.
void Preview(std::wstring const& folder) {
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    g_since[L"preview"] = GetTickCount64() - 83000;
    WNDCLASSW wc{};
    wc.lpfnWndProc = PopupProc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = L"TourneCalls_Popup";
    RegisterClassW(&wc);
    g_popup = CreateWindowExW(WS_EX_TOOLWINDOW, wc.lpszClassName, L"Call", WS_POPUP, 0, 0, kPopupW, kPopupH, nullptr, nullptr, wc.hInstance, nullptr);
    g_shown = {true, L"preview", L"Alex Morgan", PhoneCallStatus::Talking, true};
    PlacePopup();
    g_bridge.on = true;  // (drawn as if; nothing runs)
    g_micName = L"Microphone (Tourne Headset)";
    g_outName = L"Headphones (Tourne Headset Chat)";
    g_meterMic = 0.7f;
    g_meterCaller = 0.35f;
    for (bool here : {true, false}) {
        g_shown.here = here;
        RECT r;
        GetClientRect(g_popup, &r);
        HDC screen = GetDC(nullptr), dc = CreateCompatibleDC(screen);
        BITMAPINFO bi{};
        bi.bmiHeader = {sizeof(BITMAPINFOHEADER), r.right, r.bottom, 1, 32, BI_RGB};
        void* bits = nullptr;
        HBITMAP bitmap = CreateDIBSection(screen, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
        HGDIOBJ old = SelectObject(dc, bitmap);
        PaintPopup(dc);
        GdiFlush();
        std::wstring path = folder + (here ? L"\\popup-pc.bmp" : L"\\popup-phone.bmp");
        HANDLE h = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, 0, nullptr);
        DWORD size = r.right * r.bottom * 4, wrote = 0;
        BITMAPFILEHEADER fh{0x4D42, (DWORD)(sizeof(fh) + sizeof(bi.bmiHeader) + size), 0, 0, (DWORD)(sizeof(fh) + sizeof(bi.bmiHeader))};
        WriteFile(h, &fh, sizeof(fh), &wrote, nullptr);
        WriteFile(h, &bi.bmiHeader, sizeof(bi.bmiHeader), &wrote, nullptr);
        WriteFile(h, bits, size, &wrote, nullptr);
        CloseHandle(h);
        SelectObject(dc, old);
        DeleteObject(bitmap);
        DeleteDC(dc);
        ReleaseDC(nullptr, screen);
    }
    g_bridge.on = false;
}

LRESULT CALLBACK WindowProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static UINT const command = RegisterWindowMessageW(L"TourneCallsCommand");
    if (msg == command) {
        Command((int)wParam);
        return 0;
    }
    if (msg == WM_APP_CHANGED) {
        Refresh(L"a change");
        Follow();
        return 0;
    }
    if (msg == WM_COPYDATA) {
        auto* data = (COPYDATASTRUCT*)lParam;
        if (data->dwData == 6 && g_line && data->cbData >= sizeof(wchar_t)) {
            std::wstring number((wchar_t const*)data->lpData, data->cbData / sizeof(wchar_t));
            number = number.c_str();
            Log(L"dialing (%zu digits)", number.size());
            try {
                g_line.Dial(number, L"");
            } catch (...) {
                Log(L"dial: failed");
            }
        }
        return TRUE;
    }
    if (msg == WM_TIMER && wParam == 2) {  // the levels to Tourne Desktop's call window, while it shows the sound here
        static UINT const levels = RegisterWindowMessageW(L"TourneCallsLevels");
        if (HWND tourne = FindWindowW(kTourneStart, nullptr))
            PostMessageW(tourne, levels, g_levelMic.exchange(0) * 1000 / 32767, g_levelCaller.exchange(0) * 1000 / 32767);
        return 0;
    }
    if (msg == WM_TIMER) {  // (a call can talk, or move, without an event: seen here within a second)
        Refresh(L"a look");
        if (Seen() != g_seen) Follow();
        return 0;
    }
    return DefWindowProcW(wnd, msg, wParam, lParam);
}

void Run() {
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    HANDLE once = CreateMutexW(nullptr, TRUE, L"Tourne.Calls.Run");
    if (GetLastError() == ERROR_ALREADY_EXISTS) return Log(L"already running");
    Register(true);
    try {
        auto store = PhoneCallManager::RequestStoreAsync().get();
        auto lineId = store.GetDefaultLineAsync().get();
        g_line = PhoneLine::FromIdAsync(lineId).get();
        Log(L"line: %ls", g_line.DisplayName().c_str());
        g_line.LineChanged([](auto&&, auto&&) { PostMessageW(g_window, WM_APP_CHANGED, 0, 0); });
        PhoneCallManager::CallStateChanged([](auto&&, auto&&) { PostMessageW(g_window, WM_APP_CHANGED, 0, 0); });
    } catch (hresult_error const& e) {
        Log(L"line: error 0x%08X %ls", (unsigned)e.code(), e.message().c_str());
    }
    WNDCLASSW wc{};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = L"TourneCalls_Window";
    RegisterClassW(&wc);
    g_window = CreateWindowExW(0, wc.lpszClassName, L"Tourne Calls", 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, wc.hInstance, nullptr);
    // Also looked at every second: in the first real calls (2026-10-08) a call reached the PC with no event for it.
    SetTimer(g_window, 1, 1000, nullptr);
    Refresh(L"start");
    Follow();
    MSG m;
    while (GetMessageW(&m, nullptr, 0, 0) > 0) DispatchMessageW(&m);
    g_bridge.Stop();
    CloseHandle(once);
}
int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR command, int) {
    init_apartment(apartment_type::multi_threaded);
    std::wstring what = command ? command : L"";
    Log(L"--- TourneCalls %ls", what.c_str());
    // A command for the copy that's running: answer, hangup, mute, here (sound to the PC), there (to the phone).
    static std::map<std::wstring, int> const commands{{L"answer", 1}, {L"hangup", 2}, {L"mute", 3}, {L"here", 4}, {L"there", 5}};
    if (auto c = commands.find(what); c != commands.end()) {
        HWND running = FindWindowExW(HWND_MESSAGE, nullptr, L"TourneCalls_Window", nullptr);
        Log(L"sending %ls: %ls", what.c_str(), running ? L"to the running copy" : L"none running");
        if (running) PostMessageW(running, RegisterWindowMessageW(L"TourneCallsCommand"), c->second, 0);
        return 0;
    }
    if (what.rfind(L"preview ", 0) == 0) Preview(what.substr(8));
    else if (what == L"run") Run();
    else if (what == L"register") Register(true);
    else if (what == L"unregister") Register(false);
    else Status();
    return 0;
}
