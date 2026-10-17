--!nonstrict
-- Ported to Tourne Desktop from nightwatch75's Claude Cockpit (MIT).
-- live: pure data module for the Live tab, same rules as sessions.luau: no
-- UI state. M.refresh reads Claude Code's own per-process state files (what
-- live-claude-sessions did) and sorts the result into the three attention
-- groups the panel renders: waiting (needs the user), busy, idle.
--
-- Polled by the service only while the panel's Live tab or the bar widget
-- in activity mode asks for it (live_wanted_at).
--
-- Windows: Claude Code keeps one %USERPROFILE%\.claude\sessions\<pid>.json per
-- live process. A file whose process is gone, or whose pid was reused (the
-- process Windows runs under that pid started after the session did), is a
-- crash leftover and is skipped: tourne.processInfo stands in for /proc.
-- Per-session stats (model, context size, cost, title, last prompt, branch,
-- permission mode) come from the session transcript, read with tourne.jsonLines
-- (jq on Linux) and kept per session keyed on its size and mtime, so an idle
-- session costs one fileInfo per run. Cost is priced with the
-- pricing-cache.json the usage engine keeps in the plugin's data folder (USD;
-- nil when absent). Assistant lines repeat per content block with the same
-- message.id and usage, so usage is counted once per message id.

local M = {}

local CLAUDE_DIR = noctalia.expandPath("~/.claude")
local SESSIONS_DIR = CLAUDE_DIR .. "\\sessions"
local PROJECTS_DIR = CLAUDE_DIR .. "\\projects"
-- A process may be a little younger than the startedAt it wrote (clock and
-- rounding): more than this and the pid belongs to another program.
local START_SLACK_MS = 10000

-- sessionId -> { path, key = "size:mtime", stats }; dropped once the session ends.
local cache = {}

M.GROUPS = { "waiting", "busy", "idle" }

local function str(v)
    return type(v) == "string" and v or ""
end

local function shape(raw)
    local status = str(raw.status)
    if status ~= "waiting" and status ~= "busy" then
        status = "idle"
    end
    local name = str(raw.title)
    if name == "" then
        name = str(raw.name)
    end
    local bridge = str(raw.bridgeSessionId)
    return {
        pid = tonumber(raw.pid) or 0,
        id = str(raw.sessionId),
        cwd = str(raw.cwd),
        name = name ~= "" and name or str(raw.sessionId):sub(1, 8),
        status = status,
        waitingFor = str(raw.waitingFor),
        -- Claude Code stamps milliseconds; the panel works in seconds.
        since = math.floor((tonumber(raw.statusUpdatedAt) or tonumber(raw.startedAt) or 0) / 1000),
        lastPrompt = str(raw.lastPrompt),
        model = str(raw.model),
        contextTokens = tonumber(raw.contextTokens) or 0,
        costUSD = tonumber(raw.costUSD),
        gitBranch = str(raw.gitBranch),
        permissionMode = str(raw.permissionMode),
        -- Set while Remote Control is on: the session is then also open on claude.ai.
        webUrl = bridge:match("^session_[%w]+$") and ("https://claude.ai/code/" .. bridge) or nil,
    }
end

local function pricingModels()
    local dataDir = noctalia.pluginDataDir()
    local p = dataDir and noctalia.json.decode(noctalia.readFile(dataDir .. "/pricing-cache.json") or "")
    if type(p) == "table" and type(p.models) == "table" and next(p.models) ~= nil then
        return p.models
    end
    return nil
end

-- What transcript_stats' jq made of a transcript's rows.
local function statsOf(rows, price)
    local usage, order = {}, {}
    local st = { model = "", ctx = 0, title = "", prompt = "", branch = "", mode = "" }
    for _, o in ipairs(rows) do
        local t = o.type
        local u = o["message.usage"]
        if t == "assistant" and type(u) == "table" then
            local id = str(o["message.id"])
            if id == "" then id = str(o.uuid) end
            if usage[id] == nil then order[#order + 1] = id end
            usage[id] = { model = str(o["message.model"]), u = u }
            if str(o["message.model"]) ~= "" then st.model = o["message.model"] end
            st.ctx = (tonumber(u.input_tokens) or 0) + (tonumber(u.cache_read_input_tokens) or 0)
                + (tonumber(u.cache_creation_input_tokens) or 0)
        elseif t == "ai-title" then
            if type(o.aiTitle) == "string" then st.title = o.aiTitle end
        elseif t == "last-prompt" then
            if type(o.lastPrompt) == "string" then st.prompt = o.lastPrompt end
        elseif t == "permission-mode" then
            if type(o.permissionMode) == "string" then st.mode = o.permissionMode end
        end
        if type(o.gitBranch) == "string" and o.gitBranch ~= "" then st.branch = o.gitBranch end
    end
    local cost = nil
    if price ~= nil then
        cost = 0
        for _, id in ipairs(order) do
            local m = usage[id]
            local parts = string.split(m.model, "-")
            local family = parts[1] == "claude" and parts[2] or parts[1]
            local p = family and price[family]
            if type(p) == "table" then
                local u = m.u
                cost += (tonumber(u.input_tokens) or 0) * (tonumber(p.input) or 0)
                    + (tonumber(u.output_tokens) or 0) * (tonumber(p.output) or 0)
                    + (tonumber(u.cache_read_input_tokens) or 0) * (tonumber(p.cache_read) or 0)
                    + (tonumber(u.cache_creation_input_tokens) or 0) * (tonumber(p.cache_write) or 0)
            end
        end
    end
    return {
        title = st.title, lastPrompt = st.prompt, model = st.model, contextTokens = st.ctx,
        gitBranch = st.branch, permissionMode = st.mode, costUSD = cost,
    }
end

local STAT_FIELDS = {
    "type", "uuid", "message.id", "message.model", "message.usage", "aiTitle", "lastPrompt", "permissionMode", "gitBranch",
}

-- The session's live state files, each with its process still the one that wrote it.
local function liveStates()
    local out = {}
    for _, name in ipairs(noctalia.listDir(SESSIONS_DIR) or {}) do
        if name:match("%.json$") then
            local st = noctalia.json.decode(noctalia.readFile(SESSIONS_DIR .. "\\" .. name) or "")
            local pid = type(st) == "table" and tonumber(st.pid)
            local sid = type(st) == "table" and str(st.sessionId) or ""
            if pid and pid > 0 and sid:match("^[%x-]+$") then
                local p = tourne.processInfo(pid)
                local startedAt = tonumber(st.startedAt)
                local reused = type(p) == "table" and startedAt ~= nil and tonumber(p.started) ~= nil
                    and p.started > startedAt + START_SLACK_MS
                if type(p) == "table" and not reused then
                    out[#out + 1] = st
                end
            end
        end
    end
    return out
end

-- The transcript of each session not found yet: projects\*\<sessionId>.jsonl.
-- A session with none yet (just started) is looked for again after a while,
-- not on every 2 s tick.
local RETRY_SECS = 15
local function findTranscripts(states, done)
    local want, wanted = {}, {}
    for _, st in ipairs(states) do
        local c = cache[st.sessionId]
        if c == nil or (c.path == nil and os.time() - (c.missingAt or 0) >= RETRY_SECS) then
            want[#want + 1] = st.sessionId .. ".jsonl"
            wanted[st.sessionId] = true
        end
    end
    if #want == 0 then return done() end
    tourne.findFiles(PROJECTS_DIR, { name = want, maxDepth = 2, max = 200 }, function(found)
        for _, f in ipairs(type(found) == "table" and found or {}) do
            local sid = str(f.path):match("([%x-]+)%.jsonl$")
            if sid and wanted[sid] then
                cache[sid] = { path = f.path }
                wanted[sid] = nil
            end
        end
        for sid in pairs(wanted) do
            cache[sid] = { missingAt = os.time() }
        end
        done()
    end)
end

-- done(groups, err): groups = { waiting = {...}, busy = {...}, idle = {...},
-- count = n, costUSD = sum }, each list newest status change first.
function M.refresh(done)
    if not noctalia.fileExists(SESSIONS_DIR) then
        done({ waiting = {}, busy = {}, idle = {}, count = 0, costUSD = 0 }, nil)
        return
    end
    local states = liveStates()
    local live = {}
    for _, st in ipairs(states) do live[st.sessionId] = true end
    for sid in pairs(cache) do
        if not live[sid] then cache[sid] = nil end
    end

    findTranscripts(states, function()
        -- Transcripts changed since their stats were made, read together.
        local stale, staleKeys = {}, {}
        for _, st in ipairs(states) do
            local c = cache[st.sessionId]
            if c and c.path then
                local info = noctalia.fileInfo(c.path)
                local key = type(info) == "table" and `{info.size}:{info.mtime}` or ""
                if key ~= "" and key ~= c.key then
                    stale[#stale + 1] = c.path
                    staleKeys[c.path] = { c = c, key = key }
                end
            end
        end

        local function finish()
            local groups = { waiting = {}, busy = {}, idle = {}, count = 0, costUSD = 0 }
            for _, st in ipairs(states) do
                local raw = {
                    pid = st.pid, sessionId = st.sessionId, cwd = st.cwd, name = st.name, status = st.status,
                    waitingFor = st.waitingFor, statusUpdatedAt = st.statusUpdatedAt, startedAt = st.startedAt,
                    bridgeSessionId = st.bridgeSessionId,
                }
                local c = cache[st.sessionId]
                for k, v in pairs(c and c.stats or {}) do raw[k] = v end
                local s = shape(raw)
                table.insert(groups[s.status], s)
                groups.count += 1
                groups.costUSD += s.costUSD or 0
            end
            for _, g in ipairs(M.GROUPS) do
                table.sort(groups[g], function(a, b)
                    return a.since > b.since
                end)
            end
            done(groups, nil)
        end

        if #stale == 0 then return finish() end
        local price = pricingModels()
        tourne.jsonLines({ files = stale, fields = STAT_FIELDS, maxRows = 500000 }, function(files)
            for _, f in ipairs(type(files) == "table" and files or {}) do
                local entry = staleKeys[f.path]
                if entry and f.rows then
                    entry.c.stats = statsOf(f.rows, price)
                    entry.c.key = entry.key
                end
            end
            finish()
        end)
    end)
end

-- Focuses the session's terminal window (the nearest parent process with a
-- window, as focus-claude-session walked the ancestors); false when not found.
function M.focus(session, done)
    local pid = tonumber(session.pid)
    done(pid ~= nil and pid > 0 and tourne.focusProcess(pid) == true)
end

return M
