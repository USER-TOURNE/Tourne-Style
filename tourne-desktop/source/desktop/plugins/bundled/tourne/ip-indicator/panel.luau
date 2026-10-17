--!nonstrict
-- IP Indicator's panel: the public side (from ipwho.is) and this PC's own addresses.

local render

local function row(name, value)
  return ui.row({ key = name, gap = 8 }, {
    ui.label({ text = name, width = 90, color = "on_surface_variant", fontSize = 12 }),
    ui.label({ text = value or "?", flexGrow = 1, maxLines = 2, color = "on_surface" }),
  })
end

render = function()
  local ip = noctalia.state.get("ip") or {}
  local hidden = noctalia.state.get("ip.hidden") == true
  local rows = {
    ui.row({ gap = 8, align = "center" }, {
      ui.glyph({ name = ip.vpn and "shield-lock" or "world-www", size = 20, color = "primary" }),
      ui.label({ text = ip.ok and ((ip.city or "?") .. ", " .. (ip.country or "?")) or "Not connected", flexGrow = 1, color = "on_surface", fontWeight = "semibold" }),
      ui.button({ glyph = hidden and "eye" or "eye-off", variant = "ghost", tooltip = hidden and "Show the IP" or "Hide the IP", onClick = function()
        noctalia.state.set("ip.hidden", not hidden)
        render()
      end }),
      ui.button({ glyph = "refresh", variant = "ghost", tooltip = "Check again", onClick = function()
        noctalia.state.set("ip.refresh", true)
      end }),
    }),
    row("Public IP", hidden and "hidden" or ip.ip),
    row("Provider", ip.isp),
    row("Network", ip.asn and ("AS" .. tostring(ip.asn) .. " " .. (ip.org or "")) or ip.org),
    row("VPN", ip.vpn or "None found"),
    row("Round trip", ip.ms and (ip.ms .. " ms") or nil),
    ui.separator({ spacing = 6 }),
  }
  for i, a in tourne.networkInterfaces() do
    if a.up and #a.ipv4 > 0 then
      table.insert(rows, row(a.name, table.concat(a.ipv4, ", ") .. (a.gateway and ("  (router " .. a.gateway .. ")") or "")))
    end
  end
  if not ip.ok then
    -- (the bar widget asks; this only shows what it got)
  end
  panel.render(ui.scroll({ key = "all", flexGrow = 1, gap = 6, padding = 12 }, rows))
end

function onOpen(_context)
  render()
end

noctalia.state.watch("ip", function()
  render()
end)
