# Phone texts over Bluetooth MAP

When: 2026-10-08, about 11:00 to 15:00, Claude Code on the desktop PC, branch `claude/project-thread-0rkfwp`.

Why (Tourne): "start the messaging part next: Tourne connects to the phone, lists messages as they arrive, and sends replies from the phone pane, wherever it exists/is attached or detached"; "when the functionality is there turn off phone link for me"; pop-ups on arrival are working now (Tourne says).

What changed: 7cd95ca (details in DEVLOG, last section). Swapped in at 14:56.

Found on the way:
- The phone's classic node is `BTHENUM\DEV_68EFDCAC8DDA`, FriendlyName "User \ Hello World" (same as Cross Device's DisplayName); phone-map finds the address that way.
- Phone Link (PhoneExperienceHost.exe) wasn't running this afternoon; its startup task is still on (State=2), so it starts at the next sign-in and takes the channel again.
- The phone was not connected all afternoon (Bluetooth connected = false, last 09:52), so no live test.

Open, for Tourne:
- Turning Phone Link off also ends what it brings over Bluetooth besides texts: the iPhone's notifications (ANCS: the phone pane's Notifications and the centre's phone section read Phone Link's toasts) and calls. Asked before turning it off; Tourne reading notifications itself (ANCS over BLE GATT) would let Phone Link go entirely.
- Live test once the phone is near and Phone Link is closed: `map-session-probe` (metadata only), then a text in and a reply out from the pane.
