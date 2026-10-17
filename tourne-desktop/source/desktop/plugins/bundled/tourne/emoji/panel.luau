--!nonstrict
-- Emoji Picker: every Unicode emoji (emoji.tsv, from muan/unicode-emoji-json, MIT), by group or by name.
-- The idea is hthienloc's Emoji Picker (DankMaterialShell); its data is GPL, so none of it is used here.
-- The panel draws emoji in one colour (Windows' classic text drawing); the copy is the real emoji.

local PER_ROW = 8
local SHOWN = 240
local all = nil -- { { emoji, lower-case name, group } }
local groups = {}
local group = 1
local search = ""
local recent = {}
local note = ""
local render

local function recentFile()
  return (noctalia.pluginDataDir() or "") .. "\\recent.json"
end

local function loadAll()
  if all then return end
  all = {}
  local seen = {}
  for line in string.gmatch(noctalia.readFile("emoji.tsv") or "", "[^\n]+") do
    if string.sub(line, 1, 1) ~= "#" then
      local g, e, name = string.match(line, "^(.-)\t(.-)\t(.*)$")
      if g then
        table.insert(all, { e, string.lower(name), g, name })
        if not seen[g] then
          seen[g] = true
          table.insert(groups, g)
        end
      end
    end
  end
  local saved = noctalia.readFile(recentFile())
  recent = saved and noctalia.json.decode(saved) or {}
end

local function copy(e)
  noctalia.copyToClipboard(e)
  for i = #recent, 1, -1 do
    if recent[i] == e then table.remove(recent, i) end
  end
  table.insert(recent, 1, e)
  while #recent > PER_ROW * 2 do table.remove(recent) end
  noctalia.mkdirAll(noctalia.pluginDataDir() or "")
  noctalia.writeFile(recentFile(), noctalia.json.encode(recent))
  if noctalia.getConfig("closeOnCopy") then panel.close() else note = "Copied " .. e; render() end
end

local function grid(list, prefix)
  local rows, row = {}, {}
  for i, item in list do
    table.insert(row, ui.button({ key = prefix .. i, text = item[1], tooltip = item[4], variant = "ghost", fontSize = 20, width = 44, height = 40,
      onClick = function() copy(item[1]) end }))
    if #row == PER_ROW then
      table.insert(rows, ui.row({ key = prefix .. "r" .. i, gap = 2 }, row))
      row = {}
    end
  end
  if #row > 0 then table.insert(rows, ui.row({ key = prefix .. "rl", gap = 2 }, row)) end
  return rows
end

render = function()
  loadAll()
  local found = {}
  for _, item in all do
    if (search ~= "" and string.find(item[2], search, 1, true)) or (search == "" and item[3] == groups[group]) then
      table.insert(found, item)
      if #found >= SHOWN then break end
    end
  end
  local body = {}
  if search == "" and #recent > 0 then
    local items = {}
    for _, e in recent do table.insert(items, { e, "", "", e }) end
    table.insert(body, ui.label({ text = "Recent", fontSize = 12, color = "on_surface_variant" }))
    for _, r in grid(items, "recent") do table.insert(body, r) end
    table.insert(body, ui.separator({ spacing = 4 }))
  end
  for _, r in grid(found, "e") do table.insert(body, r) end
  if #found == 0 then table.insert(body, ui.label({ text = "Nothing for that", color = "on_surface_variant" })) end
  panel.render(ui.column({ flexGrow = 1, gap = 8, padding = 10 }, {
    ui.input({ key = "search", placeholder = "Search by name: heart, cat, party…", focus = true, onChange = function(v)
      search = string.lower(noctalia.string.trim(v))
      render()
    end }),
    ui.select({ options = groups, selectedIndex = group - 1, enabled = search == "", onChange = function(i)
      group = (tonumber(i) or 0) + 1
      render()
    end }),
    ui.scroll({ key = "list", flexGrow = 1, gap = 2 }, body),
    ui.label({ text = note, visible = note ~= "", fontSize = 11, color = "on_surface_variant" }),
  }))
end

function onOpen(_context)
  note = ""
  render()
end
