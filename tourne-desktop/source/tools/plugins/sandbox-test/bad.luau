--!nonstrict
local function try(what, ...)
  local ok, err = noctalia.writeFile(...)
  noctalia.log(what .. ": " .. tostring(ok) .. " " .. tostring(err))
end
try("write outside", "C:/Users/Public/tourne-sandbox-test.txt", "x")
try("write own folder", "escape.txt", "x")
try("write via a folder setting of ~", noctalia.expandPath(noctalia.getConfig("dir")) .. "/tourne-sandbox-test.txt", "x")
try("write its data folder", noctalia.pluginDataDir() .. "/ok.txt", "x")
local body, err = noctalia.readFile("../../example/plugin.toml")
noctalia.log("read neighbour: " .. tostring(body ~= nil) .. " " .. tostring(err))
noctalia.log("http: " .. tostring(noctalia.http({ url = "https://example.com" }, function(r) noctalia.log("http answered ok=" .. tostring(r.ok)) end)))
noctalia.log("io: " .. tostring(io) .. " os.execute: " .. tostring(os.execute) .. " loadstring: " .. tostring(loadstring))
local ok = pcall(function() noctalia.log = nil end)
noctalia.log("could replace noctalia.log: " .. tostring(ok))
barWidget.setText("still here")
local n = 0
function update()
  n += 1
  if n == 1 then while true do end end
  barWidget.setText("still here " .. n)
end
function onClick()
  local t = {}
  for i = 1, 1e9 do t[i] = string.rep("x", 1000) .. i end
end
