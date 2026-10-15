--!nonstrict
-- The GMK87's tray icon: a click opens its panel. With "syncClock", the clock is set once at start (cable only).

barWidget.setGlyph("keyboard")
barWidget.setTooltip("GMK87: lighting, screen and settings")

function onClick() noctalia.togglePanel("tourne/gmk87:panel") end

if noctalia.getConfig("syncClock") == true then
  noctalia.runAsync({ "powershell", "-NoProfile", "-NonInteractive", "-ExecutionPolicy", "Bypass", "-File",
    noctalia.pluginDir() .. "\\gmk87.ps1", "-Op", "time" }, function(_r) end, 60000)
end
