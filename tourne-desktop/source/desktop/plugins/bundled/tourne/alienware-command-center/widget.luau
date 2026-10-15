--!nonstrict
-- Tourne Desktop's own Alienware Command Center, after psyreactor's DMS plugin.
-- The tray icon: the current thermal mode in its tooltip (and beside it, if set). Click: the panel.

local ICONS = { quiet = "volume-off", balanced = "gauge", cool = "snowflake", performance = "rocket",
  fullspeed = "propeller", gmode = "flame", battery = "bolt", manual = "adjustments" }

local function show(raw)
  local snap = raw and noctalia.json.decode(raw) or nil
  local cur = type(snap) == "table" and snap.current or nil
  local key = type(cur) == "table" and cur.key or nil
  local gmode = type(snap) == "table" and snap.gmode == true
  barWidget.setGlyph(gmode and "flame" or (key and ICONS[key]) or "alien")
  barWidget.setColor(gmode and "primary" or nil)
  local name = gmode and "G-Mode" or (type(cur) == "table" and cur.name) or nil
  barWidget.setText(noctalia.getConfig("showMode") == true and name or "")
  local line = "Alienware Command Center"
  if type(snap) == "table" then
    if snap.phase == "none" then line ..= "\nNot an Alienware PC"
    elseif snap.phase == "needsAdmin" then line ..= "\nClick to set up (Windows wants administrator rights)"
    elseif name then line ..= "\nMode: " .. name end
  end
  barWidget.setTooltip(line)
end

function onClick() noctalia.togglePanel("tourne/alienware-command-center:panel") end

function onConfigChanged() show(noctalia.state.get("awcc.snapshot")) end

noctalia.state.watch("awcc.snapshot", show)
show(noctalia.state.get("awcc.snapshot"))
