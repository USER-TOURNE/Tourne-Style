--!nonstrict
-- Keystrokes on the bar: shows or hides the keycaps (Tourne Desktop's own overlay, Studio > Keystrokes).
-- Nothing runs here between clicks.

local function show(on)
  barWidget.setGlyph(on and "keyboard" or "keyboard-off")
  barWidget.setColor(on and "primary" or "on_surface_variant")
  barWidget.setTooltip(on and "Keystrokes on screen: click to hide them\nRight click: settings"
    or "Show keystrokes on screen\nRight click: settings")
end

function onClick()
  show(tourne.keystrokes("toggle"))
end

function onRightClick()
  tourne.openStudio("keystrokes")
end

show(tourne.keystrokes())
