--!nonstrict
-- Media Downloader's panel: paste a link, pick video or sound (and its type and size), then the queue
-- with progress, stop, retry, open and show in folder. The bar widget does the downloading.

local VIDEO_FORMATS = { "mp4", "mkv", "webm" }
local VIDEO_SIZES = { "best", "2160p", "1440p", "1080p", "720p", "480p" }
local AUDIO_FORMATS = { "mp3", "m4a", "opus", "flac", "wav" }
local AUDIO_QUALITIES = { "best", "320k", "192k", "128k" }

local url = ""
local kind = "video"
local format, quality
local version = ""
local updating = false
local render

local function indexOf(list, value)
  for i, v in list do
    if v == value then return i - 1 end
  end
  return 0
end

local function defaults()
  if kind == "audio" then
    format = noctalia.getConfig("audioFormat") or "mp3"
    quality = noctalia.getConfig("audioQuality") or "best"
  else
    format = noctalia.getConfig("videoFormat") or "mp4"
    quality = noctalia.getConfig("videoRes") or "1080p"
  end
end

local function send(c)
  noctalia.state.set("downloads.command", c)
end

local function download()
  send({ op = "add", url = url, kind = kind, format = format, quality = quality })
  url = ""
  render()
end

local function folderOf(path)
  return string.match(path or "", "^(.*)[\\/][^\\/]*$")
end

local STATUS = { queued = "Waiting", downloading = "Downloading", finishing = "Finishing", done = "Done",
  failed = "Failed", stopped = "Stopped" }

local function row(it)
  local busy = it.status == "downloading" or it.status == "finishing" or it.status == "queued"
  local line = STATUS[it.status] or it.status
  if it.status == "downloading" then
    line = string.format("%d%%", math.floor(it.percent or 0))
    if it.speed and it.speed ~= "" then line ..= "  " .. it.speed end
    if it.eta and it.eta ~= "" then line ..= "  " .. it.eta .. " left" end
  elseif it.status == "failed" and it.error then
    line = "Failed: " .. it.error
  end
  local buttons = {}
  if busy then
    table.insert(buttons, ui.button({ glyph = "player-stop", tooltip = "Stop", controlSize = "sm", variant = "ghost",
      onClick = function() send({ op = "stop", id = it.id }) end }))
  else
    if it.status == "done" and it.file then
      table.insert(buttons, ui.button({ glyph = "player-play", tooltip = "Open", controlSize = "sm", variant = "ghost",
        onClick = function() noctalia.openPath(it.file) end }))
    end
    table.insert(buttons, ui.button({ glyph = "folder", tooltip = "Show the folder", controlSize = "sm", variant = "ghost",
      onClick = function() noctalia.openPath(folderOf(it.file) or it.folder) end }))
    if it.status ~= "done" then
      table.insert(buttons, ui.button({ glyph = "refresh", tooltip = "Try again", controlSize = "sm", variant = "ghost",
        onClick = function() send({ op = "retry", id = it.id }) end }))
    end
    table.insert(buttons, ui.button({ glyph = "x", tooltip = "Take off the list", controlSize = "sm", variant = "ghost",
      onClick = function() send({ op = "remove", id = it.id }) end }))
  end
  return ui.column({ key = "d" .. tostring(it.id), gap = 4, padding = 8, fill = "surface_container", radius = 8 }, {
    ui.row({ gap = 8, align = "center" }, {
      ui.glyph({ name = it.kind == "audio" and "music" or "movie", size = 18,
        color = it.status == "failed" and "error" or "primary" }),
      ui.column({ flexGrow = 1, gap = 2 }, {
        ui.label({ text = it.title or it.url, maxLines = 1 }),
        ui.label({ text = line, fontSize = 11, maxLines = 2, color = it.status == "failed" and "error" or "on_surface_variant" }),
      }),
      ui.row({ gap = 2 }, buttons),
    }),
    ui.progress({ progress = (it.percent or 0) / 100, visible = busy }),
  })
end

render = function()
  local items = noctalia.state.get("downloads") or {}
  local rows = {}
  for i = #items, 1, -1 do
    table.insert(rows, row(items[i]))
  end
  if #rows == 0 then
    table.insert(rows, ui.label({ text = "Nothing yet. Paste a link above.", color = "on_surface_variant", textAlign = "center" }))
  end
  local formats = kind == "audio" and AUDIO_FORMATS or VIDEO_FORMATS
  local sizes = kind == "audio" and AUDIO_QUALITIES or VIDEO_SIZES
  panel.render(ui.column({ flexGrow = 1, gap = 10, padding = 12 }, {
    ui.row({ gap = 6, align = "center" }, {
      ui.input({ key = "url", value = url, placeholder = "A video or song's web link", controlSize = "sm", flexGrow = 1,
        onChange = function(v) url = v end,
        onSubmit = function(v)
          url = v
          download()
        end }),
      ui.button({ glyph = "clipboard", tooltip = "Paste", controlSize = "sm", variant = "outline", onClick = function()
        url = noctalia.clipboardText() or ""
        render()
      end }),
    }),
    ui.row({ gap = 6, align = "center" }, {
      ui.button({ text = "Video", glyph = "movie", controlSize = "sm", variant = kind == "video" and "primary" or "outline",
        onClick = function()
          kind = "video"
          defaults()
          render()
        end }),
      ui.button({ text = "Sound", glyph = "music", controlSize = "sm", variant = kind == "audio" and "primary" or "outline",
        onClick = function()
          kind = "audio"
          defaults()
          render()
        end }),
      ui.select({ options = formats, selectedIndex = indexOf(formats, format), controlSize = "sm",
        onChange = function(i) format = formats[(tonumber(i) or 0) + 1] end }),
      ui.select({ options = sizes, selectedIndex = indexOf(sizes, quality), controlSize = "sm",
        onChange = function(i) quality = sizes[(tonumber(i) or 0) + 1] end }),
      ui.spacer({ flexGrow = 1 }),
      ui.button({ text = "Download", glyph = "download", controlSize = "sm", variant = "primary", onClick = download }),
    }),
    ui.scroll({ flexGrow = 1 }, { ui.column({ gap = 6 }, rows) }),
    ui.row({ gap = 6, align = "center" }, {
      ui.label({ text = updating and "Updating yt-dlp..." or (version ~= "" and ("yt-dlp " .. version) or "yt-dlp not found"),
        fontSize = 11, color = "on_surface_variant", flexGrow = 1 }),
      ui.button({ text = "Update yt-dlp", controlSize = "sm", variant = "ghost", enabled = version ~= "" and not updating,
        onClick = function()
          updating = true
          render()
          noctalia.runAsync({ noctalia.getConfig("ytdlp") or "yt-dlp", "-U" }, function(r)
            updating = false
            if r.exitCode == 0 then
              noctalia.notify("yt-dlp", noctalia.string.trim(r.stdout))
            else
              noctalia.notifyError("yt-dlp didn't update", "If winget installed it: winget upgrade yt-dlp.yt-dlp\n" .. noctalia.string.trim(r.stderr))
            end
            render()
          end, 60000)
        end }),
      ui.button({ text = "Clear finished", controlSize = "sm", variant = "ghost", onClick = function() send({ op = "clear" }) end }),
    }),
  }))
end

function onOpen(_context)
  defaults()
  render()
  noctalia.runAsync({ noctalia.getConfig("ytdlp") or "yt-dlp", "--version" }, function(r)
    version = r.exitCode == 0 and noctalia.string.trim(r.stdout) or ""
    render()
  end, 15000)
end

noctalia.state.watch("downloads", function()
  render()
end)
