--!nonstrict
-- The tray icon: a click hides or shows the watermark (until Tourne Desktop restarts). Nothing runs between clicks.

local function show(on)
  barWidget.setGlyph("brand-windows")
  barWidget.setColor(on and "primary" or "on_surface_variant")
  barWidget.setTooltip(on and "Watermark shown: click to hide it" or "Watermark hidden: click to show it")
end

function onClick()
  local on = noctalia.state.get("hidden") == true
  noctalia.state.set("hidden", not on)
  show(on)
end

show(noctalia.state.get("hidden") ~= true)
