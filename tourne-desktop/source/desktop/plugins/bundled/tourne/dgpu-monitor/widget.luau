--!nonstrict
-- dGPU Sleep Monitor: the discrete graphics card's power state on the bar, and the battery's draw.
--
-- Idea from xantrk's dGPU Sleep Monitor (DankMaterialShell, no licence given, so no code of it is used).
-- Linux reads the PCI power state from sysfs; Windows keeps the same thing as the device's most recent
-- power state (tourne.gpus): D0 is awake, D3 is asleep. supergfxctl and cardwire mode switches have no
-- Windows counterpart, so they're left out.

local function dgpu()
  local gpus = tourne.gpus()
  for _, g in gpus do
    if g.discrete then return g, gpus end
  end
  return nil, gpus
end

local function show()
  local g = dgpu()
  local text = g and (g.active and "dGPU" or "dGPU zz") or "no dGPU"
  local tip = g and string.format("%s\n%s (%s)", g.name, g.active and "Awake" or "Asleep", g.powerState) or "No discrete graphics card found"
  local b = noctalia.getConfig("showBattery") and tourne.battery() or nil
  if b and b.rateW then
    text ..= string.format(" · %.1f W", math.abs(b.rateW))
    tip ..= string.format("\nBattery %d%%, %s %.1f W", b.percent, b.rateW < 0 and "drawing" or "charging at", math.abs(b.rateW))
  end
  barWidget.setGlyph("cpu")
  barWidget.setText(text)
  barWidget.setTooltip(tip)
  barWidget.setColor(g and g.active and "primary" or "on_surface_variant")
end

function update()
  show()
end

function onClick()
  noctalia.togglePanel("tourne/dgpu-monitor:panel")
end

noctalia.setUpdateInterval((noctalia.getConfig("refreshInterval") or 5) * 1000)
show()
