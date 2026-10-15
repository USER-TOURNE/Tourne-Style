--!nonstrict
-- The tray icon: a click opens the launcher on the games ("g "). Nothing runs between clicks.

barWidget.setGlyph("device-gamepad-2")
barWidget.setTooltip("Games: click to search them in the launcher")

function onClick() tourne.openLauncher("g ") end
