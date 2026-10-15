# Tourne Calls: a packaged helper for the phone's calls

When: 2026-10-08, about 16:10 to 16:25, Claude Code on the desktop PC, branch `claude/project-thread-0rkfwp`.

Why (Tourne): calls through the PC, with the call on the PC's own mic and output. Tourne turned Developer Mode on.

What changed: `desktop/calls/` (TourneCalls.cpp, AppxManifest.xml, TourneCalls.exe.manifest, Register-TourneCalls.ps1). Registered as a development package `Tourne.Calls_1.0.0.0_x64__7mma6cae4r2dw` with external location `%LOCALAPPDATA%\Tourne\Desktop\calls` (run through Invoke-Real). Its log: `%LOCALAPPDATA%\Tourne\Desktop\calls.log`.

Found:
- With the package identity, access to the phone's transport is Allowed (it was "denied by system" unpackaged).
- RegisterApp did nothing until the manifest also had `uap:Capability phoneCall` and `uap13:Extension windows.phoneCallActivation` (as Phone Link's has). Then registered 1, connected 1. Windows brought up the hands-free Speakers/Microphone endpoints (present, OK); audio routing to the PC possible; in-band ringing on.
- PhoneCallManager.RequestStoreAsync times out (0x800705B4): the Phone Service (PhoneSvc) is Disabled on this PC. A service's startup type is Tourne's to change; asked.
- To undo: `TourneCalls.exe unregister`, `Register-TourneCalls.ps1 -Remove`.

Next: Phone Service on (Manual), then `status` (the line, its calls); then a run mode that follows calls, answers and ends them for Tourne Desktop's buttons, moves audio to the PC, and bridges the hands-free endpoints to the default mic and output if Windows doesn't; tested in a real call with Tourne.
