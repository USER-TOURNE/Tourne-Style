--!nonstrict
-- Tourne Desktop's own Caffeine, after the DMS Caffeine plugins: how long to stay awake. The tray icon's entry
-- does the keeping; this only sets state "awake".

local CHOICES = {
  { "5 min", 5 }, { "10 min", 10 }, { "15 min", 15 }, { "30 min", 30 },
  { "1 hour", 60 }, { "2 hours", 120 }, { "4 hours", 240 }, { "Infinite", 0 },
}

local function render()
  local s = noctalia.state.get("awake")
  local status
  if not s then status = "Off: the PC sleeps as usual"
  elseif s["until"] == 0 then status = "Awake until you stop it"
  else
    local m = math.ceil(math.max(0, s["until"] - noctalia.nowMs()) / 60000)
    status = m >= 60 and string.format("Awake for %dh %02dm more", m // 60, m % 60) or string.format("Awake for %d min more", m)
  end
  local rows = {}
  for i = 1, #CHOICES, 2 do
    local row = {}
    for j = i, i + 1 do
      local c = CHOICES[j]
      table.insert(row, ui.button({ text = c[1], glyph = c[2] == 0 and "infinity" or "clock", variant = "ghost", flexGrow = 1,
        onClick = function()
          tourne.uiSound("on")
          noctalia.state.set("awake", { ["until"] = c[2] > 0 and noctalia.nowMs() + c[2] * 60000 or 0 })
          panel.close()
        end }))
    end
    table.insert(rows, ui.row({ gap = 6 }, row))
  end
  panel.render(ui.column({ flexGrow = 1, gap = 8, padding = 14 }, {
    ui.row({ gap = 8, align = "center" }, {
      ui.glyph({ name = s and "coffee" or "coffee-off", size = 22, color = s and "primary" or "on_surface_variant" }),
      ui.label({ text = "Caffeine", fontSize = 15, fontWeight = "semibold", color = "on_surface" }),
    }),
    ui.label({ text = status, color = "on_surface_variant" }),
    ui.column({ gap = 6 }, rows),
    ui.button({ text = "Turn off", glyph = "coffee-off", variant = s and "primary" or "ghost", onClick = function()
      tourne.uiSound("off")
      noctalia.state.set("awake", false)
      panel.close()
    end }),
  }))
end

function onOpen(_context)
  render()
end

noctalia.state.watch("awake", function() render() end)
