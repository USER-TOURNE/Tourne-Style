// Stub WinRT surface, enough to syntax-check the Table mod on Linux.
// Not a real projection: nothing here runs.
#pragma once
#include <chrono>
#include <cstdint>
#include <string>
#include <vector>

namespace winrt {

struct event_token { int64_t value = 0; };

enum class apartment_type { single_threaded, multi_threaded };
inline void init_apartment(apartment_type = apartment_type::multi_threaded) {}
inline void uninit_apartment() {}

struct hstring {
    std::wstring s;
    hstring() = default;
    hstring(std::wstring v) : s(std::move(v)) {}
    const wchar_t* c_str() const { return s.c_str(); }
    bool empty() const { return s.empty(); }
    operator std::wstring() const { return s; }
};

template <typename T>
struct array_view {
    T* first = nullptr;
    T* last = nullptr;
    array_view() = default;
    array_view(std::vector<T>& v) : first(v.data()), last(v.data() + v.size()) {}
    T* data() const { return first; }
    size_t size() const { return (size_t)(last - first); }
};

struct clock {
    using duration = std::chrono::duration<int64_t, std::ratio<1, 10000000>>;
    using time_point = std::chrono::time_point<std::chrono::system_clock, duration>;
    static time_point now() { return time_point{}; }
};

// Every async in this file is immediately .get()'d.
template <typename T>
struct async_of {
    T get() const { return T{}; }
};
template <>
struct async_of<void> {
    void get() const {}
};

struct any_handle {
    bool ok = false;
    any_handle() = default;
    any_handle(std::nullptr_t) {}
    any_handle& operator=(std::nullptr_t) { ok = false; return *this; }
    explicit operator bool() const { return ok; }
};

}  // namespace winrt
