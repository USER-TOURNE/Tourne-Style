--!nonstrict
-- Timer's panel: presets, your own minutes, start / pause / reset, and what happens at the end.

local ACTIONS = { "none", "lock", "sleep", "hibernate", "shutdown" }
local ACTION_NAMES = { "Nothing more", "Lock", "Sleep", "Hibernate", "Shut down" }
local typed = ""
local render

local function get()
  return noctalia.state.get("timer") or { length = 0, left = 0, action = noctalia.getConfig("whenDone") or "none" }
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

local function start(minutes)
  local t = get()
  local ms = math.floor(minutes * 60000)
  if ms <= 0 then return end
  noctalia.state.set("timer", { length = ms, endsAt = noctalia.nowMs() + ms, left = 0, action = t.action })
  render()
end

-- "25", "1:30" (minutes:seconds) or "90s".
local function parse(text)
  text = noctalia.string.trim(text)
  local m, s = string.match(text, "^(%d+):(%d%d?)$")
  if m then return tonumber(m) + tonumber(s) / 60 end
  local secs = string.match(text, "^(%d+)%s*s$")
  if secs then return tonumber(secs) / 60 end
  return tonumber(text)
end

render = function()
  local t = get()
  local left = remaining(t)
  local presets = {}
  for value in string.gmatch(noctalia.getConfig("presets") or "", "[^,]+") do
    local n = tonumber(noctalia.string.trim(value))
    if n then
      table.insert(presets, ui.button({ key = "p" .. value, text = tostring(n) .. "m", controlSize = "sm", variant = "outline", onClick = function() start(n) end }))
    end
  end
  local actionAt = 0
  for i, a in ACTIONS do
    if a == t.action then actionAt = i - 1 end
  end
  local running = t.endsAt ~= nil
  panel.render(ui.column({ flexGrow = 1, gap = 10, padding = 12 }, {
    ui.label({ text = (running or left > 0) and clock(left) or "0:00", fontSize = 40, textAlign = "center", color = running and "primary" or "on_surface" }),
    ui.progress({ progress = (t.length or 0) > 0 and (1 - left / t.length) or 0 }),
    ui.row({ gap = 6, justify = "center" }, presets),
    ui.row({ gap = 6, align = "center" }, {
      ui.input({ key = "own", placeholder = "Minutes, 1:30 or 90s", controlSize = "sm", flexGrow = 1,
        onChange = function(v) typed = v end,
        onSubmit = function(v)
          local m = parse(v)
          if m then start(m) end
        end }),
      ui.button({ text = "Start", glyph = "player-play", variant = "primary", controlSize = "sm", onClick = function()
        local m = parse(typed)
        if m then start(m) end
      end }),
    }),
    ui.row({ gap = 6, justify = "center" }, {
      ui.button({ text = running and "Pause" or "Go on", glyph = running and "player-pause" or "player-play", controlSize = "sm",
        enabled = running or left > 0, onClick = function()
          if running then
            t.left, t.endsAt = remaining(t), nil
          else
            t.endsAt, t.left = noctalia.nowMs() + (t.left or 0), 0
          end
          noctalia.state.set("timer", t)
          render()
        end }),
      ui.button({ text = "Reset", glyph = "refresh", controlSize = "sm", variant = "ghost", onClick = function()
        noctalia.state.set("timer", { length = t.length, left = 0, action = t.action })
        render()
      end }),
    }),
    ui.row({ gap = 6, align = "center" }, {
      ui.label({ text = "When it ends", color = "on_surface_variant", flexGrow = 1 }),
      ui.select({ options = ACTION_NAMES, selectedIndex = actionAt, controlSize = "sm", onChange = function(i)
        t.action = ACTIONS[(tonumber(i) or 0) + 1]
        noctalia.state.set("timer", t)
        render()
      end }),
    }),
  }))
end

function onOpen(_context)
  panel.setWantsSecondTicks(true)
  render()
end

function update()
  render()
end
