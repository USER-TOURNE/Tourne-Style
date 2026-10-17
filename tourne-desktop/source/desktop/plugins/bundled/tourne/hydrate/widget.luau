--!nonstrict
-- Hydrate on the bar. From hthienloc's Hydrate (DankMaterialShell, MIT), rewritten for Tourne Desktop.
-- Click: today's progress. Right click: a cup.

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

local log = shared.load()

local function due()
  local last = #log.cups > 0 and log.cups[#log.cups] or 0
  local since = math.max(last, log.started or 0)
  return os.time() - since >= (noctalia.getConfig("intervalMinutes") or 60) * 60
end

local function show()
  local goal = noctalia.getConfig("goal") or 8
  local thirsty = due() and #log.cups < goal
  barWidget.setGlyph(thirsty and "glass" or "glass-full")
  barWidget.setColor(thirsty and "primary" or "on_surface")
  barWidget.setText(string.format("%d/%d", #log.cups, goal))
  barWidget.setTooltip(thirsty and "Time for some water (right click logs a cup)" or "Hydrate: right click logs a cup")
end

function update()
  if log.day ~= shared.today() then
    log = shared.load()
    log.started = os.time()
  end
  if due() and #log.cups < (noctalia.getConfig("goal") or 8) and noctalia.getConfig("notify")
    and os.time() - (log.lastReminder or 0) >= (noctalia.getConfig("intervalMinutes") or 60) * 60 then
    log.lastReminder = os.time()
    shared.save(log)
    noctalia.notify("Hydrate", "Time for a glass of water")
  end
  show()
end

function onClick()
  noctalia.togglePanel("tourne/hydrate:panel")
end

function onRightClick()
  shared.add(log)
  show()
end

noctalia.state.watch("hydrate.log", function(v)
  if type(v) == "table" then log = v end
  show()
end)
log.started = log.started or os.time()
noctalia.setUpdateInterval(30000)
show()
