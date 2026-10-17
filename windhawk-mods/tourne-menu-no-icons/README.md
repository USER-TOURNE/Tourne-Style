# `Tourne'Menu` **[No Icons]**

Removes the icons from context menu items, so a menu is text and nothing else.

An icon on a menu item is a bitmap hung off the item itself: `MIIM_BITMAP` in the mask
and an `HBITMAP` in `hbmpItem`. Whoever builds the menu sets it, and there is no system
switch to turn it off. This mod clears it as the item goes in.

The two entry points that matter are `InsertMenuItemW`, which adds an item, and
`SetMenuItemInfoW`, which changes one already added. Both live in user32, and anything
building a menu from outside user32 has to call them to do it, which is what makes this
reachable at all. That covers the shell's own menus, the `New` submenu among them, and
any application or shell extension that adds items the ordinary way.

`SetMenuItemBitmaps` is covered too. It is the older call, and while it is meant for the
check mark rather than an icon, some applications use it as one.

## What it leaves alone

**System glyphs.** A handful of reserved `HBMMENU_` values ask Windows to draw a control
rather than an icon: the close and minimise buttons on a menu bar, for one. Those are not
decoration, and clearing them leaves a blank, dead control, so they are kept.

**Owner-drawn items.** An item marked `HBMMENU_CALLBACK`, or `MFT_OWNERDRAW`, paints
itself in response to `WM_DRAWITEM`. There is no bitmap to remove, because the icon is
drawn later by whoever owns the menu. Those are reported under **Verbose logging** rather
than touched, so a menu that does not respond to this mod can be identified as such
instead of looking like a failure.

**Anything not drawn with a Win32 menu at all.** The Windows 11 modern context menu is
XAML and never calls these functions. Use the classic menu, which this machine already
does, or a Windhawk styler.

## Scope

**Processes** decides where the mod loads, as a list of executable names. `*` means
everywhere, which is a big hammer. The default list is the shell and the surfaces that
host shell menus.
