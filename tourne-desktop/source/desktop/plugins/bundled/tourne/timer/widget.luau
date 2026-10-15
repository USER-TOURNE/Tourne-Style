--!nonstrict
-- Timer on the bar. The countdown lives here, so it runs with the panel closed; the panel changes it
-- through noctalia.state ("timer"). Click: the panel. Middle click: pause or go on. Right click: reset.
--
-- From hthienloc's Timer (DankMaterialShell, MIT), rewritten for Tourne Desktop.

-- timer = { endsAt = ms or nil, left = ms (while paused), length = ms, action = "none" | ... }
local function get()
  return noctalia.state.get("timer") or { length = 0, left = 0, action = noctalia.getConfig("whenDone") or "none" }
end

local function put(t)
  noctalia.state.set("timer", t)
end

local function remaining(t)
  if t.endsAt then return math.max(0, t.endsAt - noctalia.nowMs()) end
  return t.left or 0
end

local function clock(ms)
  local s = math.ceil(ms / 1000)
  if s >= 3600 then return string.format("%d:%02d:%02d", s // 3600, (s // 60) % 60, s % 60) end
  return string.format("%d:%02d", s // 60, s % 60)
end

local function show()
  local t = get()
  local left = remaining(t)
  barWidget.setGlyph("clock-hour-4")
  if t.endsAt or left > 0 then
    barWidget.setText(clock(left) .. (t.endsAt and "" or " ⏸"))
    barWidget.setColor(t.endsAt and "primary" or "on_surface_variant")
  else
    barWidget.setText("")
    barWidget.setColor("on_surface")
  end
  barWidget.setTooltip("Timer\nMiddle click: pause or go on · Right click: reset")
end

local function finished(t)
  local sound = noctalia.getConfig("sound") or ""
  if sound ~= "" then
    noctalia.sound.load("done", sound, function(ok)
      if ok then noctalia.sound.play("done") end
    end)
  end
  noctalia.notify("Time's up", string.format("%s timer done", clock(t.length or 0)))
  local action = t.action or "none"
  if noctalia.getConfig("autoReset") then
    put({ length = t.length, left = 0, action = t.action })
  else
    put({ length = t.length, left = 0, action = t.action, done = true })
  end
  if action ~= "none" then
    local ok, why = tourne.systemAction(action)
    if ok == false then noctalia.notifyError("Timer", "Couldn't " .. action .. ": " .. tostring(why)) end
  end
end

function update()
  local t = get()
  if t.endsAt and noctalia.nowMs() >= t.endsAt then finished(t) end
  show()
end

function onClick()
  noctalia.togglePanel("tourne/timer:panel")
end

function onMiddleClick()
  local t = get()
  if t.endsAt then
    t.left, t.endsAt = remaining(t), nil
  elseif (t.left or 0) > 0 then
    t.endsAt, t.left = noctalia.nowMs() + t.left, 0
  end
  put(t)
  show()
end

function onRightClick()
  local t = get()
  put({ length = t.length, left = 0, action = t.action })
  show()
end

noctalia.state.watch("timer", function()
  show()
end)
noctalia.setUpdateInterval(500)
show()
