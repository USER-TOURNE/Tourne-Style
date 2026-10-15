# Phone: call buttons, Send a file over LocalSend, Windows calling probed

When: 2026-10-08, about 15:40 to 16:10, Claude Code on the desktop PC, branch `claude/project-thread-0rkfwp`.

Why (Tourne): build what was lost with Phone Link (calls through the PC, Send a file); for calls, "test being able to use our pcs inputs/outputs as the microphone/headset device output for where the call routes". Asked what turning Developer Mode on would do.

What changed: 98c180f (call buttons), becc3b0 (LocalSend sending); details in DEVLOG, last section. Swapped in at 16:03.

Found on the way:
- Windows' calling for a Bluetooth phone is gated: access "denied by system" for an unpackaged program. Phone Link has `phoneLineTransportManagement` and `phoneCallSystem` (restricted). Developer Mode is off (AppModelUnlock has no values).
- WinHTTP's header can't be included beside WinINet's (the taskbar part has wininet.h): use WinINet.
- The TD prelude shares every module's includes, so winrt headers etc. are there for all modules; a probe needs its own.

Next, waiting on Tourne:
- Developer Mode on (Settings, System, For developers): then a "Tourne Calls" helper as a locally registered package with those capabilities. Steps: register the phone's transport, connect, move a call's audio to the PC, and bridge the hands-free endpoints to the default mic and output (WASAPI) if Windows doesn't. Tested in a real call with Tourne.
- LocalSend on the iPhone (App Store), open on the same Wi-Fi, for Send a file.
