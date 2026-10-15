--!nonstrict
-- Tourne Desktop's own Power Menu, after JDKamalakar's DMS Fullscreen Power Menu.
-- The tray icon: a click opens the menu. Nothing runs between clicks.
-- TourneDesktop.exe --msg plugin tourne/power-menu:tray all toggle

barWidget.setGlyph("power")
barWidget.setTooltip("Power menu: lock, sleep, sign out, restart, shut down")

function onClick() noctalia.togglePanel("tourne/power-menu:menu") end

function onIpc(event)
  if event == "toggle" or event == "open" then noctalia.togglePanel("tourne/power-menu:menu") end
end
