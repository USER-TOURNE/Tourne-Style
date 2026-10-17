# Tourne'Suite: Discord server blueprint

A home for everything that grew out of the Tourne'Suite showcase thread: **Tourne'Table** (the audio visualizer), the **Tourne'Suite Windhawk mods** (the suite, Tourne'Icons, Drop Install, Mod Tray), **Tourne Desktop** (the shell, Studio and the Icons Kit), and the people sharing presets, icon packs and setups.

**Fastest way to build it:** `build_server.py` (next to this file) makes every role, category, channel, forum tag and permission below in about a minute. See [Building it](#building-it) at the bottom. `python build_server.py --dry-run` prints the whole plan without touching Discord.

---

## 1. Roles

Top to bottom, in the order they sit in **Server Settings, Roles**. Colours come from the Tourne palette (`#66EA97` green, `#64AA89` soft green, `#4D8671` deep green, `#FF6F77` red).

| Role | Colour | What it's for |
|---|---|---|
| 👑 **Tourne** | `#66EA97` | Administrator. Keep it to you (and one backup you trust). 2FA on. |
| 🛡️ **Moderator** | `#FF6F77` | Manage Messages, Timeout, Kick, Manage Threads, Audit Log, voice mute/move. No Administrator, Manage Roles or Manage Server. |
| 🤖 **Bots** | `#64AA89` | Only what each bot needs. Sits above the roles a bot hands out. |
| 💻 **Developer** | `#4D8671` | Posts in the release channels, pins, manages threads, can ping for releases. |
| 🧪 **Tester** | `#3C865C` | Sees `🧪｜tester-kit` (test builds before release). A thank-you for good bug reports, like .exe's. |
| 💚 **Supporter** | `#FF8F8F` | Ko-fi supporters (https://ko-fi.com/tourne). Shown separately. Hand out by hand or with Ko-fi's Discord link. |
| 🎚️ **Tourne'Table** | `#66EA97` | Picked in Onboarding. Unlocks the Tourne'Table channels, gets its release pings. |
| 🦅 **Tourne'Suite mods** | `#64AA89` | Picked in Onboarding. Unlocks the mod channels (suite, Drop Install, Mod Tray). |
| 🖥️ **Tourne Desktop** | `#4D8671` | Picked in Onboarding. Unlocks the Tourne Desktop channels. |
| 🎨 **Tourne'Icons** | `#FF6F77` | Picked in Onboarding. Unlocks the mod channels and `🎨｜icon-packs`. |
| ✅ **Member** | `#BDC3C7` | Given when someone accepts the rules. Can talk, react, make threads, join voice. |
| `@everyone` | | Sees START HERE only, can't post. |

The four project roles are interests, not ranks: people can hold all of them, and each one is a release-ping opt-in.

---

## 2. Categories and channels

### 📌 START HERE (everyone sees it)
| Channel | Purpose | Who posts |
|---|---|---|
| 👋｜welcome | The welcome message (section 3). | Tourne |
| 📜｜rules | Rules (section 3), paired with Rules Screening. | Tourne |
| 📢｜announcements | New builds, new videos, server news. Announcement channel. | Developer |
| 🎭｜pick-your-roles | What each project role unlocks (Onboarding does the picking). | Tourne |
| ❓｜faq | Section 4, one message per question so each can be linked. | Moderator |
| 🔗｜links | GitHub (USER-TOURNE/Tourne-Suite), the Tourne'Table walkthroughs on YouTube (13 and 20 minutes), Ko-fi. | Tourne |

### 🎚️ TOURNE'TABLE (🎚️ Tourne'Table, staff)
| Channel | Purpose | Who posts |
|---|---|---|
| 🚀｜table-releases | Each build: the `.wh.cpp`, version, what changed, SHA-256. | Developer |
| 🎛️｜table-presets | **Forum.** One post per look: a `TT_Settings.yaml` or a Quick settings paste (right-click the bar, Copy quick settings), a screenshot, and the font used. Tags: `Preset (YAML)`, `Quick settings`, `Font`, `Low CPU`, `Colors`, `Shapes`. | Member+ |
| 🆘｜table-help | **Forum.** One post per problem. Ask for Tourne'Table version, Windhawk version, audio device, GPU. Tags: `Solved`, `Needs info`, `Bug`, `Windows 10`, `Windows 11`, `Insider`. | Member+ |

### 🦅 TOURNE'SUITE MODS (🦅 Tourne'Suite mods, 🎨 Tourne'Icons, staff)
| Channel | Purpose | Who posts |
|---|---|---|
| 🚀｜mod-releases | Tourne'Suite builds, Tourne'Icons (and Lasting), Tourne Drop Install, Tourne Mod Tray. | Developer |
| 🎨｜icon-packs | **Forum.** Tourne'Icons packs: a zip with `pack.tsv` and its pictures, plus a preview. Tags: `Full pack`, `Add-on`, `Pixel`, `Ink`, `Store apps`, `File types`. | Member+ |
| 🆘｜mod-help | **Forum.** Mod, version, Windows build (`winver`), Windhawk version, and the mod's log from Windhawk. Same tags as table-help. | Member+ |

### 🖥️ TOURNE DESKTOP (🖥️ Tourne Desktop, staff)
| Channel | Purpose | Who posts |
|---|---|---|
| 🚀｜desktop-releases | Tourne Desktop builds by commit hash, Studio changes, Icons Kit. | Developer |
| 🆘｜desktop-help | **Forum.** Attach `shell.log` and `crash.txt` from `%LOCALAPPDATA%\Tourne\Desktop`, and `TourneDesktop.commit.txt`. Tags add `Shell mode`, `Studio`, `Crash`. | Member+ |

### 💻 DEVELOPMENT (members)
| Channel | Purpose | Who posts |
|---|---|---|
| 💬｜dev-chat | How it's built: C++, Windows internals, Windhawk's API, Direct2D. | Member+ |
| 🐞｜bug-reports | **Forum.** Template in section 5. Tags: `Confirmed`, `Fixed`, `Can't reproduce`, `Duplicate`, plus one per project. | Member+ |
| 💡｜ideas | **Forum.** Requests for any project. Tags: `Planned`, `In progress`, `Released`, `Declined`. | Member+ |
| 🔔｜github-feed | GitHub webhook: pushes, releases, issues. | Webhook |
| 🧪｜tester-kit | Test builds (the Tester Kit) and what to try. Only 🧪 Tester and staff see it. | Tester, Developer |

### 🎉 COMMUNITY (members)
| Channel | Purpose | Who posts |
|---|---|---|
| 💬｜general | Anything, within the rules. | Member+ |
| 🖼️｜showcase | **Forum.** Your setup, one post each, tagged with what's in it (`Tourne'Table`, `Tourne'Suite`, `Tourne Desktop`, `Tourne'Icons`, `Full rice`, `Video`). | Member+ |
| 📦｜big-files | Long videos and big builds, sent in pieces with **Tourne Big Files** (section 6). | Member+ |
| 🔗｜resources | Fonts (static ones, see the FAQ), wallpapers, icon sets, tools. Slowmode 30 s. | Member+ |
| 🤡｜off-topic | Memes and gifs. | Member+ |
| 🔊｜Hangout | Voice. | Member+ |
| 🎙️｜Dev Stream | Stage: live building and Q&A. | Developer speaks |

### 🔒 STAFF (Moderator, Developer)
`🛡️｜staff-chat`, `📁｜mod-logs` (bots only), `🔊｜Staff Voice`.

---

## 3. Welcome and rules

Paste into **👋｜welcome** (or a welcome bot; swap `{user}` for its mention variable):

```
# Welcome to Tourne'Suite, {user}!

Linux-level customizing for Windows, with nothing phoning home: an audio
visualizer that lives behind your desktop icons, Windhawk mods that restyle the
taskbar, tray and every icon, and a whole desktop shell.

## Get started
**1.** Read and accept the rules in #📜｜rules
**2.** Pick what you use in #🎭｜pick-your-roles to open its channels
**3.** Check #❓｜faq first, most install questions are answered there

## Where things are
🎚️ **Tourne'Table** → #🚀｜table-releases · #🎛️｜table-presets · #🆘｜table-help
🦅 **Tourne'Suite mods** → #🚀｜mod-releases · #🎨｜icon-packs · #🆘｜mod-help
🖥️ **Tourne Desktop** → #🚀｜desktop-releases · #🆘｜desktop-help
🐞 **Found a bug?** → #🐞｜bug-reports
🖼️ **Show your setup** → #🖼️｜showcase

## Good to know
• Only use builds posted in the release channels or on GitHub (USER-TOURNE).
• Staff never DM you files and never ask for passwords.
• Support the work: https://ko-fi.com/tourne
```

Paste into **📜｜rules**:

```
# Rules

**1. Be kind.** No harassment, hate or personal attacks.
**2. Right channel.** Help goes in the help forums, one post per problem.
**3. No piracy or malware.** No cracks, keygens, cheats or anything harmful.
**4. Official builds only.** Don't repost changed builds as if they were official.
   Your own presets, icon packs and forks are welcome, labelled as yours.
**5. Credit artists.** Icon sets and themes keep their author's license
   (for example niivu's sets are links only, no reuploads).
**6. No unasked DMs or ads.**
**7. Mods are used at your own risk.** Make a restore point before system mods.
**8. Follow Discord's Terms of Service.**

Breaking these can mean a warning, timeout, kick or ban.
```

---

## 4. FAQ (one message each in ❓｜faq)

- **How do I install a `.wh.cpp`?** Windhawk, Create a new mod, paste it, Compile mod. With Windhawk 2 you can also drop the file on **Tourne Drop Install**.
- **Compile failed.** Say which Windhawk version and paste the first error line in #🆘｜mod-help. Windhawk also builds every mod for 32-bit programs, so an error can name i686 even on a 64-bit PC.
- **My font doesn't show (Tourne'Table text).** Windhawk needs a static font, not a variable one. Freeze it first with FontFreeze or FontGauntlet (for example Handjet SemiBold).
- **Tourne'Table uses CPU.** Most of it is your CPU boosting. In Task Manager you can right-click it and turn on Efficiency mode or lower its priority; it keeps working. Small spikes when the song changes are the per-song recalculation.
- **Quick settings vs Windhawk settings.** Right-clicking the bar changes quick settings for that session; they don't change the mod's settings in Windhawk. Copy quick settings gives you text to paste in #🎛️｜table-presets.
- **I turned Tourne'Icons off. What stays?** Programs, Windows icons, tray icons and Store apps go back as each program restarts. File types stay until you click **Put back original**. Your list and pictures are kept, so turning it back on brings everything back. Tourne'Icons Lasting keeps changes with the mod off.
- **Antivirus flagged something.** Mods hook into programs, which looks suspicious to scanners. Every release post has a SHA-256; check it with `Get-FileHash .\file` in PowerShell and compare. Source is on GitHub. Never turn your antivirus off for a build from anywhere else.
- **Tourne Desktop shell mode went wrong.** Hold Shift while signing in to skip it, or see #🆘｜desktop-help. Don't close the shell-mode copy from Task Manager twice in a row: two crashes within two minutes turn shell mode off on purpose.

---

## 5. Post templates (pin one in each forum)

**Release post** (release channels):
```
**Tourne'Icons 2.5.3**  ·  2026-10-12
What changed: ...
File: tourne-icons.wh.cpp  ·  SHA-256: ...
Source: https://github.com/USER-TOURNE/...
```

**Bug report** (🐞｜bug-reports):
```
Project and version (or Tourne Desktop commit):
Windows build (winver):
Windhawk version:
What I did:
What I expected:
What happened:
Logs: (Windhawk mod log / shell.log / crash.txt)
```

**Preset** (🎛️｜table-presets): screenshot or clip, the YAML or Quick settings paste, the font, your GPU (the drawing GPU and Smooth mode change the cost).

---

## 6. Big files in the server (Tourne Big Files)

The Tourne'Table walkthroughs and full builds are bigger than Discord's 10 MB limit without Nitro. **Tourne Big Files** (a Vencord plugin, in `tools/vencord/tourneBigFiles.desktop` in the repo) right-click the upload (+) button, pick **Send a big file (in pieces)**:

- The file is cut into pieces under the limit and sent as silent messages, in a thread so the channel stays clean.
- The channel gets one card. Anyone with the plugin can **Play here** (videos, music, pictures) or **Save** the whole file, checked piece by piece.
- Anyone without it sees the pieces (`name.001`, `name.002`, ...) and a line saying how to join them with 7-Zip (**Combine files**).

`📦｜big-files` is the place for these. Nothing leaves Discord unless the sender turns on "Send as a link" (Catbox, Litterbox or their own ShareX uploader).

---

## 7. Safety setup

1. **Fake "update" links.** Only Developer posts in release channels. AutoMod: block `.exe`, `.scr`, `.bat`, `.msi` links from non-staff, and a keyword list for `free nitro`, `discord.gift` and Steam look-alikes. Turn on **Block mention spam** and **Block suspected spam**.
2. **Verification.** Safety Setup: Verification **Medium**, **Require 2FA for moderator actions**, **Rules Screening** (accepting gives ✅ Member), explicit image filter for all members.
3. **Onboarding.** Question "What do you use?" with Tourne'Table, Tourne'Suite mods, Tourne Desktop, Tourne'Icons, each giving its role. Default channels: welcome, faq, general, showcase.
4. **Few bots, tight permissions.** Built-in AutoMod and Onboarding cover most of it. Add Carl-bot or Sapphire only for logging to `📁｜mod-logs`. Never give a bot Administrator after setup.
5. **GitHub feed without a bot.** In the repo: Settings, Webhooks, paste the channel's webhook URL with `/github` on the end, pick pushes, releases and issues.

---

## Building it

Discord can't import a template file; templates (`discord.new/...`) are made from a server that already exists. So: build once, then save it as a template.

**Why a bot and not a Vencord plugin:** the builder makes about 40 roles and channels in a minute. Done from your own account (a client plugin), that's automating a user account, which Discord's rules forbid and which can get the account flagged. A bot is the supported way, and you remove it afterwards.

1. **Make an empty server:** + , Create My Own, name it **Tourne'Suite**.
2. **Make a bot:** https://discord.com/developers/applications, New Application, **Bot**, **Reset Token**, copy it. Keep the token private.
3. **Invite it:** OAuth2, URL Generator, tick `bot`, then **Administrator** (only for setup). Open the link, pick the new server.
4. **Server ID:** User Settings, Advanced, **Developer Mode** on. Right-click the server icon, **Copy Server ID**.
5. **Turn on Community first** (Server Settings, Enable Community) so forums and announcement channels are made as such.
6. **Run it** (Python 3.9+):
   ```
   pip install discord.py
   python build_server.py --dry-run
   python build_server.py --token YOUR_BOT_TOKEN --guild YOUR_SERVER_ID
   ```
7. **Kick the bot** (or take away its Administrator).
8. Paste section 3 and 4, pin the section 5 templates, finish Onboarding and AutoMod (section 7).
9. **Server Settings, Server Template, Generate Template** for a one-click copy.

Then the showcase thread can point here: pin a message there with the invite.
