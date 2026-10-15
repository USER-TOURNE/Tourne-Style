--!nonstrict
-- The tray icon: a click opens the launcher on the screens' modes and profiles ("dp "). Nothing runs between clicks.

barWidget.setGlyph("device-desktop")
barWidget.setTooltip("Display profiles: click to pick a mode or profile in the launcher")

function onClick() tourne.openLauncher("dp ") end
