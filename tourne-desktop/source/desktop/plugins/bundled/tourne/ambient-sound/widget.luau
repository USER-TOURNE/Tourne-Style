--!nonstrict
-- Ambient Sounds on the bar: the mixer plays from here (so the sounds keep going with the panel closed).
-- Click: the panel. Middle click: play or pause (or mute, by the setting). Right click: mute.
-- Scroll: the overall volume.
--
-- Idea from hthienloc's Ambient Sounds (DankMaterialShell); fresh code. Its sound files aren't bundled
-- (their own licences, and 27 MB): noise is made here, everything else comes from your folder.
-- The panel asks through noctalia.state ("ambient.command"); what it shows is "ambient".

local PLAYABLE = { mp3 = true, wav = true, wma = true, m4a = true, aac = true, ogg = true, flac = true, opus = true }

local s = { vols = {}, on = {}, master = 70, playing = false, muted = false, presets = {}, timerEnds = nil }
local sounds = {}   -- { { path, name } } in the panel's order
local loaded = {}   -- path -> sound name it was loaded as

local function cfg(key, fallback)
  local v = noctalia.getConfig(key)
  if v == nil or v == "" then return fallback end
  return v
end

local function dataDir()
  return noctalia.pluginDataDir() or ""
end

-- A looping noise file: 12 s of 16-bit mono at 22050 Hz, its end blended into its start so the loop
-- has no click.
local function makeNoise(kind, path)
  local rate, len, fade = 22050, 22050 * 12, 2205
  local n = len + fade
  local raw = table.create(n, 0)
  local b0, b1, b2, b3, b4, b5, b6, last = 0, 0, 0, 0, 0, 0, 0, 0
  local peak = 0
  for i = 1, n do
    local w = math.random() * 2 - 1
    local v
    if kind == "pink" then -- Paul Kellet's filter
      b0 = 0.99886 * b0 + w * 0.0555179
      b1 = 0.99332 * b1 + w * 0.0750759
      b2 = 0.96900 * b2 + w * 0.1538520
      b3 = 0.86650 * b3 + w * 0.3104856
      b4 = 0.55000 * b4 + w * 0.5329522
      b5 = -0.7616 * b5 - w * 0.0168980
      v = b0 + b1 + b2 + b3 + b4 + b5 + b6 + w * 0.5362
      b6 = w * 0.115926
    elseif kind == "brown" then
      last = (last + 0.02 * w) / 1.02
      v = last
    else
      v = w
    end
    raw[i] = v
    peak = math.max(peak, math.abs(v))
  end
  for i = 1, fade do
    local t = (i - 1) / fade
    raw[i] = raw[i] * t + raw[len + i] * (1 - t)
  end
  local scale = 0.8 * 32767 / math.max(peak, 1e-6)
  local data = buffer.create(44 + len * 2)
  buffer.writestring(data, 0, "RIFF")
  buffer.writeu32(data, 4, 36 + len * 2)
  buffer.writestring(data, 8, "WAVEfmt ")
  buffer.writeu32(data, 16, 16)
  buffer.writeu16(data, 20, 1)
  buffer.writeu16(data, 22, 1)
  buffer.writeu32(data, 24, rate)
  buffer.writeu32(data, 28, rate * 2)
  buffer.writeu16(data, 32, 2)
  buffer.writeu16(data, 34, 16)
  buffer.writestring(data, 36, "data")
  buffer.writeu32(data, 40, len * 2)
  for i = 1, len do
    buffer.writei16(data, 44 + (i - 1) * 2, math.clamp(math.floor(raw[i] * scale), -32768, 32767))
  end
  noctalia.writeFile(path, buffer.tostring(data))
end

local function save()
  noctalia.mkdirAll(dataDir())
  noctalia.writeFile(dataDir() .. "\\mix.json", noctalia.json.encode({
    vols = s.vols, on = s.on, master = s.master, presets = s.presets,
    wasPlaying = s.playing,
  }))
end

local function publish()
  local list = {}
  for _, snd in sounds do
    table.insert(list, { path = snd.path, name = snd.name, vol = s.vols[snd.path] or cfg("defaultVolume", 50),
      on = s.on[snd.path] == true })
  end
  local names = {}
  for name in s.presets do table.insert(names, name) end
  table.sort(names)
  noctalia.state.set("ambient", { sounds = list, master = s.master, playing = s.playing, muted = s.muted,
    timerEnds = s.timerEnds, presets = names, folder = noctalia.expandPath(cfg("folder", "~/Music/Ambient")) })
end

local function scan()
  sounds = {}
  if cfg("noise", true) then
    for _, kind in { "white", "pink", "brown" } do
      local path = dataDir() .. "\\" .. kind .. "-noise.wav"
      if not noctalia.fileExists(path) then
        noctalia.mkdirAll(dataDir())
        makeNoise(kind, path)
      end
      table.insert(sounds, { path = path, name = string.upper(string.sub(kind, 1, 1)) .. string.sub(kind, 2) .. " noise" })
    end
  end
  local folder = noctalia.expandPath(cfg("folder", "~/Music/Ambient"))
  noctalia.mkdirAll(folder)
  local names = noctalia.listDir(folder) or {}
  table.sort(names, function(a, b) return string.lower(a) < string.lower(b) end)
  for _, name in names do
    local ext = string.lower(string.match(name, "%.([^%.]+)$") or "")
    if PLAYABLE[ext] then
      table.insert(sounds, { path = folder .. "\\" .. name, name = string.gsub(name, "%.[^%.]+$", "") })
    end
  end
end

-- Every sound as it should be now: playing at its volume, or quiet.
local function apply()
  for i, snd in sounds do
    local name = "s" .. tostring(i)
    local wanted = s.playing and s.on[snd.path] == true
    if wanted then
      if loaded[snd.path] ~= name then
        local why
        local ok = noctalia.sound.load(name, snd.path, function(_good, err) why = err end)
        if not ok then
          s.on[snd.path] = nil
          noctalia.notifyError("Can't play " .. snd.name, tostring(why))
          continue
        end
        loaded[snd.path] = name
        noctalia.sound.loop(name)
      end
      local vol = s.muted and 0 or math.floor((s.vols[snd.path] or cfg("defaultVolume", 50)) * s.master / 100)
      noctalia.sound.volume(name, vol)
    elseif loaded[snd.path] then
      noctalia.sound.stop(loaded[snd.path])
      loaded[snd.path] = nil
    end
  end
end

local function show()
  local count = 0
  local names = {}
  for _, snd in sounds do
    if s.on[snd.path] then
      count += 1
      table.insert(names, snd.name)
    end
  end
  barWidget.setGlyph(s.muted and "headphones-off" or "headphones")
  barWidget.setColor(s.playing and not s.muted and count > 0 and "primary" or "on_surface")
  local text = ""
  if s.timerEnds and s.playing then
    local m = math.max(0, math.ceil((s.timerEnds - noctalia.nowMs()) / 60000))
    text = tostring(m) .. "m"
  end
  barWidget.setText(text)
  if count == 0 then
    barWidget.setTooltip("Ambient sounds\nClick to pick some")
  else
    barWidget.setTooltip((s.playing and "Playing: " or "Paused: ") .. table.concat(names, ", ")
      .. string.format("\nVolume %d%%%s", s.master, s.muted and " (muted)" or ""))
  end
end

local function changed()
  apply()
  save()
  publish()
  show()
end

local function setTimer(minutes)
  minutes = tonumber(minutes) or 0
  s.timerEnds = minutes > 0 and (noctalia.nowMs() + minutes * 60000) or nil
  noctalia.setUpdateInterval(s.timerEnds and 1000 or 30000)
end

local function play(on)
  s.playing = on
  if on and not s.timerEnds then setTimer(cfg("timer", "0")) end
  if not on then setTimer(0) end
  changed()
end

function update()
  if s.timerEnds and s.playing and noctalia.nowMs() >= s.timerEnds then
    play(false)
    local after = cfg("whenTimerEnds", "stop")
    if after == "lock" or after == "sleep" then tourne.systemAction(after) end
  end
  show()
end

function onClick()
  noctalia.togglePanel("tourne/ambient-sound:panel")
end

function onMiddleClick()
  if cfg("middleClick", "play") == "mute" then
    s.muted = not s.muted
    changed()
  else
    play(not s.playing)
  end
end

function onRightClick()
  s.muted = not s.muted
  changed()
end

function onScroll(delta)
  s.master = math.clamp(s.master + ((tonumber(delta) or 0) > 0 and 5 or -5), 0, 100)
  changed()
end

noctalia.state.watch("ambient.command", function(c)
  if type(c) ~= "table" then return end
  noctalia.state.set("ambient.command", nil)
  if c.op == "toggle" then
    s.on[c.path] = not s.on[c.path] or nil
    if s.on[c.path] and not s.playing then
      play(true)
      return
    end
  elseif c.op == "volume" then
    s.vols[c.path] = math.clamp(tonumber(c.value) or 50, 0, 100)
  elseif c.op == "master" then
    s.master = math.clamp(tonumber(c.value) or 70, 0, 100)
  elseif c.op == "play" then
    play(not s.playing)
    return
  elseif c.op == "mute" then
    s.muted = not s.muted
  elseif c.op == "timer" then
    setTimer(c.minutes)
  elseif c.op == "save" and c.name and c.name ~= "" then
    local mix = {}
    for path in s.on do mix[path] = s.vols[path] or cfg("defaultVolume", 50) end
    s.presets[c.name] = mix
  elseif c.op == "load" and s.presets[c.name] then
    s.on = {}
    for path, vol in s.presets[c.name] do
      s.on[path] = true
      s.vols[path] = vol
    end
    if not s.playing then
      play(true)
      return
    end
  elseif c.op == "delete" then
    s.presets[c.name] = nil
  elseif c.op == "rescan" then
    for _, name in loaded do noctalia.sound.stop(name) end
    loaded = {}
    scan()
  end
  changed()
end)

do
  local text = noctalia.readFile(dataDir() .. "\\mix.json")
  local old = text and noctalia.json.decode(text) or nil
  if type(old) == "table" then
    s.vols = type(old.vols) == "table" and old.vols or {}
    s.on = type(old.on) == "table" and old.on or {}
    s.master = tonumber(old.master) or 70
    s.presets = type(old.presets) == "table" and old.presets or {}
    s.playing = old.wasPlaying == true and cfg("playOnStart", false) == true
  end
end
scan()
if s.playing then setTimer(cfg("timer", "0")) else noctalia.setUpdateInterval(30000) end
apply()
publish()
show()
