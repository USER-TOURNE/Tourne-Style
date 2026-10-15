# Tourne Notes, then the Start menu's phone pane

When: 2026-10-11 17:10 UTC, code-web, branch claude/project-thread-0rkfwp at 20963b8.

Why:
- "Build and apply both the notes change and the tourne icons mod for me on my pc."
- In the phone section of the Start menu:
  - right-click a notification to clear it;
  - its icons in their own style;
  - a bug review ("we dont need phone link anymore").
- Asked during the work: "fix all 15"; reply to Discord from there; "i cant scroll within the messages"; emoji "swapped".

What changed:
- Tourne Notes 2.0, built into Tourne Desktop: 61a7bc0.
- Apply-Notes-and-Icons.ps1 written into %USERPROFILE%\Downloads. It merges the branch, pushes, swaps, and installs Tourne'Icons with windhawk-cli.
  - It found the PC branch 92 commits ahead of GitHub. Its run pushed them: 7ea3cb6, which brought the Start menu phone pane, phone-*.cpp, Mission Control and the menu frame to GitHub.
- Phone pane: right-click Clear / Clear all, themed icons (new designs mail, reddit, video, shop), and no "Open Phone Link" while Tourne holds the phone: 8a3a004.
- All 15 review fixes, a click opening the notification's app on the PC, and conversation scrolling: 9e503c9. The review list is /mnt/project-files/tourne-desktop/phone-pane-review-2026-10-11.md in the project.
- Emoji shown as plain symbols (`PlainSymbols`, phone-link.cpp): 20963b8.

State:
- Everything builds (MinGW, cloud).
- Notes was tested in Wine. PlainSymbols was tested in Wine.
- Nothing about the phone has run, because it needs the iPhone over Bluetooth.
- Not confirmed that Tourne's script finished the swap and the Tourne'Icons install.

Next:
- Tourne swaps 20963b8 (`.\desktop\Swap-TourneDesktop.ps1 -Commit 20963b8`) and reports what misbehaves.
- Still queued:
  - Tourne'Icons: Save lists and Apply a pack;
  - one icon per picker entry in Tourne's red/green pixel style.

Learned:
- Remote computer use can't type in terminals on Tourne's PC (click-only). Drop a .ps1 in Downloads and give one line to paste. Tourne once pasted the file's lines instead, so say "paste this line".
- Check whether the PC branch is ahead before planning a fast-forward swap.
- ANCS (iPhone notifications) can't reply. Only MAP texts can.
