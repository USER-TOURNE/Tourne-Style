--!nonstrict
-- Bar button for the process list; with showText, CPU and memory use beside it.

local function show()
  barWidget.setGlyph("list-details")
  local s = noctalia.systemStats()
  if s and noctalia.getConfig("showText") then
    barWidget.setText(string.format("%d%% · %d%%", math.floor(s.cpu.usagePercent + 0.5), s.ram.usagePercent))
  else
    barWidget.setText("")
  end
  if s then
    barWidget.setTooltip(string.format("Processes\nCPU %d%%\nMemory %.1f of %.1f GB", math.floor(s.cpu.usagePercent + 0.5),
      s.ram.usedMb / 1024, s.ram.totalMb / 1024))
  else
    barWidget.setTooltip("Processes")
  end
end

function update()
  show()
end

function onClick()
  noctalia.togglePanel("tourne/process-list:panel")
end

noctalia.setUpdateInterval(noctalia.getConfig("showText") and 2000 or 10000)
show()
