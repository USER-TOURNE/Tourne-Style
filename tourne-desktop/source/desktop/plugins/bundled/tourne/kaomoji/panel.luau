--!nonstrict
-- Kaomoji Picker: from hthienloc's Kaomoji Picker (DankMaterialShell, MIT), rewritten for Tourne
-- Desktop, with its database trimmed to about 3,200 (kaomoji.tsv: the popular ones and up to 60 per tag).

local PER_ROW = 3
local SHOWN = 90
local all = nil -- { { text, tags } }
local tags = {} -- the most used tags, for the chips
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
  local counts = {}
  for line in string.gmatch(noctalia.readFile("kaomoji.tsv") or "", "[^\n]+") do
    if string.sub(line, 1, 1) ~= "#" then
      local text, tagText = string.match(line, "^(.-)\t(.*)$")
      if text then
        table.insert(all, { text, string.lower(tagText) })
        for t in string.gmatch(tagText, "[^,]+") do counts[t] = (counts[t] or 0) + 1 end
      end
    end
  end
  for t, n in counts do
    if n >= 40 and #t < 12 then table.insert(tags, t) end
  end
  table.sort(tags, function(a, b) return counts[a] > counts[b] end)
  while #tags > 14 do table.remove(tags) end
  local saved = noctalia.readFile(recentFile())
  recent = saved and noctalia.json.decode(saved) or {}
end

local function copy(text)
  noctalia.copyToClipboard(text)
  for i = #recent, 1, -1 do
    if recent[i] == text then table.remove(recent, i) end
  end
  table.insert(recent, 1, text)
  while #recent > PER_ROW * 3 do table.remove(recent) end
  noctalia.mkdirAll(noctalia.pluginDataDir() or "")
  noctalia.writeFile(recentFile(), noctalia.json.encode(recent))
  if noctalia.getConfig("closeOnCopy") then panel.close() else note = "Copied " .. text; render() end
end

local function grid(list, prefix)
  local rows, row = {}, {}
  for i, text in list do
    table.insert(row, ui.button({ key = prefix .. i, text = text, variant = "ghost", flexGrow = 1, onClick = function() copy(text) end }))
    if #row == PER_ROW then
      table.insert(rows, ui.row({ key = prefix .. "r" .. i, gap = 4 }, row))
      row = {}
    end
  end
  if #row > 0 then table.insert(rows, ui.row({ key = prefix .. "rl", gap = 4 }, row)) end
  return rows
end

render = function()
  loadAll()
  local found = {}
  for _, k in all do
    if search == "" or string.find(k[2], search, 1, true) then
      table.insert(found, k[1])
      if #found >= SHOWN then break end
    end
  end
  local chips = {}
  for _, t in tags do
    table.insert(chips, ui.button({ key = "t-" .. t, text = t, controlSize = "sm", variant = search == t and "primary" or "outline", onClick = function()
      search = search == t and "" or t
      render()
    end }))
  end
  local body = {}
  if search == "" and #recent > 0 then
    table.insert(body, ui.label({ text = "Recent", fontSize = 12, color = "on_surface_variant" }))
    for _, r in grid(recent, "recent") do table.insert(body, r) end
    table.insert(body, ui.separator({ spacing = 4 }))
  end
  for _, r in grid(found, "k") do table.insert(body, r) end
  if #found == 0 then table.insert(body, ui.label({ text = "Nothing for that", color = "on_surface_variant" })) end
  panel.render(ui.column({ flexGrow = 1, gap = 8, padding = 10 }, {
    ui.input({ key = "search", placeholder = "Search: happy, cat, table flip…", focus = true, onChange = function(v)
      search = string.lower(noctalia.string.trim(v))
      render()
    end }),
    ui.row({ gap = 4 }, { table.unpack(chips, 1, 7) }),
    ui.row({ gap = 4 }, { table.unpack(chips, 8, 14) }),
    ui.scroll({ key = "list", flexGrow = 1, gap = 4 }, body),
    ui.label({ text = note, visible = note ~= "", fontSize = 11, color = "on_surface_variant" }),
  }))
end

function onOpen(_context)
  note = ""
  render()
end
