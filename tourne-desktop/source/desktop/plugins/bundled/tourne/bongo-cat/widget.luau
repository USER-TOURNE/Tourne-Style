--!nonstrict
-- Bongo Cat on the bar: a paw comes down for each key or click (tourne.keyEvents: the kind of press only),
-- left and right in turn, and both rest after a moment. Asleep, it hears nothing and costs nothing.

local IDLE = "( =^･ω･^)  ┳━┳"
local LEFT = "( =^･ω･^)ﾉ ┻━┳"
local RIGHT = "( =^･ω･^)ﾉ ┳━┻"
local SLEEP = "( =ᴗ ω ᴗ)ᶻ ┳━┳"

local awake = true
local left = true
local lastHit = 0
local hits = 0

local function cfg(key, fallback)
  local v = noctalia.getConfig(key)
  if v == nil or v == "" then return fallback end
  return v
end

local function draw(face)
  barWidget.setText(face)
  barWidget.setTooltip(awake and `Bongo Cat: {hits} taps so far\nClick to send it to sleep` or "Bongo Cat is asleep: click to wake it")
end

function onKeyPress(kind)
  if not awake or (kind == "click" and not cfg("clicks", true)) then return end
  hits += 1
  left = not left
  lastHit = os.clock()
  draw(left and LEFT or RIGHT)
  noctalia.setUpdateInterval(120)  -- (to lift the paw again)
end

function update()
  if os.clock() - lastHit > 0.1 then
    draw(awake and IDLE or SLEEP)
    noctalia.setUpdateInterval(0)  -- (nothing to do until the next press)
  end
end

function onClick()
  awake = not awake
  tourne.keyEvents(awake)
  draw(awake and IDLE or SLEEP)
end

tourne.keyEvents(true)
noctalia.setUpdateInterval(0)
draw(IDLE)
