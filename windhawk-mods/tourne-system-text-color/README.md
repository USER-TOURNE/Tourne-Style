# `Tourne'Text` **[System Text Color]**

Repaints white system text in whatever color you want.

Text that looks white reaches the screen two ways on a classic Win32 surface, and this
mod covers both:

- **GDI text.** `SetTextColor` sets the color on the device context, and everything from
  `TextOut` to `DrawText` picks it up from there. Any color bright enough to count as
  white is swapped for yours on the way through.
- **Themed text.** `DrawThemeText` and `DrawThemeTextEx` take their color from the visual
  style. The theme's color is read first, and if it is white, the call is redirected to
  yours.

Menus are the awkward case. Windows paints them from inside user32 and uxtheme, which
call their own internal drawing rather than the exported entry points, so neither hook
above ever sees them. They are caught one level down instead, at `ExtTextOut` and
`TextOut`, where every piece of GDI text ends up no matter who set the color: if the
color already on the device context looks white, it is swapped for the duration of that
one call. Monochrome device contexts are skipped, because text drawn into a 1bpp bitmap
is a mask, not something anybody sees.

## Tooltips

Tooltips get their own two settings, because neither rule above fits them: their text is
usually dark rather than white, and their background is painted by the visual style.

Tooltip windows are subclassed, including ones that already existed when the mod loaded,
so the mod knows when one is painting. During that paint the background is filled with
**Tooltip background**, over the theme's own, and every piece of text is drawn in
**Tooltip text color** whatever color it started as. Leave either empty to keep what the
visual style does.

Some tooltips are not painted with GDI at all. StartAllBack renders its taskbar tooltip
itself, and no amount of hooking the drawing reaches it. Those are covered from the other
end instead, by answering the questions any renderer has to ask: `COLOR_INFOTEXT` and
`COLOR_INFOBK` among the system colors, and `TMT_TEXTCOLOR`, `TMT_FILLCOLOR` and
`TMT_BORDERCOLOR` on any theme opened for a tooltip class. Both are answered with the
same settings, so a tooltip is colored whether it draws itself or lets Windows do it.

**Keep tooltip border** insets the fill by one pixel so the outline the style drew
survives. Turn it off to fill the tooltip edge to edge.

**Tooltip border color** repaints that outline instead of preserving it, drawn over the
fill at the tooltip's own bounds, with **Tooltip border thickness** for anything heavier
than a hairline. Setting it overrides Keep tooltip border, since the fill is then inset
by the thickness of the new outline rather than by one pixel.

Both colors take alpha, and the background really is blended, so `#40000000` gives a
tint rather than a solid fill.

## Menu separators

A classic menu separator is an etched edge, and Windows draws it from two system colors:
`COLOR_3DSHADOW` for the upper line, `COLOR_3DHILIGHT` for the lower one. Setting those
directly does work, but they are system wide, so every other etched edge changes with
them: group box frames, classic dialog borders, toolbar dividers.

**Menu separator color** repaints the separator from inside the draw call instead, gated
on the device context belonging to a menu, so nothing outside a menu is touched. Leave it
empty and separators are left exactly as Windows draws them.

**Menu separator shadow color** is the upper of the two lines, the one that gives the
separator its etched look. Leave it empty for a single flat line in the separator color,
which is usually what suits a dark menu.

Both take alpha and are genuinely blended against what is behind them, so `#80000000` is
a real half-transparent black rather than a pre-mixed approximation of one.

**Menu separator thickness** applies to each line on its own, so a separator with a
shadow set to thickness 2 stands four pixels tall.

## What it cannot reach

**XAML and WinUI surfaces** are not drawn with GDI: Settings, Start, the stock Windows 11
taskbar and the modern context menus render their own text. Use the Windhawk stylers for
those, overriding brushes such as `TextFillColorPrimaryBrush`.

**Apps that draw their own text**, such as browsers and Electron apps, are untouched by
design. Leave them out of the process list and nothing in them changes.

**A visual style's own colors** can be edited at the source with msstyleEditor instead,
which needs no injection at all. That is the better tool for a blanket change; this mod
is for everything the style does not own, and for per-process control.

## Scope

**Processes** decides where the mod loads. It is a list of executable names, and `*`
means everywhere, which is a big hammer: every process on the system gets the hooks. The
default list is the shell and the classic control-panel hosts.

## Tuning

**White threshold** decides what counts as white. A color qualifies when all three of its
channels are at or above it, so 200 catches off-whites and light greys, while 255 matches
pure white only. Lower it too far and colored text starts changing too.
