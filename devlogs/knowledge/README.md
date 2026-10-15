# Shared knowledge: how every Claude session picks up where the last one left off

Tourne works on this project from five places, and none of them share memory with each other:

| Where | Accounts | Can read this repo | Can write this repo |
| --- | --- | --- | --- |
| Claude app on the phone (chats) | 2 | Through the GitHub connector, or files added to a Project | No: it writes a note for you to commit |
| Claude app on the phone (Claude Code on the web, `claude/*` branches) | 2 | Yes, the branch it starts from | Yes, its own `claude/*` branch |
| Claude desktop app (chats) | 2 | Through the GitHub connector, or a Project | No: same as phone chats |
| Claude desktop app (Code tab, local) | 2 | Yes, the working folder | Yes |
| Claude Code in the terminal on the desktop PC | whichever is logged in | Yes, `X:\Documents\Claude Workspace\tourne-suite-sab-clone` | Yes |

Claude's own memory (chat memory, Projects, Claude Code's `~/.claude/.../memory`) belongs to one account on one surface. **This folder is the one thing all of them can reach**, so it is the knowledge base. A local memory is a cache; what matters goes here too.

## The protocol

**At the start of a session:** read `CLAUDE.md` at the repo root, then the newest notes in `knowledge/sessions/` (at least the last three, newest first by file name). Check `git log -5` against them: commits newer than the last note were made by a session that left no note.

**At the end of a session** (or before a long pause): write one note in `knowledge/sessions/`, named

    YYYY-MM-DD-HHMM-<where>-<topic>.md      e.g. 2026-10-06-0330-code-pc-swap-script.md

`<where>` is one of `code-pc`, `code-desktop-app`, `code-web`, `chat-phone`, `chat-desktop`, plus `-a` or `-b` for which account when it matters. One file per session, never edits to someone else's note, so two sessions on two branches never conflict when merged.

A note says, in this order:

    # <topic>
    When: <date and time>, <where>, account <a/b>, branch <branch> at <commit>
    Why: what was asked, in Tourne's words where possible
    What changed: commits, files, settings, things installed or swapped (with paths)
    State: what works, what is checked only in tests, what is broken
    Next: the very next step, and anything waiting on Tourne
    Learned: gotchas and facts a later session needs (also put lasting ones in CLAUDE.md or HANDOFF.md)

Keep it short. Facts that stay true belong in `CLAUDE.md` (the rules and where things are), `HANDOFF.md` (build and test details) or `DEVLOG.md` (what each change did, for people); a session note is the diary that links them.

**Chats that can't commit** (phone or desktop app chats): at the end, ask the chat to "write a session note for the Tourne knowledge base", then paste it into the next Claude Code session (any surface), which commits it. Or add it on GitHub's site or mobile app straight into `knowledge/sessions/`.

## Making each surface find it

- **The claude.ai page** "Tourne'Suite: where we left off" (https://claude.ai/code/artifact/dbb63574-12d7-494f-b141-cba652c118c3) is the quick view: where things stand, the last session, next steps and the Project instructions below. It belongs to the account Claude Code on the PC uses; share it with the other account. Keep it in step with the newest session note.
- **Claude Code anywhere** (terminal, desktop app's Code tab, web/phone): `CLAUDE.md` at the repo root loads by itself and points here.
- **Chats, on each of the 4 app logins:** make a Project called Tourne, connect GitHub (`USER-TOURNE/tourne-suite-sab-clone`), and paste this into the Project's instructions:

      This project is Tourne'Suite (repo USER-TOURNE/tourne-suite-sab-clone,
      working branch claude/project-thread-0rkfwp).
      Before answering about it, read the page "Tourne'Suite: where we left off"
      https://claude.ai/code/artifact/dbb63574-12d7-494f-b141-cba652c118c3
      then CLAUDE.md and the newest files in knowledge/sessions/.
      When we finish or pause, update that page's "Where things stand",
      "Last session" and "Next steps", and write a session note in the format
      of knowledge/README.md for me to commit.
      No em dashes. Short replies: the outcome plus the next step.

- **Which branch:** new web sessions start from `main`. As of 2026-10-06 `main` is 46 commits behind the working branch `claude/project-thread-0rkfwp`, so a session started from `main` sees none of the shell-mode work or this folder. Either merge the working branch into `main` regularly (a pull request after each round), or start every web session on the working branch and say so in the prompt.
