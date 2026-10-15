--!nonstrict
-- Tourne Desktop's own Alienware Command Center, after psyreactor's DMS plugin.
-- Shows the service's snapshot ("awcc.snapshot") and sends it commands ("awcc.cmd").

local ICONS = { quiet = "volume-off", balanced = "gauge", cool = "snowflake", performance = "rocket",
  fullspeed = "propeller", gmode = "flame", battery = "bolt", manual = "adjustments" }
local seq = 0
local alienFan = false
local dragging = {}  -- fan id: the boost being dragged (shown until the service reports it)

-- Lighting, through T-Troll's alienfx-cli (AlienFX Tools; it talks to the lights over USB, no administrator
-- rights): its full path in the alienfxCli setting. The lights are read once from its "status" (device 0).
local COLORS = {
  { "Red", 255, 0, 0 }, { "Coral", 255, 90, 70 }, { "Orange", 255, 120, 0 }, { "Yellow", 255, 220, 0 },
  { "Green", 0, 255, 60 }, { "Teal", 0, 220, 180 }, { "Blue", 0, 90, 255 }, { "Purple", 160, 0, 255 },
  { "Pink", 255, 0, 160 }, { "White", 255, 255, 255 },
}
local EFFECTS = { { "Static", "color" }, { "Breathe", "breath" }, { "Pulse", "pulse" }, { "Spectrum", "spectrum" }, { "Rainbow", "rainbow" } }
local lights = nil   -- light ids of device 0, once read
local light = { color = 1, effect = 1, brightness = 100 }
local lightNote = ""
local render

local function cliPath()
  local p = noctalia.getConfig("alienfxCli")
  return type(p) == "string" and p ~= "" and p or nil
end

local function readLights()
  local cli = cliPath()
  if not cli then return end
  noctalia.runAsync({ cli, "status" }, function(r)
    local ids, dev = {}, nil
    for line in string.gmatch(r.stdout or "", "[^\r\n]+") do
      local d = string.match(line, "^Device #(%d+)")
      if d then dev = tonumber(d) end
      local id = dev == 0 and string.match(line, "Light ID#(%d+)")
      if id then table.insert(ids, tonumber(id)) end
    end
    lights = ids
    if #ids == 0 then lightNote = "alienfx-cli found no lights (run AlienFX's probe once)." end
    render()
  end, 20000)
end

local function applyLight()
  local cli = cliPath()
  if not cli or not lights or #lights == 0 then return end
  local c = COLORS[light.color]
  local effect = EFFECTS[light.effect][2]
  local argv = { cli, "lowlevel", "setdim=0," .. tostring(math.floor(light.brightness * 255 / 100)) }
  for _, id in ipairs(lights) do
    table.insert(argv, string.format("setaction=0,%d,%s,%d,%d,%d", id, effect, c[2], c[3], c[4]))
  end
  lightNote = "Setting the lights..."
  render()
  noctalia.runAsync(argv, function(r)
    lightNote = r.exitCode == 0 and ("Lights: " .. EFFECTS[light.effect][1] .. ", " .. c[1]) or ("alienfx-cli said: " .. tostring(r.stderr or r.stdout or ""))
    tourne.uiSound("switch")
    noctalia.writeFile((noctalia.pluginDataDir() or "") .. "\\light.json", noctalia.json.encode(light))
    render()
  end, 30000)
end

local function lightingSection(add)
  add(ui.label({ text = "Lighting", fontSize = 12, fontWeight = "semibold", color = "on_surface_variant" }))
  if not cliPath() then
    add(ui.label({ text = "Set alienfx-cli.exe's full path (from T-Troll's AlienFX Tools) in this plugin's settings to change the PC's lights here.",
      fontSize = 11, color = "on_surface_variant", maxWidth = 360 }))
    return
  end
  local swatches = {}
  for i, c in ipairs(COLORS) do
    local on = i == light.color
    table.insert(swatches, ui.column({
      key = "color-" .. i, width = 26, height = 26, radius = 13,
      fill = string.format("#%02x%02x%02x", c[2], c[3], c[4]), border = on and "on_surface" or "outline_variant", borderWidth = on and 2 or 1,
      onClick = function()
        light.color = i
        applyLight()
      end,
    }, {}))
  end
  add(ui.row({ gap = 6 }, swatches))
  local effects = {}
  for i, e in ipairs(EFFECTS) do
    table.insert(effects, ui.button({ text = e[1], variant = i == light.effect and "primary" or "ghost",
      onClick = function()
        light.effect = i
        applyLight()
      end }))
  end
  add(ui.row({ gap = 4 }, effects))
  add(ui.row({ gap = 8, align = "center" }, {
    ui.label({ text = "Brightness", width = 70, color = "on_surface" }),
    ui.slider({ min = 10, max = 100, step = 10, value = light.brightness, flexGrow = 1, controlSize = "sm",
      onChange = function(v)
        light.brightness = math.floor(tonumber(v) or 100)
        applyLight()
      end }),
  }))
  if lightNote ~= "" then add(ui.label({ text = lightNote, fontSize = 11, color = "on_surface_variant", maxWidth = 360 })) end
end

local function send(action, arg, arg2)
  seq += 1
  noctalia.state.set("awcc.cmd", noctalia.json.encode({ n = seq, action = action, arg = arg, arg2 = arg2 }))
end

local function snapshot()
  local raw = noctalia.state.get("awcc.snapshot")
  local s = raw and noctalia.json.decode(raw) or nil
  return type(s) == "table" and s or { phase = "loading" }
end

local function heading(text)
  return ui.label({ text = text, fontSize = 12, fontWeight = "semibold", color = "on_surface_variant" })
end

render = function()
  local s = snapshot()
  local kids = {
    ui.row({ gap = 8, align = "center" }, {
      ui.glyph({ name = "alien", size = 20, color = "primary" }),
      ui.column({ gap = 0, flexGrow = 1 }, {
        ui.label({ text = "Alienware Command Center", fontSize = 15, fontWeight = "semibold", color = "on_surface" }),
        ui.label({ text = (s.model ~= nil and s.model ~= "") and s.model or "Alienware", fontSize = 11, color = "on_surface_variant" }),
      }),
      ui.button({ glyph = "refresh", variant = "ghost", onClick = function() send("refresh") end }),
    }),
  }
  local function add(n) table.insert(kids, n) end

  if s.phase == "loading" then
    add(ui.label({ text = "Asking the PC what it has...", color = "on_surface_variant" }))
  elseif s.phase == "none" then
    add(ui.label({ text = "This PC has no Alienware firmware interface, so there's nothing to control here.",
      color = "on_surface_variant", maxWidth = 360 }))
  elseif s.phase == "needsAdmin" then
    add(ui.label({ text = "Windows only lets programs with administrator rights talk to the Alienware firmware. "
      .. "Set up the helper once (one administrator prompt): after that, changes never ask again.",
      color = "on_surface", maxWidth = 360 }))
    add(ui.button({ text = "Set up the helper (administrator, once)", glyph = "shield-check", variant = "primary",
      onClick = function() send("install") end }))
    add(ui.button({ text = "Just this once (a prompt for each change)", glyph = "shield", variant = "ghost",
      onClick = function() send("elevate") end }))
  elseif s.phase == "error" then
    add(ui.label({ text = "Something went wrong: " .. tostring(s.error or ""), color = "error", maxWidth = 360 }))
  else
    -- Modes
    local current = type(s.current) == "table" and s.current.key or nil
    if s.gmode == true then current = "gmode" end
    add(heading("Thermal mode"))
    local row, rows = {}, {}
    for _, m in ipairs(s.modes or {}) do
      if m.key ~= "manual" then
        local on = m.key == current
        table.insert(row, ui.column({
          key = "mode-" .. m.key, width = 110, height = 64, radius = 12, gap = 4, align = "center", justify = "center",
          fill = on and "primary/0.22" or "surface_container_high", border = on and "primary" or nil, borderWidth = 1,
          onClick = function() if not on then send("mode", m.key) end end,
        }, {
          ui.glyph({ name = ICONS[m.key] or "gauge", size = 20, color = on and "primary" or "on_surface" }),
          ui.label({ text = m.name, fontSize = 12, color = "on_surface", maxLines = 1 }),
        }))
        if #row == 3 then
          table.insert(rows, ui.row({ gap = 6 }, row))
          row = {}
        end
      end
    end
    if #row > 0 then table.insert(rows, ui.row({ gap = 6 }, row)) end
    add(ui.column({ gap = 6 }, rows))
    -- Fans
    if s.fans and #s.fans > 0 then
      add(heading("Fan boost (on top of what the mode runs them at)"))
      for _, f in ipairs(s.fans) do
        local pct = dragging[f.id] or math.floor((tonumber(f.boost) or 0) * 100 / 255 + 0.5)
        add(ui.row({ gap = 8, align = "center" }, {
          ui.label({ text = f.name, width = 50, color = "on_surface" }),
          ui.slider({ min = 0, max = 100, step = 5, value = pct, flexGrow = 1, controlSize = "sm",
            onChange = function(v)
              dragging[f.id] = math.floor(tonumber(v) or 0)
              send("boost", tostring(f.id), tostring(dragging[f.id]))
            end }),
          ui.label({ text = tostring(pct) .. "%", width = 40, color = "on_surface_variant", textAlign = "end" }),
        }))
      end
    end
    if s.route == "elevate" then
      add(ui.button({ text = "Stop the administrator prompts (set up the helper)", glyph = "shield-check", variant = "ghost",
        onClick = function() send("install") end }))
    end
  end
  if alienFan then
    add(ui.label({ text = "AlienFan Control is running too and may change the mode or fans back.",
      fontSize = 11, color = "on_surface_variant", maxWidth = 360 }))
  end
  local note = s.busy and "Working..." or (s.note or "")
  if note ~= "" then add(ui.label({ text = note, fontSize = 11, color = "on_surface_variant", maxWidth = 360 })) end
  lightingSection(add)
  local tools = {
    ui.button({ text = "Windows lighting", glyph = "bulb", variant = "ghost",
      onClick = function() noctalia.openPath("ms-settings:personalization-lighting") end }),
  }
  if s.taskInstalled then
    table.insert(tools, ui.button({ text = "Remove the helper", glyph = "trash", variant = "ghost",
      onClick = function() send("uninstall") end }))
  end
  add(ui.row({ gap = 6 }, tools))
  panel.render(ui.column({ gap = 10, padding = 16 }, kids))
end

function onOpen(_context)
  dragging = {}
  if lights == nil then readLights() end
  noctalia.processMatches(function(found)
    alienFan = found == true
    render()
  end, "alienfan-gui")
  send("peek")
  render()
end

do  -- the last lighting chosen, shown as it was
  local saved = noctalia.readFile((noctalia.pluginDataDir() or "") .. "\\light.json")
  local t = saved and noctalia.json.decode(saved)
  if type(t) == "table" then
    light.color = math.clamp(tonumber(t.color) or 1, 1, #COLORS)
    light.effect = math.clamp(tonumber(t.effect) or 1, 1, #EFFECTS)
    light.brightness = math.clamp(tonumber(t.brightness) or 100, 10, 100)
  end
end

function onConfigChanged()
  lights = nil
  readLights()
end

noctalia.state.watch("awcc.snapshot", function()
  local s = snapshot()
  -- a boost the service now reports drops the dragged value
  for _, f in ipairs(s.fans or {}) do
    local pct = math.floor((tonumber(f.boost) or 0) * 100 / 255 + 0.5)
    if dragging[f.id] and math.abs(dragging[f.id] - pct) <= 3 then dragging[f.id] = nil end
  end
  render()
end)
