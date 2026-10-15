--!nonstrict
-- Take a Break: the cycle lives here. From hthienloc's Take a Break (DankMaterialShell, MIT),
-- rewritten for Tourne Desktop: the break screen is the panel, opened in the middle of the screen.
-- Being away from the keyboard and mouse for a full break counts as one (the cycle starts again).
-- Click: the panel (start a break now, snooze, skip). Middle click: pause or go on.

-- state "break" = { phase = "work" | "break", at = ms the phase ends, long = bool, shortDone = n, paused = ms left or nil }
local warned = false

local function cfg(k, d)
  local v = noctalia.getConfig(k)
  if v == nil then return d end
  return v
end

local function get()
  return noctalia.state.get("break") or { phase = "work", at = noctalia.nowMs() + cfg("workMinutes", 20) * 60000, shortDone = 0 }
end

local function put(s)
  noctalia.state.set("break", s)
end

local function startWork(s)
  s.phase, s.at, s.long, s.paused = "work", noctalia.nowMs() + cfg("workMinutes", 20) * 60000, false, nil
  warned = false
end

local function show(s)
  barWidget.setGlyph(s.phase == "break" and "coffee" or "eye")
  if s.paused then
    barWidget.setText("paused")
    barWidget.setColor("on_surface_variant")
  else
    local left = math.max(0, s.at - noctalia.nowMs())
    barWidget.setText(s.phase == "break" and string.format("%d:%02d", left // 60000, (left // 1000) % 60) or string.format("%dm", math.ceil(left / 60000)))
    barWidget.setColor(s.phase == "break" and "primary" or "on_surface")
  end
  barWidget.setTooltip("Take a Break\nClick: break now, snooze or skip · Middle click: pause")
end

function update()
  local s = get()
  if s.paused then return show(s) end
  local now = noctalia.nowMs()
  if s.phase == "work" then
    local breakLength = cfg("shortSeconds", 20)
    if tourne.idleSeconds() >= breakLength then -- away: that was a break
      startWork(s)
      put(s)
      return show(s)
    end
    local warn = cfg("warnSeconds", 10)
    if warn > 0 and not warned and s.at - now <= warn * 1000 then
      warned = true
      noctalia.notify("Break soon", string.format("A break in %d seconds", warn))
    end
    if now >= s.at then
      if cfg("skipWhenBusy", true) and tourne.busy() then
        s.at = now + 60000 -- try again in a minute
      else
        s.long = (s.shortDone or 0) >= cfg("longEvery", 3)
        s.phase = "break"
        s.at = now + (s.long and cfg("longMinutes", 5) * 60000 or cfg("shortSeconds", 20) * 1000)
        noctalia.togglePanel("tourne/take-a-break:panel")
      end
      put(s)
    end
  elseif now >= s.at then
    s.shortDone = s.long and 0 or (s.shortDone or 0) + 1
    startWork(s)
    put(s)
    noctalia.notify("Break over", "Back to it")
  end
  show(s)
end

function onClick()
  noctalia.togglePanel("tourne/take-a-break:panel")
end

function onMiddleClick()
  local s = get()
  if s.paused then
    s.at, s.paused = noctalia.nowMs() + s.paused, nil
  else
    s.paused = math.max(0, s.at - noctalia.nowMs())
  end
  put(s)
  show(s)
end

noctalia.state.watch("break", function(v)
  if type(v) == "table" then
    if v.phase == "work" and v.at - noctalia.nowMs() > cfg("warnSeconds", 10) * 1000 then warned = false end
    show(v)
  end
end)
put(get())
noctalia.setUpdateInterval(1000)
