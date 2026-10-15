--!nonstrict
-- Screen Recorder on the bar: the recorder lives here (so it keeps going with the panel closed).
-- Click: the panel. Middle click: start, or stop. While recording, the time so far shows in red.
--
-- From YoungJurry's Screen Recorder YS (DankMaterialShell, MIT), which drives gpu-screen-recorder on
-- Wayland. Here ffmpeg records with the same choices: ddagrab (Desktop Duplication, on the graphics card,
-- which keeps up at 120 fps and more), or gdigrab (GDI, the old way) when asked or when the screen isn't on
-- the first graphics card.
-- The PC's sound comes from Tourne Desktop (tourne.systemSound: Windows' loopback, through a pipe), mixed
-- with a microphone when both are set.
-- The panel asks for a start or a stop through noctalia.state ("recorder.command").

local stream = nil
local startedAt = 0
local file = ""
local countdownEnd = 0
local lastLines = {}

local function cfg(key, fallback)
  local v = noctalia.getConfig(key)
  if v == nil or v == "" then return fallback end
  return v
end

local function publish(state)
  noctalia.state.set("recorder.state", state)
end

local function screen()
  local want = string.upper(cfg("monitor", ""))
  local first
  for _, o in noctalia.outputs() do
    if o.primary then first = first or o end
    if want ~= "" and string.upper(o.name) == want then return o end
  end
  return first or noctalia.outputs()[1]
end

local CRF = { medium = 28, high = 23, veryhigh = 20, ultra = 16 }

local function argv(sound)
  local o = screen()
  local fps = cfg("fps", "60")
  local enc = cfg("encoder", "libx264")
  local q = CRF[cfg("quality", "high")] or 23
  local folder = noctalia.expandPath(cfg("folder", "~/Videos"))
  noctalia.mkdirAll(folder)
  file = folder .. "\\Recording " .. os.date("%Y-%m-%d %H-%M-%S") .. "." .. cfg("container", "mkv")
  local a = { cfg("ffmpeg", "ffmpeg"), "-hide_banner", "-loglevel", "error", "-stats", "-y" }
  local function add(...)
    for _, v in { ... } do table.insert(a, v) end
  end
  local index = cfg("capture", "dda") == "dda" and o and tourne.screenIndex(o.name)
  if index then
    add("-f", "lavfi", "-i", string.format("ddagrab=output_idx=%d:framerate=%s:draw_mouse=%d", index, fps, cfg("cursor", true) and 1 or 0))
  else
    add("-f", "gdigrab", "-framerate", fps, "-draw_mouse", cfg("cursor", true) and "1" or "0")
    if o then
      add("-offset_x", tostring(o.x), "-offset_y", tostring(o.y), "-video_size", string.format("%dx%d", o.width, o.height))
    end
    add("-i", "desktop")
  end
  local inputs = 0
  if sound then
    add("-thread_queue_size", "1024", "-f", sound.format, "-ar", tostring(sound.rate), "-ac", tostring(sound.channels), "-i", sound.input)
    inputs += 1
  end
  local mic = cfg("audioDevice", "")
  if mic ~= "" then
    add("-thread_queue_size", "1024", "-f", "dshow", "-i", "audio=" .. mic)
    inputs += 1
  end
  if inputs == 2 then
    add("-filter_complex", "[1:a][2:a]amix=inputs=2:duration=longest:normalize=0[a]", "-map", "0:v", "-map", "[a]")
  elseif inputs == 1 then
    add("-map", "0:v", "-map", "1:a")
  end
  -- ddagrab's frames stay on the graphics card: NVIDIA's encoders take them as they are; everything else
  -- (and scaling) gets them copied back first.
  local res = cfg("resolution", "native")
  local filters = {}
  if index and (res ~= "native" or not string.find(enc, "nvenc")) then
    table.insert(filters, "hwdownload")
    table.insert(filters, "format=bgra")
  end
  if res ~= "native" then table.insert(filters, "scale=-2:" .. res) end
  if #filters > 0 then add("-vf", table.concat(filters, ",")) end
  add("-c:v", enc)
  if enc == "libx264" then
    add("-preset", "veryfast", "-crf", tostring(q), "-pix_fmt", "yuv420p")
  elseif string.find(enc, "nvenc") then
    add("-preset", "p4", "-rc", "vbr", "-cq", tostring(q), "-b:v", "0")
  elseif string.find(enc, "amf") then
    add("-quality", "balanced", "-rc", "cqp", "-qp_i", tostring(q), "-qp_p", tostring(q))
  elseif string.find(enc, "qsv") then
    add("-global_quality", tostring(q))
  end
  if inputs > 0 then add("-c:a", "aac", "-b:a", "160k") end
  table.insert(a, file)
  return a
end

local function show()
  barWidget.setGlyph(stream and "player-record" or "video")
  if stream then
    local s = math.floor((noctalia.nowMs() - startedAt) / 1000)
    barWidget.setText(string.format("%d:%02d", s // 60, s % 60))
    barWidget.setColor("error")
    barWidget.setTooltip("Recording " .. file .. "\nMiddle click to stop")
  elseif countdownEnd > 0 then
    barWidget.setText(tostring(math.max(1, math.ceil((countdownEnd - noctalia.nowMs()) / 1000))))
    barWidget.setColor("error")
  else
    barWidget.setText("")
    barWidget.setColor("on_surface")
    barWidget.setTooltip("Screen recorder\nMiddle click to record")
  end
end

local function begin()
  countdownEnd = 0
  lastLines = {}
  -- The PC's sound waits in a pipe for ffmpeg, so it's asked for just before ffmpeg starts.
  local sound = nil
  local output = cfg("systemSound", "none")
  if output ~= "none" then
    local got, problem = tourne.systemSound(output)
    if got then
      sound = got
    else
      noctalia.notifyError("Recording without the PC's sound", tostring(problem))
    end
  end
  local id, why = noctalia.runStream(argv(sound), function(line)
    table.insert(lastLines, line)
    if #lastLines > 6 then table.remove(lastLines, 1) end
  end, function(code)
    local was = stream
    stream = nil
    publish({ recording = false, file = file, error = code ~= 0 and table.concat(lastLines, "\n") or nil })
    if code == 0 or was then
      noctalia.notify("Recording saved", file)
    else
      noctalia.notifyError("Recording didn't start", table.concat(lastLines, "\n"))
    end
    noctalia.setUpdateInterval(10000)
    show()
  end)
  if not id then
    noctalia.notifyError("Recording didn't start", tostring(why))
    publish({ recording = false, error = tostring(why) })
    return
  end
  stream = id
  startedAt = noctalia.nowMs()
  publish({ recording = true, file = file, startedAt = startedAt })
  noctalia.setUpdateInterval(1000)
  show()
end

local function start()
  if stream or countdownEnd > 0 then return end
  local wait = tonumber(cfg("countdown", "0")) or 0
  if wait > 0 then
    countdownEnd = noctalia.nowMs() + wait * 1000
    noctalia.setUpdateInterval(250)
    publish({ recording = false, countdown = wait })
    show()
  else
    begin()
  end
end

local function stop()
  if countdownEnd > 0 then
    countdownEnd = 0
    publish({ recording = false })
    show()
    return
  end
  if not stream then return end
  tourne.write(stream, "q") -- ffmpeg finishes the file and quits
  tourne.stop(stream, 8000)
end

function update()
  if countdownEnd > 0 and noctalia.nowMs() >= countdownEnd then begin() end
  show()
end

function onClick()
  noctalia.togglePanel("tourne/screen-recorder:panel")
end

function onMiddleClick()
  if stream then stop() else start() end
end

noctalia.state.watch("recorder.command", function(command)
  if command == nil then return end
  noctalia.state.set("recorder.command", nil)
  if command == "start" then start() elseif command == "stop" then stop() end
end)

publish({ recording = false })
noctalia.setUpdateInterval(10000)
show()
