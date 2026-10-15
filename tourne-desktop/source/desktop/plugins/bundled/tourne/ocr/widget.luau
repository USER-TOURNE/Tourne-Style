--!nonstrict
-- OCR Scanner on the bar: reading happens here (so a snip finishes with the panel closed).
-- Click: the panel. Middle click: snip part of the screen and read it. Right click: read the picture
-- already on the clipboard.
--
-- Idea from hthienloc's OCR Scanner (DankMaterialShell, which runs grim, slurp and tesseract); fresh code.
-- Here Windows' own snip takes the picture and Tesseract reads it. The panel asks through
-- noctalia.state ("ocr.command"); what it shows is "ocr".

local s = { busy = false, text = "", error = nil, history = {} }
local waitingSince = nil -- a snip is open: the clipboard number before it
local waitingUntil = 0

local function cfg(key, fallback)
  local v = noctalia.getConfig(key)
  if v == nil or v == "" then return fallback end
  return v
end

local function dataDir()
  return noctalia.pluginDataDir() or ""
end

local function exe()
  local given = cfg("tesseract", "")
  if given ~= "" then return given end
  if noctalia.commandExists("tesseract") then return "tesseract" end
  return "C:\\Program Files\\Tesseract-OCR\\tesseract.exe"
end

local function publish()
  noctalia.state.set("ocr", s)
end

local function show()
  barWidget.setGlyph("text-recognition")
  barWidget.setColor((s.busy or waitingSince) and "primary" or "on_surface")
  barWidget.setText("")
  if waitingSince then
    barWidget.setTooltip("Snip the part with text in it")
  elseif s.busy then
    barWidget.setTooltip("Reading...")
  else
    barWidget.setTooltip("OCR scanner\nMiddle click: snip and read\nRight click: read the copied picture")
  end
end

local function saveHistory()
  if not cfg("keepHistory", true) then return end
  noctalia.mkdirAll(dataDir())
  noctalia.writeFile(dataDir() .. "\\history.json", noctalia.json.encode(s.history))
end

local function read(path, from)
  s.busy, s.error = true, nil
  publish()
  show()
  local lang = noctalia.state.get("ocr.lang") or cfg("language", "eng")
  local ok = noctalia.runAsync({ exe(), path, "stdout", "-l", lang, "--psm", cfg("layout", "3") }, function(r)
    s.busy = false
    local text = noctalia.string.trim(r.stdout or "")
    if r.exitCode ~= 0 or text == "" then
      s.error = r.exitCode ~= 0 and noctalia.string.trim(r.stderr or "") or "No text found in it"
      if s.error == "" then s.error = "Tesseract ended with " .. tostring(r.exitCode) end
      noctalia.notifyError("OCR", s.error)
    else
      s.text = text
      table.insert(s.history, 1, { text = text, at = os.time(), from = from })
      while #s.history > 30 do table.remove(s.history) end
      saveHistory()
      if cfg("autoCopy", true) then noctalia.copyToClipboard(text) end
      if cfg("notify", true) then
        local first = string.sub(string.gsub(text, "%s+", " "), 1, 90)
        noctalia.notify(cfg("autoCopy", true) and "Text copied" or "Text read", first)
      end
    end
    publish()
    show()
  end, 60000)
  if not ok then
    s.busy, s.error = false, "Tesseract didn't start. Install it (winget install UB-Mannheim.TesseractOCR) or set its path"
    noctalia.notifyError("OCR", s.error)
    publish()
    show()
  end
end

local function readClipboard(from)
  local path = dataDir() .. "\\clip.png"
  noctalia.mkdirAll(dataDir())
  local ok, why = noctalia.saveClipboardImage(path)
  if not ok then
    s.error = "No picture on the clipboard (" .. tostring(why) .. ")"
    noctalia.notifyError("OCR", s.error)
    publish()
    return
  end
  read(path, from or "clipboard")
end

local function snip()
  if waitingSince or s.busy then return end
  waitingSince = tourne.clipboardSequence()
  waitingUntil = noctalia.nowMs() + 60000
  local ok, why = tourne.snip()
  if ok == false then
    waitingSince = nil
    noctalia.notifyError("OCR", "The snip didn't open: " .. tostring(why))
    return
  end
  noctalia.setUpdateInterval(250)
  show()
end

function update()
  if not waitingSince then return end
  if tourne.clipboardSequence() ~= waitingSince and tourne.clipboardHasImage() then
    waitingSince = nil
    noctalia.setUpdateInterval(60000)
    readClipboard("snip")
  elseif noctalia.nowMs() > waitingUntil then
    waitingSince = nil -- given up on (closed without a picture)
    noctalia.setUpdateInterval(60000)
  end
  show()
end

function onClick()
  noctalia.togglePanel("tourne/ocr:panel")
end

function onMiddleClick()
  snip()
end

function onRightClick()
  readClipboard()
end

noctalia.state.watch("ocr.command", function(c)
  if type(c) ~= "table" then return end
  noctalia.state.set("ocr.command", nil)
  if c.op == "snip" then
    snip()
  elseif c.op == "clipboard" then
    readClipboard()
  elseif c.op == "file" and c.path and c.path ~= "" then
    read(noctalia.expandPath(c.path), "file")
  elseif c.op == "clear" then
    s.history = {}
    saveHistory()
    publish()
  end
end)

do
  local text = cfg("keepHistory", true) and noctalia.readFile(dataDir() .. "\\history.json") or nil
  local old = text and noctalia.json.decode(text) or nil
  if type(old) == "table" then s.history = old end
end
noctalia.setUpdateInterval(60000)
publish()
show()
