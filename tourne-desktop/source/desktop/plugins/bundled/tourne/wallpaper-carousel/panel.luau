--!nonstrict
-- Wallpaper Carousel: the pictures of a folder in a row, the chosen one larger in the middle.
--
-- Written for Tourne Desktop from the idea of motor-dev's Wallpaper Carousel (a skewed full-screen
-- Quickshell overlay on Linux). Here it's a floating panel: small copies of the pictures
-- (tourne.thumbnail) so the panel never holds the full images, arrows or a click to move, and a click
-- on the chosen picture (or Set) makes it the wallpaper on every screen or the one in front.

local PICTURE = { jpg = true, jpeg = true, jfif = true, png = true, bmp = true, gif = true, webp = true, tif = true, tiff = true, heic = true, avif = true }

local folder = ""
local pictures = {}
local index = 1
local thumbs = {} -- picture path -> small copy's path (false while it's being made)
local status = ""

local function cfg(key, fallback)
  local v = noctalia.getConfig(key)
  if v == nil or v == "" then return fallback end
  return v
end

local function extension(name)
  return string.lower(string.match(name, "%.([^%.\\/]+)$") or "")
end

local function baseName(path)
  return string.match(path, "([^\\/]+)$") or path
end

local function load()
  folder = noctalia.expandPath(cfg("wallpaperDirectory", "")) or ""
  if folder == "" then folder = noctalia.wallpaperDirectory() or "" end
  pictures = {}
  if folder ~= "" then
    local names = noctalia.listDir(folder) or {}
    table.sort(names, function(a, b) return string.lower(a) < string.lower(b) end)
    for _, name in names do
      if PICTURE[extension(name)] then table.insert(pictures, folder .. "\\" .. name) end
    end
  end
  if cfg("shuffleOnOpen", false) then
    for i = #pictures, 2, -1 do
      local j = math.random(i)
      pictures[i], pictures[j] = pictures[j], pictures[i]
    end
  end
  -- Open on the wallpaper showing now.
  index = 1
  local current = string.lower(noctalia.wallpaperPath() or "")
  for i, p in pictures do
    if string.lower(p) == current then index = i end
  end
  status = #pictures == 0 and (folder == "" and "No wallpaper folder: set one in this plugin's settings" or "No pictures in " .. folder) or ""
end

local function wrapIndex(i)
  local n = #pictures
  if n == 0 then return 1 end
  if cfg("carouselMode", "wrap") == "standard" then return math.clamp(i, 1, n) end
  return ((i - 1) % n) + 1
end

local render

local function want(path)
  if thumbs[path] ~= nil then return end
  thumbs[path] = false
  local width = math.floor(cfg("itemWidth", 200) * cfg("selectedScale", 125) / 100 * 1.5)
  tourne.thumbnail(path, width, function(small)
    thumbs[path] = small or path
    render()
  end)
end

local function apply(path)
  local ok, why
  if cfg("applyToAllMonitors", true) then
    ok, why = noctalia.setWallpaper(path)
  else
    ok, why = noctalia.setWallpaper(noctalia.focusedOutputName() or "", path)
  end
  if ok == false then
    status = "Couldn't set it: " .. tostring(why)
    render()
    return
  end
  status = "Set " .. baseName(path)
  if cfg("closeOnApply", true) then panel.close() else render() end
end

local function move(by)
  index = wrapIndex(index + by)
  render()
end

local function card(i, chosen)
  local path = pictures[i]
  want(path)
  local w = cfg("itemWidth", 200)
  if chosen then w = math.floor(w * cfg("selectedScale", 125) / 100) end
  local h = math.floor(w * 9 / 16)
  local picture = thumbs[path]
  local inner
  if picture then
    inner = ui.image({ path = picture, width = w, height = h, radius = 6 })
  else
    inner = ui.box({ width = w, height = h, fill = "surface_variant", radius = 6 })
  end
  return ui.column({
    key = "card-" .. i,
    gap = 4,
    align = "center",
    padding = 4,
    radius = 8,
    border = chosen and "primary" or nil,
    borderWidth = chosen and 2 or 0,
    onClick = function()
      if chosen then apply(path) else index = i; render() end
    end,
  }, {
    inner,
    ui.label({ text = baseName(path), maxLines = 1, maxWidth = w, fontSize = 11, color = chosen and "on_surface" or "on_surface_variant" }),
  })
end

render = function()
  local cards = {}
  local n = #pictures
  if n > 0 then
    local shown = math.min(cfg("visibleCount", 5), n)
    local half = math.floor(shown / 2)
    local seen = {}
    for offset = -half, shown - half - 1 do
      local i = index + offset
      if cfg("carouselMode", "wrap") == "standard" then
        if i >= 1 and i <= n then table.insert(cards, card(i, offset == 0)) end
      else
        i = wrapIndex(i)
        if not seen[i] then
          seen[i] = true
          table.insert(cards, card(i, offset == 0))
        end
      end
    end
    -- The next ones on each side, so moving shows them at once.
    want(pictures[wrapIndex(index + half + 1)])
    want(pictures[wrapIndex(index - half - 1)])
  end
  panel.render(ui.column({ flexGrow = 1, gap = 10, padding = 12 }, {
    ui.row({ gap = 8, align = "center" }, {
      ui.glyph({ name = "carousel-horizontal", size = 18, color = "primary" }),
      ui.label({ text = n > 0 and string.format("%d of %d", index, n) or "Wallpapers", flexGrow = 1, color = "on_surface" }),
      ui.button({ glyph = "arrows-shuffle", variant = "ghost", tooltip = "A random one", onClick = function()
        if n > 0 then index = math.random(n); render() end
      end }),
      ui.button({ glyph = "refresh", variant = "ghost", tooltip = "Read the folder again", onClick = function()
        thumbs = {}; load(); render()
      end }),
      ui.button({ glyph = "folder", variant = "ghost", tooltip = "Open the folder", onClick = function()
        if folder ~= "" then noctalia.openPath(folder) end
      end }),
    }),
    ui.row({ flexGrow = 1, gap = 8, align = "center", justify = "center" }, {
      ui.button({ glyph = "chevron-left", variant = "ghost", onClick = function() move(-1) end }),
      ui.row({ flexGrow = 1, gap = 10, align = "center", justify = "center" }, cards),
      ui.button({ glyph = "chevron-right", variant = "ghost", onClick = function() move(1) end }),
    }),
    ui.row({ gap = 8, align = "center" }, {
      ui.label({ text = status, flexGrow = 1, maxLines = 1, color = "on_surface_variant", fontSize = 12 }),
      ui.button({ text = "Set wallpaper", glyph = "photo", variant = "primary", enabled = n > 0, onClick = function()
        if n > 0 then apply(pictures[index]) end
      end }),
    }),
  }))
end

function onOpen(_context)
  load()
  render()
end

function onScroll(delta)
  move((tonumber(delta) or 0) > 0 and -1 or 1)
end
