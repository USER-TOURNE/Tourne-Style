--!nonstrict
-- Test Notifications: the bar button. A click sends as last chosen; a right click opens the choices.

local send = require("./send")

local function show(text)
  barWidget.setGlyph("bell-ringing")
  barWidget.setTooltip(text or "Send a test notification (right-click: how many, how urgent, to the phone too)")
end

function onClick()
  send.send(send.load(), function(text, failed)
    if failed then noctalia.notifyError("Test Notifications", text) end
    show(text)
  end)
end

function onRightClick()
  noctalia.togglePanel("tourne/test-notifications:panel")
end

show()
