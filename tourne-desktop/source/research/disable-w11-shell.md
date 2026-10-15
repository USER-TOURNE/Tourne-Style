# Turning off Windows 11's taskbar and Start menu (for running without StartAllBack)

Checked 2026-09-29 on Windows 11 26200.9550.

**Built in b29** (2026-09-30), in the Explorer relay (`suite-redirect.template.cpp`, "Windows' own taskbar and Start"). Symbols checked against this PC's PDBs (twinui.pcshell 9507F3DA..., Taskbar 0A5DD95B..., Taskbar.View 24534974...): `XamlLauncher::ShowStartView`, `TrayUI::Unhide`, `CSecondaryTray::_Unhide` all present. What differs from the plan below:
- `ViewCoordinator::ShouldTaskbarBeExpanded` isn't hooked. A cloaked taskbar can't be seen or clicked, whether expanded or not. Explorer's own uncloaking is refused through a `DwmSetWindowAttribute` hook instead (taskbar windows only).
- The relay waits on the bar's process: if the bar ends, the taskbars are uncloaked and shown.
- Start goes to the suite only while StartAllBackX64.dll isn't loaded. The relay signals this with a named event (`TourneSuite_ShellStart_<mod id>`) and the `TourneSuite_ShellStart` message, and the bar then drops its WH_KEYBOARD_LL hook.
- Step 3 (blocking StartMenuExperienceHost) isn't built.
- Not yet tried with StartAllBack off.

## What the suite does today

- **Taskbar** (`hideSystemTaskbar`, taskbar-host.cpp `HideSystemTaskbar`): switches Explorer's taskbar to auto-hide (so it reserves no screen space), hides `Shell_TrayWnd` and every `Shell_SecondaryTrayWnd` with `SW_HIDE`, and checks every 500 ms that they stay hidden. Everything is put back when the part stops, or after a crash (the state is kept in the registry).
- **StartAllBack off**: the suite can set StartAllBack's own "disabled for this user" value and restart Explorer (`kSabOffValue`).
- **Start menu**: a low-level keyboard hook (`WH_KEYBOARD_LL`, start-menu.cpp `StartKeyHook`) takes the Windows key and opens ours. Every key press on the system passes through the bar's thread while it's on.

That works with StartAllBack running, because StartAllBack already owns the taskbar and Start. Without it, Explorer's own Windows 11 taskbar comes back:
- It shows itself for notifications, full-screen changes and the pointer at the edge. The 500 ms check can let it flash.
- Anything that opens Start other than the Windows key goes to Windows' Start: Ctrl+Esc, a shell call, or a touch gesture.

## Windhawk mods that do part of it

| Mod | What it does | How |
|---|---|---|
| [Taskbar auto-hide fine tuning](https://windhawk.net/mods/taskbar-auto-hide-keyboard-only) (m417z, 2.3) | A "Never" mode: the taskbar never shows, whatever asks for it | In explorer.exe. **Cloaks** the taskbar windows (`DwmSetWindowAttribute(DWMWA_CLOAK)`) instead of hiding them. Hooks `TrayUI::Unhide`, `CSecondaryTray::_Unhide`, and on Windows 11 `ViewCoordinator::ShouldTaskbarBeExpanded` / `UpdateIsExpanded` (Taskbar.View.dll or ExplorerExtensions.dll). Blocks Explorer's unhide timers (ids 2 and 3), and hooks `twinui.pcshell.dll!XamlLauncher::ShowStartView` to decide what the Windows key does. |
| [Block Start Menu and Hosts](https://windhawk.net/mods/block-windows-startmenu-and-hosts) (Exiled Eye, 1.4.1) | Windows' Start (and optionally Search, the clipboard, Action Center, widgets) never runs | Kills the host processes (`StartMenuExperienceHost.exe` and others), then hooks `kernelbase!CreateProcessInternalW` in explorer.exe and svchost.exe to refuse relaunching them. |

Neither does the whole job. The first hides the taskbar but leaves Windows' Start in place. The second removes Start but does nothing about the taskbar. No mod in the catalogue replaces both and hands them to another bar.

## The plan for the suite

The Explorer relay already runs inside explorer.exe, so it can do what m417z's mod does, only for our purpose:

1. **Taskbar: cloaked, not hidden.**
   - Cloak `Shell_TrayWnd` and `Shell_SecondaryTrayWnd` from inside Explorer (a window can only be cloaked by its own process). A cloaked window draws nothing and takes no input, so its sliver at the screen's edge can't catch the pointer or flash.
   - Keep auto-hide on, so no space is reserved.
   - Hook `TrayUI::Unhide` and `ViewCoordinator::ShouldTaskbarBeExpanded` so Explorer doesn't slide it in.
   - Cloak new secondary taskbars as screens come and go.
   - This replaces the 500 ms check (one less timer).
2. **Start: every opening, not just the Windows key.**
   - Hook `twinui.pcshell.dll!XamlLauncher::ShowStartView` in Explorer and send it to our Start instead. That catches the Windows key, Ctrl+Esc and any other caller.
   - The low-level keyboard hook can then go, which takes every key press off the bar's thread: a latency and CPU win.
   - Keep the keyboard hook only as a fallback if the symbol isn't found.
3. **Optional: stop Windows' Start host altogether.** Do it the way Block Start Menu does, to free its memory. It isn't running here right now (StartAllBack keeps it from starting). Its neighbours use about 60 MB private each: ShellExperienceHost 62 MB, TextInputHost 65 MB. So expect about the same for Start. Off by default: it's a stronger change, and Windows updates sometimes expect it.

## Risks to check when building it

- **Symbol names change with Windows builds.** Each hook needs a check on the current build (26200.9550 here), and a fallback when it's missing. Windhawk caches symbols after the first download, so the cost comes once per Explorer update, not per start.
- **The taskbar's code has moved before.** The Windows 11 taskbar's code lives in Taskbar.View.dll on newer builds and ExplorerExtensions.dll on older ones, so both names have to be tried.
- **Test with StartAllBack truly off** (its Disabled value set, Explorer restarted). With it on, Explorer's own taskbar code isn't what's running.
- **Cloaking must be undone** when the part stops or crashes, like the current auto-hide state is.
