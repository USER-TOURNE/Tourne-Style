--!nonstrict
-- Tourne Notes: Markdown notes in a side panel.
--
-- Started from Noctalia's Notes (MIT, noctalia-dev/official-plugins). Notes are
-- plain files in the notes folder. The panel shows a note list (with search)
-- and an editor with idle autosave (after a short pause in typing, on leaving
-- a note and on closing the panel). Tourne Desktop's extras:
--   - a name typed in the name box is kept when you leave the note, not only
--     on Enter, and a new note takes its first line as its name;
--   - Ctrl+V with a picture copied, or a file dropped on the panel, copies it
--     into images\ or files\ beside the notes and puts its link in the note;
--   - a preview shows the note formatted, pictures and all, and ticks off
--     checklist lines when they're clicked;
--   - deleted notes go to .trash, where they can be put back.

local AUTOSAVE_IDLE_TICKS = 2 -- 1s panel ticks with no edits before a flush
local SCRATCHPAD_BASE = "scratchpad"
local TRASH = ".trash"
local SIDECAR = ".tourne-notes.json"
local IMAGES = "images"
local FILES = "files"
local AUTO_NAME_MAX = 60
local PICTURE_WIDTH = 380
local PICTURE_TYPES = { png = true, jpg = true, jpeg = true, gif = true, bmp = true, webp = true, tif = true, tiff = true, ico = true }

local notesDir = ""
local suffix = ".md" -- "." .. the configured extension, resolved in onOpen
local files = {}
local trashed = {}
local view = "list" -- "list" | "editor" | "trash"
local current = nil -- filename ("foo.md") while the editor is open
local buffer = ""
local loadedContent = "" -- what the file held when we last read/wrote it
local editorRev = 0 -- bumped to reseed the editor input after external changes
local dirty = false
local idleTicks = 0
local pendingDelete = nil -- filename awaiting the second (confirm) click
local confirmEmpty = false
local sortByDate = false
local pins = {} -- filename -> true; persisted in a sidecar next to the notes
local autoNamed = {} -- filename -> true: still has its date name, takes its first line on leaving
local pendingName = nil -- what's typed in the name box and not saved yet
local previewing = false
local search = ""
local snippets = {} -- filename -> the line the search found in it
local contentCache = {} -- filename -> { mtime, text }, for search
local hoveredKey = nil
local cornerRadiusScale = 1.0
local editorFont = "pixel"
local editorSize = 16

local render

local function tr(key, params)
  if params ~= nil then
    return noctalia.tr(key, params)
  end
  return noctalia.tr(key)
end

local function displayName(file)
  return file:sub(1, #file - #suffix)
end

local function hasSuffix(name)
  return #name > #suffix and name:sub(-#suffix):lower() == suffix:lower()
end

local function notePath(file)
  return notesDir .. "/" .. file
end

local function scratchName()
  return SCRATCHPAD_BASE .. suffix
end

local function editorKey()
  return "editor-" .. (current or "") .. "-" .. editorRev
end

-- Pins live in a hidden sidecar inside the notes directory so they sync and
-- survive restarts with the notes themselves.
local function pinsPath()
  return notesDir .. "/.pinned.json"
end

local function readFlags(path)
  local out = {}
  local raw = noctalia.readFile(path)
  if raw ~= nil then
    local decoded = noctalia.json.decode(raw)
    if type(decoded) == "table" then
      for name, value in pairs(decoded) do
        if type(name) == "string" and value == true then
          out[name] = true
        end
      end
    end
  end
  return out
end

local function writeFlags(path, flags)
  local encoded = noctalia.json.encode(flags)
  if encoded == nil then
    return
  end
  local ok, err = noctalia.writeFile(path, encoded)
  if not ok then
    noctalia.notifyError(tr("title"), err or "")
  end
end

local function loadPins()
  pins = readFlags(pinsPath())
  autoNamed = readFlags(notesDir .. "/" .. SIDECAR)
end

local function savePins()
  writeFlags(pinsPath(), pins)
end

local function saveAutoNamed()
  writeFlags(notesDir .. "/" .. SIDECAR, autoNamed)
end

-- A name Windows takes for a file: no \ / : * ? " < > | or control characters,
-- no dots or spaces at the end, not hidden, not a device name.
local function safeName(text)
  local name = text:gsub("%c", " ")
  name = name:gsub('[\\/:%*%?"<>|]', "-")
  name = noctalia.string.trim(name)
  name = name:gsub("^%.+", "")
  name = name:gsub("[%. ]+$", "")
  local lower = name:lower()
  if lower == "con" or lower == "prn" or lower == "aux" or lower == "nul" or lower:match("^com%d$") or lower:match("^lpt%d$") then
    name ..= "_"
  end
  if lower == SCRATCHPAD_BASE then
    name ..= " note"
  end
  return name
end

-- At most `limit` bytes, never cutting a UTF-8 character.
local function cut(text, limit)
  if #text <= limit then
    return text
  end
  local at = limit
  while at > 1 and text:byte(at + 1) ~= nil and text:byte(at + 1) >= 0x80 and text:byte(at + 1) < 0xC0 do
    at -= 1
  end
  return noctalia.string.trim(text:sub(1, at))
end

-- A name for `base` .. `ext` in `folder` that isn't taken: "base", "base (2)"...
local function uniqueIn(folder, base, ext, except)
  local name = base .. ext
  local counter = 2
  while name:lower() ~= (except or ""):lower() and noctalia.fileExists(folder .. "/" .. name) do
    name = base .. " (" .. counter .. ")" .. ext
    counter += 1
  end
  return name
end

-- The words a note starts with, for its name: its first line with text, its marks dropped.
local function firstLine(text)
  for line in (text .. "\n"):gmatch("(.-)\r?\n") do
    local words = line:gsub("!%[.-%]%(.-%)", "")
    words = words:gsub("%[(.-)%]%(.-%)", "%1")
    words = words:gsub("^%s*[#>]+%s*", "")
    words = words:gsub("^%s*[-*+]%s+%[[ xX]%]%s*", "")
    words = words:gsub("^%s*[-*+]%s+", "")
    words = words:gsub("[*_`~]", "")
    words = noctalia.string.trim(words)
    if words ~= "" then
      return cut(safeName(words), AUTO_NAME_MAX)
    end
  end
  return ""
end

local function readNote(file)
  local info = noctalia.fileInfo(notePath(file))
  local mtime = info ~= nil and info.mtime or 0
  local cached = contentCache[file]
  if cached ~= nil and cached.mtime == mtime then
    return cached.text
  end
  local text = noctalia.readFile(notePath(file)) or ""
  contentCache[file] = { mtime = mtime, text = text }
  return text
end

-- Search: the name, then the text; `snippets` keeps the line that matched.
local function matches(file, needle)
  if needle == "" then
    return true
  end
  local name = file == scratchName() and tr("scratchpad") or displayName(file)
  if name:lower():find(needle, 1, true) ~= nil then
    return true
  end
  local text = readNote(file)
  local at = text:lower():find(needle, 1, true)
  if at == nil then
    return false
  end
  local lineStart = at
  while lineStart > 1 and text:sub(lineStart - 1, lineStart - 1) ~= "\n" do
    lineStart -= 1
  end
  local lineEnd = text:find("\n", at, true) or (#text + 1)
  snippets[file] = cut(noctalia.string.trim(text:sub(lineStart, lineEnd - 1)), 80)
  return true
end

local function refreshFiles()
  files = {}
  snippets = {}
  local needle = noctalia.string.trim(search):lower()
  local list = noctalia.listDir(notesDir)
  if list ~= nil then
    for _, name in ipairs(list) do
      if hasSuffix(name) and name ~= scratchName() and name:sub(1, 1) ~= "." and matches(name, needle) then
        table.insert(files, name)
      end
    end
  end
  -- Pinned notes first, then the selected order within each group.
  local mtimes = {}
  if sortByDate then
    for _, name in ipairs(files) do
      local info = noctalia.fileInfo(notePath(name))
      mtimes[name] = info ~= nil and info.mtime or 0
    end
  end
  table.sort(files, function(a, b)
    local aPinned = pins[a] == true
    local bPinned = pins[b] == true
    if aPinned ~= bPinned then
      return aPinned
    end
    if sortByDate and mtimes[a] ~= mtimes[b] then
      return mtimes[a] > mtimes[b]
    end
    return a:lower() < b:lower()
  end)
end

local function refreshTrash()
  trashed = {}
  local list = noctalia.listDir(notesDir .. "/" .. TRASH)
  if list ~= nil then
    for _, name in ipairs(list) do
      if hasSuffix(name) then
        table.insert(trashed, name)
      end
    end
  end
  table.sort(trashed, function(a, b)
    return a:lower() < b:lower()
  end)
end

local function flush()
  if not dirty or current == nil then
    return
  end
  local ok, err = noctalia.writeFile(notePath(current), buffer)
  if not ok then
    noctalia.notifyError(tr("title"), err or tr("save_failed"))
    return
  end
  loadedContent = buffer
  dirty = false
end

-- Renames the open note to `base` (made safe). Quiet: a taken name gets " (2)" instead of an error.
local function renameTo(base, quiet)
  if current == nil or current == scratchName() then
    return false
  end
  local name = safeName(base)
  if name == "" or name == displayName(current) then
    return false
  end
  local target = name .. suffix
  if target:lower() ~= current:lower() and noctalia.fileExists(notePath(target)) then
    if not quiet then
      noctalia.notifyError(tr("title"), tr("rename_exists"))
      return false
    end
    target = uniqueIn(notesDir, name, suffix, current)
  end
  flush()
  local ok, err = noctalia.renameFile(notePath(current), notePath(target))
  if not ok then
    noctalia.notifyError(tr("title"), err or "")
    return false
  end
  if pins[current] ~= nil then
    pins[current] = nil
    pins[target] = true
    savePins()
  end
  if autoNamed[current] ~= nil then
    autoNamed[current] = nil
    saveAutoNamed()
  end
  contentCache[current] = nil
  current = target
  return true
end

-- On leaving a note (back, another note, a new one, closing the panel): the name typed
-- in its name box, or for a note still named by its date, its first line.
local function settleName()
  if current == nil then
    return
  end
  flush()
  if pendingName ~= nil then
    local typed = pendingName
    pendingName = nil
    if autoNamed[current] ~= nil then
      autoNamed[current] = nil
      saveAutoNamed()
    end
    renameTo(typed, false)
    return
  end
  if autoNamed[current] ~= nil then
    local first = firstLine(buffer)
    if first ~= "" then
      renameTo(first, true)
    end
  end
end

local function leaveNote()
  settleName()
  view = "list"
  current = nil
  buffer = ""
  dirty = false
  previewing = false
  pendingDelete = nil
end

local function openNote(file)
  if current ~= nil and current ~= file then
    settleName()
  end
  flush()
  local contents = noctalia.readFile(notePath(file))
  buffer = contents or ""
  loadedContent = buffer
  current = file
  view = "editor"
  dirty = false
  idleTicks = 0
  pendingDelete = nil
  pendingName = nil
  previewing = false
  render()
end

-- Deleting moves the note to .trash, where the trash view can put it back.
local function deleteNote(file)
  noctalia.mkdirAll(notesDir .. "/" .. TRASH)
  local target = uniqueIn(notesDir .. "/" .. TRASH, displayName(file), suffix)
  local ok, err = noctalia.renameFile(notePath(file), notesDir .. "/" .. TRASH .. "/" .. target)
  if not ok then
    noctalia.notifyError(tr("title"), err or "")
  end
  if pins[file] ~= nil then
    pins[file] = nil
    savePins()
  end
  if autoNamed[file] ~= nil then
    autoNamed[file] = nil
    saveAutoNamed()
  end
  contentCache[file] = nil
  if current == file then
    current = nil
    buffer = ""
    dirty = false
    pendingName = nil
    previewing = false
    view = "list"
  end
  pendingDelete = nil
  refreshFiles()
  render()
end

local function restoreNote(file)
  local base = displayName(file)
  local target = uniqueIn(notesDir, base, suffix)
  local ok, err = noctalia.renameFile(notesDir .. "/" .. TRASH .. "/" .. file, notePath(target))
  if not ok then
    noctalia.notifyError(tr("title"), err or "")
  end
  refreshTrash()
  refreshFiles()
  render()
end

local function deleteForever(file)
  local ok, err = noctalia.removeFile(notesDir .. "/" .. TRASH .. "/" .. file)
  if not ok then
    noctalia.notifyError(tr("title"), err or "")
  end
  pendingDelete = nil
  refreshTrash()
  render()
end

local function uniqueNoteName()
  local base = noctalia.formatTime("%Y-%m-%d %H.%M.%S")
  return uniqueIn(notesDir, base, suffix)
end

-- A file copied in beside the notes (a picture to images\, anything else to files\): its link.
local function attach(from)
  local name = from:match("[^\\/]+$") or "file"
  local ext = (name:match("%.([^.]+)$") or ""):lower()
  local picture = PICTURE_TYPES[ext] == true
  local folder = picture and IMAGES or FILES
  local dir = notesDir .. "/" .. folder
  noctalia.mkdirAll(dir)
  local base = name:match("^(.*)%.[^.]+$") or name
  local target = uniqueIn(dir, base, ext ~= "" and ("." .. ext) or "")
  local ok, err = noctalia.copyFile(from, dir .. "/" .. target)
  if not ok then
    noctalia.notifyError(tr("title"), tr("copy_failed", { name = name }) .. (err and (": " .. err) or ""))
    return nil
  end
  local link = "<" .. folder .. "/" .. target .. ">"
  if picture then
    return "![" .. base .. "](" .. link .. ")"
  end
  return "[" .. name .. "](" .. link .. ")"
end

-- Puts text in the open note: at the caret while the editor shows, else at its end.
local function insertIntoNote(text)
  if current == nil or text == "" then
    return
  end
  if not previewing and panel.insertText ~= nil and panel.insertText(editorKey(), text, true) then
    return -- (the editor's change comes back through onEdited)
  end
  local sep = (buffer == "" or buffer:sub(-1) == "\n") and "" or "\n"
  buffer = buffer .. sep .. text .. "\n"
  dirty = true
  flush()
  editorRev += 1
  render()
end

local function deleteControls(file, onYes)
  return {
    ui.label({ text = tr("confirm_delete"), color = "error" }),
    ui.button({
      glyph = "check",
      variant = "primary",
      onClick = onYes or function()
        deleteNote(file)
      end,
    }),
    ui.button({
      glyph = "close",
      variant = "ghost",
      onClick = function()
        pendingDelete = nil
        render()
      end,
    }),
  }
end

local function togglePin(file)
  pins[file] = not pins[file] and true or nil
  savePins()
  refreshFiles()
  render()
end

local function noteTitle(file, isHovered)
  local color = isHovered and "surface" or "on_surface"
  local lines = {
    ui.label({ text = displayName(file), maxLines = 1, color = color }),
  }
  if snippets[file] ~= nil and snippets[file] ~= "" then
    table.insert(lines, ui.label({
      text = snippets[file],
      maxLines = 1,
      fontSize = 11,
      color = isHovered and "surface" or "on_surface_variant",
    }))
  end
  return ui.column({ flexGrow = 1, gap = 2 }, lines)
end

local function listRow(file)
  local pinned = pins[file] == true
  local isHovered = hoveredKey == file
  -- Label elides natively with maxLines=1; the row wrapper supplies the click target.
  local titleChildren = { noteTitle(file, isHovered) }
  if pinned then
    table.insert(titleChildren, 1, ui.glyph({
      name = "pinned",
      size = 14,
      color = isHovered and "surface" or "on_surface",
    }))
  end
  local children = {
    ui.row({
      key = file,
      flexGrow = 1,
      gap = 6,
      align = "center",
      padding = 8,
      paddingH = 12,
      fill = isHovered and "tertiary" or "surface",
      radius = 6 * cornerRadiusScale,
      onHover = "onRowHover",
      onClick = function()
        openNote(file)
      end,
    }, titleChildren),
  }
  if pendingDelete == file then
    for _, node in ipairs(deleteControls(file)) do
      table.insert(children, node)
    end
  else
    table.insert(children, ui.button({
      glyph = pinned and "pinned-off" or "pin",
      variant = "ghost",
      onClick = function()
        togglePin(file)
      end,
    }))
    table.insert(children, ui.button({
      glyph = "trash",
      variant = "ghost",
      onClick = function()
        pendingDelete = file
        render()
      end,
    }))
  end
  -- Pin state is part of the key: retained buttons keep a previously set
  -- glyph, so the row is recreated when it flips.
  return ui.row({ key = "note-" .. file .. (pinned and "-pinned" or ""), gap = 6, align = "center" }, children)
end

local function listView()
  local scratchFile = scratchName()
  local isHovered = hoveredKey == scratchFile
  local needle = noctalia.string.trim(search):lower()
  local rows = {}
  if noctalia.fileExists(notePath(scratchFile)) and needle ~= "" then
    matches(scratchFile, needle)
  end
  -- The scratchpad is always present (created on first write) and not deletable from here.
  if needle == "" or snippets[scratchFile] ~= nil or tr("scratchpad"):lower():find(needle, 1, true) ~= nil then
    local title = { ui.label({ text = tr("scratchpad"), maxLines = 1, color = isHovered and "surface" or "on_surface" }) }
    if snippets[scratchFile] ~= nil then
      table.insert(title, ui.label({ text = snippets[scratchFile], maxLines = 1, fontSize = 11, color = isHovered and "surface" or "on_surface_variant" }))
    end
    table.insert(rows, ui.row({ key = "scratchpad", gap = 6, align = "center" }, {
      ui.row({
        key = scratchFile,
        flexGrow = 1,
        gap = 6,
        align = "center",
        padding = 8,
        paddingH = 12,
        fill = isHovered and "tertiary" or "surface",
        radius = 6 * cornerRadiusScale,
        onHover = "onRowHover",
        onClick = "onOpenScratchpad",
      }, {
        ui.glyph({ name = "pinned", size = 14, color = isHovered and "surface" or "on_surface" }),
        ui.column({ flexGrow = 1, gap = 2 }, title),
      }),
    }))
  end
  for _, file in ipairs(files) do
    table.insert(rows, listRow(file))
  end
  if #files == 0 then
    table.insert(rows, ui.label({ text = needle ~= "" and tr("no_matches") or tr("empty"), color = "on_surface_variant" }))
  end
  return ui.column({ flexGrow = 1, gap = 8, align = "stretch" }, {
    ui.input({
      key = "search",
      value = search,
      placeholder = tr("search_placeholder"),
      onChange = "onSearch",
    }),
    ui.scroll({ key = "list", flexGrow = 1, gap = 4 }, rows),
  })
end

local function trashView()
  local rows = {}
  for _, file in ipairs(trashed) do
    local children = {
      ui.label({ text = displayName(file), maxLines = 1, flexGrow = 1, color = "on_surface" }),
    }
    if pendingDelete == file then
      for _, node in ipairs(deleteControls(file, function()
        deleteForever(file)
      end)) do
        table.insert(children, node)
      end
    else
      table.insert(children, ui.button({
        glyph = "restore",
        variant = "ghost",
        onClick = function()
          restoreNote(file)
        end,
      }))
      table.insert(children, ui.button({
        glyph = "trash-x",
        variant = "ghost",
        onClick = function()
          pendingDelete = file
          render()
        end,
      }))
    end
    table.insert(rows, ui.row({ key = "trashed-" .. file, gap = 6, align = "center", paddingH = 12 }, children))
  end
  if #trashed == 0 then
    table.insert(rows, ui.label({ text = tr("trash_empty"), color = "on_surface_variant" }))
  end
  return ui.scroll({ key = "trash", flexGrow = 1, gap = 4 }, rows)
end

local function editorStatusLine()
  local words = select(2, buffer:gsub("%S+", ""))
  local chars = utf8.len(buffer) or #buffer
  local status = dirty and tr("editing") or tr("saved")
  return status .. "  ·  " .. noctalia.tr("counts", { words = words, chars = chars })
end

local function editorHeader()
  local trailing
  if pendingDelete == current then
    trailing = deleteControls(current)
  else
    trailing = {
      ui.button({ key = "preview", glyph = previewing and "pencil" or "eye", variant = previewing and "primary" or "ghost", onClick = "onTogglePreview" }),
      ui.button({ key = "notepad", glyph = "external-link", variant = "ghost", onClick = "onOpenInNotepad" }),
      ui.button({ key = "delete", glyph = "trash", variant = "ghost", onClick = "onEditorDelete" }),
    }
  end
  local children = {
    ui.button({ glyph = "arrow-left", variant = "ghost", onClick = "onBack" }),
    ui.input({
      key = "rename-" .. current,
      value = current == scratchName() and tr("scratchpad") or displayName(current),
      onSubmit = "onRename",
      onChange = "onNameTyped",
      -- The scratchpad's identity is its name; renaming it would just respawn it.
      enabled = current ~= scratchName(),
      flexGrow = 1,
    }),
  }
  for _, node in ipairs(trailing) do
    table.insert(children, node)
  end
  return ui.row({ gap = 6, align = "center" }, children)
end

----------------------------------------------------------------------------
-- Preview: the note's Markdown as rows (headings, lists, checklists, quotes,
-- code, pictures, links), with checklist lines ticked off by a click.

local function linkTarget(raw)
  local target = noctalia.string.trim(raw)
  target = target:gsub('%s+".*"$', "") -- (a title after the address)
  if target:sub(1, 1) == "<" and target:sub(-1) == ">" then
    target = target:sub(2, -2)
  else
    target = noctalia.string.urlDecode(target)
  end
  return target
end

local function isWeb(target)
  return target:match("^%a[%w+.-]*://") ~= nil or target:match("^mailto:") ~= nil
end

local function fullPath(target)
  if isWeb(target) or target:match("^%a:[\\/]") or target:sub(1, 2) == "\\\\" then
    return target
  end
  return notesDir .. "/" .. target
end

-- Inline marks dropped, links as their text (pictures show on their own).
local function plain(text)
  local out = text:gsub("!%[.-%]%(.-%)", "")
  out = out:gsub("%[(.-)%]%(.-%)", "%1")
  out = out:gsub("%*%*(.-)%*%*", "%1")
  out = out:gsub("__(.-)__", "%1")
  out = out:gsub("~~(.-)~~", "%1")
  out = out:gsub("`(.-)`", "%1")
  out = out:gsub("%*([^%*%s][^%*]-)%*", "%1")
  return out
end

local function textNode(text, props)
  props = props or {}
  props.text = plain(text)
  props.fontSize = props.fontSize or editorSize
  props.color = props.color or "on_surface"
  return ui.label(props)
end

-- A line's pictures and other links, as their own rows after its text.
local function lineExtras(line, rows, lineNo)
  local index = 0
  for alt, raw in line:gmatch("!%[(.-)%]%((.-)%)") do
    index += 1
    local path = fullPath(linkTarget(raw))
    table.insert(rows, ui.image({
      key = "picture-" .. lineNo .. "-" .. index,
      path = path,
      alt = alt,
      maxWidth = PICTURE_WIDTH,
      onClick = function()
        noctalia.openPath(path)
      end,
    }))
  end
  local stripped = line:gsub("!%[.-%]%(.-%)", "")
  for text, raw in stripped:gmatch("%[(.-)%]%((.-)%)") do
    index += 1
    local target = fullPath(linkTarget(raw))
    table.insert(rows, ui.row({ key = "link-" .. lineNo .. "-" .. index, gap = 6, align = "center" }, {
      ui.button({
        glyph = isWeb(target) and "external-link" or "paperclip",
        text = cut(plain(text) ~= "" and plain(text) or target, 60),
        variant = "ghost",
        controlSize = "sm",
        fontSize = 13,
        contentAlign = "start",
        onClick = function()
          noctalia.openPath(target)
        end,
      }),
    }))
  end
end

local function toggleCheck(lineNo)
  local out = {}
  local n = 0
  for line in (buffer .. "\n"):gmatch("(.-)\n") do
    n += 1
    if n == lineNo then
      if line:match("^%s*[-*+]%s+%[ %]") then
        line = line:gsub("%[ %]", "[x]", 1)
      else
        line = line:gsub("%[[xX]%]", "[ ]", 1)
      end
    end
    table.insert(out, line)
  end
  buffer = table.concat(out, "\n")
  dirty = true
  flush()
  editorRev += 1
  render()
end

local function previewView()
  local rows = {}
  local inCode = false
  local lineNo = 0
  for raw in (buffer .. "\n"):gmatch("(.-)\n") do
    lineNo += 1
    local line = raw:gsub("\r$", "")
    local key = "line-" .. lineNo
    if line:match("^%s*```") then
      inCode = not inCode
    elseif inCode then
      table.insert(rows, ui.row({ key = key, fill = "surface_variant", paddingH = 8, paddingV = 1 }, {
        ui.label({ text = line == "" and " " or line, fontSize = 13, fontFamily = "mono", color = "on_surface", flexGrow = 1 }),
      }))
    elseif line:match("^%s*$") then
      table.insert(rows, ui.row({ key = key, height = 4 }, {}))
    elseif line:match("^%s*([-*_])%s*%1%s*%1[%s%-%*_]*$") then
      table.insert(rows, ui.separator({ key = key, spacing = 4 }))
    else
      local level, heading = line:match("^(#+)%s+(.*)$")
      local indent, mark, task = line:match("^(%s*)[-*+]%s+%[([ xX])%]%s*(.*)$")
      local bulletIndent, bullet = line:match("^(%s*)[-*+]%s+(.*)$")
      local numberIndent, number, numbered = line:match("^(%s*)(%d+)[.)]%s+(.*)$")
      local quote = line:match("^%s*>%s?(.*)$")
      if level ~= nil then
        table.insert(rows, textNode(heading, { key = key, fontSize = #level <= 2 and 16 or 13, fontWeight = "bold", color = #level == 1 and "primary" or "on_surface" }))
        if #level == 1 then
          table.insert(rows, ui.separator({ key = key .. "-rule", spacing = 2, color = "primary" }))
        end
      elseif mark ~= nil then
        local done = mark ~= " "
        local at = lineNo
        table.insert(rows, ui.row({ key = key .. (done and "-done" or ""), gap = 6, align = "center", paddingH = #indent * 6 }, {
          ui.button({
            glyph = done and "square-check" or "square",
            variant = "ghost",
            controlSize = "sm",
            onClick = function()
              toggleCheck(at)
            end,
          }),
          textNode(task, { flexGrow = 1, color = done and "on_surface_variant" or "on_surface" }),
        }))
      elseif bullet ~= nil then
        table.insert(rows, ui.row({ key = key, gap = 6, paddingH = #bulletIndent * 6 }, {
          ui.label({ text = "•", fontSize = editorSize, color = "primary" }),
          textNode(bullet, { flexGrow = 1 }),
        }))
      elseif numbered ~= nil then
        table.insert(rows, ui.row({ key = key, gap = 6, paddingH = #numberIndent * 6 }, {
          ui.label({ text = number .. ".", fontSize = editorSize, color = "primary" }),
          textNode(numbered, { flexGrow = 1 }),
        }))
      elseif quote ~= nil then
        table.insert(rows, ui.row({ key = key, gap = 8 }, {
          ui.row({ width = 2, fill = "primary" }, {}),
          textNode(quote, { flexGrow = 1, color = "on_surface_variant" }),
        }))
      else
        local onlyPictures = line:gsub("!%[.-%]%(.-%)", ""):match("^%s*$") ~= nil
        if not onlyPictures then
          local whole = line:match("^%s*%*%*(.-)%*%*%s*$")
          table.insert(rows, textNode(whole or line, { key = key, fontWeight = whole and "bold" or nil }))
        end
      end
      lineExtras(line, rows, lineNo)
    end
  end
  if buffer:match("^%s*$") then
    table.insert(rows, ui.label({ text = tr("editor_placeholder"), color = "on_surface_variant" }))
  end
  return ui.scroll({ key = "preview-" .. (current or ""), flexGrow = 1, gap = 4 }, rows)
end

local function editorView()
  local body
  if previewing then
    body = previewView()
  else
    body = ui.input({
      -- The rev reseeds the (uncontrolled) editor when the file changed under us.
      key = editorKey(),
      value = buffer,
      multiline = true,
      focus = true,
      flexGrow = 1,
      fontSize = editorSize,
      fontFamily = editorFont,
      placeholder = tr("editor_placeholder"),
      onChange = "onEdited",
      onSubmit = "onFlushNow",
      onPasteImage = "onPasteImage",
      onPasteFiles = "onDrop",
    })
  end
  return ui.column({ flexGrow = 1, gap = 8, align = "stretch" }, {
    editorHeader(),
    body,
    ui.label({
      text = editorStatusLine(),
      fontSize = 11,
      color = "on_surface_variant",
    }),
  })
end

render = function()
  local isEditor = view == "editor" and current ~= nil
  local header = {}
  if view == "trash" then
    table.insert(header, ui.button({ key = "header-back", glyph = "arrow-left", variant = "ghost", onClick = "onBack" }))
    table.insert(header, ui.label({ text = tr("trash"), fontSize = 16, fontWeight = "bold", color = "on_surface", flexGrow = 1 }))
    if #trashed > 0 then
      if confirmEmpty then
        table.insert(header, ui.label({ text = tr("confirm_empty"), color = "error" }))
        table.insert(header, ui.button({ key = "header-empty-yes", glyph = "check", variant = "primary", onClick = "onEmptyTrash" }))
        table.insert(header, ui.button({
          key = "header-empty-no",
          glyph = "close",
          variant = "ghost",
          onClick = function()
            confirmEmpty = false
            render()
          end,
        }))
      else
        table.insert(header, ui.button({
          key = "header-empty",
          glyph = "trash-x",
          variant = "ghost",
          onClick = function()
            confirmEmpty = true
            render()
          end,
        }))
      end
    end
  else
    table.insert(header, ui.label({ text = tr("title"), fontSize = 16, fontWeight = "bold", color = "on_surface", flexGrow = 1 }))
    if not isEditor then
      table.insert(header, ui.button({
        key = "header-sort",
        glyph = sortByDate and "history" or "sort-ascending-letters",
        variant = "ghost",
        onClick = "onToggleSort",
      }))
      table.insert(header, ui.button({ key = "header-folder", glyph = "folder-open", variant = "ghost", onClick = "onOpenFolder" }))
      table.insert(header, ui.button({ key = "header-trash", glyph = "trash", variant = "ghost", onClick = "onShowTrash" }))
    end
    table.insert(header, ui.button({ key = "header-new", glyph = "plus", variant = "primary", onClick = "onNewNote" }))
  end
  table.insert(header, ui.button({ key = "header-close", glyph = "close", variant = "ghost", onClick = "onClosePanel" }))
  local body
  if view == "trash" then
    body = trashView()
  elseif isEditor then
    body = editorView()
  else
    body = listView()
  end
  panel.render(ui.column({ flexGrow = 1, gap = 12, align = "stretch", onDrop = "onDrop" }, {
    ui.row({ align = "center", gap = 8 }, header),
    body,
  }))
end

function onOpen(_context)
  notesDir = noctalia.expandPath(noctalia.getConfig("notes_dir"))
  cornerRadiusScale = noctalia.getSetting("shell.corner_radius_scale") or 0
  editorFont = noctalia.getConfig("editor_font") or "pixel"
  editorSize = tonumber(noctalia.getConfig("editor_size")) or 16
  local extension = noctalia.string.trim(noctalia.getConfig("extension")):gsub("^%.", "")
  if extension == "" then
    noctalia.notifyError(tr("title"), tr("bad_extension"))
    extension = "md"
  end
  suffix = "." .. extension
  -- A note left open under a different extension setting no longer matches.
  if current ~= nil and not hasSuffix(current) then
    current = nil
    buffer = ""
    dirty = false
    view = "list"
  end
  local ok, err = noctalia.mkdirAll(notesDir)
  if not ok then
    noctalia.notifyError(tr("title"), err or "")
  end
  loadPins()
  refreshFiles()
  pendingDelete = nil
  confirmEmpty = false
  panel.setWantsSecondTicks(true)
  -- A note open when the panel last closed is read again (it may have changed since).
  if view == "editor" and current ~= nil then
    if noctalia.fileExists(notePath(current)) or current == scratchName() then
      openNote(current)
      return
    end
    leaveNote()
  end
  render()
end

function onOpenScratchpad()
  openNote(scratchName())
end

function onClose()
  settleName()
end

function update()
  if dirty then
    idleTicks += 1
    if idleTicks >= AUTOSAVE_IDLE_TICKS then
      flush()
      render()
    end
  end
end

function onEdited(value)
  buffer = value
  idleTicks = 0
  dirty = true
  -- Re-render on every edit: the status line carries the live word/char count.
  render()
end

function onNameTyped(value)
  pendingName = value
end

function onToggleSort()
  sortByDate = not sortByDate
  refreshFiles()
  render()
end

function onSearch(value)
  search = value
  refreshFiles()
  render()
end

function onFlushNow(value)
  buffer = value
  dirty = true
  flush()
  render()
end

function onNewNote()
  if current ~= nil then
    settleName()
  end
  local name = uniqueNoteName()
  local ok, err = noctalia.writeFile(notePath(name), "")
  if not ok then
    noctalia.notifyError(tr("title"), err or tr("save_failed"))
    return
  end
  autoNamed[name] = true
  saveAutoNamed()
  search = ""
  refreshFiles()
  openNote(name)
end

function onRowHover(state, key)
  hoveredKey = (state == "true") and key or nil
  render()
end

function onRename(value)
  if current == nil then
    return
  end
  pendingName = nil
  if autoNamed[current] ~= nil then
    autoNamed[current] = nil
    saveAutoNamed()
  end
  renameTo(value, false)
  refreshFiles()
  render()
end

function onEditorDelete()
  pendingDelete = current
  render()
end

function onTogglePreview()
  flush()
  previewing = not previewing
  render()
end

function onOpenInNotepad()
  if current == nil then
    return
  end
  flush()
  local ok, err = noctalia.openPath(notePath(current), "notepad")
  if not ok then
    noctalia.notifyError(tr("title"), err or "")
  end
end

function onOpenFolder()
  local ok, err = noctalia.openPath(notesDir)
  if not ok then
    noctalia.notifyError(tr("title"), err or "")
  end
end

function onShowTrash()
  view = "trash"
  pendingDelete = nil
  confirmEmpty = false
  refreshTrash()
  render()
end

function onEmptyTrash()
  for _, file in ipairs(trashed) do
    noctalia.removeFile(notesDir .. "/" .. TRASH .. "/" .. file)
  end
  confirmEmpty = false
  refreshTrash()
  render()
end

-- Ctrl+V with a picture (and no text) copied: saved as a PNG in images\ and linked where the caret is.
function onPasteImage()
  if current == nil then
    return
  end
  local dir = notesDir .. "/" .. IMAGES
  noctalia.mkdirAll(dir)
  local name = uniqueIn(dir, noctalia.formatTime("%Y-%m-%d %H.%M.%S"), ".png")
  local ok, err = noctalia.saveClipboardImage(dir .. "/" .. name)
  if not ok then
    noctalia.notifyError(tr("title"), tr("paste_failed") .. (err and (": " .. err) or ""))
    return
  end
  insertIntoNote("![" .. name:gsub("%.png$", "") .. "](<" .. IMAGES .. "/" .. name .. ">)")
end

-- Files dropped on the panel (or pasted after copying them in File Explorer). In a note: copied
-- in and linked. In the list: note files become notes, anything else goes into a new note.
function onDrop(...)
  local paths = { ... }
  if #paths == 0 or notesDir == "" then
    return
  end
  if view == "editor" and current ~= nil then
    local links = {}
    for _, path in ipairs(paths) do
      local link = attach(path)
      if link ~= nil then
        table.insert(links, link)
      end
    end
    insertIntoNote(table.concat(links, "\n"))
    return
  end
  local others = {}
  for _, path in ipairs(paths) do
    local name = path:match("[^\\/]+$") or ""
    if hasSuffix(name) or name:lower():match("%.txt$") then
      local base = name:match("^(.*)%.[^.]+$") or name
      local target = uniqueIn(notesDir, safeName(base) ~= "" and safeName(base) or tr("untitled"), suffix)
      local ok, err = noctalia.copyFile(path, notePath(target))
      if not ok then
        noctalia.notifyError(tr("title"), tr("copy_failed", { name = name }) .. (err and (": " .. err) or ""))
      end
    else
      table.insert(others, path)
    end
  end
  if #others > 0 then
    local name = uniqueNoteName()
    local links = {}
    for _, path in ipairs(others) do
      local link = attach(path)
      if link ~= nil then
        table.insert(links, link)
      end
    end
    noctalia.writeFile(notePath(name), table.concat(links, "\n") .. "\n")
    autoNamed[name] = true
    saveAutoNamed()
    refreshFiles()
    openNote(name)
    return
  end
  view = "list"
  refreshFiles()
  render()
end

function onBack()
  if view == "trash" then
    view = "list"
    pendingDelete = nil
    refreshFiles()
    render()
    return
  end
  leaveNote()
  refreshFiles()
  render()
end

function onClosePanel()
  panel.close()
end

-- The launcher entry bumps "notes_bump" after writing note files (quick-capture
-- appends, new notes). Fold external changes into the open editor: an appended
-- tail is grafted onto whatever is being typed; other divergence only replaces
-- the buffer when there are no unsaved edits.
local function onExternalChange()
  if notesDir == "" then
    return -- never opened; nothing resolved yet
  end
  refreshFiles()
  if current ~= nil then
    local contents = noctalia.readFile(notePath(current)) or ""
    if contents ~= loadedContent then
      if contents:sub(1, #loadedContent) == loadedContent then
        buffer = buffer .. contents:sub(#loadedContent + 1)
      elseif not dirty then
        buffer = contents
      end
      loadedContent = contents
      dirty = buffer ~= loadedContent
      idleTicks = 0
      editorRev += 1
    end
  end
  render()
end

noctalia.state.watch("notes_bump", function(_value)
  onExternalChange()
end)
