--!nonstrict
-- OCR Scanner's panel: snip, read the copied picture or a file, the language, the last text (to edit
-- and copy) and the history. The bar widget does the reading.

local langs = {}
local file = ""
local edited = nil
local render

local function send(c)
  noctalia.state.set("ocr.command", c)
end

local function ago(at)
  local s = os.time() - (tonumber(at) or 0)
  if s < 60 then return "just now" end
  if s < 3600 then return tostring(s // 60) .. " min ago" end
  if s < 86400 then return tostring(s // 3600) .. " h ago" end
  return os.date("%Y-%m-%d", at)
end

render = function()
  local o = noctalia.state.get("ocr") or { history = {} }
  local lang = noctalia.state.get("ocr.lang") or noctalia.getConfig("language") or "eng"
  local options, at = { lang }, 0
  for _, l in langs do
    if l ~= lang then table.insert(options, l) end
  end
  local history = {}
  for i, h in o.history or {} do
    if i > 1 then
      table.insert(history, ui.row({ key = "h" .. tostring(i), gap = 6, align = "center", padding = 6, radius = 6,
        fill = "surface_container" }, {
        ui.column({ flexGrow = 1, gap = 1 }, {
          ui.label({ text = string.gsub(h.text, "%s+", " "), maxLines = 2 }),
          ui.label({ text = ago(h.at) .. ", " .. (h.from or ""), fontSize = 10, color = "on_surface_variant" }),
        }),
        ui.button({ glyph = "copy", tooltip = "Copy", controlSize = "sm", variant = "ghost",
          onClick = function() noctalia.copyToClipboard(h.text) end }),
      }))
    end
  end
  local text = edited or o.text or ""
  panel.render(ui.column({ flexGrow = 1, gap = 10, padding = 12 }, {
    ui.row({ gap = 6 }, {
      ui.button({ text = "Snip and read", glyph = "crop", variant = "primary", controlSize = "sm", enabled = not o.busy,
        onClick = function()
          panel.close()
          send({ op = "snip" })
        end }),
      ui.button({ text = "Read copied picture", glyph = "clipboard-text", controlSize = "sm", enabled = not o.busy,
        onClick = function() send({ op = "clipboard" }) end }),
      ui.spacer({ flexGrow = 1 }),
      ui.select({ options = options, selectedIndex = at, controlSize = "sm", onChange = function(i)
        noctalia.state.set("ocr.lang", options[(tonumber(i) or 0) + 1])
        render()
      end }),
    }),
    ui.row({ gap = 6, align = "center" }, {
      ui.input({ key = "file", value = file, placeholder = "Or a picture file's path", controlSize = "sm", flexGrow = 1,
        onChange = function(v) file = v end,
        onSubmit = function(v)
          file = v
          send({ op = "file", path = file })
        end }),
      ui.button({ text = "Read", glyph = "photo-scan", controlSize = "sm", enabled = not o.busy,
        onClick = function() send({ op = "file", path = file }) end }),
    }),
    ui.label({ text = o.busy and "Reading..." or (o.error or ""), fontSize = 11,
      color = o.error and not o.busy and "error" or "on_surface_variant", visible = o.busy or o.error ~= nil }),
    ui.input({ key = "text", value = text, multiline = true, placeholder = "The text shows here", height = 170,
      onChange = function(v) edited = v end }),
    ui.row({ gap = 6 }, {
      ui.button({ text = "Copy", glyph = "copy", controlSize = "sm", enabled = text ~= "", onClick = function()
        noctalia.copyToClipboard(text)
      end }),
      ui.spacer({ flexGrow = 1 }),
      ui.button({ text = "Clear history", glyph = "trash", controlSize = "sm", variant = "ghost",
        enabled = #(o.history or {}) > 0, onClick = function() send({ op = "clear" }) end }),
    }),
    ui.scroll({ flexGrow = 1 }, { ui.column({ gap = 6 }, history) }),
  }))
end

local function exe()
  local given = noctalia.getConfig("tesseract") or ""
  if given ~= "" then return given end
  if noctalia.commandExists("tesseract") then return "tesseract" end
  return "C:\\Program Files\\Tesseract-OCR\\tesseract.exe"
end

function onOpen(_context)
  edited = nil
  render()
  noctalia.runAsync({ exe(), "--list-langs" }, function(r)
    langs = {}
    local first = true
    for line in string.gmatch((r.stdout or "") .. "\n", "([^\r\n]*)\r?\n") do
      if first then
        first = false
      elseif line ~= "" and line ~= "osd" then
        table.insert(langs, line)
      end
    end
    render()
  end, 15000)
end

noctalia.state.watch("ocr", function()
  edited = nil
  render()
end)
