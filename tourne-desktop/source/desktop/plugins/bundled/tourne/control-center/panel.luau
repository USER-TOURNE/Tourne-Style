--!nonstrict
-- Control Center: each running plugin's bar button as a tile (tourne.barItems), clicked through to the plugin
-- (tourne.barClick). A tile lights up while its plugin shows its button in the accent colour (on, awake...).
-- Redrawn each second while open, so tiles follow what the plugins show.

local function cfg(k, d)
  local v = noctalia.getConfig(k)
  if v == nil then return d end
  return v
end

local render

local function firstLine(s)
  s = tostring(s or "")
  local i = string.find(s, "\n", 1, true)
  return i and string.sub(s, 1, i - 1) or s
end

local function tile(item, width)
  local on = item.color == "primary"
  local sub = item.text ~= "" and item.text or ""
  return ui.column({
    key = item.key, width = width, height = 92, radius = 14, gap = 6, padding = 8,
    align = "center", justify = "center",
    fill = on and "primary/0.22" or "surface_container_high",
    border = on and "primary" or nil, borderWidth = 1,
    onClick = function() tourne.uiSound("tap"); tourne.barClick(item.key, "left") end,
    onRightClick = function() tourne.barClick(item.key, "right") end,
  }, {
    ui.glyph({ name = item.glyph ~= "" and item.glyph or "puzzle", size = 24, color = on and "primary" or "on_surface" }),
    ui.label({ text = item.plugin, fontSize = 12, color = "on_surface", maxLines = 1, textAlign = "center", maxWidth = width - 12 }),
    sub ~= "" and ui.label({ text = sub, fontSize = 11, color = "on_surface_variant", maxLines = 1, textAlign = "center", maxWidth = width - 12 }) or nil,
  })
end

render = function()
  local items = tourne.barItems() or {}
  table.sort(items, function(a, b) return string.lower(a.plugin) < string.lower(b.plugin) end)
  local cols = math.clamp(tonumber(cfg("columns", 4)) or 4, 2, 6)
  local width = math.floor((440 - 16 * 2 - 8 * (cols - 1)) / cols)
  local rows, row = {}, {}
  for _, item in ipairs(items) do
    table.insert(row, tile(item, width))
    if #row == cols then
      table.insert(rows, ui.row({ gap = 8 }, row))
      row = {}
    end
  end
  if #row > 0 then table.insert(rows, ui.row({ gap = 8 }, row)) end
  if #items == 0 then
    table.insert(rows, ui.label({ text = "No plugin shows a bar button right now. Install some from Studio, Extensions.",
      color = "on_surface_variant", maxWidth = 400 }))
  end
  panel.render(ui.column({ flexGrow = 1, gap = 12, padding = 16 }, {
    ui.row({ gap = 8, align = "center" }, {
      ui.glyph({ name = "layout-grid", size = 18, color = "primary" }),
      ui.label({ text = "Control Center", fontSize = 15, fontWeight = "semibold", color = "on_surface" }),
      ui.spacer({}),
      ui.label({ text = "Right-click a tile for its menu", fontSize = 11, color = "on_surface_variant" }),
    }),
    ui.scroll({ flexGrow = 1 }, { ui.column({ gap = 8 }, rows) }),
  }))
end

function onOpen(_context)
  panel.setWantsSecondTicks(true)
  render()
end

function update()
  render()
end
