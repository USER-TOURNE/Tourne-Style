"""
Tourne'Style Discord server builder.

Creates the roles, categories, channels, forum tags and permissions from
tourne-server-blueprint.md inside a server you own (best an empty one).

Usage:
    pip install discord.py
    python build_server.py --dry-run                      # print the plan, no token needed
    python build_server.py --token BOT_TOKEN --guild SERVER_ID
    python build_server.py --guild SERVER_ID       # token from DISCORD_BOT_TOKEN

Safe to re-run: anything that already exists (same name) is skipped.
The bot needs Administrator while it runs; remove the bot afterwards.
"""
import argparse
import os

# ---------------------------------------------------------------- roles
# Listed top to bottom. Permissions are named by discord.Permissions flags.
MEMBER_PERMS = dict(
    view_channel=True, read_message_history=True, send_messages=True,
    send_messages_in_threads=True, create_public_threads=True,
    embed_links=True, attach_files=True, add_reactions=True,
    use_external_emojis=True, connect=True, speak=True, stream=True,
    use_voice_activation=True, change_nickname=True,
)
MOD_PERMS = dict(
    manage_messages=True, moderate_members=True, kick_members=True,
    manage_threads=True, view_audit_log=True, mute_members=True,
    move_members=True, deafen_members=True,
)
DEV_PERMS = dict(manage_threads=True, mention_everyone=True, embed_links=True, attach_files=True)

ROLES = [
    # name, colour, permissions, hoist (shown separately)
    ("👑 Tourne", 0x66EA97, dict(administrator=True), True),
    ("🛡️ Moderator", 0xFF6F77, MOD_PERMS, True),
    ("🤖 Bots", 0x64AA89, {}, False),
    ("💻 Developer", 0x4D8671, DEV_PERMS, True),
    ("🧪 Tester", 0x3C865C, {}, True),
    ("💚 Supporter", 0xFF8F8F, {}, True),
    ("🎚️ Tourne'Table", 0x66EA97, {}, False),
    ("🦅 Tourne'Suite mods", 0x64AA89, {}, False),
    ("🖥️ Tourne Desktop", 0x4D8671, {}, False),
    ("🎨 Tourne'Icons", 0xFF6F77, {}, False),
    ("✅ Member", 0xBDC3C7, MEMBER_PERMS, False),
]

# Shorthand keys used in the layout below.
ALIAS = {
    "owner": "👑 Tourne", "mod": "🛡️ Moderator", "dev": "💻 Developer",
    "tester": "🧪 Tester", "supporter": "💚 Supporter",
    "table": "🎚️ Tourne'Table", "suite": "🦅 Tourne'Suite mods",
    "desktop": "🖥️ Tourne Desktop", "icons": "🎨 Tourne'Icons", "member": "✅ Member",
}
# Interest roles people pick in Onboarding; they get release pings.
PINGABLE = {"table", "suite", "desktop", "icons", "tester"}

SUPPORT_TAGS = ["Solved", "Needs info", "Bug", "Windows 10", "Windows 11", "Insider"]
IDEA_TAGS = ["Planned", "In progress", "Released", "Declined"]
BUG_TAGS = ["Confirmed", "Fixed", "Can't reproduce", "Duplicate",
            "Tourne'Table", "Tourne'Suite", "Tourne Desktop", "Tourne'Icons"]
STYLE_TAGS = ["Style code", ".ttstyle file", "Font", "Low CPU", "Colors", "Shapes"]
PLUGIN_TAGS = ["Plugin", "Widget", "Panel", "Needs a tool", "Request"]
PACK_TAGS = ["Full pack", "Add-on", "Pixel", "Ink", "Store apps", "File types"]
SHOWCASE_TAGS = ["Tourne'Table", "Tourne'Suite", "Tourne Desktop", "Tourne'Icons", "Full rice", "Video"]

# Old channel names (from the Tourne'Suite layout) and what they are now.
RENAMES = {"🎛️｜table-presets": "🎛️｜table-styles"}

# ---------------------------------------------------------------- layout
# Category "see": who can view it ("everyone" or role keys).
# Channel "post": who can send ("all" = whoever can see it, "none" = admins
# only, or role keys). kind: text | news | forum | voice | stage.
# extra: "tags" (forums), "slowmode" (seconds), "see" (narrower than the category).
LAYOUT = [
    {
        "name": "📌 START HERE", "see": "everyone",
        "channels": [
            ("text", "👋｜welcome", "What Tourne'Style is, and where to go.", "none", {}),
            ("text", "📜｜rules", "Accept the rules to get in.", "none", {}),
            ("news", "📢｜announcements", "New builds, videos and server news.", ["dev"], {}),
            ("text", "🎭｜pick-your-roles", "Pick what you use to unlock its channels and pings.", "none", {}),
            ("text", "❓｜faq", "Installing, Windhawk, fonts, CPU use, antivirus, putting things back.", ["mod"], {}),
            ("text", "🗺️｜roadmap", "Where Tourne'Style is going: next up and further out.", "none", {}),
            ("news", "📖｜devlogs", "Each new devlog chapter: how it was built and what hurt.", ["dev"], {}),
            ("text", "🔗｜links", "The Tourne'Style repo, the styles gallery, YouTube walkthroughs, Ko-fi.", "none", {}),
        ],
    },
    {
        "name": "🎚️ TOURNE'TABLE", "see": ["table", "mod", "dev"],
        "channels": [
            ("news", "🚀｜table-releases", "Tourne'Table builds (.wh.cpp) and what changed.", ["dev"], {}),
            ("forum", "🎛️｜table-styles", "Share a style: its TT2 code (Style Editor, Copy code) or its .ttstyle file, a screenshot and the font.", "all",
             {"tags": STYLE_TAGS}),
            ("forum", "🆘｜table-help", "One post per problem: Tourne'Table version, Windhawk version, audio device.", "all",
             {"tags": SUPPORT_TAGS}),
        ],
    },
    {
        "name": "🦅 TOURNE'SUITE MODS", "see": ["suite", "icons", "mod", "dev"],
        "channels": [
            ("news", "🚀｜mod-releases", "Tourne'Suite builds, Tourne'Icons, Drop Install, Mod Tray.", ["dev"], {}),
            ("forum", "🎨｜icon-packs", "Share Tourne'Icons packs (zip with pack.tsv) and single icons.", "all",
             {"tags": PACK_TAGS, "see": ["icons", "suite", "mod", "dev"]}),
            ("forum", "🆘｜mod-help", "One post per problem: mod, version, Windows build, Windhawk version, log.", "all",
             {"tags": SUPPORT_TAGS}),
        ],
    },
    {
        "name": "🖥️ TOURNE DESKTOP", "see": ["desktop", "mod", "dev"],
        "channels": [
            ("news", "🚀｜desktop-releases", "Tourne Desktop builds (commit hash), Studio and Kit.", ["dev"], {}),
            ("forum", "🧩｜plugins", "Luau plugins for Tourne Desktop: share one, ask for one, or get help with one.", "all",
             {"tags": PLUGIN_TAGS}),
            ("forum", "🆘｜desktop-help", "Shell mode, Studio, tray, Start, phone. Attach shell.log or crash.txt.", "all",
             {"tags": SUPPORT_TAGS + ["Shell mode", "Studio", "Crash", "Phone"]}),
        ],
    },
    {
        "name": "💻 DEVELOPMENT", "see": ["member"],
        "channels": [
            ("text", "💬｜dev-chat", "How it's built: C++, Windows internals, Windhawk's API, Direct2D.", "all", {}),
            ("forum", "🐞｜bug-reports", "What, which build, Windows build, steps, expected vs what happened, logs.", "all",
             {"tags": BUG_TAGS}),
            ("forum", "💡｜ideas", "Requests and ideas for any of the projects.", "all", {"tags": IDEA_TAGS}),
            ("text", "🔔｜github-feed", "Commits, releases and issues from GitHub.", "none", {}),
            ("text", "🧪｜tester-kit", "Test builds before release, and what to try.", ["tester", "dev"],
             {"see": ["tester", "mod", "dev"]}),
        ],
    },
    {
        "name": "🎉 COMMUNITY", "see": ["member"],
        "channels": [
            ("text", "💬｜general", "Anything, within the rules.", "all", {}),
            ("forum", "🖼️｜showcase", "Your setup: screenshots and videos. Tag what's in it.", "all",
             {"tags": SHOWCASE_TAGS}),
            ("text", "📦｜big-files", "Long videos and big builds, sent in pieces with Tourne Big Files (Vencord).", "all", {}),
            ("text", "🔗｜resources", "Fonts, wallpapers, icon sets, tools, guides.", "all", {"slowmode": 30}),
            ("text", "🤡｜off-topic", "Memes and everything else.", "all", {}),
            ("voice", "🔊｜Hangout", None, "all", {}),
            ("stage", "🎙️｜Dev Stream", "Live building and Q&A.", "all", {}),
        ],
    },
    {
        "name": "🔒 STAFF", "see": ["mod", "dev"],
        "channels": [
            ("text", "🛡️｜staff-chat", "Moderation talk.", "all", {}),
            ("text", "📁｜mod-logs", "Bot and AutoMod logs.", "none", {}),
            ("voice", "🔊｜Staff Voice", None, "all", {}),
        ],
    },
]


def check_layout():
    """Catches typos before anything touches Discord."""
    names = {r[0] for r in ROLES}
    assert set(ALIAS.values()) <= names, "ALIAS names a role that isn't in ROLES"
    for cat in LAYOUT:
        keys = [] if cat["see"] == "everyone" else list(cat["see"])
        for kind, name, topic, post, extra in cat["channels"]:
            assert kind in ("text", "news", "forum", "voice", "stage"), name
            keys += [] if post in ("all", "none") else list(post)
            keys += list(extra.get("see", []))
            assert len(extra.get("tags", [])) <= 20, f"{name}: Discord allows 20 forum tags"
        for key in keys:
            assert key in ALIAS, f"{cat['name']}: unknown role key {key}"


def dry_run():
    check_layout()
    print("Roles, top to bottom:")
    for name, colour, perms, hoist in ROLES:
        print(f"  {name}  #{colour:06X}" + ("  (shown separately)" if hoist else ""))
    for cat in LAYOUT:
        see = "everyone" if cat["see"] == "everyone" else ", ".join(ALIAS[k] for k in cat["see"])
        print(f"\n{cat['name']}  (sees it: {see})")
        for kind, name, topic, post, extra in cat["channels"]:
            who = {"all": "", "none": "  [admins post]"}.get(post if isinstance(post, str) else "", "")
            if not isinstance(post, str):
                who = "  [posts: " + ", ".join(ALIAS[k] for k in post) + "]"
            tags = f"  tags: {', '.join(extra['tags'])}" if extra.get("tags") else ""
            print(f"  {kind:6} {name}{who}{tags}")


async def build(guild, server_name=""):
    import discord

    def perms(d):
        return discord.Permissions(**d) if d else discord.Permissions.none()

    def view_overwrites(roles, see):
        ow = {}
        if see == "everyone":
            ow[guild.default_role] = discord.PermissionOverwrite(view_channel=True)
        else:
            ow[guild.default_role] = discord.PermissionOverwrite(view_channel=False)
            for key in see:
                ow[roles[ALIAS[key]]] = discord.PermissionOverwrite(view_channel=True)
        return ow

    def post_overwrites(roles, base, post, kind):
        ow = {k: discord.PermissionOverwrite(**dict(v)) for k, v in base.items()}
        if post == "all":
            return ow
        deny = dict(send_messages=False, create_public_threads=False) if kind == "forum" else dict(send_messages=False)
        ow.setdefault(roles[ALIAS["member"]], discord.PermissionOverwrite())
        for target in [guild.default_role, *ow]:
            ow.setdefault(target, discord.PermissionOverwrite()).update(**deny)
        if post != "none":
            for key in post:
                ow.setdefault(roles[ALIAS[key]], discord.PermissionOverwrite()).update(
                    view_channel=True, send_messages=True, create_public_threads=True)
        return ow

    print(f"Building in: {guild.name}")
    community = "COMMUNITY" in guild.features

    # @everyone can look at START HERE but can't talk until they get ✅ Member.
    await guild.default_role.edit(permissions=discord.Permissions(
        view_channel=True, read_message_history=True, add_reactions=True, change_nickname=True))

    roles = {r.name: r for r in guild.roles}
    for name, colour, p, hoist in reversed(ROLES):
        if name in roles:
            print(f"  = role exists: {name}")
            continue
        roles[name] = await guild.create_role(
            name=name, colour=discord.Colour(colour), permissions=perms(p), hoist=hoist,
            mentionable=name in {ALIAS[k] for k in PINGABLE})
        print(f"  + role: {name}")

    # Channels from the older Tourne'Suite layout get their new names instead of a duplicate.
    for old, new in RENAMES.items():
        ch = discord.utils.get(guild.channels, name=old)
        if ch is not None and discord.utils.get(guild.channels, name=new) is None:
            await ch.edit(name=new)
            print(f"  ~ renamed: {old} -> {new}")
    existing = {c.name: c for c in guild.channels}
    rules_ch = updates_ch = None
    for cat_def in LAYOUT:
        cat_view = view_overwrites(roles, cat_def["see"])
        cat = existing.get(cat_def["name"])
        if cat is None:
            cat = await guild.create_category(cat_def["name"], overwrites=cat_view)
            print(f"+ category: {cat_def['name']}")

        for kind, name, topic, post, extra in cat_def["channels"]:
            lookup = name if kind in ("voice", "stage") else name.lower()
            ch = existing.get(lookup)
            if ch is not None:
                print(f"  = exists: {name}")
            else:
                base = view_overwrites(roles, extra["see"]) if "see" in extra else cat_view
                ow = post_overwrites(roles, base, post, kind)
                if kind in ("news", "forum", "stage") and not community:
                    print(f"  ! {name}: needs Community turned on, made as a "
                          f"{'voice' if kind == 'stage' else 'text'} channel")
                    kind = "voice" if kind == "stage" else "text"
                if kind == "forum":
                    ch = await guild.create_forum(
                        name, category=cat, topic=topic, overwrites=ow,
                        available_tags=[discord.ForumTag(name=t) for t in extra["tags"]])
                elif kind == "voice":
                    ch = await guild.create_voice_channel(name, category=cat, overwrites=ow)
                elif kind == "stage":
                    ch = await guild.create_stage_channel(name, category=cat, overwrites=ow)
                else:
                    ch = await guild.create_text_channel(
                        name, category=cat, topic=topic, overwrites=ow,
                        news=(kind == "news"), slowmode_delay=extra.get("slowmode", 0))
                print(f"  + {kind}: {name}")
            if name == "📜｜rules":
                rules_ch = ch
            if name == "📁｜mod-logs":
                updates_ch = ch

    try:
        await guild.edit(
            name=server_name or guild.name,
            verification_level=discord.VerificationLevel.medium,
            explicit_content_filter=discord.ContentFilter.all_members,
            default_notifications=discord.NotificationLevel.only_mentions,
            rules_channel=rules_ch if community else discord.utils.MISSING,
            public_updates_channel=updates_ch if community else discord.utils.MISSING,
        )
        print("Safety settings on (verification Medium, image filter on, mentions-only notifications).")
    except discord.HTTPException as e:
        print(f"Couldn't apply the safety settings: {e}")

    print("\nDone.")
    if not community:
        print("Tip: turn on Community (Server Settings, Enable Community), delete the plain-text\n"
              "versions of the forum and announcement channels, then run this again.")
    print("Next: paste the welcome, rules and FAQ, set up Onboarding and AutoMod,\n"
          "then Server Settings, Server Template, Generate Template.")


def main():
    p = argparse.ArgumentParser(description="Build the Tourne'Style Discord server.")
    p.add_argument("--token", default=os.environ.get("DISCORD_BOT_TOKEN"), help="Bot token (default: DISCORD_BOT_TOKEN)")
    p.add_argument("--guild", type=int, help="Server ID")
    p.add_argument("--name", default="Tourne'Style", help="Server name to set (empty keeps the current one)")
    p.add_argument("--dry-run", action="store_true", help="Print what would be made, touch nothing")
    args = p.parse_args()

    if args.dry_run:
        dry_run()
        return
    if not args.token or not args.guild:
        p.error("--guild and a token (--token or DISCORD_BOT_TOKEN) are needed, or use --dry-run")
    check_layout()

    import discord
    client = discord.Client(intents=discord.Intents.default())

    @client.event
    async def on_ready():
        try:
            guild = client.get_guild(args.guild)
            if guild is None:
                print("The bot isn't in that server. Invite it first (see the blueprint).")
            else:
                await build(guild, args.name)
        finally:
            await client.close()

    client.run(args.token, log_handler=None)


if __name__ == "__main__":
    main()
