--!nonstrict
-- Process List: what's running, with CPU and memory, grouped by program.
--
-- Written for Tourne Desktop from the idea of Mithgroth's Process List (a DankMaterialShell desktop
-- widget, MIT). Windows hands every process over in one call (tourne.processes), so a refresh opens no
-- process at all. "Apps" are processes in your session; "Windows" ones are services (session 0).

local SORTS = { "cpu", "memory", "name", "count", "pid" }
local SORT_NAMES = { "CPU", "Memory", "Name", "Count", "PID" }
local SCOPES = { "all", "apps", "windows" }
local SCOPE_NAMES = { "All", "Apps", "Windows" }

local sortIndex = 1
local scopeIndex = 1
local search = ""
local expanded = {}
local confirmPid = nil
local list = {}
local ticks = 0
local note = ""

local render

local function cfg(key, fallback)
  local v = noctalia.getConfig(key)
  if v == nil then return fallback end
  return v
end

local function refresh()
  list = tourne.processes()
  render()
end

local function matches(p)
  local scope = SCOPES[scopeIndex]
  if scope == "apps" and p.system then return false end
  if scope == "windows" and not p.system then return false end
  if cfg("hideIdle", false) and p.cpu < 0.05 then return false end
  if search ~= "" and not string.find(string.lower(p.name), search, 1, true) then return false end
  return true
end

local function rowsOf()
  local groups, order = {}, {}
  for _, p in list do
    if matches(p) then
      local key = cfg("groupByName", true) and string.lower(p.name) or tostring(p.pid)
      local g = groups[key]
      if not g then
        g = { name = p.name, pid = p.pid, cpu = 0, mem = 0, count = 0, user = 0, system = 0, items = {} }
        groups[key] = g
        table.insert(order, g)
      end
      g.cpu += p.cpu
      g.mem += p.memMb
      g.count += 1
      if p.system then g.system += 1 else g.user += 1 end
      table.insert(g.items, p)
    end
  end
  local sort = SORTS[sortIndex]
  table.sort(order, function(a, b)
    if sort == "cpu" and a.cpu ~= b.cpu then return a.cpu > b.cpu end
    if sort == "memory" and a.mem ~= b.mem then return a.mem > b.mem end
    if sort == "count" and a.count ~= b.count then return a.count > b.count end
    if sort == "pid" then return a.pid < b.pid end
    return string.lower(a.name) < string.lower(b.name)
  end)
  return order
end

local function memText(mb)
  if mb >= 1024 then return string.format("%.1f GB", mb / 1024) end
  return string.format("%d MB", math.floor(mb + 0.5))
end

local function endButton(pid)
  if not cfg("allowEnd", false) then return nil end
  if confirmPid == pid then
    return ui.button({ text = "End?", variant = "destructive", controlSize = "sm", onClick = function()
      local ok, why = tourne.endProcess(pid)
      note = ok and ("Ended " .. pid) or tostring(why)
      confirmPid = nil
      refresh()
    end })
  end
  return ui.button({ glyph = "x", variant = "ghost", controlSize = "sm", tooltip = "End it", onClick = function()
    confirmPid = pid
    render()
  end })
end

local function line(key, name, color, count, cpu, mem, pid, onClick, indent)
  local children = {
    ui.label({ text = name, flexGrow = 1, maxLines = 1, color = color }),
    ui.label({ text = count, width = 32, textAlign = "end", color = "on_surface_variant", fontSize = 12 }),
    ui.label({ text = string.format("%.1f%%", cpu), width = 56, textAlign = "end", color = cpu >= 20 and "error" or "on_surface" }),
    ui.label({ text = memText(mem), width = 70, textAlign = "end", color = "on_surface" }),
  }
  local b = pid and endButton(pid)
  if b then table.insert(children, b) end
  return ui.row({ key = key, gap = 6, align = "center", paddingH = indent or 0, onClick = onClick }, children)
end

render = function()
  local rows = {}
  local limit = cfg("rows", 60)
  for _, g in rowsOf() do
    if #rows >= limit then break end
    local color = g.system > 0 and g.user == 0 and "on_surface_variant" or (g.user > 0 and g.system == 0 and "primary" or "on_surface")
    local key = string.lower(g.name)
    if g.count > 1 then
      table.insert(rows, line("g-" .. key, (expanded[key] and "▾ " or "▸ ") .. g.name, color, tostring(g.count), g.cpu, g.mem, nil, function()
        expanded[key] = not expanded[key]
        render()
      end))
      if expanded[key] then
        table.sort(g.items, function(a, b) return a.cpu > b.cpu end)
        for _, p in g.items do
          table.insert(rows, line("p-" .. p.pid, tostring(p.pid), p.system and "on_surface_variant" or "primary", "", p.cpu, p.memMb, p.pid, nil, 18))
        end
      end
    else
      local p = g.items[1]
      table.insert(rows, line("p-" .. p.pid, g.name, color, "", g.cpu, g.mem, p.pid))
    end
  end
  if #rows == 0 then table.insert(rows, ui.label({ text = "Nothing matches", color = "on_surface_variant" })) end
  local s = noctalia.systemStats()
  panel.render(ui.column({ flexGrow = 1, gap = 8, padding = 12 }, {
    ui.row({ gap = 8, align = "center" }, {
      ui.glyph({ name = "list-details", size = 18, color = "primary" }),
      ui.label({ text = s and string.format("CPU %d%%   Memory %.1f / %.1f GB   %d processes", math.floor(s.cpu.usagePercent + 0.5),
        s.ram.usedMb / 1024, s.ram.totalMb / 1024, #list) or "Processes", flexGrow = 1, color = "on_surface" }),
    }),
    ui.row({ gap = 6, align = "center" }, {
      ui.select({ options = SCOPE_NAMES, selectedIndex = scopeIndex - 1, controlSize = "sm", onChange = function(i)
        scopeIndex = (tonumber(i) or 0) + 1
        render()
      end }),
      ui.select({ options = SORT_NAMES, selectedIndex = sortIndex - 1, controlSize = "sm", onChange = function(i)
        sortIndex = (tonumber(i) or 0) + 1
        render()
      end }),
      ui.input({ key = "search", placeholder = "Search", controlSize = "sm", flexGrow = 1, onChange = function(v)
        search = string.lower(v)
        render()
      end }),
    }),
    ui.row({ gap = 6, align = "center" }, {
      ui.label({ text = "Name", flexGrow = 1, fontSize = 11, color = "on_surface_variant" }),
      ui.label({ text = "#", width = 32, textAlign = "end", fontSize = 11, color = "on_surface_variant" }),
      ui.label({ text = "CPU", width = 56, textAlign = "end", fontSize = 11, color = "on_surface_variant" }),
      ui.label({ text = "Memory", width = 70, textAlign = "end", fontSize = 11, color = "on_surface_variant" }),
    }),
    ui.scroll({ key = "rows", flexGrow = 1, gap = 2 }, rows),
    ui.label({ text = note, visible = note ~= "", fontSize = 11, color = "on_surface_variant" }),
  }))
end

function onOpen(_context)
  ticks = 0
  note = ""
  confirmPid = nil
  panel.setWantsSecondTicks(true)
  refresh()
end

function update()
  ticks += 1
  if ticks >= cfg("refreshSeconds", 2) then
    ticks = 0
    refresh()
  end
end
