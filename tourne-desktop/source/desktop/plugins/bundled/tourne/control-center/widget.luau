--!nonstrict
-- Control Center's tray icon: a click opens the tiles. Nothing runs between clicks.

barWidget.setGlyph("layout-grid")
barWidget.setTooltip("Control Center: every plugin in one place")

function onClick() noctalia.togglePanel("tourne/control-center:panel") end
