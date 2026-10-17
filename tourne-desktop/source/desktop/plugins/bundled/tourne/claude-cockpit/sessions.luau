--!nonstrict
-- Ported to Tourne Desktop from nightwatch75's Claude Cockpit (MIT).
-- sessions: pure data module that lists every local Claude Code session (what
-- list-claude-sessions did) and shapes the flat list into project groups the
-- panel renders. No UI state, no closures over caller state: the panel alone
-- owns what is expanded, searched or being confirmed.
--
-- Never polled: M.refresh is called from the panel's onOpen and its refresh
-- button only. A "project" here is a distinct cwd a session was run from,
-- NOT literally a `~/.claude/projects/<encoded>/` directory, though most of
-- the time they line up 1:1; see decodeProjectDir for the one case they
-- don't (a session that states no cwd at all).
--
-- Windows: tourne.findFiles and tourne.jsonLines stand in for find, grep, tac
-- and jq; a delete moves the files to the Recycle Bin (tourne.trash) where
-- Linux ran rm -rf.

local M = {}

local HOME = noctalia.expandPath("~")
local PROJECTS_DIR = noctalia.expandPath("~/.claude/projects")
M.GLOBAL_CLAUDE_MD = noctalia.expandPath("~/.claude/CLAUDE.md")

local function join(dir, name)
    return (dir:gsub("[\\/]+$", "")) .. "\\" .. name
end

-- Best-effort decode of an encoded project dir name, used ONLY when a
-- session never states its own cwd. Claude Code writes C:\work\app as
-- "C--work-app". Not reversible in general (a literal "-" in a path is
-- indistinguishable from an encoded "\"), so this is a display fallback
-- only: actions always use the session's recorded paths.
local function decodeProjectDir(encoded)
    if type(encoded) ~= "string" then
        return nil
    end
    local drive, rest = encoded:match("^(%a)%-%-(.*)$")
    if drive == nil then
        return nil
    end
    return drive .. ":\\" .. rest:gsub("%-", "\\")
end

-- One session record from the script, reshaped for the panel: a stable
-- label (AI title, else opening prompt cut short, else the id) plus the
-- jsonl path and subagent sidecar dir a delete acts on. `firstPrompt` stays
-- its own field, untruncated, for the panel's preview.
local function shapeSession(raw, cwd)
    local id = raw.id
    local aiTitle = (type(raw.title) == "string") and raw.title or ""
    local firstPrompt = (type(raw.firstPrompt) == "string") and raw.firstPrompt or ""
    local label = aiTitle
    if label == "" then
        if firstPrompt ~= "" then
            label = (#firstPrompt > 72) and (firstPrompt:sub(1, 72) .. "…") or firstPrompt
        else
            label = noctalia.tr("session_untitled", { id = id:sub(1, 8) })
        end
    end
    local dir = tostring(raw.projectDir or "")
    return {
        id = id,
        cwd = cwd,
        gitBranch = (type(raw.gitBranch) == "string" and raw.gitBranch ~= "") and raw.gitBranch or nil,
        mtime = tonumber(raw.mtime) or 0,
        size = tonumber(raw.size) or 0,
        label = label,
        firstPrompt = firstPrompt,
        -- Model + context size from the session's last assistant turn; 0/""
        -- for a session with no assistant turn yet.
        model = (type(raw.model) == "string") and raw.model or "",
        contextTokens = tonumber(raw.contextTokens) or 0,
        jsonlPath = join(join(PROJECTS_DIR, dir), id .. ".jsonl"),
        sidecarDir = join(join(PROJECTS_DIR, dir), id),
    }
end

-- Groups the flat session list by resolved cwd, sorted most-recent-first
-- both within a project and across projects.
function M.groupByProject(rawSessions)
    local byCwd = {}
    local order = {}
    for _, raw in ipairs(rawSessions) do
        local cwd = raw.cwd
        if type(cwd) ~= "string" or cwd == "" then
            cwd = decodeProjectDir(raw.projectDir) or noctalia.tr("unknown_project")
        end
        local group = byCwd[cwd]
        if group == nil then
            group = { cwd = cwd, claudeMdPath = join(cwd, "CLAUDE.md"), sessions = {}, lastActivity = 0 }
            byCwd[cwd] = group
            table.insert(order, group)
        end
        local session = shapeSession(raw, cwd)
        table.insert(group.sessions, session)
        if session.mtime > group.lastActivity then
            group.lastActivity = session.mtime
        end
    end
    for _, group in ipairs(order) do
        table.sort(group.sessions, function(a, b)
            return a.mtime > b.mtime
        end)
    end
    table.sort(order, function(a, b)
        return a.lastActivity > b.lastActivity
    end)
    return order
end

-- The rows of one jsonLines answer, by file path.
local function byPath(files)
    local out = {}
    for _, f in ipairs(type(files) == "table" and files or {}) do
        if type(f.path) == "string" and type(f.rows) == "table" then
            out[f.path] = f.rows
        end
    end
    return out
end

local function str(v)
    return type(v) == "string" and v or ""
end

-- Lists the sessions as list-claude-sessions did and hands the grouped
-- project list to done(projects, err); err is nil on success. A session is a
-- "<uuid>.jsonl" file directly inside a project directory; the sibling
-- "<uuid>\subagents\*.jsonl" sub-transcripts are deeper and never listed.
function M.refresh(done)
    if not noctalia.fileExists(PROJECTS_DIR) then
        done(M.groupByProject({}), nil)
        return
    end
    tourne.findFiles(PROJECTS_DIR, { name = "*.jsonl", maxDepth = 2, max = 20000 }, function(found)
        if type(found) ~= "table" then
            done(nil, noctalia.tr("err_sessions_fetch"))
            return
        end
        local files, info = {}, {}
        local function norm(p)
            return (p:gsub("/", "\\"):gsub("\\+$", ""):lower())
        end
        local root = norm(PROJECTS_DIR)
        for _, f in ipairs(found) do
            local path = str(f.path)
            local parent, name = path:match("^(.*)[\\/]([^\\/]+)$")
            local projectParent, projectDir = (parent or ""):match("^(.*)[\\/]([^\\/]+)$")
            local id = name and name:match("^([%x-]+)%.jsonl$")
            -- (A session id is always a UUID; anything else isn't a session.)
            if id and projectParent and norm(projectParent) == root then
                files[#files + 1] = path
                info[path] = { id = id, projectDir = projectDir, mtime = tonumber(f.mtime) or 0, size = tonumber(f.size) or 0 }
            end
        end
        if #files == 0 then
            done(M.groupByProject({}), nil)
            return
        end
        -- First line with a cwd (queue-operation and mode events can precede it),
        -- the last ai-title, the first last-prompt (the opening message) and the
        -- last assistant turn.
        tourne.jsonLines({ files = files, match = '"cwd":"', first = 1, fields = { "cwd", "gitBranch", "version" } }, function(heads)
            tourne.jsonLines({ files = files, match = '"type":"ai-title"', last = 1, fields = { "aiTitle" } }, function(titles)
                tourne.jsonLines({ files = files, match = '"type":"last-prompt"', first = 1, fields = { "lastPrompt" } }, function(prompts)
                    tourne.jsonLines({ files = files, match = '"type":"assistant"', last = 1, fields = { "message.model", "message.usage" } }, function(turns)
                        local h, t, p, a = byPath(heads), byPath(titles), byPath(prompts), byPath(turns)
                        local sessions = {}
                        for _, path in ipairs(files) do
                            local base = info[path]
                            local head = h[path] and h[path][1] or {}
                            local title = t[path] and t[path][1] or {}
                            local prompt = p[path] and p[path][1] or {}
                            local turn = a[path] and a[path][1] or {}
                            local u = type(turn["message.usage"]) == "table" and turn["message.usage"] or {}
                            table.insert(sessions, {
                                id = base.id, projectDir = base.projectDir, mtime = base.mtime, size = base.size,
                                cwd = str(head.cwd), gitBranch = str(head.gitBranch), version = str(head.version),
                                title = str(title.aiTitle), firstPrompt = str(prompt.lastPrompt),
                                model = str(turn["message.model"]),
                                contextTokens = (tonumber(u.input_tokens) or 0) + (tonumber(u.cache_read_input_tokens) or 0)
                                    + (tonumber(u.cache_creation_input_tokens) or 0),
                            })
                        end
                        done(M.groupByProject(sessions), nil)
                    end)
                end)
            end)
        end)
    end)
end

-- Moves one session's transcript and its subagent sidecar dir (if any) to the
-- Recycle Bin (rm -rf on Linux), so a delete made by mistake can be undone.
function M.deleteSession(session, done)
    local paths = { session.jsonlPath }
    if noctalia.fileExists(session.sidecarDir) then
        table.insert(paths, session.sidecarDir)
    end
    tourne.trash(paths, function(ok)
        done(ok == true)
    end)
end

-- ── CLAUDE.md discovery ──────────────────────────────────────────────────
-- The Sessions/CLAUDE.md tabs otherwise only know about projects that have
-- run a Claude Code session: a real CLAUDE.md nested deeper, or sitting in
-- a directory nobody has run `claude` from yet, would never show up. This
-- scans the user folder for every one instead (find-claude-md on Linux).

-- Same noise list as the original, plus AppData (programs' own files, never a
-- project). Network shares aren't under the user folder on Windows, and linked
-- folders (junctions) aren't followed, so nothing remote can stall the scan.
local PRUNE = { ".git", "node_modules", ".cache", ".venv", "AppData" }

-- Every real CLAUDE.md under the user folder. done(paths, err), err nil on success.
function M.findClaudeMdFiles(done)
    tourne.findFiles(HOME, { name = "CLAUDE.md", prune = PRUNE, max = 5000 }, function(found)
        if type(found) ~= "table" then
            done(nil, noctalia.tr("err_claudemd_scan"))
            return
        end
        local paths = {}
        for _, f in ipairs(found) do
            if type(f.path) == "string" then
                table.insert(paths, f.path)
            end
        end
        done(paths, nil)
    end)
end

return M
