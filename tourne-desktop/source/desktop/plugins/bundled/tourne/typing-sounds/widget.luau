--!nonstrict
-- Typing Sounds: a click for each key pressed (tourne.keyEvents: only the kind of press, never the key).
-- The sounds are made here the first time, as short WAV files in the plugin's own folder.

local on = false
local made = {}     -- sound names ready
local voice = 0     -- round robin, so fast typing overlaps instead of cutting off

local function cfg(key, fallback)
  local v = noctalia.getConfig(key)
  if v == nil or v == "" then return fallback end
  return v
end

local function dataDir() return noctalia.pluginDataDir() or "" end

-- A click: a short burst of noise through a decaying resonance; pitch and length by style and kind.
local function makeClick(path, style, kind, seed)
  math.randomseed(seed)
  local rate = 22050
  local len = style == "typewriter" and 2600 or style == "soft" and 1100 or 1500
  if kind == "space" or kind == "enter" then len = math.floor(len * 1.5) end
  local tone = (style == "typewriter" and 1900 or style == "soft" and 700 or 2600) * (0.92 + math.random() * 0.16)
  if kind == "space" then tone *= 0.55 elseif kind == "enter" then tone *= 0.7 elseif kind == "click" then tone *= 1.4 end
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
  local decay = style == "soft" and 0.0035 or 0.0022
  local second = style == "typewriter" and math.floor(len * 0.45) or -1  -- (the bar's thunk after the strike)
  local clack = style == "mechanical" and math.floor(len * 0.22) or -1  -- (a switch's click, then the keycap's clack)
  local smooth = 0  -- (soft: the noise muffled, as a membrane does)
  for i = 0, len - 1 do
    local env = math.exp(-i * decay * (rate / 22050) * 2.2)
    local noise = math.random() * 2 - 1
    local v
    if style == "soft" then
      smooth += (noise - smooth) * 0.12
      v = smooth * 1.6 * env + math.sin(i * 2 * math.pi * tone / rate) * 0.15 * env
    elseif style == "typewriter" then
      v = noise * 0.35 * env + math.sin(i * 2 * math.pi * tone / rate) * 0.55 * env + math.sin(i * 2 * math.pi * tone * 2.7 / rate) * 0.2 * env
    else
      v = noise * 0.7 * math.exp(-i * 0.02) + math.sin(i * 2 * math.pi * tone / rate) * 0.35 * env
    end
    if clack > 0 and i >= clack then
      local j = i - clack
      v += (math.random() * 2 - 1) * 0.5 * math.exp(-j * 0.012)
    end
    if second > 0 and i >= second then
      local j = i - second
      v += (math.random() * 2 - 1) * 0.35 * math.exp(-j * 0.004)
    end
    buffer.writei16(data, 44 + i * 2, math.clamp(math.floor(v * 0.85 * 32767), -32768, 32767))
  end
  noctalia.writeFile(path, buffer.tostring(data))
end

local function prepare()
  local style = cfg("style", "mechanical")
  noctalia.mkdirAll(dataDir())
  for _, kind in { "key", "space", "enter", "backspace", "click" } do
    for v = 1, 3 do
      local path = `{dataDir()}/{style}-{kind}-{v}-b.wav`  -- ("-b": the second recipe; the first one's files aren't reused)
      if not noctalia.fileExists(path) then makeClick(path, style, kind, v * 97 + #kind) end
      for slot = 1, 4 do
        local name = `{kind}{v}-{slot}`
        if made[name] ~= path then
          noctalia.sound.load(name, path)
          made[name] = path
        end
      end
    end
  end
end

local function show()
  barWidget.setGlyph(on and "keyboard" or "keyboard-off")
  barWidget.setColor(on and "primary" or "on_surface_variant")
  barWidget.setTooltip(on and "Typing sounds on: click to turn off" or "Typing sounds off: click to turn on")
end

function onKeyPress(kind)
  if not on then return end
  if kind == "click" and not cfg("clicks", false) then return end
  voice = voice % 4 + 1
  local name = `{kind}{math.random(1, 3)}-{voice}`
  noctalia.sound.volume(name, math.clamp(tonumber(cfg("volume", 45)) or 45, 0, 100))
  noctalia.sound.play(name)
end

-- A setting changed in the Studio: the sounds for the style now, at once (it used to wait for the next turn-on).
function onConfigChanged()
  if on then prepare() end
end

function onClick()
  on = not on
  if on then prepare() end
  tourne.keyEvents(on)
  noctalia.mkdirAll(dataDir())
  noctalia.writeFile(dataDir() .. "/on", on and "1" or "0")
  show()
end

function onRightClick() noctalia.openSettings() end

on = noctalia.fileExists(dataDir() .. "/on") and noctalia.readFile(dataDir() .. "/on") == "1"
if on then
  prepare()
  tourne.keyEvents(true)
end
show()
