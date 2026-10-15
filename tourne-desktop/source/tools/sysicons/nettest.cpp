// System Icons' network flyout outside Windhawk: its own code scans for Wi-Fi, waits, loads the flyout
// and prints what it found (Windhawk's functions return their defaults: WH_EDITING).
#include "sysicons-inlined.cpp"
#include <cstdio>

int wmain() {
    winrt::init_apartment(winrt::apartment_type::single_threaded);
    CoCreateInstance(kClsidNetworkListManager, nullptr, CLSCTX_ALL, kIidNetworkListManager, (void**)&g_networkListManager);
    DWORD version;
    if (WlanOpenHandle(2, nullptr, &version, &g_wlan) != ERROR_SUCCESS) g_wlan = nullptr;
    wprintf(L"wlan handle %p\n", g_wlan);
    g_settings = LoadSettings();
    g_flyout.kind = kFlyoutNetwork;
    ScanWifi();
    Sleep(4000);
    LoadNetworkFlyout();
    wprintf(L"adapter %d, wifi off %d, needs location %d, connections %zu, networks %zu\n", g_flyout.wifiAdapter,
            g_flyout.wifiOff, g_flyout.wifiNeedsLocation, g_flyout.connections.size(), g_flyout.networks.size());
    for (auto const& n : g_flyout.networks)
        wprintf(L"  %ls  %d%%%ls\n", n.ssid.c_str(), n.quality, n.connected ? L"  (connected)" : L"");
    return 0;
}
