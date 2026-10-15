--!nonstrict
-- Tourne Desktop's own Alienware Command Center, after psyreactor's DMS plugin.
-- The headless [[service]] entry: it owns every talk with the laptop and publishes one JSON
-- snapshot under "awcc.snapshot"; the bar widget and the panel send it commands through
-- "awcc.cmd" ({n, action, arg, arg2}).
--
-- The laptop is reached through awcc.ps1 (PowerShell and the AWCCWmiMethodFunction WMI class).
-- Windows usually wants administrator rights for those calls, so there are three routes:
--   direct   the call works as the user (no prompt)
--   task     the user set up the helper task once ("Don't ask each time"); schtasks runs it
--   elevate  every change asks Windows for administrator rights (a UAC prompt)

local TASK = "\\Tourne\\AlienwareCommandCenter"

local DIR = noctalia.pluginDir()
local DATA = noctalia.pluginDataDir() or DIR
local PS1 = DIR .. "\\awcc.ps1"
local REQ = DATA .. "\\request.json"
local RES = DATA .. "\\result.json"
local PREFS = DATA .. "\\prefs.json"

local snap = {
  phase = "loading", -- loading | ready | none | needsAdmin | error
  route = "direct",
  taskInstalled = false,
  busy = false,
  maker = "",
  model = "",
  modes = {},
  fans = {},
  current = nil,
  gmode = nil,
  note = "",
  error = "",
}

local queue = {}
local running = nil -- {op, arg, arg2, nonce, started, viaFile, timeout}
local nonceSeq = 0
local lastPoll = 0
local restored = false
local prefs = {}

----------------------------------------------------------------------------------------------
-- Small helpers

local function publish()
  noctalia.state.set("awcc.snapshot", noctalia.json.encode(snap))
end

local function loadPrefs()
  local text = noctalia.readFile(PREFS)
  local data = text and noctalia.json.decode(text)
  prefs = type(data) == "table" and data or {}
end

local function savePrefs()
  noctalia.writeFile(PREFS, noctalia.json.encode(prefs))
end

local function toast(text)
  if noctalia.getConfig("confirmToast") ~= false then noctalia.notify("Alienware Command Center", text) end
end

-- The last line of output that is a JSON object.
local function parseOutput(out)
  local found = nil
  for line in string.gmatch(out or "", "[^\r\n]+") do
    local t = noctalia.string.trim(line)
    if string.sub(t, 1, 1) == "{" then found = t end
  end
  if not found then return nil end
  local data = noctalia.json.decode(found)
  return type(data) == "table" and data or nil
end

-- UTF-8 to UTF-16LE to Base64, for PowerShell's -EncodedCommand.
local B64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
local function encodeCommand(text)
  local bytes = {}
  for _, cp in utf8.codes(text) do
    if cp >= 0x10000 then
      local v = cp - 0x10000
      local hi, lo = 0xD800 + bit32.rshift(v, 10), 0xDC00 + bit32.band(v, 0x3FF)
      table.insert(bytes, bit32.band(hi, 0xFF)); table.insert(bytes, bit32.rshift(hi, 8))
      table.insert(bytes, bit32.band(lo, 0xFF)); table.insert(bytes, bit32.rshift(lo, 8))
    else
      table.insert(bytes, bit32.band(cp, 0xFF)); table.insert(bytes, bit32.rshift(cp, 8))
    end
  end
  local out = {}
  for i = 1, #bytes, 3 do
    local a, b, c = bytes[i], bytes[i + 1], bytes[i + 2]
    local n = a * 65536 + (b or 0) * 256 + (c or 0)
    local c1 = bit32.extract(n, 18, 6) + 1
    local c2 = bit32.extract(n, 12, 6) + 1
    local c3 = bit32.extract(n, 6, 6) + 1
    local c4 = bit32.extract(n, 0, 6) + 1
    table.insert(out, string.sub(B64, c1, c1) .. string.sub(B64, c2, c2)
      .. (b and string.sub(B64, c3, c3) or "=") .. (c and string.sub(B64, c4, c4) or "="))
  end
  return table.concat(out)
end

local function psQuote(s)
  return "'" .. string.gsub(s, "'", "''") .. "'"
end

local function modeName(key)
  for _, m in snap.modes do
    if m.key == key then return m.name end
  end
  return key
end

----------------------------------------------------------------------------------------------
-- Results

local pump

local function restoreBoosts()
  -- Like the original: a fan reading 0 after a restart gets the boost it had last time.
  if restored or snap.route == "elevate" then return end
  restored = true
  for _, fan in snap.fans do
    local saved = prefs["boost" .. tostring(fan.id)]
    if (tonumber(fan.boost) or 0) == 0 and (tonumber(saved) or 0) > 0 then
      table.insert(queue, { op = "boost", arg = tostring(fan.id), arg2 = tostring(saved), quiet = true })
    end
  end
end

local function apply(job, res)
  snap.busy = false
  if not res then
    snap.note = ""
    snap.error = "Couldn't read what the helper said."
    if snap.phase == "loading" then snap.phase = "error" end
    publish()
    return
  end
  if res.cancelled then
    snap.note = "Not changed: Windows didn't get administrator rights."
    publish()
    return
  end
  if res.maker then snap.maker = res.maker end
  if res.model then snap.model = res.model end
  if res.awcc == false then
    snap.phase = "none"
    snap.modes, snap.fans, snap.current = {}, {}, nil
    publish()
    return
  end
  if res.needsAdmin then
    if snap.taskInstalled and snap.route ~= "task" then
      snap.route = "task"
      table.insert(queue, 1, { op = job.op, arg = job.arg, arg2 = job.arg2, quiet = job.quiet })
      return
    end
    if snap.route ~= "task" then snap.route = "elevate" end
    if snap.phase ~= "ready" then snap.phase = "needsAdmin" end
    snap.note = "Windows needs administrator rights for this."
    publish()
    return
  end
  if res.modes then snap.modes = res.modes end
  if res.fans then snap.fans = res.fans end
  if res.current then snap.current = res.current end
  if res.gmode ~= nil then snap.gmode = res.gmode end
  snap.phase = "ready"
  snap.error = res.error or ""
  if res.ok == false then
    snap.note = res.error or "That didn't work."
  elseif res.did then
    snap.note = res.did
    if job.op == "mode" or job.op == "install" or job.op == "uninstall" then toast(res.did) end
  end
  if job.op == "install" and res.ok ~= false then
    snap.taskInstalled = true
    snap.route = "task"
  elseif job.op == "uninstall" and res.ok ~= false then
    snap.taskInstalled = false
    snap.route = "direct"
    table.insert(queue, { op = "info", quiet = true })
  end
  restoreBoosts()
  publish()
end

-- Waiting on result.json (the helper ran as administrator, by UAC or by the task).
local function checkResultFile()
  local job = running
  if not job or not job.viaFile then return end
  local text = noctalia.readFile(RES)
  local res = text and parseOutput(text)
  if res and tostring(res.nonce or "") == job.nonce then
    running = nil
    apply(job, res)
    pump()
    return
  end
  if noctalia.nowMs() - job.started > job.timeout then
    running = nil
    snap.busy = false
    snap.note = job.route == "elevate" and "Not changed: no answer from the administrator prompt."
      or "The helper task didn't answer. Try \"Run as administrator\", or set the task up again."
    if job.route == "task" then
      snap.route = "elevate"
      if snap.phase ~= "ready" then snap.phase = "needsAdmin" end
    end
    publish()
    pump()
  end
end

----------------------------------------------------------------------------------------------
-- Running the helper

local function helperArgs(job)
  local args = { "-Op", job.op }
  if job.arg and job.arg ~= "" then table.insert(args, "-Arg"); table.insert(args, job.arg) end
  if job.arg2 and job.arg2 ~= "" then table.insert(args, "-Arg2"); table.insert(args, job.arg2) end
  return args
end

local function runDirect(job)
  local argv = { "powershell", "-NoProfile", "-NonInteractive", "-ExecutionPolicy", "Bypass", "-File", PS1 }
  for _, a in helperArgs(job) do table.insert(argv, a) end
  local ok = noctalia.runAsync(argv, function(r)
    running = nil
    local res = parseOutput(r.stdout)
    if not res and r.timedOut then
      snap.busy = false
      snap.note = "The laptop didn't answer in time."
      publish()
    else
      apply(job, res)
    end
    pump()
  end, 45000)
  if not ok then
    running = nil
    snap.busy = false
    snap.phase = snap.phase == "loading" and "error" or snap.phase
    snap.error = "PowerShell couldn't be started."
    publish()
  end
end

local function runTask(job)
  noctalia.writeFile(REQ, noctalia.json.encode({ nonce = job.nonce, op = job.op, arg = job.arg or "", arg2 = job.arg2 or "" }))
  noctalia.removeFile(RES)
  job.viaFile, job.timeout, job.route = true, 30000, "task"
  noctalia.runAsync({ "schtasks", "/Run", "/TN", TASK }, function(r)
    if r.exitCode ~= 0 and running == job then
      running = nil
      snap.busy = false
      snap.taskInstalled = false
      snap.route = "elevate"
      if snap.phase ~= "ready" then snap.phase = "needsAdmin" end
      snap.note = "The helper task is gone. Set it up again, or run as administrator."
      publish()
      pump()
    end
  end, 20000)
end

-- One UAC prompt: a hidden PowerShell asks Windows to start the helper as administrator. If the
-- prompt is refused, it writes a "cancelled" result so the panel can say so straight away.
local function runElevated(job)
  noctalia.removeFile(RES)
  job.viaFile, job.route = true, "elevate"
  job.timeout = 120000
  local inner = "-NoProfile -NonInteractive -ExecutionPolicy Bypass -WindowStyle Hidden -File \"" .. PS1 .. "\""
  for _, a in helperArgs(job) do inner ..= " \"" .. a .. "\"" end
  inner ..= " -Out \"" .. RES .. "\" -Nonce \"" .. job.nonce .. "\""
  local script = "try { Start-Process -FilePath powershell -Verb RunAs -WindowStyle Hidden -ArgumentList "
    .. psQuote(inner) .. " -ErrorAction Stop } catch { [IO.File]::WriteAllText(" .. psQuote(RES)
    .. ", '{\"nonce\":\"" .. job.nonce .. "\",\"ok\":false,\"cancelled\":true}') }"
  local ok, why = tourne.launch({ "powershell", "-NoProfile", "-WindowStyle", "Hidden", "-EncodedCommand", encodeCommand(script) })
  if ok == false then
    running = nil
    snap.busy = false
    snap.note = "Couldn't ask for administrator rights: " .. tostring(why)
    publish()
  end
end

pump = function()
  if running or #queue == 0 then return end
  local job = table.remove(queue, 1)
  nonceSeq += 1
  job.nonce = tostring(noctalia.nowMs()) .. "-" .. tostring(nonceSeq)
  job.started = noctalia.nowMs()
  running = job
  if job.op ~= "info" and job.op ~= "current" then
    snap.busy = true
    publish()
  end
  local route = job.forceRoute or snap.route
  if job.op == "install" or job.op == "uninstall" then route = "elevate" end
  if route == "task" then
    runTask(job)
  elseif route == "elevate" then
    runElevated(job)
  else
    runDirect(job)
  end
end

-- Later commands of the same kind replace waiting ones (a slider dragged twice sends once).
local function enqueue(job)
  for i = #queue, 1, -1 do
    local q = queue[i]
    if q.op == job.op and (job.op ~= "boost" or q.arg == job.arg) then table.remove(queue, i) end
  end
  table.insert(queue, job)
  pump()
end

local function checkTask(done)
  noctalia.runAsync({ "schtasks", "/Query", "/TN", TASK }, function(r)
    snap.taskInstalled = r.exitCode == 0
    if done then done() end
  end, 20000)
end

----------------------------------------------------------------------------------------------
-- Commands

local function command(action, arg, arg2)
  if action == "refresh" then
    enqueue({ op = "info" })
  elseif action == "peek" then
    -- (the panel opening: read again, but never with an administrator prompt)
    if snap.route ~= "elevate" then enqueue({ op = "info", quiet = true }) end
  elseif action == "mode" and arg then
    if snap.phase == "none" then return end
    enqueue({ op = "mode", arg = arg })
  elseif action == "boost" and arg and arg2 then
    if snap.phase == "none" then return end
    prefs["boost" .. tostring(arg)] = tonumber(arg2) or 0
    savePrefs()
    enqueue({ op = "boost", arg = tostring(arg), arg2 = tostring(arg2) })
  elseif action == "elevate" then
    -- "Run as administrator": read everything once with the rights.
    enqueue({ op = "info", forceRoute = "elevate" })
  elseif action == "install" then
    local user = noctalia.getenv("USERNAME") or ""
    local domain = noctalia.getenv("USERDOMAIN") or ""
    if domain ~= "" then user = domain .. "\\" .. user end
    enqueue({ op = "install", arg = REQ, arg2 = user })
  elseif action == "uninstall" then
    enqueue({ op = "uninstall" })
  end
end

noctalia.state.watch("awcc.cmd", function(raw)
  local cmd = raw and noctalia.json.decode(raw) or nil
  if type(cmd) ~= "table" or not cmd.action then return end
  command(cmd.action, cmd.arg, cmd.arg2)
end)

-- IPC: TourneDesktop.exe --msg plugin tourne/alienware-command-center:service all mode quiet
function onIpc(event, payload)
  if event == "refresh" then
    command("refresh")
  elseif event == "mode" then
    command("mode", noctalia.string.trim(tostring(payload or "")))
  end
end

function onConfigChanged()
  noctalia.setUpdateInterval(1000)
end

function update()
  noctalia.setUpdateInterval(1000)
  checkResultFile()
  -- Keep the current mode in step (it can change from the laptop's own key or from AWCC).
  local every = math.clamp(tonumber(noctalia.getConfig("refreshInterval")) or 60, 5, 600) * 1000
  if noctalia.nowMs() - lastPoll >= every then
    lastPoll = noctalia.nowMs()
    if not running and #queue == 0 and snap.phase == "ready" and snap.route ~= "elevate" then
      enqueue({ op = "current" })
    end
  end
end

loadPrefs()
publish()
lastPoll = noctalia.nowMs()
checkTask(function()
  enqueue({ op = "info" })
end)
noctalia.setUpdateInterval(1000)
