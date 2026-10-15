--!nonstrict
-- Ported to Tourne Desktop from raycursive's Discord Voice (MIT).
-- Windows helpers for the bar, panel and service: which Python runs the
-- bridge, and how Discord is started (no shell, an argument list).

local M = {}

M.DEFAULT_DISCORD = "%LOCALAPPDATA%\\Discord\\Update.exe"

-- The Python setting: a name on the PATH ("python"), or a full path.
function M.pythonProgram()
  local configured = noctalia.getConfig("python")
  if type(configured) ~= "string" or noctalia.string.trim(configured) == "" then
    return "python"
  end
  configured = noctalia.string.trim(configured)
  if configured:find("[\\/%%~]") then
    return noctalia.expandPath(configured)
  end
  return configured
end

function M.discordBinary()
  local configured = noctalia.getConfig("discord_binary")
  if type(configured) ~= "string" or configured == "" then
    configured = M.DEFAULT_DISCORD
  end
  return noctalia.expandPath(configured)
end

-- The Discord setting as a program to run. Discord's Update.exe starts the
-- newest copy of the app named after its folder (Discord, DiscordPTB,
-- DiscordCanary) and hands it --process-start-args; anything else (Discord.exe
-- itself) is run as it is. A discord:// link goes to Discord as
-- "--url -- <link>", as on Linux.
function M.discordArgv(uri)
  local binary = M.discordBinary()
  local file = (binary:match("([^\\/]+)$") or binary):lower()
  if file == "update.exe" or file == "update" then
    local app = binary:match("([^\\/]+)[\\/][^\\/]+$") or "Discord"
    local argv = { binary, "--processStart", app .. ".exe" }
    if uri then
      table.insert(argv, "--process-start-args")
      table.insert(argv, "--url -- " .. uri)
    end
    return argv
  end
  local argv = { binary }
  if uri then
    table.insert(argv, "--url")
    table.insert(argv, "--")
    table.insert(argv, uri)
  end
  return argv
end

-- Starts Discord (at the channel's link, when given) as the taskbar starts an
-- app, so it stays open when Tourne Desktop restarts.
function M.launchDiscord(uri)
  local argv = M.discordArgv(uri)
  if tourne and tourne.launch then
    return tourne.launch(argv)
  end
  return noctalia.runAsync(argv)
end

return M
