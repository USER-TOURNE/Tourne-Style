--!nonstrict
-- Hides or shows every plugin's desktop widgets (tourne.desktopWidgets). Nothing runs between clicks.

local function show(on)
  barWidget.setGlyph(on and "layout-dashboard" or "layout-off")
  barWidget.setColor(on and "primary" or "on_surface_variant")
  barWidget.setTooltip(on and "Desktop widgets shown: click to hide them" or "Desktop widgets hidden: click to show them")
end

function onClick() show(tourne.desktopWidgets("toggle")) end

show(tourne.desktopWidgets())
