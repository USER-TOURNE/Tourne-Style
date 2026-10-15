--!nonstrict
-- Bar button for the wallpaper carousel.

barWidget.setGlyph(noctalia.getConfig("glyph") or "carousel-horizontal")
barWidget.setTooltip("Wallpaper carousel")

function onClick()
  noctalia.togglePanel("tourne/wallpaper-carousel:panel")
end
