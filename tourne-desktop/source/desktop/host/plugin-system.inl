// What plugins get of Windows itself, included into plugin-runtime.cpp (inside its namespace, after the
// helpers it uses): Noctalia's calls that a Linux shell answers from the system (runAsync, runStream,
// commandExists, processMatches, systemStats, diskStats, outputs, the wallpaper, sound) answered by Windows,
// and Tourne's own extras under `tourne.*` for what Windows has and Linux names differently (processes,
// GPUs and their power state, the battery's draw, power modes and plans, Windows' own snip, idle time, the
// PC's sound for a recorder).
//
// Programs: a plugin runs only the programs its index entry names ({"kind": "program", "label": "ffmpeg, yt-dlp"}),
// found on the PATH or at a full path a setting gives, never through a shell, with no window, and in one job
// that closes with Tourne Desktop, so nothing a plugin started outlives it. A string command line is split
// as Windows splits one; pipes, && and redirections don't exist here.
//
// Idle cost: none. Nothing here polls; each call reads what it's asked for, once. (A recording of the PC's
// sound wakes every 10 ms, only while it records.)

// (Its headers are at the top of plugin-runtime.cpp, outside the namespace.)

////////////////////////////////////////////////////////////////////////////////
// Programs

// One job for everything plugins start: closing its handle (Tourne Desktop ending) ends them all.
HANDLE ProgramJob() {
    static HANDLE job = [] {
        HANDLE j = CreateJobObjectW(nullptr, nullptr);
        JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits{};
        limits.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
        if (j) SetInformationJobObject(j, JobObjectExtendedLimitInformation, &limits, sizeof(limits));
        return j;
    }();
    return job;
}

// The programs a plugin may run (lower case, no ".exe"), from its index entry, when it was allowed programs.
std::set<std::wstring> AllowedPrograms(const Entry& e) {
    std::set<std::wstring> out;
    if (!Granted(e, L"program")) return out;
    for (auto const& info : PluginCatalogue())
        if (info.id == e.plugin->id)
            for (auto const& n : info.needs)
                if (n.kind == L"program")
                    for (size_t at = 0; at < n.detail.size();) {
                        size_t comma = n.detail.find(L',', at);
                        std::wstring name = n.detail.substr(at, comma == std::wstring::npos ? std::wstring::npos : comma - at);
                        while (!name.empty() && name.front() == L' ') name.erase(0, 1);
                        while (!name.empty() && name.back() == L' ') name.pop_back();
                        for (auto& c : name) c = (wchar_t)towlower(c);
                        if (!name.empty()) out.insert(name);
                        at = comma == std::wstring::npos ? n.detail.size() : comma + 1;
                    }
    return out;
}

// The PATH as Windows keeps it now (the machine's, then the user's), so a program installed since Tourne
// Desktop started (winget adds its folder to the user's PATH) is found without a restart. Taken as this
// process's own, as Explorer does when it hears the environment changed. At most every 5 seconds.
bool RefreshPath() {
    static ULONGLONG last = 0;
    ULONGLONG now = GetTickCount64();
    if (last && now - last < 5000) return false;
    last = now;
    auto read = [](HKEY root, const wchar_t* key) {
        wchar_t raw[32767] = L"";
        DWORD size = sizeof(raw);
        if (RegGetValueW(root, key, L"Path", RRF_RT_REG_SZ | RRF_RT_REG_EXPAND_SZ | RRF_NOEXPAND, nullptr, raw, &size) != ERROR_SUCCESS) return std::wstring();
        static wchar_t expanded[32767];
        DWORD n = ExpandEnvironmentStringsW(raw, expanded, ARRAYSIZE(expanded));
        return n && n <= ARRAYSIZE(expanded) ? std::wstring(expanded) : std::wstring(raw);
    };
    std::wstring machine = read(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment");
    std::wstring user = read(HKEY_CURRENT_USER, L"Environment");
    std::wstring path = machine + (!machine.empty() && !user.empty() ? L";" : L"") + user;
    wchar_t current[32767] = L"";
    GetEnvironmentVariableW(L"PATH", current, ARRAYSIZE(current));
    if (path.empty() || path == current) return false;
    SetEnvironmentVariableW(L"PATH", path.c_str());
    return true;
}

// A program by name (on the PATH, .exe added) or by full path; empty if there's none.
std::wstring FindProgram(const std::wstring& given) {
    if (given.empty()) return L"";
    bool path = given.find(L'\\') != std::wstring::npos || given.find(L'/') != std::wstring::npos;
    if (path) {
        std::wstring p = given;
        for (auto& c : p)
            if (c == L'/') c = L'\\';
        DWORD a = GetFileAttributesW(p.c_str());
        if (a != INVALID_FILE_ATTRIBUTES && !(a & FILE_ATTRIBUTE_DIRECTORY)) return p;
        if (GetFileAttributesW((p + L".exe").c_str()) != INVALID_FILE_ATTRIBUTES) return p + L".exe";
        return L"";
    }
    wchar_t found[MAX_PATH];
    if (SearchPathW(nullptr, given.c_str(), L".exe", MAX_PATH, found, nullptr)) return found;
    if (RefreshPath() && SearchPathW(nullptr, given.c_str(), L".exe", MAX_PATH, found, nullptr)) return found;
    return L"";
}

std::wstring ProgramStem(const std::wstring& path) {
    std::wstring name = PathFindFileNameW(path.c_str());
    for (auto& c : name) c = (wchar_t)towlower(c);
    if (name.size() > 4 && name.compare(name.size() - 4, 4, L".exe") == 0) name.resize(name.size() - 4);
    return name;
}

// One argument, quoted as CommandLineToArgvW reads it back.
std::wstring QuoteArg(const std::wstring& a) {
    if (!a.empty() && a.find_first_of(L" \t\n\v\"") == std::wstring::npos) return a;
    std::wstring out = L"\"";
    for (size_t i = 0;; i++) {
        size_t slashes = 0;
        while (i < a.size() && a[i] == L'\\') i++, slashes++;
        if (i == a.size()) {
            out.append(slashes * 2, L'\\');
            break;
        }
        if (a[i] == L'"') out.append(slashes * 2 + 1, L'\\'), out += L'"';
        else out.append(slashes, L'\\'), out += a[i];
    }
    return out + L"\"";
}

// The arguments at index i: an argv table, or a command line to split.
std::vector<std::wstring> ArgvAt(lua_State* L, int i) {
    std::vector<std::wstring> argv;
    if (lua_istable(L, i)) {
        for (int k = 1, n = lua_objlen(L, i); k <= n; k++) {
            lua_rawgeti(L, i, k);
            size_t len = 0;
            const char* s = lua_tolstring(L, -1, &len);
            if (s) argv.push_back(W(std::string(s, len)));
            lua_pop(L, 1);
        }
    } else {
        std::wstring line = W(Arg(L, i));
        int n = 0;
        if (LPWSTR* parts = line.empty() ? nullptr : CommandLineToArgvW(line.c_str(), &n)) {
            for (int k = 0; k < n; k++) argv.push_back(parts[k]);
            LocalFree(parts);
        }
    }
    return argv;
}

struct Running {
    uint64_t entry = 0;   // the entry's serial
    HANDLE process = nullptr, input = nullptr;
    DWORD pid = 0;
    std::atomic<bool> stopping{false};
};
std::mutex g_runningLock;
std::map<int, std::shared_ptr<Running>> g_running;  // stream handles, by the number the plugin holds
int g_nextRun = 1;

// Starts it: false (and why) if it isn't one this plugin may run. stdout/stderr come to the readers
// on threads of their own; stdin is kept when keepInput.
bool Launch(Entry& e, std::vector<std::wstring> argv, bool keepInput, PROCESS_INFORMATION* pi, HANDLE* out, HANDLE* err,
            HANDLE* in, std::string* why) {
    if (argv.empty()) return *why = "no program named", false;
    if (!Granted(e, L"program")) return *why = "it wasn't allowed to run programs when installed", false;
    std::wstring exe = FindProgram(argv[0]);
    if (exe.empty()) return *why = U(argv[0]) + " isn't installed (or isn't on the PATH)", false;
    auto allowed = AllowedPrograms(e);
    if (!allowed.count(ProgramStem(exe))) return *why = "it may run only " + U([&] {
                                                         std::wstring s;
                                                         for (auto const& a : allowed) s += (s.empty() ? L"" : L", ") + a;
                                                         return s.empty() ? std::wstring(L"nothing") : s;
                                                     }()) + ", not " + U(ProgramStem(exe)),
                                                     false;
    std::wstring line = QuoteArg(exe);
    for (size_t i = 1; i < argv.size(); i++) line += L" " + QuoteArg(argv[i]);
    SECURITY_ATTRIBUTES sa{sizeof(sa), nullptr, TRUE};
    HANDLE outRead = nullptr, outWrite = nullptr, errRead = nullptr, errWrite = nullptr, inRead = nullptr, inWrite = nullptr;
    bool piped = CreatePipe(&outRead, &outWrite, &sa, 0) && CreatePipe(&errRead, &errWrite, &sa, 0) && CreatePipe(&inRead, &inWrite, &sa, 0);
    if (piped) {
        SetHandleInformation(outRead, HANDLE_FLAG_INHERIT, 0);
        SetHandleInformation(errRead, HANDLE_FLAG_INHERIT, 0);
        SetHandleInformation(inWrite, HANDLE_FLAG_INHERIT, 0);
    }
    STARTUPINFOW si{sizeof(si)};
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdOutput = outWrite, si.hStdError = errWrite, si.hStdInput = inRead;
    std::wstring dir = e.plugin->dataDir;
    if (!dir.empty() && GetFileAttributesW(dir.c_str()) == INVALID_FILE_ATTRIBUTES) dir.clear();
    bool ok = piped && CreateProcessW(exe.c_str(), line.data(), nullptr, nullptr, TRUE, CREATE_NO_WINDOW | CREATE_SUSPENDED, nullptr,
                                      dir.empty() ? nullptr : dir.c_str(), &si, pi);
    for (HANDLE h : {outWrite, errWrite, inRead})
        if (h) CloseHandle(h);
    if (!ok) {
        for (HANDLE h : {outRead, errRead, inWrite})
            if (h) CloseHandle(h);
        return *why = piped ? "Windows couldn't start " + U(exe) : "no pipes", false;
    }
    if (ProgramJob()) AssignProcessToJobObject(ProgramJob(), pi->hProcess);
    ResumeThread(pi->hThread);
    CloseHandle(pi->hThread);
    pi->hThread = nullptr;
    *out = outRead, *err = errRead;
    if (keepInput) *in = inWrite;
    else CloseHandle(inWrite), *in = nullptr;
    Remember(Name(e) + L" ran " + line.substr(0, 200));
    return true;
}

std::string ReadAllFrom(HANDLE h, size_t cap, bool* truncated) {
    std::string text;
    char buffer[4096];
    DWORD got = 0;
    while (ReadFile(h, buffer, sizeof(buffer), &got, nullptr) && got) {
        if (text.size() < cap) text.append(buffer, std::min<size_t>(got, cap - text.size()));
        else *truncated = true;
    }
    return text;
}

// noctalia.runAsync(argvOrLine, onResult?, timeoutMs?): onResult({exitCode, stdout, stderr, timedOut, ...}).
int ApiRunAsync(lua_State* L) {
    Entry& e = E(L);
    std::vector<std::wstring> argv = ArgvAt(L, 1);
    bool wants = lua_isfunction(L, 2);
    int timeout = (int)luaL_optnumber(L, 3, 60000);
    timeout = std::clamp(timeout, 50, 60000);
    PROCESS_INFORMATION pi{};
    HANDLE out = nullptr, err = nullptr, in = nullptr;
    std::string why;
    if (!Launch(e, argv, false, &pi, &out, &err, &in, &why)) {
        TellOnce(e, "a program wasn't run: " + why);
        return FailBool(L, why);
    }
    int ref = wants ? lua_ref(L, 2) : LUA_NOREF;
    uint64_t serial = e.serial;
    std::thread([=] {
        constexpr size_t kCap = 4 << 20;
        bool outCut = false, errCut = false;
        std::string errText;
        std::thread errReader([&] { errText = ReadAllFrom(err, kCap, &errCut); });
        std::thread waiter([&] {});
        waiter.join();
        // stdout is read here, the wait for the timeout beside it.
        std::atomic<bool> done{false};
        bool timedOut = false;
        std::thread watch([&] {
            if (WaitForSingleObject(pi.hProcess, (DWORD)timeout) == WAIT_TIMEOUT && !done) {
                timedOut = true;
                TerminateProcess(pi.hProcess, 1);
            }
        });
        std::string outText = ReadAllFrom(out, kCap, &outCut);
        errReader.join();
        WaitForSingleObject(pi.hProcess, INFINITE);
        done = true;
        watch.join();
        DWORD code = 1;
        GetExitCodeProcess(pi.hProcess, &code);
        CloseHandle(pi.hProcess), CloseHandle(out), CloseHandle(err);
        if (ref == LUA_NOREF) return;
        Post([=, outText = std::move(outText), errText = std::move(errText)] {
            auto found = g_bySerial.find(serial);
            if (found == g_bySerial.end()) return;
            Entry& to = *found->second;
            Guarded(to, [&] {
                lua_settop(to.T, 0);
                lua_getref(to.T, ref);
                lua_unref(to.T, ref);
                lua_createtable(to.T, 0, 6);
                lua_pushinteger(to.T, (int)code);
                lua_setfield(to.T, -2, "exitCode");
                lua_pushlstring(to.T, outText.data(), outText.size());
                lua_setfield(to.T, -2, "stdout");
                lua_pushlstring(to.T, errText.data(), errText.size());
                lua_setfield(to.T, -2, "stderr");
                lua_pushboolean(to.T, timedOut);
                lua_setfield(to.T, -2, "timedOut");
                lua_pushboolean(to.T, outCut);
                lua_setfield(to.T, -2, "stdoutTruncated");
                lua_pushboolean(to.T, errCut);
                lua_setfield(to.T, -2, "stderrTruncated");
                return Call(to, 1);
            });
        });
    }).detach();
    lua_pushboolean(L, true);
    return 1;
}

// Hands one line (or the exit) to a stream's callback on the runtime thread.
void StreamLine(uint64_t serial, int ref, std::string line) {
    Post([=, line = std::move(line)] {
        auto found = g_bySerial.find(serial);
        if (found == g_bySerial.end()) return;
        Entry& to = *found->second;
        Guarded(to, [&] {
            lua_settop(to.T, 0);
            lua_getref(to.T, ref);
            lua_pushlstring(to.T, line.data(), line.size());
            return Call(to, 1);
        });
    });
}

// noctalia.runStream(argvOrLine, onLine [, onExit]): a number (the stream, for tourne.stop / tourne.write)
// while it runs, or false. stdout and stderr both come to onLine, a line at a time; \r ends a line too
// (progress bars). onExit(exitCode) is Tourne's addition.
int ApiRunStream(lua_State* L) {
    Entry& e = E(L);
    std::vector<std::wstring> argv = ArgvAt(L, 1);
    luaL_checktype(L, 2, LUA_TFUNCTION);
    bool exitWanted = lua_isfunction(L, 3);
    PROCESS_INFORMATION pi{};
    HANDLE out = nullptr, err = nullptr, in = nullptr;
    std::string why;
    if (!Launch(e, argv, true, &pi, &out, &err, &in, &why)) {
        TellOnce(e, "a program wasn't run: " + why);
        return FailBool(L, why);
    }
    int lineRef = lua_ref(L, 2);
    int exitRef = exitWanted ? lua_ref(L, 3) : LUA_NOREF;
    uint64_t serial = e.serial;
    auto run = std::make_shared<Running>();
    run->entry = serial, run->process = pi.hProcess, run->input = in, run->pid = pi.dwProcessId;
    int id;
    {
        std::lock_guard lock(g_runningLock);
        id = g_nextRun++;
        g_running[id] = run;
    }
    auto reader = [serial, lineRef](HANDLE h) {
        std::string pending;
        char buffer[4096];
        DWORD got = 0;
        while (ReadFile(h, buffer, sizeof(buffer), &got, nullptr) && got) {
            for (DWORD i = 0; i < got; i++) {
                char c = buffer[i];
                if (c == '\n' || c == '\r') {
                    if (!pending.empty()) StreamLine(serial, lineRef, std::move(pending));
                    pending.clear();
                } else if (pending.size() < (4u << 20)) pending += c;  // (a JSON snapshot can be long)
            }
        }
        if (!pending.empty()) StreamLine(serial, lineRef, std::move(pending));
        CloseHandle(h);
    };
    std::thread([=] {
        std::thread errThread(reader, err);
        reader(out);
        errThread.join();
        WaitForSingleObject(run->process, INFINITE);
        DWORD code = 1;
        GetExitCodeProcess(run->process, &code);
        {
            std::lock_guard lock(g_runningLock);
            g_running.erase(id);
            if (run->input) CloseHandle(run->input), run->input = nullptr;
            CloseHandle(run->process), run->process = nullptr;
        }
        Post([=] {
            auto found = g_bySerial.find(serial);
            if (found == g_bySerial.end()) return;
            Entry& to = *found->second;
            Guarded(to, [&] {
                lua_settop(to.T, 0);
                lua_unref(to.T, lineRef);
                if (exitRef == LUA_NOREF) return true;
                lua_getref(to.T, exitRef);
                lua_unref(to.T, exitRef);
                lua_pushinteger(to.T, (int)code);
                return Call(to, 1);
            });
        });
    }).detach();
    lua_pushinteger(L, id);
    return 1;
}

std::shared_ptr<Running> RunningOf(Entry& e, int id) {
    std::lock_guard lock(g_runningLock);
    auto found = g_running.find(id);
    return found != g_running.end() && found->second->entry == e.serial ? found->second : nullptr;
}

// tourne.write(stream, text): to its stdin (ffmpeg's "q" stops it cleanly).
int ApiStreamWrite(lua_State* L) {
    Entry& e = E(L);
    auto run = RunningOf(e, (int)luaL_checkinteger(L, 1));
    std::string text = Arg(L, 2);
    std::lock_guard lock(g_runningLock);
    DWORD put = 0;
    bool ok = run && run->input && WriteFile(run->input, text.data(), (DWORD)text.size(), &put, nullptr);
    lua_pushboolean(L, ok);
    return 1;
}

// tourne.stop(stream [, graceMs]): its stdin closed, then (after graceMs, default none) ended.
int ApiStreamStop(lua_State* L) {
    Entry& e = E(L);
    auto run = RunningOf(e, (int)luaL_checkinteger(L, 1));
    int grace = (int)luaL_optnumber(L, 2, 0);
    if (!run) return lua_pushboolean(L, false), 1;
    {
        std::lock_guard lock(g_runningLock);
        if (run->input) CloseHandle(run->input), run->input = nullptr;
    }
    run->stopping = true;
    DWORD pid = run->pid;
    std::thread([run, grace, pid] {
        HANDLE h = OpenProcess(PROCESS_TERMINATE | SYNCHRONIZE, FALSE, pid);
        if (!h) return;
        if (grace <= 0 || WaitForSingleObject(h, (DWORD)std::min(grace, 30000)) == WAIT_TIMEOUT) TerminateProcess(h, 1);
        CloseHandle(h);
    }).detach();
    lua_pushboolean(L, true);
    return 1;
}

// An entry's streams end with it (a reload, the plugin turned off).
void StopStreams(const Entry& e) {
    std::lock_guard lock(g_runningLock);
    for (auto& [id, run] : g_running)
        if (run->entry == e.serial && run->process) TerminateProcess(run->process, 1);
}

// noctalia.commandExists(name): on the PATH (as a program, not a shell built-in).
int ApiCommandExists(lua_State* L) {
    lua_pushboolean(L, !FindProgram(W(Arg(L, 1))).empty());
    return 1;
}

////////////////////////////////////////////////////////////////////////////////
// Processes

struct ProcessSample {
    DWORD pid, parent, threads, session;
    std::wstring name;
    ULONGLONG cpuTime;  // 100 ns units, user + kernel
    SIZE_T workingSet, privateBytes;
};

// SystemProcessInformation's record, as Windows lays it out (the SDK's winternl.h hides most of it).
struct ProcessRecord {
    ULONG NextEntryOffset, NumberOfThreads;
    LARGE_INTEGER WorkingSetPrivateSize;
    ULONG HardFaultCount, NumberOfThreadsHighWatermark;
    ULONGLONG CycleTime;
    LARGE_INTEGER CreateTime, UserTime, KernelTime;
    UNICODE_STRING ImageName;
    LONG BasePriority;
    HANDLE UniqueProcessId, InheritedFromUniqueProcessId;
    ULONG HandleCount, SessionId;
    ULONG_PTR UniqueProcessKey;
    SIZE_T PeakVirtualSize, VirtualSize;
    ULONG PageFaultCount;
    SIZE_T PeakWorkingSetSize, WorkingSetSize, QuotaPeakPagedPoolUsage, QuotaPagedPoolUsage, QuotaPeakNonPagedPoolUsage,
        QuotaNonPagedPoolUsage, PagefileUsage, PeakPagefileUsage, PrivatePageCount;
};

// Every process in one call (NtQuerySystemInformation: no handle opened on any of them).
std::vector<ProcessSample> SampleProcesses() {
    std::vector<ProcessSample> out;
    using Query = LONG(WINAPI*)(ULONG, PVOID, ULONG, PULONG);
    static Query query = (Query)(void*)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtQuerySystemInformation");
    if (!query) return out;
    std::vector<BYTE> buffer(1 << 20);
    ULONG need = 0;
    LONG status;
    while ((status = query(5 /*SystemProcessInformation*/, buffer.data(), (ULONG)buffer.size(), &need)) == (LONG)0xC0000004L)
        buffer.resize(std::max<size_t>(need + (64 << 10), buffer.size() * 2));
    if (status < 0) return out;
    for (BYTE* at = buffer.data();;) {
        auto* p = (ProcessRecord*)at;
        ProcessSample s{};
        s.pid = (DWORD)(ULONG_PTR)p->UniqueProcessId;
        s.parent = (DWORD)(ULONG_PTR)p->InheritedFromUniqueProcessId;
        s.threads = p->NumberOfThreads;
        s.session = p->SessionId;
        s.name = p->ImageName.Buffer ? std::wstring(p->ImageName.Buffer, p->ImageName.Length / 2) : (s.pid ? L"System" : L"Idle");
        s.cpuTime = (ULONGLONG)p->UserTime.QuadPart + (ULONGLONG)p->KernelTime.QuadPart;
        s.workingSet = p->WorkingSetSize;
        s.privateBytes = p->PrivatePageCount;
        if (s.pid) out.push_back(std::move(s));
        if (!p->NextEntryOffset) break;
        at += p->NextEntryOffset;
    }
    return out;
}

// tourne.processes(): {pid, parent, name, cpu (% of the whole machine since the last call), memMb, privateMb,
// threads, system (session 0: services)}. The first call's cpu is 0.
int ApiProcesses(lua_State* L) {
    Entry& e = E(L);
    if (!Granted(e, L"systemInfo")) {
        TellOnce(e, "it wasn't allowed to read system information when installed");
        lua_newtable(L);
        return 1;
    }
    static std::unordered_map<DWORD, ULONGLONG> last;
    static ULONGLONG lastAt = 0;
    static DWORD cores = [] {
        SYSTEM_INFO si;
        GetSystemInfo(&si);
        return std::max<DWORD>(1, si.dwNumberOfProcessors);
    }();
    ULONGLONG now = GetTickCount64();
    double span = lastAt ? (double)(now - lastAt) * 10000.0 * cores : 0;  // 100 ns units of all cores
    auto all = SampleProcesses();
    std::unordered_map<DWORD, ULONGLONG> next;
    lua_createtable(L, (int)all.size(), 0);
    int i = 0;
    for (auto const& s : all) {
        next[s.pid] = s.cpuTime;
        double cpu = 0;
        auto was = last.find(s.pid);
        if (span > 0 && was != last.end() && s.cpuTime >= was->second) cpu = std::min(100.0, (double)(s.cpuTime - was->second) * 100.0 / span);
        lua_createtable(L, 0, 8);
        lua_pushinteger(L, (int)s.pid);
        lua_setfield(L, -2, "pid");
        lua_pushinteger(L, (int)s.parent);
        lua_setfield(L, -2, "parent");
        std::string name = U(s.name);
        lua_pushlstring(L, name.data(), name.size());
        lua_setfield(L, -2, "name");
        lua_pushnumber(L, std::round(cpu * 10) / 10);
        lua_setfield(L, -2, "cpu");
        lua_pushnumber(L, std::round((double)s.workingSet / (1 << 20) * 10) / 10);
        lua_setfield(L, -2, "memMb");
        lua_pushnumber(L, std::round((double)s.privateBytes / (1 << 20) * 10) / 10);
        lua_setfield(L, -2, "privateMb");
        lua_pushinteger(L, (int)s.threads);
        lua_setfield(L, -2, "threads");
        lua_pushboolean(L, s.session == 0);
        lua_setfield(L, -2, "system");
        lua_rawseti(L, -2, ++i);
    }
    last.swap(next);
    lastAt = now;
    return 1;
}

// tourne.endProcess(pid): true when Windows ended it. Never Windows' own core, Tourne Desktop or Windhawk.
int ApiEndProcess(lua_State* L) {
    Entry& e = E(L);
    if (!Granted(e, L"processes")) return FailBool(L, "it wasn't allowed to end programs when installed");
    DWORD pid = (DWORD)luaL_checkinteger(L, 1);
    if (pid <= 4 || pid == GetCurrentProcessId()) return FailBool(L, "that one can't be ended from here");
    for (auto const& s : SampleProcesses())
        if (s.pid == pid) {
            std::wstring n = s.name;
            for (auto& c : n) c = (wchar_t)towlower(c);
            for (const wchar_t* core : {L"csrss.exe", L"wininit.exe", L"winlogon.exe", L"smss.exe", L"services.exe", L"lsass.exe",
                                        L"dwm.exe", L"fontdrvhost.exe", L"system", L"registry", L"memory compression",
                                        L"windhawk.exe", L"tournedesktop.exe"})
                if (n == core) return FailBool(L, "Windows (or Tourne) needs that one");
        }
    HANDLE h = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    bool ok = h && TerminateProcess(h, 1);
    if (h) CloseHandle(h);
    if (ok) Remember(Name(e) + L" ended process " + std::to_wstring(pid));
    if (!ok) return FailBool(L, "Windows didn't let it (it may need administrator rights)");
    lua_pushboolean(L, true);
    return 1;
}

// noctalia.processMatches(onResult, ...needles): whether a running program's name (or full path) holds them all.
int ApiProcessMatches(lua_State* L) {
    luaL_checktype(L, 1, LUA_TFUNCTION);
    std::vector<std::wstring> needles;
    for (int i = 2, n = lua_gettop(L); i <= n; i++) {
        std::wstring s = W(Arg(L, i));
        for (auto& c : s) c = (wchar_t)towlower(c);
        needles.push_back(s);
    }
    bool matched = false;
    for (auto const& s : SampleProcesses()) {
        std::wstring n = s.name;
        for (auto& c : n) c = (wchar_t)towlower(c);
        bool all = !needles.empty();
        for (auto const& needle : needles) all = all && n.find(needle) != std::wstring::npos;
        if (all) {
            matched = true;
            break;
        }
    }
    lua_pushvalue(L, 1);
    lua_pushboolean(L, matched);
    lua_call(L, 1, 0);
    lua_pushboolean(L, true);
    return 1;
}

////////////////////////////////////////////////////////////////////////////////
// System stats

void SetNumber(lua_State* L, const char* key, double v) {
    lua_pushnumber(L, v);
    lua_setfield(L, -2, key);
}

// noctalia.systemStats(): cpu, ram, swap, net (per second since the last call), gpu (empty: no probe).
int ApiSystemStats(lua_State* L) {
    Entry& e = E(L);
    if (!Granted(e, L"systemInfo")) {
        TellOnce(e, "it wasn't allowed to read system information when installed");
        lua_pushnil(L);
        return 1;
    }
    static ULONGLONG lastIdle = 0, lastTotal = 0, lastRx = 0, lastTx = 0, lastAt = 0;
    static double cpu = 0;
    FILETIME idle, kernel, user;
    if (GetSystemTimes(&idle, &kernel, &user)) {
        auto q = [](FILETIME f) { return ((ULONGLONG)f.dwHighDateTime << 32) | f.dwLowDateTime; };
        ULONGLONG i = q(idle), t = q(kernel) + q(user);  // (kernel time includes idle)
        if (lastTotal && t > lastTotal) cpu = 100.0 * (1.0 - (double)(i - lastIdle) / (double)(t - lastTotal));
        lastIdle = i, lastTotal = t;
    }
    MEMORYSTATUSEX m{sizeof(m)};
    GlobalMemoryStatusEx(&m);
    ULONGLONG rx = 0, tx = 0;
    std::vector<std::pair<std::string, std::pair<ULONGLONG, ULONGLONG>>> ifs;
    MIB_IF_TABLE2* table = nullptr;
    if (GetIfTable2(&table) == NO_ERROR) {
        for (ULONG k = 0; k < table->NumEntries; k++) {
            auto const& r = table->Table[k];
            if (r.Type == IF_TYPE_SOFTWARE_LOOPBACK || r.OperStatus != IfOperStatusUp || !r.InterfaceAndOperStatusFlags.HardwareInterface) continue;
            rx += r.InOctets, tx += r.OutOctets;
        }
        FreeMibTable(table);
    }
    ULONGLONG now = GetTickCount64();
    double seconds = lastAt ? (double)(now - lastAt) / 1000.0 : 0;
    double rxRate = seconds > 0 && rx >= lastRx ? (double)(rx - lastRx) / seconds : 0;
    double txRate = seconds > 0 && tx >= lastTx ? (double)(tx - lastTx) / seconds : 0;
    lastRx = rx, lastTx = tx, lastAt = now;

    lua_createtable(L, 0, 7);
    SetNumber(L, "sampledAtMs", (double)time(nullptr) * 1000.0);
    lua_createtable(L, 0, 1);
    SetNumber(L, "usagePercent", std::round(std::clamp(cpu, 0.0, 100.0) * 10) / 10);
    lua_setfield(L, -2, "cpu");
    lua_createtable(L, 0, 3);
    SetNumber(L, "usagePercent", m.dwMemoryLoad);
    SetNumber(L, "usedMb", (double)((m.ullTotalPhys - m.ullAvailPhys) >> 20));
    SetNumber(L, "totalMb", (double)(m.ullTotalPhys >> 20));
    lua_setfield(L, -2, "ram");
    lua_createtable(L, 0, 2);
    ULONGLONG pageTotal = m.ullTotalPageFile > m.ullTotalPhys ? m.ullTotalPageFile - m.ullTotalPhys : 0;
    ULONGLONG pageUsed = m.ullTotalPageFile - m.ullAvailPageFile;
    pageUsed = pageUsed > (m.ullTotalPhys - m.ullAvailPhys) ? pageUsed - (m.ullTotalPhys - m.ullAvailPhys) : 0;
    SetNumber(L, "usedMb", (double)(std::min(pageUsed, pageTotal) >> 20));
    SetNumber(L, "totalMb", (double)(pageTotal >> 20));
    lua_setfield(L, -2, "swap");
    lua_newtable(L);
    lua_setfield(L, -2, "gpu");
    lua_createtable(L, 0, 3);
    SetNumber(L, "rxBytesPerSec", std::round(rxRate));
    SetNumber(L, "txBytesPerSec", std::round(txRate));
    lua_newtable(L);
    lua_setfield(L, -2, "interfaces");
    lua_setfield(L, -2, "net");
    lua_newtable(L);
    lua_setfield(L, -2, "loadAvg");
    return 1;
}

// noctalia.diskStats(path): {usagePercent, totalBytes, freeBytes, availableBytes} for the drive it's on.
int ApiDiskStats(lua_State* L) {
    std::wstring path = ExpandPath(Arg(L, 1));
    ULARGE_INTEGER available{}, total{}, free{};
    if (!GetDiskFreeSpaceExW(path.c_str(), &available, &total, &free) || !total.QuadPart) return lua_pushnil(L), 1;
    lua_createtable(L, 0, 4);
    SetNumber(L, "usagePercent", std::round(100.0 * (double)(total.QuadPart - free.QuadPart) / (double)total.QuadPart * 10) / 10);
    SetNumber(L, "totalBytes", (double)total.QuadPart);
    SetNumber(L, "freeBytes", (double)free.QuadPart);
    SetNumber(L, "availableBytes", (double)available.QuadPart);
    return 1;
}

// noctalia.outputs(): the screens, {name ("DISPLAY1"), x, y, width, height, scale, primary}.
int ApiOutputs(lua_State* L) {
    lua_newtable(L);
    struct Ctx {
        lua_State* L;
        int i;
    } ctx{L, 0};
    EnumDisplayMonitors(
        nullptr, nullptr,
        [](HMONITOR m, HDC, LPRECT, LPARAM p) -> BOOL {
            auto& c = *(Ctx*)p;
            MONITORINFOEXW info{};
            info.cbSize = sizeof(info);
            if (!GetMonitorInfoW(m, &info)) return TRUE;
            UINT dpiX = 96, dpiY = 96;
            using GetDpi = HRESULT(WINAPI*)(HMONITOR, int, UINT*, UINT*);
            static GetDpi getDpi = (GetDpi)(void*)GetProcAddress(LoadLibraryW(L"shcore.dll"), "GetDpiForMonitor");
            if (getDpi) getDpi(m, 0, &dpiX, &dpiY);
            std::wstring name = info.szDevice;
            if (name.rfind(L"\\\\.\\", 0) == 0) name = name.substr(4);
            std::string n = U(name);
            lua_State* L = c.L;
            lua_createtable(L, 0, 7);
            lua_pushstring(L, n.c_str());
            lua_setfield(L, -2, "name");
            SetNumber(L, "x", info.rcMonitor.left);
            SetNumber(L, "y", info.rcMonitor.top);
            SetNumber(L, "width", info.rcMonitor.right - info.rcMonitor.left);
            SetNumber(L, "height", info.rcMonitor.bottom - info.rcMonitor.top);
            SetNumber(L, "scale", dpiX / 96.0);
            lua_pushboolean(L, (info.dwFlags & MONITORINFOF_PRIMARY) != 0);
            lua_setfield(L, -2, "primary");
            lua_rawseti(L, -2, ++c.i);
            return TRUE;
        },
        (LPARAM)&ctx);
    return 1;
}

// tourne.screenIndex(name): the screen's number among the first graphics card's outputs (what ffmpeg's
// ddagrab calls output_idx), or nil when that card doesn't drive it.
int ApiScreenIndex(lua_State* L) {
    std::wstring want = L"\\\\.\\" + W(Arg(L, 1));
    Microsoft::WRL::ComPtr<IDXGIFactory1> factory;
    Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
    Microsoft::WRL::ComPtr<IDXGIOutput> output;
    if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&factory))) && SUCCEEDED(factory->EnumAdapters1(0, &adapter)))
        for (UINT i = 0; adapter->EnumOutputs(i, &output) == S_OK; i++) {
            DXGI_OUTPUT_DESC d{};
            if (SUCCEEDED(output->GetDesc(&d)) && _wcsicmp(d.DeviceName, want.c_str()) == 0) {
                lua_pushinteger(L, (int)i);
                return 1;
            }
        }
    lua_pushnil(L);
    return 1;
}

// tourne.captureLink(state, screenshotFolder): Screen Toolkit tells the Print Screen key (capture.cpp) how its
// recording stands ("idle", "recording", "converting", "ready"; "off" as it stops) and where pictures go.
int ApiCaptureLink(lua_State* L) {
    if (E(L).plugin->id != L"tourne/screen-toolkit") return 0;
    CaptureToolkit(W(Arg(L, 1)), W(Arg(L, 2)));
    return 0;
}

int ApiFocusedOutputName(lua_State* L) {
    HMONITOR m = MonitorFromWindow(GetForegroundWindow(), MONITOR_DEFAULTTOPRIMARY);
    MONITORINFOEXW info{};
    info.cbSize = sizeof(info);
    if (!GetMonitorInfoW(m, &info)) return lua_pushnil(L), 1;
    std::wstring name = info.szDevice;
    if (name.rfind(L"\\\\.\\", 0) == 0) name = name.substr(4);
    lua_pushstring(L, U(name).c_str());
    return 1;
}

////////////////////////////////////////////////////////////////////////////////
// GPUs and the battery

// tourne.gpus(): {name, vendor ("nvidia", "amd", "intel", other's id), discrete, powerState ("D0".."D3", "?"), active}.
// powerState is the most recent device power state Windows has: D0 working, D3 asleep (an idle laptop dGPU).
int ApiGpus(lua_State* L) {
    Entry& e = E(L);
    lua_newtable(L);
    if (!Granted(e, L"systemInfo")) return TellOnce(e, "it wasn't allowed to read system information when installed"), 1;
    HDEVINFO set = SetupDiGetClassDevsW(&GUID_DEVCLASS_DISPLAY, nullptr, nullptr, DIGCF_PRESENT);
    if (set == INVALID_HANDLE_VALUE) return 1;
    static const DEVPROPKEY kPowerData = {{0xa45c254e, 0xdf1c, 0x4efd, {0x80, 0x20, 0x67, 0xd1, 0x46, 0xa8, 0x50, 0xe0}}, 32};
    SP_DEVINFO_DATA dev{sizeof(dev)};
    int count = 0;
    for (DWORD i = 0; SetupDiEnumDeviceInfo(set, i, &dev); i++) {
        wchar_t name[256] = L"", ids[1024] = L"";
        SetupDiGetDeviceRegistryPropertyW(set, &dev, SPDRP_DEVICEDESC, nullptr, (BYTE*)name, sizeof(name), nullptr);
        SetupDiGetDeviceRegistryPropertyW(set, &dev, SPDRP_HARDWAREID, nullptr, (BYTE*)ids, sizeof(ids), nullptr);
        std::wstring hw = ids, n = name;
        for (auto& c : hw) c = (wchar_t)towupper(c);
        std::string vendor = hw.find(L"VEN_10DE") != std::wstring::npos   ? "nvidia"
                             : hw.find(L"VEN_1002") != std::wstring::npos ? "amd"
                             : hw.find(L"VEN_8086") != std::wstring::npos ? "intel"
                             : hw.find(L"VEN_") != std::wstring::npos     ? U(hw.substr(hw.find(L"VEN_") + 4, 4))
                                                                          : "";
        if (vendor.empty()) continue;  // (Microsoft's basic display and remote adapters)
        std::wstring lower = n;
        for (auto& c : lower) c = (wchar_t)towlower(c);
        bool discrete = vendor == "nvidia" || (vendor == "amd" && lower.find(L"(tm) graphics") == std::wstring::npos &&
                                               lower.rfind(L" graphics") != lower.size() - 9) ||
                        (vendor == "intel" && lower.find(L"arc") != std::wstring::npos && lower.find(L"arc(tm) graphics") == std::wstring::npos);
        CM_POWER_DATA power{};
        DEVPROPTYPE type = 0;
        bool known = SetupDiGetDevicePropertyW(set, &dev, &kPowerData, &type, (BYTE*)&power, sizeof(power), nullptr, 0) != FALSE;
        int d = known ? (int)power.PD_MostRecentPowerState - 1 : -1;  // PowerDeviceD0 is 1
        lua_createtable(L, 0, 5);
        lua_pushstring(L, U(n).c_str());
        lua_setfield(L, -2, "name");
        lua_pushstring(L, vendor.c_str());
        lua_setfield(L, -2, "vendor");
        lua_pushboolean(L, discrete);
        lua_setfield(L, -2, "discrete");
        lua_pushstring(L, d >= 0 && d <= 3 ? ("D" + std::to_string(d)).c_str() : "?");
        lua_setfield(L, -2, "powerState");
        lua_pushboolean(L, d == 0);
        lua_setfield(L, -2, "active");
        lua_rawseti(L, -2, ++count);
    }
    SetupDiDestroyDeviceInfoList(set);
    return 1;
}

// tourne.battery(): nil without one, else {percent, charging, onAc, rateW (negative while it drains), minutesLeft}.
int ApiBattery(lua_State* L) {
    Entry& e = E(L);
    if (!Granted(e, L"systemInfo")) return TellOnce(e, "it wasn't allowed to read system information when installed"), lua_pushnil(L), 1;
    SYSTEM_POWER_STATUS s{};
    if (!GetSystemPowerStatus(&s) || (s.BatteryFlag & 128) || s.BatteryFlag == 255) return lua_pushnil(L), 1;
    long rate = 0;  // mW, summed over every battery
    bool rated = false;
    HDEVINFO set = SetupDiGetClassDevsW(&GUID_DEVCLASS_BATTERY, nullptr, nullptr, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    if (set != INVALID_HANDLE_VALUE) {
        SP_DEVICE_INTERFACE_DATA iface{sizeof(iface)};
        for (DWORD i = 0; i < 8 && SetupDiEnumDeviceInterfaces(set, nullptr, &GUID_DEVCLASS_BATTERY, i, &iface); i++) {
            DWORD need = 0;
            SetupDiGetDeviceInterfaceDetailW(set, &iface, nullptr, 0, &need, nullptr);
            std::vector<BYTE> buffer(need + sizeof(WCHAR));
            auto* detail = (SP_DEVICE_INTERFACE_DETAIL_DATA_W*)buffer.data();
            detail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_W);
            if (!SetupDiGetDeviceInterfaceDetailW(set, &iface, detail, need, nullptr, nullptr)) continue;
            HANDLE h = CreateFileW(detail->DevicePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING,
                                   FILE_ATTRIBUTE_NORMAL, nullptr);
            if (h == INVALID_HANDLE_VALUE) continue;
            BATTERY_QUERY_INFORMATION q{};
            DWORD wait = 0, got = 0;
            if (DeviceIoControl(h, IOCTL_BATTERY_QUERY_TAG, &wait, sizeof(wait), &q.BatteryTag, sizeof(q.BatteryTag), &got, nullptr) && q.BatteryTag) {
                BATTERY_WAIT_STATUS ws{};
                ws.BatteryTag = q.BatteryTag;
                BATTERY_STATUS bs{};
                if (DeviceIoControl(h, IOCTL_BATTERY_QUERY_STATUS, &ws, sizeof(ws), &bs, sizeof(bs), &got, nullptr) &&
                    bs.Rate != (LONG)BATTERY_UNKNOWN_RATE)
                    rate += bs.Rate, rated = true;
            }
            CloseHandle(h);
        }
        SetupDiDestroyDeviceInfoList(set);
    }
    lua_createtable(L, 0, 5);
    SetNumber(L, "percent", s.BatteryLifePercent <= 100 ? s.BatteryLifePercent : 0);
    lua_pushboolean(L, (s.BatteryFlag & 8) != 0);
    lua_setfield(L, -2, "charging");
    lua_pushboolean(L, s.ACLineStatus == 1);
    lua_setfield(L, -2, "onAc");
    if (rated) SetNumber(L, "rateW", std::round(rate / 100.0) / 10.0);
    if (s.BatteryLifeTime != (DWORD)-1) SetNumber(L, "minutesLeft", s.BatteryLifeTime / 60);
    return 1;
}

////////////////////////////////////////////////////////////////////////////////
// Power: modes, plans, turbo, and lock / sleep / shut down

const GUID kModeEfficiency = {0x961cc777, 0x2547, 0x4f9d, {0x81, 0x74, 0x7d, 0x86, 0x18, 0x1b, 0x8a, 0x7a}};
const GUID kModeBalanced = {0, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0}};
const GUID kModePerformance = {0xded574b5, 0x45a0, 0x4f42, {0x87, 0x37, 0x46, 0x34, 0x5c, 0x09, 0xc2, 0x38}};
const GUID kModeBetterBattery = {0x3af9b8d9, 0x7c97, 0x431d, {0xad, 0x78, 0x34, 0xa8, 0xbf, 0xea, 0x43, 0x9f}};
const GUID kProcessorGroup = {0x54533251, 0x82be, 0x4824, {0x96, 0xc1, 0x47, 0xb6, 0x0b, 0x74, 0x0d, 0x00}};
const GUID kBoostMode = {0xbe337238, 0x0d82, 0x4146, {0xa9, 0x60, 0x4f, 0x37, 0x49, 0xd4, 0x70, 0xc7}};

using OverlayGet = DWORD(WINAPI*)(GUID*);
using OverlaySet = DWORD(WINAPI*)(const GUID*);
HMODULE PowrProf() {
    static HMODULE m = LoadLibraryW(L"powrprof.dll");
    return m;
}

// tourne.powerMode(): "efficiency", "betterBattery", "balanced" or "performance" (Windows 10 and 11's power mode).
int ApiPowerMode(lua_State* L) {
    auto get = (OverlayGet)(void*)GetProcAddress(PowrProf(), "PowerGetEffectiveOverlayScheme");
    GUID g{};
    if (!get || get(&g) != ERROR_SUCCESS) return lua_pushnil(L), 1;
    lua_pushstring(L, IsEqualGUID(g, kModeEfficiency)    ? "efficiency"
                      : IsEqualGUID(g, kModePerformance) ? "performance"
                      : IsEqualGUID(g, kModeBetterBattery) ? "betterBattery"
                                                           : "balanced");
    return 1;
}

int ApiSetPowerMode(lua_State* L) {
    Entry& e = E(L);
    if (!Granted(e, L"power")) return FailBool(L, "it wasn't allowed to change power settings when installed");
    std::string mode = Arg(L, 1);
    auto set = (OverlaySet)(void*)GetProcAddress(PowrProf(), "PowerSetActiveOverlayScheme");
    const GUID* g = mode == "efficiency" ? &kModeEfficiency : mode == "performance" ? &kModePerformance
                    : mode == "betterBattery" ? &kModeBetterBattery : mode == "balanced" ? &kModeBalanced : nullptr;
    if (!g) return FailBool(L, "not a power mode: " + mode);
    if (!set || set(g) != ERROR_SUCCESS) return FailBool(L, "Windows didn't take it (this plan may not have power modes)");
    lua_pushboolean(L, true);
    return 1;
}

std::wstring GuidText(const GUID& g) {
    wchar_t s[64];
    StringFromGUID2(g, s, 64);
    return s;
}

// tourne.powerPlans(): {id, name, active}.
int ApiPowerPlans(lua_State* L) {
    lua_newtable(L);
    GUID* active = nullptr;
    PowerGetActiveScheme(nullptr, &active);
    GUID g;
    DWORD size = sizeof(g);
    int n = 0;
    for (ULONG i = 0; PowerEnumerate(nullptr, nullptr, nullptr, ACCESS_SCHEME, i, (UCHAR*)&g, &size) == ERROR_SUCCESS; i++, size = sizeof(g)) {
        wchar_t name[256] = L"";
        DWORD bytes = sizeof(name);
        PowerReadFriendlyName(nullptr, &g, nullptr, nullptr, (UCHAR*)name, &bytes);
        lua_createtable(L, 0, 3);
        lua_pushstring(L, U(GuidText(g)).c_str());
        lua_setfield(L, -2, "id");
        lua_pushstring(L, U(name).c_str());
        lua_setfield(L, -2, "name");
        lua_pushboolean(L, active && IsEqualGUID(*active, g));
        lua_setfield(L, -2, "active");
        lua_rawseti(L, -2, ++n);
    }
    if (active) LocalFree(active);
    return 1;
}

int ApiSetPowerPlan(lua_State* L) {
    Entry& e = E(L);
    if (!Granted(e, L"power")) return FailBool(L, "it wasn't allowed to change power settings when installed");
    GUID g;
    if (FAILED(CLSIDFromString(W(Arg(L, 1)).c_str(), &g))) return FailBool(L, "not a plan id");
    if (PowerSetActiveScheme(nullptr, &g) != ERROR_SUCCESS) return FailBool(L, "Windows didn't take it");
    lua_pushboolean(L, true);
    return 1;
}

// tourne.turbo(): whether the processor may boost in the active plan, plugged in (nil: unknown).
int ApiTurbo(lua_State* L) {
    GUID* active = nullptr;
    DWORD value = 0;
    bool ok = PowerGetActiveScheme(nullptr, &active) == ERROR_SUCCESS &&
              PowerReadACValueIndex(nullptr, active, &kProcessorGroup, &kBoostMode, &value) == ERROR_SUCCESS;
    if (active) LocalFree(active);
    if (!ok) return lua_pushnil(L), 1;
    lua_pushboolean(L, value != 0);
    return 1;
}

// tourne.setTurbo(on): boost Aggressive (2) or Disabled (0), plugged in and on battery, in the active plan.
int ApiSetTurbo(lua_State* L) {
    Entry& e = E(L);
    if (!Granted(e, L"power")) return FailBool(L, "it wasn't allowed to change power settings when installed");
    DWORD value = lua_toboolean(L, 1) ? 2 : 0;
    GUID* active = nullptr;
    bool ok = PowerGetActiveScheme(nullptr, &active) == ERROR_SUCCESS &&
              PowerWriteACValueIndex(nullptr, active, &kProcessorGroup, &kBoostMode, value) == ERROR_SUCCESS &&
              PowerWriteDCValueIndex(nullptr, active, &kProcessorGroup, &kBoostMode, value) == ERROR_SUCCESS &&
              PowerSetActiveScheme(nullptr, active) == ERROR_SUCCESS;
    if (active) LocalFree(active);
    if (!ok) return FailBool(L, "Windows didn't take it (it may need administrator rights)");
    lua_pushboolean(L, true);
    return 1;
}

// tourne.systemAction("lock" | "sleep" | "hibernate" | "signout" | "restart" | "shutdown").
int ApiSystemAction(lua_State* L) {
    Entry& e = E(L);
    if (!Granted(e, L"power")) return FailBool(L, "it wasn't allowed to change power settings when installed");
    std::string what = Arg(L, 1);
    bool ok = false;
    if (what == "lock") ok = LockWorkStation();
    else if (what == "sleep" || what == "hibernate") ok = SetSuspendState(what == "hibernate", FALSE, FALSE);
    else if (what == "signout" || what == "restart" || what == "shutdown") {
        HANDLE token = nullptr;
        if (what != "signout" && OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token)) {
            TOKEN_PRIVILEGES tp{1};
            LookupPrivilegeValueW(nullptr, SE_SHUTDOWN_NAME, &tp.Privileges[0].Luid);
            tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
            AdjustTokenPrivileges(token, FALSE, &tp, 0, nullptr, nullptr);
            CloseHandle(token);
        }
        UINT how = what == "signout" ? EWX_LOGOFF : what == "restart" ? EWX_REBOOT : EWX_SHUTDOWN | EWX_POWEROFF;
        ok = ExitWindowsEx(how, SHTDN_REASON_MAJOR_OTHER | SHTDN_REASON_FLAG_PLANNED);
    } else return FailBool(L, "not an action: " + what);
    Remember(Name(e) + L" asked Windows to " + W(what));
    lua_pushboolean(L, ok);
    return 1;
}

////////////////////////////////////////////////////////////////////////////////
// The user: idle, full screen, the clipboard, Windows' snip

// tourne.idleSeconds(): since the last key or mouse input.
int ApiIdleSeconds(lua_State* L) {
    LASTINPUTINFO info{sizeof(info)};
    GetLastInputInfo(&info);
    lua_pushnumber(L, (double)(GetTickCount() - info.dwTime) / 1000.0);
    return 1;
}

// tourne.busy(): something full screen is in front (a game, a video, a presentation), or quiet hours.
int ApiBusy(lua_State* L) {
    QUERY_USER_NOTIFICATION_STATE state{};
    bool busy = SUCCEEDED(SHQueryUserNotificationState(&state)) && (state == QUNS_BUSY || state == QUNS_RUNNING_D3D_FULL_SCREEN ||
                                                                     state == QUNS_PRESENTATION_MODE || state == QUNS_QUIET_TIME);
    lua_pushboolean(L, busy);
    return 1;
}

// tourne.clipboardSequence(): a number that changes whenever anything is copied; tourne.clipboardHasImage().
int ApiClipboardSequence(lua_State* L) {
    lua_pushnumber(L, (double)GetClipboardSequenceNumber());
    return 1;
}
int ApiClipboardHasImage(lua_State* L) {
    lua_pushboolean(L, IsClipboardFormatAvailable(CF_DIB) || IsClipboardFormatAvailable(CF_DIBV5) || IsClipboardFormatAvailable(CF_BITMAP));
    return 1;
}

// tourne.snip(): Windows' own region snip (its picture goes to the clipboard).
int ApiSnip(lua_State* L) {
    Entry& e = E(L);
    if (!Granted(e, L"open")) return FailBool(L, "it wasn't allowed to open apps when installed");
    bool ok = (INT_PTR)ShellExecuteW(nullptr, L"open", L"ms-screenclip:", nullptr, nullptr, SW_SHOWNORMAL) > 32;
    lua_pushboolean(L, ok);
    return 1;
}

////////////////////////////////////////////////////////////////////////////////
// Network interfaces (VPN detection, local addresses)

// tourne.networkInterfaces(): {name, description, up, vpn, ipv4 = {...}, ipv6 = {...}, gateway}.
int ApiNetworkInterfaces(lua_State* L) {
    Entry& e = E(L);
    lua_newtable(L);
    if (!Granted(e, L"systemInfo")) return TellOnce(e, "it wasn't allowed to read system information when installed"), 1;
    ULONG size = 32 << 10;
    std::vector<BYTE> buffer(size);
    ULONG flags = GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER | GAA_FLAG_INCLUDE_GATEWAYS;
    ULONG r = GetAdaptersAddresses(AF_UNSPEC, flags, nullptr, (IP_ADAPTER_ADDRESSES*)buffer.data(), &size);
    if (r == ERROR_BUFFER_OVERFLOW) buffer.resize(size), r = GetAdaptersAddresses(AF_UNSPEC, flags, nullptr, (IP_ADAPTER_ADDRESSES*)buffer.data(), &size);
    if (r != NO_ERROR) return 1;
    int n = 0;
    for (auto* a = (IP_ADAPTER_ADDRESSES*)buffer.data(); a; a = a->Next) {
        if (a->IfType == IF_TYPE_SOFTWARE_LOOPBACK) continue;
        std::wstring desc = a->Description ? a->Description : L"", name = a->FriendlyName ? a->FriendlyName : L"";
        std::wstring both = desc + L" " + name;
        for (auto& c : both) c = (wchar_t)towlower(c);
        bool vpn = a->IfType == IF_TYPE_PPP || a->IfType == IF_TYPE_TUNNEL || a->IfType == 53 /*PROP_VIRTUAL*/;
        for (const wchar_t* word : {L"vpn", L"wireguard", L"openvpn", L"tap-windows", L"wintun", L"tailscale", L"zerotier", L"nordlynx",
                                    L"proton", L"mullvad", L"expressvpn", L"cisco anyconnect", L"fortinet", L"globalprotect"})
            if (both.find(word) != std::wstring::npos) vpn = true;
        lua_createtable(L, 0, 6);
        lua_pushstring(L, U(name).c_str());
        lua_setfield(L, -2, "name");
        lua_pushstring(L, U(desc).c_str());
        lua_setfield(L, -2, "description");
        lua_pushboolean(L, a->OperStatus == IfOperStatusUp);
        lua_setfield(L, -2, "up");
        lua_pushboolean(L, vpn);
        lua_setfield(L, -2, "vpn");
        for (int family : {AF_INET, AF_INET6}) {
            lua_newtable(L);
            int k = 0;
            for (auto* u = a->FirstUnicastAddress; u; u = u->Next) {
                if (u->Address.lpSockaddr->sa_family != family) continue;
                wchar_t text[64];
                DWORD len = 64;
                if (WSAAddressToStringW(u->Address.lpSockaddr, u->Address.iSockaddrLength, nullptr, text, &len) == 0) {
                    lua_pushstring(L, U(text).c_str());
                    lua_rawseti(L, -2, ++k);
                }
            }
            lua_setfield(L, -2, family == AF_INET ? "ipv4" : "ipv6");
        }
        if (auto* gw = a->FirstGatewayAddress) {
            wchar_t text[64];
            DWORD len = 64;
            if (WSAAddressToStringW(gw->Address.lpSockaddr, gw->Address.iSockaddrLength, nullptr, text, &len) == 0) {
                lua_pushstring(L, U(text).c_str());
                lua_setfield(L, -2, "gateway");
            }
        }
        lua_rawseti(L, -2, ++n);
    }
    return 1;
}

////////////////////////////////////////////////////////////////////////////////
// The wallpaper

std::wstring CurrentWallpaperPath() {
    std::wstring out;
    IDesktopWallpaper* dw = nullptr;
    if (SUCCEEDED(CoCreateInstance(CLSID_DesktopWallpaper, nullptr, CLSCTX_ALL, IID_PPV_ARGS(&dw)))) {
        LPWSTR file = nullptr;
        if (dw->GetWallpaper(nullptr, &file) != S_OK || !file || !*file) {
            if (file) CoTaskMemFree(file), file = nullptr;
            LPWSTR id = nullptr;
            if (SUCCEEDED(dw->GetMonitorDevicePathAt(0, &id))) dw->GetWallpaper(id, &file);
            if (id) CoTaskMemFree(id);
        }
        if (file) out = file, CoTaskMemFree(file);
        dw->Release();
    }
    return out;
}

// The picture showing on one screen ("DISPLAY1", as noctalia.outputs() names it): Windows keeps one a screen
// when they differ. The first screen's when that screen isn't found.
std::wstring WallpaperOn(const std::string& connector) {
    std::wstring device = L"\\\\.\\" + W(connector);
    struct Find {
        std::wstring device;
        RECT rect{};
        bool found = false;
    } find{device};
    EnumDisplayMonitors(
        nullptr, nullptr,
        [](HMONITOR m, HDC, LPRECT, LPARAM p) -> BOOL {
            auto& f = *(Find*)p;
            MONITORINFOEXW info{};
            info.cbSize = sizeof(info);
            if (GetMonitorInfoW(m, &info) && !_wcsicmp(info.szDevice, f.device.c_str())) f.rect = info.rcMonitor, f.found = true;
            return !f.found;
        },
        (LPARAM)&find);
    std::wstring out;
    IDesktopWallpaper* dw = nullptr;
    if (find.found && SUCCEEDED(CoCreateInstance(CLSID_DesktopWallpaper, nullptr, CLSCTX_ALL, IID_PPV_ARGS(&dw)))) {
        UINT count = 0;
        dw->GetMonitorDevicePathCount(&count);
        for (UINT i = 0; i < count && out.empty(); i++) {
            LPWSTR id = nullptr;
            RECT r{};
            if (SUCCEEDED(dw->GetMonitorDevicePathAt(i, &id)) && id && SUCCEEDED(dw->GetMonitorRECT(id, &r)) && EqualRect(&r, &find.rect)) {
                LPWSTR file = nullptr;
                if (dw->GetWallpaper(id, &file) == S_OK && file && *file) out = file;
                if (file) CoTaskMemFree(file);
            }
            if (id) CoTaskMemFree(id);
        }
        dw->Release();
    }
    return out.empty() ? CurrentWallpaperPath() : out;
}

// Whether this file is a picture showing on a screen now (a plugin allowed the wallpaper may read it, wherever
// it is).
bool ShowingWallpaper(const std::wstring& full) {
    if (!_wcsicmp(Full(CurrentWallpaperPath()).c_str(), full.c_str())) return true;
    std::vector<std::string> screens;
    EnumDisplayMonitors(
        nullptr, nullptr,
        [](HMONITOR m, HDC, LPRECT, LPARAM p) -> BOOL {
            MONITORINFOEXW info{};
            info.cbSize = sizeof(info);
            if (GetMonitorInfoW(m, &info)) ((std::vector<std::string>*)p)->push_back(U(std::wstring(info.szDevice).substr(4)));
            return TRUE;
        },
        (LPARAM)&screens);
    for (auto const& s : screens)
        if (!_wcsicmp(Full(WallpaperOn(s)).c_str(), full.c_str())) return true;
    return false;
}

// noctalia.wallpaperPath(connector?): the picture showing (on that screen).
int ApiWallpaperPath(lua_State* L) {
    std::wstring p = lua_isstring(L, 1) ? WallpaperOn(Arg(L, 1)) : CurrentWallpaperPath();
    if (p.empty()) return lua_pushnil(L), 1;
    lua_pushstring(L, U(p).c_str());
    return 1;
}

// The slideshow's folder (shell.wallpaper.folder), else the showing picture's.
std::wstring WallpaperFolderNow() {
    std::wstring folder;
    if (const wchar_t* set = GetString(L"shell", L"wallpaper.folder")) {
        folder = set;
        FreeString(set);
    }
    if (folder.empty()) {
        std::wstring p = CurrentWallpaperPath();
        if (!p.empty()) folder = p.substr(0, p.find_last_of(L"\\/"));
    }
    return folder.empty() ? folder : Full(ExpandPath(U(folder)));
}

// noctalia.wallpaperDirectory()
int ApiWallpaperDirectory(lua_State* L) {
    std::wstring folder = WallpaperFolderNow();
    if (folder.empty()) return lua_pushnil(L), 1;
    lua_pushstring(L, U(folder).c_str());
    return 1;
}

// noctalia.setWallpaper(path) or (connector, path): set as Windows sets it, on a thread of its own (a big
// picture takes Windows a moment to copy). Tourne Desktop's desktop follows, as it does any change.
int ApiSetWallpaper(lua_State* L) {
    Entry& e = E(L);
    if (!Granted(e, L"wallpaper")) return FailBool(L, "it wasn't allowed to change the wallpaper when installed");
    bool two = lua_gettop(L) >= 2 && lua_isstring(L, 2);
    std::string given = Arg(L, two ? 2 : 1), connector = two ? Arg(L, 1) : "";
    std::wstring path = ExpandPath(given);
    DWORD a = GetFileAttributesW(path.c_str());
    if (a == INVALID_FILE_ATTRIBUTES || (a & FILE_ATTRIBUTE_DIRECTORY)) return FailBool(L, "no picture at " + given);
    std::wstring screen = W(connector);
    std::thread([path, screen] {
        CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
        IDesktopWallpaper* dw = nullptr;
        bool set = false;
        if (SUCCEEDED(CoCreateInstance(CLSID_DesktopWallpaper, nullptr, CLSCTX_ALL, IID_PPV_ARGS(&dw)))) {
            LPWSTR target = nullptr;
            if (!screen.empty()) {
                UINT count = 0;
                dw->GetMonitorDevicePathCount(&count);
                for (UINT i = 0; i < count && !target; i++) {
                    LPWSTR id = nullptr;
                    RECT r{};
                    if (SUCCEEDED(dw->GetMonitorDevicePathAt(i, &id)) && SUCCEEDED(dw->GetMonitorRECT(id, &r))) {
                        HMONITOR m = MonitorFromRect(&r, MONITOR_DEFAULTTONULL);
                        MONITORINFOEXW info{};
                        info.cbSize = sizeof(info);
                        if (m && GetMonitorInfoW(m, &info) && screen == (std::wstring(info.szDevice).rfind(L"\\\\.\\", 0) == 0 ? std::wstring(info.szDevice + 4) : std::wstring(info.szDevice)))
                            target = id, id = nullptr;
                    }
                    if (id) CoTaskMemFree(id);
                }
            }
            set = SUCCEEDED(dw->SetWallpaper(target, path.c_str()));
            if (target) CoTaskMemFree(target);
            dw->Release();
        }
        if (!set) SystemParametersInfoW(SPI_SETDESKWALLPAPER, 0, (void*)path.c_str(), SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
        CoUninitialize();
    }).detach();
    Remember(Name(e) + L" set the wallpaper: " + path);
    lua_pushboolean(L, true);
    return 1;
}

// tourne.thumbnail(picture, width, onDone): a small copy (PNG, kept in the plugin's data folder under thumbs\
// and made again when the picture changes), so a gallery doesn't hold every full picture. onDone(path or nil).
int ApiThumbnail(lua_State* L) {
    Entry& e = E(L);
    std::wstring src;
    std::string why;
    if (!Resolve(e, Arg(L, 1), false, &src, &why)) return FailBool(L, why);
    int width = std::clamp((int)luaL_checknumber(L, 2), 16, 1024);
    luaL_checktype(L, 3, LUA_TFUNCTION);
    WIN32_FILE_ATTRIBUTE_DATA info{};
    if (e.plugin->dataDir.empty() || !GetFileAttributesExW(src.c_str(), GetFileExInfoStandard, &info)) return FailBool(L, "no picture there");
    std::wstring key = src + L"|" + std::to_wstring(width) + L"|" + std::to_wstring(info.ftLastWriteTime.dwLowDateTime) + L"|" +
                       std::to_wstring(info.nFileSizeLow);
    std::wstring dir = e.plugin->dataDir + L"\\thumbs";
    std::wstring out = dir + L"\\" + std::to_wstring(std::hash<std::wstring>{}(key)) + L".png";
    int ref = lua_ref(L, 3);
    uint64_t serial = e.serial;
    auto answer = [serial, ref](std::wstring path) {
        Post([=] {
            auto found = g_bySerial.find(serial);
            if (found == g_bySerial.end()) return;
            Entry& to = *found->second;
            Guarded(to, [&] {
                lua_settop(to.T, 0);
                lua_getref(to.T, ref);
                lua_unref(to.T, ref);
                if (path.empty()) lua_pushnil(to.T);
                else lua_pushstring(to.T, U(path).c_str());
                return Call(to, 1);
            });
        });
    };
    if (GetFileAttributesW(out.c_str()) != INVALID_FILE_ATTRIBUTES) {
        answer(out);
        lua_pushboolean(L, true);
        return 1;
    }
    std::thread([=] {
        CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        SHCreateDirectoryExW(nullptr, dir.c_str(), nullptr);
        IWICImagingFactory* f = nullptr;
        IWICBitmapDecoder* decoder = nullptr;
        IWICBitmapFrameDecode* frame = nullptr;
        IWICBitmapScaler* scaler = nullptr;
        IWICFormatConverter* convert = nullptr;
        IWICStream* stream = nullptr;
        IWICBitmapEncoder* encoder = nullptr;
        IWICBitmapFrameEncode* target = nullptr;
        UINT w = 0, h = 0;
        bool ok = SUCCEEDED(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&f))) &&
                  SUCCEEDED(f->CreateDecoderFromFilename(src.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &decoder)) &&
                  SUCCEEDED(decoder->GetFrame(0, &frame)) && SUCCEEDED(frame->GetSize(&w, &h)) && w && h;
        UINT tw = std::min<UINT>(width, w), th = std::max<UINT>(1, (UINT)std::lround((double)h * tw / w));
        WICPixelFormatGUID format = GUID_WICPixelFormat32bppBGRA;
        ok = ok && SUCCEEDED(f->CreateBitmapScaler(&scaler)) && SUCCEEDED(scaler->Initialize(frame, tw, th, WICBitmapInterpolationModeFant)) &&
             SUCCEEDED(f->CreateFormatConverter(&convert)) &&
             SUCCEEDED(convert->Initialize(scaler, GUID_WICPixelFormat32bppBGRA, WICBitmapDitherTypeNone, nullptr, 0, WICBitmapPaletteTypeCustom)) &&
             SUCCEEDED(f->CreateStream(&stream)) && SUCCEEDED(stream->InitializeFromFilename((out + L".writing").c_str(), GENERIC_WRITE)) &&
             SUCCEEDED(f->CreateEncoder(GUID_ContainerFormatPng, nullptr, &encoder)) && SUCCEEDED(encoder->Initialize(stream, WICBitmapEncoderNoCache)) &&
             SUCCEEDED(encoder->CreateNewFrame(&target, nullptr)) && SUCCEEDED(target->Initialize(nullptr)) && SUCCEEDED(target->SetSize(tw, th)) &&
             SUCCEEDED(target->SetPixelFormat(&format)) && SUCCEEDED(target->WriteSource(convert, nullptr)) && SUCCEEDED(target->Commit()) &&
             SUCCEEDED(encoder->Commit());
        for (IUnknown* u : std::initializer_list<IUnknown*>{target, encoder, stream, convert, scaler, frame, decoder, f})
            if (u) u->Release();
        ok = ok && MoveFileExW((out + L".writing").c_str(), out.c_str(), MOVEFILE_REPLACE_EXISTING);
        if (!ok) DeleteFileW((out + L".writing").c_str());
        CoUninitialize();
        answer(ok ? out : L"");
    }).detach();
    lua_pushboolean(L, true);
    return 1;
}

////////////////////////////////////////////////////////////////////////////////
// Sound: Windows' own player (MCI) on the runtime's thread. WAV, MP3, WMA and whatever codecs Windows has.

std::wstring SoundAlias(const Entry& e, const std::string& name) {
    std::wstring alias = L"tp" + std::to_wstring(e.serial) + L"_";
    for (char c : name) alias += isalnum((unsigned char)c) ? (wchar_t)c : L'_';
    return alias;
}

bool Mci(const std::wstring& command) { return mciSendStringW(command.c_str(), nullptr, 0, nullptr) == 0; }

// Windows' own sounds (C:\Windows\Media) may be played by any plugin allowed sound.
bool WindowsSound(const std::string& given, std::wstring* out) {
    wchar_t windows[MAX_PATH];
    if (!GetWindowsDirectoryW(windows, MAX_PATH)) return false;
    std::wstring media = Full(std::wstring(windows) + L"\\Media"), full = Full(ExpandPath(given));
    if (full.empty() || !Inside(full, media) || GetFileAttributesW(full.c_str()) == INVALID_FILE_ATTRIBUTES) return false;
    *out = full;
    return true;
}

// noctalia.sound.load(name, path, onLoaded): ready to play under that name.
int ApiSoundLoad(lua_State* L) {
    Entry& e = E(L);
    std::string name = Arg(L, 1), given = Arg(L, 2);
    bool hasCallback = lua_isfunction(L, 3);
    std::string why;
    std::wstring path;
    if (!Granted(e, L"sound")) why = "it wasn't allowed to play sounds when installed";
    else if (!WindowsSound(given, &path) && !Resolve(e, given, false, &path, &why)) {
    } else {
        std::wstring alias = SoundAlias(e, name);
        Mci(L"close " + alias);
        // mpegvideo for .wav too: waveaudio can't loop or change volume.
        if (!Mci(L"open \"" + path + L"\" type mpegvideo alias " + alias)) why = "Windows can't play " + given;
    }
    if (hasCallback) {
        lua_pushvalue(L, 3);
        lua_pushboolean(L, why.empty());
        if (why.empty()) lua_pushnil(L);
        else lua_pushstring(L, why.c_str());
        lua_call(L, 2, 0);
    }
    lua_pushboolean(L, why.empty());
    return 1;
}

// noctalia.sound.play(name); Tourne's: sound.loop(name), sound.stop(name), sound.volume(name, 0..100).
int ApiSoundPlay(lua_State* L) {
    Entry& e = E(L);
    std::wstring alias = SoundAlias(e, Arg(L, 1));
    lua_pushboolean(L, Mci(L"play " + alias + L" from 0"));
    return 1;
}
int ApiSoundLoop(lua_State* L) {
    Entry& e = E(L);
    std::wstring alias = SoundAlias(e, Arg(L, 1));
    lua_pushboolean(L, Mci(L"play " + alias + L" from 0 repeat") || Mci(L"play " + alias + L" from 0"));
    return 1;
}
int ApiSoundStop(lua_State* L) {
    Entry& e = E(L);
    lua_pushboolean(L, Mci(L"stop " + SoundAlias(e, Arg(L, 1))));
    return 1;
}
int ApiSoundVolume(lua_State* L) {
    Entry& e = E(L);
    int v = std::clamp((int)luaL_checknumber(L, 2), 0, 100);
    lua_pushboolean(L, Mci(L"setaudio " + SoundAlias(e, Arg(L, 1)) + L" volume to " + std::to_wstring(v * 10)));
    return 1;
}

// An entry's sounds close with it.
void StopSounds(const Entry& e) {
    std::wstring prefix = L"tp" + std::to_wstring(e.serial) + L"_";
    wchar_t list[4096] = L"";
    if (mciSendStringW(L"sysinfo all name 0 open", nullptr, 0, nullptr) != 0) return;
    wchar_t count[16] = L"";
    mciSendStringW(L"sysinfo all quantity open", count, 16, nullptr);
    for (int i = _wtoi(count); i >= 1; i--) {
        if (mciSendStringW((L"sysinfo all name " + std::to_wstring(i) + L" open").c_str(), list, 4096, nullptr) != 0) continue;
        if (std::wstring(list).rfind(prefix, 0) == 0) Mci(std::wstring(L"close ") + list);
    }
}

////////////////////////////////////////////////////////////////////////////////
// The PC's sound: what an output plays (Windows' loopback), for a recorder such as ffmpeg, which reads it
// from a named pipe. Recording starts when the program opens the pipe and ends when it closes it, the
// output goes away, or the entry ends. Silence is played to the same output meanwhile: loopback sends
// nothing while nothing plays, and the sound would come out shorter than the video.

struct Capture {
    uint64_t entry = 0;
    HANDLE ready = CreateEventW(nullptr, TRUE, FALSE, nullptr), stop = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    std::string why, format;  // (set before ready)
    std::wstring pipe;
    int rate = 0, channels = 0;
    ~Capture() {
        for (HANDLE h : {ready, stop})
            if (h) CloseHandle(h);
    }
};
std::mutex g_capturesLock;
std::vector<std::shared_ptr<Capture>> g_captures;

// All of it into the pipe, or false (the program closed it, or the capture was told to stop).
bool PipeWrite(HANDLE pipe, HANDLE done, HANDLE stop, const BYTE* data, size_t size) {
    while (size) {
        OVERLAPPED ov{};
        ov.hEvent = done;
        ResetEvent(done);
        DWORD put = 0;
        if (!WriteFile(pipe, data, (DWORD)std::min<size_t>(size, 1 << 20), nullptr, &ov)) {
            if (GetLastError() != ERROR_IO_PENDING) return false;
            HANDLE both[] = {done, stop};
            if (WaitForMultipleObjects(2, both, FALSE, INFINITE) != WAIT_OBJECT_0) {
                CancelIo(pipe);
                GetOverlappedResult(pipe, &ov, &put, TRUE);
                return false;
            }
        }
        if (!GetOverlappedResult(pipe, &ov, &put, FALSE) || !put) return false;
        data += put, size -= put;
    }
    return true;
}

void CaptureSound(std::shared_ptr<Capture> c, std::wstring device) {
    using Microsoft::WRL::ComPtr;
    HRESULT com = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    {
        ComPtr<IMMDeviceEnumerator> devices;
        ComPtr<IMMDevice> output;
        ComPtr<IAudioClient> client, player;
        ComPtr<IAudioCaptureClient> capture;
        ComPtr<IAudioRenderClient> render;
        WAVEFORMATEX* mix = nullptr;
        HANDLE pipe = INVALID_HANDLE_VALUE;
        UINT32 playerFrames = 0;
        bool standard = device.empty() || device == L"default";
        if (FAILED(CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, IID_PPV_ARGS(&devices))))
            c->why = "Windows' sound isn't answering";
        else if (FAILED(standard ? devices->GetDefaultAudioEndpoint(eRender, eConsole, &output) : devices->GetDevice(device.c_str(), &output)))
            c->why = standard ? "there's no sound output" : "that sound output isn't there (unplugged?)";
        else if (FAILED(output->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, (void**)client.GetAddressOf())) ||
                 FAILED(client->GetMixFormat(&mix)))
            c->why = "the sound output can't be recorded";
        if (c->why.empty()) {
            // What ffmpeg is told it is (Windows' own mix is 32-bit float nearly always).
            WORD tag = mix->wFormatTag == WAVE_FORMAT_EXTENSIBLE ? (WORD)((WAVEFORMATEXTENSIBLE*)mix)->SubFormat.Data1 : mix->wFormatTag;
            int bits = mix->wBitsPerSample;
            c->format = tag == WAVE_FORMAT_IEEE_FLOAT && bits == 32 ? "f32le"
                        : tag == WAVE_FORMAT_PCM && bits == 16     ? "s16le"
                        : tag == WAVE_FORMAT_PCM && bits == 24     ? "s24le"
                        : tag == WAVE_FORMAT_PCM && bits == 32     ? "s32le"
                                                                   : "";
            c->rate = (int)mix->nSamplesPerSec, c->channels = mix->nChannels;
            if (c->format.empty()) c->why = "the sound output's format isn't one ffmpeg can be told";
            else if (FAILED(client->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_LOOPBACK, 2000000, 0, mix, nullptr)) ||
                     FAILED(client->GetService(IID_PPV_ARGS(&capture))))
                c->why = "the sound output can't be recorded";
        }
        if (c->why.empty()) {
            // The silence (without it, still a recording, with gaps where nothing played).
            BYTE* d = nullptr;
            if (SUCCEEDED(output->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, (void**)player.GetAddressOf())) &&
                SUCCEEDED(player->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, 2000000, 0, mix, nullptr)) &&
                SUCCEEDED(player->GetBufferSize(&playerFrames)) && SUCCEEDED(player->GetService(IID_PPV_ARGS(&render))) &&
                SUCCEEDED(render->GetBuffer(playerFrames, &d)))
                render->ReleaseBuffer(playerFrames, AUDCLNT_BUFFERFLAGS_SILENT);
            else
                render.Reset();
            static std::atomic<int> serial{0};
            c->pipe = L"\\\\.\\pipe\\tourne-sound-" + std::to_wstring(GetCurrentProcessId()) + L"-" + std::to_wstring(++serial);
            pipe = CreateNamedPipeW(c->pipe.c_str(), PIPE_ACCESS_OUTBOUND | FILE_FLAG_OVERLAPPED | FILE_FLAG_FIRST_PIPE_INSTANCE,
                                    PIPE_TYPE_BYTE | PIPE_WAIT | PIPE_REJECT_REMOTE_CLIENTS, 1, 4 << 20, 0, 0, nullptr);
            if (pipe == INVALID_HANDLE_VALUE) c->why = "Windows gave no pipe for the sound";
        }
        SetEvent(c->ready);

        // The program opens it (within 30 seconds, or it never started).
        HANDLE done = CreateEventW(nullptr, TRUE, FALSE, nullptr);
        bool connected = false;
        if (c->why.empty() && done) {
            OVERLAPPED ov{};
            ov.hEvent = done;
            DWORD got = 0;
            if (ConnectNamedPipe(pipe, &ov) || GetLastError() == ERROR_PIPE_CONNECTED) connected = true;
            else if (GetLastError() == ERROR_IO_PENDING) {
                HANDLE both[] = {done, c->stop};
                if (WaitForMultipleObjects(2, both, FALSE, 30000) == WAIT_OBJECT_0) connected = GetOverlappedResult(pipe, &ov, &got, FALSE);
                else {
                    CancelIo(pipe);
                    GetOverlappedResult(pipe, &ov, &got, TRUE);
                }
            }
        }
        if (connected && SUCCEEDED(client->Start())) {
            if (render) player->Start();
            std::vector<BYTE> silence;
            UINT32 block = mix->nBlockAlign;
            bool going = true;
            while (going && WaitForSingleObject(c->stop, 10) == WAIT_TIMEOUT) {
                UINT32 padding = 0, next = 0;
                BYTE* d = nullptr;
                if (render && SUCCEEDED(player->GetCurrentPadding(&padding)) && padding < playerFrames &&
                    SUCCEEDED(render->GetBuffer(playerFrames - padding, &d)))
                    render->ReleaseBuffer(playerFrames - padding, AUDCLNT_BUFFERFLAGS_SILENT);
                HRESULT hr = S_OK;
                while (going && SUCCEEDED(hr = capture->GetNextPacketSize(&next)) && next) {
                    BYTE* data = nullptr;
                    UINT32 frames = 0;
                    DWORD flags = 0;
                    if (FAILED(hr = capture->GetBuffer(&data, &frames, &flags, nullptr, nullptr))) break;
                    size_t bytes = (size_t)frames * block;
                    if (flags & AUDCLNT_BUFFERFLAGS_SILENT) {
                        silence.assign(bytes, 0);
                        data = silence.data();
                    }
                    going = PipeWrite(pipe, done, c->stop, data, bytes);
                    capture->ReleaseBuffer(frames);
                }
                if (FAILED(hr)) going = false;  // (the output went away)
            }
            client->Stop();
            if (render) player->Stop();
        }
        if (done) CloseHandle(done);
        if (pipe != INVALID_HANDLE_VALUE) CloseHandle(pipe);
        if (mix) CoTaskMemFree(mix);
    }
    if (SUCCEEDED(com)) CoUninitialize();
    std::lock_guard lock(g_capturesLock);
    g_captures.erase(std::remove(g_captures.begin(), g_captures.end(), c), g_captures.end());
}

// tourne.systemSound([output]): the sound an output plays, for a program to record: {input, format, rate,
// channels}, or false and why. output: "default" (or nothing) for Windows' default output, or an output's
// id (a setting with options_from = "sound-outputs" lists them). For ffmpeg, started within 30 seconds:
// -f <format> -ar <rate> -ac <channels> -i <input>.
int ApiSystemSound(lua_State* L) {
    Entry& e = E(L);
    if (!Granted(e, L"systemSound")) return FailBool(L, "it wasn't allowed to record the PC's sound when installed");
    auto c = std::make_shared<Capture>();
    c->entry = e.serial;
    if (!c->ready || !c->stop) return FailBool(L, "no events");
    {
        std::lock_guard lock(g_capturesLock);
        g_captures.push_back(c);
    }
    std::thread(CaptureSound, c, W(OptArg(L, 1))).detach();
    if (WaitForSingleObject(c->ready, 5000) != WAIT_OBJECT_0) {
        SetEvent(c->stop);
        return FailBool(L, "Windows' sound didn't answer");
    }
    if (!c->why.empty()) return FailBool(L, c->why);
    std::string input = U(c->pipe);
    lua_createtable(L, 0, 4);
    lua_pushlstring(L, input.data(), input.size());
    lua_setfield(L, -2, "input");
    lua_pushstring(L, c->format.c_str());
    lua_setfield(L, -2, "format");
    lua_pushinteger(L, c->rate);
    lua_setfield(L, -2, "rate");
    lua_pushinteger(L, c->channels);
    lua_setfield(L, -2, "channels");
    return 1;
}

// An entry's recordings of the PC's sound end with it.
void StopCaptures(const Entry& e) {
    std::lock_guard lock(g_capturesLock);
    for (auto& c : g_captures)
        if (c->entry == e.serial) SetEvent(c->stop);
}

////////////////////////////////////////////////////////////////////////////////

void OpenSystemApi(lua_State* L) {
    // On the noctalia table (at the top of the stack).
    SetFunctions(L, {{"runAsync", ApiRunAsync}, {"runStream", ApiRunStream}, {"commandExists", ApiCommandExists},
                     {"processMatches", ApiProcessMatches}, {"systemStats", ApiSystemStats}, {"diskStats", ApiDiskStats},
                     {"outputs", ApiOutputs}, {"focusedOutputName", ApiFocusedOutputName}, {"wallpaperPath", ApiWallpaperPath},
                     {"wallpaperDirectory", ApiWallpaperDirectory}, {"setWallpaper", ApiSetWallpaper}});
    lua_newtable(L);
    SetFunctions(L, {{"load", ApiSoundLoad}, {"play", ApiSoundPlay}, {"loop", ApiSoundLoop}, {"stop", ApiSoundStop},
                     {"volume", ApiSoundVolume}});
    lua_setfield(L, -2, "sound");
}

// Tourne'Table's panel (its Place In = Panel window), built in here or running in Windhawk: the top-level
// window of its overlay class titled "Tourne'Table".
BOOL CALLBACK FindTablePanel(HWND w, LPARAM out) {
    wchar_t cls[64] = L"", title[32] = L"";
    GetClassNameW(w, cls, ARRAYSIZE(cls));
    if (wcsncmp(cls, L"DesktopAudioVisOverlay_", 23)) return TRUE;
    GetWindowTextW(w, title, ARRAYSIZE(title));
    if (wcscmp(title, L"Tourne'Table")) return TRUE;
    *(HWND*)out = w;
    return FALSE;
}

// tourne.tablePanel([what]): "toggle", "show", "hide" or "pin" (on top, or not), and with nothing just
// asks. True when the panel shows afterwards, false when it's hidden, nil when there is no panel
// (Tourne'Table off, or its Place In isn't Panel).
int ApiTablePanel(lua_State* L) {
    std::string what = OptArg(L, 1);
    HWND panel = nullptr;
    EnumWindows(FindTablePanel, (LPARAM)&panel);
    if (!panel) {
        lua_pushnil(L);
        return 1;
    }
    static const UINT message = RegisterWindowMessageW(L"TourneTable.Panel");
    WPARAM op = what == "toggle" ? 0 : what == "show" ? 1 : what == "hide" ? 2 : what == "pin" ? 3 : 99;
    DWORD_PTR shown = IsWindowVisible(panel);
    if (op != 99 && !SendMessageTimeoutW(panel, message, op, 0, SMTO_ABORTIFHUNG, 2000, &shown)) shown = IsWindowVisible(panel);
    lua_pushboolean(L, shown != 0);
    return 1;
}

// tourne.keybinds(): every key Tourne takes, for the Keybind Guide. { enabled, lines = { the Switcher's bind lines },
// file = its binds file, fileLines = { that file's lines }, hotkeys = { { name, keys }, ... } (Tourne's own: the
// taskbar's, the desktops', the tiling leader and plugin panels'), keys as the setting has them ("ctrl+alt+77").
int ApiKeybinds(lua_State* L) {
    std::map<std::wstring, std::wstring> set;
    for (auto const& [k, v] : SettingsWithPrefix(L"switcher.binds.")) set[k] = v;
    lua_newtable(L);
    std::wstring on = set[L"switcher.binds.enabled"];
    lua_pushboolean(L, on == L"true" || _wtoi(on.c_str()) != 0);
    lua_setfield(L, -2, "enabled");
    lua_newtable(L);
    for (int i = 0, n = 0, empty = 0; i < 1024 && empty < 8; i++) {
        std::wstring line = set[L"switcher.binds.lines[" + std::to_wstring(i) + L"]"];
        empty = line.empty() ? empty + 1 : 0;
        if (line.empty()) continue;
        lua_pushstring(L, U(line).c_str());
        lua_rawseti(L, -2, ++n);
    }
    lua_setfield(L, -2, "lines");
    std::wstring file = set[L"switcher.binds.file"];
    lua_pushstring(L, U(file).c_str());
    lua_setfield(L, -2, "file");
    lua_newtable(L);
    if (!file.empty()) {
        std::string body;
        if (ReadAll(ExpandPath(U(file)), &body)) {
            if (body.rfind("\xEF\xBB\xBF", 0) == 0) body.erase(0, 3);
            size_t at = 0;
            for (int n = 0; at <= body.size() && n < 4096;) {
                size_t end = body.find('\n', at);
                std::string line = body.substr(at, end == std::string::npos ? std::string::npos : end - at);
                if (!line.empty() && line.back() == '\r') line.pop_back();
                lua_pushstring(L, line.c_str());
                lua_rawseti(L, -2, ++n);
                if (end == std::string::npos) break;
                at = end + 1;
            }
        }
    }
    lua_setfield(L, -2, "fileLines");
    lua_newtable(L);
    int n = 0;
    auto hotkey = [&](std::wstring const& name, std::wstring const& keys) {
        if (keys.empty() || keys == L"none" || keys == L"\"\"") return;
        lua_newtable(L);
        lua_pushstring(L, U(name).c_str());
        lua_setfield(L, -2, "name");
        lua_pushstring(L, U(keys).c_str());
        lua_setfield(L, -2, "keys");
        lua_rawseti(L, -2, ++n);
    };
    auto setting = [](const wchar_t* key) {
        for (auto const& [k, v] : SettingsWithPrefix(key))
            if (k == key) return v;
        return std::wstring();
    };
    hotkey(L"Open the Tourne Menu", setting(L"taskbar.menu.hotkey"));
    hotkey(L"Hide or show the taskbar", setting(L"taskbar.hideHotkey"));
    hotkey(L"Next desktop", setting(L"taskbar.desktopKeys.next"));
    hotkey(L"Desktop before", setting(L"taskbar.desktopKeys.previous"));
    hotkey(L"Send the window to the next desktop", setting(L"taskbar.desktopKeys.moveNext"));
    hotkey(L"Send the window to the desktop before", setting(L"taskbar.desktopKeys.movePrevious"));
    if (std::wstring m = setting(L"taskbar.desktopKeys.goTo"); !m.empty() && m != L"none") hotkey(L"Go to desktop N", m + L"+1-9");
    if (std::wstring m = setting(L"taskbar.desktopKeys.moveTo"); !m.empty() && m != L"none") hotkey(L"Send the window to desktop N", m + L"+1-9");
    hotkey(L"Tiling: the leader key", setting(L"switcher.tiling.leader"));
    for (auto const& p : g_plugins)
        for (auto const& e : p->entries)
            if (e->kind == "panel" && e->L)
                if (std::string k = e->settings["hotkey"].text; k.find_first_not_of(" \t") != std::string::npos) {
                    std::string title = e->plugin->manifest["name"].text;
                    hotkey(L"Open " + (title.empty() ? Name(*e) : W(title)), W(k));
                }
    lua_setfield(L, -2, "hotkeys");
    return 1;
}

// tourne.keystrokes([what]): "toggle", "show" or "hide" the keycaps on screen (keystrokes.cpp) for now, or with
// nothing just asks. True when they show afterwards.
int ApiKeystrokes(lua_State* L) {
    std::string what = OptArg(L, 1);
    bool on = what == "toggle" ? KeystrokesShow(-1) : what == "show" ? KeystrokesShow(1) : what == "hide" ? KeystrokesShow(0) : KeystrokesShowing();
    lua_pushboolean(L, on);
    return 1;
}

// tourne.openStudio(page): Tourne Studio at a page of its own settings ("keystrokes", "switcher.tiling").
int ApiOpenStudio(lua_State* L) {
    std::wstring page = W(OptArg(L, 1));
    if (!page.empty() && page.back() != L'.') page += L'.';
    OpenStudio(page);
    return 0;
}

// tourne.openKeysAndRules(): the Keys and rules window (binds-editor.cpp), where binds are made without a file.
int ApiOpenKeysAndRules(lua_State*) {
    OpenBindsEditor();
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// tourne.keyEvents(on): while on, the entry's onKeyPress(kind) runs for each key or button pressed anywhere
// (not held keys' repeats): kind is "key", "space", "enter", "backspace" or "click", never which key, so a
// plugin can't read what's typed (Bongo Cat, Typing Sounds). Low-level hooks on a thread of their own, only
// while some entry wants them.

std::set<uint64_t> g_keyWanters;
std::mutex g_keyLock;
HANDLE g_keyThread = nullptr;
DWORD g_keyThreadId = 0;
std::set<DWORD> g_keysDown;  // (the hook thread's own)

void FollowKeyWanters();

void KeyPressed(const char* kind) {
    std::string k = kind;
    Post([k] {
        std::vector<uint64_t> to;
        {
            std::lock_guard lock(g_keyLock);
            to.assign(g_keyWanters.begin(), g_keyWanters.end());
        }
        bool anyone = false;
        for (uint64_t serial : to) anyone |= g_bySerial.count(serial) != 0;
        if (!anyone) return FollowKeyWanters();  // (the ones that asked are gone: the hooks go too)
        for (uint64_t serial : to) {
            auto found = g_bySerial.find(serial);
            if (found == g_bySerial.end()) continue;
            CallGlobal(*found->second, "onKeyPress", [&](lua_State* T) {
                lua_pushstring(T, k.c_str());
                return 1;
            });
        }
    });
}

LRESULT CALLBACK KeyEventsKeyboard(int code, WPARAM wp, LPARAM lp) {
    if (code == HC_ACTION) {
        auto* k = (KBDLLHOOKSTRUCT*)lp;
        bool down = wp == WM_KEYDOWN || wp == WM_SYSKEYDOWN;
        if (down && !g_keysDown.count(k->vkCode)) {
            g_keysDown.insert(k->vkCode);
            KeyPressed(k->vkCode == VK_SPACE ? "space" : k->vkCode == VK_RETURN ? "enter" : k->vkCode == VK_BACK ? "backspace" : "key");
        } else if (!down) {
            g_keysDown.erase(k->vkCode);
        }
    }
    return CallNextHookEx(nullptr, code, wp, lp);
}

LRESULT CALLBACK KeyEventsMouse(int code, WPARAM wp, LPARAM lp) {
    if (code == HC_ACTION && (wp == WM_LBUTTONDOWN || wp == WM_RBUTTONDOWN || wp == WM_MBUTTONDOWN)) KeyPressed("click");
    return CallNextHookEx(nullptr, code, wp, lp);
}

DWORD WINAPI KeyEventsThread(void*) {
    HHOOK kb = SetWindowsHookExW(WH_KEYBOARD_LL, KeyEventsKeyboard, GetModuleHandleW(nullptr), 0);
    HHOOK mouse = SetWindowsHookExW(WH_MOUSE_LL, KeyEventsMouse, GetModuleHandleW(nullptr), 0);
    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) DispatchMessageW(&msg);
    if (kb) UnhookWindowsHookEx(kb);
    if (mouse) UnhookWindowsHookEx(mouse);
    g_keysDown.clear();
    return 0;
}

void FollowKeyWanters() {
    bool want;
    {
        std::lock_guard lock(g_keyLock);
        for (auto it = g_keyWanters.begin(); it != g_keyWanters.end();)
            it = g_bySerial.count(*it) ? std::next(it) : g_keyWanters.erase(it);  // (entries gone since)
        want = !g_keyWanters.empty();
    }
    if (want && !g_keyThread) {
        g_keyThread = CreateThread(nullptr, 0, KeyEventsThread, nullptr, 0, &g_keyThreadId);
    } else if (!want && g_keyThread) {
        PostThreadMessageW(g_keyThreadId, WM_QUIT, 0, 0);
        WaitForSingleObject(g_keyThread, 1000);
        CloseHandle(g_keyThread);
        g_keyThread = nullptr;
    }
}

int ApiKeyEvents(lua_State* L) {
    Entry& e = E(L);
    bool on = lua_isnoneornil(L, 1) || lua_toboolean(L, 1);
    {
        std::lock_guard lock(g_keyLock);
        if (on) g_keyWanters.insert(e.serial);
        else g_keyWanters.erase(e.serial);
    }
    FollowKeyWanters();
    lua_pushboolean(L, on);
    return 1;
}

////////////////////////////////////////////////////////////////////////////////
// tourne.displays(): each screen {name, label, width, height, hz, primary, modes = {{width, height, hz}...}} (the
// modes each size and rate once, biggest first). tourne.setDisplayMode(name, width, height, hz): that screen in
// that mode, kept (as Windows' Display settings keeps it); true if Windows took it.

int ApiDisplays(lua_State* L) {
    lua_newtable(L);
    DISPLAY_DEVICEW dd{sizeof(dd)};
    int n = 0;
    for (DWORD i = 0; EnumDisplayDevicesW(nullptr, i, &dd, 0); i++, dd = {sizeof(dd)}) {
        if (!(dd.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP)) continue;
        DEVMODEW now{};
        now.dmSize = sizeof(now);
        EnumDisplaySettingsW(dd.DeviceName, ENUM_CURRENT_SETTINGS, &now);
        DISPLAY_DEVICEW monitor{sizeof(monitor)};
        EnumDisplayDevicesW(dd.DeviceName, 0, &monitor, 0);
        lua_newtable(L);
        auto field = [&](const char* k, const std::wstring& v) {
            std::string s = U(v);
            lua_pushstring(L, s.c_str());
            lua_setfield(L, -2, k);
        };
        auto number = [&](const char* k, double v) {
            lua_pushnumber(L, v);
            lua_setfield(L, -2, k);
        };
        field("name", dd.DeviceName);
        field("label", monitor.DeviceString[0] ? monitor.DeviceString : dd.DeviceString);
        number("width", now.dmPelsWidth), number("height", now.dmPelsHeight), number("hz", now.dmDisplayFrequency);
        lua_pushboolean(L, (dd.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE) != 0);
        lua_setfield(L, -2, "primary");
        std::vector<std::tuple<DWORD, DWORD, DWORD>> modes;
        DEVMODEW m{};
        m.dmSize = sizeof(m);
        for (DWORD k = 0; EnumDisplaySettingsW(dd.DeviceName, k, &m); k++)
            if (m.dmBitsPerPel >= 24) modes.push_back({m.dmPelsWidth, m.dmPelsHeight, m.dmDisplayFrequency});
        std::sort(modes.rbegin(), modes.rend());
        modes.erase(std::unique(modes.begin(), modes.end()), modes.end());
        lua_newtable(L);
        int j = 0;
        for (auto [w, h, hz] : modes) {
            lua_newtable(L);
            number("width", w), number("height", h), number("hz", hz);
            lua_rawseti(L, -2, ++j);
        }
        lua_setfield(L, -2, "modes");
        lua_rawseti(L, -2, ++n);
    }
    return 1;
}

int ApiSetDisplayMode(lua_State* L) {
    std::wstring name = W(Arg(L, 1));
    DEVMODEW m{};
    m.dmSize = sizeof(m);
    m.dmPelsWidth = (DWORD)luaL_checknumber(L, 2), m.dmPelsHeight = (DWORD)luaL_checknumber(L, 3);
    m.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;
    if (lua_isnumber(L, 4)) m.dmDisplayFrequency = (DWORD)lua_tonumber(L, 4), m.dmFields |= DM_DISPLAYFREQUENCY;
    LONG r = ChangeDisplaySettingsExW(name.c_str(), &m, nullptr, CDS_UPDATEREGISTRY, nullptr);
    lua_pushboolean(L, r == DISP_CHANGE_SUCCESSFUL);
    return 1;
}

////////////////////////////////////////////////////////////////////////////////
// tourne.games(): the games installed through Steam and the Epic Games Launcher, {name, store, launch}, launch a
// link only those launchers open. tourne.launchGame(link): one started (steam:// and com.epicgames.launcher://
// links only).

std::string ReadSmall(const std::wstring& path) {
    std::string s;
    ReadAll(path, &s);
    return s;
}

// "key" "value" pairs of a Valve KeyValues file, flat (enough for libraryfolders.vdf and appmanifest .acf).
std::vector<std::pair<std::string, std::string>> VdfPairs(const std::string& text) {
    std::vector<std::pair<std::string, std::string>> out;
    std::vector<std::string> tokens;
    for (size_t i = 0; i < text.size(); i++) {
        if (text[i] == '"') {
            std::string t;
            for (i++; i < text.size() && text[i] != '"'; i++) {
                if (text[i] == '\\' && i + 1 < text.size()) i++;
                t += text[i];
            }
            tokens.push_back(t);
        } else if (text[i] == '{' || text[i] == '}') {
            tokens.push_back(std::string(1, text[i]));
        }
    }
    for (size_t i = 0; i + 1 < tokens.size(); i++)
        if (tokens[i] != "{" && tokens[i] != "}" && tokens[i + 1] != "{" && tokens[i + 1] != "}") out.push_back({tokens[i], tokens[i + 1]}), i++;
    return out;
}

int ApiGames(lua_State* L) {
    lua_newtable(L);
    int n = 0;
    auto add = [&](const std::string& name, const char* store, const std::string& link) {
        lua_newtable(L);
        lua_pushstring(L, name.c_str()), lua_setfield(L, -2, "name");
        lua_pushstring(L, store), lua_setfield(L, -2, "store");
        lua_pushstring(L, link.c_str()), lua_setfield(L, -2, "launch");
        lua_rawseti(L, -2, ++n);
    };
    // Steam: its libraries, then each library's app manifests.
    WCHAR steam[MAX_PATH] = L"";
    DWORD size = sizeof(steam);
    if (RegGetValueW(HKEY_CURRENT_USER, L"Software\\Valve\\Steam", L"SteamPath", RRF_RT_REG_SZ, nullptr, steam, &size) == ERROR_SUCCESS) {
        std::vector<std::wstring> libraries = {steam};
        for (auto const& [k, v] : VdfPairs(ReadSmall(std::wstring(steam) + L"\\steamapps\\libraryfolders.vdf")))
            if (k == "path") libraries.push_back(W(v));
        std::set<std::string> seen;
        for (auto const& lib : libraries) {
            WIN32_FIND_DATAW fd;
            std::wstring apps = lib + L"\\steamapps\\";
            HANDLE find = FindFirstFileW((apps + L"appmanifest_*.acf").c_str(), &fd);
            if (find == INVALID_HANDLE_VALUE) continue;
            do {
                std::string id, name;
                for (auto const& [k, v] : VdfPairs(ReadSmall(apps + fd.cFileName))) {
                    if (k == "appid" && id.empty()) id = v;
                    if (k == "name" && name.empty()) name = v;
                }
                // (Steam's own runtimes and redistributables aren't games)
                if (id.empty() || name.empty() || seen.count(id) || name.find("Steamworks") != std::string::npos ||
                    name.find("Proton") == 0 || name.find("Steam Linux Runtime") == 0)
                    continue;
                seen.insert(id);
                add(name, "Steam", "steam://rungameid/" + id);
            } while (FindNextFileW(find, &fd));
            FindClose(find);
        }
    }
    // Epic: its manifests.
    WCHAR data[MAX_PATH];
    if (ExpandEnvironmentStringsW(L"%ProgramData%\\Epic\\EpicGamesLauncher\\Data\\Manifests\\", data, MAX_PATH)) {
        WIN32_FIND_DATAW fd;
        HANDLE find = FindFirstFileW((std::wstring(data) + L"*.item").c_str(), &fd);
        if (find != INVALID_HANDLE_VALUE) {
            do {
                json::Value v;
                if (!json::Parse(ReadSmall(std::wstring(data) + fd.cFileName), &v)) continue;
                std::string name = v["DisplayName"].text, ns = v["CatalogNamespace"].text, item = v["CatalogItemId"].text,
                            app = v["AppName"].text;
                if (name.empty() || app.empty()) continue;
                add(name, "Epic", "com.epicgames.launcher://apps/" + ns + "%3A" + item + "%3A" + app + "?action=launch&silent=true");
            } while (FindNextFileW(find, &fd));
            FindClose(find);
        }
    }
    return 1;
}

int ApiLaunchGame(lua_State* L) {
    std::string link = Arg(L, 1);
    bool ok = (link.rfind("steam://rungameid/", 0) == 0 || link.rfind("com.epicgames.launcher://apps/", 0) == 0) &&
              link.find_first_of(" \"\r\n") == std::string::npos &&
              (INT_PTR)ShellExecuteW(nullptr, L"open", W(link).c_str(), nullptr, nullptr, SW_SHOWNORMAL) > 32;
    lua_pushboolean(L, ok);
    return 1;
}

////////////////////////////////////////////////////////////////////////////////
// tourne.desktopWidgets([what]): every plugin's desktop widgets "hide", "show" or "toggle" (for now; they come
// back when Tourne Desktop starts again). True when they show afterwards.

bool g_desktopWidgetsHidden = false;

int ApiDesktopWidgets(lua_State* L) {
    std::string what = OptArg(L, 1);
    if (what == "toggle") g_desktopWidgetsHidden = !g_desktopWidgetsHidden;
    else if (what == "hide") g_desktopWidgetsHidden = true;
    else if (what == "show") g_desktopWidgetsHidden = false;
    if (!what.empty())
        for (auto& p : g_plugins)
            for (auto& other : p->entries)
                if (other->kind == "desktop_widget" && other->panel && other->panel->Hwnd())
                    ShowWindow(other->panel->Hwnd(), g_desktopWidgetsHidden ? SW_HIDE : SW_SHOWNOACTIVATE);
    lua_pushboolean(L, !g_desktopWidgetsHidden);
    return 1;
}

// tourne.keepAwake("off" | "system" | "display"): the PC (and with "display" the screen too) kept awake while the
// plugin asks or runs; the most any plugin asks for wins. (Called on the runtime's thread only, which is the thread
// Windows keeps the request for.)
std::map<uint64_t, int> g_awake;  // entry serial: 1 the PC, 2 the PC and the screen

void ApplyAwake() {
    int most = 0;
    for (auto const& [serial, how] : g_awake) most = std::max(most, how);
    SetThreadExecutionState(ES_CONTINUOUS | (most ? ES_SYSTEM_REQUIRED : 0) | (most > 1 ? ES_DISPLAY_REQUIRED : 0));
}

int ApiKeepAwake(lua_State* L) {
    Entry& e = E(L);
    std::string how = OptArg(L, 1);
    if (!Granted(e, L"power")) return FailBool(L, "it wasn't allowed to change power settings when installed");
    if (how != "off" && how != "system" && how != "display") return FailBool(L, "keepAwake takes off, system or display, not " + how);
    if (how == "off") g_awake.erase(e.serial);
    else g_awake[e.serial] = how == "display" ? 2 : 1;
    ApplyAwake();
    lua_pushboolean(L, true);
    return 1;
}

void StopAwake(const Entry& e) {
    if (g_awake.erase(e.serial)) ApplyAwake();
}

// tourne.barItems(): every other plugin's bar widget as the bar shows it, for a control centre:
// {{key, plugin, glyph (its name), text, tip, color (the role it set, or "")}...}.
int ApiBarItems(lua_State* L) {
    Entry& self = E(L);
    lua_newtable(L);
    int n = 0;
    for (auto const& p : g_plugins)
        for (auto const& e : p->entries) {
            if (e->kind != "widget" || !e->L || !e->visible || e->plugin == self.plugin) continue;
            lua_newtable(L);
            auto set = [&](const char* field, const std::string& v) {
                lua_pushlstring(L, v.data(), v.size());
                lua_setfield(L, -2, field);
            };
            set("key", U(Name(*e)));
            set("plugin", p->manifest["name"].text);
            set("glyph", e->glyphName.empty() ? p->manifest["icon"].text : e->glyphName);
            set("text", U(e->text));
            set("tip", U(e->tip));
            set("color", e->colorRole);
            lua_rawseti(L, -2, ++n);
        }
    return 1;
}

// tourne.barClick(key, "left" | "right" | "middle"): as a click on that bar widget.
int ApiBarClick(lua_State* L) {
    std::wstring key = W(Arg(L, 1));
    std::string how = OptArg(L, 2);
    PluginBarEvent event = how == "right" ? kPluginRightClick : how == "middle" ? kPluginMiddleClick : kPluginClick;
    Later([key, event] { Event(key, event); });
    return 0;
}

// tourne.uiSound(name): one of Tourne Desktop's built-in UI cues ("pop", "on", "off", "success"...), when UI
// sounds are on and plugins may play them (Studio, Sounds). True if it was played.
int ApiUiSound(lua_State* L) {
    bool ok = UiSoundsForPlugins();
    if (ok) UiSound(W(Arg(L, 1)));
    lua_pushboolean(L, ok);
    return 1;
}

// tourne.openLauncher(line?): the launcher opened, with that line typed ("g " lists the games). False if it's off.
int ApiOpenLauncher(lua_State* L) {
    std::wstring line = W(OptArg(L, 1)).substr(0, 250);
    HWND launcher = FindWindowW(L"TourneLauncher", nullptr);
    ATOM atom = line.empty() || !launcher ? 0 : GlobalAddAtomW(line.c_str());
    if (launcher) PostMessageW(launcher, RegisterWindowMessageW(L"TourneDesktop.Launcher"), atom, 0);
    lua_pushboolean(L, launcher != nullptr);
    return 1;
}

void OpenTourneApi(lua_State* L) {
    lua_newtable(L);
    SetFunctions(L, {{"processes", ApiProcesses}, {"endProcess", ApiEndProcess}, {"gpus", ApiGpus}, {"battery", ApiBattery},
                     {"powerMode", ApiPowerMode}, {"setPowerMode", ApiSetPowerMode}, {"powerPlans", ApiPowerPlans},
                     {"setPowerPlan", ApiSetPowerPlan}, {"turbo", ApiTurbo}, {"setTurbo", ApiSetTurbo},
                     {"systemAction", ApiSystemAction}, {"idleSeconds", ApiIdleSeconds}, {"busy", ApiBusy},
                     {"clipboardSequence", ApiClipboardSequence}, {"clipboardHasImage", ApiClipboardHasImage}, {"snip", ApiSnip},
                     {"networkInterfaces", ApiNetworkInterfaces}, {"thumbnail", ApiThumbnail}, {"write", ApiStreamWrite},
                     {"stop", ApiStreamStop}, {"tablePanel", ApiTablePanel}, {"systemSound", ApiSystemSound},
                     {"screenIndex", ApiScreenIndex}, {"keybinds", ApiKeybinds}, {"captureLink", ApiCaptureLink},
                     {"openKeysAndRules", ApiOpenKeysAndRules},
                     {"keystrokes", ApiKeystrokes}, {"openStudio", ApiOpenStudio}, {"keyEvents", ApiKeyEvents},
                     {"displays", ApiDisplays}, {"setDisplayMode", ApiSetDisplayMode}, {"games", ApiGames},
                     {"launchGame", ApiLaunchGame}, {"desktopWidgets", ApiDesktopWidgets}, {"openLauncher", ApiOpenLauncher}, {"keepAwake", ApiKeepAwake},
                     {"barItems", ApiBarItems}, {"barClick", ApiBarClick},
                     {"uiSound", ApiUiSound}});
    lua_setglobal(L, "tourne");
}
