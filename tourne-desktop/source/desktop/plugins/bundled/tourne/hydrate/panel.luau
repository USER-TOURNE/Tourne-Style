--!nonstrict
-- Hydrate's panel: today's cups against the goal, each logged time, add or take back a cup.

-- (Shared with the other entry: the runtime has no require.)
local shared = {}

local function file()
  return (noctalia.pluginDataDir() or "") .. "\\log.json"
end

function shared.today()
  return os.date("%Y-%m-%d")
end

function shared.load()
  local text = noctalia.readFile(file())
  local log = text and noctalia.json.decode(text) or nil
  if type(log) ~= "table" or log.day ~= shared.today() then log = { day = shared.today(), cups = {}, lastReminder = 0 } end
  return log
end

function shared.save(log)
  noctalia.mkdirAll(noctalia.pluginDataDir() or "")
  noctalia.writeFile(file(), noctalia.json.encode(log))
  noctalia.state.set("hydrate.log", log)
end

function shared.add(log)
  table.insert(log.cups, os.time())
  shared.save(log)
end

local render

render = function()
  local log = shared.load()
  local goal = noctalia.getConfig("goal") or 8
  local ml = noctalia.getConfig("cupMl") or 250
  local times = {}
  for i = #log.cups, 1, -1 do
    table.insert(times, ui.label({ key = "c" .. i, text = "Cup " .. i .. " at " .. os.date("%H:%M", log.cups[i]), fontSize = 12, color = "on_surface_variant" }))
  end
  if #times == 0 then table.insert(times, ui.label({ text = "No cups yet today", color = "on_surface_variant" })) end
  panel.render(ui.column({ flexGrow = 1, gap = 10, padding = 12 }, {
    ui.row({ gap = 8, align = "center" }, {
      ui.glyph({ name = "droplet", size = 22, color = "primary" }),
      ui.label({ text = string.format("%d of %d cups · %.1f L", #log.cups, goal, #log.cups * ml / 1000), flexGrow = 1, color = "on_surface", fontWeight = "semibold" }),
    }),
    ui.progress({ progress = math.min(1, #log.cups / goal) }),
    ui.row({ gap = 6 }, {
      ui.button({ text = "A cup", glyph = "plus", variant = "primary", flexGrow = 1, onClick = function()
        shared.add(log)
        render()
      end }),
      ui.button({ text = "Undo", glyph = "arrow-back-up", variant = "ghost", enabled = #log.cups > 0, onClick = function()
        table.remove(log.cups)
        shared.save(log)
        render()
      end }),
    }),
    ui.scroll({ key = "times", flexGrow = 1, gap = 2 }, times),
  }))
end

function onOpen(_context)
  render()
end
