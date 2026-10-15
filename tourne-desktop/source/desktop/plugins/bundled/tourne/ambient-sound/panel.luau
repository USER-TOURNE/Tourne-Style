--!nonstrict
-- Ambient Sounds' panel: play or pause, the overall volume, a sleep timer, saved mixes, and a row per
-- sound (on or off, its own volume). The bar widget plays them.

local TIMERS = { 0, 15, 30, 60, 90, 120 }
local TIMER_NAMES = { "No timer", "15 min", "30 min", "1 hour", "1.5 hours", "2 hours" }
local presetName = ""
local picked = 0
local render

local function send(c)
  noctalia.state.set("ambient.command", c)
end

local function sliderRow(key, text, value, onChange)
  return ui.row({ key = key, gap = 8, align = "center" }, {
    ui.label({ text = text, width = 70, color = "on_surface_variant" }),
    ui.slider({ min = 0, max = 100, step = 1, value = value, flexGrow = 1, controlSize = "sm", onChange = onChange }),
    ui.label({ text = tostring(math.floor(value)) .. "%", width = 40, textAlign = "right", fontSize = 11 }),
  })
end

render = function()
  local a = noctalia.state.get("ambient") or { sounds = {}, master = 70, presets = {} }
  local rows = {}
  for i, snd in a.sounds or {} do
    table.insert(rows, ui.column({ key = "s" .. tostring(i), gap = 2, padding = 8, radius = 8,
      fill = snd.on and "primary/0.12" or "surface_container" }, {
      ui.row({ gap = 8, align = "center" }, {
        ui.toggle({ checked = snd.on, onChange = function() send({ op = "toggle", path = snd.path }) end }),
        ui.label({ text = snd.name, flexGrow = 1, maxLines = 1 }),
        ui.slider({ min = 0, max = 100, step = 1, value = snd.vol, width = 150, controlSize = "sm", enabled = snd.on,
          onChange = function(v) send({ op = "volume", path = snd.path, value = tonumber(v) }) end }),
      }),
    }))
  end
  if #rows == 0 then
    table.insert(rows, ui.label({ text = "No sounds yet. Put MP3 or WAV files in your sounds folder, then Look again.",
      color = "on_surface_variant", textAlign = "center", maxLines = 3 }))
  end
  local timerText = "Sleep timer"
  if a.timerEnds and a.playing then
    local m = math.max(0, math.ceil((a.timerEnds - noctalia.nowMs()) / 60000))
    timerText = string.format("Stops in %d min", m)
  end
  local presets = a.presets or {}
  if picked >= #presets then picked = 0 end
  panel.render(ui.column({ flexGrow = 1, gap = 10, padding = 12 }, {
    ui.row({ gap = 8, align = "center" }, {
      ui.button({ text = a.playing and "Pause" or "Play", glyph = a.playing and "player-pause" or "player-play",
        variant = "primary", controlSize = "sm", onClick = function() send({ op = "play" }) end }),
      ui.button({ glyph = a.muted and "volume-off" or "volume", tooltip = a.muted and "Unmute" or "Mute", controlSize = "sm",
        variant = a.muted and "primary" or "ghost", onClick = function() send({ op = "mute" }) end }),
      ui.spacer({ flexGrow = 1 }),
      ui.label({ text = timerText, fontSize = 11, color = "on_surface_variant" }),
      ui.select({ options = TIMER_NAMES, selectedIndex = 0, controlSize = "sm", onChange = function(i)
        send({ op = "timer", minutes = TIMERS[(tonumber(i) or 0) + 1] })
      end }),
    }),
    sliderRow("master", "Volume", a.master or 70, function(v) send({ op = "master", value = tonumber(v) }) end),
    ui.row({ gap = 6, align = "center" }, {
      ui.select({ options = #presets > 0 and presets or { "No saved mixes" }, selectedIndex = picked, controlSize = "sm",
        enabled = #presets > 0, flexGrow = 1, onChange = function(i) picked = tonumber(i) or 0 end }),
      ui.button({ text = "Play mix", controlSize = "sm", enabled = #presets > 0,
        onClick = function() send({ op = "load", name = presets[picked + 1] }) end }),
      ui.button({ glyph = "trash", tooltip = "Delete this mix", controlSize = "sm", variant = "ghost", enabled = #presets > 0,
        onClick = function() send({ op = "delete", name = presets[picked + 1] }) end }),
    }),
    ui.row({ gap = 6, align = "center" }, {
      ui.input({ key = "name", value = presetName, placeholder = "Name this mix", controlSize = "sm", flexGrow = 1,
        onChange = function(v) presetName = v end }),
      ui.button({ text = "Save mix", glyph = "device-floppy", controlSize = "sm", onClick = function()
        presetName = noctalia.string.trim(presetName)
        if presetName == "" then return end
        send({ op = "save", name = presetName })
        presetName = ""
      end }),
    }),
    ui.scroll({ flexGrow = 1 }, { ui.column({ gap = 6 }, rows) }),
    ui.row({ gap = 6 }, {
      ui.button({ text = "Your sounds folder", glyph = "folder", controlSize = "sm", variant = "ghost",
        onClick = function() noctalia.openPath(a.folder or "~/Music/Ambient") end }),
      ui.button({ text = "Look again", glyph = "refresh", controlSize = "sm", variant = "ghost",
        onClick = function() send({ op = "rescan" }) end }),
      ui.spacer({ flexGrow = 1 }),
      ui.button({ text = "Find sounds", glyph = "world", controlSize = "sm", variant = "ghost",
        onClick = function() noctalia.openPath("https://pixabay.com/sound-effects/search/ambience/") end }),
    }),
  }))
end

function onOpen(_context)
  panel.setWantsSecondTicks(true)
  render()
end

function update()
  render()
end

noctalia.state.watch("ambient", function()
  render()
end)
