--!nonstrict
-- Tourne Desktop's GMK87 panel. Everything goes through gmk87.ps1 (one PowerShell run each, one JSON line back):
-- the backlight over VIA (cable or dongle), and with the cable in, the screen, the clock and the settings.

local PS1 = noctalia.pluginDir() .. "\\gmk87.ps1"
local EFFECTS = { "Off", "Wave", "Colour cloud", "Vortex", "Mixed colours", "Breathe", "Light", "Fade out", "Stone",
  "Laser", "Starry", "Flowers", "Traverse", "Wave bar", "Meteor", "Rain", "Scan", "Trigger colour", "Centre spread" }
local COLORS = { { "Red", 0, 255 }, { "Orange", 21, 255 }, { "Yellow", 43, 255 }, { "Green", 85, 255 }, { "Teal", 128, 255 },
  { "Blue", 170, 255 }, { "Purple", 191, 255 }, { "Pink", 234, 255 }, { "White", 0, 0 } }
local SWATCH = { "#ff2020", "#ff8a00", "#ffd800", "#20e040", "#00d8c0", "#2060ff", "#9a30ff", "#ff30b0", "#ffffff" }
local LED_COLORS = { "Red", "Orange", "Yellow", "Green", "Teal", "Blue", "Purple", "White", "Off" }

local light = nil    -- {brightness, effect, speed, hue, sat} once read
local device = nil   -- the cable's settings once read, or {wireless = true} / {none = true}
local slots = {}     -- [1], [2]: picture paths dropped for the screen
local delay = 100
local busy = ""
local note = ""
local render

local function run(args, done)
  local argv = { "powershell", "-NoProfile", "-NonInteractive", "-ExecutionPolicy", "Bypass", "-File", PS1 }
  for _, a in ipairs(args) do table.insert(argv, a) end
  noctalia.runAsync(argv, function(r)
    local res = nil
    for line in string.gmatch(r.stdout or "", "[^\r\n]+") do
      if string.sub(line, 1, 1) == "{" then res = noctalia.json.decode(line) end
    end
    done(type(res) == "table" and res or { ok = false, error = "no answer from the helper" })
  end, 180000)
end

-- One change at a time to the keyboard; changes made meanwhile (a slider dragged) are merged and sent next.
local inFlight = { light = false, set = false }
local waiting = { light = nil, set = nil }

local function change(op, values)
  if inFlight[op] then
    waiting[op] = waiting[op] and (waiting[op] .. "," .. values) or values
    return
  end
  inFlight[op] = true
  busy = op == "light" and "Setting the lights..." or "Saving to the keyboard..."
  render()
  run({ "-Op", op, "-Values", values }, function(res)
    inFlight[op] = false
    busy = ""
    if res.ok then
      if op == "light" then light = res else device = res end
      note = ""
    else
      note = tostring(res.error or "")
    end
    if op == "light" then tourne.uiSound("switch") end
    local next = waiting[op]
    waiting[op] = nil
    if next then change(op, next) else render() end
  end)
end

local function setLight(values) change("light", values) end
local function setDevice(values) change("set", values) end

local function heading(text)
  return ui.label({ text = text, fontSize = 12, fontWeight = "semibold", color = "on_surface_variant" })
end

local function sliderRow(label, value, max, onChange)
  return ui.row({ gap = 8, align = "center" }, {
    ui.label({ text = label, width = 80, color = "on_surface" }),
    ui.slider({ min = 0, max = max, step = 1, value = value, flexGrow = 1, controlSize = "sm", onChange = onChange }),
    ui.label({ text = tostring(value), width = 24, color = "on_surface_variant", textAlign = "end" }),
  })
end

local function lightingSection(add)
  add(heading("Backlight"))
  if not light then
    add(ui.label({ text = "Reading the keyboard...", color = "on_surface_variant" }))
    return
  end
  add(ui.row({ gap = 8, align = "center" }, {
    ui.label({ text = "Effect", width = 80, color = "on_surface" }),
    ui.select({ options = EFFECTS, selectedIndex = math.clamp(light.effect or 0, 0, #EFFECTS - 1), controlSize = "sm", flexGrow = 1,
      onChange = function(i) setLight("effect=" .. tostring(math.max(0, (tonumber(i) or 1) - 1))) end }),
  }))
  add(sliderRow("Brightness", light.brightness or 0, 9, function(v) setLight("brightness=" .. tostring(math.floor(tonumber(v) or 0))) end))
  add(sliderRow("Speed", light.speed or 0, 4, function(v) setLight("speed=" .. tostring(math.floor(tonumber(v) or 0))) end))
  local swatches = {}
  for i, c in ipairs(COLORS) do
    local on = (light.sat == 0 and c[3] == 0) or (light.sat ~= 0 and c[3] ~= 0 and math.abs((light.hue or 0) - c[2]) < 8)
    table.insert(swatches, ui.column({
      key = "c" .. i, width = 26, height = 26, radius = 13, fill = SWATCH[i],
      border = on and "on_surface" or "outline_variant", borderWidth = on and 2 or 1, tooltip = c[1],
      onClick = function() setLight(string.format("color=%d/%d", c[2], c[3])) end,
    }, {}))
  end
  add(ui.row({ gap = 6 }, swatches))
end

local function screenSection(add)
  add(heading("Screen"))
  if not device then
    add(ui.label({ text = "Reading the keyboard...", color = "on_surface_variant" }))
    return
  end
  if device.none then
    add(ui.label({ text = "No GMK87 found.", color = "on_surface_variant" }))
    return
  end
  if device.wireless then
    add(ui.label({ text = "The screen, its clock and the keyboard's own settings need its USB cable plugged in (the 2.4 GHz dongle doesn't pass them on). The backlight above works either way.",
      color = "on_surface_variant", maxWidth = 380 }))
    add(ui.button({ text = "Check again", glyph = "refresh", variant = "ghost", onClick = function()
      device = nil
      render()
      run({ "-Op", "info" }, function(res) device = res.ok and res or res; render() end)
    end }))
    return
  end
  add(ui.row({ gap = 6 }, {
    ui.button({ text = "Clock", glyph = "clock", variant = device.screen == 0 and "primary" or "ghost", onClick = function() setDevice("screen=0") end }),
    ui.button({ text = "Picture 1", glyph = "photo", variant = device.screen == 1 and "primary" or "ghost", onClick = function() setDevice("screen=1") end }),
    ui.button({ text = "Picture 2", glyph = "photo", variant = device.screen == 2 and "primary" or "ghost", onClick = function() setDevice("screen=2") end }),
  }))
  for slot = 1, 2 do
    local path = slots[slot]
    add(ui.column({
      key = "drop" .. slot, gap = 4, padding = 10, radius = 10, fill = "surface_container_high", border = "outline_variant", borderWidth = 1,
      onDrop = function(first) slots[slot] = first; render() end,
    }, {
      ui.label({ text = path and ("Picture " .. slot .. ": " .. (string.match(path, "[^\\/]+$") or path)) or
        ("Drop a picture or GIF here for picture " .. slot .. " (240 x 135; GIFs keep their frames)"), color = "on_surface", maxWidth = 360 }),
    }))
  end
  add(sliderRow("Frame time", math.floor(delay / 10), 100, function(v) delay = math.max(60, math.floor(tonumber(v) or 10) * 10); render() end))
  add(ui.row({ gap = 6 }, {
    ui.button({ text = "Send to the keyboard", glyph = "upload", variant = "primary", enabled = slots[1] ~= nil or slots[2] ~= nil,
      onClick = function()
        busy = "Sending the pictures (this takes a little while)..."
        render()
        local args = { "-Op", "picture", "-Delay", tostring(delay) }
        if slots[1] then table.insert(args, "-Slot1"); table.insert(args, slots[1]) end
        if slots[2] then table.insert(args, "-Slot2"); table.insert(args, slots[2]) end
        run(args, function(res)
          busy = ""
          if res.ok then device = res; note = "Sent " .. tostring(res.uploaded) .. " frames." else note = tostring(res.error or "") end
          tourne.uiSound(res.ok and "success" or "error")
          render()
        end)
      end }),
    ui.button({ text = "Set its clock", glyph = "clock", variant = "ghost", onClick = function()
      busy = "Setting the clock..."
      render()
      run({ "-Op", "time" }, function(res)
        busy = ""
        note = res.ok and "Clock set." or tostring(res.error or "")
        render()
      end)
    end }),
  }))
  add(heading("Keys"))
  add(ui.row({ gap = 8, align = "center" }, {
    ui.label({ text = "Key light", width = 80, color = "on_surface" }),
    ui.select({ options = { "Mode 1", "Mode 2", "Mode 3", "Mode 4", "Mode 5" }, selectedIndex = math.clamp(device.ledMode or 0, 0, 4),
      controlSize = "sm", flexGrow = 1, onChange = function(i) setDevice("ledMode=" .. tostring(math.max(0, (tonumber(i) or 1) - 1))) end }),
  }))
  add(ui.row({ gap = 8, align = "center" }, {
    ui.label({ text = "Key colour", width = 80, color = "on_surface" }),
    ui.select({ options = LED_COLORS, selectedIndex = math.clamp(device.ledColor or 0, 0, 8), controlSize = "sm", flexGrow = 1,
      onChange = function(i) setDevice("ledColor=" .. tostring(math.max(0, (tonumber(i) or 1) - 1))) end }),
  }))
  add(sliderRow("Saturation", device.saturation or 0, 9, function(v) setDevice("saturation=" .. tostring(math.floor(tonumber(v) or 0))) end))
  add(ui.row({ gap = 8, align = "center" }, {
    ui.label({ text = "Windows key lock", flexGrow = 1, color = "on_surface" }),
    ui.toggle({ checked = (device.winlock or 0) ~= 0, onChange = function(v) setDevice("winlock=" .. ((v == true or v == "true") and "1" or "0")) end }),
  }))
end

render = function()
  local kids = {
    ui.row({ gap = 8, align = "center" }, {
      ui.glyph({ name = "keyboard", size = 20, color = "primary" }),
      ui.label({ text = "GMK87", fontSize = 15, fontWeight = "semibold", color = "on_surface", flexGrow = 1 }),
      ui.label({ text = light and (light.device == "dongle" and "2.4 GHz" or "Cable") or "", fontSize = 11, color = "on_surface_variant" }),
    }),
  }
  local function add(n) table.insert(kids, n) end
  lightingSection(add)
  screenSection(add)
  local line = busy ~= "" and busy or note
  if line ~= "" then add(ui.label({ text = line, fontSize = 11, color = "on_surface_variant", maxWidth = 380 })) end
  panel.render(ui.column({ gap = 10, padding = 16 }, kids))
end

function onOpen(_context)
  note = ""
  render()
  run({ "-Op", "lighting" }, function(res)
    if res.ok then light = res else note = tostring(res.error or "") end
    render()
    run({ "-Op", "info" }, function(info)
      device = info
      render()
    end)
  end)
end
