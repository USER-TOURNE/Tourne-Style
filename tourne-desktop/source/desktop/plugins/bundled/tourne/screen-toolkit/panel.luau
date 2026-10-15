--!nonstrict
-- Ported to Tourne Desktop from alexander's Screen Toolkit (MIT).
--
-- Screen Toolkit: main [[panel]].
--
-- Thin client for the toolkit. Two layouts are provided, selected by the
-- "panel-mode" plugin setting:
--   * standard: the default dense grid grouped into tinted sections
--   * legacy:   recreates the Noctalia-v4 layout
-- The panel entry to open (and thus its host-owned size) is chosen by the
-- bar [[widget]] / control-center [[shortcut]] from the same setting.
-- It only dispatches actions to the [[service]] through the "command"
-- state channel. Capture tools close the panel first so the capture never
-- includes it.

local activeView = "main"
local recordAudioOut = (noctalia.getConfig("record-audio-out") == true)
local recordAudioIn = false
local hoveredAudioIn = false
local hoveredAudioOut = false
local hoveredBack = false

local panelOpen = false
local recording = noctalia.state.get("recording") == true
local hoveredTool = nil
local focusedTool local focusedIndex = 0
local hoveredClose = false
local hoveredStop = false
local render

-- ── Helpers ────────────────────────────────────────────────────────────────

local function tr(key, subst)
  if subst then return noctalia.tr(key, subst) end
  return noctalia.tr(key)
end

local function send(action, payload)
  noctalia.state.set("command", { action = action, payload = payload })
end

local function getMode()
  local mode = noctalia.getConfig("panel-mode")
  if mode == "standard" or mode == "legacy" then return mode end
  return "standard"
end

-- Capture tools must not see the panel in the frame, so close it first.
local CAPTURE_ACTIONS = {
  colorPicker = true, ocr = true, qr = true, palette = true, lens = true, measure = true,
  annotate = true, annotateFullscreen = true, annotateWindow = true,
  record = true, recordMp4 = true, recordFullscreen = true, recordFullscreenMp4 = true,
}

local function runTool(action)
  action = recordAction(action)
  if action == "record" or action == "recordFullscreen" then noctalia.state.set("recordFormat", "gif")
  elseif action == "recordMp4" or action == "recordFullscreenMp4" then noctalia.state.set("recordFormat", "mp4") end
  if CAPTURE_ACTIONS[action] then panel.close() end
  send(action)
end

-- Windows: the service keeps the thumbnail in the plugin's data folder (Linux: /tmp).
local RECORD_THUMB = (noctalia.pluginDataDir() or "") .. "\\screen-toolkit-record-thumb.png"
local lastCapturePath, lastRecordPath = nil, nil

local function refreshPreview()
  local annotateResult = noctalia.state.get("annotateResult")
  local recordPath = noctalia.state.get("recordPath")
  if annotateResult and type(annotateResult) == "table" and annotateResult.capturePath and annotateResult.capturePath ~= "" then
    lastCapturePath = annotateResult.capturePath
  end
  if recordPath and type(recordPath) == "string" and recordPath ~= "" then lastRecordPath = recordPath end
end

-- ── Tool data ──────────────────────────────────────────────────────────────
 
-- Standard layout: three rows, each a short name and its tiles (Tourne: shorter names, one record format at a time).
local TOOLS = {
  { action = "colorPicker", label = "Color", glyph = "palette" },
  { action = "ocr", label = "Text", glyph = "scan" },
  { action = "qr", label = "QR", glyph = "qrcode" },
  { action = "palette", label = "Palette", glyph = "color-swatch" },
  { action = "lens", label = "Lens", glyph = "search" },
  { action = "measure", label = "Measure", glyph = "ruler" },
}

local ANNOTATE_TOOLS = {
  { action = "annotate", label = "Area", glyph = "crop", label_key = "tools.annotate" },
  { action = "annotateWindow", label = "Window", glyph = "window", label_key = "tools.annotate_window" },
  { action = "annotateFullscreen", label = "Screen", glyph = "maximize", label_key = "tools.annotate_fullscreen" },
}

-- The record row's tiles run "record"/"recordFullscreen" (GIF) or their Mp4 twins, by the format chip.
local recordGif = false
local RECORD_ROW = {
  { action = "recordArea", label = "Area", glyph = "crop" },
  { action = "recordScreen", label = "Screen", glyph = "maximize" },
}

-- Legacy mode's record subpanel keeps all four.
local RECORD_TOOLS = {
  { action = "record", label_key = "tools.record_gif", glyph = "gif" },
  { action = "recordMp4", label_key = "tools.record_mp4", glyph = "video" },
  { action = "recordFullscreen", label_key = "tools.record_fullscreen_gif", glyph = "gif" },
  { action = "recordFullscreenMp4", label_key = "tools.record_fullscreen_mp4", glyph = "video" },
}

local function recordAction(action)
  if action == "recordArea" then return recordGif and "record" or "recordMp4" end
  if action == "recordScreen" then return recordGif and "recordFullscreen" or "recordFullscreenMp4" end
  return action
end

 -- For Legacy-mode layout
local TOOLS_LEGACY = {
  { id = "colorPicker", action = "colorPicker", label_key = "tools.colorpicker", glyph = "palette" },
  { id = "palette",  action = "palette", label_key = "tools.palette", glyph = "color-swatch" },
  { id = "ocr", action = "ocr", label_key = "tools.ocr", glyph = "scan" },
  { id = "qr", action = "qr", label_key = "tools.qr", glyph = "qrcode" },
  { id = "annotate", action = "view_annotate", label_key = "tools.annotate", glyph = "pencil" },
  { id = "record", action = "view_record", label_key = "tools.record", glyph = "video" },
  { id = "measure", action = "measure", label_key = "tools.measure", glyph = "ruler" },
  { id = "lens", action = "lens", label_key = "tools.lens", glyph = "search" },
}

-- ── Keyboard Navigation State & Helpers ────────────────────────────────────

local function getActiveItems()
  local mode = getMode()
  if mode == "standard" then
    local list = {}
    for _, t in ipairs(TOOLS) do table.insert(list, { type = "tool", action = t.action, row = 1 }) end
    for _, t in ipairs(ANNOTATE_TOOLS) do table.insert(list, { type = "tool", action = t.action, row = 2 }) end
    for _, t in ipairs(RECORD_ROW) do table.insert(list, { type = "tool", action = t.action, row = 3 }) end
    table.insert(list, { type = "format", row = 3 })
    table.insert(list, { type = "sysAudio", row = 3 })
    table.insert(list, { type = "micAudio", row = 3 })
    return list
  else
    if activeView == "annotate" then
      local list = { { type = "back" } }
      for _, t in ipairs(ANNOTATE_TOOLS) do table.insert(list, { type = "tool", action = t.action }) end
      table.insert(list, { type = "preview_screenshot" })
      return list
    elseif activeView == "record" then
      local list = { { type = "back" } }
      for _, t in ipairs(RECORD_TOOLS) do table.insert(list, { type = "tool", action = t.action }) end
      table.insert(list, { type = "sysAudio" })
      table.insert(list, { type = "micAudio" })
      if recording then table.insert(list, { type = "stopBtn" }) end
      table.insert(list, { type = "preview_recording" })
      return list
    else
      local list = {}
      for _, t in ipairs(TOOLS_LEGACY) do table.insert(list, { type = "legacy_tile", item = t }) end
      return list
    end
  end
end

local function updateFocusedTool()
  local items = getActiveItems()
  if #items == 0 or focusedIndex == 0 or focusedIndex == nil then
    focusedTool = nil
    return
  end
  if focusedIndex > #items then focusedIndex = #items end
  if focusedIndex < 1 then focusedIndex = 1 end
  local cur = items[focusedIndex]
  if cur then
    if cur.type == "tool" then focusedTool = cur.action
    elseif cur.type == "legacy_tile" then focusedTool = cur.item.id
    else focusedTool = cur.type end
  else
    focusedTool = nil
  end
end

-- Preview handlers for record and annotate subpanels.
local function openScreenshotPreview()
  refreshPreview()
  local targetPath = lastCapturePath
  if not targetPath or targetPath == "" or (noctalia.fileExists and not noctalia.fileExists(targetPath)) then
    noctalia.notifyError("No screenshot preview available yet. Take a capture first!")
    return
  end
  -- Windows: the service opens it in the Markup editor (Linux: the shell's `annotate`).
  send("edit", targetPath)
end

local function openRecordingPreview()
  refreshPreview()
  local targetPath = lastRecordPath
  if not targetPath or targetPath == "" or (noctalia.fileExists and not noctalia.fileExists(targetPath)) then
    noctalia.notifyError("No recording preview available yet. Make a recording first!")
    return
  end
  -- Windows: plays in the default video player (Linux: mpv).
  if not noctalia.openPath(targetPath) then noctalia.notifyError(tr("messages.missing_dep", { dep = "a video player" })) end
end

local function headerActions(includeClose)
  local actions = {}
  if recording then
    table.insert(actions, ui.button({
      key = "header-stop-btn", glyph = "circle", variant = "destructive", controlSize = "sm", tooltip = tr("panel.stop"),
      onClick = function() hoveredStop = false; send("recordStop") end,
    }))
  end
  if includeClose then
    table.insert(actions, ui.button({
      key = "header-close-btn", glyph = "close", width = 24, height = 24, glyphSize = 12, onClick = function() panel.close() end,
    }))
  end
  return ui.row({ gap = 4 }, actions)
end

-- ── Compact mode ───────────────────────────────────────────────────────────

local function standardHeader()
  return ui.row({ align = "center", gap = 8, paddingH = 2 }, {
    ui.glyph({ name = "crosshair", size = 15, color = "primary" }),
    ui.label({ text = tr("panel.title"), flexGrow = 1, fontSize = 14, fontWeight = "bold", color = "on_surface" }),
    headerActions(true),
  })
end

local function setHoveredTool(action)
  hoveredTool = action
  if action then
    for idx, item in ipairs(getActiveItems()) do
      local id = (item.type == "tool") and item.action or ((item.type == "legacy_tile") and item.item.id or item.type)
      if id == action then focusedIndex = idx; break end
    end
  else
    focusedIndex = 0
  end
  updateFocusedTool()
end

-- A tile: its icon and a short name; the whole tile takes the click and the hover.
local function standardTile(t)
  local isFocused = (hoveredTool == t.action) or (focusedTool == t.action)
  local color = isFocused and "primary" or "on_surface"
  return ui.column({
    key = "tile-" .. t.action, flexGrow = 1, gap = 3, align = "center", justify = "center", paddingV = 6, radius = 8,
    fill = isFocused and "primary/0.14" or "surface_variant/0",
    onClick = function() runTool(t.action) end,
    onHover = function(state) setHoveredTool((state == "true") and t.action or nil); render() end,
  }, {
    ui.glyph({ name = t.glyph, size = 17, color = color }),
    ui.label({ text = t.label or tr(t.label_key), maxLines = 1, textAlign = "center", fontSize = 11, color = color }),
  })
end

-- A small square switch in the record row.
local function chip(id, glyph, on, onClick)
  local isFocused = (hoveredTool == id) or (focusedTool == id)
  return ui.column({
    key = "chip-" .. id, width = 30, height = 30, radius = 8, align = "center", justify = "center",
    fill = on and "primary/0.18" or (isFocused and "primary/0.10" or "surface_variant/0"),
    border = (on or isFocused) and "primary" or "outline", borderWidth = 1,
    onClick = function() onClick(); render() end,
    onHover = function(state) setHoveredTool((state == "true") and id or nil); render() end,
  }, { ui.glyph({ name = glyph, size = 15, color = (on or isFocused) and "primary" or "on_surface_variant" }) })
end

local function formatChip()
  local isFocused = (hoveredTool == "format") or (focusedTool == "format")
  return ui.column({
    key = "chip-format", width = 46, height = 30, radius = 8, align = "center", justify = "center",
    fill = isFocused and "primary/0.10" or "surface_variant/0", border = isFocused and "primary" or "outline", borderWidth = 1,
    onClick = function() recordGif = not recordGif; render() end,
    onHover = function(state) setHoveredTool((state == "true") and "format" or nil); render() end,
  }, { ui.label({ text = recordGif and "GIF" or "MP4", fontSize = 11, fontWeight = "bold", color = "primary" }) })
end

local function section(name, children)
  local row = { ui.label({ text = name, width = 50, fontSize = 11, color = "on_surface_variant" }) }
  for _, c in ipairs(children) do row[#row + 1] = c end
  return ui.row({ align = "center", gap = 2, paddingV = 2, paddingH = 8, radius = 10, fill = "surface_variant" }, row)
end

local function standardGrid()
  local tools, markup, record = {}, {}, {}
  for _, t in ipairs(TOOLS) do tools[#tools + 1] = standardTile(t) end
  for _, t in ipairs(ANNOTATE_TOOLS) do markup[#markup + 1] = standardTile(t) end
  for _, t in ipairs(RECORD_ROW) do record[#record + 1] = standardTile(t) end
  record[#record + 1] = ui.row({ gap = 4, align = "center", paddingH = 4 }, {
    formatChip(),
    chip("sysAudio", recordAudioOut and "volume-2" or "volume-x", recordAudioOut,
      function() recordAudioOut = not recordAudioOut; send("setAudioOut", recordAudioOut) end),
    chip("micAudio", recordAudioIn and "microphone" or "microphone-off", recordAudioIn,
      function() recordAudioIn = not recordAudioIn; send("setAudioIn", recordAudioIn) end),
  })
  return ui.column({ gap = 6 }, {
    section(tr("panel.tools"), tools),
    section(tr("tools.annotate"), markup),
    section(tr("panel.section_record"), record),
    ui.label({ text = "Print Screen: tap for the screen, double tap for a window, hold to record", fontSize = 10, color = "on_surface_variant", textAlign = "center" }),
  })
end

-- ── Legacy mode ────────────────────────────────────────────────────────────

local function legacyHeader()
  return ui.row({ align = "center", gap = 5 }, {
    ui.glyph({ name = "crosshair", size = 18, color = "primary" }),
    ui.label({ text = tr("panel.title"), flexGrow = 1, fontSize = 18, fontWeight = "bold", color = "on_surface" }),
    headerActions(false),
  })
end

local function SubpanelHeader(title)
  local isBackFocused = hoveredBack or (focusedTool == "back")
  return ui.row({ align = "center", gap = 5 }, {
    ui.glyph({ name = "crosshair", size = 18, color = "primary" }),
    ui.label({ text = title, flexGrow = 1, fontSize = 18, fontWeight = "bold", color = "on_surface" }),
    ui.column({
      key = "back-button", height = 30, width = 30, paddingH = 0, radius = 9,
      fill = isBackFocused and "primary/0.12" or "surface", border = isBackFocused and "primary" or "surface", borderWidth = 1,
      align = "center", justify = "center",
      onClick = function() activeView = "main"; focusedIndex = 0; updateFocusedTool(); render() end,
      onHover = function(state) setHoveredTool((state == "true") and "back" or nil); render() end,
    }, {
      ui.row({ align = "center", justify = "center", gap = 4 }, {
        ui.glyph({ name = "chevron-left", size = 16, color = isBackFocused and "primary" or "on_surface" }),
      }),
    }),
  })
end

local function legacyTile(t)
  local isFocused = (hoveredTool == t.id) or (focusedTool == t.id)
  local color = isFocused and "primary" or "on_surface"
  local onClick = function()
    if t.action == "view_annotate" then activeView = "annotate"; focusedIndex = 0; updateFocusedTool(); render()
    elseif t.action == "view_record" then activeView = "record"; focusedIndex = 0; updateFocusedTool(); render()
    else runTool(t.action) end
  end
  return ui.column({
    key = "tile-" .. t.id, flexGrow = 1, gap = 2, paddingV = 5, paddingH = 0, align = "center", justify = "center", onClick = onClick,
    onHover = function(state) setHoveredTool((state == "true") and t.id or nil); render() end,
  }, {
    ui.column({ width = 50, height = 50, radius = 15, fill = "on_primary", border = color == "primary" and "primary" or "outline", borderWidth = isFocused and 2 or 0, align = "center", justify = "center" }, {
      ui.glyph({ name = t.glyph, size = 25, color = color }),
    }),
    ui.label({ text = tr(t.label_key), maxWidth = 80, maxLines = 1, textAlign = "center", fontSize = 11, color = color }),
  })
end

local function legacyGrid()
  local row1, row2 = {}, {}
  for i, t in ipairs(TOOLS_LEGACY) do table.insert(i <= 4 and row1 or row2, legacyTile(t)) end
  return ui.column({ gap = 10, paddingV = 15, paddingH = 20, radius = 18, fill = "surface_variant" }, {
    ui.row({ flexGrow = 1, gap = 0 }, row1), ui.row({ flexGrow = 1, gap = 0 }, row2),
  })
end

-- Subpanels for legacy mode: annotate and record.

local function subpanelButton(t, paddingV)
  local isFocused = (hoveredTool == t.action) or (focusedTool == t.action)
  local color = isFocused and "primary" or "on_surface"
  return ui.column({
    key = "btn-" .. t.action, paddingV = paddingV, paddingH = 5, radius = 8,
    fill = isFocused and "primary/0.12" or "surface", border = isFocused and "primary" or "surface", borderWidth = 1,
    onClick = function() runTool(t.action) end,
    onHover = function(state) setHoveredTool((state == "true") and t.action or nil); render() end,
  }, {
    ui.row({ align = "center", gap = 6 }, {
      ui.glyph({ name = t.glyph, size = 15, color = color }),
      ui.label({ text = tr(t.label_key), fontSize = 12, fontWeight = "bold", color = color }),
    }),
  })
end

local function MarkupSubpanel()
  refreshPreview()
  local leftButtons = {}
  for _, t in ipairs(ANNOTATE_TOOLS) do table.insert(leftButtons, subpanelButton(t, 6)) end
  local fileExists = noctalia.fileExists
  local hasCaptureFile = lastCapturePath and lastCapturePath ~= "" and (not fileExists or fileExists(lastCapturePath))
  local isPreviewFocused = (focusedTool == "preview_screenshot")
  local rightPreview = ui.column({
    width = 200, height = 150, align = "center", justify = "center", padding = hasCaptureFile and 4 or 8, radius = 10, fill = "surface",
    border = isPreviewFocused and "primary" or "outline", borderWidth = isPreviewFocused and 2 or 1, onClick = openScreenshotPreview,
  }, hasCaptureFile and {
    ui.image({ path = lastCapturePath, width = 200, height = 130, fit = "contain", radius = 6 }),
    ui.label({ text = "Click to annotate", fontSize = 10, fontWeight = "bold", color = "primary" }),
  } or {
    ui.glyph({ name = "pencil", size = 28, color = "on_surface_variant" }),
    ui.label({ text = "No screenshot yet", fontSize = 11, color = "on_surface_variant" }),
  })
  return ui.column({ flexGrow = 1, gap = 10 }, {
    SubpanelHeader("Markup"),
    ui.column({ gap = 10, paddingV = 15, paddingH = 20, radius = 18, fill = "surface_variant" }, {
      ui.row({ flexGrow = 1, gap = 10 }, { ui.column({ width = 100, gap = 6 }, leftButtons), rightPreview }),
    }),
  })
end

local function RecordSubpanel()
  refreshPreview()
  local leftButtons = {}
  for _, t in ipairs(RECORD_TOOLS) do table.insert(leftButtons, subpanelButton(t, 4)) end
  local stopFocused = hoveredStop or (focusedTool == "stopBtn")
  local stopBtn = ui.column({
    key = "btn-stop-record", width = 28, height = 28, radius = 6, fill = "error", border = stopFocused and "primary" or "error", borderWidth = stopFocused and 2 or 1, align = "center", justify = "center",
    onClick = function() send("recordStop") end,
    onHover = function(state) setHoveredTool((state == "true") and "stopBtn" or nil); render() end,
  }, { ui.glyph({ name = "circle", size = 16, color = "on_primary" }) })

  local sysAudioFocused = hoveredAudioOut or (focusedTool == "sysAudio")
  local sysAudioBtn = ui.column({
    key = "toggle-sys-audio", width = 28, height = 28, radius = 6,
    fill = recordAudioOut and (sysAudioFocused and "primary/0.25" or "primary/0.15") or (sysAudioFocused and "surface_variant" or "surface_variant/0.5"),
    border = (recordAudioOut or sysAudioFocused) and "primary" or "outline", borderWidth = 1, align = "center", justify = "center",
    onClick = function() recordAudioOut = not recordAudioOut; send("setAudioOut", recordAudioOut); render() end,
    onHover = function(state) setHoveredTool((state == "true") and "sysAudio" or nil); render() end,
  }, { ui.glyph({ name = recordAudioOut and "volume-2" or "volume-x", size = 16, color = (recordAudioOut or sysAudioFocused) and "primary" or "on_surface_variant" }) })

  local micAudioFocused = hoveredAudioIn or (focusedTool == "micAudio")
  local micAudioBtn = ui.column({
    key = "toggle-mic-audio", width = 28, height = 28, radius = 6,
    fill = recordAudioIn and (micAudioFocused and "primary/0.25" or "primary/0.15") or (micAudioFocused and "surface_variant" or "surface_variant/0.5"),
    border = (recordAudioIn or micAudioFocused) and "primary" or "outline", borderWidth = 1, align = "center", justify = "center",
    onClick = function() recordAudioIn = not recordAudioIn; send("setAudioIn", recordAudioIn); render() end,
    onHover = function(state) setHoveredTool((state == "true") and "micAudio" or nil); render() end,
  }, { ui.glyph({ name = recordAudioIn and "microphone" or "microphone-off", size = 16, color = (recordAudioIn or micAudioFocused) and "primary" or "on_surface_variant" }) })

  local controlsList = { sysAudioBtn, micAudioBtn }
  if recording then table.insert(controlsList, 1, stopBtn) end
  local controlRow = ui.row({ gap = 6, align = "center", justify = "center" }, controlsList)
  local fileExists = noctalia.fileExists
  local hasRecordFile = lastRecordPath and lastRecordPath ~= "" and (not fileExists or fileExists(lastRecordPath))
  local hasRecordThumb = hasRecordFile and fileExists and fileExists(RECORD_THUMB)
  local isPreviewFocused = (focusedTool == "preview_recording")
  local rightPreview
  if hasRecordFile or hasRecordThumb then
    local thumbElement = hasRecordThumb and ui.image({ path = RECORD_THUMB, width = 240, height = 115, fit = "contain", radius = 6 }) or ui.glyph({ name = "movie", size = 28, color = "primary" })
    local filename = lastRecordPath and (lastRecordPath:match("([^/\\]+)$") or lastRecordPath) or "Saved Recording"
    rightPreview = ui.column({
      width = 200, height = 150, align = "center", justify = "center", padding = hasRecordFile and 4 or 8, radius = 10, fill = "surface",
      border = isPreviewFocused and "primary" or "outline", borderWidth = isPreviewFocused and 2 or 1, onClick = openRecordingPreview,
    }, { thumbElement, ui.label({ text = filename, maxWidth = 160, maxLines = 1, fontSize = 9, color = "on_surface_variant" }), ui.label({ text = "Click to play", fontSize = 10, fontWeight = "bold", color = "primary" }) })
  else
    rightPreview = ui.column({
      width = 200, height = 130, align = "center", justify = "center", padding = 8, radius = 10, fill = "surface",
      border = isPreviewFocused and "primary" or "outline", borderWidth = isPreviewFocused and 2 or 1, onClick = openRecordingPreview,
    }, { ui.glyph({ name = "video", size = 28, color = "on_surface_variant" }), ui.label({ text = "No recording yet", fontSize = 11, color = "on_surface_variant" }) })
  end
  local leftColumn = ui.column({ width = 100, gap = 6, align = "center" }, {
    ui.column({ width = 100, gap = 6 }, leftButtons),
    controlRow,
  })
  return ui.column({ flexGrow = 1, gap = 10 }, {
    SubpanelHeader("Record"),
    ui.column({ gap = 10, paddingV = 15, paddingH = 20, radius = 18, fill = "surface_variant" }, {
      ui.row({ flexGrow = 1, gap = 10 }, { leftColumn, rightPreview }),
    }),
  })
end

-- ── Keyboard Handler ───────────────────────────────────────────────────────

-- Standard mode, up and down: the item at the same place in the row above or below (or that row's last).
local function standardStep(items, index, dir)
  local cur = items[index]
  if not cur then return 1 end
  local col = 0
  for i = 1, index do if items[i].row == cur.row then col += 1 end end
  local target = (cur.row - 1 + dir) % 3 + 1
  local last, n = nil, 0
  for i, it in ipairs(items) do
    if it.row == target then
      n += 1
      last = i
      if n == col then return i end
    end
  end
  return last or index
end

function onKey(chord, pressed)
  if not pressed then return end
  local key = tostring(chord):lower()
  local items = getActiveItems()
  if #items == 0 then return end
  local mode = getMode()
  if chord == "Escape" then panel.close() return end

  -- If nothing is focused yet (focusedIndex == 0), pressing arrow keys/tab takes focus directly to 1st square
  if focusedIndex == 0 or focusedIndex == nil then
    if key == "tab" or key == "right" or key == "left" or key == "backtab" or key == "shift+tab" or key == "down" or key == "up" then
      focusedIndex = 1
      updateFocusedTool()
      render()
      return
    end
  end

  if key == "tab" or key == "right" then
    focusedIndex = (focusedIndex % #items) + 1
  elseif key == "backtab" or key == "left" then
    focusedIndex = (focusedIndex - 2) % #items + 1
  elseif key == "down" then
    if mode == "legacy" and activeView == "main" then
      focusedIndex = focusedIndex <= 4 and (focusedIndex + 4) or (focusedIndex - 4)
    elseif mode == "standard" then
      focusedIndex = standardStep(items, focusedIndex, 1)
    else
      focusedIndex = (focusedIndex % #items) + 1
    end
  elseif key == "up" then
    if mode == "legacy" and activeView == "main" then
      focusedIndex = focusedIndex > 4 and (focusedIndex - 4) or (focusedIndex + 4)
    elseif mode == "standard" then
      focusedIndex = standardStep(items, focusedIndex, -1)
    else
      focusedIndex = (focusedIndex - 2) % #items + 1
    end
  elseif key == "return" or key == "enter" or key == "space" then
    local current = items[focusedIndex]
    if current then
      if current.type == "tool" then runTool(current.action)
      elseif current.type == "legacy_tile" then
        if current.item.action == "view_annotate" then activeView = "annotate"; focusedIndex = 0
        elseif current.item.action == "view_record" then activeView = "record"; focusedIndex = 0
        else runTool(current.item.action) end
      elseif current.type == "back" then activeView = "main"; focusedIndex = 0
      elseif current.type == "format" then recordGif = not recordGif
      elseif current.type == "sysAudio" then recordAudioOut = not recordAudioOut; send("setAudioOut", recordAudioOut)
      elseif current.type == "micAudio" then recordAudioIn = not recordAudioIn; send("setAudioIn", recordAudioIn)
      elseif current.type == "stopBtn" then send("recordStop")
      elseif current.type == "preview_screenshot" then openScreenshotPreview()
      elseif current.type == "preview_recording" then openRecordingPreview() end
    end
  end
  updateFocusedTool()
  render()
end

-- ── Main render ────────────────────────────────────────────────────────────

render = function()
  updateFocusedTool()
  local mode = getMode()
  local body = (mode == "standard" and ui.column({ flexGrow = 1, gap = 6 }, { standardHeader(), standardGrid() }))
    or (activeView == "annotate" and MarkupSubpanel())
    or (activeView == "record" and RecordSubpanel())
    or ui.column({ flexGrow = 1, gap = 6 }, { legacyHeader(), legacyGrid() })
  panel.render(body)
end

-- ── Lifecycle ──────────────────────────────────────────────────────────────

function onOpen(_context)
  panelOpen, hoveredTool, focusedIndex, activeView = true, false, 0, "main"
  recordAudioOut = (noctalia.getConfig("record-audio-out") == true)
  -- Tourne: both start as the settings say (the original turned the microphone off on every open).
  recordAudioIn = (noctalia.getConfig("record-audio-in") == true)
  send("setAudioOut", recordAudioOut)
  send("setAudioIn", recordAudioIn)
  refreshPreview()
  updateFocusedTool()
  render()
end

function onClose()
  panelOpen, hoveredBack = false, false
end

noctalia.state.watch("recording", function(v)
  recording = v == true
  if panelOpen then render() end
end)

function update()
  noctalia.setUpdateInterval(500)
end
