--!nonstrict
-- Keybind Guide on the bar: click to open the sheet of every key Tourne takes.
barWidget.setGlyph("keyboard")
barWidget.setTooltip("Keybind Guide")

function onClick()
  noctalia.togglePanel("tourne/keybind-guide:panel")
end

function onRightClick()
  tourne.openKeysAndRules()
end
