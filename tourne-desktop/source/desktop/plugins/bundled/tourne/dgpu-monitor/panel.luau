--!nonstrict
-- Every graphics adapter with its power state, and the battery.

local function render()
  local rows = {}
  for i, g in tourne.gpus() do
    table.insert(rows, ui.row({ key = "gpu" .. i, gap = 8, align = "center" }, {
      ui.glyph({ name = g.active and "bolt" or "zzz", size = 16, color = g.active and "primary" or "on_surface_variant" }),
      ui.column({ flexGrow = 1, gap = 2 }, {
        ui.label({ text = g.name, maxLines = 1, color = "on_surface" }),
        ui.label({ text = (g.discrete and "Discrete" or "Built in") .. " · " .. (g.active and "Awake" or "Asleep") .. " (" .. g.powerState .. ")",
          fontSize = 12, color = "on_surface_variant" }),
      }),
    }))
  end
  if #rows == 0 then table.insert(rows, ui.label({ text = "No graphics adapters found", color = "on_surface_variant" })) end
  local b = tourne.battery()
  table.insert(rows, ui.separator({ spacing = 6 }))
  if b then
    local draw = b.rateW and string.format("%s %.1f W", b.rateW < 0 and "Drawing" or "Charging at", math.abs(b.rateW)) or "Draw unknown"
    table.insert(rows, ui.label({ text = string.format("Battery %d%%%s · %s", b.percent, b.onAc and ", plugged in" or "", draw), color = "on_surface" }))
    if b.minutesLeft then
      table.insert(rows, ui.label({ text = string.format("About %d h %02d min left", b.minutesLeft // 60, b.minutesLeft % 60), fontSize = 12, color = "on_surface_variant" }))
    end
  else
    table.insert(rows, ui.label({ text = "No battery", color = "on_surface_variant" }))
  end
  panel.render(ui.column({ flexGrow = 1, gap = 8, padding = 12 }, rows))
end

function onOpen(_context)
  panel.setWantsSecondTicks(true)
  render()
end

function update()
  render()
end
