#pragma once
#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Storage.Streams.h>

namespace winrt::Windows::Media::Control {

enum class GlobalSystemMediaTransportControlsSessionPlaybackStatus {
    Closed, Opened, Changing, Stopped, Playing, Paused
};

struct GlobalSystemMediaTransportControlsSessionPlaybackInfo : winrt::any_handle {
    using any_handle::any_handle;
    GlobalSystemMediaTransportControlsSessionPlaybackStatus PlaybackStatus() const {
        return GlobalSystemMediaTransportControlsSessionPlaybackStatus::Stopped;
    }
};

struct GlobalSystemMediaTransportControlsSessionTimelineProperties : winrt::any_handle {
    using any_handle::any_handle;
    winrt::clock::duration StartTime() const { return {}; }
    winrt::clock::duration EndTime() const { return {}; }
    winrt::clock::duration Position() const { return {}; }
    winrt::clock::time_point LastUpdatedTime() const { return {}; }
};

struct GlobalSystemMediaTransportControlsSessionMediaProperties : winrt::any_handle {
    using any_handle::any_handle;
    winrt::hstring Title() const { return {}; }
    winrt::hstring Artist() const { return {}; }
    winrt::Windows::Storage::Streams::IRandomAccessStreamReference Thumbnail() const { return {}; }
};

struct GlobalSystemMediaTransportControlsSession : winrt::any_handle {
    using any_handle::any_handle;
    winrt::async_of<GlobalSystemMediaTransportControlsSessionMediaProperties>
        TryGetMediaPropertiesAsync() const { return {}; }
    GlobalSystemMediaTransportControlsSessionPlaybackInfo GetPlaybackInfo() const { return {}; }
    GlobalSystemMediaTransportControlsSessionTimelineProperties GetTimelineProperties() const { return {}; }
    winrt::async_of<bool> TrySkipPreviousAsync() const { return {}; }
    winrt::async_of<bool> TrySkipNextAsync() const { return {}; }
    winrt::async_of<bool> TryTogglePlayPauseAsync() const { return {}; }
    winrt::async_of<bool> TryChangePlaybackPositionAsync(int64_t) const { return {}; }
    template <typename F> winrt::event_token MediaPropertiesChanged(F const&) const { return {}; }
    template <typename F> winrt::event_token PlaybackInfoChanged(F const&) const { return {}; }
    template <typename F> winrt::event_token TimelinePropertiesChanged(F const&) const { return {}; }
    void MediaPropertiesChanged(winrt::event_token const&) const {}
    void PlaybackInfoChanged(winrt::event_token const&) const {}
    void TimelinePropertiesChanged(winrt::event_token const&) const {}
};

struct GlobalSystemMediaTransportControlsSessionManager : winrt::any_handle {
    using any_handle::any_handle;
    static winrt::async_of<GlobalSystemMediaTransportControlsSessionManager> RequestAsync() { return {}; }
    GlobalSystemMediaTransportControlsSession GetCurrentSession() const { return {}; }
    template <typename F> winrt::event_token CurrentSessionChanged(F const&) const { return {}; }
    void CurrentSessionChanged(winrt::event_token const&) const {}
};

}  // namespace winrt::Windows::Media::Control
