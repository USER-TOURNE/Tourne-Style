--!nonstrict
-- The "Activate Windows" watermark, with your own words. Drawn once, and again only when a setting changes.

local function cfg(key, fallback)
  local v = noctalia.getConfig(key)
  if v == nil or v == "" then return fallback end
  return v
end

local function render()
  if noctalia.state.get("hidden") == true then desktopWidget.render(ui.column({}, {})) return end
  local a = math.clamp(tonumber(cfg("opacity", 45)) or 45, 10, 100) / 100
  desktopWidget.render(ui.column({ gap = 2, padding = 4 }, {
    ui.label({ text = cfg("title", "Activate Tourne"), fontSize = 20, color = `on_surface/{a}` }),
    ui.label({ text = cfg("line", "Go to Settings to activate Tourne."), fontSize = 13, color = `on_surface/{a}` }),
  }))
end

function onConfigChanged() render() end
noctalia.state.watch("hidden", function() render() end)

render()
