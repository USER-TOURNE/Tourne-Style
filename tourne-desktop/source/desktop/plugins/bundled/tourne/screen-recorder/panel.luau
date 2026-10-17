--!nonstrict
-- Screen Recorder's panel: start / stop, the choices in short, and ffmpeg's list of microphones.

local devices = nil
local render

local function cfg(key, fallback)
  local v = noctalia.getConfig(key)
  if v == nil or v == "" then return fallback end
  return v
end

local function listDevices()
  devices = { "Looking…" }
  render()
  noctalia.runAsync({ cfg("ffmpeg", "ffmpeg"), "-hide_banner", "-list_devices", "true", "-f", "dshow", "-i", "dummy" }, function(r)
    devices = {}
    for line in string.gmatch(r.stderr .. "\n" .. r.stdout, "[^\n]+") do
      local name, kind = string.match(line, '"([^"]+)"%s*%((%a+)%)')
      if name and kind == "audio" then table.insert(devices, name) end
    end
    if #devices == 0 then devices = { "None found (is ffmpeg installed?)" } end
    render()
  end, 15000)
end

local function pcSound()
  local v = cfg("systemSound", "none")
  if v == "none" then return "off" end
  if v == "default" then return "default output" end
  return "chosen output"
end

render = function()
  local s = noctalia.state.get("recorder.state") or {}
  local recording = s.recording == true
  local rows = {
    ui.row({ gap = 8, align = "center" }, {
      ui.glyph({ name = recording and "player-record" or "video", size = 20, color = recording and "error" or "primary" }),
      ui.label({ text = recording and "Recording" or (s.countdown and "Starting…" or "Ready"), flexGrow = 1, color = "on_surface", fontWeight = "semibold" }),
    }),
    ui.label({ text = string.format("%s fps · %s · %s · %s · %s", cfg("fps", "60"), cfg("quality", "high"), cfg("resolution", "native"),
      cfg("encoder", "libx264"), string.upper(cfg("container", "mkv"))), fontSize = 12, color = "on_surface_variant" }),
    ui.label({ text = "Screen: " .. cfg("monitor", "main") .. " · PC sound: " .. pcSound() .. " · Mic: " .. cfg("audioDevice", "none"),
      fontSize = 12, color = "on_surface_variant", maxLines = 2 }),
    ui.button({ text = recording and "Stop" or "Record", glyph = recording and "player-stop" or "player-record",
      variant = recording and "destructive" or "primary", onClick = function()
        noctalia.state.set("recorder.command", recording and "stop" or "start")
      end }),
  }
  if s.file then
    table.insert(rows, ui.label({ text = (recording and "To " or "Last: ") .. s.file, fontSize = 12, color = "on_surface_variant", maxLines = 2 }))
    if not recording then
      table.insert(rows, ui.button({ text = "Open the folder", glyph = "folder", variant = "ghost", onClick = function()
        noctalia.openPath(noctalia.expandPath(cfg("folder", "~/Videos")))
      end }))
    end
  end
  if s.error then table.insert(rows, ui.label({ text = s.error, fontSize = 11, color = "error", maxLines = 4 })) end
  table.insert(rows, ui.separator({ spacing = 4 }))
  table.insert(rows, ui.button({ text = "List microphones", glyph = "microphone", variant = "ghost", onClick = listDevices }))
  if devices then
    for i, d in devices do
      table.insert(rows, ui.row({ key = "dev" .. i, gap = 6, align = "center" }, {
        ui.label({ text = d, flexGrow = 1, maxLines = 1, fontSize = 12, color = "on_surface" }),
        ui.button({ glyph = "copy", variant = "ghost", controlSize = "sm", tooltip = "Copy the name (paste it in Microphone)", onClick = function()
          noctalia.copyToClipboard(d)
        end }),
      }))
    end
  end
  panel.render(ui.column({ flexGrow = 1, gap = 8, padding = 12 }, rows))
end

function onOpen(_context)
  panel.setWantsSecondTicks(true)
  render()
end

function update()
  render()
end

noctalia.state.watch("recorder.state", function()
  render()
end)
