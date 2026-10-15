# Tourne Hooks decided; b53 back for the hook parts meanwhile

When: 2026-10-07, just after midnight, Claude Code in the terminal on the desktop PC, branch `claude/project-thread-0rkfwp` (started at 3a1f5e1).

Why: Tourne wants the Windows 7 Explorer look back ("id really like to move away from having to use windhawk if at all possible to achieve classic looks") and sent screenshots of the target (2026-10-02 and 2026-10-04: Explorer with the Windows 7 navigation pane, drive bars and text in the palette's green, pixel font; Task Manager and Chrome in the same colours; Explorer with glass).

Decisions (Tourne, 2026-10-07):
- Build Tourne Hooks (`research/plans/tourne-hooks-proposal.md`): Tourne Desktop's own DLL and loader for the parts that run inside other programs. Explorer first (explorerStyle, explorerColors, themeGaps, classicMenus), then textColor and the menu parts in their listed programs, then iconRedirect and tweaks.
- Meanwhile b53 runs again in Windhawk with only the hook parts on.
- Programs run as administrator: plan the optional elevated helper too.

What changed:
- c54f16c: the proposal.
- 8667c12: `tools/windhawk/Set-SuiteParts.ps1` (run as administrator): `-HooksOnly` turns b53's drawn parts off and hook parts on, adds TourneDesktop.exe to its exclude list, enables it; `-Off <part>` later, as each part moves to Tourne Desktop. Exports the mod's settings first. Claude can't write HKLM, so Tourne runs it.

Next:
- Tourne: run the script as administrator, then turn b53 off and on in Windhawk; send a screenshot of Explorer as the reference for step 1.
- Claude: step 1 of Tourne Hooks (DLL with MinHook behind the Windhawk shim, symbol cache from Microsoft's symbol server read with dbghelp, loading into the Explorer Tourne Desktop starts, crash guard, Studio switches).
