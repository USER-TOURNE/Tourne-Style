--!nonstrict
-- Power Modes on the bar: the mode's name; click for the panel, scroll to step through modes.
--
-- Idea from psyreactor's Alienware Command Center plugin (DankMaterialShell, no licence given, so no
-- code of it is used). That one drives the awcc CLI on Alienware laptops (thermal modes, fans,
-- keyboard light, turbo). This one drives what every Windows PC has: the power mode (Best power
-- efficiency, Balanced, Best performance), the power plan and processor turbo. Fans and lighting are
-- vendor-specific and left out.

local NAMES = { efficiency = "Efficiency", betterBattery = "Battery", balanced = "Balanced", performance = "Performance" }
local ORDER = { "efficiency", "balanced", "performance" }

local function show()
  local mode = tourne.powerMode()
  barWidget.setGlyph(mode == "performance" and "rocket" or mode == "efficiency" and "leaf" or "bolt")
  barWidget.setText(noctalia.getConfig("showText") and (NAMES[mode] or "") or "")
  barWidget.setTooltip("Power mode: " .. (NAMES[mode] or "unknown"))
end

function update()
  show()
end

function onClick()
  noctalia.togglePanel("tourne/power-modes:panel")
end

function onScroll(delta)
  local mode = tourne.powerMode() or "balanced"
  local at = 2
  for i, m in ORDER do
    if m == mode then at = i end
  end
  at = math.clamp(at + ((tonumber(delta) or 0) > 0 and 1 or -1), 1, #ORDER)
  local ok, why = tourne.setPowerMode(ORDER[at])
  if ok == false then noctalia.notifyError("Power mode", tostring(why)) end
  show()
end

noctalia.setUpdateInterval((noctalia.getConfig("refreshInterval") or 10) * 1000)
show()
