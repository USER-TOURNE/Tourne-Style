--!nonstrict
-- Ported to Tourne Desktop from Nicolas Bellamy's Claude Code Usage (jrohland/claudecode, MIT).
-- Claude Cockpit carries the same engine: nightwatch75 copied get-claude-usage from jrohland/claudecode.
-- usage: the data engine. On Linux this is get-claude-usage, a Bash script that
-- runs jq, curl, awk and find; here the same steps are Luau over Tourne
-- Desktop's calls, so nothing needs installing:
--   find -name '*.jsonl' -mtime -31       tourne.findFiles
--   jq over every transcript line          tourne.jsonLines (a line at a time, on a thread)
--   curl                                   noctalia.http
--   awk sums, date math                    Luau
-- The JSON it hands back is the one `get-claude-usage --json` prints, field for
-- field, so the bar, panel and desktop entries read it unchanged.
--
-- Files: Claude Code's own under %USERPROFILE%\.claude (projects\*\*.jsonl,
-- .credentials.json, stats-cache.json) are only read. The caches the script
-- kept in ~/.claude (pricing-cache.json, usage-cache.json) live in the plugin's
-- data folder instead: a plugin writes only there.
--
-- The login token: read from .credentials.json for the one request to
-- api.anthropic.com/api/oauth/usage, as the original does, and nowhere else.
-- It is never logged, written, copied or sent to any other host.

local M = {}

local LITELLM_URL = "https://raw.githubusercontent.com/BerriAI/litellm/main/model_prices_and_context_window.json"
local USAGE_URL = "https://api.anthropic.com/api/oauth/usage"
local USAGE_CACHE_TTL = 120 -- seconds
local MONTH_SECS = 31 * 86400

local function readJson(path)
  local text = noctalia.readFile(path)
  if type(text) ~= "string" or text == "" then return nil end
  local v = noctalia.json.decode(text)
  return type(v) == "table" and v or nil
end

local function writeJson(path, value)
  noctalia.writeFile(path, noctalia.json.encode(value))
end

local function num(v)
  local n = tonumber(v)
  if n == nil or n ~= n then return 0 end
  return n
end

local function round2(v)
  return tonumber(string.format("%.2f", num(v))) or 0
end

-- ── dates (local, as `date` gives them) ────────────────────────────────────

local function dates()
  local now = os.time()
  local t = os.date("*t", now)
  -- Noon today, so stepping whole days never crosses midnight on a clock change.
  local noon = now - (t.hour - 12) * 3600 - t.min * 60 - t.sec
  local dow = ((t.wday + 5) % 7) + 1 -- 1 = Monday .. 7 = Sunday
  local weekStart = noon - (dow - 1) * 86400
  local days = {}
  for i = 0, 6 do
    days[i + 1] = os.date("%Y-%m-%d", weekStart + i * 86400)
  end
  return {
    today = os.date("%Y-%m-%d", now),
    weekStart = days[1],
    monthStart = os.date("%Y-%m-01", now),
    days = days,
  }
end

-- ── pricing (LiteLLM's table, once a day) ──────────────────────────────────

-- "claude-{family}-{major}-{minor}" with an input price, the latest per family.
local function pricingFrom(raw, today)
  local best = {}
  for key, value in pairs(raw) do
    if type(key) == "string" and type(value) == "table" and key:match("^claude%-%l+%-%d+%-%d+$")
      and num(value.input_cost_per_token) > 0 then
      local family = key:match("^claude%-(%l+)%-")
      if best[family] == nil or key > best[family].key then
        best[family] = { key = key, value = value }
      end
    end
  end
  local models, count = {}, 0
  for family, b in pairs(best) do
    models[family] = {
      input = b.value.input_cost_per_token,
      output = b.value.output_cost_per_token,
      cache_read = b.value.cache_read_input_token_cost,
      cache_write = b.value.cache_creation_input_token_cost,
    }
    count += 1
  end
  if count == 0 then return nil end
  return { updated = today, models = models }
end

local function fetchRates(opts, done)
  local url
  if opts.currencyTargets then
    url = `{opts.currencyApiUrl}?from=USD&to={opts.currencyTargets}`
  else
    url = "https://api.frankfurter.app/latest?from=USD&to=EUR"
  end
  -- (noctalia.http answers every request, a refused one too, so there's no second path here.)
  noctalia.http({ url = url, headers = { "Accept: application/json" } }, function(r)
    local data = r.ok and r.status == 200 and noctalia.json.decode(r.body or "") or nil
    done(type(data) == "table" and type(data.rates) == "table" and data.rates or nil)
  end)
end

local function withRates(pricing, rates, opts)
  if rates == nil then return end
  if opts.currencyTargets then
    if next(rates) ~= nil then pricing.currency_rates = rates end
  elseif tonumber(rates.EUR) then
    pricing.usd_eur_rate = tonumber(rates.EUR)
  end
end

local function loadPricing(opts, d, done)
  local path = `{opts.dataDir}/pricing-cache.json`
  local cached = readJson(path)
  if cached and cached.updated == d.today then
    -- The rates can fail on their own: retried on every run until one lands.
    if opts.currencyTargets and (type(cached.currency_rates) ~= "table" or next(cached.currency_rates) == nil) then
      fetchRates(opts, function(rates)
        withRates(cached, rates, opts)
        if cached.currency_rates then writeJson(path, cached) end
        done(cached)
      end)
      return
    end
    done(cached)
    return
  end
  noctalia.http({ url = LITELLM_URL }, function(r)
    local raw = r.ok and r.status == 200 and noctalia.json.decode(r.body or "") or nil
    local fresh = type(raw) == "table" and pricingFrom(raw, d.today) or nil
    raw = nil
    if fresh == nil then
      done(cached) -- yesterday's prices beat none
      return
    end
    fetchRates(opts, function(rates)
      withRates(fresh, rates, opts)
      writeJson(path, fresh)
      done(fresh)
    end)
  end)
end

-- ── tokens per profile (the awk script) ────────────────────────────────────

local FIELDS = {
  "type", "timestamp", "sessionId", "message.model",
  "message.usage.input_tokens", "message.usage.output_tokens",
  "message.usage.cache_read_input_tokens", "message.usage.cache_creation_input_tokens",
}

local function familyOf(model)
  local parts = string.split(model, "-")
  if #parts >= 3 and parts[1] == "claude" then return parts[2] end
  return model
end

local function countTokens(projDir, pricing, d, done)
  local models = type(pricing) == "table" and type(pricing.models) == "table" and pricing.models or nil
  local acc = {
    day = {}, dayCost = {}, weekModel = {}, weekSess = {},
    weekTokens = 0, weekMsgs = 0, monthTokens = 0, weekCost = 0, monthCost = 0,
  }
  local function add(row)
    if row.type ~= "assistant" then return end
    local date = type(row.timestamp) == "string" and row.timestamp:sub(1, 10) or "null"
    local model = type(row["message.model"]) == "string" and row["message.model"] or "unknown"
    local inp, out = num(row["message.usage.input_tokens"]), num(row["message.usage.output_tokens"])
    local cr, cw = num(row["message.usage.cache_read_input_tokens"]), num(row["message.usage.cache_creation_input_tokens"])
    local total = inp + out + cr + cw
    local family = familyOf(model)
    acc.day[date] = (acc.day[date] or 0) + total
    if date >= d.weekStart then
      acc.weekModel[family] = (acc.weekModel[family] or 0) + total
      acc.weekTokens += total
      acc.weekMsgs += 1
      acc.weekSess[type(row.sessionId) == "string" and row.sessionId or "unknown"] = true
    end
    if date >= d.monthStart then acc.monthTokens += total end
    local p = models and models[family]
    if type(p) == "table" then
      local cost = inp * num(p.input) + out * num(p.output) + cr * num(p.cache_read) + cw * num(p.cache_write)
      acc.dayCost[date] = (acc.dayCost[date] or 0) + cost
      if date >= d.weekStart then acc.weekCost += cost end
      if date >= d.monthStart then acc.monthCost += cost end
    end
  end

  local function finish()
    local daily, dailyCosts = {}, {}
    for i, day in d.days do
      daily[i] = acc.day[day] or 0
      dailyCosts[i] = round2(acc.dayCost[day] or 0)
    end
    local weekModels = {}
    for m, v in pairs(acc.weekModel) do
      if v > 0 then weekModels[m] = v end
    end
    local sessions = 0
    for _ in pairs(acc.weekSess) do sessions += 1 end
    done({
      week_tokens = acc.weekTokens, week_messages = acc.weekMsgs, week_sessions = sessions,
      month_tokens = acc.monthTokens, daily = daily, daily_costs = dailyCosts, week_models = weekModels,
      today_cost = round2(acc.dayCost[d.today] or 0), week_cost = round2(acc.weekCost), month_cost = round2(acc.monthCost),
    })
  end

  -- (findFiles and jsonLines always call back, a refused one with nothing.)
  tourne.findFiles(projDir, { name = "*.jsonl", newerThan = os.time() - MONTH_SECS, max = 20000 }, function(found)
    -- Read in batches of about 32 MB, so one call never holds more rows than the plugin's memory allows.
    local batches, batch, bytes = {}, {}, 0
    for _, f in found do
      if #batch > 0 and (bytes + f.size > 32 * 1048576 or #batch >= 40) then
        table.insert(batches, batch)
        batch, bytes = {}, 0
      end
      table.insert(batch, f.path)
      bytes += f.size
    end
    if #batch > 0 then table.insert(batches, batch) end
    local i = 0
    local function nextBatch()
      i += 1
      if batches[i] == nil then return finish() end
      tourne.jsonLines({ files = batches[i], match = '"type":"assistant"', fields = FIELDS, maxRows = 60000 }, function(files)
        for _, file in files do
          for _, row in file.rows or {} do add(row) end
        end
        nextBatch()
      end)
    end
    nextBatch()
  end)
end

-- ── rate windows per profile (the usage API) ───────────────────────────────

local function unknownUsage()
  return { sub = "unknown", tier = "unknown", fiveUtil = 0, fiveReset = "", sevenUtil = 0, sevenReset = "", extra = false, limits = {} }
end

local function flattenLimits(resp)
  local out = {}
  if type(resp.limits) ~= "table" then return out end
  for _, l in resp.limits do
    if type(l) == "table" then
      local scope = type(l.scope) == "table" and l.scope or {}
      local model = type(scope.model) == "table" and scope.model or {}
      table.insert(out, {
        kind = l.kind or "",
        group = l.group or "",
        model = model.display_name or "",
        percent = l.percent or 0,
        resets_at = l.resets_at or "",
        severity = l.severity or "normal",
        is_active = l.is_active or false,
      })
    end
  end
  return out
end

local function fetchUsage(credsFile, cacheFile, opts, done)
  local u = unknownUsage()
  local creds = readJson(credsFile)
  if creds == nil then
    done(u)
    return
  end
  local oauth = type(creds.claudeAiOauth) == "table" and creds.claudeAiOauth or {}
  u.sub = oauth.subscriptionType or "unknown"
  u.tier = oauth.rateLimitTier or "unknown"

  local function apply(resp)
    resp = type(resp) == "table" and resp or {}
    local five = type(resp.five_hour) == "table" and resp.five_hour or {}
    local seven = type(resp.seven_day) == "table" and resp.seven_day or {}
    local extra = type(resp.extra_usage) == "table" and resp.extra_usage or {}
    u.fiveUtil, u.fiveReset = five.utilization or 0, five.resets_at or ""
    u.sevenUtil, u.sevenReset = seven.utilization or 0, seven.resets_at or ""
    u.extra = extra.is_enabled or false
    u.limits = flattenLimits(resp)
    done(u)
  end

  local cache = readJson(cacheFile)
  if cache and os.time() - num(cache.cached_at) < USAGE_CACHE_TTL then
    apply(cache.data)
    return
  end
  local token = type(oauth.accessToken) == "string" and oauth.accessToken or ""
  oauth, creds = nil, nil
  if token == "" then
    apply({})
    return
  end
  noctalia.http({
    url = USAGE_URL,
    headers = {
      "Authorization: Bearer " .. token,
      "Accept: application/json",
      "Content-Type: application/json",
      `User-Agent: claude-code/{opts.version or "2.0.0"}`,
      "anthropic-beta: oauth-2025-04-20",
    },
  }, function(r)
    local resp = r.ok and noctalia.json.decode(r.body or "") or nil
    if type(resp) == "table" and resp.five_hour ~= nil then
      writeJson(cacheFile, { cached_at = os.time(), data = resp })
      apply(resp)
    elseif cache then
      apply(cache.data)
    else
      apply({})
    end
  end)
  token = nil
end

-- ── profiles: the default account, then CCS instances ──────────────────────

local function profiles(opts)
  local list = {}
  local info = noctalia.fileInfo(`{opts.claudeDir}/projects`)
  if info and info.isDir then
    table.insert(list, { name = "default", projects = `{opts.claudeDir}/projects`, creds = `{opts.claudeDir}/.credentials.json` })
  end
  if opts.ccsDir then
    local names = noctalia.listDir(opts.ccsDir) or {}
    table.sort(names)
    for _, entry in names do
      local inst = `{opts.ccsDir}/{entry}`
      local p = noctalia.fileInfo(`{inst}/projects`)
      local name = entry:gsub("[,|:]", "")
      if p and p.isDir and name ~= "" then
        table.insert(list, { name = name, projects = `{inst}/projects`, creds = `{inst}/.credentials.json` })
      end
    end
  end
  return list
end

-- ── the whole run ──────────────────────────────────────────────────────────

-- opts: claudeDir, dataDir, ccsDir (claudecode: ~/.ccs/instances; nil for the default account only), version,
-- currencyTargets + currencyApiUrl (Cockpit: every rate in one request) or neither (USD to EUR only).
-- done(data): the object get-claude-usage --json prints.
function M.collect(opts, done)
  local d = dates()
  loadPricing(opts, d, function(pricing)
    local list = profiles(opts)
    local results = {}
    local i = 0
    local function assemble()
      local out = {
        subscription_type = "unknown", rate_limit_tier = "unknown",
        five_hour_util = 0, five_hour_reset = "", seven_day_util = 0, seven_day_reset = "",
        extra_usage_enabled = false, limits = {},
        week_messages = 0, week_sessions = 0, week_tokens = 0, month_tokens = 0, week_models = {},
        alltime_sessions = 0, alltime_messages = 0, first_session = "unknown",
        daily = { 0, 0, 0, 0, 0, 0, 0 }, daily_costs = { 0, 0, 0, 0, 0, 0, 0 },
        today_cost = 0, week_cost = 0, month_cost = 0,
        profiles = {}, profile_data = {},
      }
      for _, r in results do
        local t, u = r.tokens, r.usage
        if r.name == "default" then
          out.subscription_type, out.rate_limit_tier = u.sub, u.tier
          out.five_hour_util, out.five_hour_reset = num(u.fiveUtil), u.fiveReset
          out.seven_day_util, out.seven_day_reset = num(u.sevenUtil), u.sevenReset
          out.extra_usage_enabled = u.extra == true
          out.limits = u.limits
        end
        out.week_tokens += t.week_tokens
        out.week_messages += t.week_messages
        out.month_tokens += t.month_tokens
        out.week_sessions += t.week_sessions
        for k = 1, 7 do
          out.daily[k] += t.daily[k]
          out.daily_costs[k] = round2(out.daily_costs[k] + t.daily_costs[k])
        end
        out.today_cost = round2(out.today_cost + t.today_cost)
        out.week_cost = round2(out.week_cost + t.week_cost)
        out.month_cost = round2(out.month_cost + t.month_cost)
        for m, v in pairs(t.week_models) do
          out.week_models[m] = math.floor((out.week_models[m] or 0) + v)
        end
        table.insert(out.profiles, r.name)
        table.insert(out.profile_data, {
          name = r.name, subscription = u.sub, tier = u.tier,
          week_tokens = t.week_tokens, month_tokens = t.month_tokens,
          week_messages = t.week_messages, week_sessions = t.week_sessions,
          today_cost = t.today_cost, week_cost = t.week_cost, month_cost = t.month_cost,
          five_hour_util = num(u.fiveUtil), seven_day_util = num(u.sevenUtil),
          five_hour_reset = u.fiveReset, seven_day_reset = u.sevenReset, extra_usage = u.extra == true,
          limits = u.limits, daily = t.daily, daily_costs = t.daily_costs, week_models = t.week_models,
        })
      end
      local stats = readJson(`{opts.claudeDir}/stats-cache.json`)
      if stats then
        out.alltime_sessions = num(stats.totalSessions)
        out.alltime_messages = num(stats.totalMessages)
        local first = type(stats.firstSessionDate) == "string" and stats.firstSessionDate or "unknown"
        out.first_session = first:match("^([^T]*)") or first
      end
      if opts.currencyTargets then
        out.currency_rates = type(pricing) == "table" and type(pricing.currency_rates) == "table" and pricing.currency_rates or {}
      else
        out.usd_eur_rate = type(pricing) == "table" and num(pricing.usd_eur_rate) or 0
      end
      done(out)
    end
    -- One profile after another (the script ran them side by side): the runtime reads on its own threads anyway.
    local function nextProfile()
      i += 1
      local p = list[i]
      if p == nil then return assemble() end
      local cacheFile = p.name == "default" and `{opts.dataDir}/usage-cache.json` or `{opts.dataDir}/usage-cache-{p.name}.json`
      countTokens(p.projects, pricing, d, function(tokens)
        fetchUsage(p.creds, cacheFile, opts, function(u)
          table.insert(results, { name = p.name, tokens = tokens, usage = u })
          nextProfile()
        end)
      end)
    end
    nextProfile()
  end)
end

return M
