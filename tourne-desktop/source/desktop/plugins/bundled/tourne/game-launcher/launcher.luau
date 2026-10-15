--!nonstrict
-- Steam's and Epic's installed games (tourne.games(): their manifests on this PC), launched through their stores.
-- The list is read when the launcher asks, at most once a minute.

local games = {}
local readAt = -1e9

local function list()
  if os.clock() - readAt > 60 or #games == 0 then
    games = tourne.games() or {}
    table.sort(games, function(a, b) return a.name:lower() < b.name:lower() end)
    readAt = os.clock()
  end
  return games
end

function onQuery(text)
  local query = noctalia.string.trim(text)
  local results = {}
  for i, g in list() do
    local score = 0
    if query ~= "" then
      score = noctalia.fuzzyScore(query, g.name)
      if not score then continue end
    end
    table.insert(results, { id = tostring(i), title = g.name, subtitle = `{g.store} · Enter to play`,
                            glyph = "device-gamepad-2", score = score })
  end
  if #results == 0 then
    table.insert(results, { id = "none", title = query == "" and "No Steam or Epic games found" or "No game by that name",
                            glyph = "mood-empty" })
  end
  launcher.setResults(text, results)
end

function onActivate(id)
  local g = games[tonumber(id) or 0]
  if not g then return end
  if not tourne.launchGame(g.launch) then noctalia.notifyError("Game Launcher", `{g.store} didn't start {g.name}`) end
end
