--!nonstrict
-- Take a Break's screen: a countdown and a tip during a break; between breaks, start one now, snooze or skip.

local TIPS = {
  "Look at something 6 metres away for 20 seconds.",
  "Roll your shoulders back a few times.",
  "Blink slowly ten times.",
  "Stand up and stretch your arms over your head.",
  "Drink some water.",
  "Look out of a window if there's one near.",
  "Loosen your jaw and relax your hands.",
  "Take three slow, deep breaths.",
}
local tip = 1

local function cfg(k, d)
  local v = noctalia.getConfig(k)
  if v == nil then return d end
  return v
end

local function get()
  return noctalia.state.get("break") or { phase = "work", at = noctalia.nowMs(), shortDone = 0 }
end

local function render()
  local s = get()
  local left = math.max(0, s.at - noctalia.nowMs())
  local onBreak = s.phase == "break"
  local length = onBreak and (s.long and cfg("longMinutes", 5) * 60000 or cfg("shortSeconds", 20) * 1000) or cfg("workMinutes", 20) * 60000
  local buttons
  if onBreak then
    buttons = {
      ui.button({ text = "Skip", glyph = "player-skip-forward", variant = "ghost", onClick = function()
        s.at = noctalia.nowMs()
        noctalia.state.set("break", s)
        panel.close()
      end }),
    }
  else
    buttons = {
      ui.button({ text = "Break now", glyph = "coffee", variant = "primary", onClick = function()
        s.at = noctalia.nowMs()
        noctalia.state.set("break", s)
      end }),
      ui.button({ text = "Snooze 5 min", glyph = "clock-pause", variant = "ghost", onClick = function()
        s.at = math.max(s.at, noctalia.nowMs()) + 300000
        noctalia.state.set("break", s)
        panel.close()
      end }),
    }
  end
  panel.render(ui.column({ flexGrow = 1, gap = 14, padding = 20, align = "center", justify = "center" }, {
    ui.glyph({ name = onBreak and (s.long and "stretching" or "eye") or "eye-check", size = 36, color = "primary" }),
    ui.label({ text = onBreak and (s.long and "Long break" or "Eye break") or "Working", fontSize = 18, color = "on_surface", fontWeight = "semibold" }),
    ui.label({ text = string.format("%d:%02d", left // 60000, (left // 1000) % 60), fontSize = 44, color = onBreak and "primary" or "on_surface" }),
    ui.progress({ progress = length > 0 and (1 - left / length) or 0, width = 360 }),
    ui.label({ text = onBreak and TIPS[tip] or string.format("Next break in %d min · %d short breaks done", math.ceil(left / 60000), s.shortDone or 0),
      color = "on_surface_variant", textAlign = "center", maxWidth = 440 }),
    ui.row({ gap = 8, justify = "center" }, buttons),
  }))
  if onBreak and left <= 0 then panel.close() end
end

function onOpen(_context)
  tip = math.random(#TIPS)
  panel.setWantsSecondTicks(true)
  render()
end

function update()
  render()
end
