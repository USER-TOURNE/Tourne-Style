--!nonstrict
-- Media Downloader on the bar: the queue lives here (so downloads keep going with the panel closed).
-- Click: the panel. Middle click: download the link on the clipboard as video.
--
-- From hthienloc's Media Downloader (DankMaterialShell, MIT), which runs yt-dlp; the same choices here.
-- The panel asks through noctalia.state ("downloads.command"); the list is "downloads".

local items = {}   -- { id, url, kind, title, status, percent, speed, eta, file, folder, error }
local running = {} -- id -> stream
local nextId = 1
local loaded = false

local function cfg(key, fallback)
  local v = noctalia.getConfig(key)
  if v == nil or v == "" then return fallback end
  return v
end

local function historyFile()
  return (noctalia.pluginDataDir() or "") .. "\\history.json"
end

local function publish(progressOnly)
  noctalia.state.set("downloads", items)
  if progressOnly then return end
  -- Finished ones are kept (the last 50) for the next start.
  local keep = {}
  for _, it in items do
    if it.status == "done" or it.status == "failed" or it.status == "stopped" then table.insert(keep, it) end
  end
  while #keep > 50 do table.remove(keep, 1) end
  noctalia.mkdirAll(noctalia.pluginDataDir() or "")
  noctalia.writeFile(historyFile(), noctalia.json.encode(keep))
end

local function load()
  if loaded then return end
  loaded = true
  local text = noctalia.readFile(historyFile())
  local old = text and noctalia.json.decode(text) or nil
  if type(old) == "table" then
    for _, it in old do
      if type(it) == "table" and it.id then
        table.insert(items, it)
        nextId = math.max(nextId, (tonumber(it.id) or 0) + 1)
      end
    end
  end
end

local function find(id)
  id = tonumber(id)
  for i, it in items do
    if it.id == id then return it, i end
  end
  return nil
end

local function argv(it)
  local a = { cfg("ytdlp", "yt-dlp"), "--ignore-config", "--encoding", "utf-8", "--newline", "--progress",
    "--progress-template", "download:TTPROG %(progress._percent_str)s|%(progress._speed_str)s|%(progress._eta_str)s",
    "--no-simulate", "--print", "video:TTTITLE %(title)s", "--print", "after_move:TTFILE %(filepath)s",
    "-P", it.folder, "-o", "%(title)s.%(ext)s" }
  local function add(...)
    for _, v in { ... } do table.insert(a, v) end
  end
  if not cfg("playlists", false) then add("--no-playlist") end
  if it.kind == "audio" then
    add("-x", "--audio-format", it.format)
    if it.quality ~= "best" then add("--audio-quality", it.quality) end
  else
    if it.quality == "best" then
      add("-f", "bestvideo+bestaudio/best")
    else
      local h = string.gsub(it.quality, "p", "")
      add("-f", "bestvideo[height<=" .. h .. "]+bestaudio/best[height<=" .. h .. "]")
    end
    add("--merge-output-format", it.format)
    if cfg("embedSubs", false) then add("--embed-subs", "--write-subs") end
  end
  if cfg("embedThumbnail", true) then
    add("--embed-thumbnail")
    if cfg("cropThumbnail", false) then
      add("--convert-thumbnails", "jpg", "--ppa",
        "EmbedThumbnail+ffmpeg_o:-c:v mjpeg -vf crop=\"'if(gt(ih,iw),iw,ih)':'if(gt(iw,ih),ih,iw)'\"")
    end
  end
  if cfg("embedMetadata", true) then add("--embed-metadata") end
  if cfg("sponsorBlock", true) then add("--sponsorblock-remove", "default") end
  local rate = tonumber(cfg("maxRate", 0)) or 0
  if rate > 0 then add("--limit-rate", tostring(math.floor(rate)) .. "K") end
  table.insert(a, it.url)
  return a
end

local show
local pump

local function begin(it)
  it.status, it.percent, it.speed, it.eta, it.error = "downloading", 0, "", "", nil
  noctalia.mkdirAll(it.folder)
  local lastError = ""
  local id, why = noctalia.runStream(argv(it), function(line)
    local pct, speed, eta = string.match(line, "^TTPROG%s*([%d%.]+)%%|([^|]*)|(.*)$")
    if pct then
      it.percent = tonumber(pct) or it.percent
      it.speed, it.eta = noctalia.string.trim(speed), noctalia.string.trim(eta)
    elseif string.sub(line, 1, 8) == "TTTITLE " then
      it.title = string.sub(line, 9)
    elseif string.sub(line, 1, 7) == "TTFILE " then
      it.file = string.sub(line, 8)
    elseif string.match(line, "^%[Merger%]") or string.match(line, "^%[ExtractAudio%]")
      or string.match(line, "^%[EmbedThumbnail%]") or string.match(line, "^%[SponsorBlock%]") then
      it.status = "finishing"
    elseif string.sub(line, 1, 6) == "ERROR:" then
      lastError = noctalia.string.trim(string.sub(line, 7))
    else
      return
    end
    publish(pct ~= nil)
  end, function(code)
    running[it.id] = nil
    if it.status ~= "stopped" then
      if code == 0 then
        it.status, it.percent = "done", 100
        noctalia.notify("Downloaded", it.title or it.url)
      else
        it.status, it.error = "failed", lastError ~= "" and lastError or ("yt-dlp ended with " .. tostring(code))
        noctalia.notifyError("Download failed", (it.title or it.url) .. "\n" .. it.error)
      end
    end
    publish()
    pump()
    show()
  end)
  if not id then
    it.status, it.error = "failed", tostring(why)
    return
  end
  running[it.id] = id
end

-- Starts queued ones up to the "At once" count.
pump = function()
  local busy = 0
  for _ in running do busy += 1 end
  local most = tonumber(cfg("parallel", "2")) or 2
  for _, it in items do
    if busy >= most then break end
    if it.status == "queued" then
      begin(it)
      busy += 1
    end
  end
  publish()
  show()
end

local function add(url, kind, format, quality)
  url = noctalia.string.trim(url or "")
  if not (string.match(url, "^https?://") or string.match(url, "^www%.")) then
    noctalia.notifyError("Not a web link", url ~= "" and url or "The clipboard has no link")
    return
  end
  kind = kind == "audio" and "audio" or "video"
  local it = {
    id = nextId, url = url, kind = kind, status = "queued", percent = 0, speed = "", eta = "",
    format = format or (kind == "audio" and cfg("audioFormat", "mp3") or cfg("videoFormat", "mp4")),
    quality = quality or (kind == "audio" and cfg("audioQuality", "best") or cfg("videoRes", "1080p")),
    folder = noctalia.expandPath(kind == "audio" and cfg("audioFolder", "~/Music") or cfg("videoFolder", "~/Videos")),
  }
  nextId += 1
  table.insert(items, it)
  pump()
end

show = function()
  local active, sum = 0, 0
  for _, it in items do
    if it.status == "downloading" or it.status == "finishing" or it.status == "queued" then
      active += 1
      sum += it.percent or 0
    end
  end
  barWidget.setGlyph("download")
  if active > 0 then
    barWidget.setText(string.format("%d%%", math.floor(sum / active)))
    barWidget.setColor("primary")
    barWidget.setTooltip(string.format("%d downloading\nClick to see them", active))
  else
    barWidget.setText("")
    barWidget.setColor("on_surface")
    barWidget.setTooltip("Media downloader\nMiddle click: the copied link as video")
  end
end

function update()
  show()
end

function onClick()
  noctalia.togglePanel("tourne/media-downloader:panel")
end

function onMiddleClick()
  add(noctalia.clipboardText() or "", "video")
end

noctalia.state.watch("downloads.command", function(c)
  if type(c) ~= "table" then return end
  noctalia.state.set("downloads.command", nil)
  if c.op == "add" then
    add(c.url, c.kind, c.format, c.quality)
  elseif c.op == "stop" then
    local it = find(c.id)
    if it and running[it.id] then
      it.status = "stopped"
      tourne.stop(running[it.id], 1500)
    elseif it and it.status == "queued" then
      it.status = "stopped"
    end
    publish()
  elseif c.op == "retry" then
    local it = find(c.id)
    if it then
      it.status = "queued"
      pump()
    end
  elseif c.op == "remove" then
    local it, at = find(c.id)
    if it and not running[it.id] then table.remove(items, at) end
    publish()
  elseif c.op == "clear" then
    local keep = {}
    for _, it in items do
      if running[it.id] or it.status == "queued" then table.insert(keep, it) end
    end
    items = keep
    publish()
  end
  show()
end)

load()
publish()
noctalia.setUpdateInterval(2000)
show()
