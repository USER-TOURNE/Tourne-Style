# Phone notifications over ANCS; Phone Link off

When: 2026-10-08, about 15:00 to 15:40, Claude Code on the desktop PC, branch `claude/project-thread-0rkfwp`.

Why (Tourne): "first make Tourne read the iPhone's notifications itself so Phone Link can go with nothing lost", after asking to turn Phone Link off once Tourne's texts work.

What changed: dd9ddd6, a92f4bb, 4b4f630 (details in DEVLOG, last section). Swapped in at 15:32. Phone Link turned off: `HKCU\...\Mobility` `PhoneLinkEnabled` 1 to 0 (real hive, Invoke-Real), process ended. To bring it back: set it to 1 and open Phone Link.

Found on the way:
- The phone was near after all: its classic link (MAP) failed with 10060 until about 15:09, while LE (ANCS) answered at once.
- Phone Link's startup task was State=1 in the real hive (not 2 as the 03:00 note says); Windows launches Phone Link from Explorer when the phone comes near, because of PhoneLinkEnabled.
- My HKCU reads can be stale for keys a sandboxed session wrote: read the truth with Invoke-Real.
- Windows' GATT shares a characteristic's notify setting between programs: turning it off then on makes the iPhone resend what it has.
- wpndatabase doesn't keep SuppressPopup, so quiet ones are marked in the toast XML for System Icons.

Lost with Phone Link (told Tourne): calls through the PC (an incoming call still shows as a notification) and Send a file (still opens Phone Link, which then takes the phone's channels until it's closed).

Not verified: a new notification popping, a text in and a reply out, the pane on screen.
