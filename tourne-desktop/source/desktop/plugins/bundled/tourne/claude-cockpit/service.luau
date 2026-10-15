--!nonstrict
-- Ported to Tourne Desktop from nightwatch75's Claude Cockpit (MIT).
-- Claude Cockpit, headless service (usage data owner).
--
-- Runs the usage engine (usage.luau, the Luau port of get-claude-usage --json,
-- which the original copied from jrohland/claudecode, MIT, see README Notes)
-- on the configured interval and publishes its result so the panel's Usage tab can read it
-- without shelling out itself. Sessions and CLAUDE.md are deliberately NOT
-- handled here: they are read on-demand by the panel (open + refresh
-- button), never polled: scanning every session file on a tick would scan
-- 60+ MB of transcripts for no reason most of the time.
--
-- Live sessions ARE polled here, every LIVE_TICK_MS, but only while someone
-- asks for them: the bar widget in activity mode and the panel on its Live
-- tab stamp "live_wanted_at" every tick; with no stamp in the last
-- LIVE_WANTED_SECS the poll stops. One poller, however many readers.
--
--   noctalia.state.set("usage", <table>)   -- the parsed usage object
--   noctalia.state.set("status", {...})    -- loading / ready / error + availability
--   noctalia.state.set("live", { groups, error })  -- see live.luau
--
-- Force a refresh from a command line:
--   TourneDesktop.exe --msg plugin tourne/claude-cockpit:service all refresh
--
-- Windows: no bash, jq or curl; the engine reads %USERPROFILE%\.claude itself
-- and asks the web through noctalia.http.

local live = require("./live.luau")
local engine = require("./usage.luau")

-- The currencies the Usage tab offers (get-claude-usage's default list).
local CURRENCIES = "EUR,GBP,JPY,CNY,CHF,AUD,CAD,INR"
local LIVE_TICK_MS = 2000
local LIVE_WANTED_SECS = 5
local livePolling = false
local lastUsageFetch = nil
local depsOk = true
local fetching = false
local fetchStarted = 0
local claudeVersion = nil

local function publishStatus(state, message)
    noctalia.state.set("status", { state = state, available = depsOk, message = message or "" })
end

-- usage_refresh_interval is in minutes (2 to 15).
local function intervalSecs()
    local mins = noctalia.getConfig("usage_refresh_interval")
    if type(mins) ~= "number" or mins < 1 then
        mins = 2
    end
    return math.floor(mins * 60)
end

-- get-claude-usage sends "claude-code/<claude --version>" as its User-Agent
-- (2.0.0 when claude isn't found); asked once, before the first fetch.
local askingVersion = false
local function askVersion(andThen)
    if askingVersion then return end
    askingVersion = true
    local function done(v)
        claudeVersion = v or ""
        andThen()
    end
    if not noctalia.commandExists("claude") then return done(nil) end
    local ok = noctalia.runAsync({ "claude", "--version" }, function(result)
        done(type(result) == "table" and result.exitCode == 0 and tostring(result.stdout or ""):match("(%d[%d.]*)") or nil)
    end, 15000)
    if not ok then done(nil) end
end

local function fetch()
    if claudeVersion == nil then
        return askVersion(fetch)
    end
    -- (A run that hit an error part way never comes back: after five minutes another may start.)
    if fetching and os.time() - fetchStarted < 300 then
        return
    end
    lastUsageFetch = os.time()
    local dataDir = noctalia.pluginDataDir()
    if dataDir == nil then
        publishStatus("error", "Could not make the plugin's data folder")
        return
    end

    fetching = true
    fetchStarted = os.time()
    publishStatus("loading")

    local apiUrl = noctalia.getConfig("currency_api_url")
    if type(apiUrl) ~= "string" or apiUrl == "" then
        apiUrl = "https://api.frankfurter.dev/v1/latest"
    end

    engine.collect({
        claudeDir = noctalia.expandPath("~/.claude"),
        dataDir = dataDir,
        version = claudeVersion ~= "" and claudeVersion or nil,
        currencyTargets = CURRENCIES,
        currencyApiUrl = apiUrl,
    }, function(data)
        fetching = false
        if type(data) ~= "table" then
            noctalia.log("claude-cockpit: the usage engine returned nothing")
            publishStatus("error", "Bad data")
            return
        end
        noctalia.state.set("usage", data)
        publishStatus("ready")
    end)
end

local function pollLive()
    if livePolling then
        return
    end
    livePolling = true
    live.refresh(function(groups, err)
        livePolling = false
        noctalia.state.set("live", { groups = groups, error = err })
    end)
end

-- update() fires immediately at load and then every LIVE_TICK_MS: usage on
-- start and then every N minutes, live sessions only while wanted.
function update()
    noctalia.setUpdateInterval(LIVE_TICK_MS)
    if lastUsageFetch == nil or os.time() - lastUsageFetch >= intervalSecs() then
        fetch()
    end
    local wanted = tonumber(noctalia.state.get("live_wanted_at"))
    if wanted ~= nil and os.time() - wanted <= LIVE_WANTED_SECS then
        pollLive()
    end
end

-- Bumped by a reader that wants fresh data now (tab switch, refresh button).
noctalia.state.watch("live_nonce", function()
    pollLive()
end)

-- The panel bumps this nonce (its refresh button) to force an out-of-band fetch.
noctalia.state.watch("refresh_nonce", function()
    fetch()
end)

function onIpc(event)
    if event == "refresh" then
        fetch()
    end
end
