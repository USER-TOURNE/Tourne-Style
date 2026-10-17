// Does UserNotificationListener's NotificationChanged fire in a plain desktop process (Tourne Desktop has
// no package)? Subscribes for a while and prints each change; send a toast meanwhile.
//
//   listener-probe.exe 15     (seconds)
#include <windows.h>

#include <cstdio>

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.Notifications.Management.h>
#include <winrt/Windows.UI.Notifications.h>

using namespace winrt;
using namespace winrt::Windows::UI::Notifications;
using namespace winrt::Windows::UI::Notifications::Management;

int main(int argc, char** argv) {
    int seconds = argc > 1 ? atoi(argv[1]) : 15;
    init_apartment(apartment_type::multi_threaded);
    try {
        auto listener = UserNotificationListener::Current();
        printf("access: %d (1 = allowed)\n", (int)listener.GetAccessStatus());
        auto token = listener.NotificationChanged([](UserNotificationListener const&, UserNotificationChangedEventArgs const& args) {
            printf("%lu ms  changed: kind %d, id %u\n", GetTickCount() % 100000, (int)args.ChangeKind(), args.UserNotificationId());
            fflush(stdout);
        });
        printf("subscribed\n");
        fflush(stdout);
        Sleep(seconds * 1000);
        listener.NotificationChanged(token);
    } catch (hresult_error const& e) {
        printf("error 0x%08X: %ls\n", (unsigned)e.code(), e.message().c_str());
    }
    return 0;
}
