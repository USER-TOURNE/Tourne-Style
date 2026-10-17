--!nonstrict
-- Test Notifications: the choices (how many, how urgent, where, the words, the ntfy topic) and a Send button.

local send = require("./send")

local c = send.load()
local note = ""
local render

local COUNTS = { 1, 2, 3, 5, 10, 20 }

local function indexOf(list, value)
  for i, v in list do
    if v == value then return i - 1 end
  end
  return 0
end

local function names(list)
  local out = {}
  for _, v in list do table.insert(out, tostring(v)) end
  return out
end

local function row(label, control)
  return ui.row({ gap = 8, crossAlign = "center" }, {
    ui.label({ text = label, color = "on_surface_variant", fontSize = 12, width = 96 }),
    control,
  })
end

local function changed(key, value)
  c[key] = value
  send.save(c)
end

render = function()
  panel.render(ui.column({ flexGrow = 1, gap = 10, padding = 12 }, {
    ui.label({ text = "Test notifications", color = "on_surface", fontWeight = "semibold" }),
    row("How many", ui.select({ options = names(COUNTS), selectedIndex = indexOf(COUNTS, c.count), onChange = function(i)
      changed("count", COUNTS[(tonumber(i) or 0) + 1] or 1)
    end })),
    row("Priority", ui.select({ options = send.PRIORITIES, selectedIndex = indexOf(send.PRIORITIES, c.priority), onChange = function(i)
      changed("priority", send.PRIORITIES[(tonumber(i) or 0) + 1] or "default")
    end })),
    row("Send to", ui.select({ options = send.TARGETS, selectedIndex = indexOf(send.TARGETS, c.target), onChange = function(i)
      changed("target", send.TARGETS[(tonumber(i) or 0) + 1] or "This PC")
      render()
    end })),
    row("Title", ui.input({ key = "title", value = c.title, placeholder = "Test notification", flexGrow = 1,
      onChange = function(v) changed("title", v) end })),
    row("Message", ui.input({ key = "message", value = c.message, placeholder = "Empty: the number and time", flexGrow = 1,
      onChange = function(v) changed("message", v) end })),
    row("ntfy topic", ui.input({ key = "topic", value = c.topic, placeholder = "Your topic, for the phone", flexGrow = 1,
      visible = c.target ~= "This PC", onChange = function(v) changed("topic", noctalia.string.trim(v)) end })),
    ui.button({ key = "send", text = "Send", glyph = "send", variant = "primary", onClick = function()
      send.send(c, function(text, failed)
        note = text
        if failed then noctalia.notifyError("Test Notifications", text) end
        render()
      end)
    end }),
    ui.label({ text = note, visible = note ~= "", fontSize = 12, color = "on_surface_variant" }),
  }))
end

function onOpen(_context)
  c = send.load()
  note = ""
  render()
end
