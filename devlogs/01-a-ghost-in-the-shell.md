# 01. A ghost in the shell: Tourne'Table

*September 12 to October 15, 2026. From a borrowed visualizer to 2.1.*

Everything in this repo started with bars dancing on a wallpaper.

I wanted a visualizer that sat behind my desktop icons and cost nothing. Salyts had already written a Windhawk mod that drew one (Desktop Audio Visualizer), and it was a great starting point: the architecture, the capture and render split, the bootstrap. I took that file and started working outward. Within a day I had more settings than it started with, and within a week I had my own mod.

## v0.1 to v0.3: making it mine

The first job was the foundation. The audio thread and the render thread were passing band values back and forth in a way that could tear (the renderer could read half of an update). I put a seqlock between them, so the render thread always reads a complete set of bands or tries again. Then came the things I actually wanted to see: a sensitivity curve, independent padding for the panel, negative padding (so the bars can bleed past their box), colour fixes, a reworked beat flash, smoother motion, and media controls (previous, play and pause, next) wired to Windows' own media session.

## v0.4: it leaves Explorer

Up to here the mod lived inside `explorer.exe`, like most Windhawk mods. I didn't like that. If the visualizer crashed, it took the shell with it, and if Explorer restarted, the visualizer died.

So I moved it into its own process with Windhawk's tool-mod pattern. It shows up as its own line in Task Manager and survives an Explorer restart: it just waits for the desktop to come back and reattaches.

Doing that properly took two releases. 0.4.0 moved it. 0.4.1 fixed the thing I hadn't noticed: every window had been created on a thread the launcher kills a moment later, so the windows were living on borrowed time with no message loop. Everything moved onto a real thread with a real `GetMessage` loop.

## v0.5 to v0.6: the drag saga

I wanted to grab the visualizer and drag it somewhere else. That turned into four releases of pain.

- 0.5.0: drag-to-move with a modifier and a mouse button.
- 0.5.1: the drag desynced, and it turned out the new process had no DPI awareness declared (inside Explorer it got that for free). Every position on a scaled screen was being worked out against a virtualised desktop.
- 0.5.2: contrast fixes and drag diagnostics, because I couldn't tell where the lag was coming from.
- 0.5.3: moved the drag hook off the render thread. It still felt like dragging through mud.

So 0.6 stopped trying to make cursor-chasing fast and did the thing that doesn't need to be fast: **Keyboard Move**. Hold a modifier, tap a direction, and it steps an exact number of pixels, like a tiling window manager nudging a window. One keypress is one discrete jump, so there's nothing for the renderer to fall behind on. Drag still exists, but it's off by default and its description says plainly why it can feel sluggish.

The same release found something embarrassing: the UI thread had never initialised COM. Every `CoCreateInstance` from it failed, which meant WIC never loaded, which meant **custom media icons had never worked in any version**. You set one and silently got the built-in glyph. That's also when I added settings validation: a typo in a colour field now tells you which field, what you typed and what it's using instead, instead of quietly doing nothing.

## v0.9: half of every description was missing

Every colour setting's help text said "Format is" and then stopped. For many versions.

Windhawk's settings block is YAML, and in YAML an unquoted value ends at the first ` #`. My descriptions said `Format is #AARRGGBB, #RRGGBB...`, so everything from the `#` on was being thrown away. A truncated sentence still reads like a finished one, so nobody noticed. Neither the compiler nor my own validator can catch it either: to the compiler the block is a comment, and the validator checks values, not the schema. Now every description with a `#` in it is quoted.

## v1.0 to v1.3: release, and the catalog

1.0.0 was the first release. Submitting it to the Windhawk catalog took three pull requests:

- The first was closed. The AI reviewer's main blocker was whether this should be its own catalog entry or a version bump of Salyts' mod, plus two smaller ones: the page read like a critique of a neighbouring mod because of my comparative benchmarks, and a licensing question. I took the benchmarks off the page (they moved to my own repo) and argued for a separate entry: it's a tool mod in its own process, a different failure model and lifetime, not a few more options.
- The second (1.2.0) went to a human review. The maintainer asked for two things: the documented form for a global `std::thread`, and to **remove the 64-bit-only line**, because `windhawk.exe` is 32-bit in Windhawk 1.x. It was merged on September 18.
- 1.3.0 added Input Gain and Auto Gain. Loopback capture sees each app's own volume but not the Windows master slider, so a quiet app read as quiet no matter how loud the system was. Merged September 20.

Remember that 32-bit note. It comes back in chapter 08.

## v1.1: Target FPS never worked above 64

This is my favourite bug of the whole project.

Every Target FPS above 64 came out at exactly 64. 90, 120, 144, 240: all 64. The render loop measured the time since the last frame with `GetTickCount64()`, which only moves when the Windows timer ticks, every **15.625 ms**. At 144 FPS the loop wanted a 6.944 ms interval, but the clock could only ever say 0 or about 15.6. So it waited while the reading was 0, and rendered the moment the tick moved: one frame per tick. 1 / 15.625 ms = 64.0.

Measured before: 63.97 FPS against a 144 target. After switching to `QueryPerformanceCounter` and doing the interval in floating point: **141.71 FPS**, 2.22 times the frames.

I deliberately didn't fix it with `timeBeginPeriod()`. That would have worked, but it changes the timer for every process on the machine and hurts power use everywhere. That's the opposite of the point of this mod.

Same release: `Present` stopped waiting for a vertical blank (DirectComposition and my own timer already do the pacing), which made stalls about four times rarer. And because the old performance numbers had all been measured at an accidental 64 FPS, I re-measured everything: **+1.6 percentage points of total CPU, +7.2 W and +1 °C** at 144 FPS, measured as the difference between back-to-back captures with the same music playing.

## v1.4 and v1.5: the oscilloscope and the NPU

1.4 gave the oscilloscope its own time base, so it shows a waveform instead of a smear. 1.5 let you pick where the work runs: GPU, CPU, or (experimentally) an NPU through OpenVINO, loaded at runtime only if it's installed. OpenVINO only ships 64-bit, so that one path stays 64-bit forever.

## 2.0: every bar its own band

2.0 was a rewrite of the analysis. Up to 1.5, the sound was split into 7 fixed bands and every bar interpolated between those 7 numbers, so changing Bar Count changed how many bars you saw, not how much they knew. The new **Precision** engine gives each bar its own frequency band, measured straight from the spectrum:

- three resolution tiers, so bass comes from long windows and treble from short fast ones;
- log, linear, Mel, Bark and ERB scales, the IEC 61260 fractional-octave bands, or one bar per musical note;
- A, C and Z weighting, tilt, RMS or peak, and a choice of FFT window;
- ballistics in real time, so bars move at the same speed at 60, 144 or 240 FPS;
- loudness (EBU R128), true peak, and a new Goniometer shape.

The renderer got rewritten too: a Direct3D 11 path that draws everything from one tiny shader, **skips any frame where nothing moved**, and stops the audio stream itself after a few seconds of silence (Deep Idle).

The first 2.0 beta failed to compile its shaders on every Windows PC, so everybody silently fell back to the old renderer. My test compiler (DXC) is more lenient than Windows' own (`fxc`), which reserves the word `pass` and rejects an empty macro. There's now a lint pass that checks for that whole class of thing before a build goes out.

Then I ran a proper review: three independent reviewers (render, engine, correctness), each scoring findings by impact, reach, confidence and cost. The render pass alone cut buffer uploads while playing from about 432 a second to 144, and compositor commits from 144 a second to nearly zero.

Then the fun stuff: eight new styles (LED Meter, Line Spectrum, Polar Bloom, Spectrogram, VU Needles with real IEC ballistics, Stereo Field, Particles), Reflection, Glow and Bloom, a Media Card with album art and a seek bar, My Styles with a live Style Editor, bar modifiers, decimal sizes, Click to Seek, Snap while dragging, Dock To App, Split Into Two Pieces, **Place In = Taskbar** and **Place In = Panel**.

## 2.1: out of Tourne Desktop, into the community

By October the visualizer lived inside Tourne Desktop (chapter 05) as one of its modules, built from the same source file. 2.1 is that build released as a Windhawk mod of its own again, with the public id so it replaces 1.x, and with the 32-bit build the catalog maintainer asked for back in September.

It also got sharing. Copy code puts your whole setup on the clipboard as one line, Save file writes a `.ttstyle`, and **Community** lists the styles people shared in this repo. Share Mine opens an issue here with your style already in it, and a bot checks it, answers with a code anyone can paste to try it, and adds it to the list once it's approved. The first style through it was mine.

**Where it is:** [`windhawk-mods/tourne-table/`](../windhawk-mods/tourne-table/), and the styles in [`community/`](../community/).
