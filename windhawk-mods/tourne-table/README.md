![Tourne'Table - a ghost in your desktop's shell](https://raw.githubusercontent.com/USER-TOURNE/Tourne-Style/main/windhawk-mods/tourne-table/media/tourne-header.png)

> **Status: in the Windhawk Explore catalog.** 2.1.0 passed review on October 17, 2026 ([ramensoftware/windhawk-mods#6269](https://github.com/ramensoftware/windhawk-mods/pull/6269)), so you can install it straight from Windhawk. This file is the same as the catalog's; what changed over the earlier 2.1 copy is in the [CHANGELOG](CHANGELOG.md).
>
> **New beta: 2.3.0.** [`tourne-table-2.3.0-beta.wh.cpp`](tourne-table-2.3.0-beta.wh.cpp) adds Bass, the Bass Line lane, Depth, Glass Bars, Engine Cores, a quick settings panel and a new Style Editor. It installs over the catalog version and keeps your settings. Everything in it is in the [CHANGELOG](CHANGELOG.md).

# `Tourne'Table` **[Audio Visualizer]**

![Tourne'Table Audio Visualizer](https://raw.githubusercontent.com/USER-TOURNE/Tourne-Style/main/windhawk-mods/tourne-table/media/11.gif)

*The Oscilloscope shape running live audio: bottom-left placement, blurred panel, single-pixel border.*

> **A real-time audio visualizer that lives on your Windows desktop.**
> Built on the foundation of Salyts' Desktop Audio Visualizer, rebuilt around performance.

Play music. Bars dance on your wallpaper. That's the whole idea.

It listens to **whatever your PC is already playing** (Spotify, YouTube, a game, a call) and draws it behind your desktop icons. No virtual audio cable, no drivers, nothing to configure. It just picks up your system audio.

It is built to be cheap to run. The render thread wakes only at the frame rate you ask for, the wallpaper blur is computed once instead of every frame, and the drawing surface is sized to the widget rather than the whole desktop, so at a 144 FPS target the visualizer costs about **a third of one CPU core** and **one degree** of CPU package temperature while it plays, and nothing at all while it doesn't.

## ABOUT THIS PROJECT

Built with an emphasis on lower resource consumption, more efficient rendering, better frame pacing, expanded visualization options, dynamic album-art integration, native Windows media info, deeper customization, and power-conscious idle behavior.

The goal is simple:

> **Make the desktop move with the music, without making the CPU move mountains to do it.**

## Runs as its own process

Tourne'Table doesn't live inside `explorer.exe`. It runs as its own dedicated process using Windhawk's tool-mod pattern, so you'll see it as its own entry in Task Manager, separate from the shell. Restarting `explorer.exe` doesn't kill it. It just reattaches behind the desktop icons once the shell is back. Requires a Windhawk build with tool-mod support (1.7.3+, including the 2.0 alpha line).

---

## ✧ NEW IN 2.0 AND 2.1

2.1 replaces 1.x. If you ran one of the 2.0 test builds from GitHub, turn it off: only one Tourne'Table should run. If you also use **Tourne Desktop** (my separate desktop app, not part of Windhawk), its taskbar counts as the taskbar for Place In = Taskbar, and it can show, hide or pin the panel through the window message `TourneTable.Panel`. The performance figures further down were measured on 1.x and haven't been re-measured on 2.x yet.

**Share your style.** Right-click > My Styles > Style Editor > Share: **Copy code** puts your setup on the clipboard as one line for any chat, **Save file** writes a `.ttstyle` file, and **Community** opens the setups people have shared in [the Tourne'Style repo](https://github.com/USER-TOURNE/Tourne-Style/tree/main/community) in your browser (bring one in with **Paste code** or **Open file**; the mod itself never goes online) or opens a ready GitHub issue to share yours. A setup carries every setting except the ones that belong to your PC: the audio device, icon files, panel hotkey, monitor, docked app and graphics card. Browse them all with live previews on the [community styles page](https://user-tourne.github.io/Tourne-Style/).

**Every bar is its own measurement now.** Up to 1.5 the analysis split the sound into 7 fixed bands and every bar interpolated between those 7 numbers, so Bar Count and FFT Size changed how many bars you saw, not how much they knew. The new **Precision** engine gives each bar its own frequency band with its own edges, measured straight from the spectrum:

- bass comes from long, finely resolved windows and treble from short, fast ones (three resolution tiers), so a 1/24-octave bar at 50 Hz is a real reading rather than a smear;
- band layouts on Log, Linear, Mel, Bark or ERB scales, the standard **IEC 61260** fractional-octave bands every RTA uses, or one bar per **musical note**;
- **A / C / Z weighting** (IEC 61672), a dB-per-octave **tilt**, RMS or peak detection, and a choice of FFT window;
- **ballistics on real time**, so bars move at the same speed at 60, 144 or 240 FPS, with presets for an analyzer, a VU meter and both broadcast PPMs, and gravity peak caps;
- **loudness** (ITU-R BS.1770-5 / EBU R128: momentary, short-term, integrated), 4x-oversampled **true peak**, PLR and stereo **correlation** in the readout, and a new **Goniometer** shape.

The 1.4 analysis is still there as **Analysis Engine = Classic**.

**You choose where the work runs.** **Workload** is GPU, CPU or Hybrid: Hybrid analyses on the CPU and draws on the GPU (the cheapest split almost everywhere), GPU does both on the graphics chip, CPU does both on the processor.

**A new renderer built to cost next to nothing.** **Renderer = Direct3D 11** draws everything from one tiny shader and a buffer of bar heights, **skips any frame where nothing moved**, keeps the text on its own surface that only redraws when the text changes, and tells Windows when the panel is opaque so it copies instead of blending. When audio stops, drawing stops, and after a few more seconds of silence the audio stream itself stops (**Deep Idle**). The 1.5 Direct2D path is still there for comparison.

**Target FPS 0** matches your display's refresh rate, whatever it is.

**Right-click for quick settings.** Shape, colours, analysis, readout, overlays and **which audio device it listens to**, any output, input or virtual device (VB-Audio, Voicemeeter, an Ableton return), all live, without opening Windhawk. Plus a **Terminal** shape (text characters: columns, a waterfall or text meters) and a **media widget** layout: title and artist inside the panel, pixel-sharp text, a track progress bar and media controls pinned to the panel.

**Seven new styles.** LED Meter, Line Spectrum, Polar Bloom, Spectrogram, VU Needles (real IEC VU ballistics), Stereo Field and Particles, all in the Shape list and the right-click menu, plus **Reflection**, a fading mirror under the bars.

**Glow and Bloom.** A soft halo around every bar, worked out in the same shader pass that draws it, and a lens-style bloom from a quarter-size blur that only runs on frames that change. Direct3D 11 renderer.

**A Media Card.** Media Controls > Layout = Card: album art with previous / play / next on hover, a seek bar, one-click output switching and a volume slider. The output button changes the **Windows default output** for every app (through the same undocumented Windows interface sound-switcher tools use), and the slider sets the **system master volume**; both act only when you click, drag or scroll them, and both stay as set after the mod is turned off, like any change made in Windows' own sound settings. Theme its background, border, corner radius, art size and accent (icon colour, custom, album art or your Windows accent).

**My Styles.** Right-click > My Styles > Style Editor: mix a base style, colours, bar sizes, reflection, glow and bloom while the visualizer previews it live, save it under a name, and pick it from the menu any time.

**Finer looks.** Outline and Shadow around every bar; bar modifiers Hollow, Dashed, Tilt, Afterimage (a slowly falling trail) and Mirror Gap; scale numbers on Spectrogram and VU Needles; album colours that ease in; any Unicode character in Terminal. Reflection now works on both renderers.

**Exact sizes.** Bar width, gap, height, padding, borders, radii, fonts and offsets take decimals down to 0.01, and so do Sensitivity, Smoothing, Input Gain, Blur, Glow, Bloom, Shadow, Reflection, Afterimage, Beat Flash Intensity, Rainbow Speed and the dock, split and snap distances (turn Pixel Snap off to keep the fractions). The Style Editor's sliders step by 0.01 with the arrow keys.

**Placement.** Drag snaps to the screen's edges and centre, the taskbar, the tray and the Start button (hold Shift to place freely). Position > Place In = Taskbar puts it inside the taskbar. Position > Dock To App keeps the visualizer beside an app's window as it moves, and Split Into Two Pieces cuts it in two, e.g. either side of the taskbar's centred icons, at almost no cost.

**Media.** Click to Seek on the progress bar (off by default, with an app list), custom icons on the Media Card, and Media Controls > Stacking (always on top, or a normal window apps can cover).

---

## ◈ PERFORMANCE AT A GLANCE

Measured on an **Intel Core Ultra 265KF** (8 P-cores plus 12 E-cores), running a 120-bar oscilloscope at a 144 FPS target with background blur on, and with media controls, the peak-frequency readout, peak hold and beat flash all enabled.

Every figure below is the **change against an idle baseline**, captured back to back in the same session with the same music playing in both, so what you are reading is the cost of the mod rather than whatever else the machine happened to be doing.

| Metric | Cost of running it |
|:--|--:|
| Total CPU usage | **+1.6 percentage points** *(about 0.3 of one core)* |
| Peak single-thread | **+10.1 points** |
| CPU package power | **+7.2 W** |
| CPU package temperature | **+1.0 °C** |

Medians across 543 samples with it running and 384 without, at 1.25 s intervals. Medians rather than averages because both captures contained brief unrelated background spikes, and a median is not moved by them.

When audio stops, rendering stops, not "slows down," *stops*.

Full methodology, raw traces and caveats live in [the bench notes](https://github.com/USER-TOURNE/Tourne-Style/tree/main/windhawk-mods/tourne-table/bench) rather than on this page.

![Tourne'Table Audio Visualizer](https://raw.githubusercontent.com/USER-TOURNE/Tourne-Style/main/windhawk-mods/tourne-table/media/10.gif)

*Bottom-left placement against a dark wallpaper, with the Now Playing label sitting above the panel.*

---

## ◆ VISUAL STYLES

### 9 Shapes

| Shape | What it does |
|:--|:--|
| **Stereo** | Classic equalizer bars. Low notes left, high notes right. |
| **Mountain** | Peaks in the middle, tapers toward both edges. |
| **Mirror** | The opposite, grows from the outside edges inward. |
| **Wave** | Normal bars with a slow ripple rolling through them. |
| **Breathe** | A gentle swell that rises with the music instead of jumping. |
| **Dots** | Stacked dots instead of solid bars, old LED-meter look. |
| **Radial** | Bars shoot outward from a center point, like a sunburst. |
| **Oscilloscope** | A single line tracing the actual sound wave. |
| **Goniometer** | The stereo field: every sample plotted with mono straight up and width sideways, with a correlation bar underneath. *(2.0)* |

![Oscilloscope closeup](https://raw.githubusercontent.com/USER-TOURNE/Tourne-Style/main/windhawk-mods/tourne-table/media/8.gif)

*Closeup: the waveform drawn as one continuous line, with the Now Playing label and media buttons alongside.*

### 9 Color Modes

| Mode | What it does |
|:--|:--|
| **Solid** | One flat color. |
| **Gradient** | Fades between two colors across the bars. |
| **Reactive Gradient** | Shifts toward the second color as things get *louder*. |
| **Windows Accent** | Matches your Windows accent color, updating instantly. |
| **Album Art** | Pulls the dominant color from the playing track's cover. |
| **Dynamic Album** | Gradient between the cover art's two strongest colors. |
| **Acrylic** | Grows more opaque the louder it gets, invisible in silence. |
| **Rainbow Cycle** | Continuously cycling hue, adjustable speed. |
| **Tourne** | Built-in teal → red gradient from my personal palette. |

### Plus

- **2 orientations**: bars grow vertically or horizontally
- **3 anchors**: grow from the Bottom, the Top, or **both directions from the Middle**
- **Peak Hold Caps**: thin markers hang at each bar's recent peak and slowly fall *(classic hardware EQ)*
- **Beat Flash**: bars brighten on detected bass hits, on top of any color mode
- **Multiband Oscilloscope**: the waveform tints toward whichever part of the spectrum is loudest

---

## ♪ NOW PLAYING DISPLAY

Shows the current **artist and title** above the visualizer, pulled from the same Windows media session that powers your volume popup. Works with Spotify, browsers, and most media players.

Fades in on track change, fades out after a configurable delay. Custom color, font family and size.

---

![Oscilloscope closeup](https://raw.githubusercontent.com/USER-TOURNE/Tourne-Style/main/windhawk-mods/tourne-table/media/7.gif)

*A narrower panel. The trace scales to whatever width the bar settings give it.*

## ◐ THE PALETTE

This project follows my personal theming palette: reflected in the repo screenshots. It can be changed to any hex / RGB / RGBA value you want.

| Name | Hex | RGB |
|:--|:--|:--|
| Green | `#15ffa2` | `21, 255, 162` |
| Red | `#ff8f8f` | `255, 143, 143` |
| Pale Green | `#64aa89` | `100, 170, 137` |
| Alt Pale Green | `#6ba6a0` | `107, 166, 160` |
| Pale Red | `#a95d5d` | `169, 93, 93` |
| Dark Teal *(background)* | `#121c21` | `18, 28, 33` |

The built-in **Tourne** color mode is drawn from these.

---

## ☰ RIGHT-CLICK MENU *(2.0)*

Right-click the visualizer for quick settings, applied the moment you pick them:

- **Audio Source**: the default output (what you hear), the default input, or **any active device by name**, outputs and inputs, virtual ones included. VB-Audio Cable and Matrix, Voicemeeter buses, an Ableton return routed to its own device: if Windows lists it, it's there. Outputs are captured by loopback, inputs recorded directly.
- **Shape**, **Terminal Style**, **Color Mode**, **Analysis Engine**, **Band Layout**, **Weighting**, **Ballistics**, **Readout**, **Workload**, **Renderer**, **Target FPS**.
- Toggles for Peak Hold Caps, Beat Flash, Now Playing, the Track Progress Bar, Media Controls, Pixel-Sharp Text, **Pixel Snap** and **Subpixel Nudges**.
- **Pause Visualizer**, which blanks it and stops the audio stream until you right-click the same spot again (or the media strip) and untick it.
- **Copy Quick Settings** and **Reset Quick Settings**.

Windhawk lets a mod read its settings but not write them, so a menu choice is kept as the mod's own saved value and laid over the settings page. The page keeps showing what's underneath, which is why the menu says how many quick settings are active. **Copy Quick Settings** puts them on the clipboard as `name = value` lines, worded as the menu shows them (for example `Shape = Terminal`, `Peak Hold Caps = On`), so you can make them permanent on the settings page. **Reset Quick Settings** then hands everything back to the page.

The menu only opens where the visualizer is actually showing on the desktop, never through a window that covers it, and a right-click there doesn't open the desktop's own menu. If you'd rather keep a plain right-click for the desktop, set **Right-Click Menu** (Interaction) to **Ctrl + Right-Click**. Right-clicking the media strip opens the same menu.

---

## ◫ SHARP AT EVERY SIZE, OR BETWEEN PIXELS *(2.0)*

**Why it could look soft.** Every size was scaled by your display scaling and every position was a percentage of the screen, both kept as exact fractions. At 125% a 2 px bar is 2.5 px wide, and a Position of 50% can put the visualizer at x = 812.37. A bar that starts or ends partway through a pixel gets an antialiased edge, so some sizes came out crisp and others blurry.

**Pixel Snap** *(Position, on by default)* rounds the position and every scaled size (bar width, gap, max size, panel padding) to whole pixels, so every bar edge, the panel and the text sit exactly on the pixel grid at any size and any scaling. Bar heights stay fractional, which keeps their motion smooth. The process also declares per-monitor DPI awareness on each of its own threads now, so Windows never stretches the picture on a monitor whose scaling differs from your main one.

**Subpixel placement** is Pixel Snap **off**. The picture can then sit between pixels, so it moves by fractions of a pixel, at the cost of slightly soft edges. Physically, a crisp edge and a position between two pixels can't both happen.

- **Keyboard Move Fine Step** *(Interaction)*: 1/2, 1/4, 1/8 or 1/16 px.
- Hold **Keyboard Move Fine Key** with the move modifier, or tick **Subpixel Nudges** in the right-click menu to make every nudge fine.
- Fine steps move the visualizer. The media strip and the text overlays still move whole pixels, since a window and a text box can't sit between them.
- With Pixel Snap on, fine nudges still add up: four quarter-pixel presses move the picture one pixel.

---

## ▣ THE TERMINAL SHAPE *(2.0)*

**Shape = Terminal** draws the visualizer as text: a grid of characters in a monospace font, in three styles.

- **Columns**: every bar a column of characters (`#` by default) that turns the **Hot Color** above **Hot Threshold**, with the peak cap as a character of its own. The cava / btop look.
- **Waterfall**: a spectrogram in characters. Each line is one moment and each column one band, with the character picked from **Waterfall Ramp** by level and the newest line on top.
- **Meters**: text meters, `Bass:    [######     48%]`, for bass, mid, treble and the loudest band.

It runs on the same per-bar levels as every other shape, so the engine, band layout, weighting and ballistics all apply. With the Direct3D 11 renderer the characters come from a glyph atlas baked once, and the whole grid is one instanced draw: a few hundred cells cost the same as a few hundred bars. Pick a pixel font and **Text Rendering = Pixel** for hard-edged characters.

---

## ◰ RECIPES

**A compact media widget** (coral bars on a dark teal panel, the title over the artist on the right, controls at the top left, a progress bar underneath):

| Setting | Value |
|:--|:--|
| Shape / Bar Count / Bar Width / Bar Gap | Stereo / `64` / `2` / `2` |
| Bar Max Size / Idle Size / Corner Radius | `36` / `2` / `0` |
| Color Mode / Color | Solid / `#FFFF8F8F` |
| Background Color / Padding / Corner Radius | `#FF121C21` / `12 12 52 12` / `4` |
| Background Border Size / Color | `1` / `#FF2A3A40` |
| Now Playing | on, Display Seconds `0`, Layout **Two lines**, Placement **Inside the panel, top**, Alignment **Right** |
| Now Playing Color / Artist Color | `#FFFF8F8F` / `#FF64AA89` |
| Font / Text Rendering | a pixel font (e.g. Press Start 2P at `8`) / **Pixel** |
| Track Progress | on, Below the panel, Height `2`, Gap `6`, Color `#FFFF8F8F`, Track `#FF64AA89` |
| Media Controls | on, Icon Size `14`, Spacing `8`, Icon Color `#FF64AA89`, Anchor **Panel, top left**, Insets `10` / `10` |

The two colours are the Red and Pale Green of the palette above, and the panel is its Dark Teal.

**A terminal readout** (green and red characters on black):

| Setting | Value |
|:--|:--|
| Shape | Terminal |
| Terminal Style | Columns, Waterfall or Meters |
| Font / Size | Consolas or Cascadia Mono / `12` |
| Background Color / Corner Radius | `#FF000000` / `0` |
| Peak Hold Caps | on (Columns) |

---

# ✦ EVERY SETTING, EXPLAINED

**You don't need any of this to use the mod.** The defaults work. This is for when you want to tweak something and you're wondering what a word means.

## Appearance

**Shape**: Which of the 8 styles above to draw.

**Orientation**: *Horizontal* = a row of bars growing up and down. *Vertical* = a column growing left and right.

**Bar Count**: How many bars. More = finer detail, wider visualizer. Range **1-2048** (enough to span a 4K or ultrawide screen).

**Bar Width**: How fat each bar is, in pixels.

**Bar Gap**: Space between bars, in pixels. Set to `0` and they touch.

**Bar Max Size**: How tall a bar gets at full volume. This is the overall height of the visualizer.

**Bar Idle Size**: How tall bars sit in silence. `0` makes them vanish completely; a few pixels leaves a thin resting line.

**Bar Corner Radius**: How rounded the bar corners are. One number rounds all four equally, or give four numbers separated by spaces for individual control: `top-left top-right bottom-right bottom-left`. Example: `5 5 0 0` rounds only the top.

**Color Mode**: Which of the 9 coloring styles above to use.

**Color**: The color used in Solid mode. Format is `#AARRGGBB` or `#RRGGBB`: that's **A**lpha (transparency), then **R**ed, **G**reen, **B**lue in hex. Lower the first two digits to make it see-through.

**Gradient Color 1 / 2**: Start and end colors for the gradient modes.

**Sensitivity**: How hard the bars react. Too low and quiet music barely moves them; too high and everything slams to max. Range 0-300. Turn it down for bass-heavy tracks, up for quiet recordings.

**Input Gain (dB)**: A fixed level trim on the captured audio, applied before everything else. Range -24 to +24, default 0.

> **Why this exists.** Loopback capture sees the mix *after* each app's own volume slider but *before* the Windows master slider. So if you keep Spotify at 40% and the system at 80%, the visualizer only ever sees that 40%, and turning the system up does not help it. Input Gain is the control that does. It is also the only setting that scales the **Oscilloscope** waveform directly, which otherwise has no level control of its own. Auto Gain scales the trace too when it is on, and the trace clips flat rather than running off the panel.
>
> **It moves the idle thresholds with it.** Pause When Silent and Auto-Hide When Idle both key off how far the bars are moving, so raising Input Gain lowers the level that counts as silence, and lowering it raises that level. Sensitivity has always behaved the same way for the same reason. If you push Input Gain a long way up, expect the mod to consider quieter things "playing".

**Auto Gain**: Adapts the level continuously so quiet sources still fill the bars, instead of retuning Sensitivity per app or per track. Off by default.

It only ever boosts, never cuts, so loud material behaves exactly as it does with this off. Through silence it holds its last value rather than winding up, which means the noise floor is never lifted and **idle shutdown still works normally**. The idle test reads the un-boosted level against the same threshold it always used, so turning Auto Gain on neither extends nor shortens how long the mod stays awake.

Two things it does affect, worth knowing before you go hunting for them:

- Because it normalizes the loudest band to a fixed target, **Sensitivity stops doing much for quiet material** while it is on. That is inherent to how a levelling gain works, not a bug.
- On the **Oscilloscope**, a lower Sensitivity produces a *larger* trace, since a smaller measured peak asks for a bigger boost.

**Auto Gain Max Boost (dB)**: The ceiling on that lift. Range 0-24, default 12. Lower it if quiet passages are being flattened more than you want, raise it if a very quiet source still will not fill the bars.

**EQ Preset**: Which frequencies get emphasized *visually*. Doesn't touch your actual audio.
`Default` no adjustment · `Bass` boosts lows · `Rock` boosts mids and highs · `Pop` heavy on highs · `Jazz` warmer, gentler highs · `Electronic` boosts bass and treble, scoops the middle.

**FFT Size**: How finely sound gets analyzed. An FFT is the math that splits audio into separate frequencies: think of it as sorting sound into buckets by pitch. More buckets = finer detail, slightly more CPU.
`1024` fastest, plenty for most · `2048` / `4096` noticeably crisper · `8192` maximum detail.

> **With the Precision engine (2.0)** this is the size of each of the three resolution tiers, and bass detail comes from the tiers rather than from raising this. `2048` *(the new default)* is the sweet spot. Sensitivity no longer has to be retuned per FFT Size either: levels are calibrated in dBFS, so a change of FFT Size changes resolution, not height.

> **The Oscilloscope no longer cares what this is set to.** It used to take its trace over a window of exactly one FFT block, so raising FFT Size stretched the trace to cover more time and updated it less often, which is why `8192` used to look so rough on that shape. The trace now runs on its own time base, set by **Oscilloscope Time Window** below. Pick FFT Size for the bars and leave the scope out of it.
>
> **One more note:** the higher the FFT Size, the more accurate the Sensitivity slider becomes for your specific audio setup. The correlation generally runs: **as FFT Size goes up, your Sensitivity will need to go up too.** For now that means fine-tuning Sensitivity per EQ Preset *and* per FFT Size, for your particular placement, size and personal adjustments.

**Frequency Scale**: How the frequency range spreads across the bars (with Band Layout = Frequency Scale). This matters more than it sounds.
- **Log**: the natural-feeling default. Gives bass and treble roughly equal visual space, matching how humans hear pitch.
- **Linear**: spreads by raw Hz. Since most musical energy lives low, this crams all the action into a sliver on the left and leaves the right mostly dead. Technically accurate, visually dull.
- **Mel**: uses the *mel scale*, built from research on how people actually perceive pitch. Like Log, tuned to human hearing.
- **Bark**: the ear's critical bands (Traunmueller's formula): more room for the midrange, where hearing is most finely tuned.
- **ERB**: equivalent rectangular bandwidths (Glasberg and Moore), the widths of the ear's own auditory filters. Between Log and Bark.

**Peak Frequency Readout**: Shows the loudest note as a live number, e.g. `1.2 kHz`, or loudness figures instead (see **Readout Content** under Analysis).

**Peak Readout Position**: Where that number sits. Horizontal: Left / Center / Right. Vertical: Above / Top / Middle / Bottom / Below.
> **Above** and **Below** place it fully *outside* the bars so it never overlaps the visualization.

**Multiband Oscilloscope Coloring**: Only affects the Oscilloscope shape. The line tints toward whatever part of the spectrum is loudest: warm for bass, green for mids, blue for treble. Overrides Color Mode for that shape.

**Oscilloscope Time Window (ms)**: How much audio the trace shows end to end. Range 5-250, default `24`.

> This is the scope's time base, and it is the setting that decides what the trace *looks* like. Small values stretch out individual wave cycles, so a bass note reads as a few big smooth curves. Large values pack more of the signal into the same width, so it reads as a rolling envelope. Around `20-30` is the hardware-scope look; **`150-180` is close to what FFT Size `8192` used to give**.
>
> It is deliberately independent of **FFT Size**. The trace is sampled on its own clock at roughly 100 updates a second whatever the FFT is doing, peak-sampled rather than point-sampled so it neither aliases nor loses amplitude as the window widens, and aligned to a rising zero crossing so the waveform holds still instead of sliding sideways.

**Oscilloscope Damping**: Slows how fast the trace changes *shape*, without slowing how often it is *drawn*. Range 0-100, default `0`.

> These are two different things and the distinction is the whole point. The trace is still redrawn every frame at your Target FPS, so nothing gets choppier. What damping changes is how far each point is allowed to move per frame, so the waveform evolves slowly enough to follow with your eyes instead of being a new picture every frame.
>
> This is what makes a wide **Time Window** legible rather than busy. On its own a 170 ms window is a lot of information changing fast; damped, it becomes a shape you can actually track. Try `150` window with `60-80` damping.
>
> It costs nothing. The easing is a single multiply per point on the render thread, and it does not touch the capture path or the frame rate.

**Anchor**: Which edge bars grow from. `Bottom` rise upward *(classic)* · `Top` hang downward · `Middle` grow **both directions** from a center line.

**Peak Hold Caps**: Leaves a thin marker floating at each bar's recent peak, which slowly drifts down.

**Beat Flash** + **Intensity**: Flashes bars brighter on bass hits. Intensity controls how hard.

**Rainbow Cycle Speed**: How fast the rainbow rotates. Rainbow mode only.

**Now Playing Text**: Toggles the artist/title display.

**Now Playing Color / Font / Font Size**: Styling for that text. The font must be **installed on your system**: type the exact family name. Windows silently falls back to a default on a typo rather than erroring, so double-check spelling if nothing changes.
> The Peak Frequency Readout shares these same font settings.

**Now Playing Display Seconds**: How long the text stays up after a track change before fading.

## Analysis (Precision Engine)

Everything here applies to **Analysis Engine = Precision**, the default. With **Classic** the bars come from the 1.4 analysis and these settings are ignored.

**Analysis Engine**: **Precision** gives every bar its own frequency band, measured from the spectrum. **Classic** is the 1.4 analysis: 7 fixed bands the bars interpolate between. Kept for comparison, and because some people will prefer its looser look.

**Band Layout**: How the spectrum is cut into bars.
- **Frequency Scale** *(default)*: **Bar Count** bars spread from Min to Max Frequency on the **Frequency Scale** set under Appearance.
- **IEC 61260**: the standard fractional-octave bands (base-10, centred on 1 kHz) that every real-time analyzer uses. **Octave Fraction** picks the width.
- **Musical**: one bar per note of equal temperament (or per quarter tone), tuned to **Tuning (A4)**.

IEC and Musical decide how many bars there are, so **Bar Count** is ignored for them: 1/3 octave from 20 Hz to 20 kHz is 31 bars, 1/6 is 60, 1/12 is 120 and 1/24 is 240.

**Octave Fraction**: 1/1, 1/3, 1/6, 1/12 or 1/24 octave. 1/3 is the classic RTA; 1/12 is a semitone. Musical only uses 1/12 and 1/24.

**Min / Max Frequency**: The analysed range. Max is capped at 21 kHz, below the Nyquist limit of every common output device.

**Tuning (A4)**: Reference pitch for Musical, 400 to 480 Hz.

**Weighting**: **Z** is flat. **A** follows the ear at quiet listening levels and pulls the bass right down; **C** is the gentler loud-level curve. Both from IEC 61672-1, added per band, free.

**Tilt (dB per octave)**: A slope around 1 kHz.

> **What 0 means here.** Bars are measured as band power and, by default, referenced to a 1/3-octave band (see Level Reference), so **pink noise already reads flat at 0**, the way it does on a hardware RTA. Most music falls a little faster than pink. **1.5** *(default)* makes a typical modern mix read level, the same thing SPAN's 4.5 dB/oct default does on a plain FFT display (an FFT display needs 3 dB/oct just to flatten pink noise; this one doesn't).

**Detector**: **RMS** sums the power of every frequency in the band, the right reading for music and noise. **Peak** shows the band's loudest single frequency, closer to what a tone reads on a meter.

**Level Reference**: **Third-octave** *(default)* scales every band to the power a 1/3-octave band at the same centre would hold. Changing Bar Count or Band Layout then changes how fine the bars are, not how tall. **Band** shows each band's true power, so wide bars read higher than narrow ones.

**FFT Window**: **Hann** is the general-purpose choice. **Blackman-Harris** has 92 dB of dynamic range, so quiet detail next to a loud tone stays visible. **Flat-top** reads a tone's exact level wherever it falls between bins, at the cost of resolution. **Hamming** has a narrower peak and more leakage.

**Bass Detail**: The resolution tiers. The audio is analysed as captured, decimated by 4, and decimated by 16, each with the same FFT size, and each bar is measured from the fastest tier that can resolve it. **High** *(default)* uses all three, which is what makes fine bass bars real; **Standard** stops at 4; **Off** is a single FFT.

> **Why tiers instead of a bigger FFT.** Bin spacing is sample rate over FFT size, 23 Hz at 48 kHz and 2048. A 1/24-octave band at 50 Hz is 1.4 Hz wide, so one FFT would need to be enormous to resolve it, and an enormous FFT reacts slowly everywhere, treble included. Decimating first gives the bass a long window and the treble a short one. At FFT Size 2048 the three windows are 43, 171 and 683 ms long, and the two deeper tiers are only re-analysed when enough new audio has arrived to make it worthwhile.

**Channel**: Which signal the spectrum comes from. **Mix** averages all channels, as before. **Mid** is what both speakers share; **Side** is their difference, which shows exactly where a mix's stereo width lives.

**Display Floor / Ceiling (dBFS)**: The levels that draw as an empty and a full bar, before Sensitivity. A full-scale sine is 0 dBFS; loud modern music sits around -20 dBFS per third-octave band. Defaults -72 and -12.

> **Sensitivity** still works with Precision, as a gain: each step from the default 150 is 0.2 dB, so the slider covers -30 to +30 dB. **Sensitivity Curve** still shapes the top of the range, **Auto Gain** lifts quiet sources toward 85% of the range (boost only, held through silence), **EQ Preset** becomes a dB gain on the low, mid and high zones, and **Motion Smoothing** slows whichever Ballistics you pick, up to 6x.

**Ballistics**: How bars rise and fall. All of them run on elapsed time, so the motion is the same at any frame rate.
- **Snappy** *(default)*: closest to the 1.4 feel.
- **Smooth**: slower, easier to follow.
- **Analyzer**: falls a steady 20 dB per second, like a hardware RTA.
- **VU**: 300 ms integration.
- **PPM, EBU (Type II)** and **PPM, DIN (Type I)**: the IEC 60268-10 return rates (24 dB in 2.8 s, 20 dB in 1.5 s).
- **Custom**: **Custom Attack (ms)** and **Custom Release (dB/s)**.

**Peak Hold Time / Peak Fall**: With **Peak Hold Caps** on, how long a cap hangs, then whether it falls with **Gravity** (accelerating, the classic analyzer look) or at a steady **Linear** rate.

**Readout Content**: What the readout set up under **Peak Frequency Readout** shows.
- **Peak frequency**: as before.
- **Loudness**: **M**omentary (400 ms), **S**hort-term (3 s) and gated **I**ntegrated loudness in LUFS, per ITU-R BS.1770-5 and EBU R128.
- **Loudness, true peak, PLR, correlation**: adds 4x-oversampled true peak (dBTP), PLR (true peak minus integrated loudness) and stereo correlation (+1 mono, 0 wide, -1 out of phase).
- **Both**: peak frequency and loudness.

The loudness meters only run while they are on screen.

**Reset Integrated Loudness On Track Change**: Starts a new integrated measurement with each track, so the figure is per track.

> **What loopback can't see.** The mod hears what Windows mixes, after each app's own volume slider and before the master slider. Players in **WASAPI exclusive** or **ASIO** mode bypass that mix entirely, so the visualizer shows nothing while they play, and loudness figures describe the mix, not the file.

## Position

**Horizontal Position**: Left-to-right placement as a percentage. `0` hard left, `50` centered, `100` hard right.

**Vertical Position**: Top-to-bottom, same idea. `0` top, `100` bottom.

**Monitor**: Which screen to draw on. `1` is your first monitor.

## Background

**Enabled**: Draws a panel behind the bars. Turn off for bars floating directly on the wallpaper.

**Color**: Panel color in `#AARRGGBB`. The alpha controls transparency.

**Padding**: Breathing room between the bars and the panel edge.

**Corner Radius**: How rounded the panel corners are. Same one-or-four-value rules as bar radius.

**Blur**: Frosted-glass blur of your wallpaper behind the panel. `0` disables.
> This used to be the single most expensive setting in the mod. It's now computed once and cached, so it's essentially free per frame.

**Border Size / Border Color**: A thin outline around the panel. `0` for none.

## Performance

**Target FPS**: How many times per second it redraws. Higher = smoother, more CPU. Little point exceeding your monitor's refresh rate. **`0` matches the refresh rate**, whatever it is. With the Direct3D 11 renderer a frame where nothing moved is skipped entirely, so a high target costs far less than it used to in quiet or steady passages.

**Pause On Fullscreen**: Stops completely when a fullscreen app runs. Detects both true fullscreen *(games)* and borderless windows. Since the visualizer lives on the desktop it's invisible anyway. This just stops it burning power. Resumes automatically.

**Pause When Silent (seconds)**: After this long without audio, drops to a trickle instead of full speed: it wakes only when audio arrives or four times a second, and with Direct3D 11 presents nothing unless something changes. `0` disables (and Deep Idle with it).

**Deep Idle** *(2.0)*: Five seconds after Pause When Silent kicks in, stops the audio stream itself and reads Windows' own peak meter four times a second instead, restarting the moment anything plays. A running capture stream registers an audio power request, which can keep the PC from sleeping (`powercfg /requests` shows it); a stopped one doesn't. On by default.

**Auto-Hide When Idle** + **Delay**: Fades out entirely after prolonged silence. Once fully faded it **stops rendering completely**, not just invisible, genuinely doing nothing until audio returns.

**Pause When Covered**: Stops rendering *and* audio capture while fully hidden behind another window.
> **Off by default.** Reliably detecting "am I covered?" on Windows 11 is genuinely tricky: the shell is full of invisible windows that report themselves as visible. The check is deliberately conservative (only a fully-covering, real application window counts), but if the visualizer ever vanishes when it shouldn't, this is the switch to flip.

## Hardware

There are two jobs in this mod, and they can run on different silicon: **analysing** the audio (the FFTs that turn sound into frequency bands) and **drawing** the result.

**Workload**: Where the work runs.
- **Hybrid** *(default)*: analysis on the CPU, drawing on the GPU. Analysis is a fraction of a percent of one core, and the GPU draws for almost nothing, so this is the cheapest split on nearly every PC.
- **GPU**: the analysis runs on the graphics chip as well, in four small compute passes that feed the drawing directly, so the CPU only copies audio and nothing comes back each frame but 8 bytes of statistics. Needs the Precision engine and the Direct3D 11 renderer, and uses at most FFT Size 4096. Worth trying on a strong iGPU; measure it against Hybrid rather than assume.
- **CPU**: analysis and drawing both on the processor (drawing in software, through WARP). The GPU is left with nothing but Windows' own compositing. Fine for a small widget; a full-width strip at a high frame rate is real CPU work.

**Drawing Device**: Which GPU draws.
- **Auto** *(default)*: the GPU your chosen monitor is plugged into. On a laptop with two GPUs, or a desktop with the iGPU left on, drawing on the *other* one means every frame gets copied across to the GPU that scans it out, which is exactly the kind of uneven frame time this mod is trying to avoid. Auto asks Windows which GPU owns that screen and uses it, and follows if the answer changes.
- **Integrated GPU**: the power-saving GPU, the same one Windows picks for "Power saving" in Graphics settings. Falls back to Auto, with a warning, if there isn't one (F and KF processors have no iGPU, and desktops often switch it off in the BIOS).
- **Discrete GPU**: the high-performance GPU. Same fallback.
- **CPU**: software rendering, the same as Workload = CPU.

If the GPU is ever lost mid-session (a driver update, a driver reset, a laptop switching GPUs), the mod rebuilds itself on whatever is available instead of freezing until restart.

**Renderer**:
- **Direct3D 11** *(default)*: every bar, dot, cap, spoke and scope segment is one instanced quad from a tiny vertex shader that reads the bar heights straight from a buffer. The CPU writes two numbers per bar and issues a handful of draw calls per frame, whatever the bar count, and the pixel shader computes exact antialiased edges, so shapes look like Direct2D's. On top of that:
  - **Frames where nothing moved are skipped.** The picture is summarised in a hash (bar heights to a quarter pixel, colours, the trace) and compared with the last frame shown. If it matches, nothing is drawn and nothing is presented, so Windows has nothing to recompose.
  - **The text lives on its own surface**, redrawn only when the text or its fade changes. The animated surface covers just the panel, not the room reserved around it for text.
  - **The panel can be opaque** (see Opaque Panel).
- **Direct2D**: the 1.5 path, unchanged, including Smooth Mode's batching. Kept for comparison, and used automatically, with a warning, if Direct3D 11 can't start (a GPU below feature level 11.0, or `d3dcompiler_47.dll` missing).

**Opaque Panel**: When the background panel can't show anything behind it (**Blur** on, or a panel colour with full alpha) and **Auto-Hide** is off, the Direct3D 11 renderer creates the panel's surface with alpha ignored and lets the compositor cut the rounded corners. Windows then copies those pixels instead of blending them every frame; Chromium measured the same switch at 3.12 W against 2.28 W. **Off** always blends, for comparison.

**Smooth Mode**: Frame timing locked to the display, and motion on elapsed time. **Auto** *(default)* turns it on when drawing runs on an integrated GPU or the CPU; **On** works on any GPU and is worth trying on a discrete one too; **Off** is the 1.4 timing. What it changes:

- **Frame timing locked to the display.** The 1.4 loop asks for a frame every `1000 / Target FPS` milliseconds and lets the timer's wake-up latency pile up, which is why 144 requested measured 141.7 delivered. On a 144 Hz panel that is a couple of refreshes every second with no new frame: a repeat, which you see as a hitch. Smooth Mode schedules from Windows' own composition clock instead, places every frame half a refresh away from the deadline on either side, and never accumulates drift.
- **Target FPS rounded to an even fraction of the refresh rate.** 60 FPS on a 144 Hz screen means frames held for 2, 3, 2, 3 refreshes, which reads as judder however precise the timer is. Smooth Mode runs the nearest whole fraction instead: 60 becomes 72 on 144 Hz, 48 if you set it to 50, 60 on a 60 Hz screen. Asking for more than the refresh rate gets the refresh rate rather than frames nobody can see.
- **Motion that doesn't depend on frame timing.** With the Classic engine, bar rise and fall, peak caps, beat flash and scope damping were all "move this fraction per frame", so a late frame visibly slowed everything down for a moment. They now scale by how much time actually passed. (The Precision engine runs on elapsed time whatever this is set to.)
- **A proper animation clock.** Wave, Breathe and Rainbow ran off a millisecond counter that only ticks every 15.6 ms, stored in a float that loses precision the longer the PC has been on. They now run off the high-resolution clock. (Always, with the Direct3D 11 renderer.)
- **With the Direct2D renderer:** the background baked once into one image, all bars in one sprite batch, and cached text layouts.

---

![Oscilloscope closeup](https://raw.githubusercontent.com/USER-TOURNE/Tourne-Style/main/windhawk-mods/tourne-table/media/4.gif)

*The same shape in a warm colour: the Oscilloscope picks up Color Mode like every other shape.*

# ▲ WHERE THE EFFICIENCY COMES FROM

In rough order of measured impact.

### 1. Precision frame pacing: the biggest single win

The obvious way to pace a desktop widget is `DwmFlush()`, which blocks until the monitor's next refresh. That wakes the render thread **on every vertical blank, forever** (60, 144, 240+ times a second) regardless of the target FPS, whether anything needs redrawing, or whether the visualizer is even visible.

This barely registers as CPU% in Task Manager, because the thread is blocked, not spinning. But every wake-up drags a core out of deep idle. Do that continuously and the core never settles into its efficient sleep states, which reads as a small, permanent bump in package power and temperature. The classic "low usage, still runs warm" signature.

> **Note:** this becomes exponentially more noticeable on AMD architecture.
>
> **Note:** also exponentially more noticeable if you have **C-States disabled** in your BIOS or elsewhere. Shoutout to Process Lasso, Core Director, Park Control and HWiNFO64 for helping me work out why all my E-cores were sitting at 65-70 °C when they were supposed to be idle.

**Instead:** a high-resolution waitable timer firing only at the configured rate. Plain `Sleep()` isn't good enough. It's quantized to ~15.6 ms, which would turn a 60 FPS target into stuttery 30-40 FPS.

### 2. Pre-rendered background blur

A Gaussian blur is a full-image convolution: the most expensive thing Direct2D does in this scene. Recomputing it every frame is pure waste, because its input (your wallpaper) never changes.

**Instead:** it is computed exactly once into a cached bitmap, and each frame just copies that. The cache covers only the widget's bounding box, which is **tens of KB of video memory rather than several MB**. It re-bakes automatically if the widget moves or resizes.

### 3. Widget-sized render surface

The visualizer occupies a thin strip, so a desktop-spanning render surface would clear and present millions of untouched pixels every frame, and park two full-desktop buffers in VRAM.

**Instead:** the surface is sized to the widget's bounding box, and the composition layer is offset to position it. That is roughly an order of magnitude less per-frame pixel work and VRAM.

### 4. Cached geometry

Building the background panel and border means allocating a path geometry and constructing four lines and four arcs by hand. It is rebuilt only when size, padding, radii or border width actually change (in normal use, almost never) rather than every frame.

### 5. Cached monitor lookup

`EnumDisplayMonitors()` is a real round-trip through the display driver stack. The monitor is resolved once and cached, refreshed on display change, instead of being looked up per frame.

### 6. Reduced frame latency

DXGI queues up to three frames ahead by default. For a passive widget that's pure latency and power draw with no upside. Capped at **1**.

### 7. Genuine idle shutdown

**Auto-Hide** now stops rendering completely once faded: presents one blank frame, then exits the render path entirely. **Pause When Covered** stops rendering and capture while hidden, checked once per second rather than per frame.

### 8. One wake per frame, for audio and video together *(2.0)*

1.5 woke two threads to get audio onto the screen: a capture thread on every WASAPI packet (about 100 times a second while anything played) and a pacing thread once per frame. 2.0 merges them: once per frame, one thread drains whatever audio has arrived, analyses it and hands the frame over. At 60 FPS that is roughly 100 fewer wake-ups a second.

### 9. Frames that didn't change are never presented *(2.0, Direct3D 11 renderer)*

Every present hands Windows' compositor a new buffer and wakes it to blend that rectangle again, even when the pixels are identical. The renderer now compares a summary of the frame with the last one shown and skips both the draw and the present when nothing moved by a quarter of a pixel. During sustained notes, quiet passages and every idle state, that is most frames, and a compositor with nothing to do goes back to sleep.

### 10. Three steps of idle *(2.0)*

- **Playing**: one wake per frame.
- **Silent** (after Pause When Silent): wakes only when audio arrives or four times a second, and presents nothing unless something changes.
- **Deep Idle** (five seconds later): the loopback stream itself is stopped, and Windows' own peak meter is read four times a second instead. A running capture stream registers an audio power request, which can hold the PC out of sleep (check `powercfg /requests`); a stopped one doesn't. The engine thread also asks Windows for efficiency-core scheduling (EcoQoS) while idle, and leaves the choice to Windows while playing.

### 11. Less for the compositor to blend *(2.0, Direct3D 11 renderer)*

The animated surface covers only the panel; the text has a surface of its own that only changes when the text does. And when the panel is opaque (Blur on, or a solid colour), Windows is told so, and copies those pixels instead of blending them.


---

![Tourne'Table Audio Visualizer](https://raw.githubusercontent.com/USER-TOURNE/Tourne-Style/main/windhawk-mods/tourne-table/media/2.gif)

*The full-width strip in a red colour mode.*

# ▦ ON THE NUMBERS

The figures above come from HWiNFO64 sensor logging: two captures taken back to back in one session, one with the mod running and one with it disabled, with the same music playing throughout both so the only thing that changed was the mod itself. 543 samples running, 384 disabled, at 1.25 s intervals.

The full write-up (raw tables, the method, run-to-run variance, and an honest account of where the measurements fall short) is in [the bench notes](https://github.com/USER-TOURNE/Tourne-Style/tree/main/windhawk-mods/tourne-table/bench). It does not belong on a catalog page, so it is not reproduced here.

The caveats worth stating up front: **HWiNFO measures the whole machine, not this process alone.** A number quoted as the mod's cost therefore includes the work the mod causes elsewhere, in the compositor and the graphics driver, not only the time spent in its own threads. That makes it larger than a per-process trace would show, and it is the honest figure to quote, because it is what the machine actually pays. Both captures also contained short unrelated background spikes, which is why the numbers above are medians rather than averages.

---

![Tourne'Table Audio Visualizer](https://raw.githubusercontent.com/USER-TOURNE/Tourne-Style/main/windhawk-mods/tourne-table/media/3.gif)

*Near-silence: the trace flattens out. When audio stops entirely, the render loop stops with it.*

## ♥ CREDITS

**[USER-TOURNE](https://github.com/USER-TOURNE)**: Author and maintainer: performance work, new features, benchmarking and documentation.

**[Salyts](https://github.com/Salyts)**: Original author of Desktop Audio Visualizer. This project exists because the foundation was good enough to be worth optimizing. Author of his own mod and repo; and a base for this one.

**[GR0UD](https://github.com/GR0UD)**: Audio visualizer code the original was adapted from. Author of his own work; and the base for Salyts.

### A coincidence worth acknowledging

While I was midway through my initial testing, **NeiZ** (author/maintainer, with **SuperSmile123** contributing) released [Desktop Audio Visualizer Plus](https://github.com/ramensoftware/windhawk-mods/commit/e01d0d0dbd6204804235831fd7f68821e4614028) (`neiz-supersmile-audio-visualizer`). I had planned to publish my own commit that same night: holy coincidence.

His release spurred another round of testing on my end, and I've since gone through roughly twelve more iterations. I didn't want to ship something that essentially achieved what I was already going for, especially if they'd figuratively led me out to pasture to put a bullet in me: aha.

To be explicit about attribution: **I did not borrow from or reference NeiZ or SuperSmile123's work at any point**, other than benchmarking theirs for efficiency to decide whether continuing development was worth it. Thanks to them and their contributors regardless.

---

## ◘ LICENSE

Released under the **GNU General Public License v3.0** (GPL-3.0), from 2.3.0 on.

```
Tourne'Table
Copyright (c) 2026 USER-TOURNE

This program is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the Free
Software Foundation, version 3.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details:
https://www.gnu.org/licenses/gpl-3.0.html
```

### What that means

> Take it, fork it, learn from it, rewrite the parts I got wrong. If you
> share a changed version, share its source too, under the same licence, so
> it stays open for the next person. Nobody gets to close it up and sell it.
>
> *(Versions up to 2.1.1 were MIT, and the copies people already have stay
> MIT.)*

### Third-party notices

This project builds on upstream work by **Salyts** and **GR0UD**. Their original
code carries its own license terms: if you redistribute this, carry their notices
along with mine.

---

## ❖ SUPPORT

If this saved you some degrees, some watts, or just made your desktop nicer to
look at, you can throw something in the hat. Entirely optional, genuinely appreciated.

**[ko-fi.com/tourne](https://ko-fi.com/tourne)**
