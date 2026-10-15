--!nonstrict
-- Keybind Guide: every key Tourne takes, on one searchable sheet. The idea is kenn's Keybind Cheatsheet for
-- Noctalia (MIT): binds grouped by the "# Heading" comments above them and by key mode, numbered binds as one
-- row, a search over keys, descriptions and commands, and an edit mode to hide rows or name them.
-- The binds are read when the guide opens (tourne.keybinds: the Switcher's lines, its binds file, Tourne's own
-- hotkeys); nothing runs while it's closed.

local editing = false
local search = ""
local renaming = nil -- a row's id while its name is being typed
local prefs = nil    -- { hidden = { id = true }, names = { id = "text" } }
local sections = {}  -- { { title, rows = { { id, keys = { "Win", "Alt", "Q" }, text, command } } } }
local render

-- What each command does, in words (as the Keys and rules window says it).
local DOES = {
  exec = "Run", execr = "Run", killactive = "Close the window", forcekillactive = "Force the program to quit",
  closewindow = "Close a window", killwindow = "Force a window's program to quit",
  togglefloating = "Float or tile the window", setfloating = "Float the window", settiled = "Tile the window",
  fullscreen = "Maximise or restore", minimize = "Minimise", pin = "Keep on top (or not)", centerwindow = "Centre the window",
  movefocus = "Focus to the", movewindow = "Move the window", swapwindow = "Swap with the window to the",
  resizeactive = "Resize the window", moveactive = "Move the window by", resizewindowpixel = "Resize a window",
  movewindowpixel = "Move a window", cyclenext = "Next window", focuswindow = "Focus", focusmonitor = "Focus screen",
  focuscurrentorlast = "Back to the window before", focusurgentorlast = "The window wanting attention",
  workspace = "Go to desktop", movetoworkspace = "Send the window to desktop",
  movetoworkspacesilent = "Send the window to desktop (stay)", renameworkspace = "Rename desktop",
  togglespecialworkspace = "Show or hide the scratchpad", layoutmsg = "Layout:", splitratio = "Split",
  togglegroup = "Make or break a tab group", changegroupactive = "Next tab", moveintogroup = "Into the group to the",
  moveoutofgroup = "Out of the group", submap = "Key mode:", sendshortcut = "Send keys", alterzorder = "Window order",
  bringactivetotop = "Bring to the top", movecursor = "Move the pointer", movecursortocorner = "Pointer to a corner",
  dpms = "Screens", lockscreen = "Lock the PC", overview = "Every window (overview)", pickcolor = "Copy the colour under the pointer",
}
local SIDES = { l = "left", r = "right", u = "up", d = "down" }

local KEYS = {
  space = "Space", ["return"] = "Enter", enter = "Enter", tab = "Tab", escape = "Esc", esc = "Esc", backspace = "Backspace",
  delete = "Del", insert = "Ins", home = "Home", ["end"] = "End", prior = "PgUp", pageup = "PgUp", next = "PgDn",
  pagedown = "PgDn", left = "←", right = "→", up = "↑", down = "↓", print = "PrtSc", pause = "Pause", menu = "Menu",
  minus = "-", equal = "=", comma = ",", period = ".", slash = "/", semicolon = ";", apostrophe = "'", grave = "`",
  bracketleft = "[", bracketright = "]", backslash = "\\", catchall = "any key",
  xf86audioraisevolume = "Vol +", xf86audiolowervolume = "Vol -", xf86audiomute = "Mute", xf86audioplay = "Play",
  xf86audiopause = "Play", xf86audionext = "Next", xf86audioprev = "Prev", xf86audiostop = "Stop",
}
-- Windows' key codes, for hotkeys written as the Studio writes them ("ctrl+alt+77").
local VK = {
  [8] = "Backspace", [9] = "Tab", [13] = "Enter", [27] = "Esc", [32] = "Space", [33] = "PgUp", [34] = "PgDn", [35] = "End",
  [36] = "Home", [37] = "←", [38] = "↑", [39] = "→", [40] = "↓", [44] = "PrtSc", [45] = "Ins", [46] = "Del", [93] = "Menu",
  [186] = ";", [187] = "=", [188] = ",", [189] = "-", [190] = ".", [191] = "/", [192] = "`", [219] = "[", [220] = "\\",
  [221] = "]", [222] = "'",
}

local function modName(m)
  local symbols = noctalia.getConfig("modifierStyle") == "symbols"
  if m == "win" or m == "super" or m == "mod4" or m == "logo" or m == "meta" or m == "windows" then return symbols and "⊞" or "Win" end
  if m == "ctrl" or m == "control" then return symbols and "⌃" or "Ctrl" end
  if m == "alt" or m == "mod1" then return symbols and "⎇" or "Alt" end
  if m == "shift" then return symbols and "⇧" or "Shift" end
  return nil
end
local MOD_ORDER = { Win = 1, ["⊞"] = 1, Ctrl = 2, ["⌃"] = 2, Alt = 3, ["⎇"] = 3, Shift = 4, ["⇧"] = 4 }

local function keyName(k)
  local low = string.lower(k)
  if KEYS[low] then return KEYS[low] end
  local code = string.match(low, "^code:(%d+)$")
  if code then return VK[tonumber(code)] or ("key " .. code) end
  if #k == 1 then return string.upper(k) end
  if string.match(low, "^f%d+$") then return string.upper(k) end
  return k
end

local function trim(s)
  return (string.gsub(s or "", "^%s*(.-)%s*$", "%1"))
end

local function split(text, parts)
  local out, at = {}, 1
  while #out + 1 < parts do
    local comma = string.find(text, ",", at, true)
    if not comma then break end
    table.insert(out, trim(string.sub(text, at, comma - 1)))
    at = comma + 1
  end
  table.insert(out, trim(string.sub(text, at)))
  return out
end

-- "WIN ALT" or "ctrl+alt" as cap names, Win first.
local function mods(text)
  local caps = {}
  for word in string.gmatch(string.lower(text or ""), "[^%s_+]+") do
    local n = modName(word)
    if n then table.insert(caps, n) end
  end
  table.sort(caps, function(a, b) return (MOD_ORDER[a] or 9) < (MOD_ORDER[b] or 9) end)
  return caps
end

local function describe(command, arg)
  local does = DOES[command]
  if not does then return command .. (arg ~= "" and (" " .. arg) or "") end
  if command == "exec" or command == "execr" then
    local shown = string.gsub(arg, "^%[.-%]%s*", "")
    shown = string.match(shown, "([^\\/\"]+)%.exe") or shown
    return "Run " .. shown
  end
  if SIDES[arg] then return does .. " " .. SIDES[arg] end
  if command == "dpms" then return "Screens " .. (arg ~= "" and arg or "toggle") end
  if arg == "" then return does end
  return does .. " " .. arg
end

local function prefsFile()
  return (noctalia.pluginDataDir() or "") .. "\\preferences.json"
end

local function loadPrefs()
  if prefs then return end
  local saved = noctalia.readFile(prefsFile())
  prefs = saved and noctalia.json.decode(saved) or {}
  prefs.hidden = prefs.hidden or {}
  prefs.names = prefs.names or {}
end

local function savePrefs()
  noctalia.mkdirAll(noctalia.pluginDataDir() or "")
  noctalia.writeFile(prefsFile(), noctalia.json.encode(prefs))
end

-- The binds, read from the lines as the Switcher reads them.
local function readBinds(lines, fromFile)
  local names = {}
  local mode, heading = "", nil
  local out = {} -- { section, keys, text, command, arg, id, order }
  for _, raw in lines do
    local line = trim(raw)
    if line == "" then
      -- (a blank line keeps the heading: headings group what's under them)
    elseif string.sub(line, 1, 1) == "#" then
      local h = trim(string.gsub(line, "^#+", ""))
      h = string.gsub(h, "^%d+[%.%)]%s*", "") -- "# 1. Apps"
      if #h > 0 and #h <= 40 then heading = h end
    elseif string.sub(line, 1, 1) == "$" then
      local name, value = string.match(line, "^(%$[%w_]+)%s*=%s*(.-)$")
      if name then names[name] = value end
    else
      local keyword, rest = string.match(line, "^([%w_]+)%s*=%s*(.*)$")
      keyword = string.lower(keyword or "")
      if keyword == "submap" then
        local m = trim(rest)
        mode = (string.lower(m) == "reset") and "" or m
      elseif string.sub(keyword, 1, 4) == "bind" and not string.find(keyword, "m", 5, true) then
        -- The $names filled in, longest first.
        local sorted = {}
        for n, v in names do table.insert(sorted, { n, v }) end
        table.sort(sorted, function(a, b) return #a[1] > #b[1] end)
        for _, nv in sorted do rest = string.gsub(rest, (string.gsub(nv[1], "%$", "%%$")), (string.gsub(nv[2], "%%", "%%%%"))) end
        local described = string.find(keyword, "d", 5, true) ~= nil
        local parts = split(rest, described and 5 or 4)
        if #parts >= 3 then
          local i = 3
          local desc = nil
          if described and #parts >= 4 then desc = parts[3]; i = 4 end
          local command = string.lower(parts[i] or "")
          local arg = parts[i + 1] or ""
          local caps = mods(parts[1])
          table.insert(caps, keyName(parts[2]))
          local flags = string.sub(keyword, 5)
          local section = mode ~= "" and ("Key mode: " .. mode) or heading or (fromFile and "From the binds file" or "Your binds")
          table.insert(out, {
            section = section, keys = caps, text = desc or describe(command, arg), command = command .. (arg ~= "" and (", " .. arg) or ""),
            held = string.find(flags, "e", 1, true) ~= nil, released = string.find(flags, "r", 1, true) ~= nil,
            key = parts[2], mods = parts[1], cmd = command, arg = arg, described = desc ~= nil,
          })
        end
      end
    end
  end
  return out
end

-- Rows the same but for a digit in the key and the argument: one row ("1-9").
local function merge(rows)
  if not noctalia.getConfig("mergeNumbered") then return rows end
  local out, i = {}, 1
  while i <= #rows do
    local r = rows[i]
    local d = tonumber(r.key)
    local j = i
    if d and #r.key == 1 and string.find(r.arg, tostring(d), 1, true) then
      while j + 1 <= #rows do
        local n = rows[j + 1]
        local nd = tonumber(n.key)
        if not (nd and #n.key == 1 and n.section == r.section and n.cmd == r.cmd and n.mods == r.mods and not n.described and not r.described
          and string.gsub(n.arg, tostring(nd), "N") == string.gsub(r.arg, tostring(d), "N")) then break end
        j += 1
      end
    end
    if j > i + 1 then
      local first, last = rows[i].key, rows[j].key
      local m = table.clone(r)
      m.keys = table.clone(r.keys)
      m.keys[#m.keys] = first .. "-" .. last
      local arg = string.gsub(r.arg, tostring(d), "N")
      m.text = string.gsub(describe(r.cmd, arg), "N", first .. "-" .. last)
      m.command = r.cmd .. ", " .. arg
      table.insert(out, m)
      i = j + 1
    else
      table.insert(out, r)
      i += 1
    end
  end
  return out
end

-- Tourne's own hotkeys ("ctrl+alt+77", "Ctrl+Alt+N", "win+alt+1-9").
local function hotkeyCaps(text)
  local caps, key = {}, nil
  for part in string.gmatch(text, "[^+]+") do
    local p = trim(part)
    local m = modName(string.lower(p))
    if m then table.insert(caps, m) else key = p end
  end
  table.sort(caps, function(a, b) return (MOD_ORDER[a] or 9) < (MOD_ORDER[b] or 9) end)
  if key then
    local code = tonumber(key)
    if code then
      if code >= 65 and code <= 90 or code >= 48 and code <= 57 then key = string.char(code)
      elseif code >= 112 and code <= 135 then key = "F" .. (code - 111)
      else key = VK[code] or ("key " .. code) end
    else
      key = keyName(key)
    end
    table.insert(caps, key)
  end
  return caps
end

local WINDOWS = {
  { { "Win" }, "Start" }, { { "Win", "E" }, "File Explorer" }, { { "Win", "D" }, "Show the desktop" },
  { { "Win", "L" }, "Lock the PC" }, { { "Win", "Tab" }, "Task view" }, { { "Win", "V" }, "Clipboard history" },
  { { "Win", "Shift", "S" }, "Snip part of the screen" }, { { "Win", "." }, "Emoji" }, { { "Win", "I" }, "Settings" },
  { { "Win", "R" }, "Run" }, { { "Win", "X" }, "Quick link menu" }, { { "Win", "↑" }, "Maximise" },
  { { "Win", "←" }, "Snap left" }, { { "Win", "→" }, "Snap right" }, { { "Win", "Ctrl", "←" }, "Desktop before" },
  { { "Win", "Ctrl", "→" }, "Next desktop" }, { { "Win", "Ctrl", "D" }, "New desktop" }, { { "Alt", "Tab" }, "Switch windows" },
  { { "Alt", "F4" }, "Close the window" }, { { "Ctrl", "Shift", "Esc" }, "Task Manager" },
}

local function build()
  loadPrefs()
  local kb = tourne.keybinds()
  local rows = readBinds(kb.lines or {}, false)
  for _, r in readBinds(kb.fileLines or {}, true) do table.insert(rows, r) end
  rows = merge(rows)
  if noctalia.getConfig("showTourne") then
    for _, h in kb.hotkeys or {} do
      table.insert(rows, { section = "Tourne", keys = hotkeyCaps(h.keys), text = h.name, command = "" })
    end
  end
  if noctalia.getConfig("showWindows") then
    for _, w in WINDOWS do
      local caps = {}
      for _, k in w[1] do table.insert(caps, modName(string.lower(k)) or k) end
      table.insert(rows, { section = "Windows' own", keys = caps, text = w[2], command = "" })
    end
  end
  sections = {}
  local bySection = {}
  for _, r in rows do
    r.id = table.concat(r.keys, "+") .. "|" .. r.command .. "|" .. r.section
    if prefs.names[r.id] then r.text = prefs.names[r.id] end
    local s = bySection[r.section]
    if not s then
      s = { title = r.section, rows = {} }
      bySection[r.section] = s
      table.insert(sections, s)
    end
    table.insert(s.rows, r)
  end
  return kb
end

local function cap(text, i)
  return ui.row({ key = "c" .. i, fill = "surface_variant", border = "outline_variant", radius = 4, padding = 3, align = "center" }, {
    ui.label({ text = text, fontSize = 11, fontWeight = "semibold", fontFamily = "Consolas", color = "on_surface" }),
  })
end

local function rowView(r)
  local hidden = prefs.hidden[r.id] == true
  local caps = {}
  for i, k in r.keys do
    if i > 1 then table.insert(caps, ui.label({ key = "p" .. i, text = "+", fontSize = 10, color = "on_surface_variant" })) end
    table.insert(caps, cap(k, i))
  end
  if r.held then table.insert(caps, ui.label({ key = "held", text = "hold", fontSize = 9, color = "on_surface_variant" })) end
  if r.released then table.insert(caps, ui.label({ key = "rel", text = "let go", fontSize = 9, color = "on_surface_variant" })) end
  local words = {}
  if renaming == r.id then
    table.insert(words, ui.input({ key = "name", value = r.text, focus = true, onChange = function(v)
      prefs.names[r.id] = trim(v) ~= "" and v or nil
      savePrefs()
    end }))
  else
    table.insert(words, ui.label({ key = "t", text = r.text, fontSize = 12, color = hidden and "on_surface_variant/0.5" or "on_surface", maxLines = 2 }))
  end
  if noctalia.getConfig("showCommands") and r.command ~= "" then
    table.insert(words, ui.label({ key = "cmd", text = r.command, fontSize = 10, fontFamily = "Consolas", color = "on_surface_variant", maxLines = 1 }))
  end
  local line = {
    ui.row({ key = "caps", gap = 3, align = "center", minWidth = 150 }, caps),
    ui.column({ key = "words", gap = 1, flexGrow = 1 }, words),
  }
  if editing then
    table.insert(line, ui.button({ key = "ren", glyph = renaming == r.id and "check" or "pencil", variant = "ghost", controlSize = "sm",
      tooltip = renaming == r.id and "Done" or "Name it", onClick = function()
        renaming = renaming ~= r.id and r.id or nil
        render()
      end }))
    table.insert(line, ui.button({ key = "eye", glyph = hidden and "eye-off" or "eye", variant = "ghost", controlSize = "sm",
      tooltip = hidden and "Show it" or "Hide it", onClick = function()
        prefs.hidden[r.id] = not hidden or nil
        savePrefs()
        render()
      end }))
  end
  return ui.row({ key = r.id, gap = 8, align = "center", padding = 2 }, line)
end

local function matches(r)
  if search == "" then return true end
  local hay = string.lower(table.concat(r.keys, " ") .. " " .. r.text .. " " .. r.command .. " " .. r.section)
  return string.find(hay, search, 1, true) ~= nil
end

render = function()
  local kb = build()
  -- The sections shown, then shared out over the columns by their rows.
  local shown = {}
  local total = 0
  for _, s in sections do
    local rows = {}
    for _, r in s.rows do
      if (editing or not prefs.hidden[r.id]) and matches(r) then table.insert(rows, r) end
    end
    if #rows > 0 then
      table.insert(shown, { title = s.title, rows = rows })
      total += #rows + 2
    end
  end
  local n = math.clamp(tonumber(noctalia.getConfig("columns")) or 3, 1, 4)
  local columns, filled, col = {}, {}, 1
  for c = 1, n do columns[c] = {}; filled[c] = 0 end
  for _, s in shown do
    -- The next section goes in the column with the fewest rows so far.
    local best = 1
    for c = 2, n do if filled[c] < filled[best] then best = c end end
    col = best
    local items = { ui.label({ key = "h", text = s.title, fontSize = 13, fontWeight = "bold", color = "primary" }) }
    for _, r in s.rows do table.insert(items, rowView(r)) end
    table.insert(columns[col], ui.column({ key = "s" .. s.title, gap = 3, padding = 4 }, items))
    filled[col] += #s.rows + 2
  end
  local body = {}
  for c = 1, n do table.insert(body, ui.column({ key = "col" .. c, gap = 10, flexGrow = 1 }, columns[c])) end
  local empty = #shown == 0
  local note = nil
  if empty then
    note = search ~= "" and "Nothing matches that." or
      ((#(kb.lines or {}) == 0 and #(kb.fileLines or {}) == 0) and "No key binds yet. Make some in Keys and rules (no config file needed)."
        or "Every row is hidden. Turn on editing (the pencil) to show them again.")
  elseif not kb.enabled and (#(kb.lines or {}) > 0 or #(kb.fileLines or {}) > 0) then
    note = "Key binds are off, so the binds below don't work yet: turn them on in Keys and rules."
  end
  panel.render(ui.column({ flexGrow = 1, gap = 8, padding = 12 }, {
    ui.row({ key = "head", gap = 8, align = "center" }, {
      ui.glyph({ name = "keyboard", size = 18, color = "primary" }),
      ui.label({ text = "Keybind Guide", fontSize = 15, fontWeight = "bold" }),
      ui.input({ key = "search", placeholder = "Search keys, what they do, commands…", focus = true, flexGrow = 1, onChange = function(v)
        search = string.lower(trim(v))
        render()
      end }),
      ui.button({ key = "edit", glyph = editing and "check" or "pencil", variant = editing and "primary" or "ghost",
        tooltip = editing and "Done editing" or "Hide rows or name them", onClick = function()
          editing = not editing
          renaming = nil
          render()
        end }),
      ui.button({ key = "keys", text = "Keys and rules…", variant = "outline", tooltip = "Make and change binds, without a config file",
        onClick = function() tourne.openKeysAndRules() end }),
    }),
    ui.label({ key = "note", text = note or "", visible = note ~= nil, fontSize = 12, color = "on_surface_variant" }),
    ui.scroll({ key = "body", flexGrow = 1 }, { ui.row({ key = "cols", gap = 16, align = "start" }, body) }),
  }))
end

function onOpen(_context)
  search = ""
  editing = false
  renaming = nil
  render()
end
