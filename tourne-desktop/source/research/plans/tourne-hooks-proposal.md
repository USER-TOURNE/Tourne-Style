# Tourne Hooks: the classic look without Windhawk

Status: not going ahead (2026-10-07). The hook parts stay in Windhawk (the suite with only them on, `tools/windhawk/Set-SuiteParts.ps1`). Kept for the record.

## The problem

Tourne Desktop draws its own taskbar, tray, clock, switcher and flyouts, so those need nothing from Windhawk. The rest of the Windows 7 look changes how *other* programs draw, and that means running code inside them (hooks). Today only Windhawk does that, and b53 is switched off, so these parts don't run:

| b53 part (on in b53) | What it does | Runs inside |
| --- | --- | --- |
| explorerStyle (with explorer-layout) | Explorer windows in a full visual style, the command bar, the details pane at the bottom, drives grouped | explorer.exe |
| explorerColors | Drive bars, selections, preview pane in your colours | explorer.exe |
| themeGaps | The parts a visual style misses, filled from its colours | explorer.exe and others |
| classicMenus | Classic menus instead of "immersive" ones | explorer.exe and others |
| textColor | Text in your green, tooltips, separators, Chrome menus | its program list (explorer, chrome, notepad++, regedit...) |
| menuNoIcons, menuNoDuplicates | Context menu clean-up | their program lists |
| iconRedirect | Themed icons in Explorer, Start and apps | explorer.exe and others |
| tweaks | Winaero Tweaker options | explorer.exe |

Target look: my screenshots of 2026-10-02 and 2026-10-04 (Explorer with the Windows 7 navigation pane and command bar, drive bars and text in the palette's green, the pixel font; Task Manager and Chrome in the same colours; Explorer with glass).

## The plan

A DLL of Tourne's own, `tourne-hooks.dll`, that Tourne Desktop loads into the programs these parts are for.

1. **The parts, unchanged.** The same part sources compile into the DLL behind a Windhawk-shaped shim, as the taskbar and switcher already do in Tourne Desktop. Function hooks go through MinHook (BSD licence). Settings come from the same `tourne.conf` and `tourne-gui.conf`, and a change is signalled to every loaded copy.
2. **Symbol hooks** (explorerStyle, classicMenus, iconRedirect find private functions by name): Tourne Desktop downloads the matching symbol file from Microsoft's symbol server once per Windows file version, reads it with Windows' own dbghelp, and keeps a small cache of the addresses. The DLL only reads that cache. A part whose function can't be found turns itself off in that program, as in Windhawk.
3. **Getting in.**
   - Explorer: Tourne Desktop already starts the hidden Explorer, so the DLL goes in as Explorer starts, before any window opens. Explorer windows get the full look from the first one.
   - Other programs (textColor's list and the menu parts' lists): Tourne Desktop watches for windows from outside (no hook) and loads the DLL when a listed program shows its first window. Programs already open when Tourne Desktop starts get it then.
   - Nothing goes into a program that isn't on a part's list. There is no global hook, so games and anti-cheat (Vanguard) never see it.
4. **Safety.**
   - A crash guard like shell mode's: if a program crashes twice within a short time of the DLL loading, the parts stop loading into it, and the Studio says so.
   - Each part has its own switch in the Studio.
   - When Tourne Desktop exits, hooks are taken out before the DLL unloads.
5. **Limits.**
   - Programs running as administrator (regedit, mmc, Task Manager when elevated) can't be reached from a normal Tourne Desktop. Windhawk reaches them through its service. These could come later through an optional elevated helper.
   - 32-bit programs would need a second, 32-bit DLL. Everything on the current list is 64-bit.

## Order

1. The DLL, the shim, the symbol cache and Explorer loading, with explorerStyle, explorerColors, themeGaps and classicMenus: the Windows 7 Explorer windows. Checked against the b53 screenshots.
2. textColor and the menu parts in their listed programs.
3. iconRedirect and tweaks.

Each step is swapped in and checked on its own. After step 3, Windhawk isn't needed for the suite (other Windhawk mods I use are separate).

## Meanwhile

b53 can run again with only the hook parts on (its taskbar, switcher, system icons, tray, clock and Start off), so the Explorer look is back today while Tourne Desktop does the rest. Each step above then turns its parts off in b53.

## Decisions for Tourne

1. Go on Tourne Hooks, in the order above?
2. Programs run as administrator: leave them out for now, or plan the elevated helper too?
3. Turn b53 back on with only the hook parts for now?
