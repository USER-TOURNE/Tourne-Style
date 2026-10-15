--!nonstrict
-- Tourne Desktop's own Caffeine, after the DMS Caffeine plugins.
-- The cup in the tray holds the request: while on, tourne.keepAwake keeps the PC (and the screen) awake; it ends
-- by itself when its time is up, or with the plugin. Click: on or off. Right-click: how long (the panel).
-- state "awake" = { until = ms it ends, or 0 for no end } while on, false while off. update() runs only while on.

local function cfg(k, d)
  local v = noctalia.getConfig(k)
  if v == nil then return d end
  return v
end

local function left(s)
  if not s or s["until"] == 0 then return nil end
  return math.max(0, s["until"] - noctalia.nowMs())
end

local function fmt(ms)
  local m = math.ceil(ms / 60000)
  if m >= 60 then return string.format("%dh %02dm", m // 60, m % 60) end
  return string.format("%dm", m)
end

local function show(s)
  if s then
    local l = left(s)
    barWidget.setGlyph("coffee")
    barWidget.setColor("primary")
    barWidget.setTooltip("Caffeine: awake " .. (l and ("for " .. fmt(l) .. " more") or "until you stop it")
      .. "\nClick: stop · Right-click: how long")
  else
    barWidget.setGlyph("coffee-off")
    barWidget.setColor("on_surface_variant")
    barWidget.setTooltip("Caffeine: off\nClick: keep awake · Right-click: how long")
  end
end

local function apply(s)
  if s then
    tourne.keepAwake(cfg("screen", true) and "display" or "system")
    noctalia.setUpdateInterval(left(s) and 15000 or 0)
  else
    tourne.keepAwake("off")
    noctalia.setUpdateInterval(0)
  end
  show(s)
end

local function set(s)
  tourne.uiSound(s and "on" or "off")  -- (heard only with Studio's UI sounds on)
  noctalia.state.set("awake", s)  -- (the watcher below applies it, here and from the panel)
end

local function startFor(minutes)
  set({ ["until"] = minutes > 0 and noctalia.nowMs() + minutes * 60000 or 0 })
end

function update()
  local s = noctalia.state.get("awake")
  if s and left(s) == 0 then
    set(false)
    if cfg("notify", true) then noctalia.notify("Caffeine", "Time's up: the PC may sleep again") end
  else
    show(s)
  end
end

function onClick()
  if noctalia.state.get("awake") then set(false) else startFor(cfg("minutes", 0)) end
end

function onRightClick()
  noctalia.togglePanel("tourne/caffeine:panel")
end

-- TourneDesktop.exe --msg plugin tourne/caffeine:tray all <toggle | on [minutes] | off>
function onIpc(event, payload)
  if event == "toggle" then onClick()
  elseif event == "on" then startFor(tonumber(payload) or cfg("minutes", 0))
  elseif event == "off" then set(false) end
end

function onConfigChanged()
  apply(noctalia.state.get("awake"))
end

noctalia.state.watch("awake", function(s) apply(s) end)
apply(noctalia.state.get("awake"))
