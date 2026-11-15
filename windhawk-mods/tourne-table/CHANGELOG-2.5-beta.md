# ❀ Tourne'Table 2.5 beta: what changed

*Newest first. Each beta updates the one before it in place and keeps its settings.*

## ❀ v2.5.14 beta (Windhawk): icons for the instrumentals

This is a beta. It keeps the same mod id, so it updates the 2.5.13 beta in place and keeps its settings.

### New: an icon as the instrumental picture

- **Show During Instrumentals: Picture, GIF or icon** now takes an icon too: an .ico file, or an app's own icon from its .exe or .dll. Add a comma and a number after the file for another of its icons (app.exe,2).
- An app's icon is taken at 256 px.

### Fixed

- An .ico file shows its biggest size, steady. Before, its sizes played one after another like a GIF's frames. The same goes for a .tiff with several pages.

## ❀ v2.5.13 beta (Windhawk): instrumental text in shared animations

This is a beta. It keeps the same mod id, so it updates the 2.5.12 beta in place and keeps its settings.

### New: shares carry your instrumental text

- **Share mine to the community...** now also sends your Show During Instrumentals text and its timings (opacity, beat pulse, before the first line, after the last, fades, leaving early, songs without lyrics), as part of your Lyrics look.
- Only text goes: a picture or GIF is a file on your PC, so with Picture or GIF picked, that choice stays out.
- **Take its look too** puts them on yours along with the rest of the look. **Keep my own look** leaves yours as it is.

## ❀ v2.5.12 beta (Windhawk): more for the instrumentals

This is a beta. It keeps the same mod id, so it updates the 2.5.11 beta in place and keeps its settings.

### New: Songs Without Lyrics

- **Lyrics > Instrumental > Songs Without Lyrics** shows your text or picture for the whole of a song no lyrics were found for, once the looking is done. Off by default.

### New: tint for the picture

- **Instrumental Picture Tint** lays a colour of yours over the picture or GIF where it isn't see-through (#RRGGBB or a colour name). Empty keeps its own colours.
- **Instrumental Tint Strength** sets how much of it shows: 100 is a flat silhouette in the tint, lower mixes it with the picture.

## ❀ v2.5.11 beta (Windhawk): two old editor bugs fixed

This is a beta. It keeps the same mod id, so it updates the 2.5.10 beta in place and keeps its settings.

### Fixed: typing in the Lyrics Animations editor

- Typing in any of the form's boxes no longer throws the cursor back to the start after each key. The box you're typing in keeps its text while the file below follows it.

### Fixed: resizing the Style Editor

- Dragging an edge now only moves that edge's side of the layout. The labels on the left (Share, Parts, Base style and the rest) stay in place instead of sliding out of view, which mostly happened at 125% and 150% scaling.
- It can't be made so narrow that buttons run into each other: below a width the labels go above their controls, and that's as small as it gets.
- The Lyrics Animations editor's smallest width is a little bigger, so its top row of buttons always fits.

## ❀ v2.5.10 beta (Windhawk): something to watch in the instrumentals

This is a beta. It keeps the same mod id, so it updates the 2.5.9 beta in place and keeps its settings.

### New: Show During Instrumentals

- **Lyrics > Instrumental > Show During Instrumentals** puts something in the lyrics' spot while the song plays without words: **Text** (your own words or symbols) or **Picture or GIF** (a file of yours; a GIF plays). Off by default (Nothing).
- It uses the same stretches Instrumental Disperse clears for: a long wait to the next line, or an empty timed line. With Disperse on, it comes in once the lines have cleared away.
- Text is drawn in your Lyrics look and comes in with your lyrics animation's In keys.
- Settings for the picture's height, how solid it is, a swell on each beat, fading in and out, and how long before the next line it leaves, so the line reads in on its own.
- **Before the First Line** and **After the Last Line** switches show it there too.
- It's also in the lyrics' own menu, under Instrumental.
- The picture's file stays on this PC: it's never in a shared style or animation. Pictures are read once, and big ones are made smaller (512 px a side at most).

### Lyrics Window stays on this PC

- Lyrics Window describes your desktop, not a look, so it's no longer carried in shared styles.

## ❀ v2.5.9 beta (Windhawk): shares that look like your desktop

This is a beta. It keeps the same mod id, so it updates the 2.5.8 beta in place and keeps its settings.

### New: a shared animation carries your Lyrics look

- **Share mine to the community...** now sends your Lyrics look along with the animation: sung, waiting and other-line colours, font, size, alignment, lines before and after, shadow, glow, bloom, outline, highlight, and the panel and its border. The gallery's preview then shows it the way it looks on your desktop.
- Nothing tied to your PC goes: no folders, files, tokens, keys, sites, screens or window places. A value that looks like a path or a web address is left out.

### New: Take its look too

- **Take the one I copied there** asks what to do when the animation comes with its sharer's look: **Keep my own look** or **Take its look too**. Your own colours and font only change if you pick the second.

## ❀ v2.5.8 beta (Windhawk): sung colours that stick

This is a beta. It keeps the same mod id, so it updates the 2.5.7 beta in place and keeps its settings.

### Fixed

- **karaoke.colors and karaoke.base did nothing in 2.5.7.** An animation file treats `#` as the start of a note, so `#3060FF` was read as a note and the colours were dropped. A `#` followed by a colour (`#rrggbb` or `#rrggbbaa`) now stays part of the value. Notes after a space and `#` work as before.
- **Sung colours... was cut off** at the editor's right edge. The link box is shorter now, so Fetch, Import a pen... and Sung colours... all fit.

## ❀ v2.5.7 beta (Windhawk): sung colours that take turns

This is a beta. It keeps the same mod id, so it updates the 2.5.6 beta in place and keeps its settings.

### New: three Karaoke keys for your lyrics animations

- **karaoke.step**: how a word lights. `sweep` runs through its letters as it's sung (as before), `word` lights the whole word at once, `current` lights only the word being sung while the rest wait.
- **karaoke.colors**: sung colours that take turns word by word, like `#3060FF, #FF3040, #FFD020`. Empty means the Lyrics page's Sung colour.
- **karaoke.base**: the waiting colour while this animation is on. Empty means the Lyrics page's Waiting colour.
- Animations without these keys look exactly as before. The Lyrics page's own colours never change: these belong to the animation.

### New: Sung colours... in the Animation editor

- Next to Import a pen. It lists the animation's sung colours, and each one opens the colour picker. You can add a colour, remove the last, or go back to the Lyrics page's Sung colour.
- **Karaoke step** and **Sung colours** are also rows in the editor's form.

### Import a pen: its colours come with it

- A word-by-word highlight pen now comes in with its colours taking turns, its waiting colour, and a word at a time, so it looks like the pen.
- When a pen brings its own colours, you choose: keep the pen's colours, choose your own with the colour picker, or use the Lyrics page's colours.
- Colours are read from hex, rgb() and common names. Anything else is listed as left out.

## ❀ v2.5.6 beta (Windhawk): pens that light word by word

This is a beta. It keeps the same mod id, so it updates the 2.5.4 beta in place and keeps its settings.

### Fixed: Import a pen asking about every word

- A pen that wraps each word (or each syllable) in a span with its own class no longer asks "Which part holds the text?" with one row per word. Three or more sibling spans like that are the words, and their parent holds the line.
- The question still comes up when a pen really has two or more places for the text, such as a heading and a paragraph, with just those.

### New: word-by-word highlight pens come in as Karaoke

- A pen where every word has keyframes of its own that only change its colour, one after another, now comes in as Karaoke by word, on our engine, with no movement. The words light as they're sung, timed by the song instead of the pen's fixed steps.
- The status line names what didn't come over: the pen's colours taking turns (set the Sung colour to one of them), its instant colour jump (ours sweeps), and its page's font and frame.

## ❀ v2.5.4 beta (Windhawk): lyrics you can see

This is a beta. It keeps the same mod id, so it updates the 2.5.3 beta in place and keeps its settings.

### Fixed: lyrics shown but not seen

- On some desktops the log said the lyrics were found and shown, but nothing appeared. They were drawn as part of Windows' desktop window, and that desktop didn't put them on screen.
- **New: Lyrics > Hiding > Lyrics Window** picks where the lines are drawn. It's also in the Lyrics Menu, under Window.
  - **Automatic** (the default): a see-through window of their own, just above the desktop. Your windows still sit over them, as before.
  - **On the desktop, with the bars**: the old way, drawn as part of the desktop window.
  - **Own window, just above the desktop**: the same as Automatic, chosen by hand.
  - **Always on top**: over everything, windows included.

### Log

- With the mod's logging on, the log now says where the lyrics are each time they appear: their place on screen, which window they're in, whether Windows calls them visible, and the fade and scale. No song names.
- The media strip no longer writes two lines on every repaint, only when its place changes.

## ❀ v2.5.3 beta (Windhawk): lyrics that show up

This is a beta. It keeps the same mod id, so it updates the 2.5.2 beta in place and keeps its settings.

### Fixed: lyrics that never showed

- **Lyrics on their own now read the song.** Before, the Table only asked the player what's playing when the album art was wanted (Color Mode on Album Art, Now Playing Text, or the Media Controls card). With all of those off, turning on Lyrics found nothing to look up. Now Lyrics counts on its own, and the art isn't read when only the lyrics want the song.
- **A desktop that won't draw them.** If Windows won't draw the lyrics on the desktop window, they now get a see-through window of their own just above the desktop (other windows still sit over them), instead of staying invisible.

### New: the lyrics' menu without lyrics on screen

- **Lyrics Menu...** is now in the quick panel (right-click the Table) and in the full menu (Shift+right-click). It opens the same menu the lyrics' own right-click does, at the pointer, so you can reach it when no words are showing. With Lyrics off, **Turn lyrics on** is at its top.

### New: why the lyrics aren't showing, in the log

- With the mod's logging turned on (Windhawk, the mod's Advanced tab), the Table writes one line each time the reason changes: Lyrics is off, no song read from the player yet, no lyrics for this song, the player hasn't said where in the song it is, hidden with the bars, and so on. Only the reasons: no song names in those lines.

### Tidied

- The Tune by ear graph's sums no longer show a note in Windhawk's editor (Problems list).

## ❀ v2.5.2 beta (Windhawk): the Animation editor in reach, and lyrics that turn

This is a beta. It keeps the same mod id, so it updates the 2.5.1 beta in place and keeps its settings.

### New: the Animation editor from the Table's own menus

- **Lyrics Animation Editor...** is now in the quick panel (right-click the Table) and in the full menu (Shift+right-click), not only in the lyrics' own menu. It opens even with Lyrics off, so you can make an animation first.

### New: lyrics turn, stretch and take in-between stops

- **in.turn** and **out.turn** (degrees), **in.sx** / **in.sy** and **out.sx** / **out.sy** (width and height apart): where a word or letter comes in from, and where the line goes.
- **in.stop** and **out.stop** lines (at x y width height turn opacity, at in per cent of the time): the whole way in or out, with as many stops between as you like.
- **Paste keyframes** and **Import a pen** keep a pen's turning (rotate), stretch (scaleX, scaleY) and in-between stops (40%, 60%...) instead of listing them as left out.
- Animations that don't use these keys look exactly as before.

## ❀ v2.5.1 beta (Windhawk): Reset Positions, your way

This is a beta. It keeps 2.5.0's mod id, so it updates the 2.5.0 beta in place and keeps its settings. 2.5.1 has 481 settings: 1 new.

### New: Reset Positions Do

- **Reset Positions Do** (Interaction): what Reset Positions does. **Back in place on its own screen** (the new default) puts each piece where its settings place it, on the screen it's on now, so resetting a piece on your second screen leaves it there. **Gather on the main screen** is what Reset Positions did before: the visualizer and the media strip or card centred side by side on your main screen, for when something is lost on a far or very large screen.
- **The other one is a Shift away**: Shift+click any Reset Positions item in the right-click menu or the quick panel. Each item says what a click does and what Shift does.
- **The keys, listed at last**: the move keys + Home reset the piece you're moving, as Reset Positions Do says, and the move keys + Shift+Home do it the other way. They're named in the Reset Positions menu and in the tips. (If Shift is one of your move keys, the keys + Home do it as set; use Shift+click in the menu for the other way.)

## ❀ v2.5.0 beta (Windhawk): everything since 2.4.0

This is a beta. It has a mod id of its own (`tourne-table-2-5-beta`), so it installs beside the 2.4.0 beta instead of over it; the 2.4.0 beta and its settings stay as they are, and 2.5.0 starts with its own settings. Every new option below starts off, or at a neutral value, so nothing changes until you turn it on. 2.4.0 had 288 settings; 2.5.0 has 480: 194 new, and 2 gone (Panel Colours and Panel Band, see the end). No default of a setting both builds have has changed.

### New: Glass Frame (48 settings)

- **A new shape, Glass Frame**: a liquid glass loop round your screen's edges that moves with the music. Pick it from the Shape list or the right-click menu, with or without the panel behind it.
- **Light**: glow, highlight and edge line colours, line width, light angle, fringe, frost, tint, opacity and a see-through bend (the picture behind is bent only where the glass curves).
- **Motion**: Height, Thickness, Inset, Columns, Smoothness (the levels glide between analysis updates), Rise Easing (a gentle two-step rise), Fill In Between (the edge moves on every frame), Dynamics (levels read against their own average, so hot settings don't pin the loop), and Mirror.
- **From the bars**: Peak Line, Colour Mode, Afterimage and Shadow, cast from the real edge and only where the music lifts it.
- **Beat Flash and Bass Glow inside the glass**, with the flash's own colour, fade and fill.
- **Necks**: the glass parts round the mouse, windows, taskbar islands, the Table's own pieces and the Depth cut-out, sloping down to the band instead of cutting a wall. Swell (with Auto Swell, sized to each object), Swell Fade, Swell Blend, Neck Gap, Softness, Speed and Tuck.
- **Sides**: hide the band, its line, glow and collars on any side. The surface stops at its own screen's edges.
- With the Glass Frame on, the other parts keep their usual places (Now Playing, progress, the bass lane, the readout, an anchored media strip).

### New: Lyrics (off by default)

- **Finding**: your own folder of .lrc files first, then a cache, then (only if you allow it) lrclib.net. More sources, each its own switch: lrclib's looser search, NetEase, Genius (with your own token), your own site, and Last.fm name fixing. Untimed lyrics can be spread over the song or held.
- **Timing**: LRC with word times, an offset, Pace With the Song, and Fill Follows the Singer (the sung colour moves word by word, from heard word starts on line-timed lyrics) with Fill Lead.
- **On the desktop**: the lines with their own animations (Rise, Fade, Typewriter, Karaoke, Wave, Drop, Prompt) and letter styles (Flip, Decode, Drop, Ripple, Split, Type, Focus, Split-flap), or an animation file of your own.
- **Animation editor**: a list, a form kept in step with the text, a live preview with a made-up song, Find animations, and a Community button to browse and share.
- **Import a pen**: drop a CodePen pen's Export zip (or its .css and .html) on the Animation editor, or use Import a pen... Nothing goes online and nothing is unpacked to disk. The keyframes on the pen's text (or the one you pick from a list) become the animation's own keys, and the pen's address, author and licence go in its source line (public CodePen pens are MIT by default, but check the pen's own note). The pen's markup is kept in the file too, for my desktop app's web engine; here the keys draw it. A pen whose effect lives only in its JavaScript can't come in, and says so.
- **From a link**: paste an https link to a .css file and press Fetch. Only that one file is fetched, 512 KB at most, with no cookies and no redirects; a sign-in or bot-check page is refused. CodePen itself isn't fetched (it turns tools away), so use its Export there. Animation Link Timeout (s) sets how long it waits.
- **Look**: font, size, colours (sung, waiting, others), shadow, glow, bloom, outline, highlight, beat flash, a background with its own corner, padding and border, width and alignment.
- **Word Pop**: each word pops with its loudness and the syllable tempo, with its own strength, motion, fade and scale rows.
- **Beat Pop** with Pop Direction (In presses the line toward the desktop) and a choice of look, linked to the bars' or the Glass Frame's beat if you like.
- **Depth**: the lyrics can pass behind the wallpaper's cut-out subject.
- **Instrumental Disperse**: when the song goes instrumental, the lines clear away with a close effect (Wake, Derez, Shatter, Melt, Dust, Motion).
- **Hiding**: with the bars, for fullscreen, when covered, or in silence. Lock in place, drag on the text, and their own right-click menu.

### New: Translation of the lyrics (off by default)

- **From people first**: your own files ("Artist - Title.<language>.lrc"), NetEase's translated lyrics and Genius's translation pages (with your token). Under each line, above it, in place of it, or in a block of its own you can move and lock.
- **Machine translation on your PC** (64-bit only): small OPUS-MT models, one folder per language pair or family, in the Translation Models Folder, run by ONNX Runtime from the ONNX Runtime Folder. Into English only. The language is decided line by line, so a song that's part English keeps its English lines as sung. It never goes online, and it's tried only after the human sources.
- **Translation by listening** (64-bit only): for timed lyrics nothing else could translate, what's sung is heard from the sound the Table already takes and put into English by whisper.cpp, a few seconds behind. Needs whisper.dll and a many-language model in the Listening Models Folder.
- **Machine Translation Note**: a small "Machine translation" line under the lyrics, with its own switch. If a model is there but can't run, it says so instead of staying silent.
- **Translation Drawn: Like the lyrics**: the translation takes the lyrics' animation, glow, beat pop and flash, sung colour and Depth, in every place, its own block included. Each part can differ: Translation Animation, Glow, Beat Pop and Flash, Sung Colour (sweeps over the line, or the whole line at once) and Depth (or always in front of the cut-out). Has Its Own Look keeps its font, colour and opacity. Plain brings back the simple fade.

You bring the models yourself: an ONNX Runtime 64-bit `onnxruntime.dll` (1.17 or newer), OPUS-MT models in ONNX form (for example from Hugging Face's Xenova/opus-mt-* pages), and for listening a `whisper.dll` built from whisper.cpp with a `ggml-small.bin` or similar many-language model. Nothing is bundled.

### New: Audio Source and sound

- **Outputs and Inputs lists** on the settings page, offering your devices by name (Windhawk 2), with Extra Inputs Only While Live.
- **Source Boost**: how strongly each device or program moves the bars (up to 1000%), its sound untouched. Programs by name capture only their own sound. Picks in the right-click panel.
- **Source Shaping**: hiss, rumble, quiet, medium and loud amounts, for every source or per source, with their edges.
- **Tune by ear**: sweep a tone, mark the bumps, and turn them into bands for the bars, Source Shaping edges, or parametric EQ text. Tone Glide, Fade and level rows.

### New: Media strip and Now Playing

- **Strip tooltips for Spotify** (off): hover Previous for the song played before, Play/Pause for the song playing with a live time-left countdown, and Next for the next song in Spotify's queue (read from Spotify's window while its queue panel is open). Styled like the right-click menu's panels, above the strip, kept on screen at the edges. Delay, Gap and Padding rows.
- **Now Playing Backdrop**: Solid, Clear, Blur, Acrylic, Mica, Tabbed or Glass.
- **Depth over the Media Card**.

### Look, menus and moving things

- **Lock everything in place**: no drag, resize or Keyboard Move.
- **Reset Positions**: one piece, or everything gathered on the main screen.
- The quick-settings panel: a number drag keeps to its own setting, colour rows keep their alpha, every option of a choice explained in its tip.
- **A tooltip on every right-click row**: actions, groups, sub-pages, styles, inputs and players included. The newer settings' tips now say their units and give a "Try" value.

### Styles across builds

- **A style knows its build**: exporting writes the build it came from. Importing an older style fills any setting it leaves out with that build's own default, not today's, so it looks the way it did. If a setting is ever renamed, older styles still load it, and a style from a newer build loads what it can, with the log naming the settings this build doesn't have.
- **Source Shaping and Tune by ear travel in styles** now.
- **What never travels**: tokens and keys, folders, files and paths, site addresses and anything with an %ENVIRONMENT% name stay on your PC when you export or share a style. A lone % (as in a ramp of characters) is fine.

### Performance

- **Compiled shaders are kept** in `%LOCALAPPDATA%\Tourne\Table\ShaderCache`, so after the first start the Direct3D renderer is ready at once (the first start still compiles them, about a minute on a busy PC).
- The analysis engine is rebuilt only when its settings change, so changing a setting no longer blanks the bars.
- The Glass Frame's peak lookup is lighter (its shader compiles in seconds, not a minute).
- Translation and listening run on their own threads, use two or three cores at most, and cost nothing while nothing needs them.

### Fixes

- Sizes, places, times and amounts that take decimals keep them (71 settings, among them the lyrics' place, padding and auto-hide, Source Shaping, Word Pop and the strip tooltips): they're stored as text, as the older size settings are, so 78.7% stays 78.7% instead of becoming 79. A whole number saved by an earlier build still reads the same.
- The quick panel's colour reads no longer leak.
- The media strip is never raised over the Table's own open menus.
- The Studio's Audio Source and Inputs rows no longer show each other's devices.
- 32-bit builds compile again; 0 errors and 0 warnings on x86, x64 and ARM64.

### Not in the Windhawk build

- **Panel Glass**: it lays the menu glass of my desktop app over the panel, which Windhawk doesn't have.
- **Panel Colours and Panel Band**: picking my desktop app's menu colours for the right-click panel, and the frame band round it, belong to my desktop app. Here the panel always uses its own colour rows (Panel Ground, Text, Accent and the rest), which start neutral.
- **Show Over Living Wallpapers** and **Show While Stepped Away**: they answer my desktop app's living wallpaper and Step away.
- **The lyrics' web engine**: it needs WebView2's SDK header, which Windhawk's compiler doesn't have. The built-in animations all work, and an imported pen runs on its keys.
- **Scripts in your own lyric animations**: they run only in that web engine, locked down, so the Allow Scripts settings aren't here and a pen's script is never taken.
- **The Glass Frame bending a living wallpaper as drawn**: that's my desktop app's living wallpaper. Here the glass bends the Windows wallpaper, as before.
- **Machine translation and listening on 32-bit Windows**: ONNX Runtime and whisper.cpp ship 64-bit only.
- **Lyrics settings... and Every look setting...** in the lyrics menu: they open my desktop app's settings, so they don't show here.

