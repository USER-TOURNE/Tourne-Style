# Hide the app icon in taskbar thumbnail previews

Targets the classic thumbnail window (`TaskListThumbnailWnd`), which is what you get
with StartAllBack's enhanced classic taskbar or ExplorerPatcher's Windows 10 taskbar.
It is not the XAML thumbnail of the stock Windows 11 taskbar.

The mod subclasses that window and raises a flag while it paints. Anything drawn inside
that flag that looks like the header icon gets dropped:

- icon APIs (`DrawIconEx`, `DrawIcon`, `DrawIconIndirect`, image lists, `DrawThemeIcon`)
  are dropped unconditionally, and
- blits (`BitBlt`, `StretchBlt`, `AlphaBlend`, `TransparentBlt`, `MaskBlt`) are dropped
  only when the destination is a small rectangle inside the icon zone, so the header
  background and the preview itself are left alone.

## Tuning

The icon zone defaults suit a header whose title starts near x=53. If your title sits
elsewhere, turn on **Verbose logging**, hover a taskbar button, and read the logged
destination rectangles: the icon is the small square just left of the title. Set
**Icon zone right/bottom** just past it.

**Title indent:** removing the icon leaves the title where it was. Use **Shift title
left** to pull it back over the freed space.

## Border color

The outline around the preview is painted by the visual style, as part 1 of the theme
background that fills the card. **Border color** repaints it in place, right after the
theme has drawn it, so it works whatever msstyles is loaded.

Accepted values, with or without the leading hash: `AARRGGBB`, `RRGGBB`, `default` to
leave the theme's own outline alone, and `none` to erase it. Alpha is honored: the
border is alpha-blended over the card, so `80` gives a half-strength outline and `00`
is the same as `none`. Use **Border thickness** for anything heavier than a hairline.

Erasing works by copying the pixels just inside the outline over it, so it follows
whatever gradient or texture the visual style put behind it.

The DWM window border is set as well, for setups where the preview has one. On
StartAllBack it does nothing, and the log says so with `hr 0x00000000` and no visible
change.
