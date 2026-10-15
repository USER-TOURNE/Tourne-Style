# Tourne'Table community styles

Styles people share for [Tourne'Table](../windhawk-mods/tourne-table/). Every one here shows up in the visualizer's own **Community** list, and you can try any of them in a few seconds: it plays at once, and you choose **Keep** or **No**.

There's also a visual gallery with live previews: the [community page](https://user-tourne.github.io/Tourne/) (GitHub Pages, from `docs/`).

## Try a style

Either way works:

- **In Tourne'Table:** right-click the visualizer, then **My Styles > Style Editor > Community**, and pick one.
- **From this page:** copy a style's code below, then press **Paste code** in the Style Editor.

A style only changes the parts it carries (Look, Motion, Extras, Place) and only the boxes you've ticked. **No** puts yours back exactly.

## Share yours

1. Open the Style Editor (right-click the visualizer, then **My Styles > Style Editor**).
2. Tick the parts you want to share. Place is off at first, since everyone's screens differ.
3. Type a name in **Saved style**, press **Community**, then **Share Mine to the Community**.
4. GitHub opens a new issue with your style already in it. Add a screenshot and an `about: one line` line if you like, then send it.

A bot reads the issue right away: it checks the style, tells you what it kept, and replies with a code anyone can paste to try it before it's even listed. When Tourne adds the `approved` label, the bot adds it to `setups/`, the index and the gallery below, and it's in everyone's Community list from then on.

No GitHub? **Copy code** puts a one-line code on your clipboard for any chat, and **Save file** writes a `.ttstyle` file you can send anywhere.

### What a style never carries

Your audio device, hotkeys, file paths (custom icons, the NPU runtime) and hardware choices (renderer, workload, GPU) never go in a style, and the bot drops anything that isn't one of the settings Tourne'Table itself shares. The list comes straight from the mod's source (`kSetupKeys`), so the two can't drift apart.

## The format

A style is plain text:

```
Tourne'Table setup 1
name=Mono Mirror
author=Tourne'Table
parts=look
appearance.shape=mirror
appearance.barCount=48
```

Then `setting=value` lines (only the ones that differ from the defaults) and `quick.<key>=value` for quick settings. A code is either `TT2:` (packed by Tourne'Table with Windows' XPRESS compression) or `TT2T:` (the same text in base64, which this page and the bot use). Paste code takes both.

## For the maintainer

- `setups/index.txt` lists the styles, one line each: `file|name|author|about`. Tourne'Table reads it from `main` when someone presses Community.
- `python community/tools/community.py check` checks every style; `gallery` rewrites the list below. The workflow in `.github/workflows/community-styles.yml` runs both.
- To add one by hand: put the text in `setups/<short-name>.ttstyle` (letters, digits, `-`, `_` and `.`), add its index line, run `gallery`, commit to `main`.

## Gallery

<!-- gallery:start -->

### Tourne Everforest

by **USER-TOURNE** · Tourne's own: thin coral bars on an acrylic panel, green beat flash, now playing in a pixel font

Shape `line` · Parts: Look, Motion, Extras · Colours: `#e57474`, `rgba(76, 227, 141, .35)`, `#a95d5d`, `#FFE57474`, `#594CE38D`, `#FFA95D5D`

[Open the file](setups/tourne-everforest.ttstyle) · copy this code, then **Paste code** in the Style Editor:

```
TT2T:VG91cm5lJ1RhYmxlIHNldHVwIDEKbmFtZT1Ub3VybmUgRXZlcmZvcmVzdAphdXRob3I9VVNFUi1UT1VSTkUKcGFydHM9bG9vayxtb3Rpb24sZXh0cmFzCmFuYWx5c2lzLmF0dGFja01zPTUwCmFuYWx5c2lzLmJhbGxpc3RpY3M9Y3VzdG9tCmFuYWx5c2lzLmJhbmRMYXlvdXQ9c2NhbGUKYW5hbHlzaXMuYmFzc0RldGFpbD0yCmFuYWx5c2lzLmNoYW5uZWw9bWl4CmFuYWx5c2lzLmRiQ2VpbGluZz0tMzUKYW5hbHlzaXMuZGJGbG9vcj0tMTA1CmFuYWx5c2lzLmRldGVjdG9yPXJtcwphbmFseXNpcy5lbmdpbmU9cHJlY2lzaW9uCmFuYWx5c2lzLmxldmVsUmVmZXJlbmNlPXRoaXJkX29jdGF2ZQphbmFseXNpcy5sb3VkbmVzc1Jlc2V0T25UcmFjaz0xCmFuYWx5c2lzLm1heEZyZXF1ZW5jeT0yMDAwMAphbmFseXNpcy5taW5GcmVxdWVuY3k9MjAKYW5hbHlzaXMub2N0YXZlRnJhY3Rpb249MTIKYW5hbHlzaXMucGVha0ZhbGw9Z3Jhdml0eQphbmFseXNpcy5wZWFrSG9sZE1zPTI3NQphbmFseXNpcy5yZWFkb3V0PWZyZXF1ZW5jeQphbmFseXNpcy5yZWxlYXNlRGJQZXJTZWNvbmQ9MjUKYW5hbHlzaXMudGlsdD0zCmFuYWx5c2lzLnR1bmluZ0E0PTQ0MAphbmFseXNpcy53ZWlnaHRpbmc9egphbmFseXNpcy53aW5kb3c9YmxhY2ttYW5faGFycmlzCmFwcGVhcmFuY2UuYWZ0ZXJpbWFnZT0wCmFwcGVhcmFuY2UuYWZ0ZXJpbWFnZUxlbmd0aD0xLjIKYXBwZWFyYW5jZS5hdXRvR2Fpbj0wCmFwcGVhcmFuY2UuYXV0b0dhaW5NYXhCb29zdD04CmFwcGVhcmFuY2UuYmFyQ29ybmVyUmFkaXVzPTAgMCAwIDAKYXBwZWFyYW5jZS5iYXJDb3VudD0xMDAKYXBwZWFyYW5jZS5iYXJEYXNoPTAKYXBwZWFyYW5jZS5iYXJEYXNoR2FwPTIKYXBwZWFyYW5jZS5iYXJHYXA9MgphcHBlYXJhbmNlLmJhckhvbGxvdz0wCmFwcGVhcmFuY2UuYmFySG9sbG93V2lkdGg9MS41CmFwcGVhcmFuY2UuYmFySWRsZVNpemU9MAphcHBlYXJhbmNlLmJhck1heFNpemU9MTgKYXBwZWFyYW5jZS5iYXJNaXJyb3JHYXA9MAphcHBlYXJhbmNlLmJhclRpbHQ9MAphcHBlYXJhbmNlLmJhcldpZHRoPTIKYXBwZWFyYW5jZS5iZWF0Rmxhc2hDb2xvcj1yZ2JhKDc2LCAyMjcsIDE0MSwgMSkKYXBwZWFyYW5jZS5iZWF0Rmxhc2hFbmFibGVkPTEKYXBwZWFyYW5jZS5iZWF0Rmxhc2hJbnRlbnNpdHk9OTgKYXBwZWFyYW5jZS5jb2xvcj0jZTU3NDc0CmFwcGVhcmFuY2UuY29sb3JNb2RlPWFjcnlsaWMKYXBwZWFyYW5jZS5lcVByZXNldD1kZWZhdWx0CmFwcGVhcmFuY2UuZmZ0U2l6ZT00MDk2CmFwcGVhcmFuY2UuZnJlcVNjYWxlPWJhcmsKYXBwZWFyYW5jZS5meEJsb29tPTE1CmFwcGVhcmFuY2UuZnhCbG9vbVJhZGl1cz0xCmFwcGVhcmFuY2UuZnhHbG93PTUKYXBwZWFyYW5jZS5meEdsb3dSYWRpdXM9MwphcHBlYXJhbmNlLmZ4T3V0bGluZUNvbG9yPSNCMEZGRkZGRgphcHBlYXJhbmNlLmZ4T3V0bGluZVdpZHRoPTAKYXBwZWFyYW5jZS5meFNoYWRvdz0wCmFwcGVhcmFuY2UuZnhTaGFkb3dDb2xvcj0jMDAwMDAwCmFwcGVhcmFuY2UuZnhTaGFkb3dTb2Z0bmVzcz00CmFwcGVhcmFuY2UuZnhTaGFkb3dYPTIKYXBwZWFyYW5jZS5meFNoYWRvd1k9MwphcHBlYXJhbmNlLmdyYWRpZW50Q29sb3IxPXJnYmEoNzYsIDIyNywgMTQxLCAuMzUpCmFwcGVhcmFuY2UuZ3JhZGllbnRDb2xvcjI9I2E5NWQ1ZAphcHBlYXJhbmNlLmlucHV0R2Fpbj0wCmFwcGVhcmFuY2Uubm93UGxheWluZ0FsaWduPWNlbnRlcgphcHBlYXJhbmNlLm5vd1BsYXlpbmdBcnRpc3RDb2xvcj0jNjRhYTg5CmFwcGVhcmFuY2Uubm93UGxheWluZ0JnQm9yZGVyQ29sb3I9IzY0YWE4OQphcHBlYXJhbmNlLm5vd1BsYXlpbmdCZ0JvcmRlclNpemU9MQphcHBlYXJhbmNlLm5vd1BsYXlpbmdCZ0NvbG9yPSMxNDFiMWUKYXBwZWFyYW5jZS5ub3dQbGF5aW5nQmdDb3JuZXJSYWRpdXM9MAphcHBlYXJhbmNlLm5vd1BsYXlpbmdCZ1BhZGRpbmc9NAphcHBlYXJhbmNlLm5vd1BsYXlpbmdDb2xvcj0jZTU3NDc0CmFwcGVhcmFuY2Uubm93UGxheWluZ0Rpc3BsYXlTZWNvbmRzPTI1CmFwcGVhcmFuY2Uubm93UGxheWluZ0VuYWJsZWQ9MQphcHBlYXJhbmNlLm5vd1BsYXlpbmdGb250PURlbGljYXR1cwphcHBlYXJhbmNlLm5vd1BsYXlpbmdGb250U2l6ZT0xMgphcHBlYXJhbmNlLm5vd1BsYXlpbmdMYXlvdXQ9b25lX2xpbmUKYXBwZWFyYW5jZS5ub3dQbGF5aW5nT2Zmc2V0WD0wCmFwcGVhcmFuY2Uubm93UGxheWluZ09mZnNldFk9MAphcHBlYXJhbmNlLm5vd1BsYXlpbmdQbGFjZW1lbnQ9YWJvdmUKYXBwZWFyYW5jZS5vcmllbnRhdGlvbj1ob3Jpem9udGFsCmFwcGVhcmFuY2Uub3NjaWxsb3Njb3BlRGFtcGluZz0wCmFwcGVhcmFuY2Uub3NjaWxsb3Njb3BlTXVsdGliYW5kRW5hYmxlZD0wCmFwcGVhcmFuY2Uub3NjaWxsb3Njb3BlV2luZG93TXM9MjQKYXBwZWFyYW5jZS5wZWFrRnJlcUFsaWduSD1sZWZ0CmFwcGVhcmFuY2UucGVha0ZyZXFBbGlnblY9dG9wCmFwcGVhcmFuY2UucGVha0ZyZXFCZ0JvcmRlckNvbG9yPSM2NGFhODkKYXBwZWFyYW5jZS5wZWFrRnJlcUJnQm9yZGVyU2l6ZT0xCmFwcGVhcmFuY2UucGVha0ZyZXFCZ0NvbG9yPSMxNDFiMWUKYXBwZWFyYW5jZS5wZWFrRnJlcUJnQ29ybmVyUmFkaXVzPTAKYXBwZWFyYW5jZS5wZWFrRnJlcUJnUGFkZGluZz0yCmFwcGVhcmFuY2UucGVha0ZyZXFFbmFibGVkPTAKYXBwZWFyYW5jZS5wZWFrRnJlcU9mZnNldFg9MAphcHBlYXJhbmNlLnBlYWtGcmVxT2Zmc2V0WT0wCmFwcGVhcmFuY2UucGVha0hvbGRDb2xvcj1yZ2JhKDc2LCAyMjcsIDE0MSwgMSkKYXBwZWFyYW5jZS5wZWFrSG9sZEVuYWJsZWQ9MQphcHBlYXJhbmNlLnJhaW5ib3dTcGVlZD00MAphcHBlYXJhbmNlLnJlZmxlY3Rpb249OTUKYXBwZWFyYW5jZS5zZW5zaXRpdml0eT03MAphcHBlYXJhbmNlLnNlbnNpdGl2aXR5Q3VydmU9cG93ZXIKYXBwZWFyYW5jZS5zaGFwZT1saW5lCmFwcGVhcmFuY2Uuc21vb3RoaW5nPTEKYXBwZWFyYW5jZS50ZXh0UmVuZGVyaW5nPXBpeGVsCmFwcGVhcmFuY2UudmVydGljYWxBbmNob3I9Ym90dG9tCmJhY2tncm91bmQuYmx1cj0wCmJhY2tncm91bmQuYm9yZGVyQ29sb3I9IzY0YWE4OQpiYWNrZ3JvdW5kLmJvcmRlclNpemU9MQpiYWNrZ3JvdW5kLmNvbG9yPSMxNDFiMWUKYmFja2dyb3VuZC5jb3JuZXJSYWRpdXM9MApiYWNrZ3JvdW5kLmVuYWJsZWQ9MQpiYWNrZ3JvdW5kLnBhZGRpbmc9NAptZWRpYV9jb250cm9scy5jYXJkQWNjZW50PWljb24KbWVkaWFfY29udHJvbHMuY2FyZEFjY2VudENvbG9yPSMxZTM2MzIKbWVkaWFfY29udHJvbHMuY2FyZEFydFNpemU9MTQ5Cm1lZGlhX2NvbnRyb2xzLmNhcmRCYWNrZ3JvdW5kPSMxMjFjMjEKbWVkaWFfY29udHJvbHMuY2FyZEJvcmRlckNvbG9yPSM2NGFhODkKbWVkaWFfY29udHJvbHMuY2FyZEJvcmRlclNpemU9MQptZWRpYV9jb250cm9scy5jYXJkQ29ybmVyUmFkaXVzPTEyCm1lZGlhX2NvbnRyb2xzLmVuYWJsZWQ9MQptZWRpYV9jb250cm9scy5pY29uQ29sb3I9IzY0YWE4OQptZWRpYV9jb250cm9scy5pY29uU2l6ZT0xNgptZWRpYV9jb250cm9scy5pY29uU3BhY2luZz0xMAptZWRpYV9jb250cm9scy5sYXlvdXQ9c3RyaXAKbWVkaWFfY29udHJvbHMucGxhdGVCb3JkZXJDb2xvcj0jNjRhYTg5Cm1lZGlhX2NvbnRyb2xzLnBsYXRlQm9yZGVyU2l6ZT0xCm1lZGlhX2NvbnRyb2xzLnBsYXRlQ29sb3I9IzE0MWIxZQptZWRpYV9jb250cm9scy5wbGF0ZUNvcm5lclJhZGl1cz0wCm1lZGlhX2NvbnRyb2xzLnBsYXRlUGFkZGluZz00CnBlcmZvcm1hbmNlLnRhcmdldEZwcz0wCnByb2dyZXNzLmNvbG9yPSM2NGFhODkKcHJvZ3Jlc3MuZW5hYmxlZD0xCnByb2dyZXNzLmdhcD0zCnByb2dyZXNzLmhlaWdodD0zCnByb2dyZXNzLnBsYWNlbWVudD1hYm92ZQpwcm9ncmVzcy50cmFja0NvbG9yPSMxNDFiMWUKdGVybWluYWwuY29sdW1uR2x5cGg9Iwp0ZXJtaW5hbC5kaW1Db2xvcj0jZTU3NDc0CnRlcm1pbmFsLmZvbnQ9U2NpZW50aWZpY2EKdGVybWluYWwuZm9udFNpemU9MTIKdGVybWluYWwuaGlnaENvbG9yPSNhOTVkNWQKdGVybWluYWwuaG90VGhyZXNob2xkPTU2CnRlcm1pbmFsLmxhYmVsQ29sb3I9I2E5NWQ1ZAp0ZXJtaW5hbC5sb3dDb2xvcj0jNjRhYTg5CnRlcm1pbmFsLm1ldGVyQ29sdW1ucz00MAp0ZXJtaW5hbC5wZWFrR2x5cGg9LQp0ZXJtaW5hbC5yYW1wPSAuOi09KyojJUAKdGVybWluYWwucm93cz04MAp0ZXJtaW5hbC5zY3JvbGxSYXRlPTcKdGVybWluYWwuc3R5bGU9d2F0ZXJmYWxsCnF1aWNrLnNoYXBlPW9zY2lsbG9zY29wZQpxdWljay5iYWxsaXN0aWNzPXNuYXBweQpxdWljay5iYW5kTGF5b3V0PXNjYWxlCnF1aWNrLnRhcmdldEZwcz0wCnF1aWNrLmJhcldpZHRoPTIKcXVpY2suYmFyR2FwPTMKcXVpY2suYmFyTWF4U2l6ZT0xOApxdWljay5meEJsb29tUmFkaXVzPTQKcXVpY2sucmVmbGVjdGlvbj02NQpxdWljay5jb2xvck1vZGU9YWNyeWxpYwpxdWljay5jb2xvcj0jRkZFNTc0NzQKcXVpY2suZ3JhZDE9IzU5NENFMzhECnF1aWNrLmdyYWQyPSNGRkE5NUQ1RApxdWljay5wZWFrSG9sZD0xCnF1aWNrLmJlYXRGbGFzaD0xCnF1aWNrLmJhclJhZGl1cz0wLjAwMDAwMApxdWljay5meEdsb3c9NQpxdWljay5meEdsb3dSYWRpdXM9MwpxdWljay5meEJsb29tPTE1Cg==
```

### Mono Mirror

by **Tourne'Table** · White mirrored bars with a soft glow

Shape `mirror` · Parts: Look

[Open the file](setups/mono-mirror.ttstyle) · copy this code, then **Paste code** in the Style Editor:

```
TT2T:VG91cm5lJ1RhYmxlIHNldHVwIDEKbmFtZT1Nb25vIE1pcnJvcgphdXRob3I9VG91cm5lJ1RhYmxlCnBhcnRzPWxvb2sKYXBwZWFyYW5jZS5zaGFwZT1taXJyb3IKYXBwZWFyYW5jZS5iYXJDb3VudD00OAphcHBlYXJhbmNlLmJhcldpZHRoPTQKYXBwZWFyYW5jZS5iYXJHYXA9MwphcHBlYXJhbmNlLmJhckNvcm5lclJhZGl1cz0yCmFwcGVhcmFuY2UuZnhHbG93PTMwCg==
```

### Neon Gradient

by **Tourne'Table** · Pink to cyan with bloom, peak caps and a reflection

Shape `stereo` · Parts: Look · Colours: `#FFFF2D95`, `#FF00E5FF`

[Open the file](setups/neon-gradient.ttstyle) · copy this code, then **Paste code** in the Style Editor:

```
TT2T:VG91cm5lJ1RhYmxlIHNldHVwIDEKbmFtZT1OZW9uIEdyYWRpZW50CmF1dGhvcj1Ub3VybmUnVGFibGUKcGFydHM9bG9vawphcHBlYXJhbmNlLmNvbG9yTW9kZT1ncmFkaWVudAphcHBlYXJhbmNlLmdyYWRpZW50Q29sb3IxPSNGRkZGMkQ5NQphcHBlYXJhbmNlLmdyYWRpZW50Q29sb3IyPSNGRjAwRTVGRgphcHBlYXJhbmNlLnBlYWtIb2xkRW5hYmxlZD0xCmFwcGVhcmFuY2UucGVha0hvbGRDb2xvcj0jRkZGRkZGRkYKYXBwZWFyYW5jZS5yZWZsZWN0aW9uPTI1CmFwcGVhcmFuY2UuZnhCbG9vbT00MAo=
```

<!-- gallery:end -->
