#pragma once
#include <winrt/base.h>
#include <windows.h>

namespace winrt::Windows::Storage::Streams {

struct IRandomAccessStream : winrt::any_handle {
    using any_handle::any_handle;
    UINT64 Size() const { return 0; }
};

struct IRandomAccessStreamReference : winrt::any_handle {
    using any_handle::any_handle;
    winrt::async_of<IRandomAccessStream> OpenReadAsync() const { return {}; }
};

struct DataReader {
    DataReader() = default;
    explicit DataReader(IRandomAccessStream const&) {}
    winrt::async_of<UINT32> LoadAsync(UINT32) const { return {}; }
    void ReadBytes(winrt::array_view<BYTE>) const {}
    void DetachStream() const {}
};

}  // namespace winrt::Windows::Storage::Streams
