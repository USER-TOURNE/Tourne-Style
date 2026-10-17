--!nonstrict
-- IP Indicator: from hthienloc's IP Indicator (DankMaterialShell, MIT), rewritten for Tourne Desktop.
-- Asks ipwho.is (no account, HTTPS) for the public address; a VPN shows as an adapter Windows has
-- (WireGuard, OpenVPN, Tailscale...) or as the provider ipwho.is names. Middle click: hide or show the IP.
-- The answer is shared with the panel through noctalia.state ("ip").

local hidden = noctalia.getConfig("hidden") == true

local function vpnAdapter()
  for _, a in tourne.networkInterfaces() do
    if a.up and a.vpn then return a.name end
  end
  return nil
end

local function show()
  local ip = noctalia.state.get("ip")
  barWidget.setGlyph(ip and ip.vpn and "shield-lock" or "world-www")
  if not ip then
    barWidget.setText("…")
    barWidget.setTooltip("Public IP: asking")
    return
  end
  if not ip.ok then
    barWidget.setText("offline")
    barWidget.setTooltip("Couldn't reach ipwho.is")
    return
  end
  barWidget.setText((ip.code or "") .. (hidden and "" or "  " .. ip.ip))
  barWidget.setTooltip(string.format("%s\n%s, %s\n%s%s\n%d ms", hidden and "IP hidden (middle click)" or ip.ip, ip.city or "?", ip.country or "?",
    ip.isp or "", ip.vpn and ("\nVPN: " .. ip.vpn) or "", ip.ms or 0))
end

local function refresh()
  local started = noctalia.nowMs()
  noctalia.http({ url = "https://ipwho.is/" }, function(r)
    local ms = math.floor(noctalia.nowMs() - started)
    local d = r.ok and r.status == 200 and noctalia.json.decode(r.body) or nil
    if type(d) ~= "table" or d.success == false then
      noctalia.state.set("ip", { ok = false, at = os.time() })
    else
      local conn = d.connection or {}
      noctalia.state.set("ip", {
        ok = true, ip = d.ip, code = d.country_code, country = d.country, city = d.city, region = d.region,
        isp = conn.isp or conn.org, org = conn.org, asn = conn.asn, ms = ms, at = os.time(),
        vpn = vpnAdapter(),
      })
    end
    show()
  end)
end

function update()
  refresh()
end

function onClick()
  noctalia.togglePanel("tourne/ip-indicator:panel")
end

function onMiddleClick()
  hidden = not hidden
  noctalia.state.set("ip.hidden", hidden)
  show()
end

noctalia.state.watch("ip.refresh", function(v)
  if v then
    noctalia.state.set("ip.refresh", nil)
    refresh()
  end
end)
noctalia.state.set("ip.hidden", hidden)
noctalia.setUpdateInterval((noctalia.getConfig("refreshMinutes") or 10) * 60000)
refresh()
show()
