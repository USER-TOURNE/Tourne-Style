# 2026-10-12 07:00, Claude Code (web), Discord: Tourne Big Files and the server plan

**Done**
- Tourne Big Files (`tools/vencord/tourneBigFiles.desktop`): Vencord/Testcord plugin, sends files over 10 MB in pieces (thread in servers, chat in DMs) with a `.tbf.json` card that plays or saves the whole file; optional link hosts (Catbox, Litterbox, own .sxcu), off by default. Builds clean in Vencord and Testcord; native side tested on Node. Not yet run inside Discord.
- `Install-TourneBigFiles.ps1`: clones and builds Vencord or Testcord (`-Client Testcord`) with the plugin, then runs their installer.
- `community/discord/`: server blueprint and bot builder retargeted to Tourne'Table, the Windhawk mods and Tourne Desktop, FAQ from Tourne's showcase thread; `build_server.py --dry-run` prints the plan.
- Copies in /mnt/project-files/discord/.

**Waiting on Tourne**
- Run the install line, turn on TourneBigFiles, try a big video in a DM.
- Make a bot (Developer Portal), invite it, run build_server.py on a new server.
- From before: ink add-on + Tourne'Icons 2.5.3, Windhawk window check, swap Tourne Desktop to 0e68534.

**Notes**
- Building here needed gifenc cloned locally (codeload.github.com is blocked from the cloud); on the PC the normal `pnpm install` works.
- A user account must not be automated (Discord rules), so the server is built by a bot, never by a client plugin.
