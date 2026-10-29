// The examples, written into the Layouts folder the first time it is made, and
// run by tools/layouts/layouttest.cpp. They are the documentation: short,
// commented, and each one does something the layouts built in don't.

struct LsExample {
    const char* name;
    const char* text;
};

constexpr LsExample kLsExamples[] = {
    {"example-columns.luau", R"LUAU(-- The simplest layout there is: a column each, side by side.
--
-- A layout is handed one table and answers with one rectangle per window.
--   ask.screen   {x, y, w, h}: the room to lay out in, gaps around it already off
--   ask.count    how many windows
--   ask.gap      the gap to leave between them, in pixels
--   ask.ratio    the main window's share, in percent (Studio's ratio keys)
--   ask.options  the text you wrote after the layout's name in Studio
--   ask.windows  one each: x, y, w, h (where it is now), program, class, title, focused
--
-- Answer with a list of {x = , y = , w = , h = }, one per window, in the same
-- order. Four numbers in a row, {x, y, w, h}, work too.
function layout(ask)
    local out = {}
    local screen, gap, n = ask.screen, ask.gap, ask.count
    local each = (screen.w - gap * (n - 1)) / n
    for i = 1, n do
        out[i] = {x = screen.x + (each + gap) * (i - 1), y = screen.y, w = each, h = screen.h}
    end
    return out
end
)LUAU"},
    {"example-main-and-grid.luau", R"LUAU(-- The main window on the left, everything else in a grid on the right.
--
-- Write the number of grid columns after the layout's name in Studio ("2" by
-- default), so one layout file serves several shapes.
function layout(ask)
    local screen, gap, n = ask.screen, ask.gap, ask.count
    if n == 1 then return {{x = screen.x, y = screen.y, w = screen.w, h = screen.h}} end
    -- Never more columns than there are windows to put in them, or the last
    -- row would be a stripe of empty screen.
    local columns = math.max(1, math.floor(tonumber(ask.options) or 2))
    columns = math.min(columns, n - 1)
    local mainWidth = math.floor((screen.w - gap) * ask.ratio / 100)
    local out = {{x = screen.x, y = screen.y, w = mainWidth, h = screen.h}}
    local rest, left = n - 1, screen.x + mainWidth + gap
    local width = screen.w - mainWidth - gap
    local rows = math.ceil(rest / columns)
    for i = 1, rest do
        local column = (i - 1) % columns
        local row = math.floor((i - 1) / columns)
        local tall = (screen.h - gap * (rows - 1)) / rows
        local wide = (width - gap * (columns - 1)) / columns
        out[i + 1] = {x = left + (wide + gap) * column, y = screen.y + (tall + gap) * row, w = wide, h = tall}
    end
    return out
end
)LUAU"},
    {"example-by-program.luau", R"LUAU(-- A layout that knows which window is which: the players along the bottom,
-- everything else in a row above them.
--
-- Write the programs after the layout's name in Studio, commas between
-- ("Spotify.exe, Discord.exe"). ask.windows[i] carries each window's program,
-- class and title, so a layout can place a window by what it is rather than by
-- where it happens to be in the order.
local function wanted(options, window)
    for name in string.gmatch(options or "", "[^,]+") do
        name = string.lower((string.gsub(name, "^%s*(.-)%s*$", "%1")))
        if name ~= "" and (string.lower(window.program) == name or string.lower(window.class) == name) then
            return true
        end
    end
    return false
end

function layout(ask)
    local screen, gap = ask.screen, ask.gap
    local strip, rest = {}, {}
    for i, window in ipairs(ask.windows) do
        table.insert(wanted(ask.options, window) and strip or rest, i)
    end
    if #strip == 0 or #rest == 0 then  -- nothing to put along the bottom, or nothing above
        strip, rest = {}, {}
        for i = 1, ask.count do table.insert(rest, i) end
    end
    local out = {}
    local tall = #strip > 0 and math.floor((screen.h - gap) * 30 / 100) or 0
    local above = screen.h - (tall > 0 and tall + gap or 0)
    local function row(list, y, height)
        local each = (screen.w - gap * (#list - 1)) / math.max(1, #list)
        for place, i in ipairs(list) do
            out[i] = {x = screen.x + (each + gap) * (place - 1), y = y, w = each, h = height}
        end
    end
    row(rest, screen.y, above)
    if tall > 0 then row(strip, screen.y + above + gap, tall) end
    return out
end
)LUAU"},
};
