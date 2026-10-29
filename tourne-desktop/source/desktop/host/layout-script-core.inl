// The part of a layout script that is only arithmetic and Luau: the VM with
// its limits, the table a layout is handed, and the rectangles read back from
// it. No Windows in it, so `tools/layouts/layouttest.cpp` runs the real thing,
// the examples included, on any machine.
//
// layout-scripts.cpp has the rest: the folder, the files, the examples and
// turning these boxes into the tiling's rectangles.
//
// Whoever includes this has already included <algorithm>, <chrono>, <cmath>,
// <string>, <vector> and Luau's lua.h, lualib.h and luacode.h.

constexpr int kLsRunLimitMs = 40;            // it runs while windows wait to be moved
constexpr size_t kLsMemoryLimit = 4u << 20;  // 4 MB is a great deal of arithmetic
constexpr size_t kLsMostWindows = 256;       // more than any screen holds, and a limit all the same

struct LsBox {
    double x = 0, y = 0, w = 0, h = 0;
};

struct LsWindow {
    LsBox box;                            // where it is now
    std::string program, cls, title;      // UTF-8, as Luau wants its strings
    bool focused = false;
};

struct LsAsk {
    LsBox screen;                         // the room to lay out in
    double gap = 0, ratio = 50;
    std::string options;                  // the rest of the setting's line, the script's own to read
    std::vector<LsWindow> windows;
};

// One script's VM: compiled once, called again and again.
struct LsVm {
    lua_State* L = nullptr;  // the VM
    lua_State* T = nullptr;  // the thread the script itself runs on, with globals of its own
    int fn = LUA_NOREF;   // its layout function
    size_t memory = 0;
    long long deadline = 0;  // steady_clock ms, 0 while it isn't running
    std::string problem;     // why it last failed, empty when it didn't
};

inline long long LsNow() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch())
        .count();
}

inline void* LsAlloc(void* ud, void* ptr, size_t osize, size_t nsize) {
    LsVm& vm = *(LsVm*)ud;
    if (!ptr) osize = 0;
    if (nsize == 0) {
        free(ptr);
        vm.memory -= std::min(vm.memory, osize);
        return nullptr;
    }
    if (nsize > osize && vm.memory + (nsize - osize) > kLsMemoryLimit) return nullptr;
    void* p = realloc(ptr, nsize);
    if (p) vm.memory = vm.memory - std::min(vm.memory, osize) + nsize;
    return p;
}

inline void LsInterrupt(lua_State* L, int gc) {
    if (gc >= 0) return;  // (never stop it in the middle of collecting)
    LsVm& vm = *(LsVm*)lua_callbacks(L)->userdata;
    if (vm.deadline && LsNow() > vm.deadline) {
        vm.deadline = 0;
        luaL_error(L, "took longer than %d ms to answer", kLsRunLimitMs);
    }
}

inline void LsClose(LsVm& vm) {
    if (vm.L) lua_close(vm.L);
    vm.L = vm.T = nullptr;
    vm.fn = LUA_NOREF;
    vm.memory = 0;
}

// Compiled, run once, and its layout function kept: the function it returned,
// the layout in the table it returned, or the global it defined. False with
// vm.problem saying why not.
inline bool LsCompile(LsVm& vm, std::string const& source, std::string const& name) {
    LsClose(vm);
    vm.problem.clear();
    vm.L = lua_newstate(LsAlloc, &vm);
    if (!vm.L) {
        vm.problem = "no memory for its VM";
        return false;
    }
    lua_callbacks(vm.L)->userdata = &vm;
    lua_callbacks(vm.L)->interrupt = LsInterrupt;
    luaL_openlibs(vm.L);
    luaL_sandbox(vm.L);
    // The script runs on a thread of its own, as the plugins do: the VM's own
    // globals stay readonly (so no script can change Luau's own tables for
    // another) while the script still writes its own, which is what "function
    // layout(ask)" does.
    vm.T = lua_newthread(vm.L);
    lua_ref(vm.L, -1);
    lua_pop(vm.L, 1);
    luaL_sandboxthread(vm.T);
    size_t size = 0;
    char* bytecode = luau_compile(source.data(), source.size(), nullptr, &size);
    std::string chunk = "=" + name;
    int loaded = luau_load(vm.T, chunk.c_str(), bytecode, size, 0);
    free(bytecode);
    if (loaded != 0) {
        const char* why = lua_tostring(vm.T, -1);
        vm.problem = why ? why : "didn't compile";
        LsClose(vm);
        return false;
    }
    vm.deadline = LsNow() + kLsRunLimitMs;
    int status = lua_pcall(vm.T, 0, 1, 0);
    vm.deadline = 0;
    if (status != LUA_OK) {
        const char* why = lua_tostring(vm.T, -1);
        vm.problem = why ? why : "stopped while it was being read";
        LsClose(vm);
        return false;
    }
    if (lua_istable(vm.T, -1)) {
        lua_getfield(vm.T, -1, "layout");
        lua_remove(vm.T, -2);
    }
    if (!lua_isfunction(vm.T, -1)) {
        lua_pop(vm.T, 1);
        lua_getglobal(vm.T, "layout");
    }
    if (!lua_isfunction(vm.T, -1)) {
        vm.problem = "has no layout function in it";
        LsClose(vm);
        return false;
    }
    vm.fn = lua_ref(vm.T, -1);
    lua_settop(vm.T, 0);
    return true;
}

inline void LsSetNumber(lua_State* L, const char* key, double value) {
    lua_pushnumber(L, value);
    lua_setfield(L, -2, key);
}

inline void LsSetText(lua_State* L, const char* key, std::string const& value) {
    lua_pushlstring(L, value.data(), value.size());
    lua_setfield(L, -2, key);
}

inline void LsPushBox(lua_State* L, LsBox const& box) {
    lua_createtable(L, 0, 4);
    LsSetNumber(L, "x", box.x);
    LsSetNumber(L, "y", box.y);
    LsSetNumber(L, "w", box.w);
    LsSetNumber(L, "h", box.h);
}

inline void LsPushAsk(lua_State* L, LsAsk const& ask) {
    lua_createtable(L, 0, 6);
    LsPushBox(L, ask.screen);
    lua_setfield(L, -2, "screen");
    LsSetNumber(L, "gap", ask.gap);
    LsSetNumber(L, "ratio", ask.ratio);
    LsSetNumber(L, "count", (double)ask.windows.size());
    LsSetText(L, "options", ask.options);
    lua_createtable(L, (int)ask.windows.size(), 0);
    for (size_t i = 0; i < ask.windows.size(); i++) {
        LsWindow const& w = ask.windows[i];
        LsPushBox(L, w.box);
        LsSetText(L, "program", w.program);
        LsSetText(L, "class", w.cls);
        LsSetText(L, "title", w.title);
        lua_pushboolean(L, w.focused);
        lua_setfield(L, -2, "focused");
        lua_rawseti(L, -2, (int)i + 1);
    }
    lua_setfield(L, -2, "windows");
}

// A number from the table at `at`: its field, or the number in that place
// ({x = 10} and {10, ...} both read).
inline bool LsNumber(lua_State* L, int at, const char* field, int which, double* out) {
    lua_rawgeti(L, at, which);
    if (!lua_isnumber(L, -1)) {
        lua_pop(L, 1);
        lua_getfield(L, at, field);
    }
    bool ok = lua_isnumber(L, -1);
    if (ok) *out = lua_tonumber(L, -1);
    lua_pop(L, 1);
    return ok && std::isfinite(*out) && *out > -1e6 && *out < 1e6;
}

// One rectangle of the answer, kept inside the screen with room in it: a
// layout that is a pixel out shouldn't put a window off the screen.
inline bool LsReadBox(lua_State* L, int at, LsBox const& screen, LsBox* out) {
    if (!lua_istable(L, at)) return false;
    LsBox box;
    if (!LsNumber(L, at, "x", 1, &box.x) || !LsNumber(L, at, "y", 2, &box.y) ||
        !LsNumber(L, at, "w", 3, &box.w) || !LsNumber(L, at, "h", 4, &box.h))
        return false;
    double left = std::clamp(box.x, screen.x, screen.x + screen.w - 1);
    double top = std::clamp(box.y, screen.y, screen.y + screen.h - 1);
    double right = std::clamp(box.x + box.w, left + 1, screen.x + screen.w);
    double bottom = std::clamp(box.y + box.h, top + 1, screen.y + screen.h);
    *out = LsBox{left, top, right - left, bottom - top};
    return true;
}

// The layout's answer: a box for every window, or false with vm.problem saying
// what was wrong with it.
inline bool LsRun(LsVm& vm, LsAsk const& ask, std::vector<LsBox>* out) {
    out->clear();
    if (!vm.T || vm.fn == LUA_NOREF) return false;
    if (ask.windows.empty() || ask.windows.size() > kLsMostWindows) {
        vm.problem = "was asked for more windows than a screen holds";
        return false;
    }
    lua_getref(vm.T, vm.fn);
    LsPushAsk(vm.T, ask);
    vm.deadline = LsNow() + kLsRunLimitMs;
    int status = lua_pcall(vm.T, 1, 1, 0);
    vm.deadline = 0;
    bool ok = false;
    if (status != LUA_OK) {
        const char* why = lua_tostring(vm.T, -1);
        vm.problem = why ? why : "stopped";
    } else if (!lua_istable(vm.T, -1)) {
        vm.problem = "answered with something that isn't a list of rectangles";
    } else {
        ok = true;
        for (size_t i = 0; i < ask.windows.size() && ok; i++) {
            lua_rawgeti(vm.T, -1, (int)i + 1);
            LsBox box;
            ok = LsReadBox(vm.T, lua_gettop(vm.T), ask.screen, &box);
            lua_pop(vm.T, 1);
            if (ok) out->push_back(box);
        }
        if (!ok) {
            out->clear();
            vm.problem = "answered with a rectangle that doesn't read as x, y, w, h for every window";
        } else {
            vm.problem.clear();
        }
    }
    lua_settop(vm.T, 0);
    lua_gc(vm.T, LUA_GCSTEP, 1);  // (a little at a time, never a whole collection mid-tiling)
    return ok;
}
