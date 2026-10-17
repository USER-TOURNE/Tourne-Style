// Which file changes does Windows report in the notifications folder when a toast arrives? Watches it
// with every change filter for a while and prints each entry; send a toast meanwhile.
//
//   folder-probe.exe <folder> 15     (seconds)
#include <windows.h>

#include <cstdio>
#include <cwchar>

int wmain(int argc, wchar_t** argv) {
    if (argc < 2) return 1;
    int seconds = argc > 2 ? _wtoi(argv[2]) : 15;
    HANDLE dir = CreateFileW(argv[1], FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                             nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, nullptr);
    if (dir == INVALID_HANDLE_VALUE) {
        printf("open failed: %lu\n", GetLastError());
        return 1;
    }
    printf("watching\n");
    fflush(stdout);
    DWORD end = GetTickCount() + seconds * 1000;
    alignas(8) BYTE buffer[16384];
    OVERLAPPED ov{};
    ov.hEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    DWORD filters = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE |
                    FILE_NOTIFY_CHANGE_LAST_ACCESS | FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_CREATION;
    while (GetTickCount() < end) {
        ResetEvent(ov.hEvent);
        if (!ReadDirectoryChangesW(dir, buffer, sizeof buffer, TRUE, filters, nullptr, &ov, nullptr)) break;
        if (WaitForSingleObject(ov.hEvent, end - GetTickCount()) != WAIT_OBJECT_0) break;
        DWORD got = 0;
        GetOverlappedResult(dir, &ov, &got, FALSE);
        for (BYTE* p = buffer; got;) {
            auto* info = (FILE_NOTIFY_INFORMATION*)p;
            printf("%lu ms  action %lu  %.*ls\n", GetTickCount() % 100000, info->Action,
                   (int)(info->FileNameLength / 2), info->FileName);
            if (!info->NextEntryOffset) break;
            p += info->NextEntryOffset;
        }
        fflush(stdout);
    }
    CancelIo(dir);
    return 0;
}
