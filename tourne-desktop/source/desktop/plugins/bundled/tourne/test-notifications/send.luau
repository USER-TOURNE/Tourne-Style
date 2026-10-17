--!nonstrict
-- Test Notifications: what's sent and how (shared by the bar button and its panel).
--
-- On this PC: a real Windows notification from an app of its own ("Test Notifications", registered for the
-- current user under HKCU\Software\Classes\AppUserModelId\Tourne.TestNotifications), so Tourne draws its pop-up
-- and keeps it in the notification centre like any app's. On the phone: a push to an ntfy topic (ntfy.sh), which
-- the ntfy app shows. Windows PowerShell does both in one run (plugins have no notification library yet), spaced
-- by the interval setting so each pair arrives together.

local M = {}

M.PRIORITIES = { "min", "low", "default", "high", "urgent" }
M.TARGETS = { "This PC", "Phone", "Both" }

local function dataDir() return noctalia.pluginDataDir() or "" end

-- The choices made in the panel (kept in the plugin's own folder).
function M.load()
  local c = { count = 1, priority = "default", target = "This PC", title = "Test notification", message = "", topic = "" }
  local path = dataDir() .. "/choices.json"
  if noctalia.fileExists(path) then
    local saved = noctalia.json.decode(noctalia.readFile(path) or "{}") or {}
    for k, v in saved do c[k] = v end
  end
  c.count = math.clamp(tonumber(c.count) or 1, 1, 20)
  return c
end

function M.save(c)
  noctalia.mkdirAll(dataDir())
  noctalia.writeFile(dataDir() .. "/choices.json", noctalia.json.encode(c))
end

-- Text for PowerShell inside single quotes.
local function quoted(s) return "'" .. (tostring(s):gsub("'", "''")) .. "'" end

local function flag(b) return b and "$true" or "$false" end

-- One send as chosen; told(text, failed) once it's on its way, and again if it failed.
function M.send(c, told)
  local gap = math.clamp(tonumber(noctalia.getConfig("interval")) or 2, 1, 30)
  gap = math.max(0, math.min(gap, math.floor(50 / math.max(1, c.count - 1))))  -- (PowerShell is given a minute at most)
  local pc = c.target ~= "Phone"
  local phone = c.target ~= "This PC"
  if phone and (c.topic or "") == "" then
    told("No ntfy topic yet: right-click the button and fill one in", true)
    phone = false
    if not pc then return end
  end
  local lines = {
    "$ErrorActionPreference='Stop'",
    `$count={c.count}; $gap={gap}; $title={quoted(c.title)}; $message={quoted(c.message)}`,
    `$pc={flag(pc)}; $phone={flag(phone)}; $topic={quoted(c.topic or "")}; $priority={quoted(c.priority)}`,
    `$urgent={flag(c.priority == "urgent")}; $quiet={flag(c.priority == "min" or c.priority == "low")}`,
    "if ($pc) {",
    "  $id='Tourne.TestNotifications'; $k='HKCU:\\Software\\Classes\\AppUserModelId\\'+$id",
    "  if (-not (Test-Path $k)) { New-Item $k -Force | Out-Null }",
    "  Set-ItemProperty $k DisplayName 'Test Notifications'",
    "  [Windows.UI.Notifications.ToastNotificationManager, Windows.UI.Notifications, ContentType = WindowsRuntime] | Out-Null",
    "  [Windows.Data.Xml.Dom.XmlDocument, Windows.Data.Xml.Dom.XmlDocument, ContentType = WindowsRuntime] | Out-Null",
    "  $n=[Windows.UI.Notifications.ToastNotificationManager]::CreateToastNotifier($id)",
    "}",
    "$failed=@()",
    "for ($i=1; $i -le $count; $i++) {",
    "  $t = if ($count -gt 1) { \"$title ($i/$count)\" } else { $title }",
    "  $m = if ($message) { $message } else { 'Test #' + $i + ' sent at ' + (Get-Date -Format 'HH:mm:ss') }",
    "  if ($pc) { try {",
    "    $x = New-Object Windows.Data.Xml.Dom.XmlDocument",
    "    $s = if ($urgent) { ' scenario=\"urgent\"' } else { '' }",
    "    $a = if ($quiet) { '<audio silent=\"true\"/>' } else { '' }",
    "    $x.LoadXml('<toast' + $s + '><visual><binding template=\"ToastGeneric\"><text>' + [Security.SecurityElement]::Escape($t) + '</text><text>' + [Security.SecurityElement]::Escape($m) + '</text></binding></visual>' + $a + '</toast>')",
    "    $n.Show([Windows.UI.Notifications.ToastNotification]::new($x))",
    "  } catch { $failed += \"this PC's #$i ($($_.Exception.Message))\" } }",
    "  if ($phone) { try {",
    "    Invoke-RestMethod -Method Post -Uri ('https://ntfy.sh/' + $topic) -Body $m -Headers @{ Title = $t; Priority = $priority; Tags = 'test_tube' } | Out-Null",
    "  } catch { $failed += \"the phone's #$i ($($_.Exception.Message))\" } }",
    "  if ($i -lt $count) { Start-Sleep -Seconds $gap }",
    "}",
    "if ($failed) { [Console]::Error.WriteLine(($failed -join '; ')); exit 1 }",
  }
  -- (a file, not -Command: the script's quotes would have to survive Windows' command line)
  noctalia.mkdirAll(dataDir())
  local script = dataDir() .. "/send.ps1"
  noctalia.writeFile(script, table.concat(lines, "\r\n"))
  noctalia.runAsync({ "powershell", "-NoProfile", "-NonInteractive", "-ExecutionPolicy", "Bypass", "-File", script },
    function(result)
      if result and result.exitCode == 0 then return end
      local why = result and noctalia.string.trim(result.stderr or "") or ""
      told(why ~= "" and ("Didn't go: " .. why) or "PowerShell didn't run", true)
    end, 60000)
  local where = pc and phone and "this PC and the phone" or phone and "the phone" or "this PC"
  told(`Sending {c.count} to {where}`, false)
end

return M
