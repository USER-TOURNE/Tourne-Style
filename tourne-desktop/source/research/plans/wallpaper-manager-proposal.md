# TourneSuite wallpaper manager proposal

Written 2026-10-04. Proposal only: no code until I say go.

My ask: a wallpaper manager "stylish, almost exactly like" saneAspect's "The Perfect Wallpaper Recolorizer for Hyprland" (YouTube S3QsAuLO7fQ), "way more user friendly, absolutely no terminal". The video and its thumbnail can't be reached from the server (YouTube is blocked here, and this time its thumbnails were too), so this proposal is built from the handoff's description: a wallpaper picker, plus a recolourer that fits any wallpaper to any palette or set. Screenshots or the clip would let the look match it closely; until then the look below is a guess.

## What the user gets

One window, **Tourne Wallpapers**, opened from the tray menu (and later from a bind). No terminal, no config file needed.

1. **Library**: big thumbnails of every picture in the folders the user picks (Pictures\Wallpapers by default, more can be added). Filter by colour, orientation, or "fits this screen". Favourites.
2. **Apply**: click a picture, pick a screen (or all screens), pick the fit (fill, fit, stretch, centre, span). Applies at once.
3. **Recolour**: the picture opens in a side by side view with a split slider (before on the left, after on the right, drag the line). Below it, a row of palettes as swatch strips. Click one and the preview recolours live. A few sliders change how.
4. **Save and apply**: the recoloured copy is saved next to Tourne's data, never over the original, and can be applied straight away.
5. **Slideshow**: a folder or a list, every N minutes, optionally recoloured to the current palette as each one shows.

## The recolourer

The goal is a picture that keeps its shapes, light and depth, but uses only (or mostly) the palette's colours, the way the reference thumbnail shows.

Everything is done in OKLab, which `palette-gen.cpp` already uses, so equal steps look equal.

Methods, picked in the window as plain names:

| Name in the window | What it does | Good for |
| --- | --- | --- |
| **Smooth** (default) | Builds a 3D colour lookup table: every colour is pulled toward the palette colours near it, weighted by distance (a Gaussian blend), so gradients stay smooth. | Photos, soft art |
| **Exact** | Every pixel becomes its nearest palette colour. | Flat art, logos, pixel art |
| **Tone map** | Keeps each pixel's lightness and maps it onto a ramp made from the palette (dark to light). | Monochrome or duotone looks |
| **Keep lightness** (switch, on by default for Smooth) | After mapping, puts the original lightness back so faces and skies don't flatten. | Any |

Sliders:

- **Strength**: 0 to 100, blends the result with the original.
- **Softness**: how wide the Smooth blend reaches (only for Smooth).
- **Dither**: off, fine (ordered), or grain (error diffusion). Hides banding in Exact mode and on pixel art.

Speed: the lookup table is 33 x 33 x 33 entries, made once per palette and settings. Applying it is one table lookup per pixel, so a 4K picture should take well under a second on a worker thread with WIC decoding. The preview works on a screen-sized copy, so sliders feel instant.

## Palettes

Where palettes come from:

1. **The current Tourne theme** (`HKCU\Software\Tourne\Theme`, the twelve roles).
2. **Sets**: a set's palette (once the Pieces and sets format exists).
3. **Built in**: a starter list of well-known community palettes (for example Catppuccin, Nord, Gruvbox, Rosé Pine, Tokyo Night, Everforest, Dracula, Solarized). Only their colour values are used, which are published for exactly this use; no code from their projects.
4. **Made by the user**: from another picture (the existing palette maker), from a seed colour, or by picking colours.
5. **From the screen**: with the colour picker from the parity map, later.

## How it fits Tourne theme

The theme engine already makes a palette from whatever wallpaper Windows shows. That makes a loop possible: recolour to palette A, the theme reads the new wallpaper and makes palette B. Two choices:

- **Recoloured to a palette**: Tourne sets the theme straight from that palette (as `source = seed` does today, but with all roles) and writes `MadeFrom = recolour`, so the theme engine does not re-read the picture. The shell, apps and wallpaper all match exactly.
- **Plain wallpaper**: unchanged; the theme engine reads it as now.

## Windows side

- Setting the picture: `IDesktopWallpaper` (per screen `SetWallpaper`, `SetPosition`, `SetSlideshow`). This is the public COM interface Settings uses, so Windows' own Themes folder gets rewritten as usual and the theme engine keeps working.
- Thumbnails: `IShellItemImageFactory`, which uses Windows' thumbnail cache, so browsing a big folder is cheap.
- Files: `%APPDATA%\Tourne\Wallpapers\` holds recoloured copies (named by picture, palette and settings, so the same request is never made twice) and the library list.
- Idle: the window does nothing while closed. A slideshow is one timer to the next change. Folder changes are noticed with a folder watch only while the window is open.

## The window's look

"Stylish" with no terminal means a real window, not a settings list. Two ways to build it:

| Option | Look | Cost |
| --- | --- | --- |
| **A. Direct2D, drawn by Tourne** (recommended) | Same drawing as the taskbar and switcher, coloured by the Tourne theme, rounded cards, smooth scrolling. | No new runtime. Small exe growth. More work for text boxes and scrolling. |
| B. WebView2 page inside Tourne | Easiest to make very fancy (CSS, animations); could reuse the CSS injector's palette CSS. | Runs a browser engine while open (about 100 MB of memory), though it's already on every Windows 11 PC. |

Recommended: A, because I asked for resource-efficient, and the window then looks like the rest of the shell.

## Config (all off by default)

    wallpaper {
        folders = %USERPROFILE%\Pictures\Wallpapers
        slideshow = off            # off, or minutes
        slideshowRecolour = off    # recolour each one to the current palette
        recolour {
            method = smooth        # smooth, exact, tone
            strength = 100
            softness = 50
            keepLightness = true
            dither = off           # off, fine, grain
        }
    }

The window writes these; nobody has to edit them.

## Phases

1. **Library and apply**: the window, folders, thumbnails, apply per screen with fit. Tray menu item.
2. **Recolourer**: palettes (theme, built in, from picture, seed), the three methods, sliders, split preview, save and apply, theme hand-off.
3. **Slideshow** with optional recolour, and sets (a set carries a wallpaper and its recolour settings).
4. Later: animated and video wallpapers (a Tourne window behind the desktop icons, paused when covered), and the colour picker as a palette source.

## Decisions for Tourne

1. The look: please upload the clip or a few screenshots of the reference so the window can match it.
2. Window built with Direct2D (recommended) or WebView2.
3. Which built-in palettes to start with (the list above is the suggestion).
4. Phases 1 and 2 together as one step, or phase 1 first so it can be tested sooner (recommended: together, since the recolourer is the point).
