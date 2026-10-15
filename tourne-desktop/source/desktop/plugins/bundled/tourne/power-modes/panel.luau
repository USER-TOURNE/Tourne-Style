--!nonstrict
-- Power Modes: mode, plan and turbo.

local MODES = {
  { id = "efficiency", name = "Best power efficiency", glyph = "leaf" },
  { id = "balanced", name = "Balanced", glyph = "scale" },
  { id = "performance", name = "Best performance", glyph = "rocket" },
}

local note = ""
local render

local function told(what, ok, why)
  if ok == false then
    note = tostring(why)
  else
    note = what
    if noctalia.getConfig("confirmToast") then noctalia.notify("Power", what) end
  end
  render()
end

local function card(title, subtitle, children)
  local inner = { ui.label({ text = title, color = "on_surface", fontWeight = "semibold" }) }
  if subtitle then table.insert(inner, ui.label({ text = subtitle, fontSize = 12, color = "on_surface_variant" })) end
  for _, c in children do table.insert(inner, c) end
  return ui.column({ gap = 6, padding = 10, radius = 8, fill = "surface_variant" }, inner)
end

render = function()
  local mode = tourne.powerMode()
  local modeRows = {}
  for _, m in MODES do
    table.insert(modeRows, ui.button({ key = m.id, text = m.name, glyph = m.glyph, contentAlign = "start",
      variant = mode == m.id and "primary" or "ghost", onClick = function()
        local ok, why = tourne.setPowerMode(m.id)
        told("Power mode: " .. m.name, ok, why)
      end }))
  end
  local plans = tourne.powerPlans()
  local planNames, planAt = {}, 0
  for i, p in plans do
    table.insert(planNames, p.name)
    if p.active then planAt = i - 1 end
  end
  local turbo = tourne.turbo()
  local cards = {
    card("Power mode", mode and nil or "This plan has no power modes", modeRows),
    card("Power plan", nil, {
      ui.select({ options = planNames, selectedIndex = planAt, onChange = function(i)
        local p = plans[(tonumber(i) or 0) + 1]
        if p then
          local ok, why = tourne.setPowerPlan(p.id)
          told("Power plan: " .. p.name, ok, why)
        end
      end }),
    }),
  }
  if turbo ~= nil then
    table.insert(cards, card("CPU turbo", turbo and "On (the processor may boost)" or "Off (base clock, cooler and quieter)", {
      ui.toggle({ checked = turbo, onChange = function(v)
        local ok, why = tourne.setTurbo(v == "true")
        told("CPU turbo " .. (v == "true" and "on" or "off"), ok, why)
      end }),
    }))
  end
  table.insert(cards, ui.label({ text = note, visible = note ~= "", fontSize = 12, color = "on_surface_variant" }))
  panel.render(ui.column({ flexGrow = 1, gap = 10, padding = 12 }, cards))
end

function onOpen(_context)
  note = ""
  render()
end
