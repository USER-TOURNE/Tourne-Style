# ❀ CHANGELOG

*What changed in each `Tourne'Suite` build I've published here. Every build keeps its own id (`tourne-suite-bNN`), so it installs beside older ones: run one at a time.*

---

## ❀ b65: Square Corners, and Task Manager in my colours

Everything since b55. Off unless you turn it on.

### ✦ Square Corners (new part)
- Every window gets 0 corner radius, asked for by each program on its own windows, so nothing outside the program is touched.
- Menus too: each one is caught as it opens.
- An exclude list for programs that shouldn't be touched (anti-cheat games and the like).
- The switch is in Tourne Desktop: Studio > Taskbar > Square window corners, with the part on in the suite. Without Tourne Desktop, set `HKCU\Software\Tourne\Corners`, `Square` (DWORD) to 1.

### ✦ Task Manager in my colours (Explorer Colours)
- Task Manager's usage cells follow the drive bar's colours, and its rows take the selection colours.
- The Details and Services lists take the Task Manager colours too (background, text, dark headers), caught as each list is made so they keep them.
- Group heading rows (grouped by type) lose their cream usage cells, wherever Task Manager draws them from.

### ✦ Fixes
- The suite builds again with the newer compiler.
- The taskbar's drag settings only check Tourne Desktop's own when built there.
