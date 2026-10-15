--!nonstrict
-- Breathing: from hthienloc's Breathing (DankMaterialShell, MIT), rewritten for Tourne Desktop.
-- Pick a technique and a length; the circle follows each phase, a second at a time.

local TECHNIQUES = {
  { name = "Deep breathing", phases = { { "Breathe in", 4 }, { "Breathe out", 6 } } },
  { name = "4-7-8", phases = { { "Breathe in", 4 }, { "Hold", 7 }, { "Breathe out", 8 } } },
  { name = "Box", phases = { { "Breathe in", 4 }, { "Hold", 4 }, { "Breathe out", 4 }, { "Hold", 4 } } },
  { name = "Equal", phases = { { "Breathe in", 5 }, { "Breathe out", 5 } } },
  { name = "Resonance", phases = { { "Breathe in", 6 }, { "Breathe out", 6 } } },
  { name = "Alternate nostril", phases = { { "In, left nostril", 4 }, { "Hold", 4 }, { "Out, right nostril", 4 }, { "In, right nostril", 4 }, { "Hold", 4 }, { "Out, left nostril", 4 } } },
}
local LENGTHS = { 1, 3, 5, 10, 15, 30 }

local technique = 1
local minutes = 3
local running = false
local elapsed = 0 -- seconds into the session
local render

local function chime()
  local path = noctalia.getConfig("chime") or ""
  if path == "" then return end
  noctalia.sound.load("chime", path, function(ok)
    if ok then noctalia.sound.play("chime") end
  end)
end

-- Where the session is: the phase, seconds into it, and its length.
local function phaseAt(t)
  local phases = TECHNIQUES[technique].phases
  local cycle = 0
  for _, p in phases do cycle += p[2] end
  local inCycle = t % cycle
  for i, p in phases do
    if inCycle < p[2] then return i, inCycle, p end
    inCycle -= p[2]
  end
  return 1, 0, phases[1]
end

render = function()
  local children = {
    ui.select({ options = (function()
      local names = {}
      for _, t in TECHNIQUES do table.insert(names, t.name) end
      return names
    end)(), selectedIndex = technique - 1, enabled = not running, onChange = function(i)
      technique = (tonumber(i) or 0) + 1
      render()
    end }),
  }
  local lengths = {}
  for _, m in LENGTHS do
    table.insert(lengths, ui.button({ key = "m" .. m, text = m .. "m", controlSize = "sm", variant = minutes == m and "primary" or "outline",
      enabled = not running, onClick = function()
        if minutes == m and not running then
          running, elapsed = true, 0
          chime()
        end
        minutes = m
        render()
      end }))
  end
  table.insert(children, ui.row({ gap = 4, justify = "center" }, lengths))
  local i, into, phase = phaseAt(elapsed)
  local word, size
  if running then
    local fraction = (into + 1) / phase[2]
    local label = phase[1]
    local function grows(text)
      local l = string.lower(text)
      return string.sub(l, 1, 10) == "breathe in" or string.sub(l, 1, 3) == "in,"
    end
    if grows(label) then size = 0.35 + 0.65 * fraction
    elseif string.find(string.lower(label), "out") then size = 1 - 0.65 * fraction
    else
      local phases = TECHNIQUES[technique].phases
      size = grows(phases[i > 1 and i - 1 or #phases][1]) and 1 or 0.35
    end
    word = string.format("%s · %d", label, phase[2] - into)
  else
    size, word = 0.5, "Ready (tap a length twice, or Start)"
  end
  local d = math.floor(220 * size)
  table.insert(children, ui.column({ height = 240, align = "center", justify = "center" }, {
    ui.box({ width = d, height = d, radius = d / 2, fill = "primary/0.35", border = "primary", borderWidth = 2 }),
  }))
  table.insert(children, ui.label({ text = word, textAlign = "center", fontSize = 16, color = "on_surface" }))
  if running then
    local left = minutes * 60 - elapsed
    table.insert(children, ui.label({ text = string.format("%d:%02d left", left // 60, left % 60), textAlign = "center", fontSize = 12, color = "on_surface_variant" }))
  end
  table.insert(children, ui.button({ text = running and "Stop" or "Start", glyph = running and "player-stop" or "player-play",
    variant = running and "ghost" or "primary", onClick = function()
      running, elapsed = not running, 0
      if running then chime() end
      render()
    end }))
  panel.render(ui.column({ flexGrow = 1, gap = 10, padding = 14 }, children))
end

function onOpen(_context)
  panel.setWantsSecondTicks(true)
  render()
end

function onClose()
  running = false
end

function update()
  if running then
    local before = phaseAt(elapsed)
    elapsed += 1
    if elapsed >= minutes * 60 then
      running = false
      chime()
      noctalia.notify("Breathing", "Done. Well breathed.")
    elseif phaseAt(elapsed) ~= before then
      chime()
    end
  end
  render()
end
