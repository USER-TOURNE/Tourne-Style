--!nonstrict
-- Tourne'Table's panel on the bar. Click: show or hide it. Right click: keep it on top, or not.
-- The panel is Tourne'Table with Position > Place In = Panel, built into Tourne Desktop or running
-- in Windhawk; tourne.tablePanel finds it either way.

local function show(shown)
  barWidget.setGlyph("wave-sine")
  if shown == nil then
    barWidget.setColor("on_surface_variant")
    barWidget.setTooltip("No Tourne'Table panel\nTurn on Tourne'Table and set Position > Place In to Panel")
  else
    barWidget.setColor(shown and "primary" or "on_surface")
    barWidget.setTooltip(shown and "Hide the visualizer\nRight click: keep it on top, or not"
      or "Show the visualizer\nRight click: keep it on top, or not")
  end
end

function update()
  show(tourne.tablePanel())
end

function onClick()
  local shown = tourne.tablePanel("toggle")
  if shown == nil then
    noctalia.notify("Tourne'Table", "There's no panel to show. Turn on Tourne'Table and set Position > Place In to Panel.")
  end
  show(shown)
end

function onRightClick()
  show(tourne.tablePanel("pin"))
end

noctalia.setUpdateInterval(3000)
update()
