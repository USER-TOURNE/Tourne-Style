--!nonstrict
-- Tourne Desktop's own Power Menu, after JDKamalakar's DMS Fullscreen Power Menu.
-- Big round tiles in a row over the dimmed desktop. Arrows or Tab move, Enter or Space chooses, 1 to 6 pick
-- one directly, Esc or a click outside closes. Sign out, restart and shut down are armed by the first press when
-- "confirm" is on, done by the second.

local ALL = {
  { key = "lock", label = "Lock", glyph = "lock", action = "lock" },
  { key = "sleep", label = "Sleep", glyph = "moon", action = "sleep" },
  { key = "hibernate", label = "Hibernate", glyph = "zzz", action = "hibernate" },
  { key = "signout", label = "Sign out", glyph = "logout", action = "signout", risky = true },
  { key = "restart", label = "Restart", glyph = "refresh", action = "restart", risky = true },
  { key = "shutdown", label = "Shut down", glyph = "power", action = "shutdown", risky = true },
}

local focused = 1
local armed = nil   -- the key of a risky tile waiting for its second press
local armedAt = 0

local function cfg(k, d)
  local v = noctalia.getConfig(k)
  if v == nil then return d end
  return v
end

local function shown()
  local list = {}
  for _, t in ipairs(ALL) do
    if cfg(t.key, t.key ~= "hibernate") then table.insert(list, t) end
  end
  return list
end

local render

local function choose(t)
  if t.risky and cfg("confirm", true) and armed ~= t.key then
    armed, armedAt = t.key, noctalia.nowMs()
    tourne.uiSound("warning")
    render()
    return
  end
  armed = nil
  tourne.uiSound("press")
  panel.close()
  local ok, why = tourne.systemAction(t.action)
  if ok == false then noctalia.notifyError("Power menu", tostring(why or ("couldn't " .. t.label:lower()))) end
end

local function tile(t, i)
  local on = i == focused
  local isArmed = armed == t.key
  local ink = isArmed and "on_error" or on and "on_primary" or "on_surface"
  return ui.column({
    key = t.key, width = 120, gap = 12, align = "center", padding = 4,
    onClick = function() focused = i; choose(t) end,
    onHover = function(inside) if (inside == true or inside == "true") and focused ~= i then focused = i; render() end end,
  }, {
    ui.column({
      width = 112, height = 112, radius = 56, align = "center", justify = "center",
      fill = isArmed and "error" or on and "primary" or "surface_container_high",
      border = on and not isArmed and "primary" or nil, borderWidth = 2,
    }, {
      ui.glyph({ name = t.glyph, size = 44, color = ink }),
    }),
    ui.label({ text = isArmed and ("Again to " .. t.label:lower()) or t.label, fontSize = 15,
      fontWeight = on and "semibold" or "normal", color = isArmed and "error" or "on_surface", textAlign = "center" }),
  })
end

render = function()
  local list = shown()
  if focused > #list then focused = math.max(1, #list) end
  if armed and noctalia.nowMs() - armedAt > 5000 then armed = nil end
  local tiles = {}
  for i, t in ipairs(list) do table.insert(tiles, tile(t, i)) end
  panel.render(ui.column({ flexGrow = 1, gap = 28, padding = 28, align = "center", justify = "center" }, {
    ui.label({ text = noctalia.formatTime("%A, ") .. tostring(tonumber(noctalia.formatTime("%d"))) .. noctalia.formatTime(" %B  %H:%M"), fontSize = 18, color = "on_surface_variant" }),
    ui.row({ gap = 16, justify = "center" }, tiles),
    ui.label({ text = #list == 0 and "Every button is turned off in this plugin's settings"
      or "Arrows to move · Enter to choose · Esc to close", fontSize = 12, color = "on_surface_variant" }),
  }))
end

function onKey(chord, pressed)
  if not pressed then return end
  local list = shown()
  if chord == "Escape" then panel.close() return end
  if #list == 0 then return end
  if chord == "Right" or chord == "Down" or chord == "Tab" then
    focused = focused % #list + 1
  elseif chord == "Left" or chord == "Up" or chord == "Shift+Tab" or chord == "Backtab" then
    focused = (focused - 2) % #list + 1
  elseif chord == "Return" or chord == "Enter" or chord == "Space" then
    return choose(list[focused])
  elseif tonumber(chord) then
    local n = tonumber(chord)
    if list[n] then focused = n; return choose(list[n]) end
  else
    return
  end
  if armed and armed ~= list[focused].key then armed = nil end
  tourne.uiSound("tick")
  render()
end

function onOpen(_context)
  tourne.uiSound("whoosh")
  focused, armed = 1, nil
  panel.setWantsSecondTicks(true)
  render()
end

function update()
  render()
end
