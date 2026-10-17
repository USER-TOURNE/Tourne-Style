--!nonstrict
-- Ported to Tourne Desktop from alexander's Screen Toolkit (MIT).
--
-- Screen Toolkit: headless [[service]].
--
-- Owns every tool pipeline (color pick, OCR, QR, palette, lens, annotate,
-- measure, recording, sharing) plus state persistence. The bar [[widget]], the
-- control-center [[shortcut]], and the main [[panel]] are thin clients: they
-- read the published state and drive this service by writing to the plugin's
-- "command" state channel (or by IPC). A capture can be started even with no
-- UI entry placed.
--
-- On Windows, screen captures, the window picker and the color picker go through
-- Tourne's own picker over a frozen picture of the screens (tourne.screenPick).
-- Pixel work (OCR, QR, palette) runs over the picked file with `tesseract` /
-- `zbarimg` / `magick`; uploads (share, Google Lens) use Windows' built-in
-- `curl` and read the reply here. Markup opens the capture in an editor (Paint
-- by default, the "annotate-editor" setting for another).
--
-- Recording runs ffmpeg (Desktop Duplication through ddagrab, or GDI through
-- gdigrab), with the PC's sound from Tourne (tourne.systemSound) and a
-- microphone through DirectShow. The raw output lands in the plugin's data
-- folder as MP4; GIF and MOV are produced on save.

local PANEL_ID = "tourne/screen-toolkit:panel"
local LEGACY_PANEL_ID = "tourne/screen-toolkit:panel-legacy"
local RESULT_PANEL_ID = "tourne/screen-toolkit:result"

-- Windows: what Linux kept in /tmp lives in the plugin's data folder (cleaned at start).
local DATA_DIR = noctalia.pluginDataDir() or ""

local function dataPath(name)
  return DATA_DIR .. "\\" .. name
end

local COLOR_PNG = dataPath("screen-toolkit-colorpicker.png")
local RECORD_THUMB = dataPath("screen-toolkit-record-thumb.png")

local MAX_HISTORY = 8

-- ── Small helpers ──────────────────────────────────────────────────────────

local function cfg(key)
  return noctalia.getConfig(key)
end

local function log(msg)
  noctalia.log(`screen-toolkit: {msg}`)
end

local function trim(s)
  return (tostring(s):gsub("^%s+", ""):gsub("%s+$", ""))
end

-- Windows: Tourne Desktop puts a text/uri-list on the clipboard as the file itself
-- (it pastes into a folder, a chat or a mail), as Explorer's Copy does.
local function copyFileUri(path)
  local escaped = tostring(path):gsub(" ", "%%20"):gsub("'", "%%27"):gsub('"', "%%22")
  noctalia.copyToClipboard(`file://{escaped}`, "text/uri-list")
end

-- Windows: noctalia.runAsync answers false (and never calls back) when a program
-- isn't there; this calls back either way, so every pipeline below finishes.
local function run(argv, onResult, timeout)
  local ok = noctalia.runAsync(argv, onResult, timeout)
  if not ok and onResult then
    onResult({ exitCode = -1, stdout = "", stderr = "", timedOut = false })
  end
  return ok
end

-- For programs that may run past runAsync's one minute (uploads, GIF conversion):
-- runStream has no time limit. onDone(exitCode, everything it printed).
local function runLong(argv, onDone)
  local lines = {}
  local id = noctalia.runStream(argv, function(line)
    table.insert(lines, line)
  end, function(code)
    onDone(code, table.concat(lines, "\n"))
  end)
  if not id then onDone(-1, "") end
end

-- Programs some Windows installers leave off the PATH.
local KNOWN_PATHS = {
  tesseract = {
    "C:\\Program Files\\Tesseract-OCR\\tesseract.exe",
    "C:\\Program Files (x86)\\Tesseract-OCR\\tesseract.exe",
  },
  zbarimg = {
    "C:\\Program Files (x86)\\ZBar\\bin\\zbarimg.exe",
    "C:\\Program Files\\ZBar\\bin\\zbarimg.exe",
  },
}

-- The program to run for a name: on the PATH, or where its installer puts it; nil when missing.
local function program(name)
  if noctalia.commandExists(name) then return name end
  for _, p in ipairs(KNOWN_PATHS[name] or {}) do
    if noctalia.commandExists(p) then return p end
  end
  return nil
end

-- Whether the cursor is excluded from recordings. Screenshots are taken from
-- the picker's frozen picture; this flag only reaches ffmpeg (draw_mouse).
local cursorHidden = true
local CAPTURE_DELAY = 0.15

local function publish(key, value)
  noctalia.state.set(key, value)
end

-- Windows: `sleep` before a capture (so the just-closed panel isn't in it) is a
-- queue that update() runs.
local pending = {}

local function after(seconds, fn)
  table.insert(pending, { at = noctalia.nowMs() + seconds * 1000, fn = fn })
  noctalia.setUpdateInterval(100)
end

function update()
  local now = noctalia.nowMs()
  local due = {}
  for i = #pending, 1, -1 do
    if pending[i].at <= now then table.insert(due, 1, table.remove(pending, i)) end
  end
  for _, p in ipairs(due) do p.fn() end
  noctalia.setUpdateInterval(if #pending > 0 then 100 else 60000)
end

local function videoDir()
  local p = noctalia.expandPath(cfg("video-path") or "")
  if p == "" then p = noctalia.expandPath("~/Videos") end
  return p
end

local function screenshotDir()
  local p = noctalia.expandPath(cfg("screenshot-path") or "")
  if p == "" then p = noctalia.expandPath("~/Pictures/Screenshots") end
  return p
end

-- Characters Windows doesn't allow in a file name become "-".
local function safeName(s)
  return (tostring(s):gsub('[<>:"/\\|%?%*]', "-"))
end

local function buildFilename(tool, ext)
  local stem = noctalia.formatTime(cfg("filename-format") or "")
  if stem == nil or stem == "" then
    stem = tool .. "-" .. noctalia.formatTime("%Y-%m-%d_%H-%M-%S")
  end
  return safeName(stem) .. ext
end

-- Windows' own naming: "Screenshot 2026-10-13 141502.png", " (2)" when taken twice in a second.
local function screenshotFile()
  local dir = screenshotDir()
  noctalia.mkdirAll(dir)
  local stem = "Screenshot " .. noctalia.formatTime("%Y-%m-%d %H%M%S")
  local path = dir .. "\\" .. stem .. ".png"
  local n = 1
  while noctalia.fileExists(path) do
    n += 1
    path = dir .. "\\" .. stem .. " (" .. n .. ").png"
  end
  return path
end

-- ── Screen picker (tourne.screenPick) ──────────────────────────────────────

local picking = false
local ocrRunning = false
local ocrSerial = 0

-- One picker at a time. onResult(result) gets {x, y, w, h, path?, color?, connector};
-- a cancel (Esc or right click) is silent, like slurp's.
local function pick(options, onResult)
  if picking then
    noctalia.notify(noctalia.tr("panel.running"))
    return
  end
  if type(tourne) ~= "table" or type(tourne.screenPick) ~= "function" then
    noctalia.notifyError(noctalia.tr("messages.capture_failed"))
    return
  end
  picking = true
  local started, why = tourne.screenPick(options, function(res)
    picking = false
    if res == nil then return end -- cancelled
    onResult(res)
  end)
  if not started then
    picking = false
    noctalia.notifyError(noctalia.tr("messages.capture_failed"), why and tostring(why) or nil)
  end
end

-- Region *recording* only needs a rectangle (the picker's "area": nothing
-- captured). Waits a moment first so the just-closed panel's close animation
-- is not in the frozen picture. Invokes callback(rect) or nothing on cancel.
local function pickArea(callback)
  after(0.3, function()
    pick({ mode = "area" }, callback)
  end)
end

-- Region capture for the text-only tools (OCR/QR/palette/lens): the picture
-- goes to a temp file in the data folder, never the screenshots folder, so
-- these tools leave no screenshot behind. Markup keeps its file instead.
local tmpCaptureSerial = 0
local function captureTemp(onFile)
  after(0.3, function()
    tmpCaptureSerial = tmpCaptureSerial + 1
    local tmp = dataPath(string.format("st-capture-%d-%d.png", os.time(), tmpCaptureSerial))
    pick({ mode = "region", save = tmp }, function(res)
      local path = res.path or tmp
      if not noctalia.fileExists(path) then
        noctalia.notifyError(noctalia.tr("messages.no_file"))
        return
      end
      onFile(path, res)
    end)
  end)
end

-- A capture that is the deliverable (Markup): saved in the screenshots folder.
local function captureToScreenshots(mode, delay, onFile)
  after(delay, function()
    local path = screenshotFile()
    pick({ mode = mode, save = path }, function(res)
      local file = res.path or path
      if not noctalia.fileExists(file) then
        noctalia.notifyError(noctalia.tr("messages.no_file"))
        return
      end
      onFile(file)
    end)
  end)
end

-- Opens a file in the annotation editor (Paint unless the "annotate-editor"
-- setting names another). The capture's path goes last on the command line.
-- tourne.launch starts it as the taskbar would, so it stays open when the
-- plugin reloads or Tourne Desktop restarts.
local function editFile(path, onOk)
  local editor = trim(cfg("annotate-editor") or "")
  if editor == "" then editor = "mspaint" end
  local id = tourne.launch(editor .. ' "' .. path .. '"')
  if not id then
    -- That editor isn't there (or isn't one this plugin may run): the picture
    -- opens in its usual app instead.
    log(`editor '{editor}' didn't start; opening the picture in its usual app`)
    if not noctalia.openPath(path) then
      noctalia.notifyError(noctalia.tr("messages.no_annotator"))
      return
    end
  end
  if onOk then onOk(path) end
end

-- ── Color conversions ──────────────────────────────────────────────────────

local function clamp(n)
  return math.max(0, math.min(255, n))
end

local function rgbToHsv(r, g, b)
  local rn, gn, bn = r / 255, g / 255, b / 255
  local max, min = math.max(rn, gn, bn), math.min(rn, gn, bn)
  local d = max - min
  local h, sat, val = 0, 0, max
  if d ~= 0 then
    sat = d / max
    if max == rn then
      h = ((gn - bn) / d + (gn < bn and 6 or 0)) % 6
    elseif max == gn then
      h = (bn - rn) / d + 2
    else
      h = (rn - gn) / d + 4
    end
    h = math.round(h * 60)
  end
  return h, math.round(sat * 100), math.round(val * 100)
end

local function rgbToHsl(r, g, b)
  local rn, gn, bn = r / 255, g / 255, b / 255
  local max, min = math.max(rn, gn, bn), math.min(rn, gn, bn)
  local d = max - min
  local h, l = 0, (max + min) / 2
  local s = if d == 0 then 0 else d / (1 - math.abs(2 * l - 1))
  if d ~= 0 then
    if max == rn then
      h = ((gn - bn) / d + (gn < bn and 6 or 0)) % 6
    elseif max == gn then
      h = (bn - rn) / d + 2
    else
      h = (rn - gn) / d + 4
    end
    h = h / 6
  end
  return math.floor(h * 360 + 0.5), math.round(s * 100), math.round(l * 100)
end

-- ── Swatch PNG (Windows: written here, where Linux used magick) ────────────

local crcTable = nil

local function crc32(s)
  if not crcTable then
    crcTable = {}
    for i = 0, 255 do
      local c = i
      for _ = 1, 8 do
        if bit32.band(c, 1) ~= 0 then
          c = bit32.bxor(0xEDB88320, bit32.rshift(c, 1))
        else
          c = bit32.rshift(c, 1)
        end
      end
      crcTable[i] = c
    end
  end
  local c = 0xFFFFFFFF
  for i = 1, #s do
    c = bit32.bxor(crcTable[bit32.band(bit32.bxor(c, string.byte(s, i)), 0xFF)], bit32.rshift(c, 8))
  end
  return bit32.bxor(c, 0xFFFFFFFF)
end

local function u32(n)
  return string.char(bit32.band(bit32.rshift(n, 24), 0xFF), bit32.band(bit32.rshift(n, 16), 0xFF),
    bit32.band(bit32.rshift(n, 8), 0xFF), bit32.band(n, 0xFF))
end

local function pngChunk(kind, data)
  return u32(#data) .. kind .. data .. u32(crc32(kind .. data))
end

-- A size x size picture of one color: an RGB PNG whose zlib data is one stored
-- (uncompressed) block, which needs no compressor.
local function writeSwatch(path, r, g, b)
  local size = 21
  local raw = string.rep(string.char(0) .. string.rep(string.char(r, g, b), size), size)
  local a, s2 = 1, 0
  for i = 1, #raw do
    a = (a + string.byte(raw, i)) % 65521
    s2 = (s2 + a) % 65521
  end
  local len = #raw
  local nlen = bit32.bxor(len, 0xFFFF)
  local zlib = string.char(0x78, 0x01, 0x01, bit32.band(len, 0xFF), bit32.rshift(len, 8), bit32.band(nlen, 0xFF), bit32.rshift(nlen, 8))
    .. raw .. u32(s2 * 65536 + a)
  local png = "\137PNG\r\n\26\n"
    .. pngChunk("IHDR", u32(size) .. u32(size) .. string.char(8, 2, 0, 0, 0))
    .. pngChunk("IDAT", zlib)
    .. pngChunk("IEND", "")
  return noctalia.writeFile(path, png)
end

-- ── Persistence (survives restarts; plugin data dir is update-safe) ────────

local RESULTS_FILE = dataPath("results.json")
local HISTORY_FILE = dataPath("color-history.json")

local function loadState()
  local raw = noctalia.readFile(RESULTS_FILE)
  if raw then
    local d = noctalia.json.decode(raw)
    if type(d) == "table" then
      if d.activeTool then publish("activeTool", d.activeTool) end
      if d.colorResult then publish("colorResult", d.colorResult) end
      if d.ocrResult then publish("ocrResult", d.ocrResult) end
      if d.qrResult then publish("qrResult", d.qrResult) end
      if d.paletteColors then publish("paletteColors", d.paletteColors) end
      if d.paletteCapture then publish("paletteCapture", d.paletteCapture) end
    end
  end
  local hraw = noctalia.readFile(HISTORY_FILE)
  if hraw then
    local d = noctalia.json.decode(hraw)
    if type(d) == "table" then publish("colorHistory", d) end
  end
end

local function saveResults()
  local d = {
    activeTool = noctalia.state.get("activeTool"),
    colorResult = noctalia.state.get("colorResult"),
    ocrResult = noctalia.state.get("ocrResult"),
    qrResult = noctalia.state.get("qrResult"),
    paletteColors = noctalia.state.get("paletteColors"),
    paletteCapture = noctalia.state.get("paletteCapture"),
  }
  local enc = noctalia.json.encode(d)
  if enc then noctalia.writeFile(RESULTS_FILE, enc) end

  local h = noctalia.state.get("colorHistory") or {}
  local henc = noctalia.json.encode(h)
  if henc then noctalia.writeFile(HISTORY_FILE, henc) end
end

-- Windows: the data folder isn't emptied at boot like /tmp, so temp captures
-- the saved results no longer show, and work files left by an interrupted
-- recording or OCR, are removed when the service starts.
local function cleanTemp()
  local keep = {}
  local function hold(path)
    if type(path) == "string" and path ~= "" then keep[path:lower()] = true end
  end
  for _, key in ipairs({ "ocrResult", "qrResult" }) do
    local r = noctalia.state.get(key)
    if type(r) == "table" then hold(r.capturePath) end
  end
  hold(noctalia.state.get("paletteCapture"))
  for _, name in ipairs(noctalia.listDir(DATA_DIR) or {}) do
    local full = dataPath(name)
    local temp = name:match("^st%-capture%-.*%.png$") ~= nil
      or name:match("^screen%-toolkit%-record%-%d+%.mp4$") ~= nil
      or name:match("^screen%-toolkit%-record%-palette") ~= nil
      or name:match("^screen%-toolkit%-ocr%-work") ~= nil
    if temp and not keep[full:lower()] then noctalia.removeFile(full) end
  end
end

local function pushHistory(hex)
  hex = hex:upper()
  local history = noctalia.state.get("colorHistory") or {}
  for i, c in ipairs(history) do
    if c == hex then
      table.remove(history, i)
      break
    end
  end
  table.insert(history, 1, hex)
  while #history > MAX_HISTORY do
    table.remove(history)
  end
  publish("colorHistory", history)
end

-- ── Tools: color picker ────────────────────────────────────────────────────

local function runColorPicker()
  -- The picker's magnifier waits for a click on a pixel; Esc cancels.
  after(CAPTURE_DELAY, function()
    pick({ mode = "color" }, function(res)
      local r, g, b = tostring(res.color or ""):match("^#?(%x%x)(%x%x)(%x%x)")
      if not r then return end
      r, g, b = clamp(tonumber(r, 16)), clamp(tonumber(g, 16)), clamp(tonumber(b, 16))
      local hex = string.format("#%02X%02X%02X", r, g, b)
      local rgb = string.format("rgb(%d, %d, %d)", r, g, b)
      local h, s, v = rgbToHsv(r, g, b)
      local hh, ss, ll = rgbToHsl(r, g, b)
      local hsv = string.format("hsv(%d, %d%%, %d%%)", h, s, v)
      local hsl = string.format("hsl(%d, %d%%, %d%%)", hh, ss, ll)

      -- The preview swatch is a solid render of the picked color (no capture step).
      writeSwatch(COLOR_PNG, r, g, b)
      noctalia.copyToClipboard(hex, "text/plain;charset=utf-8")
      pushHistory(hex)
      publish("colorResult", {
        hex = hex, rgb = rgb, hsv = hsv, hsl = hsl,
        capturePath = COLOR_PNG, cacheBust = os.time(),
      })
      publish("activeTool", "colorpicker")
      saveResults()
      noctalia.togglePanel(RESULT_PANEL_ID)
    end)
  end)
end

-- ── Tools: OCR ─────────────────────────────────────────────────────────────

local OCR_EXIT_KEYS = {
  [2] = "ocr.exit_args",
  [4] = "ocr.exit_process",
}

local function countChars(text)
  local s = tostring(text):gsub("%s", "")
  return #s
end

-- The original's ocr.sh in Luau: upscale small captures, grayscale, normalize,
-- negate dark ones, denoise, then up to four tesseract passes, keeping the
-- longest text. onDone(code, out): 0 with the text, 1 with a missing program's
-- name, 2 bad file, 4 image processing failed.
local ocrWorkSerial = 0
local function ocrImage(file, gw, gh, rawLang, onDone)
  local magick, tess = program("magick"), program("tesseract")
  if not magick then onDone(1, "magick") return end
  if not tess then onDone(1, "tesseract") return end
  if not noctalia.fileExists(file) then onDone(2, "") return end

  ocrWorkSerial += 1
  local base = dataPath(string.format("screen-toolkit-ocr-work-%d-%d", os.time(), ocrWorkSerial))
  local tmp, tmpNoise, tmpThreshold = base .. ".pnm", base .. "-nr.pnm", base .. "-th.pnm"
  local function finish(code, out)
    for _, f in ipairs({ tmp, tmpNoise, tmpThreshold }) do
      if noctalia.fileExists(f) then noctalia.removeFile(f) end
    end
    onDone(code, out or "")
  end

  local wanted = {}
  for l in tostring(rawLang or "eng"):gmatch("[^+]+") do
    l = trim(l)
    if l ~= "" and l ~= "osd" then table.insert(wanted, l) end
  end
  if #wanted == 0 then wanted = { "eng" } end

  run({ tess, "--list-langs" }, function(listRes)
    local available = {}
    for line in tostring(listRes.stdout or ""):gmatch("[^\r\n]+") do
      if not line:find("^List of") then available[trim(line)] = true end
    end
    local valid = {}
    for _, l in ipairs(wanted) do
      if available[l] then table.insert(valid, l) end
    end
    local lang = if #valid > 0 then table.concat(valid, "+") else "eng"

    -- Upscale tiny captures and pick a page-segmentation mode from the geometry
    -- (ratio > 8 reads as a single line).
    local w, h = tonumber(gw) or 0, tonumber(gh) or 0
    local area = w * h
    local up = {}
    if h < 30 then
      up = { "-resize", "400%" }
    elseif area < 50000 or w < 200 then
      up = { "-resize", "200%" }
    end
    local ratio = w / (if h > 0 then h else 1)
    local psm
    if ratio > 8 then
      psm = 7
    elseif area < 60000 then
      psm = 6
    elseif h < 40 then
      psm = 7
    else
      psm = 3
    end
    if #up == 0 and w < 200 and w > 0 then
      up = { "-scale", tostring(math.floor(300 / w + 0.5)) .. "00%" }
    end

    local function tesseractPass(input, mode, cb)
      run({ tess, input, "stdout", "-l", lang, "--psm", tostring(mode), "--oem", "1" }, function(r)
        cb((tostring(r.stdout or ""):gsub("\r", "")))
      end)
    end

    local prep = { magick, file }
    for _, a in ipairs(up) do table.insert(prep, a) end
    for _, a in ipairs({ "-colorspace", "Gray", "-normalize", "-contrast-stretch", "2%x1%", "-sharpen", "0x1.5", "+repage", tmp }) do
      table.insert(prep, a)
    end
    run(prep, function(prepRes)
      if prepRes.exitCode ~= 0 then finish(4) return end
      run({ magick, tmp, "-format", "%[fx:mean]", "info:" }, function(meanRes)
        local mean = tonumber(trim(meanRes.stdout or ""))
        local function passes()
          tesseractPass(tmp, psm, function(text)
            local best, bestLen = text, countChars(text)
            local function lastPass()
              if bestLen >= 4 then finish(0, best) return end
              run({ magick, tmp, "-threshold", "85%", tmpThreshold }, function()
                tesseractPass(tmpThreshold, 11, function(t4)
                  if countChars(t4) > bestLen then best = t4 end
                  finish(0, best)
                end)
              end)
            end
            local function thirdPass()
              if bestLen >= 4 then lastPass() return end
              tesseractPass(tmpNoise, 4, function(t3)
                local l3 = countChars(t3)
                if l3 > bestLen then best, bestLen = t3, l3 end
                lastPass()
              end)
            end
            if bestLen < 4 or psm ~= 6 then
              tesseractPass(tmpNoise, 6, function(t2)
                local l2 = countChars(t2)
                if l2 > bestLen then best, bestLen = t2, l2 end
                thirdPass()
              end)
            else
              thirdPass()
            end
          end)
        end
        local function denoise()
          run({ magick, tmp, "-median", "1", tmpNoise }, function() passes() end)
        end
        if mean and mean < 0.4 then
          run({ magick, tmp, "-negate", tmp }, function() denoise() end)
        else
          denoise()
        end
      end)
    end)
  end)
end

local function runOcr()
  if ocrRunning then
    noctalia.notify(noctalia.tr("panel.running"))
    return
  end
  for _, dep in ipairs({ "magick", "tesseract" }) do
    if not program(dep) then
      noctalia.notifyError(noctalia.tr("messages.missing_dep", { dep = dep }))
      return
    end
  end
  captureTemp(function(path, capture)
    ocrRunning = true
    ocrSerial = ocrSerial + 1
    local serial = ocrSerial
    local lang = cfg("selected-ocr-lang") or "eng"
    local dw, dh = tonumber(capture.w) or 0, tonumber(capture.h) or 0
    ocrImage(path, dw, dh, lang, function(code, out)
      ocrRunning = false
      if code == 1 then
        noctalia.notifyError(noctalia.tr("messages.missing_dep", { dep = trim(out or "") }))
        return
      elseif code ~= 0 then
        noctalia.notifyError(noctalia.tr(OCR_EXIT_KEYS[code] or "ocr.exit_unknown", { code = tostring(code) }))
        return
      end
      local text = trim(out or "")
      if text == "" then
        noctalia.notifyError(noctalia.tr("messages.no_text"))
        return
      end
      noctalia.copyToClipboard(text, "text/plain;charset=utf-8")
      publish("ocrResult", {
        text = text,
        capturePath = path,
        cacheBust = tostring(os.time()) .. "-" .. tostring(serial),
        gw = math.floor(dw),
        gh = math.floor(dh),
      })
      publish("translateResult", nil)
      publish("activeTool", "ocr")
      saveResults()
      noctalia.togglePanel(RESULT_PANEL_ID)
    end)
  end)
end

local function ocrSearch(payload)
  local result = noctalia.state.get("ocrResult")
  local text = if type(payload) == "table" and type(payload.text) == "string"
    then payload.text
    else type(result) == "table" and result.text or nil
  if not text or text == "" then return end
  local url = cfg("search-engine-url")
  if url == nil or url == "" then
    url = "https://www.google.com/search?q="
  end
  local encoded = noctalia.string.urlEncode(text)
  if url:find("{text}", 1, true) then
    url = url:gsub("{text}", function() return encoded end)
  else
    url = url .. encoded
  end
  noctalia.openPath(url)
end

local function ocrTranslate(payload)
  local result = noctalia.state.get("ocrResult")
  local text = if type(payload) == "table" and type(payload.text) == "string"
    then payload.text
    else type(result) == "table" and result.text or nil
  if not text or text == "" then return end
  local targetLang = if type(payload) == "table" then payload.lang else payload
  local lang = if type(targetLang) == "string" and targetLang ~= "" then targetLang else "en"
  if not noctalia.commandExists("trans") then
    -- Windows: translate-shell is rarely there; the text opens in Google Translate instead.
    noctalia.openPath("https://translate.google.com/?sl=auto&tl=" .. noctalia.string.urlEncode(lang)
      .. "&text=" .. noctalia.string.urlEncode(text) .. "&op=translate")
    return
  end
  publish("translateResult", nil)
  run({ "trans", "-brief", "-no-bidi", "-to", lang, text }, function(res)
    if res.exitCode ~= 0 then
      publish("translateResult", noctalia.tr("messages.translate_failed"))
      return
    end
    -- trans bidi conversion outputs presentation-forms + terminal padding (visual order);
    -- -no-bidi gives logical order (U+0600) for shaping. Collapse stray newlines.
    local out = trim(res.stdout or "")
    out = out:gsub("%s+", " ")
    -- strip BOM FEFF that trans emits in bidi mode
    out = out:gsub("^\239\187\191", "")
    publish("translateResult", out)
  end)
end

-- ── Uploads (Windows' curl; the reply is read here, where Linux used jq) ───

-- curl's -F "name=@file": forward slashes need no escaping, and quotes keep a
-- comma or semicolon in a path from splitting it.
local function formFile(path)
  local p = tostring(path):gsub("\\", "/")
  if p:find("[,;]") then p = '"' .. p .. '"' end
  return "@" .. p
end

local function uguuUrl(text)
  local body = tostring(text or ""):match("{.*}")
  local d = body and noctalia.json.decode(body)
  local f = type(d) == "table" and type(d.files) == "table" and d.files[1]
  local url = type(f) == "table" and f.url
  if type(url) == "string" and url:sub(1, 4) == "http" then return url end
  return nil
end

-- uguu.se, anonymous: /upload first, /upload.php when that fails.
-- onDone(url or nil, requestFailed).
local function uguuUpload(file, onDone)
  local function args(endpoint, strict)
    local a = { "curl", "-sS" }
    if strict then table.insert(a, "-f") end
    for _, v in ipairs({ "-A", "Mozilla/5.0", "--connect-timeout", "20", "--max-time", "60", "-F", "files[]=" .. formFile(file), endpoint }) do
      table.insert(a, v)
    end
    return a
  end
  runLong(args("https://uguu.se/upload", true), function(code, text)
    if code == 0 then
      onDone(uguuUrl(text), false)
      return
    end
    runLong(args("https://uguu.se/upload.php", false), function(code2, text2)
      if code2 ~= 0 then
        onDone(nil, true)
        return
      end
      onDone(uguuUrl(text2), false)
    end)
  end)
end

-- ── Tools: QR / palette / lens ─────────────────────────────────────────────

local function runQr()
  local zbar = program("zbarimg")
  if not zbar then
    noctalia.notifyError(noctalia.tr("messages.missing_dep", { dep = "zbarimg" }))
    return
  end
  captureTemp(function(path)
    run({ zbar, "-q", "--raw", path }, function(res)
      local text = trim((tostring(res.stdout or ""):gsub("\r\n", "\n")))
      if res.exitCode ~= 0 or text == "" then
        noctalia.notifyError(noctalia.tr("messages.no_qr"))
        return
      end
      noctalia.copyToClipboard(text, "text/plain;charset=utf-8")
      publish("qrResult", { text = text, capturePath = path })
      publish("activeTool", "qr")
      saveResults()
      noctalia.togglePanel(RESULT_PANEL_ID)
    end)
  end)
end

local function runPalette()
  local magick = program("magick")
  if not magick then
    noctalia.notifyError(noctalia.tr("messages.missing_dep", { dep = "magick" }))
    return
  end
  captureTemp(function(path)
    -- Up to 8 dominant colours: ImageMagick's txt: listing, one "#RRGGBB" per line.
    run({ magick, path, "-alpha", "off", "+dither", "-colors", "8", "-unique-colors", "-depth", "8", "txt:-" }, function(res)
      if res.exitCode ~= 0 then
        noctalia.notifyError(noctalia.tr("messages.palette_failed"))
        return
      end
      local colors = {}
      for line in (res.stdout or ""):gmatch("[^\r\n]+") do
        if line:sub(1, 1) ~= "#" then
          local c = line:match("#%x%x%x%x%x%x")
          if c and #colors < 8 then colors[#colors + 1] = c:upper() end
        end
      end
      local seen, out = {}, {}
      for _, c in ipairs(colors) do
        if not seen[c] then
          seen[c] = true
          out[#out + 1] = c
        end
      end
      if #out == 0 then
        noctalia.notifyError(noctalia.tr("messages.palette_failed"))
        return
      end
      noctalia.copyToClipboard(table.concat(out, "\n"), "text/plain;charset=utf-8")
      publish("paletteColors", out)
      publish("paletteCapture", path)
      publish("activeTool", "palette")
      saveResults()
      noctalia.togglePanel(RESULT_PANEL_ID)
    end)
  end)
end

local function runLens()
  if not noctalia.commandExists("curl") then
    noctalia.notifyError(noctalia.tr("messages.missing_dep", { dep = "curl" }))
    return
  end
  captureTemp(function(path)
    uguuUpload(path, function(url)
      if url then
        noctalia.openPath("https://lens.google.com/uploadbyurl?url=" .. url)
      else
        noctalia.notifyError(noctalia.tr("messages.lens_failed"))
      end
    end)
  end)
end

-- ── Tools: measure ─────────────────────────────────────────────────────────

-- The picker hands back the rectangle itself ("area": nothing captured).
local function runMeasure()
  pickArea(function(res)
    local gw, gh = tonumber(res.w), tonumber(res.h)
    if not gw or not gh then
      noctalia.notifyError(noctalia.tr("messages.measure_failed"))
      return
    end
    gw, gh = math.floor(gw), math.floor(gh)
    local text = string.format("%d x %d px", gw, gh)
    noctalia.copyToClipboard(text, "text/plain;charset=utf-8")
    publish("measureResult", { width = gw, height = gh, text = text })
    noctalia.notify(noctalia.tr("messages.measure_result", { width = tostring(gw), height = tostring(gh) }))
  end)
end

-- ── Tools: annotate (editor handoff) ───────────────────────────────────────

local function afterAnnotate(path)
  publish("activeTool", "annotate")
  publish("annotateResult", { capturePath = path })
  saveResults()
end

local function annotateRegion()
  captureToScreenshots("region", 0.3, function(path) editFile(path, afterAnnotate) end)
end

local function annotateFullscreen()
  captureToScreenshots("screen", 0.4, function(path) editFile(path, afterAnnotate) end)
end

-- Hover to aim (everything but the window under the pointer dims), click to capture it.
local function annotateWindow()
  captureToScreenshots("window", 0.3, function(path) editFile(path, afterAnnotate) end)
end

-- ── Recording ──────────────────────────────────────────────────────────────

-- Windows: one backend, ffmpeg, for both fullscreen and region capture.
local ffmpegAvailable = false
local lastRecorderLog = ""

local recordState = "idle" -- idle | recording | converting | ready
local recordFormat = "mp4"
local rawPath = ""
local recordPath = ""
local recordStartedAt = nil
local recordStream = nil -- ffmpeg's runStream handle while it records
local recordLines = {}

local function refreshRecorders()
  ffmpegAvailable = noctalia.commandExists("ffmpeg")
  local key = string.format("ffmpeg=%s", tostring(ffmpegAvailable))
  if key ~= lastRecorderLog then
    lastRecorderLog = key
    log("recorders: " .. key)
  end
end

local function bestFullscreenRecorder()
  if ffmpegAvailable then return "ffmpeg" end
  return ""
end

local function bestRegionRecorder()
  if ffmpegAvailable then return "ffmpeg" end
  return ""
end

-- Tourne Desktop's Print Screen key (capture.cpp) hands recordings to this service and keeps pictures in its
-- screenshot folder; it's told the state so the key (or its red dot) can stop a recording.
local function linkCapture(state)
  if tourne and tourne.captureLink then tourne.captureLink(state or recordState, screenshotDir()) end
end

-- Print Screen's recording (recordWith): Tourne Desktop's Capture settings for it, in place of this plugin's own;
-- nil for the panel's recordings.
local recOpts = nil

local function setRecordState(next)
  if recordState == next then return end
  recordState = next
  if next == "idle" then recOpts = nil end  -- (Print Screen's settings were for that one recording)
  linkCapture()
  publish("recordState", recordState)
  publish("recording", recordState == "recording")
  publish("recordPath", recordPath)
  publish("recordFormat", recordFormat)
  publish("recordStartedAt", if recordState == "recording" then recordStartedAt else nil)
end

local overrideAudioOut = nil
local overrideAudioIn = nil

local function opt(name, fallback)
  if recOpts and recOpts[name] ~= nil then return recOpts[name] end
  return fallback
end

local function getAudioOut()
  if recOpts then return recOpts.sound == true end
  if overrideAudioOut ~= nil then return overrideAudioOut end
  return cfg("record-audio-out") == true
end

local function getAudioIn()
  if recOpts then return recOpts.mic == true end
  if overrideAudioIn ~= nil then return overrideAudioIn end
  return cfg("record-audio-in") == true
end

-- The first audio device in ffmpeg's DirectShow list (both ffmpeg's newer
-- `"name" (audio)` lines and the older "DirectShow audio devices" section).
local function firstAudioDevice(text)
  local section = nil
  for line in tostring(text):gmatch("[^\r\n]+") do
    if line:find("DirectShow audio devices", 1, true) then
      section = "audio"
    elseif line:find("DirectShow video devices", 1, true) then
      section = "video"
    end
    if not line:find("Alternative name", 1, true) then
      local name, kind = line:match('"([^"]+)"%s*%((%a+)%)')
      if name and kind == "audio" then return name end
      if not name and section == "audio" then
        local n = line:match('"([^"]+)"')
        if n then return n end
      end
    end
  end
  return nil
end

-- The microphone for ffmpeg's dshow input: the "record-mic-device" setting, or
-- the first one ffmpeg finds (Linux asked pactl for the default source).
local function resolveMicrophone(callback)
  local named = trim(opt("micDevice", nil) or cfg("record-mic-device") or "")
  if named ~= "" then
    callback(named)
    return
  end
  run({ "ffmpeg", "-hide_banner", "-list_devices", "true", "-f", "dshow", "-i", "dummy" }, function(res)
    callback(firstAudioDevice(tostring(res.stderr or "") .. "\n" .. tostring(res.stdout or "")))
  end, 15000)
end

local function outputNamed(name)
  if type(name) ~= "string" or name == "" then return nil end
  for _, o in ipairs(noctalia.outputs() or {}) do
    if string.upper(o.name) == string.upper(name) then return o end
  end
  return nil
end

local function primaryOutput()
  local outs = noctalia.outputs() or {}
  for _, o in ipairs(outs) do
    if o.primary then return o end
  end
  return outs[1]
end

local function outputAt(x, y)
  for _, o in ipairs(noctalia.outputs() or {}) do
    if x >= o.x and x < o.x + o.width and y >= o.y and y < o.y + o.height then return o end
  end
  return primaryOutput()
end

-- The screen with the focused window (Linux: the compositor's focused output).
local function focusedOutput()
  return outputNamed(noctalia.focusedOutputName()) or primaryOutput()
end

-- record-codec x record-encoder -> ffmpeg's encoder.
local ENCODERS = {
  cpu = { h264 = "libx264", hevc = "libx265", av1 = "libsvtav1" },
  nvidia = { h264 = "h264_nvenc", hevc = "hevc_nvenc", av1 = "av1_nvenc" },
  amd = { h264 = "h264_amf", hevc = "hevc_amf", av1 = "av1_amf" },
  intel = { h264 = "h264_qsv", hevc = "hevc_qsv", av1 = "av1_qsv" },
}

-- ffmpeg's arguments for a recording. geom: "screen" (the focused screen) or
-- the picker's rectangle {x, y, w, h, connector}. sound: tourne.systemSound's
-- answer, or nil. mic: a DirectShow device name, or nil.
local function buildRecorderArgv(geom, out, sound, mic)
  local fps = tostring(opt("fps", cfg("record-fps") or 60))
  local codec = opt("codec", cfg("record-codec") or "h264")
  local family = ENCODERS[opt("encoder", cfg("record-encoder") or "cpu")] or ENCODERS.cpu
  local q = tostring(math.floor(tonumber(opt("quality", 25)) or 25))
  local height = math.floor(tonumber(opt("height", 0)) or 0)
  local enc = family[codec] or family.h264
  local mouse = if cursorHidden then "0" else "1"
  if recOpts and recOpts.cursor ~= nil then mouse = if recOpts.cursor then "1" else "0" end

  local x, y, w, h, o
  local hwnd = if type(geom) == "table" and type(geom.hwnd) == "string" then geom.hwnd else nil
  if type(geom) == "table" then
    x, y = math.floor(tonumber(geom.x) or 0), math.floor(tonumber(geom.y) or 0)
    w, h = math.floor(tonumber(geom.w) or 0), math.floor(tonumber(geom.h) or 0)
    o = outputNamed(geom.connector) or outputAt(x + w / 2, y + h / 2)
  else
    o = focusedOutput()
    if not o then return {} end
    x, y, w, h = o.x, o.y, o.width, o.height
  end
  -- Encoders want even sizes.
  w -= w % 2
  h -= h % 2
  if w < 2 or h < 2 then return {} end

  local argv = { "ffmpeg", "-hide_banner", "-loglevel", "error", "-nostats", "-y" }
  local function add(...)
    for _, v in ipairs({ ... }) do table.insert(argv, v) end
  end

  -- Desktop Duplication when the rectangle is on one screen of the first
  -- graphics card (it crops on the card), else GDI with offsets.
  local inside = o ~= nil and x >= o.x and y >= o.y and x + w <= o.x + o.width and y + h <= o.y + o.height
  local index = nil
  local method = opt("method", cfg("record-capture"))
  if method ~= "gdi" and inside and not hwnd then index = tourne.screenIndex(o.name) end
  if hwnd then
    -- A window (Print Screen's "Window"): GDI reads the window itself, so it's followed when it moves.
    -- GDI can't keep up past 60 fps.
    add("-f", "gdigrab", "-framerate", tostring(math.min(tonumber(fps) or 60, 60)), "-draw_mouse", mouse, "-i", "hwnd=" .. hwnd)
  elseif index then
    local src = string.format("ddagrab=output_idx=%d:framerate=%s:draw_mouse=%s", index, fps, mouse)
    if w ~= o.width or h ~= o.height then
      src ..= string.format(":video_size=%dx%d:offset_x=%d:offset_y=%d", w, h, x - o.x, y - o.y)
    end
    add("-f", "lavfi", "-i", src)
  else
    add("-f", "gdigrab", "-framerate", fps, "-draw_mouse", mouse,
      "-offset_x", tostring(x), "-offset_y", tostring(y), "-video_size", string.format("%dx%d", w, h), "-i", "desktop")
  end

  local inputs = 0
  if sound then
    add("-thread_queue_size", "1024", "-f", sound.format, "-ar", tostring(sound.rate), "-ac", tostring(sound.channels), "-i", sound.input)
    inputs += 1
  end
  if mic and mic ~= "" then
    add("-thread_queue_size", "1024", "-f", "dshow", "-i", "audio=" .. mic)
    inputs += 1
  end
  if inputs == 2 then
    add("-filter_complex", "[1:a][2:a]amix=inputs=2:duration=longest:normalize=0[a]", "-map", "0:v", "-map", "[a]")
  elseif inputs == 1 then
    add("-map", "0:v", "-map", "1:a")
  end

  -- ddagrab's frames stay on the graphics card: NVIDIA's encoders take them as
  -- they are; everything else gets them copied back first.
  -- (one filter chain: the frames copied back from the card when the encoder or a scale needs them, scaled down,
  -- a window's cut to even sizes)
  local scale = height > 0 and height < h
  local vf = {}
  if index and (scale or not string.find(enc, "nvenc", 1, true)) then table.insert(vf, "hwdownload,format=bgra") end
  if scale then table.insert(vf, `scale=-2:{height}:flags=lanczos`) end
  if hwnd then table.insert(vf, "crop=trunc(iw/2)*2:trunc(ih/2)*2") end
  if #vf > 0 then add("-vf", table.concat(vf, ",")) end
  add("-c:v", enc)
  if enc == "libx264" or enc == "libx265" then
    add("-preset", "veryfast", "-crf", q, "-pix_fmt", "yuv420p")
  elseif enc == "libsvtav1" then
    add("-preset", "10", "-crf", tostring(math.floor(tonumber(q) * 1.3)), "-pix_fmt", "yuv420p")
  elseif string.find(enc, "nvenc", 1, true) then
    add("-preset", "p4", "-rc", "vbr", "-cq", q, "-b:v", "0")
  elseif string.find(enc, "amf", 1, true) then
    add("-quality", "balanced", "-rc", "cqp", "-qp_i", q, "-qp_p", q)
  elseif string.find(enc, "qsv", 1, true) then
    add("-global_quality", q)
  end
  if codec == "hevc" then add("-tag:v", "hvc1") end
  if inputs > 0 then add("-c:a", "aac", "-b:a", "160k") end
  table.insert(argv, out)
  return argv
end

local function parseRecordInfo(raw)
  local d, wh, size = tostring(raw):match("([%d%.]+)|(%d+x%d+)|(%d+)")
  local w, h = 0, 0
  if wh then
    local ww, hh = wh:match("(%d+)x(%d+)")
    w, h = tonumber(ww) or 0, tonumber(hh) or 0
  end
  return {
    duration = tonumber(d) or 0,
    width = w,
    height = h,
    size = tonumber(size) or 0,
  }
end

-- A mid-frame thumbnail for the result panel (the original's record.sh thumb).
-- onDone(duration as ffprobe printed it).
local function makeThumb(src, onDone)
  run({ "ffprobe", "-v", "error", "-show_entries", "format=duration", "-of", "default=noprint_wrappers=1:nokey=1", src }, function(durRes)
    local dur = trim(durRes.stdout or "")
    local seconds = tonumber(dur) or 1
    run({ "ffmpeg", "-hide_banner", "-loglevel", "error", "-y", "-ss", string.format("%.3f", seconds / 2), "-i", src, "-frames:v", "1", RECORD_THUMB }, function()
      if onDone then onDone(dur) end
    end)
  end)
end

-- Print Screen's "After it's copied": a second file beside the recording, made from it. onDone(path or nil).
local function afterRecording(src, what, copy, nvidia)
  if what == nil or what == "" or what == "none" then return end
  local stem = src:gsub("%.%w+$", "")
  local function finish(out, code)
    if code ~= 0 or not noctalia.fileExists(out) then
      noctalia.notifyError(noctalia.tr("messages.recording_failed"), "the smaller copy didn't come out")
      return
    end
    local info = noctalia.fileInfo(out)
    local mb = string.format("%.1f MB", ((info and info.size) or 0) / 1048576)
    if copy then copyFileUri(out) end
    noctalia.notify(if copy then "Smaller copy on the clipboard" else "Smaller copy made", `{out} ({mb})`)
  end
  local function ffmpeg(args, out)
    local argv = { "ffmpeg", "-hide_banner", "-loglevel", "error", "-y", "-i", src }
    for _, a in ipairs(args) do table.insert(argv, a) end
    table.insert(argv, out)
    log(`after: {table.concat(argv, " ")}`)
    runLong(argv, function(code) finish(out, code) end)
  end
  if what == "av1" then
    local v = if nvidia then { "-c:v", "av1_nvenc", "-preset", "p6", "-rc", "vbr", "-cq", "34", "-b:v", "0" }
      else { "-c:v", "libsvtav1", "-preset", "6", "-crf", "36" }
    for _, a in ipairs({ "-c:a", "aac", "-b:a", "128k", "-movflags", "+faststart" }) do table.insert(v, a) end
    ffmpeg(v, stem .. " (AV1).mp4")
  elseif what == "hevc" then
    local v = if nvidia then { "-c:v", "hevc_nvenc", "-preset", "p6", "-rc", "vbr", "-cq", "30", "-b:v", "0" }
      else { "-c:v", "libx265", "-preset", "medium", "-crf", "28" }
    for _, a in ipairs({ "-tag:v", "hvc1", "-c:a", "aac", "-b:a", "128k", "-movflags", "+faststart" }) do table.insert(v, a) end
    ffmpeg(v, stem .. " (HEVC).mp4")
  elseif what == "gif" then
    ffmpeg({ "-vf", "fps=15,scale='min(720,iw)':-2:flags=lanczos,split[a][b];[a]palettegen=stats_mode=diff[p];[b][p]paletteuse=dither=bayer:bayer_scale=5" },
      stem .. ".gif")
  elseif string.sub(what, 1, 7) == "discord" then
    -- Sized to fit: the bitrate the length allows under the limit (H.264, which Discord shows in the chat).
    local limitMb = tonumber(string.sub(what, 8)) or 10
    run({ "ffprobe", "-v", "error", "-show_entries", "format=duration", "-of", "default=noprint_wrappers=1:nokey=1", src }, function(res)
      local seconds = tonumber(trim(res.stdout or "")) or 0
      if seconds <= 0 then finish("", -1) return end
      local kbps = math.floor(limitMb * 8192 * 0.92 / seconds) - 96
      if kbps < 150 then
        noctalia.notifyError("Too long to fit", `{math.floor(seconds)} s won't fit under {limitMb} MB`)
        return
      end
      local enc = if nvidia then { "-c:v", "h264_nvenc", "-preset", "p6", "-rc", "cbr" } else { "-c:v", "libx264", "-preset", "medium" }
      local v = { "-b:v", `{kbps}k`, "-maxrate", `{kbps}k`, "-bufsize", `{kbps * 2}k`, "-pix_fmt", "yuv420p",
        "-c:a", "aac", "-b:a", "96k", "-movflags", "+faststart" }
      if kbps < 2500 then table.insert(v, 1, "scale='min(1280,iw)':-2"); table.insert(v, 1, "-vf") end
      for i = #enc, 1, -1 do table.insert(v, 1, enc[i]) end
      ffmpeg(v, stem .. ` (under {limitMb} MB).mp4`)
    end, 15000)
  end
end

local function finalizeRecord(format, copyToClipboard)
  if rawPath == "" then return end
  local destDir = opt("folder", nil) or videoDir()
  local after, afterCopy = opt("after", "none"), opt("afterCopy", true) == true
  local nvidia = string.find(tostring(opt("encoder", "")), "nvidia", 1, true) ~= nil
  noctalia.mkdirAll(destDir)
  local ext
  if format == "gif" then ext = ".gif" elseif format == "mov" then ext = ".mov" else ext = ".mp4" end
  local dest = destDir .. "\\" .. buildFilename("record", ext)
  setRecordState("converting")
  local src = rawPath
  local function finished(ok)
    if not ok then
      setRecordState("idle")
      noctalia.notifyError(noctalia.tr("messages.recording_failed"))
      return
    end
    makeThumb(dest, function()
      recordPath = dest
      rawPath = ""
      setRecordState("idle")
      if copyToClipboard then
        copyFileUri(dest)
        noctalia.notify(noctalia.tr("panel.copied_to_clipboard"), dest)
      else
        noctalia.notify(noctalia.tr("messages.saved_to", { path = dest }))
      end
      recOpts = nil
      afterRecording(dest, after, afterCopy, nvidia)
    end)
  end
  local quiet = { "ffmpeg", "-hide_banner", "-loglevel", "error", "-y" }
  local function argv(...)
    local a = table.clone(quiet)
    for _, v in ipairs({ ... }) do table.insert(a, v) end
    return a
  end
  if format == "gif" then
    -- Palette-optimized GIF at 15 fps, capped at gif-max-seconds.
    local maxsec = math.floor(tonumber(cfg("gif-max-seconds")) or 0)
    local palette = dataPath(`screen-toolkit-record-palette-{os.time()}.png`)
    local first = table.clone(quiet)
    local second = table.clone(quiet)
    if maxsec > 0 then
      for _, a in ipairs({ "-t", tostring(maxsec) }) do
        table.insert(first, a)
        table.insert(second, a)
      end
    end
    for _, a in ipairs({ "-i", src, "-vf", "fps=15,scale=trunc(iw/2)*2:trunc(ih/2)*2:flags=bilinear,palettegen=stats_mode=diff", palette }) do
      table.insert(first, a)
    end
    for _, a in ipairs({ "-i", src, "-i", palette, "-lavfi",
      "fps=15,scale=trunc(iw/2)*2:trunc(ih/2)*2:flags=bilinear[x];[x][1:v]paletteuse=dither=bayer:bayer_scale=5", dest }) do
      table.insert(second, a)
    end
    runLong(first, function(c1)
      if c1 ~= 0 then finished(false) return end
      runLong(second, function(c2)
        if noctalia.fileExists(palette) then noctalia.removeFile(palette) end
        if c2 ~= 0 then finished(false) return end
        noctalia.removeFile(src)
        finished(true)
      end)
    end)
  elseif format == "mov" then
    -- Remux MP4 -> MOV (stream copy).
    runLong(argv("-i", src, "-c", "copy", "-movflags", "+faststart", dest), function(code)
      if code ~= 0 then finished(false) return end
      noctalia.removeFile(src)
      finished(true)
    end)
  elseif noctalia.renameFile(src, dest) then
    finished(true)
  else
    -- Another drive: ffmpeg copies the streams over.
    runLong(argv("-i", src, "-c", "copy", dest), function(code)
      if code ~= 0 then finished(false) return end
      noctalia.removeFile(src)
      finished(true)
    end)
  end
end

-- ffmpeg has finished the file: save it, or preview it in the result panel.
local function afterStop()
  local info = rawPath ~= "" and noctalia.fileInfo(rawPath) or nil
  if not info or (info.size or 0) <= 0 then
    rawPath = ""
    setRecordState("idle")
    noctalia.notifyError(noctalia.tr("messages.recording_failed"), table.concat(recordLines, "\n"))
    return
  end
  local skipConfirm = cfg("record-skip-confirmation") == true
  local toClipboard = recOpts ~= nil or cfg("record-copy-to-clipboard") == true
  if toClipboard then
    finalizeRecord(recordFormat, true)
  elseif skipConfirm then
    finalizeRecord(recordFormat, false)
  else
    -- Extract a mid-frame thumbnail so the result panel can preview the
    -- recording, then probe it for size / duration / resolution.
    makeThumb(rawPath, function(duration)
      run({ "ffprobe", "-v", "error", "-select_streams", "v:0", "-show_entries", "stream=width,height", "-of", "csv=s=x:p=0", rawPath }, function(whRes)
        local wh = trim(whRes.stdout or "")
        local fi = noctalia.fileInfo(rawPath)
        local size = string.format("%d", fi and fi.size or 0)
        publish("recordInfo", parseRecordInfo(`{duration}|{wh}|{size}`))
        setRecordState("ready")
        noctalia.togglePanel(RESULT_PANEL_ID)
      end)
    end)
  end
end

local function recorderExited(code)
  recordStream = nil
  if recordState == "recording" then
    -- ffmpeg ended by itself (a screen went away, an encoder it lacks...).
    if code ~= 0 then
      if rawPath ~= "" and noctalia.fileExists(rawPath) then noctalia.removeFile(rawPath) end
      rawPath = ""
      setRecordState("idle")
      noctalia.notifyError(noctalia.tr("messages.recording_failed"), table.concat(recordLines, "\n"))
      return
    end
    setRecordState("converting")
  end
  if recordState == "converting" then afterStop() end
end

local function launchRecord(geom, format, mic)
  rawPath = dataPath(`screen-toolkit-record-{os.time()}.mp4`)
  recordFormat = if format == "gif" then "gif" else "mp4"
  publish("recordFormat", recordFormat)
  recordPath = ""
  -- The PC's sound waits in a pipe for ffmpeg, so it's asked for just before ffmpeg starts.
  local sound = nil
  if getAudioOut() then
    local got, problem = tourne.systemSound(opt("soundDevice", "default"))
    if got then
      sound = got
    else
      log(`no system sound: {tostring(problem)}`)
      noctalia.notifyError(noctalia.tr("messages.no_system_sound", { why = tostring(problem) }))
    end
  end
  local argv = buildRecorderArgv(geom, rawPath, sound, mic)
  log(`starting recording: {table.concat(argv, " ")}`)
  if #argv == 0 then
    rawPath = ""
    setRecordState("idle")
    noctalia.notifyError(noctalia.tr("messages.recording_failed"))
    return
  end
  recordLines = {}
  -- runStream keeps ffmpeg running (no time limit) and lets the stop path ask
  -- it to finish the file ("q" on its input), where Linux used pkill -INT.
  local id = noctalia.runStream(argv, function(line)
    table.insert(recordLines, line)
    if #recordLines > 6 then table.remove(recordLines, 1) end
  end, recorderExited)
  if not id then
    rawPath = ""
    setRecordState("idle")
    noctalia.notifyError(noctalia.tr("messages.recording_failed"))
    return
  end
  recordStream = id
  recordStartedAt = os.time()
  setRecordState("recording")
  noctalia.notify(noctalia.tr("plugin_name"), noctalia.tr("messages.recording_started"))
end

local startingRecord = false

local function startRecord(geom, format, bin)
  refreshRecorders()
  if bin == "" then
    noctalia.notifyError(noctalia.tr("messages.no_recorder"))
    return
  end
  if recordState ~= "idle" or startingRecord then return end
  if getAudioIn() then
    startingRecord = true
    resolveMicrophone(function(device)
      startingRecord = false
      if device == nil then log("microphone lookup failed; recording without one") end
      launchRecord(geom, format, device)
    end)
    return
  end
  launchRecord(geom, format, nil)
end

local function recordStop()
  if recordState ~= "recording" then return end
  setRecordState("converting")
  noctalia.notify(noctalia.tr("plugin_name"), noctalia.tr("messages.recording_stopped"))
  if recordStream then
    -- ffmpeg finishes the file and quits; it's ended after 8 s if it doesn't.
    -- recorderExited carries on from there.
    tourne.write(recordStream, "q")
    tourne.stop(recordStream, 8000)
  else
    afterStop()
  end
end

local function recordDiscard()
  if rawPath ~= "" then
    if noctalia.fileExists(rawPath) then noctalia.removeFile(rawPath) end
    rawPath = ""
  end
  setRecordState("idle")
end

-- A recording that finished but was never saved is superseded the moment the
-- user runs any other capture tool; drop its temp file so the result panel
-- can't get stuck showing the stale "save recording" card.
local function discardPendingRecording()
  if recordState ~= "ready" then return end
  recordDiscard()
end

local SHARE_EXIT_KEYS = {
  [1] = "messages.share_bad_args",
  [2] = "messages.share_file_not_found",
  [3] = "messages.share_missing_dep",
  [4] = "messages.share_request_failed",
  [5] = "messages.share_invalid_response",
  [6] = "messages.share_file_too_large",
}

-- ── Sharing ────────────────────────────────────────────────────────────────

local UGUU_MAX_BYTES = 128 * 1024 * 1024

-- The original's share-upload.sh: up.x02.me with an API key, else uguu.se
-- (anonymous, 3h, 128 MB max). onDone(exitCode, url) with the script's codes.
local function shareUpload(file, apiKey, expiry, onDone)
  if file == "" then onDone(1) return end
  local info, why = noctalia.fileInfo(file)
  if not info and why == "not found" then onDone(2) return end
  if not noctalia.commandExists("curl") then onDone(3) return end

  if apiKey ~= "" then
    local argv = { "curl", "-sS", "-f", "-X", "POST", "https://up.x02.me/api/upload", "-H", "x-api-key: " .. apiKey, "-F", "file=" .. formFile(file) }
    if expiry ~= "permanent" and expiry ~= "" then
      table.insert(argv, "-F")
      table.insert(argv, "expiry=" .. expiry)
    end
    for _, a in ipairs({ "--connect-timeout", "20", "--max-time", "120" }) do table.insert(argv, a) end
    runLong(argv, function(code, text)
      if code ~= 0 then onDone(4) return end
      local url = trim(text)
      if url ~= "" and url:sub(1, 4) == "http" then onDone(0, url) else onDone(5) end
    end)
    return
  end

  if info and (info.size or 0) > UGUU_MAX_BYTES then onDone(6) return end
  uguuUpload(file, function(url, requestFailed)
    if requestFailed then onDone(4) return end
    if url then onDone(0, url) else onDone(5) end
  end)
end

local function shareFile(file)
  if file == nil or file == "" then
    noctalia.notifyError(noctalia.tr("messages.share_bad_args"))
    return
  end
  local apiKey = cfg("x02-api-key") or ""
  local expiry = cfg("x02-expiry") or "7d"
  shareUpload(tostring(file), apiKey, expiry, function(code, url)
    if code ~= 0 then
      local msg = SHARE_EXIT_KEYS[code]
      noctalia.notifyError(if msg then noctalia.tr(msg) else noctalia.tr("messages.share_unknown_error"))
      return
    end
    url = trim(url or "")
    if url == "" then
      noctalia.notifyError(noctalia.tr("messages.share_invalid_response"))
      return
    end
    noctalia.copyToClipboard(url, "text/plain;charset=utf-8")
    noctalia.notify(noctalia.tr("panel.share_url"), url)
  end)
end

-- ── Result housekeeping ────────────────────────────────────────────────────

local function clearResult()
  publish("activeTool", nil)
  publish("colorResult", nil)
  publish("ocrResult", nil)
  publish("translateResult", nil)
  publish("qrResult", nil)
  publish("paletteColors", nil)
  publish("paletteCapture", nil)
  publish("annotateResult", nil)
  saveResults()
end

local function clearHistory()
  publish("colorHistory", {})
  saveResults()
end

local function setCursorHidden(payload)
  cursorHidden = payload ~= "false"
  publish("cursorHidden", cursorHidden)
  log(`cursor hidden: {tostring(cursorHidden)}`)
end

-- ── Dispatch ───────────────────────────────────────────────────────────────

local function runRecordRegion(format)
  refreshRecorders()
  local bin = bestRegionRecorder()
  if bin == "" then noctalia.notifyError(noctalia.tr("messages.no_recorder")) return end
  pickArea(function(rect) startRecord(rect, format, bin) end)
end

local function runRecordFullscreen(format)
  refreshRecorders()
  local bin = bestFullscreenRecorder()
  if bin == "" then noctalia.notifyError(noctalia.tr("messages.no_recorder")) return end
  -- A moment for the just-closed panel to go.
  after(0.3, function() startRecord("screen", format, bin) end)
end

-- Print Screen held (capture.cpp): "x y w h" in screen pixels, a screen or a window's frame, recorded as MP4.
local function recordRect(payload)
  local nums = {}
  for n in tostring(payload or ""):gmatch("-?%d+") do table.insert(nums, tonumber(n)) end
  if #nums < 4 then log(`recordRect: bad area '{tostring(payload)}'`) return end
  refreshRecorders()
  local bin = bestRegionRecorder()
  if bin == "" then noctalia.notifyError(noctalia.tr("messages.no_recorder")) return end
  startRecord({ x = nums[1], y = nums[2], w = nums[3], h = nums[4] }, "mp4", bin)
end

-- Print Screen held, "Window": "0xHWND x y w h"; the window is recorded wherever it goes.
local function recordWindow(payload)
  local text = tostring(payload or "")
  local hwnd = text:match("^%s*(0x%x+)")
  local nums = {}
  for n in text:gsub("^%s*0x%x+", ""):gmatch("-?%d+") do table.insert(nums, tonumber(n)) end
  if not hwnd or #nums < 4 then log(`recordWindow: bad window '{text}'`) return end
  refreshRecorders()
  local bin = bestRegionRecorder()
  if bin == "" then noctalia.notifyError(noctalia.tr("messages.no_recorder")) return end
  startRecord({ x = nums[1], y = nums[2], w = nums[3], h = nums[4], hwnd = hwnd }, "mp4", bin)
end

-- Print Screen held (capture.cpp): {x, y, w, h, hwnd?} and Studio's Capture settings, as JSON.
local function recordWith(payload)
  local d = type(payload) == "string" and noctalia.json.decode(payload) or nil
  if type(d) ~= "table" or not d.w or not d.h then log(`recordWith: bad payload '{tostring(payload)}'`) return end
  refreshRecorders()
  local bin = bestRegionRecorder()
  if bin == "" then noctalia.notifyError(noctalia.tr("messages.no_recorder")) return end
  if recordState ~= "idle" then return end
  recOpts = d
  startRecord({ x = d.x, y = d.y, w = d.w, h = d.h, hwnd = d.hwnd }, "mp4", bin)
  if recordState == "idle" and not startingRecord then recOpts = nil end
end

local HANDLERS = {
  recordRect = recordRect, recordWindow = recordWindow, recordWith = recordWith,
  colorPicker = runColorPicker, ocr = runOcr, qr = runQr, palette = runPalette, lens = runLens, measure = runMeasure,
  annotate = annotateRegion, annotateFullscreen = annotateFullscreen, annotateWindow = annotateWindow,
  record = function() runRecordRegion("gif") end, recordMp4 = function() runRecordRegion("mp4") end,
  recordFullscreen = function() runRecordFullscreen("gif") end, recordFullscreenMp4 = function() runRecordFullscreen("mp4") end,
  recordStop = recordStop,
  recordSave = function(payload) finalizeRecord((type(payload) == "table" and payload.format or nil) or recordFormat, false) end,
  recordCopy = function() finalizeRecord(recordFormat, true) end, recordDiscard = recordDiscard,
  ocrSearch = ocrSearch, ocrTranslate = ocrTranslate, share = shareFile, clearResult = clearResult, clearHistory = clearHistory, setCursorHidden = setCursorHidden,
  setAudioOut = function(payload) overrideAudioOut = (type(payload) == "table" and payload.value or payload) == true end,
  setAudioIn = function(payload) overrideAudioIn = (type(payload) == "table" and payload.value or payload) == true end,
  -- Windows: the legacy Markup preview opens the last capture in the editor (Linux: `noctalia msg annotate`).
  edit = function(payload)
    local path = if type(payload) == "table" then payload.path else payload
    if type(path) == "string" and path ~= "" then editFile(path) end
  end,
  toggle = function()
    local mode = cfg("panel-mode")
    local id = mode == "legacy" and LEGACY_PANEL_ID or PANEL_ID
    noctalia.togglePanel(id)
  end,
}

-- Tools that begin a brand-new capture. Starting one discards any recording
-- that finished but was never saved, so the result panel shows the new tool's
-- result instead of the stale "save recording" card.
local CAPTURE_STARTS = {
  colorPicker = true, ocr = true, qr = true, palette = true, lens = true,
  measure = true, annotate = true, annotateFullscreen = true, annotateWindow = true,
  record = true, recordMp4 = true, recordFullscreen = true, recordFullscreenMp4 = true, recordRect = true, recordWindow = true, recordWith = true,
}

local function dispatch(event, payload)
  if event == nil then return end
  local handler = HANDLERS[event]
  if handler then
    if CAPTURE_STARTS[event] then discardPendingRecording() end
    log(`ipc '{event}'`)
    handler(payload)
  else
    log(`unknown command '{event}'`)
  end
end

-- UI entries drive the service through this channel; the CLI uses onIpc.
noctalia.state.watch("command", function(cmd)
  if type(cmd) == "table" then
    dispatch(cmd.action, cmd.payload)
  elseif type(cmd) == "string" then
    dispatch(cmd)
  end
end)

function onIpc(event, payload)
  dispatch(event, payload)
end

-- ── Boot ───────────────────────────────────────────────────────────────────

local function boot()
  noctalia.mkdirAll(DATA_DIR)
  loadState()
  cleanTemp()
  cursorHidden = cfg("hide-cursor") ~= false
  publish("cursorHidden", cursorHidden)
  refreshRecorders()
  noctalia.setUpdateInterval(60000)
  linkCapture()
end

function onExit()
  linkCapture("off")
end

-- Services stay alive while plugin settings change. Keep capture and recorder
-- flags in sync without requiring a restart.
function onConfigChanged()
  local nextCursorHidden = cfg("hide-cursor") ~= false
  if nextCursorHidden ~= cursorHidden then
    cursorHidden = nextCursorHidden
    publish("cursorHidden", cursorHidden)
  end
  refreshRecorders()
  linkCapture()
end

boot()
