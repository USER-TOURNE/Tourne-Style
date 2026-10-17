// Can Tourne use Windows' own calling for a Bluetooth phone (the PC as the phone's hands-free, as Phone Link
// drives it)? Read-only: lists the phone-line transports (Bluetooth hands-free) with whether an app is registered
// for them and Tourne's access, then the phone lines Windows has and their active calls. Registers nothing,
// connects nothing, dials nothing.
//
// Build (Windhawk's compiler):
//   clang++ -std=c++20 -municode -static calls-probe.cpp -o calls-probe.exe -lole32 -loleaut32 -lruntimeobject
#include <windows.h>

#include <cstdio>
#include <string>

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.ApplicationModel.Calls.h>
#include <winrt/Windows.Devices.Enumeration.h>

using namespace winrt;
using namespace winrt::Windows::ApplicationModel::Calls;
using namespace winrt::Windows::Devices::Enumeration;

template <typename F>
void Try(wchar_t const* what, F f) {
    try {
        f();
    } catch (hresult_error const& e) {
        wprintf(L"  %ls: error 0x%08X %ls\n", what, (unsigned)e.code(), e.message().c_str());
    } catch (...) {
        wprintf(L"  %ls: failed\n", what);
    }
}

int wmain() {
    init_apartment(apartment_type::multi_threaded);
    wprintf(L"Phone-line transports (hands-free):\n");
    Try(L"transports", [] {
        auto found = DeviceInformation::FindAllAsync(PhoneLineTransportDevice::GetDeviceSelector()).get();
        wprintf(L"  %u found\n", found.Size());
        for (auto const& d : found) {
            wprintf(L"  - %ls\n", d.Name().c_str());
            Try(L"open", [&] {
                auto t = PhoneLineTransportDevice::FromId(d.Id());
                wprintf(L"    transport %d, an app registered for it: %d\n", (int)t.Transport(), t.IsRegistered());
                Try(L"audio routing", [&] { wprintf(L"    audio routing: %d (0 can't, 1 can)\n", (int)t.AudioRoutingStatus()); });
                Try(L"in-band ringing", [&] { wprintf(L"    in-band ringing: %d\n", t.InBandRingingEnabled()); });
                Try(L"access", [&] { wprintf(L"    Tourne's access: %d (1 allowed, 2 denied by user, 3 by system)\n", (int)t.RequestAccessAsync().get()); });
            });
        }
    });
    wprintf(L"Phone lines:\n");
    Try(L"store", [] {
        auto store = PhoneCallManager::RequestStoreAsync().get();
        wprintf(L"  store: ok\n");
        Try(L"default line", [&] { wprintf(L"  default line: %ls\n", to_hstring(store.GetDefaultLineAsync().get()).c_str()); });
        auto watcher = store.RequestLineWatcher();
        watcher.LineAdded([](auto&&, PhoneLineWatcherEventArgs const& a) {
            Try(L"line", [&] {
                auto line = PhoneLine::FromIdAsync(a.LineId()).get();
                wprintf(L"  - %ls: transport %d, can dial %d, network %d\n", line.DisplayName().c_str(), (int)line.Transport(),
                        line.CanDial(), (int)line.NetworkState());
                Try(L"active calls", [&] {
                    auto calls = line.GetAllActivePhoneCallsAsync().get();
                    wprintf(L"    active calls: status %d, %u\n", (int)calls.OperationStatus(), calls.AllActivePhoneCalls().Size());
                });
            });
        });
        watcher.Start();
        Sleep(4000);
        watcher.Stop();
    });
    return 0;
}
