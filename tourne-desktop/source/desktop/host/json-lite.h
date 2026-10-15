// Just enough JSON for the Chrome DevTools protocol: parse a message into a
// tree, look values up by path, and quote a string for sending. Plain C++.
#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace td::json {

struct Value {
    enum Kind { Null, Bool, Number, String, Array, Object } kind = Null;
    bool boolean = false;
    double number = 0;
    std::string text;                                  // String (UTF-8)
    std::vector<Value> items;                          // Array
    std::vector<std::pair<std::string, Value>> fields; // Object, in order

    // A field of an object (or a null value).
    const Value& operator[](const char* name) const;
    // An item of an array (or a null value).
    const Value& operator[](size_t index) const;
    bool IsNull() const { return kind == Null; }
};

// False if the text isn't JSON.
bool Parse(const std::string& text, Value* out);

// "text" with JSON's escapes, quotes included.
std::string Quote(const std::string& text);

}  // namespace td::json
