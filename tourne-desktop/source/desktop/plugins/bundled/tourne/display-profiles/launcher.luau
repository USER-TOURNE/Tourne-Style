--!nonstrict
-- Display profiles: each screen's modes (tourne.displays()) and saved profiles, kept in the plugin's folder as
-- profiles.json. Typing "save NAME" saves how every screen is set now.

local function dataDir() return noctalia.pluginDataDir() or "" end

local function profiles()
  local path = dataDir() .. "/profiles.json"
  if not noctalia.fileExists(path) then return {} end
  return noctalia.json.decode(noctalia.readFile(path) or "{}") or {}
end

local function saveProfiles(p)
  noctalia.mkdirAll(dataDir())
  noctalia.writeFile(dataDir() .. "/profiles.json", noctalia.json.encode(p))
end

local actions = {}  -- result id: what Enter does

local function mode(d) return `{d.width}×{d.height} at {d.hz} Hz` end

function onQuery(text)
  local query = noctalia.string.trim(text)
  local results = {}
  actions = {}
  local function add(id, title, subtitle, glyph, act)
    if query ~= "" and query:sub(1, 5) ~= "save " then
      local score = noctalia.fuzzyScore(query, `{title} {subtitle}`)
      if not score then return end
    end
    actions[id] = act
    table.insert(results, { id = id, title = title, subtitle = subtitle, glyph = glyph })
  end
  local screens = tourne.displays() or {}
  if query:sub(1, 5) == "save " and #query > 5 then
    local name = query:sub(6)
    add("save", `Save "{name}"`, "How every screen is set now, as a profile", "device-floppy", function()
      local p = profiles()
      local set = {}
      for _, d in screens do table.insert(set, { name = d.name, width = d.width, height = d.height, hz = d.hz }) end
      p[name] = set
      saveProfiles(p)
      noctalia.notify("Display Profiles", `Saved "{name}"`)
    end)
  end
  for name, set in profiles() do
    local parts = {}
    for _, d in set do table.insert(parts, mode(d)) end
    add("p:" .. name, name, table.concat(parts, ", "), "bookmark", function()
      local failed = 0
      for _, d in set do
        if not tourne.setDisplayMode(d.name, d.width, d.height, d.hz) then failed += 1 end
      end
      if failed > 0 then noctalia.notifyError("Display Profiles", `{failed} screens didn't take "{name}"`) end
    end)
  end
  for i, d in screens do
    local label = `Screen {i}{d.primary and " (main)" or ""}`
    for j, m in d.modes do
      if j > 12 then break end  -- (the biggest few: the list goes on to sizes nobody wants)
      local now = m.width == d.width and m.height == d.height and m.hz == d.hz
      add(`m:{i}:{j}`, `{label}: {mode(m)}`, now and "Now" or d.label, now and "check" or "device-desktop", function()
        if not tourne.setDisplayMode(d.name, m.width, m.height, m.hz) then
          noctalia.notifyError("Display Profiles", `Windows didn't take {mode(m)}`)
        end
      end)
    end
  end
  if #results == 0 then
    table.insert(results, { id = "help", title = "Type save NAME to keep how the screens are set now", glyph = "info-circle" })
  end
  launcher.setResults(text, results)
end

function onActivate(id)
  local act = actions[id]
  if act then act() end
end
