--!nonstrict
-- Ported to Tourne Desktop from raycursive's Discord Voice (MIT).
-- Headless service: owns the long-lived Python bridge and publishes each
-- complete Discord voice snapshot for the widget and panel.
-- Windows: the bridge runs as "python" (or the Python setting's full path),
-- given its arguments as a list (no shell). Tourne Desktop shows only pictures
-- in the plugin's own folders, so on Windows the bridge names each avatar's
-- address and this service downloads it into the data folder.

local BRIDGE_STATE_KEY = "voice_snapshot"

local bridgePath = noctalia.pluginDir() .. "/discord_bridge.py"

local windows = require("./windows.luau")

local function bridgeCommand(...)
  return { windows.pythonProgram(), bridgePath, ... }
end

local function initialSnapshot(status, message)
  return {
    revision = 0,
    status = status,
    status_message = message,
    authenticated = false,
    user = {},
    channel = nil,
    recent_channels = {},
    favorite_channels = {},
    participants = {},
    voice = { mute = false, deaf = false, input_volume = 100 },
    connection = {
      state = "DISCONNECTED",
      hostname = "",
      average_ping = 0,
      last_ping = 0,
    },
    connected_since = nil,
  }
end

local function shallowCopy(value)
  local copy = {}
  for key, item in pairs(value) do
    copy[key] = item
  end
  return copy
end

-- (Tourne's formatTime has no "%s"; os.time() is the same count of seconds.)
local function nowSeconds()
  return os.time()
end

local lastLineAt = nowSeconds()
local streamStarted = false
local restartInProgress = false
local nextRestartProbeAt = 0
local pythonMissing = false
local lastLogLine = nil

-- Windows avatars: <data folder>/avatars/<user id>-<avatar hash>.png, fetched
-- once from Discord's CDN with noctalia.download.
local avatarFolderPath = nil
local avatarKnown = {}
local avatarLoading = {}
local avatarRetryAt = {}

local function avatarFolder()
  if avatarFolderPath then
    return avatarFolderPath
  end
  local data = noctalia.pluginDataDir()
  if not data then
    return nil
  end
  local folder = data .. "/avatars"
  if not noctalia.mkdirAll(folder) then
    return nil
  end
  avatarFolderPath = folder
  return folder
end

local fillAvatars

local function fetchAvatar(file, url, destination)
  avatarLoading[file] = true
  local started = noctalia.download(url, destination, function(ok)
    avatarLoading[file] = nil
    if not ok then
      avatarRetryAt[file] = nowSeconds() + 60
      noctalia.log("discord-voice: could not cache an avatar from " .. url)
      return
    end
    avatarKnown[file] = destination
    local current = noctalia.state.get(BRIDGE_STATE_KEY)
    if type(current) == "table" and fillAvatars(current) then
      noctalia.state.set(BRIDGE_STATE_KEY, current)
    end
  end)
  if not started then
    avatarLoading[file] = nil
    avatarRetryAt[file] = nowSeconds() + 60
  end
end

-- Gives each participant whose avatar is saved its avatar_path, and starts the
-- downloads still missing. True when a path was filled in.
fillAvatars = function(snapshot)
  local filled = false
  if type(snapshot.participants) ~= "table" then
    return filled
  end
  for _, participant in ipairs(snapshot.participants) do
    local url, file = participant.avatar_url, participant.avatar_file
    if (participant.avatar_path or "") == "" and type(url) == "string" and url ~= ""
      and type(file) == "string" and file:match("^[%w_%-]+%.png$") then
      local folder = avatarFolder()
      if folder then
        local destination = folder .. "/" .. file
        if not avatarKnown[file] and noctalia.fileExists(destination) then
          avatarKnown[file] = destination
        end
        if avatarKnown[file] then
          participant.avatar_path = avatarKnown[file]
          filled = true
        elseif not avatarLoading[file] and nowSeconds() >= (avatarRetryAt[file] or 0) then
          fetchAvatar(file, url, destination)
        end
      end
    end
  end
  return filled
end

local function onBridgeLine(line)
  lastLineAt = nowSeconds()
  nextRestartProbeAt = 0
  -- Tourne's runStream also hands over the bridge's stderr (its log lines),
  -- each logged once in a row (it repeats "Discord desktop is not running"
  -- every few seconds while Discord is closed).
  if line:sub(1, 1) ~= "{" then
    if line ~= lastLogLine then
      lastLogLine = line
      noctalia.log("discord-voice: " .. line)
    end
    return
  end
  local event, err = noctalia.json.decode(line)
  if type(event) ~= "table" then
    noctalia.log("discord-voice: ignored malformed bridge output: " .. tostring(err or "unknown error"))
    return
  end
  if event.type == "heartbeat" then
    return
  end
  if event.type ~= "snapshot" then
    return
  end
  event.type = nil
  fillAvatars(event)
  noctalia.state.set(BRIDGE_STATE_KEY, event)
end

-- Exit code 9009 is Windows' "python" stand-in that only offers the Microsoft
-- Store: no Python is installed.
local function onBridgeExit(code)
  if code == 9009 then
    pythonMissing = true
    noctalia.state.set(BRIDGE_STATE_KEY, initialSnapshot("error", noctalia.tr("error.python_missing")))
  end
end

local function startBridge()
  if streamStarted or pythonMissing then
    return
  end
  if not noctalia.commandExists(windows.pythonProgram()) then
    noctalia.state.set(BRIDGE_STATE_KEY, initialSnapshot("error", noctalia.tr("error.python_missing")))
    return
  end
  noctalia.state.set(BRIDGE_STATE_KEY, initialSnapshot("starting", ""))
  streamStarted = noctalia.runStream(bridgeCommand("daemon"), onBridgeLine, onBridgeExit) and true or false
  if streamStarted then
    lastLineAt = nowSeconds()
  else
    noctalia.state.set(BRIDGE_STATE_KEY, initialSnapshot("error", noctalia.tr("error.bridge_start")))
  end
end

local function markBridgeStalled()
  local current = noctalia.state.get(BRIDGE_STATE_KEY)
  local previous = type(current) == "table"
    and shallowCopy(current)
    or initialSnapshot("error", "")
  previous.status_message = noctalia.tr("error.bridge_stalled")
  noctalia.state.set(BRIDGE_STATE_KEY, previous)
end

local function restartBridge()
  if restartInProgress then
    return
  end
  restartInProgress = true
  markBridgeStalled()

  local started = noctalia.runAsync(
    bridgeCommand("command", "shutdown"),
    function(result)
      restartInProgress = false
      if result.exitCode == 9009 then
        onBridgeExit(9009)
        return
      end
      local response = noctalia.json.decode(result.stdout or "")
      local bridgeAbsent = type(response) == "table" and response.bridge_state == "absent"
      if result.exitCode == 0 or bridgeAbsent then
        streamStarted = false
        nextRestartProbeAt = 0
        startBridge()
        return
      end

      -- A live but unresponsive helper may still own the control socket. Do
      -- not launch a duplicate; probe it again later in case it exits.
      nextRestartProbeAt = nowSeconds() + 30
      markBridgeStalled()
    end,
    10000
  )
  if not started then
    restartInProgress = false
    nextRestartProbeAt = nowSeconds() + 30
  end
end

noctalia.setUpdateInterval(5000)
startBridge()

function update()
  if pythonMissing then
    return
  end
  -- The bridge emits a heartbeat every five seconds. Surface a stalled helper
  -- and restart it. This also recovers from a helper that exits during a
  -- Noctalia hot-reload handoff.
  if not streamStarted then
    startBridge()
    return
  end
  local now = nowSeconds()
  if now - lastLineAt > 20 and now >= nextRestartProbeAt then
    restartBridge()
  end
end

-- (Noctalia's `noctalia msg plugin ...` events. Tourne Desktop has no such
-- command line; the panel runs the bridge's commands itself.)
function onIpc(event, _payload)
  if event == "authorize" then
    noctalia.runAsync(bridgeCommand("command", "authorize"))
  elseif event == "refresh" then
    noctalia.runAsync(bridgeCommand("command", "refresh"))
  elseif event == "hangup" then
    noctalia.runAsync(bridgeCommand("command", "hang-up"))
  elseif event == "dump" then
    local snapshot = noctalia.state.get(BRIDGE_STATE_KEY)
    local safe = {
      status = snapshot.status,
      authenticated = snapshot.authenticated,
      channel = snapshot.channel,
      recent_channels = snapshot.recent_channels,
      favorite_channels = snapshot.favorite_channels,
      participants = #snapshot.participants,
      voice = snapshot.voice,
      connection = snapshot.connection,
    }
    noctalia.log("discord-voice: " .. (noctalia.json.encode(safe) or "?"))
  end
end
