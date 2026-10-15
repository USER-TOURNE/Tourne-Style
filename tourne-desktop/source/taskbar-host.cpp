// ==WindhawkMod==
// @id              tourne-taskbar
// @name            Tourne'Taskbar [test]
// @description     A classic taskbar drawn from StartAllBack themes, in test mode: a second bar next to the real one
// @version         0.3.0
// @author          USER-TOURNE
// @github          https://github.com/USER-TOURNE
// @include         windhawk.exe
// @architecture    x86-64
// @compilerOptions -luxtheme -ldwmapi -lole32 -lshell32 -lgdi32 -lmsimg32 -luser32 -lcomctl32
// @license         MIT
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# `Tourne'Taskbar` **[test]**

Replacing StartAllBack: our own classic taskbar, drawn straight from a
StartAllBack theme (`.msstyles`) through Windows' own theme engine, so the
themes people already use keep working, with or without StartAllBack
installed. Or no theme at all: a flat bar in any of the colour schemes.

Themes and orbs are read from StartAllBack's folders if it's installed, and
from `%LOCALAPPDATA%\Tourne\Styles` and `%LOCALAPPDATA%\Tourne\Orbs`: drop a
theme's `.msstyles` or orb there and it shows up in the dropdowns.

In test mode it's a second bar beside the real taskbar, which StartAllBack
keeps running. Its settings follow StartAllBack's Taskbar page.

Not yet: progress bars, and recent and frequent items in jump lists.
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- preset: own
  $name: Where do you want to go today?
  $description: >-
    Select theme to apply, as StartAllBack's Welcome page does: it sets the
    Start menu's and taskbar's visual style, the Start orb, the corner icons'
    flyouts and Explorer's command bar and details pane. While one is chosen
    it decides those; choose "Your own" to set them separately below.
  $options:
  - own: Your own (the settings below)
  - proper11: >-
      Proper 11 - Round Start menu and taskbar elements - Corner taskbar icons
      open Win11 flyouts if possible - New Command Bar in Windows Explorer
  - kinda10: >-
      Kinda 10 - Square Start Menu and taskbar elements - Corner taskbar icons
      open Win10 flyouts if possible - Ribbon UI in Windows Explorer
  - remastered7: >-
      Remastered 7 - Aero Start Menu and taskbar style - Corner taskbar icons
      open Win7 flyouts if possible - Simple UI and details on bottom in
      Windows Explorer
- hideSystemTaskbar: false
  $name: Hide StartAllBack's taskbar
  $description: >-
    Makes this bar the only one: the taskbar StartAllBack draws (Explorer's,
    or Windows' own without StartAllBack) is hidden and set to auto-hide, so
    it gives its space back. From inside Explorer it's also cloaked, so it
    can't flash back for a notification or the pointer at the screen's edge.
    Its tray keeps working behind the scenes, and turning this or the
    Taskbar part off (or this bar closing for any reason) brings it back as
    it was.
- noSystemTaskbarContent: false
  $name: Don't load Windows' taskbar (test)
  $description: >-
    With "Hide StartAllBack's taskbar" and StartAllBack off: Windows 11's
    taskbar is made without its buttons, tray and progress bars, as
    StartAllBack does, so none of it runs hidden. Takes effect when Explorer
    next starts; turning it off needs one more Explorer restart to bring
    Windows' taskbar back.
- multiMonitor: windows
  $name: A bar on every screen
  $description: >-
    Every other screen gets a bar too, with the buttons of its windows and the
    clock. Which windows show where follows Windows' "Show taskbar buttons on".
    "As Windows is set" follows its "Show my taskbar on all displays".
  $options:
  - windows: As Windows is set
  - always: Always
  - never: Never
- disableStartAllBack: false
  $name: Turn StartAllBack off
  $description: >-
    Switches StartAllBack off for your account (its own "Disable program for
    current user") and restarts Explorer, so it no longer loads at all; this
    bar's tray takes over. Turning this off switches it back on and restarts
    Explorer again. With StartAllBack off Windows' own taskbar returns, so
    use it with "Hide StartAllBack's taskbar".
- style: startallback
  $name: Visual style
  $description: >-
    A StartAllBack theme, from StartAllBack's Styles folder or
    %LOCALAPPDATA%\Tourne\Styles; the one StartAllBack is set to; or none, a
    flat bar in the colour scheme.
  #! $dynamicSelect: true
- yasbTheme: none
  $name: YASB theme !WIP
  $description: >-
    Work in progress: your own CSS can't be added to a theme yet, and themes
    may not look as they do in YASB.
    Draws the bar from a YASB theme (its styles.css and config.yaml, in a folder
    under %LOCALAPPDATA%\Tourne\Yasb): the bar's look, its widgets and where they
    go. Task buttons, the tray, the clock and the Start button stay ours, in the
    theme's boxes. Drop a theme's folder or .zip on the bar to install it. None:
    the normal bar.
  #! $dynamicSelect: true
- yasbFill: true
  $name: Add buttons and tray to YASB themes without them !WIP
  $description: >-
    Most YASB themes are status bars with no task buttons or tray. On: they get
    ours, at the left and the right end, so the bar still works as a taskbar.
- yasbCommands: false
  $name: Let YASB themes run commands !WIP
  $description: >-
    A theme's buttons can name commands to run when clicked ("exec ..."). Off:
    only the Start menu, the power menu and Windows settings pages open. On: any
    command a theme names runs when you click it, so only turn it on for themes
    you trust. It also lets its custom widgets run their command on a timer.
- yasbNetwork: false
  $name: Let YASB widgets use the internet !WIP
  $description: >-
    Off: nothing here contacts anyone. On: the weather widget asks
    weatherapi.com, with the API key and the place named in the theme's own
    config.yaml, every ten minutes or more.
- yasbFades: true
  $name: Fade YASB theme hovers !WIP
  $description: >-
    Follow a theme's own "transition" lines, so a hovered widget fades to its
    hover colours over the time the theme names. (YASB itself doesn't animate
    these, Qt ignores them, so off is the same as in YASB.)
- orb: startallback
  $name: Start orb
  $description: >-
    An orb from StartAllBack's Orbs folders or %LOCALAPPDATA%\Tourne\Orbs, or
    the one StartAllBack is set to.
  #! $dynamicSelect: true
- scheme: matchIcons
  $name: Colour scheme
  $description: >-
    For the flat bar (visual style: none), and the colours drawn on top of a
    theme, like the segment edges.
  $options:
  - matchIcons: Same as System Icons
  - taskbarStyle: From the visual style above
  - visualStyle: From Windows' visual style
  - tourneTheme: Tourne theme (Tourne Desktop's, from your wallpaper)
  - tourne: Tourne
  - gruvboxDarkHard: Gruvbox dark (hard)
  - gruvboxDark: Gruvbox dark
  - gruvboxDarkSoft: Gruvbox dark (soft)
  - gruvboxLight: Gruvbox light
  - monokai: Monokai
  - catppuccinMocha: Catppuccin Mocha
  - catppuccinMacchiato: Catppuccin Macchiato
  - catppuccinFrappe: Catppuccin Frappé
  - catppuccinLatte: Catppuccin Latte
  - everforestDarkHard: Everforest dark (hard)
  - everforestDark: Everforest dark
  - everforestDarkSoft: Everforest dark (soft)
  - everforestLightHard: Everforest light (hard)
  - everforestLight: Everforest light
  - everforestLightSoft: Everforest light (soft)
  - nord: Nord
  - dracula: Dracula
  - tokyoNight: Tokyo Night
  - tokyoNightStorm: Tokyo Night Storm
  - rosePine: Rosé Pine
  - rosePineMoon: Rosé Pine Moon
  - rosePineDawn: Rosé Pine Dawn
  - kanagawa: Kanagawa
  - oneDark: One Dark
  - solarizedDark: Solarized dark
  - solarizedLight: Solarized light
- location: top
  $name: Taskbar location on screen
  $description: >-
    In test mode StartAllBack's taskbar stays where it is, so top keeps the
    two apart.
  $options:
  - top: Top
  - bottom: Bottom
  - left: Left
  - right: Right
- startPosition: edge
  $name: Start button position
  $options:
  - edge: At the start of the bar
  - withTasks: Next to the task buttons
  - end: At the end of the bar
- center: centered
  $name: Task buttons
  $options:
  - centered: Centered
  - edge: From the start button
- combine: always
  $name: Combine taskbar buttons
  $options:
  - always: Always, hide labels
  - whenFull: When taskbar is full
  - never: Never
  - neverHideLabels: Never, hide labels
  - neverGroup: Never group
- iconSize: small
  $name: Icon size
  $options:
  - xsmall: XS
  - small: S
  - medium: M
  - large: L
  - xlarge: XL
- iconMargins: small
  $name: Icon margins
  $options:
  - xsmall: XS
  - small: S
  - medium: M
  - large: L
  - xlarge: XL
- thickness: 40
  $name: Thickness (px)
  $description: How tall the bar is (how wide, on the left or right), at 100% scaling.
  #! $min: 20
  #! $max: 96
- segments: none
  $name: Segments
  $description: >-
    Splits the bar into floating islands, with the desktop showing (and
    clickable) between them: the Start button, the task buttons and the tray.
    "Just central segment" leaves the Start button floating on its own.
  $options:
  - none: Off
  - three: Up to three segments
  - central: Just central segment
- autoHide: false
  $name: Automatic hiding
  $description: The bar slides away and comes back when the pointer reaches its edge of the screen.
- dragRoom: 240
  $name: Room made for drops (pixels)
  $description: >-
    A file held still for half a second at a screen's edge, or in the gap between
    two windows, slides the windows beside it back this far so it can be dropped on
    the desktop; they grow back after the drop. 0: off.
- emptyDoubleClick: show-desktop
  $name: Double-click on the bar's empty space
  $description: What a double-click on the bar where there's nothing (or between its segments) does. The same actions as the Tourne Menu's.
  $options:
  - show-desktop: Show the desktop (or bring the windows back)
  - start: Open Start
  - run: Run...
  - task-manager: Task Manager
  - studio: Tourne Studio
  - menu-settings: The Tourne Menu's settings
  - settings: Windows Settings
  - launcher: Open the launcher
  - desktop-next: Next desktop
  - desktop-previous: Previous desktop
  - desktop-new: New desktop
  - lock: Lock
  - sleep: Sleep
  - none: Nothing
- emptyMiddleClick: none
  $name: Middle click on the bar's empty space
  $description: The same actions as the double-click. A middle click on a task button still closes it. With a double middle click set too, a single one waits a moment to be sure it isn't one.
  $options:
  - none: Nothing
  - show-desktop: Show the desktop (or bring the windows back)
  - start: Open Start
  - run: Run...
  - task-manager: Task Manager
  - studio: Tourne Studio
  - menu-settings: The Tourne Menu's settings
  - settings: Windows Settings
  - launcher: Open the launcher
  - desktop-next: Next desktop
  - desktop-previous: Previous desktop
  - desktop-new: New desktop
  - lock: Lock
  - sleep: Sleep
- emptyMiddleDoubleClick: none
  $name: Double middle click on the bar's empty space
  $description: The same actions again, for two middle clicks in quick succession.
  $options:
  - none: Nothing
  - show-desktop: Show the desktop (or bring the windows back)
  - start: Open Start
  - run: Run...
  - task-manager: Task Manager
  - studio: Tourne Studio
  - menu-settings: The Tourne Menu's settings
  - settings: Windows Settings
  - launcher: Open the launcher
  - desktop-next: Next desktop
  - desktop-previous: Previous desktop
  - desktop-new: New desktop
  - lock: Lock
  - sleep: Sleep
- dragHold: 250
  $name: Drop room wait (ms)
  $description: How long a drag has to rest in a gap or at a screen's edge before the windows make room. Passing through faster doesn't count.
- dragGapPad: 4
  $name: Drop room gap padding (pixels)
  $description: How far off a gap between two windows the pointer can be and still count as in it.
- dynamicShyness: false
  $name: Dynamic shyness
  $description: >-
    Stay in view, and only slide away while a window touches the bar (it comes
    back when the pointer reaches the edge). Works with or without automatic
    hiding.
- shyTolerance: 8
  $name: Shyness tolerance (px)
  $description: >-
    How far a window may reach into the bar before it counts as touching it.
    Raise it if windows placed near the bar keep hiding it.
  #! $min: 0
  #! $max: 200
- shyAnimation: slide
  $name: Hiding animation
  $description: >-
    How the bar goes away and comes back (dynamic shyness and automatic
    hiding). Windows moves and fades the finished bar itself, in step with the
    screen's refresh, so the bar isn't drawn again for it.
  $options:
  - slide: Slide
  - fade: Fade
  - slidefade: Slide and fade
  - none: None (at once)
- shySmoothing: smooth
  $name: Smoothing
  $description: How the animation eases in and out.
  $options:
  - smooth: Smooth (eases in and out)
  - soft: Soft landing (quick start, slow finish)
  - gentle: Gentle (light easing)
  - off: Off (even speed)
- shyDuration: 220
  $name: Animation length (ms)
  #! $min: 0
  #! $max: 1000
- shyHideDelay: 600
  $name: Hide delay (ms)
  $description: >-
    How long a window has to touch the bar (or, with automatic hiding, the
    pointer be away from it) before the bar goes.
  #! $min: 0
  #! $max: 5000
- shyShowDelay: 250
  $name: Return delay (ms)
  $description: >-
    With dynamic shyness: how long windows have to stay clear of the bar before
    it comes back, so a window dragged past doesn't make it flicker. Nothing
    changes while a window is being moved or sized; the pointer at the screen's
    edge still brings it back at once.
  #! $min: 0
  #! $max: 5000
- shyEdge: bar
  $name: While hidden
  $description: What stays at the screen's edge while the bar is away. It never takes clicks.
  $options:
  - bar: A sliver of the bar (StartAllBack)
  - glow: A soft glow in the accent colour
  - none: Nothing
- shyGlow: 60
  $name: Glow strength (%)
  #! $min: 5
  #! $max: 100
  #! $showIf: {shyEdge: glow}
- shyGlowSize: 4
  $name: Glow depth (px)
  #! $min: 2
  #! $max: 12
  #! $showIf: {shyEdge: glow}
- coloring: startallback
  $name: Custom taskbar coloring
  $description: >-
    Tints the bar's background (the buttons and icons stay as they are). With
    StartAllBack's setting it follows StartAllBack's own taskbar coloring;
    Off keeps the style's or scheme's background as it is.
  $options:
  - startallback: Same as StartAllBack
  - none: Off
  - custom: Custom
- barBackground: startallback
  $name: Bar background
  $description: >-
    What shows behind a style's buttons where the style leaves the bar clear:
    StartAllBack's own bar look (the gradient it draws), or the colour scheme's
    ground.
  $options:
  - startallback: StartAllBack's
  - scheme: The colour scheme's
- coloringColor: '121C21'
  $name: Taskbar color
  #! $format: colorRgb
  #! $showIf: {coloring: custom}
- coloringAlpha: 255
  $name: Taskbar opacity
  $description: 0 shows only what is behind the bar, 255 is solid.
  #! $format: slider
  #! $min: 0
  #! $max: 255
  #! $showIf: {coloring: custom}
- coloringBlur: none
  $name: Taskbar blur
  $description: Blurs what shows through a see-through bar. Not used with segments.
  $options:
  - none: Off
  - blur: Blur
  - acrylic: Acrylic
  #! $showIf: {coloring: custom}
- allAppIcons: false
  $name: Always show all app icons
  $description: >-
    Every app's tray icon starts in the tray, none in the hidden icons flyout
    (StartAllBack's setting of the same name). Icons you have placed yourself
    stay where you put them.
- trayArrow: tourne
  $name: Overflow arrow
  $description: >-
    The arrow that opens the hidden icons: System Icons' own (the Tourne pixel
    arrow), or the visual style's, when it has one.
  $options:
  - tourne: System Icons' own
  - style: The visual style's
- dynamicTransparency: false
  $name: Dynamic transparency
  $description: The bar's background clears while no window touches it, and fills in when one does.
- dynamicSegments: false
  $name: Dynamic segments
  $description: Segments join into one bar while a window touches it.
- plumpness: none
  $name: Plumpness
  $description: >-
    Buffer and Aura give the bar StartAllBack's taller height (at least 38 px). Aura is the Windows 7 glow:
    a button lights up in the main colour of its app's icon.
  $options:
  - none: Off
  - buffer: Buffer
  - aura: Aura
- dynamicAura: false
  $name: Dynamic aura
  $description: The aura glow follows the pointer along the button instead of sitting in its middle.
- auraFollow: 0
  $name: Dynamic aura's trail (ms)
  $description: >-
    How the dynamic aura catches up with the pointer: 0 keeps it right under
    the pointer; higher lets it trail behind softly (the time it takes to
    close half the gap). It moves once per screen refresh either way.
  #! $min: 0
  #! $max: 400
  #! $showIf: {dynamicAura: true}
- auraSize: 100
  $name: Aura size (%)
  $description: How far the Aura glow reaches, against the button's size.
  #! $min: 40
  #! $max: 300
  #! $showIf: {plumpness: aura}
- auraStrength: 100
  $name: Aura strength (%)
  #! $min: 0
  #! $max: 300
  #! $showIf: {plumpness: aura}
- shadows: none
  $name: Shadows
  $description: >-
    A soft shadow under the task buttons' icons, their frames, or both. It
    falls on whatever is below, the bar's own colour included. Each shape's
    shadow is worked out once and kept.
  $options:
  - none: None
  - icons: Under icons
  - frames: Under button frames
  - both: Under icons and frames
- shadowColor: "000000"
  $name: Shadow colour
  #! $format: colorRgb
  #! $showIf: {shadows: [icons, frames, both]}
- shadowStrength: 50
  $name: Shadow strength (%)
  #! $format: slider
  #! $min: 0
  #! $max: 100
  #! $showIf: {shadows: [icons, frames, both]}
- shadowSoftness: 2
  $name: Shadow softness (px)
  $description: How far it spreads; 0 is a hard copy of the shape.
  #! $min: 0
  #! $max: 8
  #! $showIf: {shadows: [icons, frames, both]}
- shadowX: 1
  $name: Shadow offset, right (px)
  $description: Below 0, to the left.
  #! $min: -8
  #! $max: 8
  #! $showIf: {shadows: [icons, frames, both]}
- shadowY: 1
  $name: Shadow offset, down (px)
  $description: Below 0, up.
  #! $min: -8
  #! $max: 8
  #! $showIf: {shadows: [icons, frames, both]}
- tooltipPlace: pointer
  $name: Tooltips
  $description: >-
    Where the bar's tooltips show: by the pointer, where Windows puts them
    (over the next buttons, and where previews open), or beside the bar over
    what's pointed at, as Windows 7 had them. A tooltip closes when a preview
    opens either way.
  $options:
  - pointer: By the pointer
  - bar: Beside the bar
- tooltipAway: 0
  $name: Tooltips, further from the bar (px)
  $description: >-
    Moves tooltips away from the bar (or towards it, below 0), whichever
    side of the screen it's on.
  #! $min: -200
  #! $max: 400
- tooltipAlong: 0
  $name: Tooltips, along the bar (px)
  $description: >-
    Moves tooltips along the bar: right on a bar at the top or bottom, down
    on one at the side; below 0 the other way.
  #! $min: -400
  #! $max: 400
- menuLook: theme
  $name: Menus
  $description: >-
    Every menu the taskbar, Start menu and system icons open. "Same as
    Windows' menus" draws them the way Windows draws other programs' classic
    menus on this PC: its menu colours and menu font (Control Panel's colours),
    flat and square, with System Text Color's colour over the text when that
    part is on. Menu No Icons still decides about icons.
  $options:
  - theme: The visual style and colour scheme (as set for each)
  - windows: Same as Windows' menus
- menuFrameGap: 6
  $name: Menu frame gap
  $description: >-
    With StartAllBack's menu look (a line round each menu, a band, a second
    line), the band's width in pixels, 1 to 16. StartAllBack's own is 6.
- menuFrameColors: ""
  $name: Menu frame colours
  $description: 'The two lines'' colour, or two or three colours for a gradient from the top left to the bottom right, for example #1e3632 #152026 #121c21. Empty: the style''s own.'
- menuFrameGlow: 0
  $name: Menu frame glow
  $description: A soft light round the lines, 0 (none) to 100.
- menuFlyoutClick: false
  $name: Submenus open on a click
  $description: >-
    Right-click menus: submenus (Send to, 7-Zip, View...) open when clicked
    (or with Right or Enter), not as the pointer passes over them, so
    sweeping across a menu never stops to open one. Here and, with Tourne
    Menus, in Explorer. The Start menu keeps its own setting ("Open submenus
    when I pause on them").
- menuAnimation: none
  $name: Menus open
  $description: >-
    How every menu here opens. At once is the quickest. The others take their
    curve and length from the motion engine's "popups" line (motion.popups,
    or its parents), else 0.15 s easing out. Tourne Menus uses the same in
    Explorer.
  $options:
  - none: At once (no animation)
  - fade: Fade in
  - slide: Slide in and fade
  - unfold: Unfold
- menuHide: ""
  $name: Leave out of right-click menus
  $description: >-
    Labels or verbs, separated by commas, taken out of the shell's menus for
    files, folders and the desktop (here and, with Tourne Menus, in
    Explorer), for example: Cast to device, Give access to, Add to Favorites.
    Case and a trailing "..." don't matter; "Paste (greyed)" takes an item
    out only while it's greyed out.
- menuMore: ""
  $name: Move into More options
  $description: >-
    Labels or verbs, separated by commas, moved from those menus into a
    submenu at their end, for example: Pin to Start, Restore previous
    versions, Open in Terminal.
- menuMoreLabel: More options
  $name: The More options submenu's name
- desktopMenuReady: false
  $name: Desktop menu ready ahead
  $description: >-
    Tourne Desktop's desktop menu (Tourne's look): the empty space's menu is
    kept built between right-clicks, a file's menu starts building as the
    button goes down, and a few seconds after starting, one menu for each
    kind of file on the desktop is built and thrown away in the background,
    so their extensions are loaded before the first right-click.
- jumpListPlace: pointer
  $name: Jump lists and window menus open
  $description: >-
    A button's right-click menu: at the pointer, or beside the bar lined up
    with the button (its start, or its centre), clear of the button. Its
    previews and tooltip close first either way.
  $options:
  - pointer: At the pointer
  - start: Beside the bar, from the button's start
  - centre: Beside the bar, centred on the button
- jumpListAway: 0
  $name: Jump lists, further from the bar (px)
  $description: Beside the bar only. Below 0, over the bar.
  #! $min: -200
  #! $max: 400
- jumpListAlong: 0
  $name: Jump lists, along the bar (px)
  $description: >-
    Beside the bar only: right on a bar at the top or bottom, down on one at
    the side; below 0 the other way.
  #! $min: -800
  #! $max: 800
- tooltipDelay: 0
  $name: Tooltip delay (ms)
  $description: How long the pointer rests before a tooltip shows. 0 is Windows' own.
  #! $min: 0
  #! $max: 5000
- shyRevealZone: 0
  $name: Reveal zone (px)
  $description: >-
    How close to the screen's edge the pointer has to come to bring a hidden
    bar back. 0 is the edge itself (the sliver); more lets it come back sooner.
  #! $min: 0
  #! $max: 200
- shyRevealAttention: false
  $name: Come back for a window wanting attention
  $description: >-
    A hidden bar comes back while a window's button flashes, and stays until
    it stops (or you look at it).
- shyRevealStart: false
  $name: Come back with the Start menu
  $description: A hidden bar comes back while the suite's Start menu is open.
- fullscreenStayAway: true
  $name: Stay hidden over full-screen programs
  $description: >-
    While a game, a video or any window filling the whole screen is in front,
    the bar on that screen stays hidden, even with the pointer at the edge, and
    drops behind it if it doesn't hide. Windows' own taskbar does the same.
    Works for exclusive and borderless full screen.
- fullscreenPrograms: [""]
  $name: Also stay hidden for these programs
  $description: >-
    Program names (like game.exe) the bar keeps out of the way of whenever
    they're in front, full screen or not: for games played in a window.
- idleFade: false
  $name: Fade when idle
  $description: >-
    The bar fades to a lower opacity once the pointer has been away from it for
    a while, and comes back up when it returns (or comes near, as set). Only
    Windows' compositor changes: the bar isn't drawn again for it.
- idleAfter: 5
  $name: Idle after (s)
  #! $min: 0
  #! $max: 600
  #! $showIf: {idleFade: true}
- idleOpacity: 40
  $name: Idle opacity (%)
  #! $format: slider
  #! $min: 0
  #! $max: 100
  #! $showIf: {idleFade: true}
- idleWhen: always
  $name: Idle fading applies
  $description: Always, or only while no window touches the bar, or only while one does.
  $options:
  - always: Always
  - clear: Only while nothing touches the bar
  - touching: Only while a window touches the bar
  #! $showIf: {idleFade: true}
- idleNear: 60
  $name: Wake as the pointer nears (px)
  $description: >-
    Within this distance of the bar the opacity rises the closer the pointer
    comes, so it's back by the time you reach it. 0 wakes only on the bar.
  #! $min: 0
  #! $max: 400
  #! $showIf: {idleFade: true}
- idleFadeTime: 600
  $name: Fade out over (ms)
  #! $min: 0
  #! $max: 5000
  #! $showIf: {idleFade: true}
- idleWakeTime: 120
  $name: Wake over (ms)
  #! $min: 0
  #! $max: 2000
  #! $showIf: {idleFade: true}
- idleWakeAttention: true
  $name: Wake for a window wanting attention
  #! $showIf: {idleFade: true}
- hideHotkey: ''
  $name: Hide/show hotkey
  $description: >-
    Toggles automatic hiding (StartAllBack uses Win+Alt+T). None until you set
    one. Once set, the suite takes that key press for itself: the program in
    front never hears it, so any shortcut it had on the same keys silently
    stops working. Pick a combination with two modifiers (like Ctrl+Alt or
    Win+Alt) that nothing else you use takes; with just Ctrl, Ctrl+S could
    stop saving without any sign.
  #! $format: hotkey
- previews: thumbnails
  $name: Window previews
  $description: >-
    Hovering a running app's button shows its windows, live, to pick one or
    close it. A list of titles is used when they don't all fit.
  $options:
  - thumbnails: Thumbnails
  - off: Off
- previewDelay: 400
  $name: Preview delay (ms)
  $description: How long the pointer rests on a button before its previews show; 0 is instant.
  #! $min: 0
  #! $max: 2000
- previewOpen: hover
  $name: Previews open
  $description: >-
    What brings a button's previews up. A click on a group whose windows are
    all in the back, or Ctrl+click on any running button, opens them in every
    case. While they're up, moving to another button moves them there.
  $options:
  - hover: When the pointer rests on a button
  - groups: On resting, only for buttons with several windows
  - click: Only on a click
  #! $showIf: {previews: thumbnails}
- previewPlace: button
  $name: Previews placed
  $description: Beside the bar, centred on the button, or on the pointer.
  $options:
  - button: On the button
  - pointer: On the pointer
  #! $showIf: {previews: thumbnails}
- previewAway: 0
  $name: Previews, further from the bar (px)
  $description: >-
    Moves previews away from the bar (or over it, below 0), whichever side of
    the screen it's on.
  #! $min: -200
  #! $max: 400
  #! $showIf: {previews: thumbnails}
- previewAlong: 0
  $name: Previews, along the bar (px)
  $description: >-
    Moves previews along the bar: right on a bar at the top or bottom, down on
    one at the side; below 0 the other way.
  #! $min: -800
  #! $max: 800
  #! $showIf: {previews: thumbnails}
- previewStay: leave
  $name: Previews stay up
  $description: >-
    Until the pointer leaves the button and its previews, or until you dismiss
    them (the ways below), so you can move away, work, and come back to them.
  $options:
  - leave: Until the pointer leaves
  - dismissed: Until dismissed
  #! $showIf: {previews: thumbnails}
- previewLeaveDelay: 300
  $name: Close after the pointer leaves (ms)
  #! $min: 0
  #! $max: 5000
  #! $showIf: {previews: thumbnails, previewStay: leave}
- previewCloseClick: true
  $name: Dismiss with a click elsewhere
  $description: Any click outside the previews and their button.
  #! $showIf: {previews: thumbnails, previewStay: dismissed}
- previewCloseFocus: true
  $name: Dismiss when another window comes to the front
  $description: Picking one of the previews' own windows doesn't count.
  #! $showIf: {previews: thumbnails, previewStay: dismissed}
- previewCloseEsc: true
  $name: Dismiss with Esc
  $description: The Esc press goes no further, as with a menu.
  #! $showIf: {previews: thumbnails, previewStay: dismissed}
- previewCloseKey: false
  $name: Dismiss with any key
  $description: Shift, Ctrl, Alt and Windows alone don't count; the key still reaches its window.
  #! $showIf: {previews: thumbnails, previewStay: dismissed}
- previewCloseAfter: 0
  $name: Dismiss after the pointer is away (s)
  $description: 0 keeps them up however long the pointer is elsewhere.
  #! $min: 0
  #! $max: 600
  #! $showIf: {previews: thumbnails, previewStay: dismissed}
- previewAfterPick: close
  $name: After picking a window from them
  $description: >-
    Close them, or keep them up to flip between the windows (the one in front
    is marked).
  $options:
  - close: Close them
  - stay: Keep them up
  #! $showIf: {previews: thumbnails}
- previewPeek: true
  $name: Peek at windows from their previews
  $description: >-
    Resting on a preview fades the other windows away and shows that window
    where it is, until the pointer leaves the previews.
- peekDelay: 400
  $name: Peek after (ms)
  #! $min: 0
  #! $max: 3000
- previewIcons: menuNoIcons
  $name: Icons in window previews
  $description: >-
    The app's small icon beside each window's title. "Like menus" leaves them
    out while the suite's Menu No Icons is on, which also takes the icons out
    of the taskbar's own menus.
  $options:
  - menuNoIcons: Like menus (Menu No Icons)
  - show: Show
  - hide: Hide
- previewLook: style
  $name: Preview look
  $description: >-
    The visual style's own previews (when it has them), or your own colours
    below, which are also used when the style has none. Sizes, padding,
    corners, shadow and backdrop apply either way.
  $options:
  - style: The visual style's
  - custom: My own colours
  #! $showIf: {previews: thumbnails}
- previewBackground: ""
  $name: Previews, background
  $description: Empty for the colour scheme's.
  #! $format: colorRgb
  #! $showIf: {previews: thumbnails, previewLook: custom}
- previewText: ""
  $name: Previews, titles
  #! $format: colorRgb
  #! $showIf: {previews: thumbnails, previewLook: custom}
- previewHover: ""
  $name: Previews, pointed-at window
  #! $format: colorRgb
  #! $showIf: {previews: thumbnails, previewLook: custom}
- previewHoverBorder: ""
  $name: Previews, pointed-at window's border
  #! $format: colorRgb
  #! $showIf: {previews: thumbnails, previewLook: custom}
- previewActive: ""
  $name: Previews, the window in front (its mark)
  #! $format: colorRgb
  #! $showIf: {previews: thumbnails, previewLook: custom}
- previewFlashing: ""
  $name: Previews, a window wanting attention
  #! $format: colorRgb
  #! $showIf: {previews: thumbnails, previewLook: custom}
- previewCloseHover: ""
  $name: Previews, close button pointed at
  #! $format: colorRgb
  #! $showIf: {previews: thumbnails, previewLook: custom}
- previewCloseGlyph: ""
  $name: Previews, close button's cross
  #! $format: colorRgb
  #! $showIf: {previews: thumbnails, previewLook: custom}
- previewBorder: ""
  $name: Previews, border colour
  $description: >-
    Windows' thin outer border, and your own inner one. Empty for the colour
    scheme's; "none" for no outer border.
  #! $showIf: {previews: thumbnails}
- previewBorderWidth: 1
  $name: Previews, inner border width (px)
  #! $min: 0
  #! $max: 8
  #! $showIf: {previews: thumbnails, previewLook: custom}
- previewThumbWidth: 200
  $name: Previews, thumbnail width (px)
  #! $min: 80
  #! $max: 480
  #! $showIf: {previews: thumbnails}
- previewThumbHeight: 120
  $name: Previews, thumbnail height (px)
  #! $min: 48
  #! $max: 360
  #! $showIf: {previews: thumbnails}
- previewPadding: 8
  $name: Previews, padding in each window (px)
  #! $min: 0
  #! $max: 32
  #! $showIf: {previews: thumbnails}
- previewEdge: 6
  $name: Previews, padding to the border (px)
  $description: Between the border and the windows, and between windows.
  #! $min: 0
  #! $max: 40
  #! $showIf: {previews: thumbnails}
- previewCorners: square
  $name: Previews, corners
  $options:
  - square: Square
  - small: Slightly rounded
  - round: Rounded
  #! $showIf: {previews: thumbnails}
- previewShadow: false
  $name: Previews, shadow
  #! $showIf: {previews: thumbnails}
- previewBackdrop: none
  $name: Previews, backdrop
  $description: >-
    What shows through the background (at the opacity below): the desktop
    blurred, acrylic, or Mica. The backdrop takes the place of a style's
    background picture. Mica follows the wallpaper only while Windows counts
    the window as active, and previews never take the focus, so it may show
    as Windows' plain Mica colour.
  $options:
  - none: None
  - blur: Blur
  - acrylic: Acrylic
  - mica: Mica
  #! $showIf: {previews: thumbnails}
- previewOpacity: 100
  $name: Previews, background opacity (%)
  #! $format: slider
  #! $min: 0
  #! $max: 100
  #! $showIf: {previews: thumbnails}
- tray: shown
  $name: Notification area
  $description: >-
    The tray icons (System Icons' among them), mirrored from Explorer's tray
    through the suite's Explorer relay, which needs to be on. Shift+right-click
    an icon to put it in the tray, the overflow (System Icons' hidden icons
    flyout) or hidden, or to move it along; right-click the bar for the whole
    list.
  $options:
  - shown: Shown
  - hidden: Hidden
- trayEverywhere: false
  $name: Tray on every screen's bar
  $description: >-
    The other screens' bars show the tray icons too, the same ones as the main
    bar's, and their clicks and menus open there. Off: they only have the clock,
    as Windows' own secondary taskbars.
- showDesktop: windows
  $name: Show desktop button
  $description: >-
    The thin button at the far end of the bar that shows the desktop, drawn from
    the style as StartAllBack draws it. "As Windows is set" follows Windows'
    "Select the far corner of the taskbar to show the desktop".
  $options:
  - windows: As Windows is set
  - always: Always
  - never: Never
- desktops: true
  $name: Desktops pager
  $description: >-
    A mark for each of Windows' desktops (the ones Win+Tab makes), the one
    showing lit: click one to go to it, or scroll over them. Drag it along the
    bar to move it, or off the bar to float it anywhere on the screen (drag it
    back to dock it). Right-click it to lock it in place, pick its look (dots,
    numbers or names), or add and close desktops. Middle-click one to rename it.
- desktopKeys:
  - next: ''
    $name: Next desktop
    $description: >-
      Your own key for the next desktop; Windows' Ctrl+Win+Right keeps working
      too. None until you set one. Once set, the suite takes that key press
      for itself: the program in front never hears it, so any shortcut it had
      on the same keys silently stops working. Pick a combination with two
      modifiers (like Ctrl+Alt or Win+Alt) that nothing else you use takes;
      with just Ctrl, Ctrl+S could stop saving without any sign.
    #! $format: hotkey
  - previous: ''
    $name: Previous desktop
    $description: >-
      Your own key for the desktop before; Windows' Ctrl+Win+Left keeps
      working too. None until you set one. Once set, the suite takes that key
      press for itself: the program in front never hears it, so any shortcut
      it had on the same keys silently stops working. Pick a combination with
      two modifiers (like Ctrl+Alt or Win+Alt) that nothing else you use
      takes; with just Ctrl, Ctrl+S could stop saving without any sign.
    #! $format: hotkey
  - goTo: none
    $name: Go to desktop 1 to 9
    $description: >-
      Held with a number key, goes straight to that desktop. None until you
      pick one. While held, the number keys go to this instead of the program
      in front, so pick modifiers nothing else you use takes with numbers.
    $options:
    - none: None
    - alt: Alt
    - ctrlAlt: Ctrl+Alt
    - shiftAlt: Shift+Alt
    - winAlt: Win+Alt
  - moveTo: none
    $name: Send the window to desktop 1 to 9
    $description: >-
      Held with a number key, sends the window in front to that desktop. None
      until you pick one. While held, the number keys go to this instead of
      the program in front, so pick modifiers nothing else you use takes with
      numbers (and not the same as Go to desktop's).
    $options:
    - none: None
    - alt: Alt
    - ctrlAlt: Ctrl+Alt
    - shiftAlt: Shift+Alt
    - winAlt: Win+Alt
  - moveNext: ''
    $name: Send the window to the next desktop
    $description: >-
      None until you set one. Once set, the suite takes that key press for
      itself: the program in front never hears it, so any shortcut it had on
      the same keys silently stops working. Pick a combination with two
      modifiers (like Ctrl+Alt or Win+Alt) that nothing else you use takes;
      with just Ctrl, Ctrl+S could stop saving without any sign.
    #! $format: hotkey
  - movePrevious: ''
    $name: Send the window to the desktop before
    $description: >-
      None until you set one. Once set, the suite takes that key press for
      itself: the program in front never hears it, so any shortcut it had on
      the same keys silently stops working. Pick a combination with two
      modifiers (like Ctrl+Alt or Win+Alt) that nothing else you use takes;
      with just Ctrl, Ctrl+S could stop saving without any sign.
    #! $format: hotkey
  - follow: false
    $name: Go along with a window sent
    $description: Switch to the desktop a window was sent to, the window still in front.
  $name: Desktop keys
  $description: >-
    Your own keys for switching Windows' desktops and sending windows to
    them, with the Desktops pager on. Sending a window needs Windows 11 24H2
    or later; elsewhere the keys do nothing.
- desktopsLook:
  - size: 6
    $name: Mark size
    $description: A mark's size in pixels at 100%; the desktop showing is 2 px bigger.
    #! $format: slider
    #! $min: 2
    #! $max: 24
  - spacing: 8
    $name: Space between marks
    #! $min: 0
    #! $max: 40
  - shape: pixel
    $name: Mark shape
    $description: Pill draws the desktop showing as a wide pill. Numbers and names take the matching box.
    $options:
    - pixel: Pixel dot
    - square: Square
    - circle: Circle
    - diamond: Diamond
    - pill: Pill
    - bar: Bar
  - outline: 0
    $name: Mark outline
    $description: Pixels of the outer colour around each mark, the inner colour inside.
    #! $min: 0
    #! $max: 6
  - colors: scheme
    $name: Colours
    $options:
    - scheme: From the colour scheme
    - custom: Custom
  - markColor: '4F8F6A'
    $name: Mark inner colour
    #! $format: colorRgb
    #! $showIf: {colors: custom}
  - markOutline: '1E2F36'
    $name: Mark outer colour
    #! $format: colorRgb
    #! $showIf: {colors: custom}
  - currentColor: '15FFA2'
    $name: Desktop showing, inner colour
    #! $format: colorRgb
    #! $showIf: {colors: custom}
  - currentOutline: '66EA97'
    $name: Desktop showing, outer colour
    #! $format: colorRgb
    #! $showIf: {colors: custom}
  - panelColor: '121C21'
    $name: Floating panel colour
    #! $format: colorRgb
    #! $showIf: {colors: custom}
  - panelEdge: '4F8F6A'
    $name: Floating panel edge colour
    #! $format: colorRgb
    #! $showIf: {colors: custom}
  - panel: rounded
    $name: Floating panel shape
    $description: The panel behind a floating pager's marks. None leaves just the marks.
    $options:
    - rounded: Rounded
    - square: Square
    - pill: Pill
    - none: None
  - panelOpacity: 100
    $name: Floating panel opacity
    $description: The panel only; the marks stay solid.
    #! $format: slider
    #! $min: 0
    #! $max: 100
  - opacity: 100
    $name: Floating pager opacity
    $description: The whole floating pager, panel and marks.
    #! $format: slider
    #! $min: 10
    #! $max: 100
  - fade: none
    $name: Fade a floating pager
    $options:
    - none: Never
    - idle: When the pointer isn't on it
  - fadeTo: 30
    $name: Faded opacity
    #! $format: slider
    #! $min: 0
    #! $max: 100
    #! $showIf: {fade: idle}
  - fadeDelay: 1500
    $name: Fade after (ms)
    #! $min: 0
    #! $max: 10000
    #! $showIf: {fade: idle}
  - vertical: false
    $name: Floating pager runs top to bottom
  - previews: true
    $name: Desktop previews
    $description: >-
      Pointing at a mark for a moment shows that desktop: its wallpaper with
      its windows where they are, live.
  - previewWidth: 280
    $name: Preview width
    $description: In pixels at 100%; the height follows the screen's shape.
    #! $format: slider
    #! $min: 160
    #! $max: 640
    #! $showIf: {previews: true}
  $name: Desktops pager look
  $description: >-
    Size, shape and colours of the pager's marks, on the bar and floating; the
    panel, opacity and fading apply to a floating pager.
- trayMargins: small
  $name: Tray icon margins
  $options:
  - xsmall: XS
  - small: S
  - medium: M
  - large: L
  - xlarge: XL
- clock: shown
  $name: Clock
  $description: >-
    Drawn from the Clock part's settings: its lines, font, colours and
    clicks.
  $options:
  - shown: Shown
  - hidden: Hidden
- startMenu:
  - enabled: true
    $name: Use enhanced classic start menu
    $description: >-
      Our own Start menu, drawn as StartAllBack's is. Off: the Start button
      opens Windows' (or StartAllBack's) own.
  - visualStyle: startallback
    $name: Visual style
    $description: >-
      StartAllBack's styles, from its Styles folder or
      %LOCALAPPDATA%\Tourne\Styles: the one StartAllBack's Start menu is set
      to, the taskbar's, or Default (none: the colour scheme). Its scroll bar
      and menus come from Windows' visual style when the style has none.
    #! $dynamicSelect: true
  - iconSize: small
    $name: Icon size
    $options:
    - small: S
    - large: L
    - medium: M
  - iconCount: 10
    $name: Icon count
    $description: How many recently opened programs show under your pinned ones.
    #! $min: 0
    #! $max: 60
  - myPrograms:
    - largeIcons: false
      $name: Use large icons
    $name: My Programs
  - allPrograms:
    - largeIcons: false
      $name: Use large icons
    - flyout: false
      $name: Display as flyout menu (Windows XP style)
    - modernAppsFolder: true
      $name: Display Modern Apps in a folder
    - sortOrder: itemsFirst
      $name: Override sort order
      $options:
      - itemsFirst: Items first
      - foldersFirst: Folders first
      - noPreference: No preference
    - appsFolderIcon: ''
      $name: Apps folder's icon
      $description: >-
        Our own addition. An .ico file, or a file and an icon number like
        imageres.dll,3. Empty: StartAllBack's, if it had one.
    - folderIcon: ''
      $name: Other folders' icon
      $description: >-
        Our own addition. One icon for every other folder in All Programs,
        written the same way. Empty: Windows' own folder icon.
    $name: All Programs
  - search:
    - largeIcons: false
      $name: Use large icons
    - programsAndSettings: true
      $name: Search programs and settings
    - publicFolders: false
      $name: Search public folders
    - flyout: false
      $name: Use search flyout for more results
    $name: Search
  - general:
    - highlightNew: false
      $name: Highlight newly installed programs
    - autoCascade: false
      $name: Open submenus when I pause on them with the mouse pointer
    $name: General
  - rightSide:
    - userPicture: false
      $name: Display user account picture
    - glyphIcons: false
      $name: Display glyph icons
    - multiColumn: false
      $name: Use multi-column flyout menus for right side items
    - user: dontDisplay
      $name: Your folder
      $options:
      - dontDisplay: Don't display
      - link: Link
      - menu: Menu
    - documents: dontDisplay
      $name: Documents
      $options:
      - dontDisplay: Don't display
      - link: Link
      - menu: Menu
    - pictures: dontDisplay
      $name: Pictures
      $options:
      - dontDisplay: Don't display
      - link: Link
      - menu: Menu
    - music: dontDisplay
      $name: Music
      $options:
      - dontDisplay: Don't display
      - link: Link
      - menu: Menu
    - videos: dontDisplay
      $name: Videos
      $options:
      - dontDisplay: Don't display
      - link: Link
      - menu: Menu
    - downloads: dontDisplay
      $name: Downloads
      $options:
      - dontDisplay: Don't display
      - link: Link
      - menu: Menu
    - oneDrive: dontDisplay
      $name: OneDrive
      $options:
      - dontDisplay: Don't display
      - link: Link
      - menu: Menu
    - favorites: dontDisplay
      $name: Favorites
      $options:
      - dontDisplay: Don't display
      - menu: Menu
    - recentItems: dontDisplay
      $name: Recent Items
      $options:
      - dontDisplay: Don't display
      - menu: Menu
    - network: dontDisplay
      $name: Network
      $options:
      - dontDisplay: Don't display
      - link: Link
    - networkConnections: dontDisplay
      $name: Network Connections
      $options:
      - dontDisplay: Don't display
      - link: Link
      - menu: Menu
    - computer: dontDisplay
      $name: This PC
      $options:
      - dontDisplay: Don't display
      - link: Link
      - menu: Menu
    - controlPanel: link
      $name: Control Panel
      $options:
      - dontDisplay: Don't display
      - link: Link
      - menu: Menu
    - settings: link
      $name: Settings
      $options:
      - dontDisplay: Don't display
      - link: Link
      - menu: Menu
    - windowsTools: dontDisplay
      $name: Windows Tools
      $options:
      - dontDisplay: Don't display
      - menu: Menu
    - devicesAndPrinters: dontDisplay
      $name: Devices and Printers
      $options:
      - dontDisplay: Don't display
      - link: Link
      - menu: Menu
    - defaultPrograms: dontDisplay
      $name: Default Programs
      $options:
      - dontDisplay: Don't display
      - link: Link
    - terminal: link
      $name: Windows Terminal
      $description: Its jump list is our own addition.
      $options:
      - dontDisplay: Don't display
      - link: Link
      - jumpList: Jump list
    - commandPrompt: dontDisplay
      $name: Command Prompt
      $options:
      - dontDisplay: Don't display
      - link: Link
    - run: link
      $name: Run
      $options:
      - dontDisplay: Don't display
      - link: Link
    - customFolders:
      - - path: ''
          $name: Folder
          $description: >-
            A folder, or (our own addition) a program, shortcut, file or shell:
            name, like shell:AppsFolder\Microsoft.WindowsCalculator_8wekyb3d8bbwe!App.
        - show: menu
          $name: Shown as
          $options:
          - dontDisplay: Don't display
          - link: Link
          - menu: Menu
          - jumpList: Jump list (our own addition)
        - name: ''
          $name: Name
          $description: Leave empty for its own name.
      $name: Custom folders
      $description: >-
        Add custom folders to the right side of the Start Menu. They go at the
        top. Left empty, StartAllBack's own custom folders are used.
    - largeFlyoutIcons: false
      $name: Use large icons
      $description: In the custom folders' menus.
    $name: Right side items
    $description: >-
      Right-clicking one in the Start menu changes it too, as StartAllBack's
      "Display as a menu" and "Remove from this list" do, until these are
      changed here.
  - powerButton: shutDown
    $name: Power button default action
    $options:
    - shutDown: Shut down
    - switchUser: Switch user
    - signOut: Sign out
    - lock: Lock
    - restart: Restart
    - sleep: Sleep
    - hibernate: Hibernate
  - winKey: false
    $name: Open it with the Windows key
    $description: >-
      Our own addition. A lone press of the Windows key opens this Start
      menu instead of Windows' own; its shortcuts (like Windows+E) keep
      working. Leave it off while StartAllBack's Start menu is in use.
      Without StartAllBack, Explorer hands every opening of Windows' Start
      to this one (the key, Ctrl+Esc, anything else), and no key press has
      to pass through the taskbar.
  - onPress: true
    $name: Open on press
    $description: >-
      Opens the moment the Start button is pressed, as a menu does, instead
      of when it's let go: the length of a click (about a tenth of a
      second) sooner.
  - capitals: false
    $name: Capital letters
    $description: Our own addition. All of the Start menu's text in capitals.
  - menuColors: auto
    $name: Menus and flyouts
    $description: >-
      Our own addition. The flyouts, jump lists and right-click menus it
      opens. Automatic makes them from the Start menu's own colours.
    $options:
    - auto: Automatic, from the Start menu's look
    - visualStyle: The visual style's menus
    - scheme: The colour scheme
    - tourne: Tourne Menus, as the bar's own menus look
  - textColor: ''
    $name: Text colour
    $description: >-
      Our own addition. Like #64aa89. Empty: the style's, or System Text
      Color's when that's on.
  - hotTextColor: ''
    $name: Text colour, pointed at
    $description: >-
      Our own addition. Empty: the text colour above when it's set, else the
      style's.
  - searchIcon: ''
    $name: Search icon
    $description: >-
      Our own addition. An .ico file (or an .exe or .dll with ,index) for the
      search box's magnifier and the "See more results" line. Empty:
      StartAllBack's magnifier.
  - phone: false
    $name: My phone
    $description: >-
      Our own addition. With a phone linked in Phone Link, a pane beside
      the places, as Windows 11 shows your phone beside Start: its name,
      battery and connection (from Bluetooth, so right with Phone Link
      closed), Messages, Notifications, Send a file, and its latest
      notifications, each as its own app with its icon. Drawn in the Start
      menu's look.
  - phoneNotifications: 4
    $name: Phone notifications shown
    $description: Our own addition. How many of its latest, 0 to 8.
  - phoneMessages: 3
    $name: Phone texts shown
    $description: >-
      Our own addition, in Tourne Desktop. Tourne holds your phone's texts
      itself over Bluetooth, as a car does: its latest conversations show
      in the phone (in Start or on its own), a click opens one with a box
      to reply in, and each new text pops up. Only one program can hold
      them: while Phone Link runs it has them. An iPhone shares only texts
      that come while it's connected. 0: Phone Link's Messages link
      instead.
  - phoneCallOutput: default
    $name: "Calls: sound out"
    $description: >-
      Our own addition, with Tourne Calls (desktop\calls). Where a call
      answered on this PC plays: Windows' default for calls, or one of your
      speakers or headsets. Changed during a call, it moves at once.
    #! $dynamicSelect: true
  - phoneCallMic: default
    $name: "Calls: microphone"
    $description: >-
      Our own addition, with Tourne Calls. The mic the caller hears on a
      call answered on this PC: Windows' default for calls, or one of yours.
    #! $dynamicSelect: true
  - phoneIcon: ''
    $name: Phone icon
    $description: >-
      Our own addition. tourne:phone (one of System Icons' tray designs, in
      its colours), an .ico file, or an .exe or .dll with ,index. Empty: the
      tray's phone. The phone's own buttons and right-click menu move it out
      of Start into a window of its own (or drag its name out), keep that
      open, or close it; Start's right-click brings it back.
  - phoneMessagesIcon: ''
    $name: Messages icon
    $description: Our own addition. As above. Empty, tourne:chat.
  - phoneNotificationsIcon: ''
    $name: Notifications icon
    $description: Our own addition. As above. Empty, tourne:bell (lit while there are new ones).
  - phoneSendIcon: ''
    $name: Send a file icon
    $description: Our own addition. As above. Empty, tourne:cloud-sync.
  - phoneSettingsIcon: ''
    $name: Phone settings icon
    $description: Our own addition. As above. Empty, tourne:system.
  - phoneTray: false
    $name: Open from the tray's phone
    $description: >-
      Our own addition. A click on Phone Link's tray icon opens this phone
      as a flyout above it, instead of Phone Link's window. Moving it or
      pinning it keeps it open as a window of its own.
  - phoneAppIcons: false
    $name: Notifications with their apps' logos
    $description: >-
      Our own addition. Each notification with its phone app's own logo
      (Gmail, Reddit...), as Phone Link keeps them. Off: the design the
      notification centre gives it (mail, reddit, chat, video, shop...), in
      the theme's colours.
  $name: Start Menu
  $description: Configure Start menu items and behavior
- buttonIcons:
  - - program: ''
      $name: Program
      $description: >-
        Its file name (like GitHubDesktop.exe), its full path, or an app's
        AppUserModelID (like Claude_pzs8sxrjxfjjc!Claude, from Get-StartApps).
        * and ? match any text, like *\NVIDIA Corporation\*.exe for every
        program in that folder.
    - icon: ''
      $name: Icon
      $description: >-
        An .ico file, or an .exe or .dll with ,index (like shell32.dll,4). Or
        one of System Icons' designs, drawn in your colour scheme like the
        built-in ones, on its buttons, its tray icon and in Alt+Tab: tourne:
        and its name, like tourne:screenshot (also capture, chat, cloud, game,
        audio, claude, spotify, wallpaper, fan, tiling, gpu, sensor, bell,
        clipboard, bin, phone, printer, usb, keyboard, power, mic, vpn,
        update, mail, reddit, video, shop).
    - everywhere: false
      $name: Use everywhere
      $description: >-
        The same icon in the tray, Alt+Tab and, with an .ico, wherever Windows
        takes it from the program's file (Explorer, Start, its windows):
        through Icon Redirect, which matches a file name in any folder, so the
        icon stays when an update moves the program.
  $name: Button icons
  $description: >-
    Your own icon for a program's buttons, running or pinned, in place of the
    one it brings; packaged apps included, whose icons can't be changed
    otherwise. Tourne's own are in %LOCALAPPDATA%\Tourne\Icons.
- menu:
  - enabled: false
    $name: Tourne Menu
    $description: >-
      A menu of your own, a full LiteMenu replacement: on the desktop's empty
      space, the bar's, or a hotkey, in Tourne's menu look. With no items
      below, LiteMenu's own menu (its config.xml) is used, read each time it
      opens.
  - button: middle
    $name: Mouse button on the desktop
    $options:
    - middle: Middle button
    - right: Right button (Shift + right-click keeps Windows' menu)
    - none: None (the hotkey only)
  - onBar: false
    $name: On the bar's empty space too
    $description: A middle click on the bar where there's nothing.
  - hotkey: ''
    $name: Hotkey
    $description: >-
      Modifiers, then the key's code, like alt+win+77 (Alt+Win+M). Empty for
      none.
  - items:
    - - depth: 0
        $name: Depth
        $description: >-
          0 for the top level; one more than the row above puts it in that
          row's submenu.
      - name: ''
        $name: Name
        $description: Its text; - for a separator.
      - command: ''
        $name: Command
        $description: >-
          A program, file, folder or web address; shell:AppsFolder\ and an app
          id; or Tourne's own: tourne:show-desktop, run, start, task-manager,
          windhawk, studio, menu-settings, close-litemenu, settings, lock, sleep,
          sign-out, restart, shutdown,
          desktop-next, desktop-previous, desktop-new, desktop-close,
          desktop:N. Live submenus: tourne:windows, tourne:desktops,
          tourne:programs, tourne:recent, tourne:folder: and a path.
      - args: ''
        $name: Arguments
    $name: Items
  $name: Tourne Menu
  $description: >-
    A full LiteMenu replacement, with Tourne's own actions and live submenus.
*/
// ==/WindhawkModSettings==

#include <windows.h>
#include <commctrl.h>
#include <dwmapi.h>
#include <propsys.h>
#include <shellapi.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <shlwapi.h>
#include <uxtheme.h>
#include <vssym32.h>
#include <wincodec.h>
#include <windowsx.h>
#include <cfgmgr32.h>
#include <tlhelp32.h>

#include <endpointvolume.h>
#include <mmdeviceapi.h>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <cwctype>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <urlmon.h>
#include <wininet.h>
#include <iphlpapi.h>
#include <wlanapi.h>
#include <shlobj.h>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// Programs Tourne Desktop leaves alone (its exclusions.cpp: shell.exclude, games, Windhawk's exclusions):
// the bar keeps away from them as from a full-screen program; the Start key's hook is let go while one of shell.excludeKeys
// is in front. None in the suite, where Windhawk's own exclusions keep it out of them.
#ifdef TD_PREFIX
inline bool LeftAlone(HWND h) { return td::ProgramExcluded(h); }
inline bool LeftAloneInFront() { return td::ExcludedInFront(); }
#else
inline bool LeftAlone(HWND) { return false; }
inline bool LeftAloneInFront() { return false; }
#endif

UINT LeftAloneMessage() {  // to the main bar: a program left alone came to the front, or went
    static UINT msg = RegisterWindowMessageW(L"TourneDesktop.LeftAlone");
    return msg;
}

// @inline palettes.cpp
// @inline theme-palette.cpp
// @inline tray-shared.cpp
// @inline plugin-bar-shared.cpp
// @inline shell-core.cpp
// @inline menu-style.cpp
// @inline live-peek.cpp
// @inline motion.cpp

////////////////////////////////////////////////////////////////////////////////
// Theme files (loaded by theme-palette.cpp)

// TaskBar background parts per edge (TBP_BACKGROUND*), and the TaskBand2
// button part. Button states: normal, hover, pressed; the same three for the
// active window; then flashing. Decoded from StartAllBack's Windows 7 style,
// where state 7 is the orange "wants attention" button.
constexpr int kBarBackgroundBottom = 1, kBarBackgroundRight = 2, kBarBackgroundTop = 3, kBarBackgroundLeft = 4;
constexpr int kBandButton = 5;
constexpr int kBandStack = 13, kBandStackActive = 14;  // the edge of a stack of windows (states: 2 two windows, 3 more)
int StackEdgeWidth(int part, int state);
constexpr int kStateNormal = 1, kStateHot = 2, kStatePressed = 3;
constexpr int kStateActive = 4, kStateActiveHot = 5, kStateActivePressed = 6;
constexpr int kStateFlashing = 7;

// A part drawn into a small bitmap, to tell two theme handles apart.
bool SamePartPixels(HTHEME a, HTHEME b, int part, int state) {
    constexpr int kSize = 16;
    HDC dc = CreateCompatibleDC(nullptr);
    BITMAPINFO bi{};
    bi.bmiHeader = {sizeof(BITMAPINFOHEADER), kSize, -kSize, 1, 32, BI_RGB};
    void* bitsA = nullptr;
    void* bitsB = nullptr;
    HBITMAP bmpA = CreateDIBSection(dc, &bi, DIB_RGB_COLORS, &bitsA, nullptr, 0);
    HBITMAP bmpB = CreateDIBSection(dc, &bi, DIB_RGB_COLORS, &bitsB, nullptr, 0);
    bool same = false;
    if (bmpA && bmpB) {
        RECT r{0, 0, kSize, kSize};
        memset(bitsA, 0, kSize * kSize * 4);
        memset(bitsB, 0, kSize * kSize * 4);
        HGDIOBJ old = SelectObject(dc, bmpA);
        DrawThemeBackground(a, dc, part, state, &r, nullptr);
        SelectObject(dc, bmpB);
        DrawThemeBackground(b, dc, part, state, &r, nullptr);
        SelectObject(dc, old);
        GdiFlush();
        same = memcmp(bitsA, bitsB, kSize * kSize * 4) == 0;
    }
    if (bmpA) DeleteObject(bmpA);
    if (bmpB) DeleteObject(bmpB);
    DeleteDC(dc);
    return same;
}

struct Theme {
    ThemeFileData file{};
    HTHEME bar = nullptr;
    HTHEME band = nullptr;
    HTHEME tip = nullptr;   // the style's tooltip look, if it has one
    HTHEME menu = nullptr;  // and its menu look
    // The rest of what StartAllBack takes from its style, when the style has them:
    HTHEME trayArrow[4] = {};     // the show-hidden-icons arrow: TrayNotifyHoriz, HorizOpen, Vert, VertOpen ::Button
    HTHEME clock = nullptr;       // TrayNotify::Clock: behind the clock, and its text colour
    HTHEME trayButton = nullptr;  // TrayNotify::Toolbar: behind a tray icon pointed at
    HTHEME showDesktop[2] = {};   // ShowDesktop::Button, VerticalShowDesktop::Button
    HTHEME preview = nullptr;     // TaskbandExtendedUI: the window previews
    HWND barHost = nullptr;
    HWND bandHost = nullptr;
    HWND tipHost = nullptr;
    HWND partsHost = nullptr;

    // A class as an app asks for it (App::Class), only when the style has that very class: for an app name
    // it doesn't know, uxtheme hands back the plain class, which a full visual style has (an ordinary push
    // button, say). Those draw the same as the plain class, so that's how they're told apart.
    HTHEME OpenOwn(PCWSTR app, PCWSTR cls, OpenThemeDataFromFile_t open) {
        SetWindowTheme(partsHost, app, nullptr);
        HTHEME theme = open(&file, partsHost, cls, 0);
        SetWindowTheme(partsHost, L"", nullptr);
        if (!theme || !*app) return theme;
        if (HTHEME plain = open(&file, partsHost, cls, 0)) {
            bool fallback = SamePartPixels(theme, plain, 1, 2) && SamePartPixels(theme, plain, 1, 1);
            CloseThemeData(plain);
            if (fallback) {
                CloseThemeData(theme);
                return nullptr;
            }
        }
        return theme;
    }

    HTHEME Open(HWND host, PCWSTR app, PCWSTR cls, int wantPart, OpenThemeDataFromFile_t open) {
        SetWindowTheme(host, app, nullptr);
        HTHEME theme = open(&file, host, cls, 0);
        if (theme && IsThemePartDefined(theme, wantPart, 0)) return theme;
        if (theme) CloseThemeData(theme);
        // Plain class names, for themes without the composited ones.
        SetWindowTheme(host, L"", nullptr);
        return open(&file, host, cls, 0);
    }

    bool Load(std::wstring const& path) {
        Close();
        auto open = ThemeFileOpener();
        if (!open || !LoadThemeFile(path, &file)) return false;
        barHost = CreateWindowExW(0, L"STATIC", L"", 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, nullptr, nullptr);
        bandHost = CreateWindowExW(0, L"STATIC", L"", 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, nullptr, nullptr);
        bar = Open(barHost, L"Taskbar2Composited", L"TaskBar", kBarBackgroundTop, open);
        band = Open(bandHost, L"Taskband2Composited", L"TaskBand2", kBandButton, open);
        tipHost = CreateWindowExW(0, L"STATIC", L"", 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, nullptr, nullptr);
        tip = Open(tipHost, L"", L"Tooltip", TTP_STANDARD, open);
        if (tip && !IsThemePartDefined(tip, TTP_STANDARD, 0)) {
            CloseThemeData(tip);
            tip = nullptr;
        }
        menu = open(&file, tipHost, L"Menu", 0);
        if (menu && !IsThemePartDefined(menu, MENU_POPUPITEM, 0)) {
            CloseThemeData(menu);
            menu = nullptr;
        }
        partsHost = CreateWindowExW(0, L"STATIC", L"", 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, nullptr, nullptr);
        PCWSTR arrows[] = {L"TrayNotifyHoriz", L"TrayNotifyHorizOpen", L"TrayNotifyVert", L"TrayNotifyVertOpen"};
        for (int i = 0; i < 4; i++) trayArrow[i] = OpenOwn(arrows[i], L"Button", open);
        clock = OpenOwn(L"TrayNotify", L"Clock", open);
        trayButton = OpenOwn(L"TrayNotify", L"Toolbar", open);
        showDesktop[0] = OpenOwn(L"ShowDesktop", L"Button", open);
        showDesktop[1] = OpenOwn(L"VerticalShowDesktop", L"Button", open);
        preview = OpenOwn(L"", L"TaskbandExtendedUI", open);
        if (preview && !IsThemePartDefined(preview, 1, 0) && !IsThemePartDefined(preview, 3, 0)) {
            CloseThemeData(preview);
            preview = nullptr;
        }
        return bar || band;
    }

    void Close() {
        for (HTHEME* t : {&bar, &band, &tip, &menu, &trayArrow[0], &trayArrow[1], &trayArrow[2], &trayArrow[3], &clock,
                          &trayButton, &showDesktop[0], &showDesktop[1], &preview}) {
            if (*t) CloseThemeData(*t);
            *t = nullptr;
        }
        for (HWND* h : {&barHost, &bandHost, &tipHost, &partsHost}) {
            if (*h) DestroyWindow(*h);
            *h = nullptr;
        }
        CloseThemeFile(&file);
    }
};

////////////////////////////////////////////////////////////////////////////////
// The Start orb: an image with three states stacked (normal, hover, pressed),
// decoded with WIC into a premultiplied 32-bit bitmap.

constexpr GUID kClsidWicImagingFactory = {0xcacaf262, 0x9370, 0x4615, {0xa1, 0x3b, 0x9f, 0x55, 0x39, 0xda, 0x4c, 0x0a}};
constexpr GUID kWicPixelFormat32bppPBGRA = {0x6fddc324, 0x4e03, 0x4bfe, {0xb1, 0x85, 0x3d, 0x77, 0x76, 0x8d, 0xc9, 0x10}};

struct Orb {
    HBITMAP bitmap = nullptr;
    int width = 0;
    int height = 0;
    int frames = 1;

    bool Load(std::wstring const& path) {
        Close();
        if (path.empty()) return false;
        IWICImagingFactory* factory = nullptr;
        if (FAILED(CoCreateInstance(kClsidWicImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
                                    __uuidof(IWICImagingFactory), (void**)&factory))) {
            return false;
        }
        IWICBitmapDecoder* decoder = nullptr;
        IWICBitmapFrameDecode* frame = nullptr;
        IWICFormatConverter* converter = nullptr;
        bool ok = SUCCEEDED(factory->CreateDecoderFromFilename(path.c_str(), nullptr, GENERIC_READ,
                                                               WICDecodeMetadataCacheOnDemand, &decoder)) &&
                  SUCCEEDED(decoder->GetFrame(0, &frame)) && SUCCEEDED(factory->CreateFormatConverter(&converter)) &&
                  SUCCEEDED(converter->Initialize(frame, kWicPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, nullptr,
                                                  0, WICBitmapPaletteTypeCustom));
        UINT w = 0, h = 0;
        if (ok) {
            converter->GetSize(&w, &h);
            BITMAPINFO bi{};
            bi.bmiHeader = {sizeof(BITMAPINFOHEADER), (LONG)w, -(LONG)h, 1, 32, BI_RGB};
            void* bits = nullptr;
            bitmap = CreateDIBSection(nullptr, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
            ok = bitmap && SUCCEEDED(converter->CopyPixels(nullptr, w * 4, w * h * 4, (BYTE*)bits));
        }
        if (converter) converter->Release();
        if (frame) frame->Release();
        if (decoder) decoder->Release();
        factory->Release();
        if (!ok) {
            Close();
            Wh_Log(L"Couldn't read the orb %s", path.c_str());
            return false;
        }
        width = (int)w;
        height = (int)h;
        // Stacked states make the image about three times as tall as wide.
        frames = height >= width * 5 / 2 ? 3 : 1;
        return true;
    }

    int FrameHeight() const { return frames ? height / frames : height; }

    // state: 0 normal, 1 hover, 2 pressed.
    void Draw(HDC dc, RECT const& rc, int state) const {
        if (!bitmap) return;
        int fh = FrameHeight();
        int s = std::min(state, frames - 1);
        HDC mem = CreateCompatibleDC(dc);
        HGDIOBJ old = SelectObject(mem, bitmap);
        BLENDFUNCTION blend{AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
        AlphaBlend(dc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, mem, 0, s * fh, width, fh, blend);
        SelectObject(mem, old);
        DeleteDC(mem);
    }

    void Close() {
        if (bitmap) DeleteObject(bitmap);
        bitmap = nullptr;
        width = height = 0;
        frames = 1;
    }
};

////////////////////////////////////////////////////////////////////////////////
// Settings, laid out like StartAllBack's Taskbar page

enum class Edge { Top, Bottom, Left, Right };
enum class StartAt { Edge, WithTasks, End };
enum class Combine { Always, WhenFull, Never, NeverHideLabels, NeverGroup };
enum class Segments { Off, Three, Central };

// A row of the Tourne Menu (tourne-menu.cpp): one more depth than the row above puts it in that row's submenu.
struct MenuRow {
    int depth = 0;
    std::wstring name, command, args;
};

struct Settings {
    std::wstring style;       // a theme file, or empty for the flat bar
    std::wstring orb;
    Palette palette = SchemePalette(L"tourne");
    Edge edge = Edge::Top;
    StartAt startAt = StartAt::Edge;
    bool centered = true;
    Combine combine = Combine::Always;
    Segments segments = Segments::Off;
    int iconSize = 16;        // at 100%
    bool onlyThisScreen = false;  // Windows' "taskbar where the window is open"
    int mmMode = 0;               // Windows' "Show taskbar buttons on": 0 all taskbars, 1 main and where open, 2 where open
    bool multiBars = false;       // a bar on every screen
    bool tray = true;
    bool trayEverywhere = false;  // the other screens' bars show the tray too
    bool showDesktop = false;     // the show desktop button at the far end
    bool desktops = true;         // the virtual desktops pager (desktops-pager.cpp)
    struct PagerLook {
        int size = 6, spacing = 8;  // at 100%
        int shape = 0;              // kMark* (desktops-pager.cpp)
        int outline = 0;
        bool custom = false;        // colours of its own, below; else the scheme's
        uint32_t mark = 0, markOutline = 0, current = 0, currentOutline = 0, panel = 0, panelEdge = 0;  // 0xRRGGBB
        int panelShape = 0;         // 0 rounded, 1 square, 2 pill, 3 none
        int panelOpacity = 100, opacity = 100;  // %
        bool fade = false;
        int fadeTo = 30, fadeDelay = 1500;
        bool vertical = false;
        bool previews = true;       // pointing at a mark shows that desktop
        int previewWidth = 280;     // at 100%
    } pagerLook;
    std::wstring yasbTheme;      // a folder under Tourne\Yasb: the bar is drawn from that YASB theme
    bool yasbFill = true;         // a theme without task buttons or a tray gets ours
    bool yasbCommands = false;    // a theme's buttons may run commands
    bool yasbNetwork = false;     // the weather widget may ask the internet
    bool yasbFades = true;        // a theme's transition lines are followed
    int trayMargin = 3;       // at 100%
    bool clock = true;
    int margin = 8;           // either side of the icon, at 100%
    int thickness = 40;       // at 100%
    bool autoHide = false;
    int shyTolerance = 8;  // px a window may reach into the bar before shyness hides it
    bool shy = false;
    // The hiding animation: 0 none, 1 slide, 2 fade, 3 both; its easing: 0 off, 1 gentle, 2 soft, 3 smooth.
    // What stays at the edge while the bar is away: 0 a sliver of the bar, 1 a glow, 2 nothing.
    int shyAnim = 1, shyEase = 3, shyDuration = 220, shyHideDelay = 600, shyShowDelay = 250;
    motion::Anim motionBar;  // the motion tree's "bar" line (motion.cpp), when one is set
    motion::Anim motionPopups;  // and its "popups" line, for menus opening
    motion::Anim motionStart;   // "start": the Start menu opening and closing
    motion::Anim motionPreviewIn;  // "fadePopupsIn": the window previews fading in
    motion::Anim motionDesksIn, motionDesksOut;  // "workspacesIn", "workspacesOut": the pager's lit desktop moving
    motion::Anim motionLayersIn, motionLayersOut;  // "fadeLayersIn", "fadeLayersOut": the floating pager's fade
    motion::Anim motionScreenAdded;  // "monitorAdded": a new screen's bar coming in
    int menuAnimation = 0;      // MenuMotionStyle: 0 at once
    MenuRules menuRules;        // labels or verbs left out of shell menus, or moved into "More options"
    bool desktopMenuReady = false;
    int shyRevealZone = 0;                               // px from the edge that brings a hidden bar back
    bool shyRevealAttention = false, shyRevealStart = false;
    bool fullscreenStayAway = true;               // keep out of a full-screen program's way
    std::wstring emptyDoubleClick = L"show-desktop";  // a Tourne Menu action (tourne:...), or "none"
    std::wstring emptyMiddleClick = L"none", emptyMiddleDoubleClick = L"none";  // the same, with the middle button
    std::vector<std::wstring> fullscreenPrograms;  // and of these (lower case), full screen or not
    // Fading when idle: after idleAfter s away, to idleOpacity %, always (0), only while nothing touches
    // the bar (1) or only while something does (2); rising within idleNear px; over these ms.
    bool idleFade = false, idleWakeAttention = true;
    int idleAfter = 5, idleOpacity = 40, idleWhen = 0, idleNear = 60, idleFadeTime = 600, idleWakeTime = 120;
    // Shadows: under icons (1), frames (2) or both (3); colour, strength %, softness and offset (px at 100%).
    int shadows = 0, shadowStrength = 50, shadowSoftness = 2, shadowX = 1, shadowY = 1;
    COLORREF shadowColor = 0;
    int shyEdge = 0, shyGlow = 60, shyGlowSize = 4;
    bool previews = true;     // window previews on hovering a button
    bool previewIcons = true; // app icons beside the previews' titles
    bool previewPeek = true;  // resting on a preview shows that window alone
    int peekDelay = 400;
    bool menuIcons = true;    // icons in the bar's menus (off with Menu No Icons)
    int previewDelay = 400;   // ms
    // The previews' look: the style's (0) or own colours (1; CLR_INVALID for the scheme's); sizes and
    // padding (px at 100%); corners 0 square, 1 small, 2 round; backdrop 0 none, 1 blur, 2 acrylic, 3 mica;
    // background opacity (%).
    int previewLook = 0, previewThumbW = 200, previewThumbH = 120, previewPad = 8, previewEdge = 6, previewBorderW = 1;
    int previewCorners = 0, previewBackdrop = 0, previewOpacity = 100;
    bool previewShadow = false, previewBorderNone = false;
    COLORREF previewBg = CLR_INVALID, previewTextColor = CLR_INVALID, previewHover = CLR_INVALID,
             previewHoverBorder = CLR_INVALID, previewActive = CLR_INVALID, previewFlashing = CLR_INVALID,
             previewCloseHover = CLR_INVALID, previewCloseGlyph = CLR_INVALID, previewBorder = CLR_INVALID;
    // A button's right-click menu: at the pointer (0), beside the bar from the button's start (1) or
    // centred on it (2), moved away from the bar and along it by these (px at 100%).
    int jumpPlace = 0, jumpAway = 0, jumpAlong = 0;
    bool trayArrowStyle = false;  // the overflow arrow from the visual style, not System Icons' own
    // Previews open on resting on a button (0), on resting on a group (1), or on a click only (2);
    // placed on the button (0) or the pointer (1), then moved away from the bar and along it (px
    // at 100%); closed once the pointer leaves (after previewLeaveDelay ms), or kept until
    // dismissed in the ways chosen (previewCloseAfter: s the pointer is away, 0 never).
    int previewOpen = 0, previewPlace = 0, previewAway = 0, previewAlong = 0;
    bool previewSticky = false;
    int previewLeaveDelay = 300, previewCloseAfter = 0;
    bool previewCloseClick = true, previewCloseFocus = true, previewCloseEsc = true, previewCloseKey = false;
    bool previewKeepAfterPick = false;
    bool dynamicTransparency = false, dynamicSegments = false, dynamicAura = false;
    int plumpness = 0;        // 0 off, 1 buffer, 2 aura
    int auraFollow = 0;       // ms for the dynamic aura to catch up halfway; 0 sticks to the pointer
    int auraSize = 100, auraStrength = 100;  // % of the Aura look's reach and strength
    // Tooltips: 0 by the pointer (Windows' own place), 1 beside the bar over what's pointed at;
    // then moved away from the bar, and along it, by these (px at 100%); shown after this delay
    // (ms, 0 Windows' own).
    int tipPlace = 0, tipAway = 0, tipAlong = 0, tipDelay = 0;
    UINT hotkeyMods = 0, hotkeyVk = 0;  // toggles automatic hiding
    // The Tourne Menu (tourne-menu.cpp): its button on the desktop (0 none, 1 middle, 2 right), the bar's
    // empty space too, its hotkey, and its rows (none: LiteMenu's own menu).
    bool menuEnabled = false, menuOnBar = false;
    int menuButton = 1;
    UINT menuHotkeyMods = 0, menuHotkeyVk = 0;
    std::vector<MenuRow> menuRows;
    UINT nextDesktopMods = 0, nextDesktopVk = 0, prevDesktopMods = 0, prevDesktopVk = 0;
    UINT goToDesktopMods = 0;           // with 1 to 9; 0 for none
    UINT moveNextMods = 0, moveNextVk = 0, movePrevMods = 0, movePrevVk = 0;
    UINT moveToDesktopMods = 0;         // with 1 to 9: the window in front sent there; 0 for none
    bool followMoved = false;           // and switched to that desktop with it
    bool sabBackdrop = true;  // StartAllBack's own bar gradient behind clear styles
    bool coloring = false;    // custom background colour
    COLORREF tint = 0;
    int tintAlpha = 255;
    int blur = 0;             // 0 off, 1 blur, 2 acrylic
    bool hideSystemTaskbar = false;
    bool disableStartAllBack = false;  // StartAllBack's (Explorer's) taskbar hidden
    std::vector<std::pair<std::wstring, std::wstring>> buttonIcons;  // program (lower case), icon
};

std::wstring ReadStartIsBack(PCWSTR name) {
    WCHAR value[MAX_PATH * 2] = L"";
    DWORD size = sizeof(value);
    if (RegGetValueW(HKEY_CURRENT_USER, L"Software\\StartIsBack", name, RRF_RT_REG_SZ, nullptr, value, &size) !=
        ERROR_SUCCESS) {
        return {};
    }
    return value;
}

std::wstring Expand(PCWSTR path) {
    WCHAR out[MAX_PATH * 2];
    ExpandEnvironmentStringsW(path, out, ARRAYSIZE(out));
    return out;
}

bool FileExists(std::wstring const& path) {
    DWORD a = GetFileAttributesW(path.c_str());
    return a != INVALID_FILE_ATTRIBUTES && !(a & FILE_ATTRIBUTE_DIRECTORY);
}

// Where themes and orbs are found: StartAllBack's folders when it's
// installed, and Tourne's own, which work without it.
std::wstring SabStyles() { return Expand(L"%ProgramFiles%\\StartAllBack\\Styles\\"); }
std::wstring TourneStyles() { return Expand(L"%LOCALAPPDATA%\\Tourne\\Styles\\"); }
std::wstring TourneYasb() { return Expand(L"%LOCALAPPDATA%\\Tourne\\Yasb\\"); }
std::wstring TourneFonts() { return Expand(L"%LOCALAPPDATA%\\Tourne\\Fonts\\"); }
std::vector<std::wstring> OrbFolders() {
    return {Expand(L"%ProgramFiles%\\StartAllBack\\Orbs\\"), Expand(L"%LOCALAPPDATA%\\StartAllBack\\Orbs\\"),
            Expand(L"%LOCALAPPDATA%\\Tourne\\Orbs\\")};
}

void EnsureTourneFolders() {
    CreateDirectoryW(Expand(L"%LOCALAPPDATA%\\Tourne").c_str(), nullptr);
    CreateDirectoryW(Expand(L"%LOCALAPPDATA%\\Tourne\\Styles").c_str(), nullptr);
    CreateDirectoryW(Expand(L"%LOCALAPPDATA%\\Tourne\\Orbs").c_str(), nullptr);
    CreateDirectoryW(Expand(L"%LOCALAPPDATA%\\Tourne\\Yasb").c_str(), nullptr);
    CreateDirectoryW(Expand(L"%LOCALAPPDATA%\\Tourne\\Fonts").c_str(), nullptr);
}

// A theme name (as StartAllBack stores it) to its file, in either folder.
std::wstring FindStyle(std::wstring const& name) {
    if (name.empty()) return {};
    if (name.find(L'\\') != std::wstring::npos) return FileExists(name) ? name : L"";
    for (auto const& folder : {SabStyles(), TourneStyles()})
        if (FileExists(folder + name)) return folder + name;
    return {};
}

// The build files this under the part's settings (its name is in the build's
// list), so every read gets the "taskbar." prefix.
std::wstring TextSetting(PCWSTR name) {
    PCWSTR value = Wh_GetStringSetting(name);
    std::wstring s = value ? value : L"";
    Wh_FreeStringSetting(value);
    return s;
}

// System Icons' own setting, read by its full name (a variable, so the build
// doesn't file it under this part).
constexpr PCWSTR kIconsScheme = L"systemIcons.scheme";

bool ParseHexColor(std::wstring text, COLORREF* color);

// StartAllBack's own DWORD values (colour is a COLORREF).
bool ReadStartIsBackDword(PCWSTR name, DWORD* value) {
    DWORD bytes = sizeof(DWORD);
    return RegGetValueW(HKEY_CURRENT_USER, L"Software\\StartIsBack", name, RRF_RT_REG_DWORD, nullptr, value, &bytes) ==
           ERROR_SUCCESS;
}

Settings LoadSettings() {
    Settings s;
    std::wstring style = TextSetting(L"style");
    std::wstring orb = TextSetting(L"orb");

    std::wstring scheme = TextSetting(L"scheme");
    if (scheme.empty() || scheme == L"matchIcons") {
        PCWSTR icons = Wh_GetStringSetting(kIconsScheme);
        scheme = icons ? icons : L"";
        Wh_FreeStringSetting(icons);
    }
    s.palette = SuiteSchemePalette(scheme);

    std::wstring location = TextSetting(L"location");
    s.edge = location == L"bottom" ? Edge::Bottom : location == L"left" ? Edge::Left
           : location == L"right"  ? Edge::Right  : Edge::Top;
    // "center" once also held where the Start button went; old values still read.
    std::wstring center = TextSetting(L"center");
    std::wstring startAt = TextSetting(L"startPosition");
    s.centered = center != L"edge" && center != L"off";
    s.startAt = startAt == L"withTasks" ? StartAt::WithTasks : startAt == L"end" ? StartAt::End
              : startAt.empty() && center == L"together" ? StartAt::WithTasks : StartAt::Edge;
    std::wstring combine = TextSetting(L"combine");
    s.combine = combine == L"whenFull"        ? Combine::WhenFull
              : combine == L"never"           ? Combine::Never
              : combine == L"neverHideLabels" ? Combine::NeverHideLabels
              : combine == L"neverGroup"      ? Combine::NeverGroup
                                              : Combine::Always;
    std::wstring segments = TextSetting(L"segments");
    s.segments = segments == L"three" ? Segments::Three : segments == L"central" ? Segments::Central : Segments::Off;
    std::wstring size = TextSetting(L"iconSize");
    s.iconSize = size == L"xsmall" ? 12 : size == L"medium" ? 24 : size == L"large" ? 32 : size == L"xlarge" ? 48 : 16;
    std::wstring margins = TextSetting(L"iconMargins");
    s.margin = margins == L"xsmall" ? 3 : margins == L"medium" ? 12 : margins == L"large" ? 16
             : margins == L"xlarge" ? 24 : 8;
    int thickness = Wh_GetIntSetting(L"thickness");
    s.thickness = thickness <= 0 ? 40 : std::clamp(thickness, 20, 96);
    s.autoHide = Wh_GetIntSetting(L"autoHide") != 0;
    // Buttons on all taskbars, or only where the window is (as Windows is set).
    DWORD mmEnabled = 0, mmMode = 0, bytes = sizeof(DWORD);
    constexpr WCHAR kAdvanced[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced";
    RegGetValueW(HKEY_CURRENT_USER, kAdvanced, L"MMTaskbarEnabled", RRF_RT_REG_DWORD, nullptr, &mmEnabled, &bytes);
    bytes = sizeof(DWORD);
    RegGetValueW(HKEY_CURRENT_USER, kAdvanced, L"MMTaskbarMode", RRF_RT_REG_DWORD, nullptr, &mmMode, &bytes);
    s.onlyThisScreen = mmEnabled && mmMode == 2;
    // A bar per screen: as Windows' "Show my taskbar on all displays" is set, or always, or never. Forced on
    // without Windows' setting, each bar shows its own screen's windows.
    std::wstring multi = TextSetting(L"multiMonitor");
    s.multiBars = multi == L"always" || (multi != L"never" && mmEnabled);
    s.mmMode = mmEnabled ? (int)std::min<DWORD>(mmMode, 2) : 2;
    s.tray = TextSetting(L"tray") != L"hidden";
    s.trayEverywhere = Wh_GetIntSetting(L"trayEverywhere") != 0;
    {
        std::wstring sd = TextSetting(L"showDesktop");
        if (sd == L"always") s.showDesktop = true;
        else if (sd == L"never") s.showDesktop = false;
        else {
            // Windows' own "far corner shows the desktop" (on unless turned off).
            DWORD corner = 1, size = sizeof(corner);
            RegGetValueW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced",
                         L"TaskbarSd", RRF_RT_REG_DWORD, nullptr, &corner, &size);
            s.showDesktop = corner != 0;
        }
    }
    s.desktops = Wh_GetIntSetting(L"desktops") != 0;
    {
        auto& l = s.pagerLook;
        auto rgb = [](std::wstring const& text, uint32_t fallback) {
            COLORREF c;
            return ParseHexColor(text, &c) ? (uint32_t)(GetRValue(c) << 16 | GetGValue(c) << 8 | GetBValue(c)) : fallback;
        };
        // (0 for size and opacity: not set yet, so the defaults.)
        int size = Wh_GetIntSetting(L"desktopsLook.size");
        l.size = size ? std::clamp(size, 2, 24) : 6;
        l.spacing = std::clamp(Wh_GetIntSetting(L"desktopsLook.spacing"), 0, 40);
        std::wstring shape = TextSetting(L"desktopsLook.shape");
        PCWSTR shapes[] = {L"pixel", L"square", L"circle", L"diamond", L"pill", L"bar"};
        l.shape = 0;
        for (int i = 0; i < 6; i++)
            if (shape == shapes[i]) l.shape = i;
        l.outline = std::clamp(Wh_GetIntSetting(L"desktopsLook.outline"), 0, 6);
        l.custom = TextSetting(L"desktopsLook.colors") == L"custom";
        l.mark = rgb(TextSetting(L"desktopsLook.markColor"), 0x4F8F6A);
        l.markOutline = rgb(TextSetting(L"desktopsLook.markOutline"), 0x1E2F36);
        l.current = rgb(TextSetting(L"desktopsLook.currentColor"), 0x15FFA2);
        l.currentOutline = rgb(TextSetting(L"desktopsLook.currentOutline"), 0x66EA97);
        l.panel = rgb(TextSetting(L"desktopsLook.panelColor"), 0x121C21);
        l.panelEdge = rgb(TextSetting(L"desktopsLook.panelEdge"), 0x4F8F6A);
        std::wstring panel = TextSetting(L"desktopsLook.panel");
        l.panelShape = panel == L"square" ? 1 : panel == L"pill" ? 2 : panel == L"none" ? 3 : 0;
        l.panelOpacity = std::clamp(Wh_GetIntSetting(L"desktopsLook.panelOpacity"), 0, 100);
        int opacity = Wh_GetIntSetting(L"desktopsLook.opacity");
        l.opacity = opacity ? std::clamp(opacity, 10, 100) : 100;
        l.fade = TextSetting(L"desktopsLook.fade") == L"idle";
        l.fadeTo = std::clamp(Wh_GetIntSetting(L"desktopsLook.fadeTo"), 0, 100);
        l.fadeDelay = std::clamp(Wh_GetIntSetting(L"desktopsLook.fadeDelay"), 0, 10000);
        l.vertical = Wh_GetIntSetting(L"desktopsLook.vertical") != 0;
        l.previews = Wh_GetIntSetting(L"desktopsLook.previews") != 0;
        int previewWidth = Wh_GetIntSetting(L"desktopsLook.previewWidth");
        l.previewWidth = previewWidth ? std::clamp(previewWidth, 160, 640) : 280;
    }
    {
        std::wstring yasb = TextSetting(L"yasbTheme");
        // A folder name only: nothing that walks out of the themes' folder.
        bool plain = yasb.find_first_of(L"\\/:") == std::wstring::npos && yasb.find(L"..") == std::wstring::npos;
        s.yasbTheme = yasb == L"none" || !plain ? L"" : yasb;
        s.yasbFill = Wh_GetIntSetting(L"yasbFill") != 0;
        s.yasbCommands = Wh_GetIntSetting(L"yasbCommands") != 0;
        s.yasbNetwork = Wh_GetIntSetting(L"yasbNetwork") != 0;
        s.yasbFades = Wh_GetIntSetting(L"yasbFades") != 0;
    }
    std::wstring trayMargins = TextSetting(L"trayMargins");
    s.trayMargin = trayMargins == L"xsmall" ? 1 : trayMargins == L"medium" ? 5 : trayMargins == L"large" ? 8
                 : trayMargins == L"xlarge" ? 12 : 3;
    s.clock = TextSetting(L"clock") != L"hidden";
    s.shy = Wh_GetIntSetting(L"dynamicShyness") != 0;
    s.shyTolerance = std::clamp(Wh_GetIntSetting(L"shyTolerance"), 0, 200);
    std::wstring shyAnim = TextSetting(L"shyAnimation");
    s.shyAnim = shyAnim == L"none" ? 0 : shyAnim == L"fade" ? 2 : shyAnim == L"slidefade" ? 3 : 1;
    std::wstring shyEase = TextSetting(L"shySmoothing");
    s.shyEase = shyEase == L"off" ? 0 : shyEase == L"gentle" ? 1 : shyEase == L"soft" ? 2 : 3;
    s.shyDuration = std::clamp(Wh_GetIntSetting(L"shyDuration"), 0, 1000);
    // A motion line for the bar (or a parent: layers, global) sets the
    // hiding animation's curve, length and style instead.
    motion::Forget();
    s.motionBar = motion::Get(L"bar");
    s.motionPopups = motion::Get(L"popups");
    s.motionStart = motion::Get(L"start");
    s.motionPreviewIn = motion::Get(L"fadePopupsIn");
    s.motionDesksIn = motion::Get(L"workspacesIn");
    s.motionDesksOut = motion::Get(L"workspacesOut");
    s.motionLayersIn = motion::Get(L"fadeLayersIn");
    s.motionLayersOut = motion::Get(L"fadeLayersOut");
    s.motionScreenAdded = motion::Get(L"monitorAdded");
    s.menuAnimation = MenuMotionStyle(TextSetting(L"menuAnimation"));
    if (s.motionPopups.set) s.menuAnimation = s.motionPopups.on ? MenuMotionStyleWord(motion::StyleWord(s.motionPopups), s.menuAnimation) : 0;
    s.menuRules.hide = MenuRuleList(TextSetting(L"menuHide"));
    s.menuRules.more = MenuRuleList(TextSetting(L"menuMore"));
    if (std::wstring label = TextSetting(L"menuMoreLabel"); !label.empty()) s.menuRules.moreLabel = label;
    s.desktopMenuReady = Wh_GetIntSetting(L"desktopMenuReady") != 0;
    if (s.motionBar.set) {
        std::wstring style = motion::StyleWord(s.motionBar);
        if (!s.motionBar.on) s.shyAnim = 0;
        else if (style == L"slide") s.shyAnim = 1;
        else if (style == L"fade") s.shyAnim = 2;
        else if (style == L"slidefade") s.shyAnim = 3;
        else if (s.shyAnim == 0) s.shyAnim = 1;  // (on, with no style of its own: it slides, not waits and jumps)
    }
    s.shyHideDelay = std::clamp(Wh_GetIntSetting(L"shyHideDelay"), 0, 5000);
    s.shyShowDelay = std::clamp(Wh_GetIntSetting(L"shyShowDelay"), 0, 5000);
    std::wstring shyEdge = TextSetting(L"shyEdge");
    s.shyEdge = shyEdge == L"glow" ? 1 : shyEdge == L"none" ? 2 : 0;
    s.shyGlow = std::clamp(Wh_GetIntSetting(L"shyGlow"), 5, 100);
    s.shyGlowSize = std::clamp(Wh_GetIntSetting(L"shyGlowSize"), 2, 12);
    s.shyRevealZone = std::clamp(Wh_GetIntSetting(L"shyRevealZone"), 0, 200);
    s.shyRevealAttention = Wh_GetIntSetting(L"shyRevealAttention") != 0;
    s.shyRevealStart = Wh_GetIntSetting(L"shyRevealStart") != 0;
    s.fullscreenStayAway = Wh_GetIntSetting(L"fullscreenStayAway") != 0;
    s.emptyDoubleClick = TextSetting(L"emptyDoubleClick");
    if (s.emptyDoubleClick.empty()) s.emptyDoubleClick = L"show-desktop";
    s.emptyMiddleClick = TextSetting(L"emptyMiddleClick");
    s.emptyMiddleDoubleClick = TextSetting(L"emptyMiddleDoubleClick");
    if (s.emptyMiddleClick.empty()) s.emptyMiddleClick = L"none";
    if (s.emptyMiddleDoubleClick.empty()) s.emptyMiddleDoubleClick = L"none";
    for (int i = 0; i < 64; i++) {
        PCWSTR value = Wh_GetStringSetting(L"fullscreenPrograms[%d]", i);
        std::wstring program = value ? value : L"";
        Wh_FreeStringSetting(value);
        size_t first = program.find_first_not_of(L" \t"), last = program.find_last_not_of(L" \t");
        program = first == std::wstring::npos ? L"" : program.substr(first, last - first + 1);
        CharLowerBuffW(program.data(), (DWORD)program.size());
        if (!program.empty()) s.fullscreenPrograms.push_back(program);  // (an empty row in the middle doesn't end the list)
    }
    s.idleFade = Wh_GetIntSetting(L"idleFade") != 0;
    s.idleAfter = std::clamp(Wh_GetIntSetting(L"idleAfter"), 0, 600);
    s.idleOpacity = std::clamp(Wh_GetIntSetting(L"idleOpacity"), 0, 100);
    std::wstring idleWhen = TextSetting(L"idleWhen");
    s.idleWhen = idleWhen == L"clear" ? 1 : idleWhen == L"touching" ? 2 : 0;
    s.idleNear = std::clamp(Wh_GetIntSetting(L"idleNear"), 0, 400);
    s.idleFadeTime = std::clamp(Wh_GetIntSetting(L"idleFadeTime"), 0, 5000);
    s.idleWakeTime = std::clamp(Wh_GetIntSetting(L"idleWakeTime"), 0, 2000);
    s.idleWakeAttention = Wh_GetIntSetting(L"idleWakeAttention") != 0;
    std::wstring shadows = TextSetting(L"shadows");
    s.shadows = shadows == L"icons" ? 1 : shadows == L"frames" ? 2 : shadows == L"both" ? 3 : 0;
    if (!ParseHexColor(TextSetting(L"shadowColor"), &s.shadowColor)) s.shadowColor = 0;
    s.shadowStrength = std::clamp(Wh_GetIntSetting(L"shadowStrength"), 0, 100);
    s.shadowSoftness = std::clamp(Wh_GetIntSetting(L"shadowSoftness"), 0, 8);
    s.shadowX = std::clamp(Wh_GetIntSetting(L"shadowX"), -8, 8);
    s.shadowY = std::clamp(Wh_GetIntSetting(L"shadowY"), -8, 8);
    s.previews = TextSetting(L"previews") != L"off";
    s.previewDelay = std::clamp(Wh_GetIntSetting(L"previewDelay"), 0, 2000);
    s.previewLook = TextSetting(L"previewLook") == L"custom" ? 1 : 0;
    auto colorOr = [](std::wstring const& text) {
        COLORREF c;
        return ParseHexColor(text, &c) ? c : CLR_INVALID;
    };
    s.previewBg = colorOr(TextSetting(L"previewBackground"));
    s.previewTextColor = colorOr(TextSetting(L"previewText"));
    s.previewHover = colorOr(TextSetting(L"previewHover"));
    s.previewHoverBorder = colorOr(TextSetting(L"previewHoverBorder"));
    s.previewActive = colorOr(TextSetting(L"previewActive"));
    s.previewFlashing = colorOr(TextSetting(L"previewFlashing"));
    s.previewCloseHover = colorOr(TextSetting(L"previewCloseHover"));
    s.previewCloseGlyph = colorOr(TextSetting(L"previewCloseGlyph"));
    std::wstring previewBorder = TextSetting(L"previewBorder");
    s.previewBorderNone = previewBorder == L"none";
    s.previewBorder = colorOr(previewBorder);
    s.previewBorderW = std::clamp(Wh_GetIntSetting(L"previewBorderWidth"), 0, 8);
    s.previewThumbW = std::clamp(Wh_GetIntSetting(L"previewThumbWidth"), 80, 480);
    s.previewThumbH = std::clamp(Wh_GetIntSetting(L"previewThumbHeight"), 48, 360);
    s.previewPad = std::clamp(Wh_GetIntSetting(L"previewPadding"), 0, 32);
    s.previewEdge = std::clamp(Wh_GetIntSetting(L"previewEdge"), 0, 40);
    std::wstring corners = TextSetting(L"previewCorners");
    s.previewCorners = corners == L"small" ? 1 : corners == L"round" ? 2 : 0;
    s.previewShadow = Wh_GetIntSetting(L"previewShadow") != 0;
    std::wstring backdrop = TextSetting(L"previewBackdrop");
    s.previewBackdrop = backdrop == L"blur" ? 1 : backdrop == L"acrylic" ? 2 : backdrop == L"mica" ? 3 : 0;
    s.previewOpacity = std::clamp(Wh_GetIntSetting(L"previewOpacity"), 0, 100);
    std::wstring jumpPlace = TextSetting(L"jumpListPlace");
    s.jumpPlace = jumpPlace == L"start" ? 1 : jumpPlace == L"centre" ? 2 : 0;
    s.jumpAway = std::clamp(Wh_GetIntSetting(L"jumpListAway"), -200, 400);
    s.jumpAlong = std::clamp(Wh_GetIntSetting(L"jumpListAlong"), -800, 800);
    s.trayArrowStyle = TextSetting(L"trayArrow") == L"style";
    std::wstring previewOpen = TextSetting(L"previewOpen");
    s.previewOpen = previewOpen == L"groups" ? 1 : previewOpen == L"click" ? 2 : 0;
    s.previewPlace = TextSetting(L"previewPlace") == L"pointer" ? 1 : 0;
    s.previewAway = std::clamp(Wh_GetIntSetting(L"previewAway"), -200, 400);
    s.previewAlong = std::clamp(Wh_GetIntSetting(L"previewAlong"), -800, 800);
    s.previewSticky = TextSetting(L"previewStay") == L"dismissed";
    s.previewLeaveDelay = std::clamp(Wh_GetIntSetting(L"previewLeaveDelay"), 0, 5000);
    s.previewCloseClick = Wh_GetIntSetting(L"previewCloseClick") != 0;
    s.previewCloseFocus = Wh_GetIntSetting(L"previewCloseFocus") != 0;
    s.previewCloseEsc = Wh_GetIntSetting(L"previewCloseEsc") != 0;
    s.previewCloseKey = Wh_GetIntSetting(L"previewCloseKey") != 0;
    s.previewCloseAfter = std::clamp(Wh_GetIntSetting(L"previewCloseAfter"), 0, 600);
    s.previewKeepAfterPick = TextSetting(L"previewAfterPick") == L"stay";
    s.previewPeek = Wh_GetIntSetting(L"previewPeek") != 0;
    s.peekDelay = std::clamp(Wh_GetIntSetting(L"peekDelay"), 0, 3000);
    s.dynamicTransparency = Wh_GetIntSetting(L"dynamicTransparency") != 0;
    s.dynamicSegments = Wh_GetIntSetting(L"dynamicSegments") != 0;
    s.dynamicAura = Wh_GetIntSetting(L"dynamicAura") != 0;
    s.auraFollow = std::clamp(Wh_GetIntSetting(L"auraFollow"), 0, 1000);
    s.auraSize = std::clamp(Wh_GetIntSetting(L"auraSize"), 40, 300);
    s.auraStrength = std::clamp(Wh_GetIntSetting(L"auraStrength"), 0, 300);
    std::wstring tipPlace = TextSetting(L"tooltipPlace");
    s.tipPlace = tipPlace == L"bar" ? 1 : 0;
    s.tipAway = std::clamp(Wh_GetIntSetting(L"tooltipAway"), -200, 400);
    s.tipAlong = std::clamp(Wh_GetIntSetting(L"tooltipAlong"), -400, 400);
    s.tipDelay = std::clamp(Wh_GetIntSetting(L"tooltipDelay"), 0, 5000);
    std::wstring plumpness = TextSetting(L"plumpness");
    s.plumpness = plumpness == L"aura" ? 2 : plumpness == L"buffer" ? 1 : 0;
    if (s.plumpness != 0) s.thickness = std::max(s.thickness, 38);  // StartAllBack's plumpness is a taller bar
    // "alt+win+84": modifiers, then the virtual-key code in decimal.
    // (Each name written out at its call, where the build adds the part's prefix.)
    auto hotkey = [](std::wstring const& key, UINT* mods, UINT* vk) {
        size_t at = 0;
        while (at <= key.size()) {
            size_t end = key.find(L'+', at);
            std::wstring part = key.substr(at, end == std::wstring::npos ? end : end - at);
            if (part == L"ctrl") *mods |= MOD_CONTROL;
            else if (part == L"alt") *mods |= MOD_ALT;
            else if (part == L"shift") *mods |= MOD_SHIFT;
            else if (part == L"win") *mods |= MOD_WIN;
            else if (!part.empty()) *vk = (UINT)_wtoi(part.c_str());
            if (end == std::wstring::npos) break;
            at = end + 1;
        }
    };
    hotkey(TextSetting(L"hideHotkey"), &s.hotkeyMods, &s.hotkeyVk);
    s.menuEnabled = Wh_GetIntSetting(L"menu.enabled") != 0;
    std::wstring menuButton = TextSetting(L"menu.button");
    s.menuButton = menuButton == L"right" ? 2 : menuButton == L"none" ? 0 : 1;
    s.menuOnBar = Wh_GetIntSetting(L"menu.onBar") != 0;
    hotkey(TextSetting(L"menu.hotkey"), &s.menuHotkeyMods, &s.menuHotkeyVk);
    for (int i = 0; i < 500; i++) {
        MenuRow row;
        PCWSTR name = Wh_GetStringSetting(L"menu.items[%d].name", i);
        row.name = name ? name : L"";
        Wh_FreeStringSetting(name);
        PCWSTR command = Wh_GetStringSetting(L"menu.items[%d].command", i);
        row.command = command ? command : L"";
        Wh_FreeStringSetting(command);
        if (row.name.empty() && row.command.empty()) break;
        PCWSTR args = Wh_GetStringSetting(L"menu.items[%d].args", i);
        row.args = args ? args : L"";
        Wh_FreeStringSetting(args);
        row.depth = std::clamp(Wh_GetIntSetting(L"menu.items[%d].depth", i), 0, 16);
        s.menuRows.push_back(std::move(row));
    }
    hotkey(TextSetting(L"desktopKeys.next"), &s.nextDesktopMods, &s.nextDesktopVk);
    hotkey(TextSetting(L"desktopKeys.previous"), &s.prevDesktopMods, &s.prevDesktopVk);
    hotkey(TextSetting(L"desktopKeys.moveNext"), &s.moveNextMods, &s.moveNextVk);
    hotkey(TextSetting(L"desktopKeys.movePrevious"), &s.movePrevMods, &s.movePrevVk);
    auto numberMods = [](std::wstring const& v) -> UINT {
        return v == L"alt"        ? MOD_ALT
               : v == L"ctrlAlt"  ? MOD_CONTROL | MOD_ALT
               : v == L"shiftAlt" ? MOD_SHIFT | MOD_ALT
               : v == L"winAlt"   ? MOD_WIN | MOD_ALT
                                  : 0;
    };
    s.goToDesktopMods = numberMods(TextSetting(L"desktopKeys.goTo"));
    s.moveToDesktopMods = numberMods(TextSetting(L"desktopKeys.moveTo"));
    s.followMoved = Wh_GetIntSetting(L"desktopKeys.follow") != 0;
    s.sabBackdrop = TextSetting(L"barBackground") != L"scheme";
    std::wstring coloring = TextSetting(L"coloring");
    if (coloring == L"custom") {
        s.coloring = ParseHexColor(TextSetting(L"coloringColor"), &s.tint);
        s.tintAlpha = std::clamp(Wh_GetIntSetting(L"coloringAlpha"), 0, 255);
        std::wstring blur = TextSetting(L"coloringBlur");
        s.blur = blur == L"acrylic" ? 2 : blur == L"blur" ? 1 : 0;
    } else if (coloring != L"none") {
        // What StartAllBack has set (unset means no colouring).
        DWORD on = 0, color = 0, alpha = 255, blur = 0;
        ReadStartIsBackDword(L"TaskbarColoring", &on);
        if (on && ReadStartIsBackDword(L"TaskbarColor", &color)) {
            s.coloring = true;
            s.tint = color & 0xFFFFFF;
            if (ReadStartIsBackDword(L"TaskbarAlpha", &alpha)) s.tintAlpha = std::min<DWORD>(alpha, 255);
            ReadStartIsBackDword(L"TaskbarBlur", &blur);
            s.blur = blur == 0 ? 0 : blur == 1 ? 1 : 2;
        }
    }
    s.hideSystemTaskbar = Wh_GetIntSetting(L"hideSystemTaskbar") != 0;
    s.disableStartAllBack = Wh_GetIntSetting(L"disableStartAllBack") != 0;
    // Menu No Icons' own switch (through a variable, so the build doesn't file
    // it under this part).
    constexpr PCWSTR kMenuNoIconsSetting = L"menuNoIcons.enabled";
    s.menuIcons = Wh_GetIntSetting(kMenuNoIconsSetting) == 0;
    std::wstring previewIcons = TextSetting(L"previewIcons");
    s.previewIcons = previewIcons == L"show" ? true : previewIcons == L"hide" ? false : s.menuIcons;
    for (int i = 0; i < 64; i++) {
        PCWSTR program = Wh_GetStringSetting(L"buttonIcons[%d].program", i);
        std::wstring match = program ? program : L"";
        Wh_FreeStringSetting(program);
        CharLowerBuffW(match.data(), (DWORD)match.size());
        PCWSTR icon = Wh_GetStringSetting(L"buttonIcons[%d].icon", i);
        std::wstring file = icon && *icon ? Expand(icon) : L"";
        Wh_FreeStringSetting(icon);
        if (match.empty()) {
            if (i > 0) break;
            continue;
        }
        if (!file.empty()) s.buttonIcons.push_back({match, file});
    }

    // A preset from the Welcome page decides the style and orb while chosen,
    // as StartAllBack's presets set them.
    std::wstring preset = TextSetting(L"preset");
    if (preset == L"proper11") {
        style = L"none";
        orb = L"default";  // Windows' own button, not StartAllBack's orb
    } else if (preset == L"kinda10") {
        style = L"Plain8.msstyles";
        orb = Expand(L"%ProgramFiles%\\StartAllBack\\Orbs\\w8logo.svg");
    } else if (preset == L"remastered7") {
        style = L"Windows 7.msstyles";
        orb = Expand(L"%ProgramFiles%\\StartAllBack\\Orbs\\Windows 7.orb");
    }
    // "startallback" (or unset): whatever StartAllBack is set to; "none": flat.
    if (style.empty() || style == L"startallback") style = ReadStartIsBack(L"TaskbarStyle");
    s.style = style == L"none" ? L"" : FindStyle(style);
    if (orb.empty() || orb == L"startallback") orb = ReadStartIsBack(L"OrbBitmap");
    s.orb = FileExists(orb) ? orb : L"";
    return s;
}

// Fills the Visual style and Start orb dropdowns (Windhawk 2.0's
// $dynamicSelect) with what's installed. The paths are the settings' full
// names inside the suite.
void PublishChoices() {
    // Written only when the list changed: each is a write to Windhawk's storage.
    static std::wstring published;
    std::vector<std::pair<std::wstring, std::wstring>> options;
    auto option = [&](PCWSTR setting, std::wstring const& value, std::wstring const& label) {
        options.push_back({L"::wh_select_option::" + std::wstring(setting) + L"::" + value, label});
    };
    option(L"taskbar.style", L"startallback", L"Same as StartAllBack");
    option(L"taskbar.style", L"none", L"None: flat, in the colour scheme");
    option(L"taskbar.orb", L"startallback", L"Same as StartAllBack");
    option(L"taskbar.startMenu.visualStyle", L"startallback", L"Same as StartAllBack's Start menu");
    option(L"taskbar.startMenu.visualStyle", L"taskbar", L"Same as the taskbar");
    option(L"taskbar.startMenu.visualStyle", L"none", L"None: flat, in the colour scheme");
    auto each = [](std::wstring const& folder, PCWSTR pattern, auto&& fn) {
        WIN32_FIND_DATAW fd;
        HANDLE find = FindFirstFileW((folder + pattern).c_str(), &fd);
        if (find == INVALID_HANDLE_VALUE) return;
        do {
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) fn(folder + fd.cFileName, std::wstring(fd.cFileName));
        } while (FindNextFileW(find, &fd));
        FindClose(find);
    };
    auto stem = [](std::wstring const& name) { return name.substr(0, name.rfind(L'.')); };
    // StartAllBack's own are stored by name, as StartAllBack stores them.
    each(SabStyles(), L"*.msstyles", [&](std::wstring const&, std::wstring const& name) {
        option(L"taskbar.style", name, stem(name));
        option(L"taskbar.startMenu.visualStyle", name, stem(name));
    });
    each(TourneStyles(), L"*.msstyles", [&](std::wstring const& path, std::wstring const& name) {
        option(L"taskbar.style", path, stem(name) + L" (Tourne)");
        option(L"taskbar.startMenu.visualStyle", path, stem(name) + L" (Tourne)");
    });
    option(L"taskbar.yasbTheme", L"none", L"None: the normal bar");
    {
        // A YASB theme is a folder with a styles.css in it.
        std::wstring folder = TourneYasb();
        WIN32_FIND_DATAW fd;
        HANDLE find = FindFirstFileW((folder + L"*").c_str(), &fd);
        if (find != INVALID_HANDLE_VALUE) {
            do {
                std::wstring name = fd.cFileName;
                if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && name != L"." && name != L".." &&
                    GetFileAttributesW((folder + name + L"\\styles.css").c_str()) != INVALID_FILE_ATTRIBUTES)
                    option(L"taskbar.yasbTheme", name, name);
            } while (FindNextFileW(find, &fd));
            FindClose(find);
        }
    }
    // The PC's own speakers and mics, for a phone call's sound (Tourne Calls); a phone's hands-free ones aren't.
    option(L"taskbar.startMenu.phoneCallOutput", L"default", L"Windows' default for calls");
    option(L"taskbar.startMenu.phoneCallMic", L"default", L"Windows' default for calls");
    IMMDeviceEnumerator* devices = nullptr;
    if (SUCCEEDED(CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&devices))) {
        constexpr PROPERTYKEY kFriendlyName{{0xa45c254e, 0xdf1c, 0x4efd, {0x80, 0x20, 0x67, 0xd1, 0x46, 0xa8, 0x50, 0xe0}}, 14};
        for (auto [flow, setting] : {std::pair{eRender, L"taskbar.startMenu.phoneCallOutput"}, std::pair{eCapture, L"taskbar.startMenu.phoneCallMic"}}) {
            IMMDeviceCollection* all = nullptr;
            UINT count = 0;
            if (SUCCEEDED(devices->EnumAudioEndpoints(flow, DEVICE_STATE_ACTIVE, &all)) && SUCCEEDED(all->GetCount(&count)))
                for (UINT i = 0; i < count; i++) {
                    IMMDevice* d = nullptr;
                    IPropertyStore* props = nullptr;
                    LPWSTR id = nullptr;
                    if (SUCCEEDED(all->Item(i, &d)) && SUCCEEDED(d->GetId(&id)) && SUCCEEDED(d->OpenPropertyStore(STGM_READ, &props))) {
                        PROPVARIANT v;
                        PropVariantInit(&v);
                        if (SUCCEEDED(props->GetValue(kFriendlyName, &v)) && v.vt == VT_LPWSTR && !wcsstr(v.pwszVal, L"Hands-Free"))
                            option(setting, id, v.pwszVal);
                        PropVariantClear(&v);
                    }
                    if (id) CoTaskMemFree(id);
                    if (props) props->Release();
                    if (d) d->Release();
                }
            if (all) all->Release();
        }
        devices->Release();
    }
    for (auto const& folder : OrbFolders()) {
        for (PCWSTR pattern : {L"*.png", L"*.bmp"}) {
            each(folder, pattern, [&](std::wstring const& path, std::wstring const& name) {
                option(L"taskbar.orb", path, stem(name));
            });
        }
    }
    std::wstring all;
    for (auto const& [key, label] : options) all += key + L"=" + label + L"\n";
    if (all == published) return;
    published = all;
    for (auto const& [key, label] : options) Wh_SetStringValue(key.c_str(), label.c_str());
}

////////////////////////////////////////////////////////////////////////////////
// Windows and buttons

struct Win {
    HWND hwnd;
    std::wstring key;       // what groups windows: the app id, or the program file
    std::wstring appId;     // the app id Windows groups it under
    std::wstring shortcut;  // the shortcut Windows shows for that app
    std::wstring exe;
    HICON icon = nullptr;   // the window's own icon
    int iconPx = 0;         // its size
    bool ownsIcon = false;
    bool flashing = false;
};

struct Button {
    std::vector<HWND> windows;  // none for a pinned app that isn't running
    RECT rc{};
    int pin = -1;               // its place among the pinned apps
    int stack = 0;              // room at its end for the edge of the stack behind it (several windows)
};

constexpr UINT WM_APP_EXIT = WM_APP + 1;
constexpr UINT WM_APP_SETTINGS = WM_APP + 2;
constexpr UINT WM_APP_BAR = WM_APP + 3;
constexpr UINT WM_APP_ICONS = WM_APP + 4;  // read the app icons again
constexpr UINT WM_APP_SHYFRAME = WM_APP + 20;  // a frame of the hiding animation (see FramePacer)
constexpr UINT_PTR kTimerShy = 14;  // the hiding animation's frames, when its frame clock can't start
constexpr UINT_PTR kTimerRescan = 1;
constexpr UINT_PTR kTimerReload = 10;  // a theme or orb file, or StartAllBack's settings, changed: the bar reloads
constexpr UINT_PTR kTimerToast = 11;   // the message after installing a theme goes away
constexpr UINT_PTR kTimerGlow = 12;    // the hover glow fading in and out
constexpr UINT_PTR kTimerAutoHide = 2;
constexpr UINT_PTR kTimerTray = 3;
constexpr UINT_PTR kTimerTrayAgain = 4;  // a second read, in case Explorer lagged the first
constexpr UINT_PTR kTimerDragOpen = 7;   // a dragged tray icon resting on the overflow arrow opens it
constexpr int kHotkeyHide = 1;
constexpr int kHotkeyNextDesktop = 2, kHotkeyPrevDesktop = 3, kHotkeyDesktop1 = 11;  // (to kHotkeyDesktop1 + 8)
constexpr int kHotkeyMoveNext = 4, kHotkeyMovePrev = 5, kHotkeyMove1 = 21;            // (to kHotkeyMove1 + 8)
constexpr int kHotkeyMenu = 40;                                                          // the Tourne Menu
constexpr UINT_PTR kTimerDynamic = 9;       // is a window touching the bar (dynamic settings)
constexpr UINT_PTR kTimerSystemTaskbar = 8;  // keeps StartAllBack's taskbar hidden
constexpr UINT_PTR kTimerBootLog = 16;       // how the start went, written down (startup.txt)
constexpr UINT_PTR kTimerScreens = 17;       // screens came, went or woke: the bars follow, once they settle
constexpr UINT_PTR kTimerIdle = 18;          // fading when idle: is the pointer near, has the bar been left alone
constexpr WCHAR kWindowClass[] = L"TourneTaskbarTest_" WH_MOD_ID;

std::atomic<HWND> g_wnd;      // the main screen's bar: the one everything else talks to
HWND g_curWnd = nullptr;      // the bar being handled (see BarState)
HMONITOR g_curMonitor = nullptr;  // its screen; null for the main bar, which follows the main screen
HANDLE g_thread;
HANDLE g_hooksLive;  // set when Windhawk has put every hook in (Wh_ModAfterInit)

// How long the bar took to get where, from its process's start: kept in memory and written once
// to %LOCALAPPDATA%\Tourne\startup.txt, so a restart's real cost can be read afterwards. A few
// small records and one small file write per start.
struct BootStep {
    PCWSTR what;
    double ms;
};
SRWLOCK g_bootLock = SRWLOCK_INIT;
std::vector<BootStep> g_bootSteps;
bool g_bootWritten = false;

// Milliseconds since this process started.
double ProcessAgeMs() {
    FILETIME created, exited, kernel, user, now;
    if (!GetProcessTimes(GetCurrentProcess(), &created, &exited, &kernel, &user)) return 0;
    GetSystemTimePreciseAsFileTime(&now);
    auto ticks = [](FILETIME f) { return ((ULONGLONG)f.dwHighDateTime << 32) | f.dwLowDateTime; };
    return (ticks(now) - ticks(created)) / 10000.0;
}

void BootMark(PCWSTR what) {
    AcquireSRWLockExclusive(&g_bootLock);
    if (!g_bootWritten && g_bootSteps.size() < 32) g_bootSteps.push_back({what, ProcessAgeMs()});
    ReleaseSRWLockExclusive(&g_bootLock);
}

void WriteBootLog() {
    AcquireSRWLockExclusive(&g_bootLock);
    if (g_bootWritten) {
        ReleaseSRWLockExclusive(&g_bootLock);
        return;
    }
    g_bootWritten = true;
    std::vector<BootStep> steps = g_bootSteps;
    ReleaseSRWLockExclusive(&g_bootLock);
    // When this process started: after Windows started, and after Explorer did.
    double age = ProcessAgeMs();
    double sinceBoot = GetTickCount64() - age;
    double sinceExplorer = -1;
    DWORD pid = 0;
    if (HWND tray = FindWindowW(L"Shell_TrayWnd", nullptr)) GetWindowThreadProcessId(tray, &pid);
    if (HANDLE explorer = pid ? OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid) : nullptr) {
        FILETIME created, exited, kernel, user, now;
        if (GetProcessTimes(explorer, &created, &exited, &kernel, &user)) {
            GetSystemTimePreciseAsFileTime(&now);
            auto ticks = [](FILETIME f) { return ((ULONGLONG)f.dwHighDateTime << 32) | f.dwLowDateTime; };
            sinceExplorer = (ticks(now) - ticks(created)) / 10000.0 - age;
        }
        CloseHandle(explorer);
    }
    wchar_t line[160];
    std::wstring text;
    swprintf_s(line, L"Taskbar process started %.1f s after Windows started", sinceBoot / 1000);
    text += line;
    if (sinceExplorer >= 0) {
        swprintf_s(line, L" and %.1f s after Explorer", sinceExplorer / 1000);
        text += line;
    }
    text += L".\r\nFrom the process's start:\r\n";
    for (auto const& s : steps) {
        swprintf_s(line, L"%8.0f ms  %ls\r\n", s.ms, s.what);
        text += line;
    }
    std::wstring folder = Expand(L"%LOCALAPPDATA%\\Tourne");
    if (folder.empty()) return;
    HANDLE file = CreateFileW((folder + L"\\startup.txt").c_str(), GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS,
                              FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE) return;
    std::string utf8(WideCharToMultiByte(CP_UTF8, 0, text.c_str(), (int)text.size(), nullptr, 0, nullptr, nullptr), 0);
    WideCharToMultiByte(CP_UTF8, 0, text.c_str(), (int)text.size(), utf8.data(), (int)utf8.size(), nullptr, nullptr);
    DWORD written;
    WriteFile(file, utf8.data(), (DWORD)utf8.size(), &written, nullptr);
    CloseHandle(file);
}

Settings g_settings;
Theme g_theme;
bool g_themed = false;
Orb g_orb;
std::vector<Win> g_wins;
std::vector<Button> g_buttons;

// Progress apps show on their buttons (ITaskbarList3), passed on by the relay in
// Explorer, which is where apps send it. Shared by all the bars.
struct Progress {
    int state = 0;  // TBPF_*
    int value = 0;  // 0..65535
};
std::unordered_map<HWND, Progress> g_progress;
uint64_t g_barsDrawn = 0;  // what the once-a-second refresh last drew (see BarsSignature); 0 when drawn since
UINT g_msgProgress = 0;
UINT g_msgCrashGuard = 0;  // the crash guard kept the suite's parts out of Explorer
UINT g_msgShellStart = 0;  // Explorer's relay now sends Windows' Start here, or stopped
constexpr UINT_PTR kTimerProgress = 15;  // the busy sweep, only while one shows
constexpr int kBandProgressNormal = 9, kBandProgressBusy = 10, kBandProgressError = 11, kBandProgressPaused = 12;
bool g_labels = false;
UINT g_shellHookMessage;
int g_scale = 1;
RECT g_orbRect{};
std::vector<RECT> g_islands;   // segments: the floating islands
int g_centralIsland = -1;      // the task buttons' island
HWND g_tooltip;
HFONT g_font;
HFONT g_tipFont;  // the Tourne pixel font, for tooltips and menus
bool g_appBar = false;
bool g_shown = true;
bool g_underFullscreen = false;  // dropped behind a full-screen program on this bar's screen
DWORD g_fullscreenPid = 0;       // the last program in front FullscreenInFront looked up (all bars)
bool g_fullscreenNamed = false;  // and whether it's in the settings' list
bool g_touching = false;  // a window touches the bar (dynamic transparency, segments, aura)
bool g_hideFlip = false;  // the hotkey has turned automatic hiding the other way
// Hiding, animated: how far in the bar is (1 in view, 0 away), where and when the animation started;
// then the hint left at the edge (a glow, or a sliver after a fade) fading in once it's away.
double g_slide = 1, g_slideFrom = 1, g_slideStart = 0, g_hint = 0, g_hintStart = 0;
ULONGLONG g_wantSince = 0;  // since when the bar has wanted to go, or come back (the delays)
bool g_edgeHint = false;    // drawn as the glow at the edge instead of the bar
BYTE g_barAlpha = 255;      // the whole bar's opacity (fades), kept when it's drawn again
// Frames in step with the screen's refresh while the bar slides or fades: a worker waits on the
// compositor (DwmFlush) and posts one frame at a time, only while an animation runs.
struct FramePacer {
    HWND wnd = nullptr;
    std::atomic<bool> run{true}, posted{false};
};
std::shared_ptr<FramePacer> g_pacer;
bool g_inMenu = false;
// Pointer state: -2 none, -1 the orb, else a button index.
int g_hot = -2;
int g_pressed = -2;
bool g_tracking = false;
// The hover glow fades: in over about 90 ms, out over about 140, from wherever it had got to.
struct GlowFade {
    int button = -1;
    double level = 0;
};
GlowFade g_glowIn, g_glowOut;
DWORD g_glowTick = 0;
// The dynamic aura's centre, catching up with the pointer once per screen frame (on the
// frame clock the hiding animation uses), as quickly as the smoothing setting says.
struct AuraFollow {
    bool running = false, placed = false;
    double x = 0, y = 0, toX = 0, toY = 0, last = 0;
};
AuraFollow g_aura;
bool g_shyRunning = false;  // the hiding animation has the frame clock
// Fading when idle: the bar's level (1 awake, down to the idle opacity), moving towards its target on
// the same frame clock; and when the pointer (or anything else) last kept it awake.
struct IdleFadeState {
    bool running = false;
    double level = 1, from = 1, to = 1, start = 0;
    ULONGLONG lastActive = 0;
};
IdleFadeState g_idle;
double GlowLevel(int i) { return i == g_glowIn.button ? g_glowIn.level : i == g_glowOut.button ? g_glowOut.level : 0; }
bool g_swallowUp = false;  // the up of a double click already handled
bool g_startOnPress = false;  // the Start button's press opened (or closed) the menu; its release is ignored
std::vector<std::pair<UINT_PTR, RECT>> g_placed;  // the tooltips' tools as last placed
std::wstring g_placedContent;

int Px(int v) { return v * g_scale; }

Segments EffSegments() { return g_settings.dynamicSegments && g_touching ? Segments::Off : g_settings.segments; }
bool AuraOn() { return g_settings.plumpness == 2; }
bool BackdropClear() { return g_settings.dynamicTransparency && !g_touching; }
bool SlideOn() { return g_settings.shyAnim == 1 || g_settings.shyAnim == 3; }
bool FadeOn() { return g_settings.shyAnim >= 2; }

bool Vertical() {
    return g_settings.edge == Edge::Left || g_settings.edge == Edge::Right;
}

// anyDesktop: a window on another virtual desktop counts too (Windows cloaks those, DWM_CLOAKED_SHELL).
bool IsTaskWindow(HWND h, bool anyDesktop = false) {
    if (h == g_wnd || !IsWindowVisible(h)) return false;
    LONG_PTR ex = GetWindowLongPtrW(h, GWL_EXSTYLE);
    bool app = ex & WS_EX_APPWINDOW;
    if (!app && ((ex & WS_EX_TOOLWINDOW) || (ex & WS_EX_NOACTIVATE) || GetWindow(h, GW_OWNER))) return false;
    DWORD cloaked = 0;
    DwmGetWindowAttribute(h, DWMWA_CLOAKED, &cloaked, sizeof(cloaked));
    if (cloaked && !(anyDesktop && cloaked == DWM_CLOAKED_SHELL)) return false;
    WCHAR cls[64];
    GetClassNameW(h, cls, ARRAYSIZE(cls));
    if (!wcscmp(cls, L"Shell_TrayWnd") || !wcscmp(cls, L"Shell_SecondaryTrayWnd") || !wcscmp(cls, L"Progman") ||
        !wcscmp(cls, L"WorkerW") || !wcscmp(cls, kWindowClass)) {  // kWindowClass: our bars, one per screen
        return false;
    }
    return GetWindowTextLengthW(h) > 0;
}

std::wstring ProcessPath(HWND h) {
    DWORD pid = 0;
    GetWindowThreadProcessId(h, &pid);
    std::wstring result;
    if (HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid)) {
        WCHAR path[MAX_PATH * 2];
        DWORD size = ARRAYSIZE(path);
        if (QueryFullProcessImageNameW(process, 0, path, &size)) result = path;
        CloseHandle(process);
    }
    return result;
}

// PKEY_AppUserModel_ID: what Windows itself groups taskbar buttons by.
constexpr PROPERTYKEY kAppUserModelId = {{0x9F4C2855, 0x9F79, 0x4B39, {0xA8, 0xD0, 0xE1, 0xD4, 0x2D, 0xE1, 0xD5, 0xF3}}, 5};

std::wstring Lower(std::wstring s) {
    std::transform(s.begin(), s.end(), s.begin(), towlower);
    return s;
}

std::wstring FileNameOf(std::wstring const& path) {
    size_t slash = path.find_last_of(L"\\/");
    return Lower(slash == std::wstring::npos ? path : path.substr(slash + 1));
}

// Windows' app resolver, what Explorer's taskbar asks (undocumented, stable
// since Windows 8): the app id a window is grouped under, including ids a
// program sets for its whole process, and the shortcut Windows shows for an
// app, pinned or remembered in ImplicitAppShortcuts.
struct IApplicationResolver8 : IUnknown {
    virtual HRESULT STDMETHODCALLTYPE GetAppIDForShortcut(IShellItem*, LPWSTR*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetAppIDForShortcutObject(IShellLinkW*, IShellItem*, LPWSTR*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetAppIDForWindow(HWND, LPWSTR*, BOOL*, BOOL*, BOOL*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetAppIDForProcess(DWORD, LPWSTR*, BOOL*, BOOL*, BOOL*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetShortcutForProcess(DWORD, IShellItem**) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetBestShortcutForAppID(LPCWSTR, IShellItem**) = 0;
};
constexpr CLSID kClsidAppResolver = {0x660b90c8, 0x73a9, 0x4b58, {0x8c, 0xae, 0x35, 0x5b, 0x7f, 0x55, 0x34, 0x1b}};
constexpr IID kIidAppResolver8 = {0xde25675a, 0x72de, 0x44b4, {0x93, 0x73, 0x05, 0x17, 0x04, 0x50, 0xc1, 0x40}};

IApplicationResolver8* g_resolver;  // on the bar's thread

IApplicationResolver8* Resolver() {
    if (!g_resolver)
        CoCreateInstance(kClsidAppResolver, nullptr, CLSCTX_INPROC_SERVER, kIidAppResolver8, (void**)&g_resolver);
    return g_resolver;
}

// The shortcut Windows shows for an app: a .lnk path, or a shell item's
// parsing name (a packaged app's).
std::wstring BestShortcut(std::wstring const& appId) {
    std::wstring result;
    IShellItem* item = nullptr;
    if (!appId.empty() && Resolver() && SUCCEEDED(g_resolver->GetBestShortcutForAppID(appId.c_str(), &item)) && item) {
        LPWSTR name = nullptr;
        if (SUCCEEDED(item->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &name)) && name) {
            result = name;
            CoTaskMemFree(name);
        }
        item->Release();
    }
    return result;
}

// The window's app id, as Windows groups by: the resolver's, else the
// window's own property. Explorer's folder windows carry none; theirs is
// implied, and it's what the File Explorer pin carries.
std::wstring WindowAppId(HWND h, std::wstring const& exe) {
    if (Resolver()) {
        LPWSTR resolved = nullptr;
        BOOL noPin = FALSE, isExplicit = FALSE, embedded = FALSE;
        if (SUCCEEDED(g_resolver->GetAppIDForWindow(h, &resolved, &noPin, &isExplicit, &embedded)) && resolved) {
            std::wstring id = resolved;
            CoTaskMemFree(resolved);
            if (!id.empty()) return id;
        }
    }
    IPropertyStore* store = nullptr;
    std::wstring id;
    if (SUCCEEDED(SHGetPropertyStoreForWindow(h, __uuidof(IPropertyStore), (void**)&store)) && store) {
        PROPVARIANT pv;
        PropVariantInit(&pv);
        if (SUCCEEDED(store->GetValue(kAppUserModelId, &pv)) && pv.vt == VT_LPWSTR && pv.pwszVal) id = pv.pwszVal;
        PropVariantClear(&pv);
        store->Release();
    }
    if (id.empty() && FileNameOf(exe) == L"explorer.exe") id = L"Microsoft.Windows.Explorer";
    return id;
}

// The size icons are drawn at, in pixels.
int IconPixels() {
    return Px(g_settings.iconSize);
}

int IconSizeOf(HICON icon) {
    ICONINFO ii{};
    if (!icon || !GetIconInfo(icon, &ii)) return 0;
    BITMAP bm{};
    int size = 0;
    if (ii.hbmColor && GetObjectW(ii.hbmColor, sizeof(bm), &bm)) size = bm.bmWidth;
    else if (ii.hbmMask && GetObjectW(ii.hbmMask, sizeof(bm), &bm)) size = bm.bmWidth;
    if (ii.hbmColor) DeleteObject(ii.hbmColor);
    if (ii.hbmMask) DeleteObject(ii.hbmMask);
    return size;
}

// The window's own icon, the small one first when icons are drawn small, so
// the app's hand-drawn 16 px art is used instead of its 32 px one shrunk.
void FetchIcon(Win& w) {
    if (w.ownsIcon && w.icon) DestroyIcon(w.icon);
    w.icon = nullptr;
    w.ownsIcon = false;
    w.iconPx = 0;
    bool small = IconPixels() < 32;
    WPARAM kinds[] = {small ? (WPARAM)ICON_SMALL : (WPARAM)ICON_BIG, small ? (WPARAM)ICON_BIG : (WPARAM)ICON_SMALL,
                      (WPARAM)ICON_SMALL2};
    DWORD_PTR result = 0;
    for (WPARAM kind : kinds) {
        if (SendMessageTimeoutW(w.hwnd, WM_GETICON, kind, 0, SMTO_ABORTIFHUNG | SMTO_BLOCK, 100, &result) && result) {
            w.icon = (HICON)result;
            break;
        }
    }
    if (!w.icon) w.icon = (HICON)GetClassLongPtrW(w.hwnd, small ? GCLP_HICONSM : GCLP_HICON);
    if (!w.icon) w.icon = (HICON)GetClassLongPtrW(w.hwnd, small ? GCLP_HICON : GCLP_HICONSM);
    w.iconPx = IconSizeOf(w.icon);
}

Win* FindWin(HWND h) {
    for (auto& w : g_wins)
        if (w.hwnd == h) return &w;
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
// App icons, found the way Explorer's taskbar finds them: the app's pinned
// shortcut (matched by app id, or by program file when the shortcut has no
// arguments), else its Start menu shortcut, else a packaged app's own icon,
// else the window's icon or the program file's. Each is read at the exact
// size drawn, so nothing is shrunk, and the suite's Icon Redirect runs in
// this process too, so icon themes apply here as they do in Explorer.

struct Shortcut {
    std::wstring file;       // the .lnk
    std::wstring appId;      // lower case
    std::wstring target;     // lower case; empty when it passes arguments
    std::wstring program;    // what it runs, arguments or not
    std::wstring iconPath;   // its own icon, if it sets one
    int iconIndex = 0;
    bool pinned = false;
};

std::vector<Shortcut> g_shortcuts;
ULONGLONG g_shortcutsRead = 0;

struct CachedIcon {
    std::wstring id;
    int size;
    HICON icon;   // may be null: nothing found, don't look again
};
std::vector<CachedIcon> g_iconCache;

void ClearIconCache() {
    for (auto& c : g_iconCache)
        if (c.icon) DestroyIcon(c.icon);
    g_iconCache.clear();
}

std::wstring Expand(std::wstring const& s) {
    WCHAR out[MAX_PATH * 2];
    DWORD n = ExpandEnvironmentStringsW(s.c_str(), out, ARRAYSIZE(out));
    return n && n <= ARRAYSIZE(out) ? out : s;
}

bool ParseShortcut(std::wstring const& file, bool pinned, Shortcut* out) {
    IShellLinkW* link = nullptr;
    if (FAILED(CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&link)))) return false;
    bool ok = false;
    IPersistFile* persist = nullptr;
    if (SUCCEEDED(link->QueryInterface(IID_PPV_ARGS(&persist)))) {
        if (SUCCEEDED(persist->Load(file.c_str(), STGM_READ))) {
            Shortcut s{file};
            s.pinned = pinned;
            WCHAR buf[MAX_PATH * 2] = L"";
            if (SUCCEEDED(link->GetPath(buf, ARRAYSIZE(buf), nullptr, SLGP_RAWPATH)) && buf[0]) {
                s.program = Expand(buf);
                s.target = Lower(s.program);
            }
            WCHAR args[8] = L"";
            // A shortcut that passes arguments launches something else
            // through its program (explorer.exe shell:AppsFolder\...).
            if (SUCCEEDED(link->GetArguments(args, ARRAYSIZE(args))) && args[0]) s.target.clear();
            buf[0] = 0;
            if (SUCCEEDED(link->GetIconLocation(buf, ARRAYSIZE(buf), &s.iconIndex)) && buf[0]) s.iconPath = Expand(buf);
            IPropertyStore* store = nullptr;
            if (SUCCEEDED(link->QueryInterface(IID_PPV_ARGS(&store)))) {
                PROPVARIANT pv;
                PropVariantInit(&pv);
                if (SUCCEEDED(store->GetValue(kAppUserModelId, &pv)) && pv.vt == VT_LPWSTR && pv.pwszVal)
                    s.appId = Lower(pv.pwszVal);
                PropVariantClear(&pv);
                store->Release();
            }
            *out = std::move(s);
            ok = true;
        }
        persist->Release();
    }
    link->Release();
    return ok;
}

void ReadShortcut(std::wstring const& file, bool pinned) {
    Shortcut s;
    if (ParseShortcut(file, pinned, &s) && (!s.appId.empty() || !s.target.empty())) g_shortcuts.push_back(std::move(s));
}

void ReadShortcutFolder(std::wstring const& folder, bool pinned, int depth) {
    WIN32_FIND_DATAW fd;
    HANDLE find = FindFirstFileW((folder + L"\\*").c_str(), &fd);
    if (find == INVALID_HANDLE_VALUE) return;
    do {
        std::wstring name = fd.cFileName;
        if (name == L"." || name == L"..") continue;
        std::wstring path = folder + L"\\" + name;
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (depth > 0) ReadShortcutFolder(path, pinned, depth - 1);
        } else if (name.size() > 4 && Lower(name.substr(name.size() - 4)) == L".lnk") {
            ReadShortcut(path, pinned);
        }
    } while (FindNextFileW(find, &fd));
    FindClose(find);
}

// Reads the pinned and Start menu shortcuts; at most every 30 seconds
// unless forced. Icons found through old shortcuts are dropped.
void RefreshShortcuts(bool force) {
    ULONGLONG now = GetTickCount64();
    if (!force && g_shortcutsRead && now - g_shortcutsRead < 30000) return;
    g_shortcutsRead = now;
    g_shortcuts.clear();
    ClearIconCache();
    WCHAR appData[MAX_PATH], programData[MAX_PATH];
    if (GetEnvironmentVariableW(L"APPDATA", appData, MAX_PATH)) {
        ReadShortcutFolder(std::wstring(appData) + L"\\Microsoft\\Internet Explorer\\Quick Launch\\User Pinned\\TaskBar",
                           true, 0);
        ReadShortcutFolder(std::wstring(appData) + L"\\Microsoft\\Windows\\Start Menu\\Programs", false, 3);
    }
    if (GetEnvironmentVariableW(L"ProgramData", programData, MAX_PATH))
        ReadShortcutFolder(std::wstring(programData) + L"\\Microsoft\\Windows\\Start Menu\\Programs", false, 3);
}

// The app's pinned shortcut, or its Start menu one: by app id, then by
// program file.
Shortcut const* FindShortcut(Win const& w, bool pinned) {
    std::wstring id = Lower(w.appId), exe = Lower(w.exe);
    for (bool byId : {true, false}) {
        for (auto const& s : g_shortcuts) {
            if (s.pinned != pinned) continue;
            if (byId ? (!id.empty() && s.appId == id) : (!exe.empty() && s.target == exe && s.appId.empty()))
                return &s;
        }
    }
    return nullptr;
}

HICON ExtractAt(std::wstring const& path, int index, int size) {
    if (path.empty()) return nullptr;
    HICON icon = nullptr;
    UINT id = 0;
    UINT got = PrivateExtractIconsW(path.c_str(), index, size, size, &icon, &id, 1, LR_DEFAULTCOLOR);
    return got == 1 ? icon : nullptr;
}

// The shell's own icon for something (a packaged app, a shortcut to a
// folder), at the size asked for.
HICON ShellIcon(std::wstring const& parsingName, int size) {
    IShellItemImageFactory* factory = nullptr;
    if (FAILED(SHCreateItemFromParsingName(parsingName.c_str(), nullptr, IID_PPV_ARGS(&factory)))) return nullptr;
    HBITMAP bitmap = nullptr;
    HICON icon = nullptr;
    if (SUCCEEDED(factory->GetImage({size, size}, SIIGBF_ICONONLY, &bitmap)) && bitmap) {
        HBITMAP mask = CreateBitmap(size, size, 1, 1, nullptr);
        ICONINFO ii{TRUE, 0, 0, mask, bitmap};
        icon = CreateIconIndirect(&ii);
        DeleteObject(mask);
        DeleteObject(bitmap);
    }
    factory->Release();
    return icon;
}

// @inline program-icons.cpp

// Your own icon for a program (Button icons, and Tourne'Icons' Taskbar buttons
// ahead of them), matched by app id, full path or file name, with * and ?
// matching any text. The icon is a file, or "file,index".
HICON CustomIcon(std::wstring const& appId, std::wstring const& program, int size) {
    TourneIconsButtons published = ReadTourneIconsButtons();
    if (g_settings.buttonIcons.empty() && published.rows.empty()) return nullptr;
    std::vector<std::pair<std::wstring, std::wstring>> rows;
    for (auto const& row : published.rows) rows.push_back({row.program, row.icon});
    if (!published.holds) rows.insert(rows.end(), g_settings.buttonIcons.begin(), g_settings.buttonIcons.end());
    std::wstring id = Lower(appId), path = Lower(program);
    for (auto const& [match, file] : rows) {
        if (!ProgramIconMatches(match, id, path)) continue;
        std::wstring key = L"custom:" + file;
        for (auto const& c : g_iconCache)
            if (c.size == size && c.id == key) return c.icon;
        if (IsTourneDesign(file)) {  // one of System Icons' designs, in the colour scheme
            HICON icon = part_system_icons::DesignIconAt(file, size);
            g_iconCache.push_back({key, size, icon});
            return icon;
        }
        std::wstring source = file;
        int index = 0;
        size_t comma = source.rfind(L',');
        if (comma != std::wstring::npos && comma + 1 < source.size() && iswdigit(source.back())) {
            index = _wtoi(source.c_str() + comma + 1);
            source.resize(comma);
        }
        HICON icon = ExtractAt(source, index, size);
        g_iconCache.push_back({key, size, icon});
        return icon;
    }
    return nullptr;
}

HICON CachedIconFor(std::wstring const& id, int size, HICON (*load)(Win const&, int), Win const& w) {
    for (auto const& c : g_iconCache)
        if (c.size == size && c.id == id) return c.icon;
    HICON icon = load(w, size);
    g_iconCache.push_back({id, size, icon});
    return icon;
}

HICON ShortcutIcon(Shortcut const& s, int size) {
    if (HICON icon = ExtractAt(s.iconPath, s.iconIndex, size)) return icon;
    if (s.iconPath.empty())
        if (HICON icon = ExtractAt(s.program, 0, size)) return icon;
    return ShellIcon(s.file, size);
}

// The window's relaunch icon (PKEY_AppUserModel_RelaunchIconResource), which
// some programs set per window: "path,index".
HICON RelaunchIcon(HWND h, int size) {
    constexpr PROPERTYKEY kRelaunchIcon = {{0x9F4C2855, 0x9F79, 0x4B39, {0xA8, 0xD0, 0xE1, 0xD4, 0x2D, 0xE1, 0xD5, 0xF3}}, 3};
    IPropertyStore* store = nullptr;
    HICON icon = nullptr;
    if (SUCCEEDED(SHGetPropertyStoreForWindow(h, __uuidof(IPropertyStore), (void**)&store)) && store) {
        PROPVARIANT pv;
        PropVariantInit(&pv);
        if (SUCCEEDED(store->GetValue(kRelaunchIcon, &pv)) && pv.vt == VT_LPWSTR && pv.pwszVal) {
            std::wstring value = pv.pwszVal;
            int index = 0;
            size_t comma = value.rfind(L',');
            if (comma != std::wstring::npos) {
                index = _wtoi(value.c_str() + comma + 1);
                value.resize(comma);
            }
            icon = ExtractAt(Expand(value), index, size);
        }
        PropVariantClear(&pv);
        store->Release();
    }
    return icon;
}

// The app's icon, in the order Windows looks: its pin (running windows join
// their pinned button), the shortcut Windows keeps for it, its Start menu
// shortcut, a packaged app's own icon, the window's relaunch icon.
HICON LoadAppIcon(Win const& w, int size) {
    if (Shortcut const* s = FindShortcut(w, true)) return ShortcutIcon(*s, size);
    if (!w.shortcut.empty()) {
        Shortcut s;
        if (Lower(w.shortcut).ends_with(L".lnk") ? ParseShortcut(w.shortcut, false, &s) : false) {
            if (HICON icon = ShortcutIcon(s, size)) return icon;
        } else if (HICON icon = ShellIcon(w.shortcut, size)) {
            return icon;
        }
    }
    if (Shortcut const* s = FindShortcut(w, false)) return ShortcutIcon(*s, size);
    // A packaged app, which has no program file icon of its own.
    if (w.appId.find(L'!') != std::wstring::npos) return ShellIcon(L"shell:AppsFolder\\" + w.appId, size);
    return RelaunchIcon(w.hwnd, size);
}

// The app's icon at a size, else its program file's (kept in the cache).
HICON AppIcon(Win const& w, int size) {
    if (HICON custom = CustomIcon(w.appId, w.exe, size)) return custom;
    if (HICON app = CachedIconFor(L"app:" + w.key, size, LoadAppIcon, w)) return app;
    return CachedIconFor(L"exe:" + Lower(w.exe), size,
                         [](Win const& w, int size) { return ExtractAt(w.exe, 0, size); }, w);
}

// The icon a button shows. A combined button shows the app's icon, as
// Windows does; a button for one window prefers the window's own icon when
// it's drawn for this size.
HICON ButtonIcon(Win const& w, bool combined) {
    int size = IconPixels();
    if (HICON custom = CustomIcon(w.appId, w.exe, size)) return custom;
    if (!combined && w.icon && w.iconPx == size) return w.icon;
    HICON app = CachedIconFor(L"app:" + w.key, size, LoadAppIcon, w);
    if (app) return app;
    HICON exe = CachedIconFor(L"exe:" + Lower(w.exe), size,
                              [](Win const& w, int size) { return ExtractAt(w.exe, 0, size); }, w);
    return exe ? exe : w.icon;
}

////////////////////////////////////////////////////////////////////////////////
// Pinned apps, in the order the taskbar keeps them: Taskband\Favorites holds
// the pinned shortcuts' item id lists, one after another. A running app's
// windows join its pin's button.

struct Pin {
    Shortcut shortcut;
    std::wstring appId;  // lower case: what Windows groups the app's windows under
    std::wstring name;   // the shortcut's name, for its tooltip
};

std::vector<Pin> g_pins;
DWORD g_pinsChanges = (DWORD)-1;  // Taskband's FavoritesChanges when they were read

constexpr WCHAR kTaskband[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Taskband";

std::wstring AppIdForShortcut(std::wstring const& file) {
    std::wstring id;
    IShellItem* item = nullptr;
    if (Resolver() && SUCCEEDED(SHCreateItemFromParsingName(file.c_str(), nullptr, IID_PPV_ARGS(&item))) && item) {
        LPWSTR resolved = nullptr;
        if (SUCCEEDED(g_resolver->GetAppIDForShortcut(item, &resolved)) && resolved) {
            id = resolved;
            CoTaskMemFree(resolved);
        }
        item->Release();
    }
    return id;
}

// Reads the pins when they've changed (or when forced); true if they had.
bool LoadPins(bool force) {
    DWORD changes = 0, size = sizeof(changes);
    RegGetValueW(HKEY_CURRENT_USER, kTaskband, L"FavoritesChanges", RRF_RT_REG_DWORD, nullptr, &changes, &size);
    if (!force && changes == g_pinsChanges) return false;
    g_pinsChanges = changes;

    std::vector<BYTE> data;
    DWORD bytes = 0;
    if (RegGetValueW(HKEY_CURRENT_USER, kTaskband, L"Favorites", RRF_RT_REG_BINARY, nullptr, nullptr, &bytes) ==
            ERROR_SUCCESS &&
        bytes) {
        data.resize(bytes);
        if (RegGetValueW(HKEY_CURRENT_USER, kTaskband, L"Favorites", RRF_RT_REG_BINARY, nullptr, data.data(), &bytes) !=
            ERROR_SUCCESS)
            data.clear();
    }
    // A version byte, then per pin its list's length, the list and a 0 byte;
    // 0xFF ends it.
    std::vector<Pin> pins;
    size_t at = 1;
    while (at + 4 <= data.size()) {
        DWORD length = 0;
        memcpy(&length, &data[at], 4);
        at += 4;
        if (!length || at + length > data.size()) break;
        std::vector<BYTE> list(data.begin() + at, data.begin() + at + length);
        list.insert(list.end(), 4, 0);  // the terminator, whether or not it was counted
        at += length + 1;
        WCHAR path[MAX_PATH * 2];
        Pin pin;
        if (!SHGetPathFromIDListEx((PCIDLIST_ABSOLUTE)list.data(), path, ARRAYSIZE(path), GPFIDL_DEFAULT) ||
            !ParseShortcut(path, true, &pin.shortcut))
            continue;
        pin.appId = Lower(AppIdForShortcut(path));
        if (pin.appId.empty()) pin.appId = !pin.shortcut.appId.empty() ? pin.shortcut.appId : pin.shortcut.target;
        pin.name = path;
        pin.name = pin.name.substr(pin.name.find_last_of(L'\\') + 1);
        if (Lower(pin.name).ends_with(L".lnk")) pin.name.resize(pin.name.size() - 4);
        pins.push_back(std::move(pin));
    }
    g_pins = std::move(pins);
    return true;
}

// The pin a window's app belongs to, or -1.
int PinOf(Win const& w) {
    for (size_t i = 0; i < g_pins.size(); i++) {
        auto const& pin = g_pins[i];
        if (!pin.appId.empty() && pin.appId == w.key) return (int)i;
    }
    // A pin Windows had no app id for: by program file.
    for (size_t i = 0; i < g_pins.size(); i++) {
        auto const& pin = g_pins[i];
        if (pin.appId == pin.shortcut.target && !pin.shortcut.target.empty() && pin.shortcut.target == Lower(w.exe))
            return (int)i;
    }
    return -1;
}

HICON PinIcon(int index, int size = 0) {
    if (index < 0 || index >= (int)g_pins.size()) return nullptr;
    if (!size) size = IconPixels();
    if (HICON custom = CustomIcon(g_pins[index].appId, g_pins[index].shortcut.program, size)) return custom;
    std::wstring id = L"pin:" + g_pins[index].shortcut.file;
    for (auto const& c : g_iconCache)
        if (c.size == size && c.id == id) return c.icon;
    HICON icon = ShortcutIcon(g_pins[index].shortcut, size);
    g_iconCache.push_back({id, size, icon});
    return icon;
}

void LaunchPin(int index) {
    if (index < 0 || index >= (int)g_pins.size()) return;
    ShellExecuteW(nullptr, nullptr, g_pins[index].shortcut.file.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
}

// Keeps the order windows first appeared in; new ones go at the end. Every
// window is kept, whatever screen it is on; each bar takes its own (WinOnBar).
void Rescan() {
    bool newWindows = false;
    std::vector<HWND> now;
    EnumWindows(
        [](HWND h, LPARAM param) __stdcall -> BOOL {
            if (IsTaskWindow(h)) ((std::vector<HWND>*)param)->push_back(h);
            return TRUE;
        },
        (LPARAM)&now);
    for (size_t i = 0; i < g_wins.size();) {
        if (std::find(now.begin(), now.end(), g_wins[i].hwnd) == now.end()) {
            if (g_wins[i].ownsIcon && g_wins[i].icon) DestroyIcon(g_wins[i].icon);
            g_wins.erase(g_wins.begin() + i);
        } else {
            i++;
        }
    }
    // Windows found together (all of them when the bar starts) go in the
    // order their programs started, close to the order Explorer's taskbar
    // got them in.
    auto started = [](HWND h) {
        DWORD pid = 0;
        GetWindowThreadProcessId(h, &pid);
        ULONGLONG when = ~0ull;
        if (HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid)) {
            FILETIME created, exited, kernel, user;
            if (GetProcessTimes(process, &created, &exited, &kernel, &user))
                when = (ULONGLONG)created.dwHighDateTime << 32 | created.dwLowDateTime;
            CloseHandle(process);
        }
        return when;
    };
    std::vector<std::pair<ULONGLONG, HWND>> fresh;
    for (auto it = now.rbegin(); it != now.rend(); ++it)
        if (!FindWin(*it)) fresh.push_back({started(*it), *it});
    std::stable_sort(fresh.begin(), fresh.end(), [](auto const& a, auto const& b) { return a.first < b.first; });
    now.clear();
    for (auto const& [when, h] : fresh) now.push_back(h);
    for (auto it = now.begin(); it != now.end(); ++it) {
        if (!FindWin(*it)) {
            Win w{*it};
            w.exe = ProcessPath(*it);
            w.appId = WindowAppId(*it, w.exe);
            w.key = Lower(w.appId.empty() ? w.exe : w.appId);
            w.shortcut = BestShortcut(w.appId);
            FetchIcon(w);
            g_wins.push_back(w);
            newWindows = true;
        }
    }
    // A new app may have been pinned or installed since shortcuts were read.
    if (newWindows) RefreshShortcuts(false);
}

HWND ForegroundRoot() {
    HWND fg = GetForegroundWindow();
    return fg ? GetAncestor(fg, GA_ROOTOWNER) : nullptr;
}

////////////////////////////////////////////////////////////////////////////////
// The tray: Explorer's notification icons, System Icons' among them, fetched
// through the suite's Explorer relay and shown where the user put them (see
// tray-shared.cpp), then the clock, drawn from the Clock part's settings.

struct TrayItem {
    HiddenIcon icon;
    std::wstring key;
    RECT rc{};
};

std::vector<HiddenIcon> g_trayAll;  // every icon, in Explorer's order
std::vector<TrayItem> g_tray;       // the ones in the bar, in order
uint64_t g_trayHash = 0;
bool g_trayChanged = false;
UINT g_msgRelay = 0;                // the relay's "TourneSuite_RelayOverflow"
UINT g_msgTrayChanged = 0;          // System Icons changed an icon: read now
RECT g_clockRect{};
RECT g_showDesktopRect{};           // the show desktop button at the far end, when shown
bool g_arrowClicked = false;        // the show-hidden-icons arrow opened the overflow flyout
bool g_arrowDrawnOpen = false;      // and the bar shows it open
bool IsOverflowArrow(TrayItem const& item);  // (with the tray's dragging, below)
int g_trayDrag = -1;                // a tray icon being dragged: its index in g_tray
int g_pluginDrag = -1;              // a plugin item being dragged along the tray: its index in g_pluginItems
bool g_pluginDragMoved = false;
int g_pluginDragSlot = -1;          // while it's dragged: the tray icon it would go before (g_tray.size(): the end)
std::vector<std::wstring> g_trayVisual;  // the tray's keys as laid out: plugin items ("plugin:<key>") among the icons
int g_trayDropAt = -1;              // where an icon dragged from the overflow flyout would land

bool IsMainBar();  // (defined with the bars' state, below the clock)


// Hit test results past the task buttons.
constexpr int kHitShowDesktop = 9998;
constexpr int kHitClock = 9999;
constexpr int kHitTray = 10000;     // + the icon's index in g_tray
constexpr int kHitCss = 30000;      // + the index of one of a YASB theme's own buttons in g_cssTargets
constexpr int kHitDesktops = 9000;  // + a desktop's index in the desktops pager, docked on the main bar
constexpr int kHitPlugin = 8000;    // + a plugin item's index (below the pager's codes)
constexpr int kPluginMax = 900;

// Plugin items: what Tourne Desktop's plugin runtime shows on the bar (plugin-bar-shared.cpp), on the
// main bar just before the tray. Outside Tourne Desktop there's no runtime, so none.
std::vector<PluginBarItem> g_pluginItems;  // as the runtime last sent them (the main bar asks)
unsigned g_pluginVersion = 0;              // that list's version, sent back with a click
std::vector<RECT> g_pluginRects;           // where each is on this bar (kept per bar, with its state)
UINT g_msgPluginsChanged = 0, g_msgPluginsAsk = 0, g_msgPluginsEvent = 0;
// An item's picture (barWidget.setImage), read from the file of pixels the runtime made for it.
struct PluginPicture {
    int w = 0, h = 0;
    std::vector<uint32_t> pixels;  // premultiplied BGRA, top-down
};
std::vector<PluginPicture> g_pluginPictures;  // (by item, as g_pluginItems)
PluginPicture ReadPluginPicture(std::wstring const& path) {
    PluginPicture p;
    if (!ReadPluginImageFile(path, &p.w, &p.h, &p.pixels)) p = {};
    return p;
}
// The plugin items the user moved out of the tray (into the overflow, or hidden), by "plugin:<key>": the bar
// leaves them out. Read with the tray's places (BuildTray) and as the items arrive.
std::vector<std::wstring> g_pluginAway;
bool PluginAway(PluginBarItem const& item) {
    return std::find(g_pluginAway.begin(), g_pluginAway.end(), L"plugin:" + item.key) != g_pluginAway.end();
}
void ReadPluginAway(TrayPlaces const& places) {
    g_pluginAway.clear();
    for (auto const& [key, place] : places.places)
        if (key.starts_with(L"plugin:") && (place == kPlaceOverflow || place == kPlaceHidden)) g_pluginAway.push_back(key);
}
int PluginIndexOf(int hit) {
    int count = std::min({(int)g_pluginItems.size(), (int)g_pluginRects.size(), kPluginMax});
    return hit >= kHitPlugin && hit < kHitPlugin + count ? hit - kHitPlugin : -1;
}
HWND PluginHost() { return FindWindowExW(HWND_MESSAGE, nullptr, kPluginHostClass, nullptr); }
void AskForPlugins(HWND bar) {
    if (HWND host = PluginHost()) PostMessageW(host, g_msgPluginsAsk, (WPARAM)bar, 0);
}
void PluginEvent(int index, PluginBarEvent event) {
    if (HWND host = PluginHost()) PostMessageW(host, g_msgPluginsEvent, g_pluginVersion, MakePluginBarEvent(index, event));
}
constexpr int kPagerMax = 900;      // desktops it shows, at most (their hit codes stay below kHitShowDesktop)

// The desktops pager (desktops-pager.cpp, inlined with the YASB bar's files below): where it goes and how
// it looks, and what Layout, the drawing and the hit test ask of it.
enum : int { kPagerBeforeTray, kPagerAfterStart, kPagerAfterClock, kPagerFloating };
enum : int { kPagerDots, kPagerNumbers, kPagerNames };
RECT g_pagerRect{};  // where it's docked on this bar (kept per bar, with its state; empty elsewhere)
std::vector<RECT> g_pagerItems;  // its marks there (only the main bar has it)
std::wstring PagerLabel(int i);
int DockedPagerLength();
int DockedPagerPlace();
void PlaceDockedPager(RECT area);
void DrawDockedPager(HDC dc, uint32_t* bits, int w, int h);
int DockedPagerHit(POINT pt);

// One of a YASB theme's buttons (volume, the power menu, an apps list icon...): where it is and what it does.
struct CssTarget {
    RECT rc{};
    std::wstring left, right, middle;  // the theme's callbacks: "start_menu", "exec ...", "power_menu"
    std::wstring tip;
};
std::vector<CssTarget> g_cssTargets;

// The bar drawn from a YASB theme (css-bar.cpp).
bool CssActive();
void CssApply();
void CssLayout(HWND wnd);
void CssRedraw(HWND wnd);
RECT CssBody(RECT const& reserved);
void UpdateTooltips(HWND wnd);

// The CSS engine, its config reader and its drawing (a YASB theme's styles.css and config.yaml).
// @inline css-engine.cpp
// @inline yaml-lite.cpp
// @inline css-draw.cpp

uint64_t HashBytes(const BYTE* data, size_t size) {
    uint64_t hash = 14695981039346656037ull;
    for (size_t i = 0; i < size; i++) hash = (hash ^ data[i]) * 1099511628211ull;
    return hash;
}

void BuildTray() {
    if (!IsMainBar()) return;  // the tray is the main bar's
    TrayPlaces places = TrayPlaces::Load();
    std::vector<TrayItem> shown;
    for (auto const& icon : g_trayAll) {
        std::wstring key = HiddenIconKey(icon);
        if (places.For(icon, key) == kPlaceTray) shown.push_back({icon, key});
    }
    ApplyTrayOrder(shown, LoadTrayList(kTrayOrderValue), [](TrayItem const& item) { return item.key; });
    g_tray = std::move(shown);
    ReadPluginAway(places);
}

// The tray built again from the icons' places and order; true when the bar's icons changed.
bool RebuildTray() {
    if (!IsMainBar() || g_trayDrag >= 0) return false;
    std::vector<TrayItem> before = g_tray;
    std::vector<std::wstring> awayBefore = g_pluginAway;
    BuildTray();
    bool same = g_tray.size() == before.size() && g_pluginAway == awayBefore;
    for (size_t i = 0; same && i < before.size(); i++) same = g_tray[i].key == before[i].key;
    // Unchanged, the icons keep their places: built afresh they'd have none until the next layout, and a
    // redraw meanwhile (a hover, as an app updates its icon) drew the tray empty.
    if (same) g_tray = std::move(before);
    return !same;
}

// The relay's answer, which arrives while FetchTray waits.
void OnTrayPacket(COPYDATASTRUCT const* copyData) {
    uint64_t hash = HashBytes((const BYTE*)copyData->lpData, copyData->cbData);
    if (hash == g_trayHash) return;
    g_trayHash = hash;
    g_trayAll = UnpackHiddenIcons((const BYTE*)copyData->lpData, copyData->cbData);
    g_trayChanged = true;
}

// Asks the relay for every tray icon. True when something changed.
bool FetchTray(HWND wnd) {
    if (!IsMainBar()) return false;
    g_trayChanged = false;
    if (!g_settings.tray) {
        bool had = !g_trayAll.empty();
        g_trayAll.clear();
        g_tray.clear();
        g_trayHash = 0;
        return had;
    }
    HWND relay = RelayToolbar();
    if (!relay || !g_msgRelay) return false;
    DWORD_PTR result = 0;
    // Not SMTO_BLOCK: the answer is a message sent to this thread meanwhile.
    SendMessageTimeoutW(relay, g_msgRelay, (WPARAM)wnd, kRelayAllIcons, SMTO_ABORTIFHUNG, 1000, &result);
    // Mid-drag the tray keeps its order; the drop builds it again.
    if (g_trayChanged && g_trayDrag < 0) BuildTray();
    return g_trayChanged;
}

// The cell a tray icon sits in, along the bar.
int TrayCell() {
    int icon = Px(16);
    for (auto const& item : g_tray) icon = std::max(icon, item.icon.size);
    return icon + 2 * Px(g_settings.trayMargin);
}

////////////////////////////////////////////////////////////////////////////////
// The clock, from the Clock part's settings: its lines (with the same tokens),
// font, colours, layout and clicks. Read by their full names, through a
// variable, so the build doesn't file them under this part.

struct Clock {
    std::vector<std::wstring> lines;
    HFONT primaryFont = nullptr;
    HFONT secondaryFont = nullptr;
    bool hasPrimary = false, hasSecondary = false;
    COLORREF primary = 0, secondary = 0;
    int align = 0;  // 0 centre, 1 left, 2 right
    int padding = 1;
    int lineSpacing = 0;
    std::wstring clicks[3];    // left, right, middle
    std::wstring commands[3];
    int width = 0;             // the widest it gets, so the tray doesn't shift
    int height = 0;
};

Clock g_clock;

////////////////////////////////////////////////////////////////////////////////
// One bar per screen. The drawing and hit testing work on globals (the bar
// being handled), so every bar keeps its own copy of the few that differ
// between bars, and they're swapped in when a message arrives for it (and out
// again after, as menus and previews pump messages for other bars meanwhile).
// The main bar has the Start button and the tray; the others the buttons of
// their screen's windows, and the clock.

// A bar coming in for a screen just added (motion.monitorAdded): waiting until it's set up (unseen till
// then), then slid in from its edge, faded in, or both.
struct BarArrival {
    bool pending = false, running = false, fade = false, slide = false;
    double level = 1, start = 0;  // eased progress: 0 at its edge (or unseen), 1 in place
    POINT home{};                 // its place
};

struct BarState {
    HWND hwnd = nullptr;              // never swapped: how a window finds its state
    BarArrival arrive;                // never swapped: read through g_active
    HMONITOR monitor = nullptr;       // its screen; null for the main bar
    std::vector<Button> buttons;
    bool labels = false;
    RECT orbRect{};
    std::vector<RECT> islands;
    int centralIsland = -1;
    HWND tooltip = nullptr;
    bool appBar = false, shown = true, touching = false, underFullscreen = false;
    double slide = 1, slideFrom = 1, slideStart = 0, hint = 0, hintStart = 0;
    ULONGLONG wantSince = 0;
    bool edgeHint = false;
    BYTE barAlpha = 255;
    std::shared_ptr<FramePacer> pacer;
    bool shyRunning = false;
    AuraFollow aura;
    IdleFadeState idle;
    int hot = -2, pressed = -2;
    bool tracking = false;
    GlowFade glowIn, glowOut;
    DWORD glowTick = 0;
    bool swallowUp = false;
    std::vector<TrayItem> tray;
    RECT clockRect{};
    RECT showDesktopRect{};
    RECT pagerRect{};
    std::vector<RECT> pluginRects;
    std::vector<CssTarget> cssTargets;
    std::vector<std::pair<UINT_PTR, RECT>> placed;
    std::wstring placedContent;
    // What depends on its screen's scale: the sizes, the fonts and the clock (measured in those fonts).
    int scale = 1;
    HFONT font = nullptr, tipFont = nullptr;
    Clock clock;
};

BarState g_mainBar;                   // its state is in the globals to begin with
std::vector<BarState*> g_bars;        // the main bar first
BarState* g_active = &g_mainBar;      // whose state the globals hold now

void SwapBarState(BarState& s) {
    using std::swap;
    swap(g_buttons, s.buttons);
    swap(g_labels, s.labels);
    swap(g_orbRect, s.orbRect);
    swap(g_islands, s.islands);
    swap(g_centralIsland, s.centralIsland);
    swap(g_tooltip, s.tooltip);
    swap(g_appBar, s.appBar);
    swap(g_shown, s.shown);
    swap(g_underFullscreen, s.underFullscreen);
    swap(g_touching, s.touching);
    swap(g_slide, s.slide);
    swap(g_slideFrom, s.slideFrom);
    swap(g_slideStart, s.slideStart);
    swap(g_hint, s.hint);
    swap(g_hintStart, s.hintStart);
    swap(g_wantSince, s.wantSince);
    swap(g_edgeHint, s.edgeHint);
    swap(g_barAlpha, s.barAlpha);
    swap(g_pacer, s.pacer);
    swap(g_shyRunning, s.shyRunning);
    swap(g_aura, s.aura);
    swap(g_idle, s.idle);
    swap(g_hot, s.hot);
    swap(g_pressed, s.pressed);
    swap(g_tracking, s.tracking);
    swap(g_glowIn, s.glowIn);
    swap(g_glowOut, s.glowOut);
    swap(g_glowTick, s.glowTick);
    swap(g_swallowUp, s.swallowUp);
    swap(g_tray, s.tray);
    swap(g_clockRect, s.clockRect);
    swap(g_showDesktopRect, s.showDesktopRect);
    swap(g_pagerRect, s.pagerRect);
    swap(g_pluginRects, s.pluginRects);
    swap(g_cssTargets, s.cssTargets);
    swap(g_placed, s.placed);
    swap(g_placedContent, s.placedContent);
    swap(g_scale, s.scale);
    swap(g_font, s.font);
    swap(g_tipFont, s.tipFont);
    swap(g_clock, s.clock);
}

void BarSwitch(BarState* to) {
    if (!to || to == g_active) return;
    SwapBarState(*g_active);  // the active bar's copy takes its state back (the globals get the blank it held)
    SwapBarState(*to);
    g_active = to;
    g_curWnd = to->hwnd;
    g_curMonitor = to->monitor;
}

// Handles what follows as the given bar; the one before is back after.
struct BarScope {
    BarState* previous;
    explicit BarScope(BarState* to) : previous(g_active) { BarSwitch(to); }
    ~BarScope() { BarSwitch(previous); }
    BarScope(BarScope const&) = delete;
    BarScope& operator=(BarScope const&) = delete;
};

BarState* BarOf(HWND wnd) {
    if (wnd && g_mainBar.hwnd == wnd) return &g_mainBar;
    for (BarState* bar : g_bars)
        if (bar->hwnd == wnd) return bar;
    return nullptr;
}

bool IsMainBar() { return g_curWnd == g_wnd || !g_curWnd; }

std::wstring ClockSetting(PCWSTR name) {
    std::wstring full = std::wstring(L"clock.") + name;
    PCWSTR value = Wh_GetStringSetting(full.c_str());
    std::wstring s = value ? value : L"";
    Wh_FreeStringSetting(value);
    return s;
}

int ClockInt(PCWSTR name) {
    std::wstring full = std::wstring(L"clock.") + name;
    return Wh_GetIntSetting(full.c_str());
}

bool ParseHexColor(std::wstring text, COLORREF* color) {
    text.erase(std::remove_if(text.begin(), text.end(), iswspace), text.end());
    if (!text.empty() && text[0] == L'#') text.erase(0, 1);
    if (text.size() != 6 && text.size() != 8) return false;
    wchar_t* end = nullptr;
    unsigned long value = wcstoul(text.c_str(), &end, 16);
    if (!end || *end) return false;
    *color = RGB((value >> 16) & 0xFF, (value >> 8) & 0xFF, value & 0xFF);
    return true;
}

std::wstring FormatTimeText(SYSTEMTIME const& time, PCWSTR picture, DWORD flags = 0) {
    WCHAR buffer[128];
    return GetTimeFormatEx(LOCALE_NAME_USER_DEFAULT, flags, &time, picture, buffer, ARRAYSIZE(buffer)) ? buffer : L"";
}

std::wstring FormatDateText(SYSTEMTIME const& time, PCWSTR picture, DWORD flags = 0) {
    WCHAR buffer[128];
    return GetDateFormatEx(LOCALE_NAME_USER_DEFAULT, flags, &time, picture, buffer, ARRAYSIZE(buffer), nullptr) ? buffer
                                                                                                            : L"";
}

// The Clock part's tokens: {time}, {time:long}, {date}, {date:long},
// {weekday}, {weekday:short}, {t:picture} and {d:picture}.
std::wstring ExpandClockToken(std::wstring_view token, SYSTEMTIME const& time) {
    if (token == L"time") {
        WCHAR picture[80] = L"";
        GetLocaleInfoEx(LOCALE_NAME_USER_DEFAULT, LOCALE_SSHORTTIME, picture, ARRAYSIZE(picture));
        return FormatTimeText(time, picture[0] ? picture : nullptr, picture[0] ? 0 : TIME_NOSECONDS);
    }
    if (token == L"time:long") return FormatTimeText(time, nullptr);
    if (token == L"date") return FormatDateText(time, nullptr, DATE_SHORTDATE);
    if (token == L"date:long") return FormatDateText(time, nullptr, DATE_LONGDATE);
    if (token == L"weekday") return FormatDateText(time, L"dddd");
    if (token == L"weekday:short") return FormatDateText(time, L"ddd");
    if (token.starts_with(L"t:")) return FormatTimeText(time, std::wstring(token.substr(2)).c_str());
    if (token.starts_with(L"d:")) return FormatDateText(time, std::wstring(token.substr(2)).c_str());
    return L"{" + std::wstring(token) + L"}";
}

std::wstring ExpandClockLine(std::wstring const& line, SYSTEMTIME const& time) {
    std::wstring out;
    size_t at = 0;
    while (at < line.size()) {
        size_t open = line.find(L'{', at);
        size_t close = open == std::wstring::npos ? open : line.find(L'}', open);
        if (close == std::wstring::npos) {
            out += line.substr(at);
            break;
        }
        out += line.substr(at, open - at);
        out += ExpandClockToken(std::wstring_view(line).substr(open + 1, close - open - 1), time);
        at = close + 1;
    }
    return out;
}

void FreeClockFonts() {
    if (g_clock.secondaryFont && g_clock.secondaryFont != g_clock.primaryFont) DeleteObject(g_clock.secondaryFont);
    if (g_clock.primaryFont) DeleteObject(g_clock.primaryFont);
    g_clock.primaryFont = g_clock.secondaryFont = nullptr;
}

void LoadClock(UINT dpi) {
    FreeClockFonts();
    Clock c;
    for (PCWSTR key : {L"lines.line1", L"lines.line2", L"lines.line3"}) {
        std::wstring line = ClockSetting(key);
        if (!line.empty()) c.lines.push_back(line);
    }
    // The Clock part was never set up: the time and the date.
    if (c.lines.empty()) c.lines = {L"{time}", L"{date}"};

    NONCLIENTMETRICSW ncm{sizeof(ncm)};
    SystemParametersInfoForDpi(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0, dpi);
    std::wstring face = ClockSetting(L"font.face");
    std::wstring smoothing = ClockSetting(L"font.smoothing");
    int size = std::clamp(ClockInt(L"font.size"), 0, 200);
    int secondarySize = std::clamp(ClockInt(L"font.secondarySize"), 0, 200);
    bool bold = ClockInt(L"font.bold") != 0;
    auto make = [&](int px, bool heavy) {
        LOGFONTW font = ncm.lfStatusFont;
        if (!face.empty()) wcsncpy_s(font.lfFaceName, face.c_str(), _TRUNCATE);
        if (px > 0) {
            font.lfHeight = -MulDiv(px, dpi, USER_DEFAULT_SCREEN_DPI);
            font.lfWidth = 0;
        }
        if (heavy) font.lfWeight = FW_BOLD;
        if (smoothing == L"crisp") font.lfQuality = NONANTIALIASED_QUALITY;
        else if (smoothing == L"smooth") font.lfQuality = ANTIALIASED_QUALITY;
        return CreateFontIndirectW(&font);
    };
    c.primaryFont = make(size, bold);
    c.secondaryFont = secondarySize > 0 || bold ? make(secondarySize ? secondarySize : size, false) : c.primaryFont;
    c.hasPrimary = ParseHexColor(ClockSetting(L"color.primary"), &c.primary);
    c.hasSecondary = ParseHexColor(ClockSetting(L"color.secondary"), &c.secondary);
    std::wstring align = ClockSetting(L"layout.align");
    c.align = align == L"left" ? 1 : align == L"right" ? 2 : 0;
    c.padding = std::clamp(ClockInt(L"layout.padding"), 0, 100);
    c.lineSpacing = std::clamp(ClockInt(L"layout.lineSpacing"), -50, 50);
    PCWSTR buttons[3] = {L"left", L"right", L"middle"};
    for (int i = 0; i < 3; i++) {
        c.clicks[i] = ClockSetting((std::wstring(L"clicks.") + buttons[i]).c_str());
        c.commands[i] = ClockSetting((std::wstring(L"clicks.") + buttons[i] + L"Command").c_str());
    }
    if (c.clicks[0].empty()) c.clicks[0] = L"notificationCenter";
    if (c.clicks[1].empty()) c.clicks[1] = L"taskbarMenu";

    // Measured at the widest times and dates (8 is the widest digit in most
    // fonts; the 28th of each month and a full week cover every name).
    HDC dc = GetDC(nullptr);
    int width = 0, height = 0;
    for (size_t i = 0; i < c.lines.size(); i++) {
        HGDIOBJ old = SelectObject(dc, i ? c.secondaryFont : c.primaryFont);
        int lineWidth = 0, lineHeight = 0;
        std::vector<std::pair<WORD, WORD>> dates;  // month, day
        for (WORD month = 1; month <= 12; month++) dates.push_back({month, 28});
        for (WORD day = 18; day <= 24; day++) dates.push_back({9, day});
        for (auto [month, day] : dates) {
            for (WORD hour : {10, 12, 23}) {
                SYSTEMTIME sample{2028, month, 0, day, hour, 58, 58, 0};
                FILETIME ft;
                SystemTimeToFileTime(&sample, &ft);  // fills in the day of the week
                FileTimeToSystemTime(&ft, &sample);
                std::wstring text = ExpandClockLine(c.lines[i], sample);
                SIZE extent{};
                GetTextExtentPoint32W(dc, text.empty() ? L" " : text.c_str(), text.empty() ? 1 : (int)text.size(),
                                      &extent);
                lineWidth = std::max(lineWidth, text.empty() ? 0 : (int)extent.cx);
                lineHeight = std::max(lineHeight, (int)extent.cy);
            }
        }
        SelectObject(dc, old);
        width = std::max(width, lineWidth);
        height += lineHeight + (i ? MulDiv(c.lineSpacing, dpi, USER_DEFAULT_SCREEN_DPI) : 0);
    }
    ReleaseDC(nullptr, dc);
    c.width = width + 2 * MulDiv(c.padding, dpi, USER_DEFAULT_SCREEN_DPI);
    c.height = height;
    g_clock = std::move(c);
}

// How long the tray is along the bar, and its icons' and clock's places from
// a starting point. Horizontal bars take one row of icons; vertical ones as
// many columns as fit across.
HFONT PluginGlyphFont();

// Each plugin item's length along the bar (0: not shown): its glyph and text across a
// horizontal bar, a cell with its glyph down a vertical one. Only the main bar has them.
std::vector<int> PluginLengths() {
    std::vector<int> out;
    if (!IsMainBar() || g_pluginItems.empty()) return out;
    HDC dc = GetDC(nullptr);
    auto width = [&](HFONT font, std::wstring const& text) {
        HGDIOBJ old = SelectObject(dc, font);
        SIZE s{};
        GetTextExtentPoint32W(dc, text.c_str(), (int)text.size(), &s);
        SelectObject(dc, old);
        return (int)s.cx;
    };
    for (auto const& item : g_pluginItems) {
        if (out.size() >= (size_t)kPluginMax) break;
        if (PluginAway(item)) {
            out.push_back(0);  // (in the overflow flyout, or hidden)
            continue;
        }
        if (Vertical()) {
            bool picture = out.size() < g_pluginPictures.size() && g_pluginPictures[out.size()].w;
            out.push_back(item.glyph.empty() && !picture ? 0 : TrayCell());
            continue;
        }
        size_t index = out.size();
        int picture = index < g_pluginPictures.size() ? g_pluginPictures[index].w : 0;
        int len = picture ? picture : item.glyph.empty() ? 0 : width(PluginGlyphFont(), item.glyph);
        if (!item.text.empty()) len += (len ? Px(6) : 0) + std::min(width(g_font, item.text), Px(320));
        out.push_back(len ? len + Px(12) : 0);
    }
    ReleaseDC(nullptr, dc);
    return out;
}

int TrayLength(int across) {
    int count = (int)g_tray.size();
    bool clock = g_settings.clock && !g_clock.lines.empty();
    int plugins = 0;
    for (int l : PluginLengths()) plugins += l;
    if (!count && !clock && !plugins) return 0;
    int cell = TrayCell();
    if (!Vertical()) return plugins + count * cell + (clock ? g_clock.width + Px(8) : 0);
    int columns = std::max(1, across / cell);
    int rows = (count + columns - 1) / columns;
    return plugins + rows * cell + (clock ? g_clock.height + Px(8) : 0);
}

// Where each plugin item goes among the tray's icons (the icon it comes before; g_tray.size(): after the
// last): before the first icon that comes after it in the tray's saved order, where a moved one keeps a
// "plugin:<key>" place. One never moved goes first, as before plugin items could move.
std::vector<int> PluginSlots() {
    std::vector<std::wstring> order = LoadTrayList(kTrayOrderValue);
    auto rank = [&](std::wstring const& key) {
        auto it = std::find(order.begin(), order.end(), key);
        return it == order.end() ? -1 : (int)(it - order.begin());
    };
    std::vector<int> slots(g_pluginItems.size(), 0);
    for (size_t i = 0; i < g_pluginItems.size(); i++) {
        if ((int)i == g_pluginDrag && g_pluginDragSlot >= 0) {
            slots[i] = std::min(g_pluginDragSlot, (int)g_tray.size());
            continue;
        }
        int r = rank(L"plugin:" + g_pluginItems[i].key);
        if (r < 0) continue;
        int slot = 0;
        for (; slot < (int)g_tray.size(); slot++) {
            int t = rank(g_tray[slot].key);
            if (t < 0 || t > r) break;
        }
        slots[i] = slot;
    }
    return slots;
}

void PlaceTray(int start, int across0, int across1) {
    int cell = TrayCell();
    int count = (int)g_tray.size();
    bool vertical = Vertical();
    bool clock = g_settings.clock && !g_clock.lines.empty();
    auto lengths = PluginLengths();
    g_pluginRects.assign(lengths.size(), RECT{});
    g_trayVisual.clear();
    if (!vertical) {
        // Plugin items among the icons, each in its place; the keys as laid out, for the tray's saved order.
        std::vector<int> slots = PluginSlots();
        auto plugins = [&](int slot) {
            for (size_t i = 0; i < lengths.size() && i < slots.size(); i++) {
                if (slots[i] != slot) continue;
                g_trayVisual.push_back(L"plugin:" + g_pluginItems[i].key);
                if (!lengths[i]) continue;
                g_pluginRects[i] = RECT{start, across0, start + lengths[i], across1};
                start += lengths[i];
            }
        };
        for (int i = 0; i < count; i++) {
            plugins(i);
            g_tray[i].rc = {start, across0, start + cell, across1};
            g_trayVisual.push_back(g_tray[i].key);
            start += cell;
        }
        plugins(count);
        g_clockRect = clock ? RECT{start, across0, start + g_clock.width + Px(8), across1} : RECT{};
        return;
    }
    for (size_t i = 0; i < lengths.size(); i++) {
        if (!lengths[i]) continue;
        g_pluginRects[i] = RECT{across0, start, across1, start + lengths[i]};
        start += lengths[i];
    }
    int columns = std::max(1, (across1 - across0) / cell);
    int left = across0 + (across1 - across0 - columns * cell) / 2;
    for (int i = 0; i < count; i++) {
        int x = left + (i % columns) * cell, y = start + (i / columns) * cell;
        g_tray[i].rc = {x, y, x + cell, y + cell};
    }
    int y = start + ((count + columns - 1) / columns) * cell;
    g_clockRect = clock ? RECT{across0, y, across1, y + g_clock.height + Px(8)} : RECT{};
}

////////////////////////////////////////////////////////////////////////////////
// Layout

// The bar on its screen: shown, or tucked away (2 px left in view).
RECT BarRect(bool shown) {
    MONITORINFO mi{sizeof(mi)};
    if (!g_curMonitor || !GetMonitorInfoW(g_curMonitor, &mi))
        GetMonitorInfoW(MonitorFromPoint({0, 0}, MONITOR_DEFAULTTOPRIMARY), &mi);
    RECT m = mi.rcMonitor;
    int t = Px(g_settings.thickness);
    int peek = shown ? t : Px(2);
    switch (g_settings.edge) {
        case Edge::Top: return {m.left, m.top - (t - peek), m.right, m.top + peek};
        case Edge::Bottom: return {m.left, m.bottom - peek, m.right, m.bottom + (t - peek)};
        case Edge::Left: return {m.left - (t - peek), m.top, m.left + peek, m.bottom};
        case Edge::Right: return {m.right - peek, m.top, m.right + (t - peek), m.bottom};
    }
    return m;
}

int ButtonLength(bool labels) {
    int iconOnly = Px(g_settings.iconSize + 2 * g_settings.margin) + g_scale;  // StartAllBack's pitch is one more than icon + margins
    return labels && !Vertical() ? std::max(iconOnly, Px(160)) : iconOnly;
}

// The order buttons were dragged into, by app: pinned apps by their shortcut,
// the others by the app they group under. Kept with the tray's order.
constexpr WCHAR kButtonOrderValue[] = L"ButtonOrder";
std::vector<std::wstring> g_buttonOrder;
bool g_buttonOrderLoaded = false;

std::wstring ButtonKey(Button const& b) {
    if (b.pin >= 0 && b.pin < (int)g_pins.size()) return L"pin:" + Lower(g_pins[b.pin].shortcut.file);
    if (!b.windows.empty())
        if (Win const* w = FindWin(b.windows[0])) return w->key;
    return {};
}

// Dragged buttons first, in their order; the rest after them as they came.
void ApplyButtonOrder() {
    if (!g_buttonOrderLoaded) {
        g_buttonOrder = LoadTrayList(kButtonOrderValue);
        g_buttonOrderLoaded = true;
    }
    ApplyTrayOrder(g_buttons, g_buttonOrder, ButtonKey);
}

// Buttons for the pinned apps and the current windows, following "Combine
// taskbar buttons": the pins first, in their order, each with its app's
// windows, then the other apps in the order their windows appeared; then the
// order they were dragged into.
// Whether a window's button goes on this bar. One bar (or Windows set to show
// every button everywhere): all of them; with "taskbar where the window is
// open" only the bar of its screen; with "main taskbar and where the window is
// open" the main bar has them all and the others their own.
bool WinOnBar(HWND h) {
    HMONITOR main = MonitorFromPoint({0, 0}, MONITOR_DEFAULTTOPRIMARY);
    HMONITOR mine = g_curMonitor ? g_curMonitor : main;
    HMONITOR of = MonitorFromWindow(h, MONITOR_DEFAULTTONEAREST);
    if (g_bars.size() < 2) return !g_settings.onlyThisScreen || of == main;
    switch (g_settings.mmMode) {
        case 0: return true;
        case 1: return mine == main || of == mine;
        default: return of == mine;
    }
}

void BuildButtons(int available) {
    std::vector<std::vector<Win const*>> pinned(g_pins.size());
    std::vector<Win const*> order;
    std::vector<Win*> mine;  // the windows this bar shows
    for (auto& w : g_wins)
        if (WinOnBar(w.hwnd)) mine.push_back(&w);
    // Pinned apps stay on the main bar; the others show pins only when Windows shares every button everywhere.
    bool showPins = IsMainBar() || g_bars.size() < 2 || g_settings.mmMode == 0;
    for (auto* w : mine) {
        int pin = PinOf(*w);
        if (pin >= 0) pinned[pin].push_back(w);
    }
    // Windows of one app sit together unless "Never group".
    auto unpinned = [](Win const& w) { return PinOf(w) < 0; };
    if (g_settings.combine == Combine::NeverGroup) {
        for (auto* w : mine)
            if (unpinned(*w)) order.push_back(w);
    } else {
        std::vector<std::wstring> keys;
        for (auto* w : mine)
            if (unpinned(*w) && std::find(keys.begin(), keys.end(), w->key) == keys.end()) keys.push_back(w->key);
        for (auto& k : keys)
            for (auto* w : mine)
                if (w->key == k) order.push_back(w);
    }
    auto build = [&](bool combine) {
        g_buttons.clear();
        for (size_t p = 0; p < g_pins.size(); p++) {
            if (pinned[p].empty() && !showPins) continue;
            if (pinned[p].empty() || combine) {
                Button b;
                b.pin = (int)p;
                for (auto* w : pinned[p]) b.windows.push_back(w->hwnd);
                g_buttons.push_back(b);
            } else {
                for (auto* w : pinned[p]) g_buttons.push_back({{w->hwnd}, {}, (int)p});
            }
        }
        for (auto* w : order) {
            if (combine && !g_buttons.empty() && g_buttons.back().pin < 0 && !g_buttons.back().windows.empty()) {
                Win const* first = FindWin(g_buttons.back().windows[0]);
                if (first && first->key == w->key) {
                    g_buttons.back().windows.push_back(w->hwnd);
                    continue;
                }
            }
            g_buttons.push_back({{w->hwnd}});
        }
        ApplyButtonOrder();
    };
    switch (g_settings.combine) {
        case Combine::Always: build(true); g_labels = false; break;
        case Combine::Never:
        case Combine::NeverGroup: build(false); g_labels = true; break;
        case Combine::NeverHideLabels: build(false); g_labels = false; break;
        case Combine::WhenFull:
            build(false);
            g_labels = true;
            if ((int)g_buttons.size() * ButtonLength(true) > available) {
                build(true);
                if ((int)g_buttons.size() * ButtonLength(true) > available) g_labels = false;
            }
            break;
    }
}

std::wstring TooltipText(UINT_PTR id);  // (with the tooltips' placing, below)
std::wstring g_tipShownText;            // what the tooltip showing was given last

// The tooltips follow the buttons, the tray icons, the clock and a theme's own buttons.
void UpdateTooltips(HWND wnd) {
    int count = (int)g_buttons.size();
    // Tooltips follow the buttons. Rebuilding them hides the one showing, so
    // that's only done when something moved; a changed text (a tray icon's
    // new tooltip, another window on a button) is refreshed in place.
    if (g_tooltip) {
        std::vector<std::pair<UINT_PTR, RECT>> placed;
        std::wstring content;
        for (int i = 0; i < count; i++) {
            placed.push_back({(UINT_PTR)i + 1, g_buttons[i].rc});
            for (HWND h : g_buttons[i].windows) content += std::to_wstring((ULONG_PTR)h) + L",";
            content += std::to_wstring(g_buttons[i].pin) + L";";
        }
        for (int i = 0; i < (int)g_tray.size(); i++) {
            placed.push_back({(UINT_PTR)kHitTray + i, g_tray[i].rc});
            content += g_tray[i].icon.tip + L";";
        }
        for (int i = 0; i < (int)g_pluginRects.size() && i < (int)g_pluginItems.size(); i++) {
            if (IsRectEmpty(&g_pluginRects[i])) continue;
            placed.push_back({(UINT_PTR)kHitPlugin + i, g_pluginRects[i]});
            content += g_pluginItems[i].tip + L";";
        }
        if (!IsRectEmpty(&g_clockRect)) placed.push_back({(UINT_PTR)kHitClock, g_clockRect});
        if (!IsRectEmpty(&g_showDesktopRect)) placed.push_back({(UINT_PTR)kHitShowDesktop, g_showDesktopRect});
        if (!IsRectEmpty(&g_pagerRect))
            for (int i = 0; i < (int)g_pagerItems.size(); i++) {
                placed.push_back({(UINT_PTR)kHitDesktops + i, g_pagerItems[i]});
                content += PagerLabel(i) + L";";
            }
        for (int i = 0; i < (int)g_cssTargets.size(); i++) {
            placed.push_back({(UINT_PTR)kHitCss + i, g_cssTargets[i].rc});
            content += g_cssTargets[i].tip + L";";
        }
        // (Kept per bar, with its state.)
        bool same = placed.size() == g_placed.size() &&
                    std::equal(placed.begin(), placed.end(), g_placed.begin(), [](auto const& a, auto const& b) {
                        return a.first == b.first && EqualRect(&a.second, &b.second);
                    });
        if (same) {
            // Only the tooltip showing is refreshed, and only when its own text changed: a refresh puts it
            // back where the tooltip control would (TooltipProc keeps it where PlaceTooltip puts it), so a
            // tray icon's tooltip changing every second mustn't touch another one that's showing.
            if (content != g_placedContent && IsWindowVisible(g_tooltip)) {
                TOOLINFOW ti{sizeof(ti)};
                if (SendMessageW(g_tooltip, TTM_GETCURRENTTOOLW, 0, (LPARAM)&ti) && TooltipText(ti.uId) != g_tipShownText)
                    SendMessageW(g_tooltip, TTM_UPDATE, 0, 0);
            }
            g_placedContent = content;
            return;
        }
        g_placed = placed;
        g_placedContent = content;
        int tools = (int)SendMessageW(g_tooltip, TTM_GETTOOLCOUNT, 0, 0);
        for (int i = tools - 1; i >= 0; i--) {
            TOOLINFOW ti{sizeof(ti)};
            if (SendMessageW(g_tooltip, TTM_ENUMTOOLSW, i, (LPARAM)&ti)) SendMessageW(g_tooltip, TTM_DELTOOLW, 0, (LPARAM)&ti);
        }
        auto addTool = [&](UINT_PTR id, RECT const& r) {
            TOOLINFOW ti{sizeof(ti)};
            ti.uFlags = TTF_SUBCLASS;
            ti.hwnd = wnd;
            ti.uId = id;
            ti.rect = r;
            ti.lpszText = LPSTR_TEXTCALLBACKW;
            SendMessageW(g_tooltip, TTM_ADDTOOLW, 0, (LPARAM)&ti);
        };
        for (auto const& [id, r] : placed) addTool(id, r);
    }
}

void Layout(HWND wnd) {
    g_barsDrawn = 0;
    if (CssActive()) {
        // A YASB theme lays the bar out; the buttons it will place are made here.
        g_showDesktopRect = {};
        g_pagerRect = {};
        g_pluginRects.clear();
        if (!IsMainBar()) g_tray = g_settings.trayEverywhere && g_settings.tray ? g_mainBar.tray : std::vector<TrayItem>();
        BuildButtons(1 << 20);
        CssLayout(wnd);
        return;
    }
    RECT rc;
    GetClientRect(wnd, &rc);
    bool vertical = Vertical();
    int along = vertical ? rc.bottom : rc.right;     // the bar's length
    int across = vertical ? rc.right : rc.bottom;    // its thickness
    bool segmented = EffSegments() != Segments::Off;
    // Segments as StartAllBack lays them out: islands 5 px off every edge,
    // hugging their contents (the orb island is the orb's width, the task
    // island exactly its buttons, which fill its height). Flat islands keep
    // a little room inside their border.
    bool themedIslands = segmented && g_themed && g_theme.bar;
    int inset = segmented ? Px(5) : 0;               // islands float off the screen edge
    int pad = Px(2);
    // Between the Start button and the tasks: an orb frame of 54 px puts the
    // first button at x = 59, as StartAllBack does.
    int gap = segmented ? Px(10) : g_orb.bitmap ? Px(2) : Px(6);
    int islandPad = segmented && !themedIslands ? Px(3) : 0;  // inside an island, along the bar

    // The orb at its own size, centred across the bar like StartAllBack draws
    // it, so pixel-art orbs stay sharp; only a big orb is scaled down. With
    // no orb, a square for the Tourne mark.
    int room = across - 2 * inset;
    int orbSize = room - 2 * pad, orbW = orbSize;
    if (g_orb.bitmap) {
        int fh = g_orb.FrameHeight();
        // StartAllBack draws an orb's frame at up to 54 px (its own cache says
        // 54 x 54 at 96 dpi, for a 216 px frame as for a 54 px one); the art
        // sits in transparent padding, so the frame may be taller than the bar.
        orbSize = Px(std::min(fh, 54));
        orbW = MulDiv(orbSize, g_orb.width, fh);
    } else {
        orbSize = orbW = std::min(room - 2 * pad, Px(24));
    }
    // Only the main bar has the Start button.
    bool mainBar = IsMainBar();
    if (!mainBar) {
        orbSize = orbW = 0;
        gap = 0;
    }
    int orbAlong = vertical ? orbSize : orbW;
    int orbAcross = vertical ? orbW : orbSize;

    // The tray goes at the far end, or just before the orb when that's at
    // the end; the task buttons get what's left.
    int buttonInset = inset + (segmented && !themedIslands ? Px(2) : 0);
    // Another screen's bar shows the main bar's tray when set (its own copy: rects and hover are its own).
    if (!mainBar) g_tray = g_settings.trayEverywhere && g_settings.tray ? g_mainBar.tray : std::vector<TrayItem>();
    int trayLen = TrayLength(across - 2 * buttonInset);
    // The desktops pager, when docked here: with the tray (before it or after the clock) or after Start.
    int pagerLen = DockedPagerLength();
    int pagerPlace = pagerLen ? DockedPagerPlace() : -1;
    bool pagerInTray = pagerPlace == kPagerBeforeTray || pagerPlace == kPagerAfterClock;
    int pagerGap = trayLen ? Px(4) : 0;
    int trayBlock = trayLen + (pagerInTray ? pagerLen + pagerGap : 0);  // the tray and a pager with it
    int lead = pagerPlace == kPagerAfterStart ? pagerLen + gap : 0;     // a pager after Start
    int trayRoom = trayBlock ? trayBlock + gap + 2 * islandPad : 0;
    // The show desktop button: a sliver at the very end, the style's own size (StartAllBack's styles: 10 px
    // along a horizontal bar, 14 down a vertical one); with segments, at the end of the tray's island.
    int sdLen = 0;
    if (g_settings.showDesktop && g_settings.startAt != StartAt::End) {
        int size = vertical ? 14 : 10;
        if (HTHEME t = g_themed ? g_theme.showDesktop[vertical ? 1 : 0] : nullptr)
            GetThemeInt(t, 1, 1, vertical ? TMT_HEIGHT : TMT_WIDTH, &size);
        sdLen = Px(std::clamp(size, 2, 40));
    }
    int available = along - (2 * pad + orbAlong + lead + gap + 2 * islandPad) - trayRoom - sdLen - Px(8);
    BuildButtons(available);
    int len = ButtonLength(g_labels);
    int count = (int)g_buttons.size();
    if (count * len > available && count) len = std::max(Px(g_settings.iconSize + 4), available / count);
    // A group of windows is drawn as a stack, its edge showing behind the button: room for it,
    // so the edge doesn't run into the next button.
    int total = count * len;
    // With a style, as much as its edge shows (StartAllBack: the next button starts right after it,
    // 5 px for two windows in Windows 7's and Bouquet's styles, 8 for more).
    int stackTwo = 4, stackMore = 4;
    if (g_themed && g_theme.band) {
        stackTwo = StackEdgeWidth(kBandStack, 2);
        stackMore = StackEdgeWidth(kBandStack, 3);
    }
    for (auto& b : g_buttons) {
        b.stack = b.windows.size() > 1 && !g_labels ? (b.windows.size() > 2 ? stackMore : stackTwo) : 0;
        total += b.stack;
    }

    // Where the Start button and the task buttons go.
    int edgeStart = (segmented ? inset : g_orb.bitmap ? Px(4) : 2 * pad) + islandPad;
    int orbStart = edgeStart, tasksStart;
    switch (g_settings.startAt) {
        case StartAt::WithTasks: {
            int block = orbAlong + lead + gap + 2 * islandPad + total;
            int blockStart = g_settings.centered ? std::max(edgeStart, (along - block) / 2) : edgeStart;
            orbStart = blockStart;
            tasksStart = blockStart + orbAlong + lead + gap + 2 * islandPad;
            break;
        }
        case StartAt::End:
            orbStart = along - edgeStart - orbAlong;
            tasksStart = g_settings.centered ? (along - total) / 2 : edgeStart;
            tasksStart = std::min(tasksStart, orbStart - lead - gap - 2 * islandPad - total);
            tasksStart = std::max(tasksStart, edgeStart);
            break;
        default:
            tasksStart = g_settings.centered ? (along - total) / 2 : orbStart + orbAlong + lead + gap + 2 * islandPad;
            tasksStart = std::max(tasksStart, orbStart + orbAlong + lead + gap + 2 * islandPad);
            break;
    }
    int trayEnd = g_settings.startAt == StartAt::End ? orbStart - lead - gap - 2 * islandPad : along - edgeStart - sdLen;
    int sdAcross0 = segmented ? inset : 0, sdAcross1 = segmented ? across - inset : across;
    int sdAlong0 = segmented ? trayEnd : along - sdLen;
    g_showDesktopRect = !sdLen   ? RECT{}
                        : vertical ? RECT{sdAcross0, sdAlong0, sdAcross1, sdAlong0 + sdLen}
                                   : RECT{sdAlong0, sdAcross0, sdAlong0 + sdLen, sdAcross1};
    int trayStart = trayEnd - trayBlock;
    if (trayBlock) {
        // Centred buttons move over rather than run into the tray.
        int limit = trayStart - gap - 2 * islandPad - total;
        if (tasksStart > limit) {
            int shift = tasksStart - std::max(limit, edgeStart);
            tasksStart -= shift;
            if (g_settings.startAt == StartAt::WithTasks) orbStart -= shift;
        }
    }

    int crossPos = (across - orbAcross) / 2;
    g_orbRect = vertical ? RECT{crossPos, orbStart, crossPos + orbAcross, orbStart + orbAlong}
                         : RECT{orbStart, crossPos, orbStart + orbAlong, crossPos + orbAcross};
    PlaceTray(trayStart + (pagerPlace == kPagerBeforeTray ? pagerLen + pagerGap : 0), buttonInset, across - buttonInset);
    int pagerStart = pagerPlace == kPagerBeforeTray  ? trayStart
                   : pagerPlace == kPagerAfterClock  ? trayEnd - pagerLen
                   : g_settings.startAt == StartAt::End ? orbStart - lead
                                                       : orbStart + orbAlong + gap;
    if (pagerLen)
        PlaceDockedPager(vertical ? RECT{buttonInset, pagerStart, across - buttonInset, pagerStart + pagerLen}
                                  : RECT{pagerStart, buttonInset, pagerStart + pagerLen, across - buttonInset});
    else
        g_pagerRect = {};
    int along0 = tasksStart;
    for (int i = 0; i < count; i++) {
        int a = along0, l = len + g_buttons[i].stack;
        g_buttons[i].rc = vertical ? RECT{buttonInset, a, across - buttonInset, a + l}
                                   : RECT{a, buttonInset, a + l, across - buttonInset};
        along0 += l;
    }

    // Segments: an island for the Start button (unless only the centre one
    // floats), one around the task buttons and one around the tray.
    g_islands.clear();
    g_centralIsland = -1;
    if (segmented) {
        auto island = [&](int a0, int a1) {
            return vertical ? RECT{inset, a0, across - inset, a1} : RECT{a0, inset, a1, across - inset};
        };
        // A pager after Start shares the Start button's island, or has its own when Start has none.
        bool startIsland = EffSegments() == Segments::Three && mainBar;
        bool pagerLeads = pagerPlace == kPagerAfterStart;
        if (startIsland) {
            int a0 = pagerLeads ? std::min(orbStart, pagerStart) : orbStart;
            int a1 = pagerLeads ? std::max(orbStart + orbAlong, pagerStart + pagerLen) : orbStart + orbAlong;
            g_islands.push_back(island(a0 - islandPad, a1 + islandPad));
        } else if (pagerLeads) {
            g_islands.push_back(island(pagerStart - islandPad, pagerStart + pagerLen + islandPad));
        }
        if (count) {
            g_centralIsland = (int)g_islands.size();
            g_islands.push_back(island(tasksStart - islandPad, tasksStart + total + islandPad));
        }
        // The tray's island, with the show desktop button at its end (or on its own, with no tray).
        if (trayBlock || sdLen) g_islands.push_back(island(trayStart - islandPad, trayEnd + sdLen + islandPad));
    }

    UpdateTooltips(wnd);
}

// The gaps between segments take clicks as the bar's empty space (drawn at alpha 1), except over windows of
// Tourne's own that sit in the bar under it (Tourne'Table's media strip, its quick settings on a right click):
// there they stay see-through, so those still get their clicks. In the bar's own terms (client pixels).
std::vector<RECT> BarPassThrough(HWND wnd) {
    std::vector<RECT> out;
    RECT bar;
    if (!GetWindowRect(wnd, &bar)) return out;
    for (HWND h = GetWindow(wnd, GW_HWNDNEXT); h; h = GetWindow(h, GW_HWNDNEXT)) {
        DWORD pid = 0;
        GetWindowThreadProcessId(h, &pid);
        if (pid != GetCurrentProcessId() || !IsWindowVisible(h) || (GetWindowLongW(h, GWL_EXSTYLE) & WS_EX_TRANSPARENT)) continue;
        wchar_t cls[64] = L"";
        GetClassNameW(h, cls, 64);
        if (!wcscmp(cls, L"Progman") || !wcsncmp(cls, L"TourneTaskbar", 13)) continue;
        RECT r, in;
        if (!GetWindowRect(h, &r) || !IntersectRect(&in, &r, &bar)) continue;
        if ((in.right - in.left) * (in.bottom - in.top) * 2 > (bar.right - bar.left) * (bar.bottom - bar.top)) continue;  // (not a big one)
        OffsetRect(&in, -bar.left, -bar.top);
        out.push_back(in);
    }
    return out;
}

bool InPassThrough(std::vector<RECT> const& rects, int x, int y) {
    for (auto const& r : rects)
        if (x >= r.left && x < r.right && y >= r.top && y < r.bottom) return true;
    return false;
}

////////////////////////////////////////////////////////////////////////////////
// Drawing. The bar is a layered window: everything is drawn opaque, then the
// islands (or the whole bar) are cut out, so the desktop shows between
// segments and clicks there go through to it.

bool ButtonActive(Button const& b) {
    HWND fg = ForegroundRoot();
    for (HWND h : b.windows)
        if (h == fg && !IsIconic(h)) return true;
    return false;
}

int ButtonState(int index) {
    Button const& b = g_buttons[index];
    bool active = ButtonActive(b);
    bool flashing = false;
    for (HWND h : b.windows)
        if (Win* w = FindWin(h); w && w->flashing) flashing = true;
    if (flashing && !active) return kStateFlashing;
    bool hot = g_hot == index;
    bool pressed = hot && g_pressed == index;
    if (active) return pressed ? kStateActivePressed : hot ? kStateActiveHot : kStateActive;
    return pressed ? kStatePressed : hot ? kStateHot : kStateNormal;
}

std::wstring Title(HWND h) {
    WCHAR text[256] = L"";
    GetWindowTextW(h, text, ARRAYSIZE(text));
    return text;
}

void Fill(HDC dc, RECT const& r, uint32_t rgb) {
    SetDCBrushColor(dc, PaletteRef(rgb));
    FillRect(dc, &r, (HBRUSH)GetStockObject(DC_BRUSH));
}

int BackgroundPart() {
    switch (g_settings.edge) {
        case Edge::Top: return kBarBackgroundTop;
        case Edge::Left: return kBarBackgroundLeft;
        case Edge::Right: return kBarBackgroundRight;
        default: return kBarBackgroundBottom;
    }
}

void DrawThemePartAlpha(HDC dc, HTHEME theme, int part, int state, RECT const& r);

// A stretch of bar background: the theme's, or the scheme's ground with its
// border on the side facing the screen (a hairline for the full bar, all
// round for an island).
int g_barFrame = 0;  // px of frame around an opaque themed bar part (kept out of the custom colouring)
void DrawBackground(HDC dc, RECT const& r, bool island) {
    auto const& p = g_settings.palette;
    // A style's own bar background goes over StartAllBack's gradient, which
    // shows through where the style leaves it clear (Windows 7's glass, and
    // no style at all).
    if (g_themed && g_theme.bar) {
        // An opaque bar part (Bouquet's) sits 3 px in from the bar's edge, inside a frame
        // (#1E2E36, measured against StartAllBack's own bar); see-through ones fill the bar.
        static HTHEME probed = nullptr;
        static bool opaque = false;
        if (probed != g_theme.bar) {
            probed = g_theme.bar;
            uint32_t px[16 * 16] = {};
            BITMAPINFO bi{};
            bi.bmiHeader = {sizeof(BITMAPINFOHEADER), 16, -16, 1, 32, BI_RGB};
            void* bits = nullptr;
            HDC sd = GetDC(nullptr);
            HBITMAP dib = CreateDIBSection(sd, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
            if (dib) {
                HDC md = CreateCompatibleDC(sd);
                HGDIOBJ old = SelectObject(md, dib);
                memset(bits, 0, sizeof(px));
                RECT pr{0, 0, 16, 16};
                DrawThemeBackground(g_theme.bar, md, BackgroundPart(), 0, &pr, nullptr);
                GdiFlush();
                opaque = (((uint32_t*)bits)[8 * 16 + 8] >> 24) == 0xFF;
                SelectObject(md, old);
                DeleteDC(md);
                DeleteObject(dib);
            }
            ReleaseDC(nullptr, sd);
        }
        g_barFrame = 0;
        if (opaque && g_settings.sabBackdrop && !island) {
            g_barFrame = Px(3);
            Fill(dc, r, 0x1E2E36);
            RECT in = r;
            InflateRect(&in, -Px(3), -Px(3));
            DrawThemePartAlpha(dc, g_theme.bar, BackgroundPart(), 0, in);
            return;
        }
        DrawThemePartAlpha(dc, g_theme.bar, BackgroundPart(), 0, r);
        return;
    }
    if (g_settings.sabBackdrop && !island) return;  // the gradient (already laid down)
    Fill(dc, r, p.ground);
    if (island) {
        SetDCBrushColor(dc, PaletteRef(p.edge));
        FrameRect(dc, &r, (HBRUSH)GetStockObject(DC_BRUSH));
        return;
    }
    RECT line = r;
    switch (g_settings.edge) {
        case Edge::Top: line.top = line.bottom - g_scale; break;
        case Edge::Bottom: line.bottom = line.top + g_scale; break;
        case Edge::Left: line.left = line.right - g_scale; break;
        case Edge::Right: line.right = line.left + g_scale; break;
    }
    Fill(dc, line, p.line);
}

// A theme part drawn with its own transparency: into a clear 32-bit picture,
// then blended over what is there (as StartAllBack does), so a part's
// see-through areas show the bar's background.
// The scratch pictures theme parts are drawn into, kept by size: a redraw draws a dozen
// buttons of the same size, and making a picture and a device context for each was a
// large share of its time.
struct AlphaScratch {
    int w = 0, h = 0;
    HDC dc = nullptr;
    HBITMAP dib = nullptr;
    HGDIOBJ old = nullptr;
    void* bits = nullptr;
};
std::vector<AlphaScratch> g_alphaScratch;

void FreeAlphaScratch() {
    for (auto& s : g_alphaScratch) {
        if (s.dc) {
            SelectObject(s.dc, s.old);
            DeleteDC(s.dc);
        }
        if (s.dib) DeleteObject(s.dib);
    }
    g_alphaScratch.clear();
}

// A cleared 32-bit scratch bitmap of this size (the last few sizes are kept).
AlphaScratch* ScratchFor(HDC dc, int w, int h) {
    AlphaScratch* scratch = nullptr;
    for (auto& s : g_alphaScratch)
        if (s.w == w && s.h == h) scratch = &s;
    if (!scratch) {
        if (g_alphaScratch.size() >= 8) {  // the oldest goes
            AlphaScratch& first = g_alphaScratch.front();
            SelectObject(first.dc, first.old);
            DeleteDC(first.dc);
            DeleteObject(first.dib);
            g_alphaScratch.erase(g_alphaScratch.begin());
        }
        BITMAPINFO bi{};
        bi.bmiHeader = {sizeof(BITMAPINFOHEADER), w, -h, 1, 32, BI_RGB};
        AlphaScratch made;
        made.w = w;
        made.h = h;
        made.dib = CreateDIBSection(dc, &bi, DIB_RGB_COLORS, &made.bits, nullptr, 0);
        if (!made.dib) return nullptr;
        made.dc = CreateCompatibleDC(dc);
        made.old = SelectObject(made.dc, made.dib);
        g_alphaScratch.push_back(made);
        scratch = &g_alphaScratch.back();
    }
    memset(scratch->bits, 0, (size_t)w * h * 4);
    return scratch;
}

void DrawThemePartAlpha(HDC dc, HTHEME theme, int part, int state, RECT const& r) {
    int w = r.right - r.left, h = r.bottom - r.top;
    if (w <= 0 || h <= 0) return;
    AlphaScratch* scratch = ScratchFor(dc, w, h);
    if (!scratch) {
        DrawThemeBackground(theme, dc, part, state, &r, nullptr);
        return;
    }
    RECT local{0, 0, w, h};
    DrawThemeBackground(theme, scratch->dc, part, state, &local, nullptr);
    GdiFlush();
    BLENDFUNCTION blend{AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
    AlphaBlend(dc, r.left, r.top, w, h, scratch->dc, 0, 0, w, h, blend);
}

// A stack edge picture at its own size (the style's, times the scale), into a scratch bitmap.
AlphaScratch* RenderStackEdge(int part, int state, int& w, int& h) {
    SIZE natural{};
    if (!g_theme.band || FAILED(GetThemePartSize(g_theme.band, nullptr, part, state, nullptr, TS_TRUE, &natural)) ||
        natural.cx <= 0 || natural.cy <= 0)
        return nullptr;
    w = Px(natural.cx);
    h = Px(natural.cy);
    HDC screen = GetDC(nullptr);
    AlphaScratch* scratch = ScratchFor(screen, w, h);
    ReleaseDC(nullptr, screen);
    if (!scratch) return nullptr;
    RECT local{0, 0, w, h};
    DrawThemeBackground(g_theme.band, scratch->dc, part, state, &local, nullptr);
    GdiFlush();
    return scratch;
}

// How far a stack edge reaches past the button: its picture's last column with anything in it.
int StackEdgeWidth(int part, int state) {
    int w = 0, h = 0;
    AlphaScratch* scratch = RenderStackEdge(part, state, w, h);
    if (!scratch) return Px(4);
    auto const* src = (uint32_t const*)scratch->bits;
    int reach = 0;
    for (int y = 0; y < h; y++)
        for (int x = reach; x < w; x++)
            if (src[(size_t)y * w + x] >> 24) reach = x + 1;
    return reach ? reach : Px(4);
}

// Shadows: a shape's alpha, blurred, kept by its pixels (an icon that changes gets its own).
struct ShadowShape {
    int w = 0, h = 0;
    std::vector<uint8_t> a;
};
std::unordered_map<uint64_t, ShadowShape> g_shadowCache;

// Box blur of radius r, twice, across and down: close to a Gaussian, and cheap.
void BlurAlpha(std::vector<uint8_t>& a, int w, int h, int r) {
    if (r <= 0) return;
    std::vector<uint8_t> tmp(a.size());
    int span = 2 * r + 1;
    for (int pass = 0; pass < 2; pass++) {
        for (int y = 0; y < h; y++) {
            int sum = 0;
            for (int x = 0; x <= r && x < w; x++) sum += a[(size_t)y * w + x];
            for (int x = 0; x < w; x++) {
                tmp[(size_t)y * w + x] = (uint8_t)(sum / span);
                if (x - r >= 0) sum -= a[(size_t)y * w + x - r];
                if (x + r + 1 < w) sum += a[(size_t)y * w + x + r + 1];
            }
        }
        for (int x = 0; x < w; x++) {
            int sum = 0;
            for (int y = 0; y <= r && y < h; y++) sum += tmp[(size_t)y * w + x];
            for (int y = 0; y < h; y++) {
                a[(size_t)y * w + x] = (uint8_t)(sum / span);
                if (y - r >= 0) sum -= tmp[(size_t)(y - r) * w + x];
                if (y + r + 1 < h) sum += tmp[(size_t)(y + r + 1) * w + x];
            }
        }
    }
}

ShadowShape const& ShadowOf(uint8_t const* mask, int w, int h) {
    int r = Px(g_settings.shadowSoftness);
    uint64_t key = 1469598103934665603ULL;
    auto mix = [&](uint64_t v) { key = (key ^ v) * 1099511628211ULL; };
    mix((uint64_t)w);
    mix((uint64_t)h);
    mix((uint64_t)r);
    for (int i = 0; i < w * h; i++) mix(mask[i]);
    auto it = g_shadowCache.find(key);
    if (it != g_shadowCache.end()) return it->second;
    if (g_shadowCache.size() >= 128) g_shadowCache.clear();
    int pad = 2 * r;
    ShadowShape shape;
    shape.w = w + 2 * pad;
    shape.h = h + 2 * pad;
    shape.a.assign((size_t)shape.w * shape.h, 0);
    for (int y = 0; y < h; y++) memcpy(&shape.a[(size_t)(y + pad) * shape.w + pad], mask + (size_t)y * w, w);
    BlurAlpha(shape.a, shape.w, shape.h, r);
    return g_shadowCache.emplace(key, std::move(shape)).first->second;
}

// The shadow colour at strength sa, laid over a premultiplied pixel.
uint32_t OverShadow(uint32_t d, uint32_t sa) {
    COLORREF c = g_settings.shadowColor;
    uint32_t src[4] = {GetBValue(c) * sa / 255, GetGValue(c) * sa / 255, GetRValue(c) * sa / 255, sa};
    uint32_t out = 0;
    for (int k = 0; k < 4; k++) out |= std::min<uint32_t>(255, src[k] + ((d >> (8 * k)) & 0xFF) * (255 - sa) / 255) << (8 * k);
    return out;
}

// Lays a shape's shadow (its top left at x, y on the bar) under what's drawn next. Where the bar's
// background is still showing and the colour scheme will replace it (shade given), the shadow is kept
// aside to fall on the scheme's colour at the end.
void CastShadow(uint32_t* bits, int bw, int bh, std::vector<uint8_t>* shade, uint8_t const* mask, int w, int h, int x, int y) {
    auto const& s = g_settings;
    ShadowShape const& shape = ShadowOf(mask, w, h);
    int pad = (shape.w - w) / 2;
    int ox = x - pad + Px(s.shadowX), oy = y - pad + Px(s.shadowY);
    uint32_t strength = s.shadowStrength * 255 / 100;
    for (int j = 0; j < shape.h; j++) {
        int py = oy + j;
        if (py < 0 || py >= bh) continue;
        for (int i = 0; i < shape.w; i++) {
            int px = ox + i;
            if (px < 0 || px >= bw) continue;
            uint32_t sa = shape.a[(size_t)j * shape.w + i] * strength / 255;
            if (!sa) continue;
            size_t at = (size_t)py * bw + px;
            if (shade && (bits[at] >> 24) == 1) {
                uint8_t& kept = (*shade)[at];
                kept = (uint8_t)(kept + sa * (255 - kept) / 255);
            } else {
                bits[at] = OverShadow(bits[at], sa);
            }
        }
    }
}

// An icon's shadow: its own shape (from its alpha, or from its pixels if it has none).
void IconShadow(uint32_t* bits, int bw, int bh, std::vector<uint8_t>* shade, HICON icon, int size, int x, int y) {
    HDC screen = GetDC(nullptr);
    AlphaScratch* scratch = ScratchFor(screen, size, size);
    ReleaseDC(nullptr, screen);
    if (!scratch) return;
    DrawIconEx(scratch->dc, 0, 0, icon, size, size, 0, nullptr, DI_NORMAL);
    GdiFlush();
    auto const* px = (uint32_t const*)scratch->bits;
    std::vector<uint8_t> mask((size_t)size * size);
    bool any = false;
    for (size_t i = 0; i < mask.size(); i++) any |= (mask[i] = (uint8_t)(px[i] >> 24)) != 0;
    if (!any)
        for (size_t i = 0; i < mask.size(); i++) mask[i] = (px[i] & 0xFFFFFF) ? 255 : 0;
    CastShadow(bits, bw, bh, shade, mask.data(), size, size, x, y);
}

// A button frame's shadow: its rectangle.
void FrameShadow(uint32_t* bits, int bw, int bh, std::vector<uint8_t>* shade, RECT const& r) {
    int w = r.right - r.left, h = r.bottom - r.top;
    if (w <= 0 || h <= 0) return;
    std::vector<uint8_t> mask((size_t)w * h, 255);
    CastShadow(bits, bw, bh, shade, mask.data(), w, h, r.left, r.top);
}
// The edge of a stack of windows, beside a button's frame (after it along the bar). The style's
// picture is the button's own end repeated, as the windows behind would show it (Windows 7's and
// Bouquet's styles: 8x40, one edge for two windows, two for more). StartAllBack draws it at its own
// width, right against the frame, as tall as the frame: its dark outer rows at the frame's, and the
// rest one straight line, without the little caps the picture has at the top and bottom (uxtheme
// would squeeze those in, a pixel off the button's lines, as hooks). On a bar at the side it's turned
// to go below the frame.
void DrawStackEdge(uint32_t* bits, int bitsW, int bitsH, int part, int state, RECT const& fr) {
    int w = 0, h = 0;
    AlphaScratch* scratch = RenderStackEdge(part, state, w, h);
    if (!scratch) return;
    auto const* src = (uint32_t const*)scratch->bits;
    bool vertical = Vertical();
    int across = vertical ? fr.right - fr.left : fr.bottom - fr.top;
    if (across <= 0) return;
    // Its outer rows: those the same as its first (or last) row.
    auto sameRow = [&](int a, int b) { return memcmp(src + (size_t)a * w, src + (size_t)b * w, (size_t)w * 4) == 0; };
    int endA = 1, endB = 1;
    while (endA < h / 4 && sameRow(endA, 0)) endA++;
    while (endB < h / 4 && sameRow(h - 1 - endB, h - 1)) endB++;
    endA = std::min(endA, across / 2);
    endB = std::min(endB, across / 2);
    for (int j = 0; j < across; j++) {
        int row = j < endA ? j : j >= across - endB ? h - (across - j) : h / 2;
        for (int c = 0; c < w; c++) {
            int x = vertical ? fr.left + j : fr.right + c, y = vertical ? fr.bottom + c : fr.top + j;
            if (x < 0 || y < 0 || x >= bitsW || y >= bitsH) continue;
            uint32_t s = src[(size_t)row * w + c];
            uint32_t a = s >> 24;
            if (!a) continue;
            uint32_t& d = bits[(size_t)y * bitsW + x];
            uint32_t out = 0;
            for (int shift = 0; shift < 32; shift += 8) {
                uint32_t sc = (s >> shift) & 0xFF, dc = (d >> shift) & 0xFF;
                out |= std::min<uint32_t>(255, sc + dc * (255 - a) / 255) << shift;
            }
            d = out;
        }
    }
}
// A task button without a theme: hover and pressed fills, and a bar in the
// accent colour along the screen side for the active window (warning
// colour while it wants attention).
void DrawFlatButton(HDC dc, RECT const& r, int state) {
    auto const& p = g_settings.palette;
    bool active = state >= kStateActive && state <= kStateActivePressed;
    if (state == kStateFlashing) {
        Fill(dc, r, MixRgb(p.ground, p.warn, 0.30));
    } else if (state == kStatePressed || state == kStateActivePressed) {
        Fill(dc, r, p.raised);
    } else if (state == kStateHot || state == kStateActiveHot) {
        Fill(dc, r, p.hover);
    } else if (active) {
        Fill(dc, r, MixRgb(p.ground, p.raised, 0.6));
    }
    if (active || state == kStateFlashing) {
        int thick = Px(2);
        int w = r.right - r.left, h = r.bottom - r.top;
        RECT mark = r;
        if (Vertical()) {
            mark.top += h / 5;
            mark.bottom -= h / 5;
            if (g_settings.edge == Edge::Left) mark.right = mark.left + thick; else mark.left = mark.right - thick;
        } else {
            mark.left += w / 5;
            mark.right -= w / 5;
            if (g_settings.edge == Edge::Top) mark.bottom = mark.top + thick; else mark.top = mark.bottom - thick;
        }
        Fill(dc, mark, state == kStateFlashing ? p.warn : p.accent);
    }
}

// The progress of a button's windows: the first one showing any.
Progress const* ButtonProgress(Button const& b) {
    for (HWND h : b.windows) {
        auto it = g_progress.find(h);
        if (it != g_progress.end() && it->second.state) return &it->second;
    }
    return nullptr;
}

// An app's progress on its button, over the button's face and under its icon: the style's parts 9 to 12
// (normal, busy, error, paused), stretched as far across as it has got; busy sweeps a piece of part 10
// across the face. Without a style, the colour scheme's accent (warning colour for errors, a mix of the
// two when paused). Returns whether it is busy, which needs frames.
bool DrawProgress(HDC dc, RECT const& face, Progress const& pr) {
    bool busy = pr.state == TBPF_INDETERMINATE;
    if (!busy && pr.value <= 0) return false;
    int w = face.right - face.left;
    RECT fill = face;
    if (busy) {
        int piece = std::max(Px(8), w / 2);
        double t = (double)(GetTickCount64() % 1600) / 1600.0;
        fill.left = face.left - piece + (int)((w + piece) * t);
        fill.right = fill.left + piece;
    } else {
        fill.right = face.left + (int)((long long)w * std::min(pr.value, 0xFFFF) / 0xFFFF);
    }
    RECT shown;
    if (!IntersectRect(&shown, &fill, &face)) return busy;
    int part = busy ? kBandProgressBusy
             : pr.state == TBPF_ERROR ? kBandProgressError
             : pr.state == TBPF_PAUSED ? kBandProgressPaused
             : kBandProgressNormal;
    if (g_themed && g_theme.band) {
        int saved = SaveDC(dc);
        IntersectClipRect(dc, shown.left, shown.top, shown.right, shown.bottom);
        DrawThemePartAlpha(dc, g_theme.band, part, 0, fill);
        RestoreDC(dc, saved);
    } else {
        auto const& p = g_settings.palette;
        auto color = pr.state == TBPF_ERROR ? p.warn : pr.state == TBPF_PAUSED ? MixRgb(p.accent, p.warn, 0.5) : p.accent;
        Fill(dc, shown, MixRgb(p.ground, color, 0.45));
    }
    return busy;
}

// No orb image: the Tourne window mark in the accent colour.
void DrawMark(HDC dc, RECT const& r, int state) {
    auto const& p = g_settings.palette;
    int s = std::min(r.right - r.left, r.bottom - r.top);
    int pane = std::max(2, (s - Px(8)) / 2 - g_scale);
    int total = pane * 2 + g_scale;
    int x = r.left + (r.right - r.left - total) / 2, y = r.top + (r.bottom - r.top - total) / 2;
    uint32_t c = state == 2 ? p.raised : state == 1 ? p.text : p.accent;
    for (int i = 0; i < 2; i++)
        for (int j = 0; j < 2; j++) {
            RECT q{x + i * (pane + g_scale), y + j * (pane + g_scale), 0, 0};
            q.right = q.left + pane;
            q.bottom = q.top + pane;
            Fill(dc, q, c);
        }
}

void DrawOrb(HDC dc) {
    int orbState = g_pressed == -1 && g_hot == -1 ? 2 : g_hot == -1 ? 1 : 0;
    if (g_orb.bitmap) g_orb.Draw(dc, g_orbRect, orbState);
    else DrawMark(dc, g_orbRect, orbState);
}

// Inside an island's rounded rectangle.
bool InRounded(RECT const& r, int x, int y, int radius) {
    if (x < r.left || x >= r.right || y < r.top || y >= r.bottom) return false;
    int dx = std::max({0, (int)r.left + radius - x, x - ((int)r.right - 1 - radius)});
    int dy = std::max({0, (int)r.top + radius - y, y - ((int)r.bottom - 1 - radius)});
    return dx * dx + dy * dy <= radius * radius;
}

struct Surface {
    HDC dc = nullptr;
    HBITMAP bitmap = nullptr;
    HGDIOBJ old = nullptr;
    uint32_t* bits = nullptr;
    int w = 0, h = 0;
    Surface(HDC ref, int width, int height) : w(width), h(height) {
        dc = CreateCompatibleDC(ref);
        BITMAPINFO bi{};
        bi.bmiHeader = {sizeof(BITMAPINFOHEADER), w, -h, 1, 32, BI_RGB};
        bitmap = CreateDIBSection(ref, &bi, DIB_RGB_COLORS, (void**)&bits, nullptr, 0);
        old = SelectObject(dc, bitmap);
    }
    ~Surface() {
        SelectObject(dc, old);
        DeleteObject(bitmap);
        DeleteDC(dc);
    }
};

// The clock's lines, centred across the bar, in the Clock part's fonts and
// colours (the theme's button text colour, or the scheme's, when it sets none).
// Whether the overflow flyout the arrow opened is still up.
bool ArrowOpen() {
    if (!g_arrowClicked) return false;
    HWND flyout = FindWindowW(L"TourneTrayFlyout_" WH_MOD_ID, nullptr);
    if (!flyout || !IsWindowVisible(flyout)) g_arrowClicked = false;
    return g_arrowClicked;
}

// The style's show-hidden-icons arrow for the bar's direction, its "Open" one while the flyout is up; null
// when the style has none, or System Icons' own arrow icon is wanted (it shows then).
HTHEME TrayArrowTheme() {
    if (!g_themed || !g_settings.trayArrowStyle) return nullptr;
    int i = (Vertical() ? 2 : 0) + (ArrowOpen() ? 1 : 0);
    return g_theme.trayArrow[i] ? g_theme.trayArrow[i] : g_theme.trayArrow[i & 2];
}

void DrawClock(HDC dc, RECT const& r) {
    SYSTEMTIME now;
    GetLocalTime(&now);
    COLORREF base = PaletteRef(g_settings.palette.text);
    // The style's clock colour, as StartAllBack uses it; else its buttons' text colour.
    if (g_themed && g_theme.clock && SUCCEEDED(GetThemeColor(g_theme.clock, 1, 1, TMT_TEXTCOLOR, &base))) {
    } else if (g_themed && g_theme.band) {
        GetThemeColor(g_theme.band, kBandButton, kStateNormal, TMT_TEXTCOLOR, &base);
    }
    COLORREF primary = g_clock.hasPrimary ? g_clock.primary : base;
    COLORREF secondary = g_clock.hasSecondary ? g_clock.secondary : primary;
    std::vector<std::wstring> lines;
    std::vector<int> heights;
    int total = 0;
    for (size_t i = 0; i < g_clock.lines.size(); i++) {
        lines.push_back(ExpandClockLine(g_clock.lines[i], now));
        HGDIOBJ old = SelectObject(dc, i ? g_clock.secondaryFont : g_clock.primaryFont);
        TEXTMETRICW tm{};
        GetTextMetricsW(dc, &tm);
        SelectObject(dc, old);
        heights.push_back(tm.tmHeight);
        total += tm.tmHeight + (i ? Px(g_clock.lineSpacing) : 0);
    }
    int pad = Px(g_clock.padding + 4);  // clear of the island's edge
    int y = r.top + (r.bottom - r.top - total) / 2;
    UINT align = g_clock.align == 1 ? DT_LEFT : g_clock.align == 2 ? DT_RIGHT : DT_CENTER;
    for (size_t i = 0; i < lines.size(); i++) {
        if (i) y += Px(g_clock.lineSpacing);
        RECT line{r.left + pad, y, r.right - pad, y + heights[i]};
        HGDIOBJ old = SelectObject(dc, i ? g_clock.secondaryFont : g_clock.primaryFont);
        SetTextColor(dc, i ? secondary : primary);
        DrawTextW(dc, lines[i].c_str(), -1, &line, DT_SINGLELINE | DT_NOPREFIX | DT_VCENTER | DT_NOCLIP | align);
        SelectObject(dc, old);
        y += heights[i];
    }
}

// The plugins' glyph font (Tabler icons, which Tourne Desktop loads for this process), at this bar's scale.
HFONT PluginGlyphFont() {
    static HFONT font = nullptr;
    static int height = 0;
    int h = Px(16);
    if (font && height == h) return font;
    if (font) DeleteObject(font);
    height = h;
    font = CreateFontW(-h, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                       ANTIALIASED_QUALITY, DEFAULT_PITCH, kPluginGlyphFont);
    return font;
}

// A plugin item's picture over the bar's (opaque) pixels, at `left` (centred when `left` is -1), middle down.
void BlendPluginPicture(uint32_t* bits, int w, int h, PluginPicture const& p, RECT const& r, int left) {
    if (!bits || !p.w) return;
    GdiFlush();
    int x0 = left >= 0 ? left : (r.left + r.right - p.w) / 2, y0 = (r.top + r.bottom - p.h) / 2;
    for (int y = 0; y < p.h; y++) {
        int ty = y0 + y;
        if (ty < 0 || ty >= h || ty < r.top || ty >= r.bottom) continue;
        for (int x = 0; x < p.w; x++) {
            int tx = x0 + x;
            if (tx < 0 || tx >= w || tx < r.left || tx >= r.right) continue;
            uint32_t src = p.pixels[(size_t)y * p.w + x], a = src >> 24;
            if (!a) continue;
            uint32_t& dst = bits[(size_t)ty * w + tx];
            uint32_t out = 0;
            for (int shift = 0; shift < 24; shift += 8) {
                uint32_t sc = (src >> shift) & 0xFF, dc = (dst >> shift) & 0xFF;
                out |= std::min<uint32_t>(255, sc + dc * (255 - a) / 255) << shift;
            }
            dst = out | (dst & 0xFF000000);
        }
    }
}

// A plugin item: its glyph (or picture), then its text, in the clock's colour unless the plugin chose one.
void DrawPluginItem(HDC dc, PluginBarItem const& item, RECT const& r, PluginPicture const* picture = nullptr, uint32_t* bits = nullptr,
                    int bitsW = 0, int bitsH = 0) {
    COLORREF color = PaletteRef(g_settings.palette.text);
    if (g_themed && g_theme.clock && SUCCEEDED(GetThemeColor(g_theme.clock, 1, 1, TMT_TEXTCOLOR, &color))) {
    } else if (g_themed && g_theme.band) {
        GetThemeColor(g_theme.band, kBandButton, kStateNormal, TMT_TEXTCOLOR, &color);
    }
    if (item.color != CLR_INVALID) color = item.color;
    COLORREF glyphColor = item.glyphColor != CLR_INVALID ? item.glyphColor : color;
    bool hasPicture = picture && picture->w;
    RECT at = r;
    if (Vertical() || item.text.empty()) {
        if (hasPicture) return BlendPluginPicture(bits, bitsW, bitsH, *picture, r, -1);
        SetTextColor(dc, glyphColor);
        HGDIOBJ old = SelectObject(dc, PluginGlyphFont());
        DrawTextW(dc, item.glyph.c_str(), (int)item.glyph.size(), &at, DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOPREFIX | DT_NOCLIP);
        SelectObject(dc, old);
        return;
    }
    at.left += Px(6), at.right -= Px(6);
    if (hasPicture) {
        SetTextColor(dc, color);
        HGDIOBJ old = SelectObject(dc, g_font);
        DrawTextW(dc, item.text.c_str(), (int)item.text.size(), &at, DT_SINGLELINE | DT_LEFT | DT_VCENTER | DT_NOPREFIX | DT_CALCRECT);
        SelectObject(dc, old);
        at = r;
        at.left += Px(6), at.right -= Px(6);
        BlendPluginPicture(bits, bitsW, bitsH, *picture, r, at.left);
        at.left += picture->w + Px(6);
    } else if (!item.glyph.empty()) {
        SetTextColor(dc, glyphColor);
        HGDIOBJ old = SelectObject(dc, PluginGlyphFont());
        SIZE s{};
        GetTextExtentPoint32W(dc, item.glyph.c_str(), (int)item.glyph.size(), &s);
        RECT g{at.left, at.top, at.left + s.cx, at.bottom};
        DrawTextW(dc, item.glyph.c_str(), (int)item.glyph.size(), &g, DT_SINGLELINE | DT_LEFT | DT_VCENTER | DT_NOPREFIX | DT_NOCLIP);
        SelectObject(dc, old);
        at.left += s.cx + Px(6);
    }
    SetTextColor(dc, color);
    HGDIOBJ old = SelectObject(dc, g_font);
    DrawTextW(dc, item.text.c_str(), (int)item.text.size(), &at, DT_SINGLELINE | DT_LEFT | DT_VCENTER | DT_NOPREFIX | DT_END_ELLIPSIS);
    SelectObject(dc, old);
}

// A tray icon's premultiplied pixels over the (opaque) picture, centred in
// its cell.
void BlendIcon(uint32_t* bits, int w, int h, HiddenIcon const& icon, RECT const& cell) {
    int size = icon.size;
    if (!size || icon.pixels.size() < (size_t)size * size) return;
    int x0 = (cell.left + cell.right - size) / 2, y0 = (cell.top + cell.bottom - size) / 2;
    for (int y = 0; y < size; y++) {
        int ty = y0 + y;
        if (ty < 0 || ty >= h) continue;
        for (int x = 0; x < size; x++) {
            int tx = x0 + x;
            if (tx < 0 || tx >= w) continue;
            uint32_t src = icon.pixels[y * size + x];
            uint32_t a = src >> 24;
            if (!a) continue;
            uint32_t& dst = bits[ty * w + tx];
            uint32_t out = 0;
            for (int shift = 0; shift < 24; shift += 8) {
                uint32_t s = (src >> shift) & 0xFF, d = (dst >> shift) & 0xFF;
                out |= std::min<uint32_t>(255, s + d * (255 - a) / 255) << shift;
            }
            dst = out | (dst & 0xFF000000);
        }
    }
}

// The main colour of an icon: its pixels averaged, the more saturated and
// opaque counting for more; the fallback if it's grey or empty.
std::unordered_map<HICON, uint32_t> g_iconColors;

uint32_t IconColor(HICON icon, uint32_t fallback) {
    auto found = g_iconColors.find(icon);
    if (found != g_iconColors.end()) return found->second;
    uint32_t result = fallback;
    ICONINFO info{};
    if (GetIconInfo(icon, &info)) {
        BITMAP bm{};
        if (info.hbmColor && GetObjectW(info.hbmColor, sizeof(bm), &bm) && bm.bmWidth > 0 && bm.bmHeight > 0) {
            std::vector<uint32_t> px((size_t)bm.bmWidth * bm.bmHeight);
            BITMAPINFO bi{};
            bi.bmiHeader = {sizeof(BITMAPINFOHEADER), bm.bmWidth, -bm.bmHeight, 1, 32, BI_RGB};
            HDC dc = GetDC(nullptr);
            if (GetDIBits(dc, info.hbmColor, 0, bm.bmHeight, px.data(), &bi, DIB_RGB_COLORS)) {
                bool hasAlpha = false;
                for (uint32_t v : px) if (v >> 24) { hasAlpha = true; break; }
                double r = 0, g = 0, b = 0, total = 0;
                for (uint32_t v : px) {
                    double a = hasAlpha ? (v >> 24) : 255;
                    if (a < 64) continue;
                    int cr = (v >> 16) & 0xFF, cg = (v >> 8) & 0xFF, cb = v & 0xFF;
                    int hi = std::max({cr, cg, cb}), lo = std::min({cr, cg, cb});
                    double weight = a * (hi - lo + 8.0);
                    r += cr * weight; g += cg * weight; b += cb * weight; total += weight;
                }
                if (total > 0) {
                    int hi = std::max({(int)(r / total), (int)(g / total), (int)(b / total)});
                    int lo = std::min({(int)(r / total), (int)(g / total), (int)(b / total)});
                    // Grey icons keep the scheme's colour.
                    if (hi - lo > 24) result = 0xFF000000 | ((uint32_t)(r / total) << 16) | ((uint32_t)(g / total) << 8) | (uint32_t)(b / total);
                }
            }
            ReleaseDC(nullptr, dc);
        }
        if (info.hbmColor) DeleteObject(info.hbmColor);
        if (info.hbmMask) DeleteObject(info.hbmMask);
    }
    g_iconColors[icon] = result;
    return result;
}

// A soft round glow over a button, centred at a point, fading to nothing at
// its edge; only inside the button.
void DrawAura(uint32_t* bits, int w, int h, RECT const& cell, POINT at, uint32_t color, double strength,
              double reach = 1) {
    double radius = std::max(1.0, std::max(cell.right - cell.left, cell.bottom - cell.top) * reach);
    int cr = (color >> 16) & 0xFF, cg = (color >> 8) & 0xFF, cb = color & 0xFF;
    for (int y = std::max<int>(0, cell.top); y < std::min<int>(h, cell.bottom); y++) {
        for (int x = std::max<int>(0, cell.left); x < std::min<int>(w, cell.right); x++) {
            double dx = x - at.x, dy = y - at.y;
            double d = std::sqrt(dx * dx + dy * dy) / radius;
            if (d >= 1) continue;
            double k = strength * (1 - d);
            uint32_t& px = bits[(size_t)y * w + x];
            int pr = (px >> 16) & 0xFF, pg = (px >> 8) & 0xFF, pb = px & 0xFF;
            pr += (int)((cr - pr) * k); pg += (int)((cg - pg) * k); pb += (int)((cb - pb) * k);
            px = (px & 0xFF000000) | (pr << 16) | (pg << 8) | pb;
        }
    }
}
// StartAllBack's own bar look, measured from a screenshot of its Default style
// (Snipping Tool, 38 px bar at the bottom): the row against the screen's inner
// side is a darker edge, the next a light hairline, then dark acrylic. (My own
// BitBlt and DXGI captures show this acrylic as a copper gradient; the
// screenshots the user takes, and the display, show it dark, so those rule.)
uint32_t SabBackdropAt(int depth, int thickness) {
    (void)thickness;
    if (depth <= 0) return 0x101618;
    if (depth == 1) return 0x31373A;
    return 0x141E23;
}
void FillSabBackdrop(uint32_t* bits, int w, int h) {
    int thickness = Vertical() ? w : h;
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int depth = g_settings.edge == Edge::Bottom ? y : g_settings.edge == Edge::Top ? h - 1 - y
                      : g_settings.edge == Edge::Right ? x : w - 1 - x;
            bits[(size_t)y * w + x] = SabBackdropAt(depth, thickness) | 0xFF000000;
        }
    }
}

// While the bar is away with the glow set: a soft line of light in the accent colour along the
// screen's edge, a bright core that blooms a little towards white and a softer halo around it,
// fading into the screen and towards both ends. Drawn once as the bar goes; the compositor fades it.
void PaintEdgeHint(HWND wnd) {
    RECT rc;
    GetClientRect(wnd, &rc);
    int w = rc.right, h = rc.bottom;
    if (w <= 0 || h <= 0) return;
    HDC screen = GetDC(nullptr);
    Surface out(screen, w, h);
    std::fill(out.bits, out.bits + (size_t)w * h, 0u);
    bool vertical = Vertical();
    int depth = std::min(Px(g_settings.shyGlowSize), vertical ? w : h);
    int length = vertical ? h : w;
    uint32_t c = g_settings.palette.accent;
    double strength = g_settings.shyGlow / 100.0;
    for (int d = 0; d < depth; d++) {
        double f = 1 - (d + 0.5) / depth;  // 1 at the screen's edge
        double core = f * f * f;
        double level = std::min(1.0, core + 0.35 * f) * strength;
        double white = 0.4 * core * strength;
        // The row (or column) d px in from the screen's edge; the window sits with only these in view.
        int line = g_settings.edge == Edge::Bottom ? depth - 1 - d : g_settings.edge == Edge::Top ? h - depth + d
                 : g_settings.edge == Edge::Left ? w - depth + d : depth - 1 - d;
        for (int u = 0; u < length; u++) {
            double e = std::min(u, length - 1 - u) / std::max(1.0, 0.1 * length);  // the last tenth at each end fades
            double a = level * (e >= 1 ? 1 : e * e * (3 - 2 * e));
            if (a < 0.004) continue;
            auto channel = [&](int shift) {
                double v = ((c >> shift) & 0xFF) * (1 - white) + 255 * white;
                return (uint32_t)std::lround(v * a);
            };
            uint32_t px = ((uint32_t)std::lround(255 * a) << 24) | (channel(16) << 16) | (channel(8) << 8) | channel(0);
            out.bits[(size_t)(vertical ? u : line) * w + (vertical ? line : u)] = px;
        }
    }
    GdiFlush();
    POINT src{0, 0};
    SIZE size{w, h};
    BLENDFUNCTION blend{AC_SRC_OVER, 0, g_barAlpha, AC_SRC_ALPHA};
    UpdateLayeredWindow(wnd, screen, nullptr, &size, out.dc, &src, 0, &blend, ULW_ALPHA);
    ReleaseDC(nullptr, screen);
}

// @inline css-data.cpp
// @inline css-bar.cpp
// @inline css-popup.cpp

// The taskbar's own settings, narrowed to what's asked for (a setting's name or part of its key, like
// "startmenu."): in Tourne Desktop, Tourne Studio with its search set to that; in the Windhawk suite, which
// runs inside windhawk.exe, Windhawk's window (starting it again opens it). Settings menus' items come here.
void OpenOwnSettings(std::wstring const& query = L"") {
#ifdef TD_PREFIX
    td::OpenStudio(std::wstring(TD_PREFIX) + L".", query);
#else
    (void)query;
    WCHAR self[MAX_PATH];
    GetModuleFileNameW(nullptr, self, ARRAYSIZE(self));
    ShellExecuteW(nullptr, nullptr, self, nullptr, nullptr, SW_SHOWNORMAL);
#endif
}

// @inline desktops-pager.cpp

void Redraw(HWND wnd) {
    g_barsDrawn = 0;  // (the once-a-second refresh sets it again after drawing)
    if (g_edgeHint) {
        PaintEdgeHint(wnd);
        return;
    }
    if (CssActive()) {
        CssRedraw(wnd);
        return;
    }
    RECT rc;
    GetClientRect(wnd, &rc);
    if (rc.right <= 0 || rc.bottom <= 0) return;
    HDC screen = GetDC(nullptr);
    Surface content(screen, rc.right, rc.bottom);
    auto const& p = g_settings.palette;
    bool segmented = EffSegments() != Segments::Off;
    bool floatingOrb = EffSegments() == Segments::Central;
    std::vector<RECT> through = segmented ? BarPassThrough(wnd) : std::vector<RECT>{};

    // The opaque picture: background, buttons, labels, orb.
    Fill(content.dc, rc, p.ground);
    if (!segmented) {
        if (g_settings.sabBackdrop) {
            GdiFlush();
            FillSabBackdrop(content.bits, rc.right, rc.bottom);
        }
        DrawBackground(content.dc, rc, false);
    } else {
        for (auto const& island : g_islands) DrawBackground(content.dc, island, true);
    }
    // Custom coloring replaces whatever is still the background at the end. The background is marked
    // with an alpha of 1 (the bar ends up opaque either way), so anything drawn over it changes its alpha
    // as well: a frame drawn in exactly the background's colour (Bouquet's dark border is) isn't taken
    // for background and lost to the tint.
    std::vector<uint32_t> backdrop;
    if (g_settings.coloring || BackdropClear()) {
        GdiFlush();
        size_t count = (size_t)rc.right * rc.bottom;
        backdrop.assign(content.bits, content.bits + count);
        for (size_t i = 0; i < count; i++) content.bits[i] = (content.bits[i] & 0xFFFFFF) | 0x01000000;
    }
    // Shadows falling on that background, applied over the scheme's colour at the end.
    std::vector<uint8_t> shade;
    if (!backdrop.empty() && g_settings.shadows) shade.assign((size_t)rc.right * rc.bottom, 0);
    std::vector<uint8_t>* shadeOut = shade.empty() ? nullptr : &shade;
    int icon = Px(g_settings.iconSize);
    HGDIOBJ oldFont = SelectObject(content.dc, g_font);
    SetBkMode(content.dc, TRANSPARENT);
    bool progressBusy = false;
    for (int i = 0; i < (int)g_buttons.size(); i++) {
        Button const& b = g_buttons[i];
        RECT face = b.rc;  // without the room the stack's edge takes at its end
        if (b.stack) (Vertical() ? face.bottom : face.right) -= b.stack;
        int state = ButtonState(i);
        // A pinned app that isn't running is just its icon, until pointed at.
        bool running = !b.windows.empty();
        RECT progressRect = face;  // where the button's frame is drawn
        if (g_settings.sabBackdrop && !g_themed) {
            // StartAllBack's Default style (measured from its screenshot): the
            // active window's button is a grey box 30 px high, 1 px in from the
            // button's sides, edged darker; a running app has a small pill under
            // its icon (3 px high, 6 wide, 4 px from the bar's outer edge): light
            // grey, teal for the active one; pointing at a button lights it.
            bool activeBox = state >= kStateActive && state <= kStateActivePressed;
            {
                int across = Vertical() ? rc.right : rc.bottom;
                int lo = 2 * g_scale + (across - 2 * g_scale - Px(30)) / 2, hi = lo + Px(30);
                progressRect = Vertical() ? RECT{lo, face.top + 1, hi, face.bottom} : RECT{face.left + 1, lo, face.right, hi};
            }
            if (activeBox) {
                RECT box = progressRect;
                SetDCBrushColor(content.dc, RGB(0x44, 0x4C, 0x4F));
                FillRect(content.dc, &box, (HBRUSH)GetStockObject(DC_BRUSH));
                SetDCBrushColor(content.dc, RGB(0x31, 0x3A, 0x3D));
                FrameRect(content.dc, &box, (HBRUSH)GetStockObject(DC_BRUSH));
            }
            if (state == kStateHot || state == kStateActiveHot || state == kStatePressed || state == kStateActivePressed) {
                GdiFlush();
                int lift = state == kStatePressed || state == kStateActivePressed ? 10 : 22;
                for (int y = std::max<int>(0, face.top); y < std::min<int>(rc.bottom, face.bottom); y++)
                    for (int x = std::max<int>(0, face.left); x < std::min<int>(rc.right, face.right); x++) {
                        uint32_t& px = content.bits[(size_t)y * rc.right + x];
                        uint32_t out = px & 0xFF000000;
                        for (int shift = 0; shift < 24; shift += 8) {
                            int c = (int)((px >> shift) & 0xFF);
                            out |= (uint32_t)std::min(255, c + lift) << shift;
                        }
                        px = out;
                    }
            }
            if (running) {
                RECT pill;
                int mid = Vertical() ? (face.top + face.bottom) / 2 : (face.left + face.right) / 2;
                int w = Px(6), h = Px(3), edge = Px(4);
                if (Vertical()) {
                    int x0 = g_settings.edge == Edge::Left ? edge : rc.right - edge - h;
                    pill = {x0, mid - w / 2, x0 + h, mid - w / 2 + w};
                } else {
                    int y0 = g_settings.edge == Edge::Bottom ? rc.bottom - edge - h : edge;
                    pill = {mid - w / 2, y0, mid - w / 2 + w, y0 + h};
                }
                SetDCBrushColor(content.dc, activeBox ? RGB(0x44, 0x70, 0x6C) : RGB(0x91, 0x93, 0x95));
                FillRect(content.dc, &pill, (HBRUSH)GetStockObject(DC_BRUSH));
            }
        } else if (running || state != kStateNormal) {
            if (g_themed && g_theme.band) {
                // StartAllBack draws a button's frame 4 px in from both sides of the bar: 30 px on a
                // 38 px bar, its dark outer rows included (measured with Bouquet SAB, whose dark rows are
                // the bar's own colour there); segments already are 28.
                RECT fr = face;
                if (!segmented && g_settings.sabBackdrop) {
                    int across = Vertical() ? rc.right : rc.bottom, inner = Px(4), len = std::max(Px(16), across - 2 * inner);
                    bool innerFirst = g_settings.edge == Edge::Bottom || g_settings.edge == Edge::Right;
                    int start = innerFirst ? inner : across - inner - len;
                    if (Vertical()) { fr.left = start; fr.right = start + len; }
                    else { fr.top = start; fr.bottom = start + len; }
                }
                // A group of windows is a stack, drawn with the style's own stack parts (13 for a
                // button, 14 for the active one; state 2 for two windows, 3 for more): their edge
                // showing behind the button, where the room at its end is.
                if (g_settings.shadows & 2) {
                    GdiFlush();
                    FrameShadow(content.bits, rc.right, rc.bottom, shadeOut, fr);
                }
                if (b.windows.size() > 1) {
                    bool activeGroup = state >= kStateActive && state <= kStateActivePressed;
                    GdiFlush();
                    DrawStackEdge(content.bits, rc.right, rc.bottom, activeGroup ? kBandStackActive : kBandStack,
                                  b.windows.size() > 2 ? 3 : 2, fr);
                }
                DrawThemePartAlpha(content.dc, g_theme.band, kBandButton, state, fr);
                progressRect = fr;
            }
            else DrawFlatButton(content.dc, face, state);
        }
        if (running && !g_progress.empty())
            if (Progress const* pr = ButtonProgress(b)) progressBusy |= DrawProgress(content.dc, progressRect, *pr);
        Win* w = running ? FindWin(b.windows[0]) : nullptr;
        int bw = face.right - face.left;
        bool label = running && g_labels && bw >= icon + Px(40);
        int ix = label ? face.left + Px(g_settings.margin) : (face.left + face.right - icon) / 2;
        int iy = (face.top + face.bottom - icon) / 2;
        bool combined = g_settings.combine != Combine::NeverGroup && g_settings.combine != Combine::Never &&
                        g_settings.combine != Combine::NeverHideLabels;
        // A pinned app's button shows its pin's icon, as the pin shows it.
        HICON drawn = b.pin >= 0 && (!running || combined) ? PinIcon(b.pin) : nullptr;
        if (!drawn && w) drawn = ButtonIcon(*w, combined);
        // Aura: the button glows in its icon's main colour, strongest under the pointer.
        // StartAllBack lights a pointed-at button this way whatever the plumpness (fading in and
        // out); the running-app glow and the pointer-following one (dynamic aura) are the Aura
        // setting's.
        double glow = GlowLevel(i);
        if (drawn && ((AuraOn() && (running || g_hot == i)) || (g_settings.sabBackdrop && glow > 0))) {
            double strength = glow > 0 ? glow : g_hot == i ? 0.0 : g_settings.sabBackdrop ? 0.0 : state >= 4 ? 0.3 : 0.12;
            POINT at{(face.left + face.right) / 2, (face.top + face.bottom) / 2};
            // StartAllBack keeps the glow inside the button's frame (28 px across the bar), its
            // centre a little towards the screen's edge (measured: 0.4 at the sides, 0.2 at the top).
            RECT cell = face;
            if (!segmented && g_settings.sabBackdrop) {
                int across = Vertical() ? rc.right : rc.bottom, len = Px(28), inner = Px(4);
                bool innerFirst = g_settings.edge == Edge::Bottom || g_settings.edge == Edge::Right;
                int start = innerFirst ? inner : across - inner - len;
                if (Vertical()) { cell.left = start; cell.right = start + len; }
                else { cell.top = start; cell.bottom = start + len; }
                // Towards the screen's edge; a pixel less on the bottom and right, so it mirrors the top and left
                // (a glow centred on a pixel index is one pixel off when the bar is flipped).
                (Vertical() ? at.x : at.y) += innerFirst ? Px(6) - 1 : -Px(6);
            }
            if (g_settings.dynamicAura && g_hot == i) {
                if (g_aura.placed) {
                    at = {(LONG)std::lround(g_aura.x), (LONG)std::lround(g_aura.y)};
                } else {
                    GetCursorPos(&at);
                    ScreenToClient(wnd, &at);
                }
            }
            // The Aura look's own size and strength (StartAllBack's hover glow stays as it is).
            double reach = 1;
            if (AuraOn()) {
                strength = std::min(1.0, strength * g_settings.auraStrength / 100.0);
                reach = g_settings.auraSize / 100.0;
            }
            GdiFlush();
            DrawAura(content.bits, rc.right, rc.bottom, cell, at, IconColor(drawn, p.accent), strength, reach);
        }
        if (drawn && (g_settings.shadows & 1)) {
            GdiFlush();
            IconShadow(content.bits, rc.right, rc.bottom, shadeOut, drawn, icon, ix, iy);
        }
        if (drawn) DrawIconEx(content.dc, ix, iy, drawn, icon, icon, 0, nullptr, DI_NORMAL);
        if (label) {
            RECT tr{ix + icon + Px(6), face.top, face.right - Px(6), face.bottom};
            std::wstring text = Title(b.windows[0]);
            if (b.windows.size() > 1) text += L" (" + std::to_wstring(b.windows.size()) + L")";
            COLORREF color = PaletteRef(p.text);
            if (g_themed && g_theme.band) GetThemeColor(g_theme.band, kBandButton, state, TMT_TEXTCOLOR, &color);
            SetTextColor(content.dc, color);
            DrawTextW(content.dc, text.c_str(), -1, &tr, DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX);
        }
    }
    // The busy sweep's frames, only while one shows.
    if (progressBusy) SetTimer(wnd, kTimerProgress, 40, nullptr);
    else KillTimer(wnd, kTimerProgress);
    // The tray: a hover or press behind an icon or the clock, and the clock.
    auto trayState = [&](int hit, RECT const& r) {
        if (g_hot != hit) return;
        int state = g_pressed == hit ? kStatePressed : kStateHot;
        // The style's own hover behind tray icons and the clock, as StartAllBack draws them.
        HTHEME own = !g_themed ? nullptr : hit == kHitClock ? g_theme.clock : g_theme.trayButton;
        if (own) DrawThemePartAlpha(content.dc, own, 1, state, r);
        else if (g_themed && g_theme.band) DrawThemeBackground(g_theme.band, content.dc, kBandButton, state, &r, nullptr);
        else DrawFlatButton(content.dc, r, state);
    };
    // The style's show-hidden-icons arrow in place of System Icons' arrow icon.
    HTHEME arrow = TrayArrowTheme();
    g_arrowDrawnOpen = g_arrowClicked;
    for (int i = 0; i < (int)g_tray.size(); i++) {
        int hit = kHitTray + i;
        if (arrow && IsOverflowArrow(g_tray[i])) {
            DrawThemePartAlpha(content.dc, arrow, 1, g_hot != hit ? 1 : g_pressed == hit ? 3 : 2, g_tray[i].rc);
            continue;
        }
        trayState(hit, g_tray[i].rc);
    }
    for (int i = 0; i < (int)g_pluginRects.size() && i < (int)g_pluginItems.size(); i++) {
        if (IsRectEmpty(&g_pluginRects[i])) continue;
        trayState(kHitPlugin + i, g_pluginRects[i]);
        DrawPluginItem(content.dc, g_pluginItems[i], g_pluginRects[i], i < (int)g_pluginPictures.size() ? &g_pluginPictures[i] : nullptr,
                       content.bits, rc.right, rc.bottom);
    }
    if (!IsRectEmpty(&g_clockRect)) {
        trayState(kHitClock, g_clockRect);
        DrawClock(content.dc, g_clockRect);
    }
    SelectObject(content.dc, g_font);  // (the clock may have left its own)
    DrawDockedPager(content.dc, content.bits, rc.right, rc.bottom);
    // The show desktop button: the style's own, lit when pointed at; without a style, a line where it starts.
    if (!IsRectEmpty(&g_showDesktopRect)) {
        int state = g_hot != kHitShowDesktop ? 1 : g_pressed == kHitShowDesktop ? 3 : 2;
        HTHEME sd = !g_themed ? nullptr : g_theme.showDesktop[Vertical() ? 1 : 0] ? g_theme.showDesktop[Vertical() ? 1 : 0]
                                                                                : g_theme.showDesktop[0];
        if (sd) {
            DrawThemePartAlpha(content.dc, sd, 1, state, g_showDesktopRect);
        } else {
            if (state > 1) Fill(content.dc, g_showDesktopRect, state == 3 ? p.raised : p.hover);
            RECT line = g_showDesktopRect;
            if (Vertical()) line.bottom = line.top + g_scale;
            else line.right = line.left + g_scale;
            Fill(content.dc, line, p.line);
        }
    }
    bool mainBar = IsMainBar();
    if (!floatingOrb && mainBar) DrawOrb(content.dc);
    // So the test bar can be told apart from the real one.
    if (mainBar && !segmented && !Vertical() && g_tray.empty() && IsRectEmpty(&g_clockRect)) {
        SetTextColor(content.dc, PaletteRef(p.dim));
        RECT label{rc.right - Px(160), 0, rc.right - Px(8), rc.bottom};
        int buttonsEnd = g_buttons.empty() ? 0 : g_buttons.back().rc.right;
        int orbEnd = g_orbRect.right;
        if (label.left > std::max(buttonsEnd, orbEnd))
            DrawTextW(content.dc, L"// tourne taskbar test", -1, &label, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
    }
    SelectObject(content.dc, oldFont);
    GdiFlush();
    for (auto const& item : g_tray)
        if (!arrow || !IsOverflowArrow(item)) BlendIcon(content.bits, rc.right, rc.bottom, item.icon, item.rc);
    // Where an icon dragged out of the overflow flyout would land: a line in
    // the accent colour between two icons.
    if (g_trayDropAt >= 0 && (!g_tray.empty() || !IsRectEmpty(&g_clockRect))) {
        bool vertical = Vertical();
        bool after = g_trayDropAt >= (int)g_tray.size() && !g_tray.empty();
        RECT const& r = g_tray.empty() ? g_clockRect : g_tray[std::min(g_trayDropAt, (int)g_tray.size() - 1)].rc;
        int edge = vertical ? (after ? r.bottom : r.top) : (after ? r.right : r.left);
        RECT m = vertical ? RECT{r.left + Px(2), edge - Px(1), r.right - Px(2), edge + Px(1)}
                          : RECT{edge - Px(1), r.top + Px(2), edge + Px(1), r.bottom - Px(2)};
        for (int y = std::max(0, (int)m.top); y < std::min((int)rc.bottom, (int)m.bottom); y++)
            for (int x = std::max(0, (int)m.left); x < std::min((int)rc.right, (int)m.right); x++)
                content.bits[y * rc.right + x] = p.accent | 0xFF000000;
    }

    // The layered picture, premultiplied: opaque where the bar or an island
    // is, clear elsewhere; a floating orb keeps its own transparency.
    Surface out(screen, rc.right, rc.bottom);
    // A theme's islands are its own background images, square like
    // StartAllBack's; flat ones are rounded.
    int radius = g_themed && g_theme.bar ? 0 : Px(5);
    for (int y = 0; y < rc.bottom; y++) {
        for (int x = 0; x < rc.right; x++) {
            bool solid = !segmented;
            for (size_t i = 0; segmented && !solid && i < g_islands.size(); i++)
                solid = InRounded(g_islands[i], x, y, radius);
            size_t at = (size_t)y * rc.right + x;
            bool framed = !segmented && g_barFrame && (x < g_barFrame || y < g_barFrame || x >= rc.right - g_barFrame || y >= rc.bottom - g_barFrame);
            if (solid && !framed && !backdrop.empty() && (content.bits[at] >> 24) == 1 &&
                ((content.bits[at] ^ backdrop[at]) & 0xFFFFFF) == 0) {
                // Background: the tint at its opacity (premultiplied), or clear (dynamic transparency, no
                // tint); then any shadow falling on it.
                uint32_t a = BackdropClear() ? 0 : (uint32_t)g_settings.tintAlpha;
                uint32_t px = !g_settings.coloring ? 0
                                                   : (a << 24) | ((GetRValue(g_settings.tint) * a / 255) << 16) |
                                                         ((GetGValue(g_settings.tint) * a / 255) << 8) | (GetBValue(g_settings.tint) * a / 255);
                if (!shade.empty() && shade[at]) px = OverShadow(px, shade[at]);
                out.bits[at] = px;
                continue;
            }
            // (between segments: all but see-through, alpha 1, so the bar still takes clicks there as its empty space)
            out.bits[at] = solid ? (content.bits[at] | 0xFF000000) : segmented && !framed && !InPassThrough(through, x, y) ? 0x01000000 : 0;
        }
    }
    if (floatingOrb && mainBar) {
        GdiFlush();
        DrawOrb(out.dc);
    }
    GdiFlush();

    RECT wr;
    GetWindowRect(wnd, &wr);
    POINT dst{wr.left, wr.top}, src{0, 0};
    SIZE size{rc.right, rc.bottom};
    BLENDFUNCTION blend{AC_SRC_OVER, 0, g_barAlpha, AC_SRC_ALPHA};
    UpdateLayeredWindow(wnd, screen, &dst, &size, out.dc, &src, 0, &blend, ULW_ALPHA);
    ReleaseDC(nullptr, screen);
}

// The pointer has come to a button (or left them all): its glow starts to fade in, and the one before it out.
void GlowTo(HWND wnd, int hot) {
    int now = hot >= 0 ? hot : -1;
    if (g_glowIn.button == now) return;
    if (g_glowIn.button >= 0 && g_glowIn.level > 0.01) g_glowOut = g_glowIn;
    g_glowIn = {now, 0};
    g_glowTick = GetTickCount();
    SetTimer(wnd, kTimerGlow, 15, nullptr);
}

void GlowStep(HWND wnd) {
    DWORD now = GetTickCount();
    double ms = (double)(now - g_glowTick);
    g_glowTick = now;
    if (g_glowIn.button >= 0 && g_glowIn.level < 1) g_glowIn.level = std::min(1.0, g_glowIn.level + ms / 90.0);
    if (g_glowOut.button >= 0) {
        g_glowOut.level -= ms / 140.0;
        if (g_glowOut.level <= 0) g_glowOut = {};
    }
    if ((g_glowIn.button < 0 || g_glowIn.level >= 1) && g_glowOut.button < 0) KillTimer(wnd, kTimerGlow);
    Redraw(wnd);
}

int HitTest(POINT pt) {
    if (PtInRect(&g_orbRect, pt)) return -1;
    for (int i = 0; i < (int)g_buttons.size(); i++)
        if (PtInRect(&g_buttons[i].rc, pt)) return i;
    for (int i = 0; i < (int)g_pluginRects.size(); i++)
        if (PtInRect(&g_pluginRects[i], pt)) return kHitPlugin + i;
    for (int i = 0; i < (int)g_tray.size(); i++)
        if (PtInRect(&g_tray[i].rc, pt)) return kHitTray + i;
    if (PtInRect(&g_clockRect, pt)) return kHitClock;
    if (PtInRect(&g_showDesktopRect, pt)) return kHitShowDesktop;
    if (int pager = DockedPagerHit(pt); pager >= 0) return pager;
    for (int i = 0; i < (int)g_cssTargets.size(); i++)
        if (PtInRect(&g_cssTargets[i].rc, pt)) return kHitCss + i;
    return -2;
}

////////////////////////////////////////////////////////////////////////////////
// Actions and menus

void ActivateWindow(HWND h) {
    if (IsIconic(h)) ShowWindowAsync(h, SW_RESTORE);
    if (!SetForegroundWindow(h)) SwitchToThisWindow(h, TRUE);
}

void ClickWindow(HWND h) {
    if (h == ForegroundRoot() && !IsIconic(h)) ShowWindowAsync(h, SW_MINIMIZE);
    else ActivateWindow(h);
}

// The window of a group used last: the first of them in the stacking order, which follows use.
HWND ActiveOfGroup(Button const& b) {
    for (HWND h = GetTopWindow(nullptr); h; h = GetWindow(h, GW_HWNDNEXT))
        if (std::find(b.windows.begin(), b.windows.end(), h) != b.windows.end()) return h;
    return nullptr;
}

void SendKeys(std::initializer_list<WORD> keys) {
    std::vector<INPUT> inputs;
    for (WORD k : keys) {
        INPUT in{INPUT_KEYBOARD};
        in.ki.wVk = k;
        inputs.push_back(in);
    }
    for (auto it = keys.end(); it != keys.begin();) {
        INPUT in{INPUT_KEYBOARD};
        in.ki.wVk = *--it;
        in.ki.dwFlags = KEYEVENTF_KEYUP;
        inputs.push_back(in);
    }
    SendInput((UINT)inputs.size(), inputs.data(), sizeof(INPUT));
}

void Launch(std::wstring const& exe) {
    if (!exe.empty()) ShellExecuteW(nullptr, nullptr, exe.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
}

// Windhawk's own window, where its mods' settings are changed: in the bar's menu, as in shell mode
// there's no other quick way in. Empty when it isn't installed.
std::wstring WindhawkExe() {
    std::wstring path = Expand(L"%ProgramFiles%\\Windhawk\\windhawk.exe");
    return GetFileAttributesW(path.c_str()) == INVALID_FILE_ATTRIBUTES ? L"" : path;
}

// Windows' Run dialog, opened by shell32 itself (RunFileDlg, ordinal 61): Win+R needs Explorer's taskbar
// to answer it, which shell mode hides. On a thread of its own, as the dialog is modal.
void ShowRunDialog() {
    HANDLE thread = CreateThread(
        nullptr, 0,
        [](LPVOID) -> DWORD {
            using RunFileDlg = void(WINAPI*)(HWND, HICON, LPCWSTR, LPCWSTR, LPCWSTR, UINT);
            HMODULE shell = LoadLibraryW(L"shell32.dll");
            auto run = shell ? (RunFileDlg)GetProcAddress(shell, MAKEINTRESOURCEA(61)) : nullptr;
            if (run) {
                bool com = SUCCEEDED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED));
                run(nullptr, nullptr, nullptr, nullptr, nullptr, 0);
                if (com) CoUninitialize();
            }
            if (shell) FreeLibrary(shell);
            return 0;
        },
        nullptr, 0, nullptr);
    if (thread) CloseHandle(thread);
}

// Shows the desktop, or brings back the windows that did it, as Win+D does: that key needs Explorer's
// taskbar to answer it, which shell mode hides. Only this desktop's windows; frontmost first.
std::vector<HWND> g_desktopShown;  // what Show desktop minimized, to bring back
void ToggleShowDesktop() {
    bool back = std::any_of(g_desktopShown.begin(), g_desktopShown.end(), [](HWND h) { return IsWindow(h) && IsIconic(h); });
    if (back) {
        // From the back, so the one that was in front ends in front.
        for (auto it = g_desktopShown.rbegin(); it != g_desktopShown.rend(); ++it)
            if (IsWindow(*it) && IsIconic(*it)) ShowWindowAsync(*it, SW_SHOWNOACTIVATE);
        if (IsWindow(g_desktopShown.front())) SetForegroundWindow(g_desktopShown.front());
        g_desktopShown.clear();
        return;
    }
    g_desktopShown.clear();
    for (HWND h = GetTopWindow(nullptr); h; h = GetWindow(h, GW_HWNDNEXT))
        if (IsTaskWindow(h) && !IsIconic(h)) g_desktopShown.push_back(h);
    for (HWND h : g_desktopShown) ShowWindowAsync(h, SW_MINIMIZE);
}

////////////////////////////////////////////////////////////////////////////////
// Jump lists: the tasks and categories an app gives the taskbar, read the way
// Explorer reads them, through Windows' destination lists (undocumented,
// unchanged since Windows 7).

struct APPDESTCATEGORY {
    int type;  // 0 an app's own category, 1 a known one (frequent, recent), 2 its tasks
    union {
        LPWSTR name;
        int subType;
    };
    int count;
    int pad[10];
};

struct IInternalCustomDestinationList : IUnknown {
    virtual HRESULT STDMETHODCALLTYPE SetMinItems(int) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetApplicationID(LPCWSTR) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetSlotCount(int*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetCategoryCount(int*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetCategory(int, int, APPDESTCATEGORY*) = 0;
    virtual HRESULT STDMETHODCALLTYPE DeleteCategory(int, BOOL) = 0;
    virtual HRESULT STDMETHODCALLTYPE EnumerateCategoryDestinations(int, REFIID, void**) = 0;
    virtual HRESULT STDMETHODCALLTYPE RemoveDestination(IUnknown*) = 0;
};
constexpr CLSID kClsidDestinationList = {0x77f10cf0, 0x3db5, 0x4966, {0xb5, 0x20, 0xb7, 0xc5, 0x4f, 0xd3, 0x5e, 0xd6}};
constexpr IID kIidCustomDestinations = {0x507101cd, 0xf6ad, 0x46c8, {0x8e, 0x20, 0xee, 0xb9, 0xe6, 0xba, 0xc4, 0x7f}};
constexpr PROPERTYKEY kTitleKey = {{0xF29F85E0, 0x4FF9, 0x1068, {0xAB, 0x91, 0x08, 0x00, 0x2B, 0x27, 0xB3, 0xD9}}, 2};
constexpr PROPERTYKEY kSeparatorKey = {{0x9F4C2855, 0x9F79, 0x4B39, {0xA8, 0xD0, 0xE1, 0xD4, 0x2D, 0xE1, 0xD5, 0xF3}}, 6};

struct JumpItem {
    std::wstring title;
    PIDLIST_ABSOLUTE target = nullptr;  // what it opens or runs
    std::wstring args, dir;
    int show = SW_SHOWNORMAL;
    HICON icon = nullptr;
    bool separator = false;
};

struct JumpCategory {
    std::wstring name;
    std::vector<JumpItem> items;
};

// "@dll,-id" names, as apps give them, in the user's language.
std::wstring IndirectString(std::wstring const& s) {
    if (s.empty() || s[0] != L'@') return s;
    WCHAR out[512];
    return SUCCEEDED(SHLoadIndirectString(s.c_str(), out, ARRAYSIZE(out), nullptr)) ? out : s;
}

bool ReadJumpItem(IUnknown* unk, int iconSize, JumpItem* out) {
    IShellLinkW* link = nullptr;
    if (SUCCEEDED(unk->QueryInterface(IID_PPV_ARGS(&link)))) {
        IPropertyStore* store = nullptr;
        if (SUCCEEDED(link->QueryInterface(IID_PPV_ARGS(&store)))) {
            PROPVARIANT pv;
            PropVariantInit(&pv);
            if (SUCCEEDED(store->GetValue(kTitleKey, &pv)) && pv.vt == VT_LPWSTR && pv.pwszVal)
                out->title = IndirectString(pv.pwszVal);
            PropVariantClear(&pv);
            if (SUCCEEDED(store->GetValue(kSeparatorKey, &pv)) && pv.vt == VT_BOOL && pv.boolVal)
                out->separator = true;
            PropVariantClear(&pv);
            store->Release();
        }
        WCHAR buf[MAX_PATH * 2] = L"";
        if (out->title.empty() && SUCCEEDED(link->GetDescription(buf, ARRAYSIZE(buf)))) out->title = IndirectString(buf);
        link->GetIDList(&out->target);
        buf[0] = 0;
        if (SUCCEEDED(link->GetArguments(buf, ARRAYSIZE(buf)))) out->args = buf;
        buf[0] = 0;
        if (SUCCEEDED(link->GetWorkingDirectory(buf, ARRAYSIZE(buf)))) out->dir = buf;
        link->GetShowCmd(&out->show);
        // Only an icon the task names, as the classic taskbar shows them.
        int index = 0;
        buf[0] = 0;
        if (SUCCEEDED(link->GetIconLocation(buf, ARRAYSIZE(buf), &index)) && buf[0])
            out->icon = ExtractAt(Expand(buf), index, iconSize);
        link->Release();
        return out->separator || (!out->title.empty() && out->target);
    }
    IShellItem* item = nullptr;
    if (SUCCEEDED(unk->QueryInterface(IID_PPV_ARGS(&item)))) {
        LPWSTR name = nullptr;
        if (SUCCEEDED(item->GetDisplayName(SIGDN_NORMALDISPLAY, &name)) && name) {
            out->title = name;
            CoTaskMemFree(name);
        }
        SHGetIDListFromObject(item, &out->target);
        if (SUCCEEDED(item->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &name)) && name) {
            out->icon = ShellIcon(name, iconSize);
            CoTaskMemFree(name);
        }
        item->Release();
        return !out->title.empty() && out->target;
    }
    return false;
}

void FreeJumpList(std::vector<JumpCategory>& list) {
    for (auto& category : list) {
        for (auto& item : category.items) {
            if (item.target) CoTaskMemFree(item.target);
            if (item.icon) DestroyIcon(item.icon);
        }
    }
    list.clear();
}

// The app's own categories and tasks, in its order. Recent and frequent
// items aren't read yet.
std::vector<JumpCategory> ReadJumpList(std::wstring const& appId, int iconSize) {
    std::vector<JumpCategory> list;
    IInternalCustomDestinationList* custom = nullptr;
    if (appId.empty() || FAILED(CoCreateInstance(kClsidDestinationList, nullptr, CLSCTX_INPROC_SERVER,
                                                 kIidCustomDestinations, (void**)&custom)))
        return list;
    int count = 0;
    if (SUCCEEDED(custom->SetApplicationID(appId.c_str())) && SUCCEEDED(custom->GetCategoryCount(&count))) {
        for (int c = 0; c < count; c++) {
            APPDESTCATEGORY cat{};
            if (FAILED(custom->GetCategory(c, 1, &cat))) continue;
            JumpCategory category;
            if (cat.type == 0 && cat.name) {
                category.name = IndirectString(cat.name);
                CoTaskMemFree(cat.name);
            } else if (cat.type == 2) {
                category.name = L"Tasks";
            } else {
                continue;
            }
            IObjectArray* items = nullptr;
            if (SUCCEEDED(custom->EnumerateCategoryDestinations(c, IID_PPV_ARGS(&items)))) {
                UINT n = 0;
                items->GetCount(&n);
                for (UINT i = 0; i < n; i++) {
                    IUnknown* unk = nullptr;
                    if (FAILED(items->GetAt(i, IID_PPV_ARGS(&unk)))) continue;
                    JumpItem item;
                    if (ReadJumpItem(unk, iconSize, &item)) {
                        category.items.push_back(item);
                    } else {
                        if (item.target) CoTaskMemFree(item.target);
                        if (item.icon) DestroyIcon(item.icon);
                    }
                    unk->Release();
                }
                items->Release();
            }
            // No separator at either end.
            auto& v = category.items;
            while (!v.empty() && v.back().separator) v.pop_back();
            while (!v.empty() && v.front().separator) v.erase(v.begin());
            if (!v.empty()) list.push_back(std::move(category));
        }
    }
    custom->Release();
    return list;
}

// Jump lists are read once and kept for a minute, and read ahead in the background when the
// pointer rests on a button, so the right-click that follows doesn't wait for the app's
// destinations file and its icons.
struct JumpCacheEntry {
    std::vector<JumpCategory> list;
    DWORD when = 0;
    int iconSize = 0;
};
std::unordered_map<std::wstring, JumpCacheEntry> g_jumpCache;
std::vector<std::wstring> g_jumpPending;  // apps being read now
SRWLOCK g_jumpLock = SRWLOCK_INIT;
LONG g_jumpThreads = 0;
constexpr DWORD kJumpKeepMs = 60000;

std::vector<JumpCategory> CloneJumpList(std::vector<JumpCategory> const& list) {
    std::vector<JumpCategory> copy = list;
    for (auto& category : copy) {
        for (auto& item : category.items) {
            if (item.target) item.target = ILCloneFull(item.target);
            if (item.icon) item.icon = CopyIcon(item.icon);
        }
    }
    return copy;
}

void StoreJumpList(std::wstring const& appId, int iconSize, std::vector<JumpCategory> const& list) {
    JumpCacheEntry entry;
    entry.list = CloneJumpList(list);
    entry.when = GetTickCount();
    entry.iconSize = iconSize;
    AcquireSRWLockExclusive(&g_jumpLock);
    auto& slot = g_jumpCache[appId];
    FreeJumpList(slot.list);
    slot = std::move(entry);
    ReleaseSRWLockExclusive(&g_jumpLock);
}

bool FreshJumpList(std::wstring const& appId, int iconSize, std::vector<JumpCategory>* out) {
    bool fresh = false;
    AcquireSRWLockShared(&g_jumpLock);
    auto it = g_jumpCache.find(appId);
    if (it != g_jumpCache.end() && it->second.iconSize == iconSize && GetTickCount() - it->second.when < kJumpKeepMs) {
        fresh = true;
        if (out) *out = CloneJumpList(it->second.list);
    }
    ReleaseSRWLockShared(&g_jumpLock);
    return fresh;
}

std::vector<JumpCategory> ReadJumpListCached(std::wstring const& appId, int iconSize) {
    std::vector<JumpCategory> list;
    if (appId.empty() || FreshJumpList(appId, iconSize, &list)) return list;
    list = ReadJumpList(appId, iconSize);
    StoreJumpList(appId, iconSize, list);
    return list;
}

DWORD WINAPI PrefetchJumpThread(LPVOID param) {
    std::unique_ptr<std::pair<std::wstring, int>> job((std::pair<std::wstring, int>*)param);
    if (SUCCEEDED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED))) {
        std::vector<JumpCategory> list = ReadJumpList(job->first, job->second);
        StoreJumpList(job->first, job->second, list);
        FreeJumpList(list);
        CoUninitialize();
    }
    AcquireSRWLockExclusive(&g_jumpLock);
    std::erase(g_jumpPending, job->first);
    ReleaseSRWLockExclusive(&g_jumpLock);
    InterlockedDecrement(&g_jumpThreads);
    return 0;
}

void PrefetchJumpList(std::wstring const& appId, int iconSize) {
    if (appId.empty() || FreshJumpList(appId, iconSize, nullptr)) return;
    AcquireSRWLockExclusive(&g_jumpLock);
    bool started = std::find(g_jumpPending.begin(), g_jumpPending.end(), appId) == g_jumpPending.end();
    if (started) g_jumpPending.push_back(appId);
    ReleaseSRWLockExclusive(&g_jumpLock);
    if (!started) return;
    InterlockedIncrement(&g_jumpThreads);
    auto* job = new std::pair<std::wstring, int>(appId, iconSize);
    HANDLE thread = CreateThread(nullptr, 0, PrefetchJumpThread, job, 0, nullptr);
    if (thread) {
        CloseHandle(thread);
    } else {
        delete job;
        InterlockedDecrement(&g_jumpThreads);
        AcquireSRWLockExclusive(&g_jumpLock);
        std::erase(g_jumpPending, appId);
        ReleaseSRWLockExclusive(&g_jumpLock);
    }
}

void FreeJumpCache() {
    for (int wait = 0; g_jumpThreads > 0 && wait < 60; wait++) Sleep(50);  // a read in progress finishes first
    AcquireSRWLockExclusive(&g_jumpLock);
    for (auto& [id, entry] : g_jumpCache) FreeJumpList(entry.list);
    g_jumpCache.clear();
    ReleaseSRWLockExclusive(&g_jumpLock);
}

void RunJumpItem(JumpItem const& item) {
    SHELLEXECUTEINFOW info{sizeof(info)};
    info.fMask = SEE_MASK_IDLIST | SEE_MASK_ASYNCOK;
    info.lpIDList = item.target;
    info.lpParameters = item.args.empty() ? nullptr : item.args.c_str();
    info.lpDirectory = item.dir.empty() ? nullptr : item.dir.c_str();
    info.nShow = item.show ? item.show : SW_SHOWNORMAL;
    ShellExecuteExW(&info);
}

void HidePreview();
bool StartCapsSetting();  // (the Start menu's, defined with it)

// The glyphs StartAllBack's task menus show beside Unpin, End task and Close (measured from a
// capture of its Bouquet menu: 16 x 16, ARGB, clear where the menu's ground showed).
constexpr uint32_t kGlyphUnpin[16 * 16] = {
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xFF0F232C, 0xFF043D55, 0xFF02425D, 0xFF07384C, 0xFF102129, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xFF043E57, 0xFF1383B0, 0xFF55ADD1, 0xFF4FA0C2, 0xFF01638C, 0xFF0A2D3B, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xFF014460, 0xFF3E9FC6, 0xFF0E8BBE, 0xFF138DBF, 0xFF59ABCC, 0xFF02638B, 0xFF102129, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0xFF083446, 0xFF023F57, 0xFF034159, 0xFF07374B, 0xFF0A2D3C, 0xFF014C6B, 0xFF2A8BB2, 0xFF0482B6, 0xFF016A95, 0xFF0985B7, 0xFF3F95B8, 0xFF07374B, 0x00000000,
    0x00000000, 0x00000000, 0xFF083446, 0xFF015A7D, 0xFF1182B0, 0xFF147FAB, 0xFF015A7F, 0xFF016086, 0xFF308FB5, 0xFF015B80, 0xFF2E93BC, 0xFF017CAF, 0xFF017AAB, 0xFF3798BF, 0xFF02415B, 0x00000000,
    0x00000000, 0x00000000, 0xFF023F57, 0xFF248BB5, 0xFF0172A0, 0xFF015679, 0xFF0E82B1, 0xFF238FBB, 0xFF0C85B6, 0xFF015A7F, 0xFF01597E, 0xFF1D82AB, 0xFF238CB7, 0xFF016F9D, 0xFF043C53, 0x00000000,
    0x00000000, 0x00000000, 0xFF034159, 0xFF409ABF, 0xFF01597C, 0xFF015273, 0xFF015273, 0xFF015B80, 0xFF016189, 0xFF0A85B7, 0xFF248AB3, 0xFF014B69, 0xFF01435F, 0xFF043E55, 0xFF0F232C, 0x00000000,
    0x00000000, 0x00000000, 0xFF073749, 0xFF348EB3, 0xFF017BAC, 0xFF015273, 0xFF015273, 0xFF01577A, 0xFF015E84, 0xFF168AB9, 0xFF01597E, 0xFF0A2D3C, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xFF0E242D, 0xFF025A7E, 0xFF489FC3, 0xFF01597E, 0xFF015273, 0xFF015273, 0xFF01577A, 0xFF0179A9, 0xFF01587B, 0xFF07374B, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0xFF135C78, 0xFF2C85A9, 0xFF2E93BC, 0xFF01597E, 0xFF015273, 0xFF015273, 0xFF015679, 0xFF137EA9, 0xFF034159, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xFF666E72, 0xFF9BA2A5, 0xFF177195, 0xFF2B84A9, 0xFF469EC2, 0xFF017AAB, 0xFF01597C, 0xFF0172A0, 0xFF1282B0, 0xFF023F57, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0xFF666F73, 0xFF9FA6A9, 0xFFCDD3D5, 0xFF9BA2A5, 0xFF135C78, 0xFF025679, 0xFF318CB2, 0xFF3E99BE, 0xFF248BB5, 0xFF01577A, 0xFF083446, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0xFF969DA0, 0xFFCDD3D5, 0xFF9FA6A9, 0xFF666E72, 0x00000000, 0xFF0E242D, 0xFF073749, 0xFF034159, 0xFF023F57, 0xFF083446, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0xFF929A9D, 0xFF969DA0, 0xFF666F73, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
};
constexpr uint32_t kGlyphEndTask[16 * 16] = {
    0x00000000, 0x00000000, 0xFF727573, 0xFF888A85, 0xFF888A85, 0xFF888A85, 0xFF888A85, 0xFF888A85, 0xFF888A85, 0xFF888A85, 0xFF888A85, 0xFF888A85, 0xFF888A85, 0xFF727573, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xFF888A85, 0xFFF1F1F1, 0xFFEBEBEB, 0xFFEBEBEB, 0xFFEBEBEB, 0xFFEAEAEA, 0xFFEAEAEA, 0xFFEAEAEA, 0xFFEAEAEA, 0xFFEAEAEA, 0xFFE9E9E9, 0xFF888A85, 0x00000000, 0x00000000,
    0xFF193354, 0xFF204A87, 0xFF888A85, 0xFFE7E7E7, 0xFFD7D7D7, 0xFFD6D6D6, 0xFFD6D6D6, 0xFFD6D6D6, 0xFFD6D6D6, 0xFFD5D5D5, 0xFFD5D5D5, 0xFFDADADA, 0xFFDCDCDC, 0xFF888A85, 0xFF204A87, 0xFF193354,
    0xFF204A87, 0xFF9CBBDD, 0xFF7A7C77, 0xFFD9D9D9, 0xFFBDBDBD, 0xFFBDBDBD, 0xFFBDBDBD, 0xFFBDBDBD, 0xFFBCBCBC, 0xFFBCBCBC, 0xFFBCBCBC, 0xFFBFBFBF, 0xFFC0C0C0, 0xFF7A7C77, 0xFF9CBBDD, 0xFF204A87,
    0xFF204A87, 0xFF9CBBDD, 0xFF4E5B6A, 0xFFC9C9C9, 0xFF9C9C9C, 0xFF9C9C9C, 0xFF9C9C9C, 0xFF9B9B9B, 0xFF9B9B9B, 0xFF9B9B9B, 0xFF9B9B9B, 0xFF9B9B9B, 0xFF9B9B9B, 0xFF4E5B6A, 0xFF9CBBDD, 0xFF204A87,
    0xFF204A87, 0xFF9CBBDD, 0xFF2C5286, 0xFF2B5186, 0xFF2B5186, 0xFF2B5186, 0xFF2B5186, 0xFF2B5186, 0xFF2C5286, 0xFF2C5286, 0xFF2C5286, 0xFF2C5286, 0xFF2C5286, 0xFF2C5286, 0xFF9CBBDD, 0xFF204A87,
    0xFF204A87, 0xFF9CBBDD, 0xFF588ACA, 0xFF588ACA, 0xFF588ACA, 0xFF588ACA, 0xFF588ACA, 0xFF588ACA, 0xFF588ACA, 0xFF588ACA, 0xFF588ACA, 0xFF588ACA, 0xFF588ACA, 0xFF588ACA, 0xFF9CBBDD, 0xFF204A87,
    0xFF204A87, 0xFF9CBBDD, 0xFF9CBBDD, 0xFF9CBBDD, 0xFF9CBBDD, 0xFF9CBBDD, 0xFF9CBBDD, 0xFF9CBBDD, 0xFF9CBBDD, 0xFF9CBBDD, 0xFF9CBBDD, 0xFF9CBBDD, 0xFF9CBBDD, 0xFF9CBBDD, 0xFF9CBBDD, 0xFF204A87,
    0xFF204A87, 0xFF9CBBDD, 0xFF729FCF, 0xFF729FCF, 0xFF729FCF, 0xFF729FCF, 0xFF729FCF, 0xFF729FCF, 0xFF729FCF, 0xFF729FCF, 0xFF729FCF, 0xFF729FCF, 0xFF729FCF, 0xFF729FCF, 0xFF9CBBDD, 0xFF204A87,
    0xFF1C3E6E, 0xFF204A87, 0xFF204A87, 0xFF204A87, 0xFF204A87, 0xFF204A87, 0xFF204A87, 0xFF204A87, 0xFF204A87, 0xFF204A87, 0xFF204A87, 0xFF204A87, 0xFF204A87, 0xFF204A87, 0xFF204A87, 0xFF1C3E6E,
    0x00000000, 0x00000000, 0xFF6D6E6A, 0xFFC9CAC9, 0xFF656562, 0xFFA7A9A7, 0xFF6E6F6C, 0xFFACADAC, 0xFF6C6D6A, 0xFFAEB0AE, 0xFF70716D, 0xFFB0B0B0, 0xFF6E6F6B, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xFF737571, 0xFFB9BBBA, 0xFF727671, 0xFF929393, 0xFF8B8B8A, 0xFFA2A4A3, 0xFF858786, 0xFFA0A2A1, 0xFF7F807C, 0xFFAAACAC, 0xFF747572, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xFF80827F, 0xFF878B8C, 0xFF7B7F7A, 0xFFB5B5B6, 0xFF7E827D, 0xFFBCBEBD, 0xFF909290, 0xFFAAACAC, 0xFF7F807C, 0xFFD8D8D8, 0xFF868885, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0xFF5D6161, 0xFF7F817D, 0xFFF2F2F3, 0xFF9B9C99, 0xFFACADAB, 0xFF7A7E7A, 0xFFCFD1D0, 0xFF7F8180, 0xFFB3B5B4, 0xFF9A9B98, 0xFFDEDFDF, 0xFF8B8D8A, 0xFF484E50, 0x00000000, 0x00000000,
    0x00000000, 0xFF8E8F8B, 0xFFFAFAFA, 0xFF939592, 0xFF858885, 0xFF929491, 0xFF989A96, 0xFFE2E3E2, 0xFF7C807D, 0xFF9D9E9B, 0xFF9A9B98, 0xFF8D8E8B, 0xFFE0E2E2, 0xFF717372, 0x00000000, 0x00000000,
    0x00000000, 0xFF5D6161, 0xFF939491, 0xFFAFB0AD, 0x00000000, 0x00000000, 0xFF929490, 0xFF9E9F9C, 0xFF8C8F8C, 0x00000000, 0x00000000, 0xFF4E5557, 0xFF676968, 0xFF696E6E, 0x00000000, 0x00000000,
};
constexpr uint32_t kGlyphClose[16 * 16] = {
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0xFF88292F, 0xFF882A30, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xFF882A30, 0xFF882A30, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xFF8D272D, 0xFFC5494F, 0xFFC54A50, 0xFF8D272D, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xFF8E292E, 0xFFC64B51, 0xFFC64C52, 0xFF8E292E, 0x00000000, 0x00000000,
    0x00000000, 0xFF85252C, 0xFFC4454B, 0xFFE98F93, 0xFFE99094, 0xFFC4464C, 0xFF8D262C, 0x00000000, 0x00000000, 0xFF8D262C, 0xFFC4464C, 0xFFE99094, 0xFFE98F93, 0xFFC4454B, 0xFF85252C, 0x00000000,
    0x00000000, 0xFF85232A, 0xFFC24045, 0xFFE68589, 0xFFE5787D, 0xFFE6878B, 0xFFC24146, 0xFF8B242A, 0xFF8B242A, 0xFFC24146, 0xFFE6878B, 0xFFE4787D, 0xFFE7878B, 0xFFC24045, 0xFF84232A, 0x00000000,
    0x00000000, 0x00000000, 0xFF892026, 0xFFC13B41, 0xFFE57D82, 0xFFE37075, 0xFFE67F84, 0xFFC03D43, 0xFFC03D43, 0xFFE67F84, 0xFFE37176, 0xFFE57D82, 0xFFC03A3F, 0xFF892026, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0xFF871F25, 0xFFBD363C, 0xFFE3767B, 0xFFE16A6F, 0xFFE4777C, 0xFFE4777C, 0xFFE0686D, 0xFFE3767B, 0xFFBD363C, 0xFF871F25, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xFF831E23, 0xFFBC343B, 0xFFDF6E73, 0xFFDE696E, 0xFFDE696E, 0xFFDF6E73, 0xFFBB343B, 0xFF831E23, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xFF7B1D22, 0xFFB72E35, 0xFFDA6469, 0xFFDA6267, 0xFFDA6267, 0xFFDA6469, 0xFFB72E35, 0xFF7B1D22, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0xFF771C21, 0xFFB5282E, 0xFFD5585E, 0xFFD35157, 0xFFD65A60, 0xFFD65A60, 0xFFD45359, 0xFFD5585E, 0xFFB5282E, 0xFF771C21, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xFF721B20, 0xFFB22127, 0xFFD15056, 0xFFCF494F, 0xFFD15056, 0xFFB2232B, 0xFFB2232B, 0xFFD15056, 0xFFCF494F, 0xFFD15056, 0xFFB22127, 0xFF721B20, 0x00000000, 0x00000000,
    0x00000000, 0xFF681A1F, 0xFFAC1F26, 0xFFCD454B, 0xFFCC4147, 0xFFCC454B, 0xFFAC1F26, 0xFF6B1A1E, 0xFF6B1A1E, 0xFFAC1F26, 0xFFCC454B, 0xFFCB4147, 0xFFCD454B, 0xFFAC1F26, 0xFF681A1F, 0x00000000,
    0x00000000, 0xFF63191F, 0xFFA01C22, 0xFFC73A40, 0xFFC73A40, 0xFFA01D23, 0xFF6A1A1E, 0x00000000, 0x00000000, 0xFF6A1A1E, 0xFFA01D23, 0xFFC73B41, 0xFFC73A40, 0xFFA01D23, 0xFF63191F, 0x00000000,
    0x00000000, 0x00000000, 0xFF61181C, 0xFF961B21, 0xFF961B21, 0xFF61181C, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xFF61181C, 0xFF961B21, 0xFF961B21, 0xFF61181C, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0xFF58171C, 0xFF58171C, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xFF58171C, 0xFF58171C, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
};

HICON GlyphIcon(uint32_t const* argb) {
    BITMAPINFO bi{};
    bi.bmiHeader = {sizeof(BITMAPINFOHEADER), 16, -16, 1, 32, BI_RGB};
    void* bits = nullptr;
    HDC dc = GetDC(nullptr);
    HBITMAP color = CreateDIBSection(dc, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
    ReleaseDC(nullptr, dc);
    if (!color) return nullptr;
    memcpy(bits, argb, 16 * 16 * 4);
    HBITMAP mask = CreateBitmap(16, 16, 1, 1, nullptr);
    ICONINFO info{TRUE, 0, 0, mask, color};
    HICON icon = CreateIconIndirect(&info);
    DeleteObject(color);
    DeleteObject(mask);
    return icon;
}

// An icon from Windows' own imageres.dll, as StartAllBack's task menu takes its glyphs (5100 the
// pin, 89 the shredder, 5102 the X): so an icon theme that replaces imageres's icons (Resource
// or Icon Redirect) reaches these too. The pictures copied above are only for a PC without them.
HICON ImageresIcon(int id, uint32_t const* fallback) {
    static HMODULE module = LoadLibraryExW(L"imageres.dll", nullptr, LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_IMAGE_RESOURCE);
    int size = 16 * g_scale;
    HICON icon = module ? (HICON)LoadImageW(module, MAKEINTRESOURCEW(id), IMAGE_ICON, size, size, 0) : nullptr;
    return icon ? icon : GlyphIcon(fallback);
}

// The icon title font, which StartAllBack draws its menus in (its own capitals face on this PC).
HFONT g_menuIconFont = nullptr;
HFONT MenuIconFont() {
    if (!g_menuIconFont) {
        LOGFONTW lf;
        if (SystemParametersInfoForDpi(SPI_GETICONTITLELOGFONT, sizeof(lf), &lf, 0, 96 * g_scale))
            g_menuIconFont = CreateFontIndirectW(&lf);
    }
    return g_menuIconFont;
}

// The bar's menu look: the style's menu look if it has one, else the scheme; StartAllBack's framed look
// with its bar background (its icon title font, capitals, icons; Menu No Icons still decides about icons).
MenuLook BarMenuLook() {
    auto const& p = g_settings.palette;
    MenuLook look = {p.ground, p.line, p.edge, p.text, p.accent, p.dim, p.hover, g_tipFont,
                     g_themed ? g_theme.menu : nullptr, g_scale, !g_settings.menuIcons};
    if (g_settings.sabBackdrop && g_themed && !look.theme) FrameLikeStartAllBack(look, MenuIconFont(), StartCapsSetting());
    return look;
}

// How menus open (menuAnimation), on the motion engine's popups curve when a motion line sets one.
double PopupsEase(double t) { return motion::Ease(g_settings.motionPopups, t); }

void ApplyMenuMotion() {
    g_menuMotion = {};
    g_menuMotion.style = g_settings.menuAnimation;
    if (g_settings.motionPopups.set && g_settings.motionPopups.on) {
        g_menuMotion.ms = std::clamp(motion::Ms(g_settings.motionPopups), 0.0, 1500.0);
        g_menuMotion.ease = PopupsEase;
    }
}

// The bar's menus as Tourne Menus draws them in Explorer, the file dialogs and Windows' own menus
// (tourne-menus.wh.cpp): their look, how they open and the rules, in HKCU\Software\Tourne\Menus.
// Written by the main bar as its settings apply; Tourne Menus reads it as each menu opens.
void PublishMenuLook() {
    MenuLook look = BarMenuLook();
    if (MenusLikeWindows()) LookLikeWindowsMenus(look);
    HKEY key = nullptr;
    if (RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\Tourne\\Menus", 0, nullptr, 0, KEY_SET_VALUE, nullptr, &key, nullptr))
        return;
    auto dword = [&](PCWSTR name, DWORD v) { RegSetValueExW(key, name, 0, REG_DWORD, (BYTE const*)&v, sizeof(v)); };
    auto text = [&](PCWSTR name, std::wstring const& v) {
        RegSetValueExW(key, name, 0, REG_SZ, (BYTE const*)v.c_str(), (DWORD)((v.size() + 1) * sizeof(wchar_t)));
    };
    auto joined = [](std::vector<std::wstring> const& list) {
        std::wstring out;
        for (auto const& x : list) out += (out.empty() ? L"" : L", ") + x;
        return out;
    };
    dword(L"Ground", look.ground), dword(L"Line", look.line), dword(L"Edge", look.edge), dword(L"Text", look.text);
    dword(L"Accent", look.accent), dword(L"Dim", look.dim), dword(L"Hover", look.hover);
    dword(L"HoverText", look.hoverText), dword(L"Arrow", look.arrow), dword(L"FrameLine", look.frameLine), dword(L"Band", look.band);
    dword(L"Native", look.native), dword(L"Classic", look.classic), dword(L"OwnText", look.ownText), dword(L"Frame", look.frame);
    dword(L"Caps", look.caps), dword(L"NoIcons", look.noIcons), dword(L"Windows", look.windows);
    dword(L"FrameGap", look.frameGap), dword(L"FrameGlow", look.frameGlow), dword(L"FrameStopCount", look.frameStopCount);
    RegSetValueExW(key, L"FrameStops", 0, REG_BINARY, (BYTE const*)look.frameStops, sizeof(look.frameStops));
    dword(L"Scale", look.scale);
    LOGFONTW font{};
    if (look.font && GetObjectW(look.font, sizeof(font), &font)) RegSetValueExW(key, L"Font", 0, REG_BINARY, (BYTE const*)&font, sizeof(font));
    else RegDeleteValueW(key, L"Font");
    text(L"Theme", look.theme ? g_settings.style : L"");
    dword(L"Animation", g_menuMotion.style);
    dword(L"FlyoutClick", Wh_GetIntSetting(L"menuFlyoutClick") != 0);
    dword(L"AnimationMs", (DWORD)std::lround(g_menuMotion.ms));
    std::wstring curve;
    if (g_menuMotion.ease) {
        auto const& c = g_settings.motionPopups.curve;
        WCHAR line[160];
        if (c.spring) swprintf_s(line, L"spring %g %g %g", c.mass, c.stiffness, c.damping);
        else swprintf_s(line, L"bezier %g %g %g %g", c.x1, c.y1, c.x2, c.y2);
        curve = line;
    }
    text(L"Curve", curve);
    text(L"Hide", joined(g_settings.menuRules.hide));
    text(L"More", joined(g_settings.menuRules.more));
    text(L"MoreLabel", g_settings.menuRules.moreLabel);
    dword(L"Stamp", GetTickCount());
    RegCloseKey(key);
}

// A popup menu at the pointer; returns the chosen command, or 0. Menus that
// open their own menu on a right-click leave out TPM_RIGHTBUTTON, and
// TPM_NONOTIFY, which would hide the right-click from the bar.
// A button whose right-click menu is opening (on the screen), for placing it beside the bar.
bool g_menuFromButton = false;
RECT g_menuButton{};

int RunMenu(HWND wnd, HMENU menu, UINT flags = TPM_RIGHTBUTTON | TPM_NONOTIFY) {
    POINT pt;
    GetCursorPos(&pt);
    HidePreview();
    if (g_tooltip) SendMessageW(g_tooltip, TTM_POP, 0, 0);
    // A button's menu beside the bar, as set: lined up with the button, clear of it (and kept off the
    // bar and on its screen by TrackStyledMenu).
    RECT bar;
    GetWindowRect(wnd, &bar);
    if (g_menuFromButton && g_settings.jumpPlace) {
        RECT const& r = g_menuButton;
        bool centre = g_settings.jumpPlace == 2;
        int away = Px(g_settings.jumpAway), along = Px(g_settings.jumpAlong);
        LONG midX = (r.left + r.right) / 2, midY = (r.top + r.bottom) / 2;
        switch (g_settings.edge) {
            case Edge::Bottom:
                pt = {(centre ? midX : r.left) + along, bar.top - away};
                flags |= TPM_BOTTOMALIGN | (centre ? TPM_CENTERALIGN : TPM_LEFTALIGN);
                break;
            case Edge::Top:
                pt = {(centre ? midX : r.left) + along, bar.bottom + away};
                flags |= TPM_TOPALIGN | (centre ? TPM_CENTERALIGN : TPM_LEFTALIGN);
                break;
            case Edge::Left:
                pt = {bar.right + away, (centre ? midY : r.top) + along};
                flags |= TPM_LEFTALIGN | (centre ? TPM_VCENTERALIGN : TPM_TOPALIGN);
                break;
            case Edge::Right:
                pt = {bar.left - away, (centre ? midY : r.top) + along};
                flags |= TPM_RIGHTALIGN | (centre ? TPM_VCENTERALIGN : TPM_TOPALIGN);
                break;
        }
        if (g_settings.edge == Edge::Bottom || g_settings.edge == Edge::Top) flags |= TPM_VERTICAL;
    }
    bool beside = g_menuFromButton && g_settings.jumpPlace;
    g_menuFromButton = false;
    g_inMenu = true;
    SetForegroundWindow(wnd);
    g_menuLook = BarMenuLook();
    // Beside the bar it mustn't cover it; at the pointer it only has to stay on the pointer's screen.
    int cmd = (int)TrackStyledMenu(menu, TPM_RETURNCMD | flags, pt.x, pt.y, wnd, false, beside ? &bar : nullptr);
    PostMessageW(wnd, WM_NULL, 0, 0);
    g_inMenu = false;
    DestroyMenu(menu);
    return cmd;
}

// A group with several windows: a list to pick one from, until thumbnails.
void PickWindow(HWND wnd, Button const& b) {
    HMENU menu = CreatePopupMenu();
    for (size_t i = 0; i < b.windows.size(); i++) {
        std::wstring title = Title(b.windows[i]);
        AppendMenuW(menu, MF_STRING | (b.windows[i] == ForegroundRoot() ? MF_CHECKED : 0), i + 1, title.c_str());
    }
    int cmd = RunMenu(wnd, menu);
    if (cmd > 0 && cmd <= (int)b.windows.size()) ActivateWindow(b.windows[cmd - 1]);
}

enum : int { kCmdNew = 1, kCmdRestore, kCmdMinimize, kCmdMaximize, kCmdClose, kCmdTaskManager, kCmdDesktop,
             kCmdTaskbarSettings, kCmdTourneSettings, kCmdOpenStyles, kCmdMoveLeft, kCmdMoveRight, kCmdEndTask, kCmdInstallTheme, kCmdGetYasb,
             kCmdWindhawk, kCmdPluginIcon, kCmdPluginIconBack };
// Tray placement commands: kCmdPlace + icon * 4 + TrayPlace; a running icon in or out of the tray
// (kCmdTrayToggle + icon); one not running, placed for when it starts (kCmdTrayKnown + icon).
constexpr int kCmdPlace = 1000, kCmdTrayToggle = 3000, kCmdTrayKnown = 4000;

enum : int { kCmdOpenPin = 200, kCmdRunAsAdmin, kCmdOpenLocation };

// A pinned app that isn't running.
void PinMenu(HWND wnd, int index) {
    if (index < 0 || index >= (int)g_pins.size()) return;
    Pin const pin = g_pins[index];
    HMENU menu = CreatePopupMenu();
    AppendMenuW(menu, MF_STRING, kCmdOpenPin, pin.name.c_str());
    AppendMenuW(menu, MF_STRING, kCmdRunAsAdmin, L"Run as administrator");
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING, kCmdOpenLocation, L"Open file location");
    switch (RunMenu(wnd, menu)) {
        case kCmdOpenPin: LaunchPin(index); break;
        case kCmdRunAsAdmin:
            ShellExecuteW(nullptr, L"runas", pin.shortcut.file.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
            break;
        case kCmdOpenLocation: {
            std::wstring target = pin.shortcut.program.empty() ? pin.shortcut.file : pin.shortcut.program;
            std::wstring args = L"/select,\"" + target + L"\"";
            ShellExecuteW(nullptr, nullptr, L"explorer.exe", args.c_str(), nullptr, SW_SHOWNORMAL);
            break;
        }
    }
}

// Shift+right-click on a button: the window's own commands (or, for a pin
// that isn't running, how to open it).
void WindowMenu(HWND wnd, Button const& b) {
    if (b.windows.empty()) {
        PinMenu(wnd, b.pin);
        return;
    }
    HWND h = b.windows[0];
    Win* w = FindWin(h);
    HMENU menu = CreatePopupMenu();
    if (b.pin >= 0 && b.pin < (int)g_pins.size()) {
        AppendMenuW(menu, MF_STRING, kCmdNew, (L"New window (" + g_pins[b.pin].name + L")").c_str());
        AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    } else if (w && !w->exe.empty()) {
        std::wstring name = w->exe.substr(w->exe.rfind(L'\\') + 1);
        AppendMenuW(menu, MF_STRING, kCmdNew, (L"New window (" + name + L")").c_str());
        AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    }
    if (b.windows.size() == 1) {
        bool iconic = IsIconic(h), zoomed = IsZoomed(h);
        AppendMenuW(menu, MF_STRING | (iconic || zoomed ? 0 : MF_GRAYED), kCmdRestore, L"Restore");
        AppendMenuW(menu, MF_STRING | (iconic ? MF_GRAYED : 0), kCmdMinimize, L"Minimize");
        AppendMenuW(menu, MF_STRING | (zoomed ? MF_GRAYED : 0), kCmdMaximize, L"Maximize");
        AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
        AppendMenuW(menu, MF_STRING, kCmdClose, L"Close window");
    } else {
        AppendMenuW(menu, MF_STRING, kCmdClose, L"Close all windows");
    }
    switch (RunMenu(wnd, menu)) {
        case kCmdNew:
            if (b.pin >= 0) LaunchPin(b.pin);
            else if (w) Launch(w->exe);
            break;
        case kCmdRestore: PostMessageW(h, WM_SYSCOMMAND, SC_RESTORE, 0); break;
        case kCmdMinimize: PostMessageW(h, WM_SYSCOMMAND, SC_MINIMIZE, 0); break;
        case kCmdMaximize:
            if (IsIconic(h)) ShowWindowAsync(h, SW_RESTORE);
            PostMessageW(h, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
            break;
        case kCmdClose:
            for (HWND each : b.windows) PostMessageW(each, WM_SYSCOMMAND, SC_CLOSE, 0);
            break;
    }
}

// The name Windows shows for a running app: its shortcut's, else its
// program file's.
std::wstring AppName(Win const& w) {
    std::wstring shortcut = w.shortcut;
    if (shortcut.empty())
        if (Shortcut const* s = FindShortcut(w, false)) shortcut = s->file;
    // A packaged app (Settings runs in ApplicationFrameHost): its Start entry.
    if (shortcut.empty() && w.appId.find(L'!') != std::wstring::npos) shortcut = L"shell:AppsFolder\\" + w.appId;
    IShellItem* item = nullptr;
    if (!shortcut.empty() && SUCCEEDED(SHCreateItemFromParsingName(shortcut.c_str(), nullptr, IID_PPV_ARGS(&item)))) {
        LPWSTR name = nullptr;
        std::wstring result;
        if (SUCCEEDED(item->GetDisplayName(SIGDN_NORMALDISPLAY, &name)) && name) {
            result = name;
            CoTaskMemFree(name);
        }
        item->Release();
        if (!result.empty()) return result;
    }
    std::wstring file = w.exe.substr(w.exe.find_last_of(L'\\') + 1);
    if (Lower(file).ends_with(L".exe")) file.resize(file.size() - 4);
    return file;
}

// Another window of a running app, through its shortcut when it has one.
void LaunchApp(Win const& w) {
    if (ShellLaunchAppsFolder(w.shortcut)) return;  // (an app's Start entry: works without Explorer too)
    PIDLIST_ABSOLUTE target = nullptr;
    if (!w.shortcut.empty() && SUCCEEDED(SHParseDisplayName(w.shortcut.c_str(), nullptr, &target, 0, nullptr))) {
        SHELLEXECUTEINFOW info{sizeof(info)};
        info.fMask = SEE_MASK_IDLIST | SEE_MASK_ASYNCOK;
        info.lpIDList = target;
        info.nShow = SW_SHOWNORMAL;
        BOOL ok = ShellExecuteExW(&info);
        CoTaskMemFree(target);
        if (ok) return;
    }
    Launch(w.exe);
}

// Pins or unpins through the suite's Explorer relay; Windows lets only
// Explorer change the taskbar's pins. The bar follows once Taskband changes.
bool RelayPin(HWND wnd, std::wstring const& path, bool pin) {
    HWND relay = RelayToolbar();
    if (!relay || path.empty()) return false;
    std::wstring packet = (pin ? L"1" : L"0") + path;
    COPYDATASTRUCT copyData{kRelayPinCopyId, (DWORD)((packet.size() + 1) * sizeof(WCHAR)), packet.data()};
    DWORD_PTR result = 0;
    return SendMessageTimeoutW(relay, WM_COPYDATA, (WPARAM)wnd, (LPARAM)&copyData, SMTO_ABORTIFHUNG, 2000, &result) &&
           result;
}

// What pinning a running app pins: the shortcut Windows keeps for it, its
// Start menu shortcut, else its program.
std::wstring PinTarget(Win const& w) {
    if (!w.shortcut.empty()) return w.shortcut;
    if (Shortcut const* s = FindShortcut(w, false)) return s->file;
    return w.exe;
}

enum : int { kCmdJump = 300, kCmdPin = 299 };  // kCmdJump + the item's place in the jump list

// The app's own shell menu (Open, Run as administrator, Open file location,
// Properties to change its icon...), from right-clicking the app in its jump
// list as on the classic taskbar. The command chosen runs once the jump list
// has closed.
struct ShellMenu {
    IContextMenu* menu = nullptr;
    IContextMenu2* menu2 = nullptr;
    IContextMenu3* menu3 = nullptr;
    HMENU popup = nullptr;
    int command = 0;
    POINT pt{};
    bool open = false;  // on screen: its submenus are styled as they fill in
    std::set<HMENU> filled;  // submenus the extensions have filled in already (ahead, or on a first hover)
};
ShellMenu g_shellMenu;
std::wstring g_jumpApp;  // while a jump list is open: its app's shortcut or program

void ReleaseShellMenu(ShellMenu& m) {
    if (m.menu3) m.menu3->Release();
    if (m.menu2) m.menu2->Release();
    if (m.menu) m.menu->Release();
    if (m.popup) DestroyMenu(m.popup);
    m = {};
}

void CloseShellMenu() { ReleaseShellMenu(g_shellMenu); }

// A shell menu's verb for one of its commands (ids from 1), lower case; empty when it has none.
std::wstring ShellMenuVerb(IContextMenu* menu, UINT id) {
    WCHAR verb[80] = L"";
    if (!menu || id < 1 || id > 0x7FFF ||
        FAILED(menu->GetCommandString(id - 1, GCS_VERBW, nullptr, (LPSTR)verb, ARRAYSIZE(verb))))
        return L"";
    verb[ARRAYSIZE(verb) - 1] = 0;
    for (WCHAR* c = verb; *c; c++) *c = (WCHAR)towlower(*c);
    return verb;
}

// The shell menu's own drawing and submenus.
bool ShellMenuMessageTo(ShellMenu& m, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT* result) {
    *result = 0;
    if (m.menu3) return SUCCEEDED(m.menu3->HandleMenuMsg2(msg, wParam, lParam, result));
    if (m.menu2) return SUCCEEDED(m.menu2->HandleMenuMsg(msg, wParam, lParam));
    return false;
}

// Every submenu of a shell menu filled in now (Send to, 7-Zip, WinRAR...), as each would be on its first hover:
// a flyout made ahead opens at once, instead of its extension filling it on the menu's thread as the pointer
// passes (which made menus with flyouts hitch).
void FillShellSubmenus(ShellMenu& m, HMENU menu, int depth = 0) {
    if (!menu || depth > 3) return;
    int count = GetMenuItemCount(menu);
    for (int i = 0; i < count; i++) {
        HMENU sub = GetSubMenu(menu, i);
        if (!sub || m.filled.count(sub)) continue;
        LRESULT result;
        ShellMenuMessageTo(m, WM_INITMENUPOPUP, (WPARAM)sub, MAKELPARAM(i, FALSE), &result);
        m.filled.insert(sub);
        FillShellSubmenus(m, sub, depth + 1);
    }
}

bool ShellMenuMessage(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT* result) {
    *result = 0;
    if (g_shellMenu.menu3) return SUCCEEDED(g_shellMenu.menu3->HandleMenuMsg2(msg, wParam, lParam, result));
    if (g_shellMenu.menu2) return SUCCEEDED(g_shellMenu.menu2->HandleMenuMsg(msg, wParam, lParam));
    return false;
}

void OpenShellMenu(HWND wnd, std::wstring const& path) {
    CloseShellMenu();
    PIDLIST_ABSOLUTE pidl = nullptr;
    if (path.empty() || FAILED(SHParseDisplayName(path.c_str(), nullptr, &pidl, 0, nullptr))) return;
    IShellItem* item = nullptr;
    if (SUCCEEDED(SHCreateItemFromIDList(pidl, IID_PPV_ARGS(&item)))) {
        item->BindToHandler(nullptr, BHID_SFUIObject, IID_PPV_ARGS(&g_shellMenu.menu));
        item->Release();
    }
    CoTaskMemFree(pidl);
    if (!g_shellMenu.menu) return;
    g_shellMenu.menu->QueryInterface(IID_PPV_ARGS(&g_shellMenu.menu2));
    g_shellMenu.menu->QueryInterface(IID_PPV_ARGS(&g_shellMenu.menu3));
    g_shellMenu.popup = CreatePopupMenu();
    UINT query = CMF_NORMAL | (GetKeyState(VK_SHIFT) < 0 ? CMF_EXTENDEDVERBS : 0);
    if (FAILED(g_shellMenu.menu->QueryContextMenu(g_shellMenu.popup, 0, 1, 0x7FFF, query))) {
        CloseShellMenu();
        return;
    }
    GetCursorPos(&g_shellMenu.pt);
    IContextMenu* shell = g_shellMenu.menu;
    MenuRulesUndo rules = ApplyMenuRules(g_shellMenu.popup, g_settings.menuRules, [&](UINT id) { return ShellMenuVerb(shell, id); });
    g_shellMenu.open = true;
    int cmd = (int)TrackStyledMenu(g_shellMenu.popup, TPM_RECURSE | TPM_RETURNCMD | TPM_RIGHTBUTTON,
                                   g_shellMenu.pt.x, g_shellMenu.pt.y, wnd, true);
    g_shellMenu.open = false;
    UndoMenuRules(rules);
    if (cmd > 0) {
        g_shellMenu.command = cmd;
        EndMenu();  // the jump list too
    } else {
        CloseShellMenu();
    }
}

// Runs what was chosen in the shell menu, if anything.
void RunShellCommand(HWND wnd) {
    if (g_shellMenu.menu && g_shellMenu.command > 0) {
        CMINVOKECOMMANDINFOEX info{sizeof(info)};
        info.fMask = CMIC_MASK_UNICODE | CMIC_MASK_PTINVOKE;
        info.hwnd = wnd;
        info.lpVerb = MAKEINTRESOURCEA(g_shellMenu.command - 1);
        info.lpVerbW = MAKEINTRESOURCEW(g_shellMenu.command - 1);
        info.nShow = SW_SHOWNORMAL;
        info.ptInvoke = g_shellMenu.pt;
        g_shellMenu.menu->InvokeCommand((CMINVOKECOMMANDINFO*)&info);
    }
    CloseShellMenu();
}

// Right-click on a button, as on the classic taskbar: the app's jump list,
// then the app itself and closing its windows.
void ButtonMenu(HWND wnd, Button const& b) {
    Win* w = b.windows.empty() ? nullptr : FindWin(b.windows[0]);
    bool pinned = b.pin >= 0 && b.pin < (int)g_pins.size();
    if (!w && !pinned) return;
    std::wstring appId = w && !w->appId.empty() ? w->appId : pinned ? g_pins[b.pin].appId : std::wstring();
    int iconSize = 16 * g_scale;  // menu-style draws item icons at this size
    std::vector<JumpCategory> list = ReadJumpListCached(appId, iconSize);

    HMENU menu = CreatePopupMenu();
    std::vector<JumpItem const*> items;
    for (auto const& category : list) {
        if (GetMenuItemCount(menu)) AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
        AppendMenuHeader(menu, category.name.c_str());
        for (auto const& item : category.items) {
            if (item.separator) {
                AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
                continue;
            }
            AppendMenuIcon(menu, MF_STRING, kCmdJump + items.size(), item.title.c_str(), item.icon);
            items.push_back(&item);
        }
    }
    if (GetMenuItemCount(menu)) AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    std::wstring name = pinned ? g_pins[b.pin].name : AppName(*w);
    // A copy: the icon cache can be refreshed while the menu is open.
    HICON cached = pinned ? PinIcon(b.pin, iconSize) : AppIcon(*w, iconSize);
    HICON appIcon = cached ? CopyIcon(cached) : nullptr;
    AppendMenuIcon(menu, MF_STRING, kCmdNew, name.c_str(), appIcon);
    std::wstring pinPath = pinned ? g_pins[b.pin].shortcut.file : PinTarget(*w);
    HICON pinGlyph = ImageresIcon(5100, kGlyphUnpin), endGlyph = ImageresIcon(89, kGlyphEndTask), closeGlyph = ImageresIcon(5102, kGlyphClose);
    AppendMenuIcon(menu, MF_STRING | (RelayToolbar() ? 0 : MF_GRAYED), kCmdPin,
                   pinned ? L"Unpin from taskbar" : L"Pin to taskbar", pinGlyph);
    if (!b.windows.empty()) {
        AppendMenuIcon(menu, MF_STRING, kCmdEndTask, L"End task", endGlyph);
        AppendMenuIcon(menu, MF_STRING, kCmdClose,
                       b.windows.size() == 1 ? L"Close window" : L"Close all windows on this display", closeGlyph);
    }

    // Right-clicking the app opens its shell menu (see WM_MENURBUTTONUP).
    g_jumpApp = pinned ? g_pins[b.pin].shortcut.file : PinTarget(*w);
    int cmd = RunMenu(wnd, menu, 0);
    g_jumpApp.clear();
    RunShellCommand(wnd);
    if (cmd >= kCmdJump && cmd - kCmdJump < (int)items.size()) {
        RunJumpItem(*items[cmd - kCmdJump]);
    } else if (cmd == kCmdNew) {
        if (pinned) LaunchPin(b.pin);
        else LaunchApp(*w);
    } else if (cmd == kCmdPin) {
        RelayPin(wnd, pinPath, !pinned);
    } else if (cmd == kCmdClose) {
        for (HWND each : b.windows) PostMessageW(each, WM_SYSCOMMAND, SC_CLOSE, 0);
    } else if (cmd == kCmdEndTask) {
        // The app's processes, as Task Manager's End task ends them.
        for (HWND each : b.windows) {
            DWORD pid = 0;
            GetWindowThreadProcessId(each, &pid);
            if (HANDLE process = pid ? OpenProcess(PROCESS_TERMINATE, FALSE, pid) : nullptr) {
                TerminateProcess(process, 1);
                CloseHandle(process);
            }
        }
    }
    for (HICON glyph : {pinGlyph, endGlyph, closeGlyph})
        if (glyph) DestroyIcon(glyph);
    if (appIcon) DestroyIcon(appIcon);
    FreeJumpList(list);
}

////////////////////////////////////////////////////////////////////////////////
// Window previews: resting on a running app's button shows its windows, live
// (DWM thumbnails), in a popup by the bar, each with its icon, title and a
// close button, as on the classic taskbar. When they don't all fit on the
// screen, a list of titles. Click one to switch to it, middle-click to close
// it, right-click for its window menu.

constexpr UINT_PTR kTimerPreviewShow = 5;
constexpr UINT_PTR kTimerPreviewHide = 6;
constexpr UINT_PTR kTimerPeek = 1;  // on the preview window
constexpr UINT_PTR kTimerPreviewFade = 2;  // on the preview window, while it fades in
constexpr WCHAR kPreviewClass[] = L"TournePreview_" WH_MOD_ID;
constexpr WCHAR kPreviewShadowClass[] = L"TournePreviewShadow_" WH_MOD_ID;  // the same, with a drop shadow
HINSTANCE g_instance;  // the classes' module

struct PreviewCell {
    HWND hwnd = nullptr;
    RECT rc{};                    // the cell, in the popup
    RECT title{};                 // its icon and title
    RECT close{};                 // its close button
    RECT thumb{};                 // where the thumbnail goes (empty in a list)
    HTHUMBNAIL thumbnail = nullptr;
};

struct Preview {
    HWND wnd = nullptr;
    HWND bar = nullptr;           // the bar it belongs to
    int button = -1;
    bool list = false;
    std::vector<PreviewCell> cells;
    int hot = -1;
    bool hotClose = false;
    bool tracking = false;
    bool dismiss = false;         // dismissed: goes at the hide timer whatever the pointer's on
    POINT at{};                   // the pointer when it opened (placed on the pointer)
    RECT buttonRc{};              // its button, on the screen
    bool fading = false;          // fading in (motion.fadePopupsIn)
    bool layered = false;         // the whole window fading, layered until it's in
    double fade = 1, fadeStart = 0;
};
Preview g_preview;

// Previews kept until dismissed: a click elsewhere, or a key, while they're up. The hooks are
// only in while they are.
HHOOK g_previewMouseHook = nullptr, g_previewKeyHook = nullptr;

void DismissPreview() {
    if (!g_preview.wnd || !g_preview.bar) return;
    g_preview.dismiss = true;
    SetTimer(g_preview.bar, kTimerPreviewHide, 1, nullptr);
}

LRESULT CALLBACK PreviewMouseProc(int code, WPARAM wParam, LPARAM lParam) {
    if (code == HC_ACTION && g_preview.wnd &&
        (wParam == WM_LBUTTONDOWN || wParam == WM_RBUTTONDOWN || wParam == WM_MBUTTONDOWN || wParam == WM_XBUTTONDOWN)) {
        POINT pt = ((MSLLHOOKSTRUCT*)lParam)->pt;
        RECT popup;
        GetWindowRect(g_preview.wnd, &popup);
        if (!PtInRect(&popup, pt) && !PtInRect(&g_preview.buttonRc, pt)) DismissPreview();
    }
    return CallNextHookEx(nullptr, code, wParam, lParam);
}

LRESULT CALLBACK PreviewKeyProc(int code, WPARAM wParam, LPARAM lParam) {
    if (code == HC_ACTION && g_preview.wnd && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)) {
        DWORD vk = ((KBDLLHOOKSTRUCT*)lParam)->vkCode;
        if (vk == VK_ESCAPE && g_settings.previewCloseEsc) {
            DismissPreview();
            return 1;  // as a menu takes its Esc
        }
        bool modifier = (vk >= VK_LSHIFT && vk <= VK_RMENU) || vk == VK_SHIFT || vk == VK_CONTROL || vk == VK_MENU ||
                        vk == VK_LWIN || vk == VK_RWIN;
        if (!modifier && g_settings.previewCloseKey) DismissPreview();
    }
    return CallNextHookEx(nullptr, code, wParam, lParam);
}

void PreviewHooks(bool on) {
    auto const& s = g_settings;
    bool mouse = on && s.previewSticky && s.previewCloseClick;
    bool keys = on && s.previewSticky && (s.previewCloseEsc || s.previewCloseKey);
    if (mouse && !g_previewMouseHook) {
        g_previewMouseHook = SetWindowsHookExW(WH_MOUSE_LL, PreviewMouseProc, g_instance, 0);
    } else if (!mouse && g_previewMouseHook) {
        UnhookWindowsHookEx(g_previewMouseHook);
        g_previewMouseHook = nullptr;
    }
    if (keys && !g_previewKeyHook) {
        g_previewKeyHook = SetWindowsHookExW(WH_KEYBOARD_LL, PreviewKeyProc, g_instance, 0);
    } else if (!keys && g_previewKeyHook) {
        UnhookWindowsHookEx(g_previewKeyHook);
        g_previewKeyHook = nullptr;
    }
}

// The pointer left the previews and their button: they go after a moment, or (kept until
// dismissed) after the time chosen, if any.
void ScheduleHidePreview(HWND bar) {
    auto const& s = g_settings;
    if (!s.previewSticky) SetTimer(bar, kTimerPreviewHide, std::max(1, s.previewLeaveDelay), nullptr);
    else if (s.previewCloseAfter > 0) SetTimer(bar, kTimerPreviewHide, s.previewCloseAfter * 1000, nullptr);
}

// Another window came to the front: previews kept until dismissed go, unless it's one of theirs.
void PreviewActivated(HWND h) {
    if (!g_preview.wnd || !g_settings.previewSticky || !g_settings.previewCloseFocus) return;
    for (auto const& cell : g_preview.cells)
        if (cell.hwnd == h) return;
    DismissPreview();
}

void ClearThumbnails() {
    for (auto& cell : g_preview.cells) {
        if (cell.thumbnail) DwmUnregisterThumbnail(cell.thumbnail);
        cell.thumbnail = nullptr;
    }
}

void HidePreview() {
    for (BarState* bar : g_bars) {
        if (!bar->hwnd) continue;
        KillTimer(bar->hwnd, kTimerPreviewShow);
        KillTimer(bar->hwnd, kTimerPreviewHide);
    }
    LivePeek(nullptr, nullptr, kPeekTaskbar);
    PreviewHooks(false);
    ClearThumbnails();
    if (g_preview.wnd) DestroyWindow(g_preview.wnd);
    g_preview = {};
}

// Peeks at the cell under the pointer once it has rested there, or at once
// when already peeking; none, the peek ends.
void PeekAtCell(int cell) {
    HWND wnd = g_preview.wnd;
    if (!wnd) return;
    KillTimer(wnd, kTimerPeek);
    if (!g_settings.previewPeek || cell < 0 || cell >= (int)g_preview.cells.size()) {
        LivePeek(nullptr, nullptr, kPeekTaskbar);
    } else if (g_peeking) {
        LivePeek(g_preview.cells[cell].hwnd, wnd, kPeekTaskbar);
    } else {
        SetTimer(wnd, kTimerPeek, std::max(1, g_settings.peekDelay), nullptr);
    }
}

// A preview colour: the one set (with its own look), else the scheme's.
uint32_t PreviewRgb(COLORREF set, uint32_t scheme) {
    if (g_settings.previewLook != 1 || set == CLR_INVALID) return scheme;
    return (GetRValue(set) << 16) | (GetGValue(set) << 8) | GetBValue(set);
}

// The previews' background lets something show through: a backdrop, or less than full opacity.
bool PreviewSeeThrough() { return g_settings.previewBackdrop || g_settings.previewOpacity < 100; }

void PaintPreview(HWND wnd, HDC target) {
    RECT client;
    GetClientRect(wnd, &client);
    if (client.right <= 0 || client.bottom <= 0) return;
    // Drawn into a 32-bit picture, so a see-through background keeps its alpha.
    BITMAPINFO bi{};
    bi.bmiHeader = {sizeof(BITMAPINFOHEADER), client.right, -client.bottom, 1, 32, BI_RGB};
    uint32_t* bits = nullptr;
    HDC dc = CreateCompatibleDC(target);
    HBITMAP bitmap = CreateDIBSection(target, &bi, DIB_RGB_COLORS, (void**)&bits, nullptr, 0);
    if (!bitmap) {
        DeleteDC(dc);
        return;
    }
    HGDIOBJ oldBitmap = SelectObject(dc, bitmap);
    auto const& p = g_settings.palette;
    auto const& s = g_settings;
    // The style's previews (TaskbandExtendedUI), as StartAllBack draws them: its background (part 1),
    // a thumbnail's hover and flashing states (part 3: 1 hot, 5 flashing) and its close button (part 12);
    // or the colours set, or the scheme's.
    HTHEME ext = g_themed && s.previewLook == 0 ? g_theme.preview : nullptr;
    bool seeThrough = PreviewSeeThrough();
    uint32_t ground = PreviewRgb(s.previewBg, p.ground);
    uint32_t edge = s.previewBorder != CLR_INVALID
                        ? (GetRValue(s.previewBorder) << 16) | (GetGValue(s.previewBorder) << 8) | GetBValue(s.previewBorder)
                        : p.edge;
    Fill(dc, client, ground);
    size_t count = (size_t)client.right * client.bottom;
    if (seeThrough) {
        // The background marked (alpha 1), to be made see-through once everything is drawn.
        GdiFlush();
        for (size_t i = 0; i < count; i++) bits[i] = (bits[i] & 0xFFFFFF) | 0x01000000;
    }
    if (ext && !seeThrough) {
        DrawThemePartAlpha(dc, ext, 1, 0, client);  // (a backdrop takes the place of the style's background)
    } else if (!ext) {
        int width = s.previewLook == 1 ? Px(s.previewBorderW) : Px(1);
        SetDCBrushColor(dc, MenuRgb(edge));
        for (int i = 0; i < width; i++) {
            RECT r = client;
            InflateRect(&r, -i, -i);
            FrameRect(dc, &r, (HBRUSH)GetStockObject(DC_BRUSH));
        }
    }
    HGDIOBJ oldFont = SelectObject(dc, g_tipFont ? g_tipFont : g_font);
    SetBkMode(dc, TRANSPARENT);
    HWND active = ForegroundRoot();
    for (int i = 0; i < (int)g_preview.cells.size(); i++) {
        auto const& cell = g_preview.cells[i];
        bool hot = i == g_preview.hot;
        Win* w = FindWin(cell.hwnd);
        if (ext) {
            if (w && w->flashing) DrawThemePartAlpha(dc, ext, 3, 5, cell.rc);
            else if (hot) DrawThemePartAlpha(dc, ext, 3, 1, cell.rc);
        } else {
            if (hot) {
                Fill(dc, cell.rc, PreviewRgb(s.previewHover, p.hover));
                SetDCBrushColor(dc, MenuRgb(PreviewRgb(s.previewHoverBorder, p.edge)));
                FrameRect(dc, &cell.rc, (HBRUSH)GetStockObject(DC_BRUSH));
            } else if (w && w->flashing && s.previewLook == 1) {
                Fill(dc, cell.rc, PreviewRgb(s.previewFlashing, p.warn));
            }
            // The active window: a line along the cell's foot.
            if (cell.hwnd == active) {
                RECT mark{cell.rc.left + Px(2), cell.rc.bottom - Px(2), cell.rc.right - Px(2), cell.rc.bottom};
                Fill(dc, mark, PreviewRgb(s.previewActive, p.accent));
            }
        }
        RECT title = cell.title;
        if (g_settings.previewIcons) {
            int icon = Px(16);
            if (w) {
                HICON small = w->icon ? w->icon : AppIcon(*w, icon);
                if (small)
                    DrawIconEx(dc, title.left, (title.top + title.bottom - icon) / 2, small, icon, icon, 0, nullptr,
                               DI_NORMAL);
            }
            title.left += icon + Px(6);
        }
        if (hot) title.right = cell.close.left - Px(4);
        SetTextColor(dc, MenuRgb(PreviewRgb(s.previewTextColor, p.text)));
        std::wstring text = Title(cell.hwnd);
        DrawTextW(dc, text.c_str(), -1, &title, DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS | DT_NOPREFIX);
        // No thumbnail (a window DWM has nothing for): its app's icon.
        if (!IsRectEmpty(&cell.thumb) && !cell.thumbnail && w) {
            int big = Px(32);
            if (HICON app = AppIcon(*w, big))
                DrawIconEx(dc, (cell.thumb.left + cell.thumb.right - big) / 2, (cell.thumb.top + cell.thumb.bottom - big) / 2,
                           app, big, big, 0, nullptr, DI_NORMAL);
        }
        if (hot && ext) {
            DrawThemePartAlpha(dc, ext, 12, g_preview.hotClose ? 2 : 1, cell.close);
        } else if (hot) {
            bool closeHot = g_preview.hotClose;
            if (closeHot) Fill(dc, cell.close, PreviewRgb(s.previewCloseHover, p.bad));
            uint32_t glyph = PreviewRgb(s.previewCloseGlyph, closeHot ? p.ground : p.text);
            // A pixel cross.
            int size = Px(8);
            int x = (cell.close.left + cell.close.right - size) / 2, y = (cell.close.top + cell.close.bottom - size) / 2;
            for (int k = 0; k < size; k += g_scale) {
                RECT a{x + k, y + k, x + k + Px(1), y + k + Px(1)};
                RECT b{x + size - Px(1) - k, y + k, x + size - k, y + k + Px(1)};
                Fill(dc, a, glyph);
                Fill(dc, b, glyph);
            }
        }
    }
    SelectObject(dc, oldFont);
    GdiFlush();
    if (seeThrough) {
        // The background at its opacity (premultiplied); everything drawn over it solid.
        uint32_t a = s.previewOpacity * 255 / 100;
        uint32_t clear = (a << 24) | ((((ground >> 16) & 0xFF) * a / 255) << 16) | ((((ground >> 8) & 0xFF) * a / 255) << 8) |
                         ((ground & 0xFF) * a / 255);
        for (size_t i = 0; i < count; i++)
            bits[i] = (bits[i] >> 24) == 1 && (bits[i] & 0xFFFFFF) == ground ? clear : bits[i] | 0xFF000000;
    } else {
        for (size_t i = 0; i < count; i++) bits[i] |= 0xFF000000;
    }
    BitBlt(target, 0, 0, client.right, client.bottom, dc, 0, 0, SRCCOPY);
    SelectObject(dc, oldBitmap);
    DeleteObject(bitmap);
    DeleteDC(dc);
}

void ShowWindowMenuFor(HWND h);
void CloseFromPreview(int index);

void PreviewFadeFrame();

LRESULT CALLBACK PreviewProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    BarScope barScope(BarOf(g_preview.bar));  // its bar's scale and fonts
    switch (msg) {
        case WM_MOUSEACTIVATE:
            return MA_NOACTIVATE;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC dc = BeginPaint(wnd, &ps);
            PaintPreview(wnd, dc);
            EndPaint(wnd, &ps);
            return 0;
        }
        case WM_ERASEBKGND:
            return 1;
        case WM_TIMER:
            if (wParam == kTimerPreviewFade) {
                if (wnd == g_preview.wnd) PreviewFadeFrame();
                else KillTimer(wnd, kTimerPreviewFade);
            } else if (wParam == kTimerPeek) {
                KillTimer(wnd, kTimerPeek);
                int hot = g_preview.hot;
                if (hot >= 0 && hot < (int)g_preview.cells.size()) LivePeek(g_preview.cells[hot].hwnd, wnd, kPeekTaskbar);
            }
            return 0;
        case WM_MOUSEMOVE: {
            if (!g_preview.tracking) {
                TRACKMOUSEEVENT tme{sizeof(tme), TME_LEAVE, wnd, 0};
                TrackMouseEvent(&tme);
                g_preview.tracking = true;
            }
            if (HWND bar = g_preview.bar) KillTimer(bar, kTimerPreviewHide);
            POINT pt{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            int hot = -1;
            bool hotClose = false;
            for (int i = 0; i < (int)g_preview.cells.size(); i++) {
                if (PtInRect(&g_preview.cells[i].rc, pt)) {
                    hot = i;
                    hotClose = PtInRect(&g_preview.cells[i].close, pt) != 0;
                }
            }
            if (hot != g_preview.hot) PeekAtCell(hot);
            if (hot != g_preview.hot || hotClose != g_preview.hotClose) {
                g_preview.hot = hot;
                g_preview.hotClose = hotClose;
                InvalidateRect(wnd, nullptr, FALSE);
            }
            return 0;
        }
        case WM_MOUSELEAVE:
            g_preview.tracking = false;
            g_preview.hot = -1;
            PeekAtCell(-1);
            InvalidateRect(wnd, nullptr, FALSE);
            if (HWND bar = g_preview.bar) ScheduleHidePreview(bar);
            return 0;
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP: {
            int hot = g_preview.hot;
            if (hot < 0 || hot >= (int)g_preview.cells.size()) return 0;
            HWND h = g_preview.cells[hot].hwnd;
            if (msg == WM_MBUTTONUP || (msg == WM_LBUTTONUP && g_preview.hotClose)) {
                CloseFromPreview(hot);
            } else if (msg == WM_LBUTTONUP) {
                bool keep = g_settings.previewKeepAfterPick;
                if (!keep) HidePreview();
                if (Win* w = FindWin(h)) w->flashing = false;
                ActivateWindow(h);
                if (keep && g_preview.wnd) InvalidateRect(g_preview.wnd, nullptr, FALSE);  // the one in front
            } else {
                ShowWindowMenuFor(h);
            }
            return 0;
        }
    }
    return DefWindowProcW(wnd, msg, wParam, lParam);
}

// What shows through a see-through preview: the window's own client area made see-through (the frame
// extended over it), then the desktop blurred, acrylic (both through the compositor's accent, which works
// whether or not the window is active) or Mica (the system backdrop).
void SetPreviewBackdrop(HWND wnd) {
    MARGINS all{-1, -1, -1, -1};
    DwmExtendFrameIntoClientArea(wnd, &all);
    int backdrop = g_settings.previewBackdrop;
    if (backdrop == 3) {
        uint32_t g = PreviewRgb(g_settings.previewBg, g_settings.palette.ground);
        BOOL dark = ((g >> 16) & 0xFF) * 3 + ((g >> 8) & 0xFF) * 6 + (g & 0xFF) < 128 * 10;
        DwmSetWindowAttribute(wnd, 20 /* DWMWA_USE_IMMERSIVE_DARK_MODE */, &dark, sizeof(dark));
        DWORD type = 2;  // DWMSBT_MAINWINDOW
        DwmSetWindowAttribute(wnd, 38 /* DWMWA_SYSTEMBACKDROP_TYPE */, &type, sizeof(type));
        return;
    }
    if (!backdrop) return;
    struct AccentPolicy {
        int state, flags;
        DWORD color;
        int animation;
    };
    struct CompositionData {
        int attribute;
        void* data;
        SIZE_T size;
    };
    using SetWindowCompositionAttribute_t = BOOL(WINAPI*)(HWND, CompositionData*);
    static auto set =
        (SetWindowCompositionAttribute_t)GetProcAddress(GetModuleHandleW(L"user32.dll"), "SetWindowCompositionAttribute");
    if (!set) return;
    AccentPolicy policy{backdrop == 2 ? 4 : 3, 0, 0x01000000, 0};  // ACRYLICBLURBEHIND, BLURBEHIND; the drawn tint carries the colour
    CompositionData data{19 /* WCA_ACCENT_POLICY */, &policy, sizeof(policy)};
    set(wnd, &data);
}

// Fading in on the motion engine's "fadePopupsIn" line (or a parent's): the thumbnails through their
// own opacity, and an opaque preview's whole window through layering, only while it fades (a
// see-through one's backdrop stays as it is: only its thumbnails fade). No line: up at once, as ever.
double NowMs();

bool PreviewFadeOn() {
    auto const& a = g_settings.motionPreviewIn;
    return a.set && a.on && motion::Ms(a) > 0;
}

BYTE PreviewFadeAlpha() { return g_preview.fading ? (BYTE)std::lround(255 * std::clamp(g_preview.fade, 0.0, 1.0)) : 255; }

void PreviewThumbnailsAlpha() {
    DWM_THUMBNAIL_PROPERTIES props{};
    props.dwFlags = DWM_TNP_OPACITY;
    props.opacity = PreviewFadeAlpha();
    for (auto const& cell : g_preview.cells)
        if (cell.thumbnail) DwmUpdateThumbnailProperties(cell.thumbnail, &props);
}

// In: opaque, and no longer layered (drawn again at once).
void PreviewFadeDone(HWND wnd) {
    KillTimer(wnd, kTimerPreviewFade);
    g_preview.fading = false;
    g_preview.fade = 1;
    if (g_preview.layered) {
        g_preview.layered = false;
        SetWindowLongPtrW(wnd, GWL_EXSTYLE, GetWindowLongPtrW(wnd, GWL_EXSTYLE) & ~(LONG_PTR)WS_EX_LAYERED);
        RedrawWindow(wnd, nullptr, nullptr, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_ALLCHILDREN | RDW_UPDATENOW);
    }
    PreviewThumbnailsAlpha();
}

void PreviewFadeFrame() {
    HWND wnd = g_preview.wnd;
    if (!wnd || !g_preview.fading) return;
    auto const& a = g_settings.motionPreviewIn;
    double ms = std::min(motion::Ms(a), 1500.0);
    double t = ms > 0 && PreviewFadeOn() ? (NowMs() - g_preview.fadeStart) / ms : 1;
    if (t >= 1) {
        PreviewFadeDone(wnd);
        return;
    }
    g_preview.fade = motion::Ease(a, t);
    if (g_preview.layered) SetLayeredWindowAttributes(wnd, 0, PreviewFadeAlpha(), LWA_ALPHA);
    PreviewThumbnailsAlpha();
}

// A preview window just made, before it shows: it starts to fade in.
void PreviewFadeIn(HWND wnd) {
    if (!PreviewFadeOn()) return;
    g_preview.fading = true;
    g_preview.fade = 0;
    g_preview.fadeStart = NowMs();
    if (!PreviewSeeThrough()) {
        SetWindowLongPtrW(wnd, GWL_EXSTYLE, GetWindowLongPtrW(wnd, GWL_EXSTYLE) | WS_EX_LAYERED);
        g_preview.layered = SetLayeredWindowAttributes(wnd, 0, 0, LWA_ALPHA) != 0;
        if (!g_preview.layered) SetWindowLongPtrW(wnd, GWL_EXSTYLE, GetWindowLongPtrW(wnd, GWL_EXSTYLE) & ~(LONG_PTR)WS_EX_LAYERED);
    }
    if (!SetTimer(wnd, kTimerPreviewFade, USER_TIMER_MINIMUM, nullptr)) PreviewFadeDone(wnd);  // (no frames: up at once)
}

void ShowPreview(HWND bar, int index) {
    KillTimer(bar, kTimerPreviewShow);
    KillTimer(bar, kTimerPreviewHide);
    if (index < 0 || index >= (int)g_buttons.size() || g_buttons[index].windows.empty()) {
        HidePreview();
        return;
    }
    if (g_preview.wnd && g_preview.bar != bar) HidePreview();  // from another screen's bar
    if (g_tooltip) SendMessageW(g_tooltip, TTM_POP, 0, 0);  // the preview takes over from a tooltip
    ClearThumbnails();
    // Placed on the pointer: where it was when they opened, kept while they only follow their windows.
    if (!g_preview.wnd || g_preview.button != index) GetCursorPos(&g_preview.at);
    Button const& b = g_buttons[index];
    g_preview.bar = bar;
    g_preview.button = index;
    g_preview.hot = -1;
    g_preview.cells.clear();
    for (HWND h : b.windows) g_preview.cells.push_back({h});
    int n = (int)g_preview.cells.size();

    RECT barRc;
    GetWindowRect(bar, &barRc);
    RECT anchor = b.rc;
    OffsetRect(&anchor, barRc.left, barRc.top);
    g_preview.buttonRc = anchor;
    MONITORINFO mi{sizeof(mi)};
    GetMonitorInfoW(MonitorFromRect(&anchor, MONITOR_DEFAULTTONEAREST), &mi);
    RECT screen = mi.rcMonitor;
    bool vertical = Vertical();

    int gap = Px(g_settings.previewEdge), pad = Px(g_settings.previewPad), titleH = Px(20), room = Px(16);
    int thumbW = Px(g_settings.previewThumbW), thumbH = Px(g_settings.previewThumbH);
    int cellW = thumbW + 2 * pad, cellH = pad + titleH + Px(6) + thumbH + pad;
    int width = vertical ? cellW + 2 * gap : n * cellW + (n + 1) * gap;
    int height = vertical ? n * cellH + (n + 1) * gap : cellH + 2 * gap;
    g_preview.list = vertical ? height > screen.bottom - screen.top - room : width > screen.right - screen.left - room;
    if (g_preview.list) {
        int rowH = Px(28);
        width = Px(320);
        height = std::min(n * rowH + 2 * gap, (int)(screen.bottom - screen.top) - room);
        for (int i = 0; i < n; i++) {
            auto& cell = g_preview.cells[i];
            cell.rc = {gap, gap + i * rowH, width - gap, gap + (i + 1) * rowH};
            cell.close = {cell.rc.right - rowH, cell.rc.top, cell.rc.right, cell.rc.bottom};
            InflateRect(&cell.close, -Px(4), -Px(4));
            cell.title = {cell.rc.left + pad, cell.rc.top, cell.rc.right - pad, cell.rc.bottom};
        }
    } else {
        for (int i = 0; i < n; i++) {
            auto& cell = g_preview.cells[i];
            int left = vertical ? gap : gap + i * (cellW + gap);
            int top = vertical ? gap + i * (cellH + gap) : gap;
            cell.rc = {left, top, left + cellW, top + cellH};
            cell.title = {left + pad, top + pad, left + cellW - pad, top + pad + titleH};
            cell.close = {cell.title.right - titleH, cell.title.top, cell.title.right, cell.title.bottom};
            cell.thumb = {left + pad, cell.title.bottom + Px(6), left + cellW - pad, top + cellH - pad};
        }
    }

    // Beside the bar, centred on the button, kept on the screen.
    int x = (anchor.left + anchor.right - width) / 2, y = (anchor.top + anchor.bottom - height) / 2;
    switch (g_settings.edge) {
        case Edge::Top: y = barRc.bottom + Px(4); break;
        case Edge::Bottom: y = barRc.top - height - Px(4); break;
        case Edge::Left: x = barRc.right + Px(4); break;
        case Edge::Right: x = barRc.left - width - Px(4); break;
    }
    if (g_settings.previewPlace == 1) {
        if (vertical) y = g_preview.at.y - height / 2;
        else x = g_preview.at.x - width / 2;
    }
    int away = Px(g_settings.previewAway), along = Px(g_settings.previewAlong);
    switch (g_settings.edge) {
        case Edge::Bottom: y -= away; x += along; break;
        case Edge::Top: y += away; x += along; break;
        case Edge::Left: x += away; y += along; break;
        case Edge::Right: x -= away; y += along; break;
    }
    x = std::clamp(x, (int)screen.left + Px(4), std::max((int)screen.left + Px(4), (int)screen.right - width - Px(4)));
    y = std::clamp(y, (int)screen.top + Px(4), std::max((int)screen.top + Px(4), (int)screen.bottom - height - Px(4)));

    if (!g_preview.wnd) {
        auto const& s = g_settings;
        g_preview.wnd = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE,
                                        s.previewShadow ? kPreviewShadowClass : kPreviewClass, L"", WS_POPUP, x, y, width,
                                        height, bar, nullptr, g_instance, nullptr);
        if (!g_preview.wnd) return;
        KeepOutOfPeek(g_preview.wnd);
        DWORD corners = s.previewCorners == 2 ? 2 : s.previewCorners == 1 ? 3 : 1;  // DWMWCP_ROUND, ROUNDSMALL, DONOTROUND
        DwmSetWindowAttribute(g_preview.wnd, 33 /* DWMWA_WINDOW_CORNER_PREFERENCE */, &corners, sizeof(corners));
        COLORREF border = s.previewBorderNone               ? 0xFFFFFFFE  // DWMWA_COLOR_NONE
                          : s.previewBorder != CLR_INVALID ? s.previewBorder
                                                           : MenuRgb(s.palette.edge);
        DwmSetWindowAttribute(g_preview.wnd, 34 /* DWMWA_BORDER_COLOR */, &border, sizeof(border));
        if (PreviewSeeThrough()) SetPreviewBackdrop(g_preview.wnd);
        BOOL noFade = TRUE;  // no DWM fade: the thumbnails are up as soon as it is
        DwmSetWindowAttribute(g_preview.wnd, 3 /* DWMWA_TRANSITIONS_FORCEDISABLED */, &noFade, sizeof(noFade));
        PreviewFadeIn(g_preview.wnd);
    }
    SetWindowPos(g_preview.wnd, HWND_TOPMOST, x, y, width, height, SWP_NOACTIVATE | SWP_SHOWWINDOW);
    PreviewHooks(true);

    // The thumbnails, fitted to their cells.
    for (auto& cell : g_preview.cells) {
        if (IsRectEmpty(&cell.thumb) || FAILED(DwmRegisterThumbnail(g_preview.wnd, cell.hwnd, &cell.thumbnail))) {
            cell.thumbnail = nullptr;
            continue;
        }
        SIZE source{};
        DwmQueryThumbnailSourceSize(cell.thumbnail, &source);
        if (source.cx < Px(40) || source.cy < Px(40)) {
            DwmUnregisterThumbnail(cell.thumbnail);
            cell.thumbnail = nullptr;
            continue;
        }
        int areaW = cell.thumb.right - cell.thumb.left, areaH = cell.thumb.bottom - cell.thumb.top;
        double scale = std::min({(double)areaW / source.cx, (double)areaH / source.cy, 1.0});
        int w = (int)(source.cx * scale), h = (int)(source.cy * scale);
        RECT dest{cell.thumb.left + (areaW - w) / 2, cell.thumb.top + (areaH - h) / 2, 0, 0};
        dest.right = dest.left + w;
        dest.bottom = dest.top + h;
        DWM_THUMBNAIL_PROPERTIES props{};
        props.dwFlags = DWM_TNP_RECTDESTINATION | DWM_TNP_VISIBLE | DWM_TNP_OPACITY | DWM_TNP_SOURCECLIENTAREAONLY;
        props.rcDestination = dest;
        props.fVisible = TRUE;
        props.opacity = PreviewFadeAlpha();  // (255 unless it's fading in)
        props.fSourceClientAreaOnly = FALSE;
        DwmUpdateThumbnailProperties(cell.thumbnail, &props);
    }
    InvalidateRect(g_preview.wnd, nullptr, FALSE);
}

void CloseFromPreview(int index) {
    if (index < 0 || index >= (int)g_preview.cells.size()) return;
    PostMessageW(g_preview.cells[index].hwnd, WM_SYSCOMMAND, SC_CLOSE, 0);
    // The popup follows once the window has gone (see SyncPreview).
}

void ShowWindowMenuFor(HWND h) {
    HWND bar = g_preview.bar ? g_preview.bar : (HWND)g_wnd;
    HidePreview();
    Button one;
    one.windows = {h};
    if (bar) WindowMenu(bar, one);
}

// The pointer moved to another part of the bar.
void PreviewHover(HWND bar, int hot) {
    if (!g_settings.previews || g_inMenu) return;
    bool running = hot >= 0 && hot < (int)g_buttons.size() && !g_buttons[hot].windows.empty();
    if (g_preview.wnd) {
        if (running) {
            KillTimer(bar, kTimerPreviewHide);
            if (hot != g_preview.button || g_preview.bar != bar) ShowPreview(bar, hot);
        } else {
            ScheduleHidePreview(bar);
        }
        return;
    }
    // Opened on resting (for any button, or groups only), or only by a click.
    int open = g_settings.previewOpen;
    running = running && (open == 0 || (open == 1 && g_buttons[hot].windows.size() > 1));
    if (running) SetTimer(bar, kTimerPreviewShow, std::max(1, g_settings.previewDelay), nullptr);
    else KillTimer(bar, kTimerPreviewShow);
}

// After the windows were scanned: the popup follows its windows (one closed,
// one opened, the button moved), or goes when they're all gone.
void SyncPreview(HWND bar) {
    if (!g_preview.wnd || g_preview.bar != bar) return;
    std::vector<HWND> shown;
    for (auto const& cell : g_preview.cells) shown.push_back(cell.hwnd);
    for (int i = 0; i < (int)g_buttons.size(); i++) {
        auto const& windows = g_buttons[i].windows;
        bool mine = std::any_of(shown.begin(), shown.end(), [&](HWND h) {
            return std::find(windows.begin(), windows.end(), h) != windows.end();
        });
        if (!mine) continue;
        if (windows != shown) {
            ShowPreview(bar, i);
        } else {
            g_preview.button = i;
            InvalidateRect(g_preview.wnd, nullptr, FALSE);  // titles change
        }
        return;
    }
    HidePreview();
}

////////////////////////////////////////////////////////////////////////////////
// Tray icons: clicks go to their programs as Explorer's tray would send them,
// and the Tourne flyouts open next to the icon clicked here.

// System Icons reads this (see TakeTaskbarAnchor there): Explorer's tray is
// all Shell_NotifyIconGetRect knows about.
void LeaveAnchor(HWND owner, RECT const& cell) {
    DWORD pid = 0;
    GetWindowThreadProcessId(owner, &pid);
    if (pid != GetCurrentProcessId()) return;
    RECT r = cell;
    MapWindowPoints(g_curWnd ? g_curWnd : (HWND)g_wnd, nullptr, (POINT*)&r, 2);  // the bar clicked, on whichever screen
    uint64_t packed = (uint64_t)(uint16_t)r.left | (uint64_t)(uint16_t)r.top << 16 | (uint64_t)(uint16_t)r.right << 32 |
                      (uint64_t)(uint16_t)r.bottom << 48;
    SetPropW(owner, L"TourneTaskbar_Anchor", (HANDLE)(ULONG_PTR)packed);
    SetPropW(owner, L"TourneTaskbar_AnchorTick", (HANDLE)(ULONG_PTR)GetTickCount());
}

// System Icons' list of programs that open on a double click.
constexpr PCWSTR kIconsDoubleClickApps = L"systemIcons.doubleClickApps";

bool OpensOnDoubleClick(HWND owner) {
    PCWSTR list = Wh_GetStringSetting(kIconsDoubleClickApps);
    bool result = list && OpensOnDoubleClickIn(owner, list);
    Wh_FreeStringSetting(list);
    return result;
}

// The menu or pop-up a program opened from its tray icon, if one is still
// up: a menu window of its process, or a captionless pop-up of it in front.
HWND OpenTrayPopup(HWND owner) {
    struct Find {
        DWORD pid;
        HWND found;
    } find{0, nullptr};
    GetWindowThreadProcessId(owner, &find.pid);
    EnumWindows(
        [](HWND h, LPARAM param) __stdcall -> BOOL {
            auto* f = (Find*)param;
            DWORD pid = 0;
            GetWindowThreadProcessId(h, &pid);
            WCHAR cls[16];
            if (pid == f->pid && IsWindowVisible(h) && GetClassNameW(h, cls, ARRAYSIZE(cls)) && !wcscmp(cls, L"#32768")) {
                f->found = h;
                return FALSE;
            }
            return TRUE;
        },
        (LPARAM)&find);
    if (find.found) return find.found;
    HWND front = GetForegroundWindow();
    DWORD pid = 0;
    GetWindowThreadProcessId(front, &pid);
    LONG style = front ? GetWindowLongW(front, GWL_STYLE) : 0;
    return front && pid == find.pid && (style & WS_POPUP) && (style & WS_CAPTION) != WS_CAPTION ? front : nullptr;
}

// The last icon clicked and how, so the next click on it can close what the
// first opened: menus and pop-ups toggle, as in Windows 7's tray.
std::wstring g_lastTrayKey;
bool g_lastTrayRight = false;

// One of System Icons' own icons: its kind (volume, brightness...), which
// System Icons' scroll and middle click messages carry, plus one.
int SystemIconKind(HiddenIcon const& icon) {
    constexpr UINT kIconIdBase = 0x7E00, kIconCount = 64;
    WCHAR cls[64];
    if (icon.id < kIconIdBase || icon.id >= kIconIdBase + kIconCount || !GetClassNameW(icon.owner, cls, ARRAYSIZE(cls)) ||
        wcsncmp(cls, L"TourneTraySystemIcons_", 22) != 0)
        return 0;
    return (int)(icon.id - kIconIdBase) + 1;
}

bool PhoneFromTray(RECT const& cell, bool asked = false);  // start-menu.cpp: the phone's flyout opened by the tray's phone

void ClickTrayIcon(int index, int click) {
    if (index < 0 || index >= (int)g_tray.size() || !IsWindow(g_tray[index].icon.owner)) return;
    TrayItem const& item = g_tray[index];
    // The arrow points the other way while the flyout it opened is up (the style's "Open" arrow).
    if (IsOverflowArrow(item) && click == kClickLeft) {
        g_arrowClicked = true;
        if (HWND bar = g_wnd) SetTimer(bar, kTimerTrayAgain, 150, nullptr);
    }
    // Tray icons never get middle clicks; System Icons takes them its own way.
    if (click == kClickMiddle) {
        if (int kind = SystemIconKind(item.icon))
            PostMessageW(item.icon.owner, RegisterWindowMessageW(L"TourneSuite_RelayMiddle"), 0, kind);
        else
            ForwardHiddenClick(item.icon, click);
        return;
    }
    bool right = click == kClickRight;
    if (click != kClickSecondOfDouble && click != kClickMiddle && item.key == g_lastTrayKey && right == g_lastTrayRight) {
        if (HWND popup = OpenTrayPopup(item.icon.owner)) {
            // A menu ends with its owner's cancel; a pop-up closes on Escape.
            PostMessageW(item.icon.owner, WM_CANCELMODE, 0, 0);
            PostMessageW(popup, WM_KEYDOWN, VK_ESCAPE, 0x00010001);
            PostMessageW(popup, WM_KEYUP, VK_ESCAPE, 0xC0010001);
            g_lastTrayKey.clear();
            return;
        }
    }
    if (click != kClickSecondOfDouble) {
        g_lastTrayKey = item.key;
        g_lastTrayRight = right;
    }
    // Phone Link's own icon: Tourne's phone as its flyout (startMenu.phoneTray), else Phone Link as usual.
    if (click == kClickLeft && !_wcsicmp(PathFindFileNameW(OwnerFileName(item.icon.owner).c_str()), L"PhoneExperienceHost.exe")) {
        RECT cell = item.rc;
        MapWindowPoints(g_curWnd ? g_curWnd : (HWND)g_wnd, nullptr, (POINT*)&cell, 2);
        if (PhoneFromTray(cell)) return;
    }
    LeaveAnchor(item.icon.owner, item.rc);
    ForwardHiddenClick(item.icon, click);
}

// The pointer arriving over an icon, which some programs answer.
void HoverTrayIcon(int index) {
    if (index < 0 || index >= (int)g_tray.size()) return;
    HoverHiddenIcon(g_tray[index].icon);
}

// The tray's order as it is now, with icons not in the tray keeping their
// places for when they come back.
void SaveTrayOrder() {
    std::vector<std::wstring> keys = g_trayVisual;  // as laid out, plugin items in their places among the icons
    for (auto const& item : g_tray)
        if (std::find(keys.begin(), keys.end(), item.key) == keys.end()) keys.push_back(item.key);
    for (auto const& key : LoadTrayList(kTrayOrderValue))
        if (std::find(keys.begin(), keys.end(), key) == keys.end()) keys.push_back(key);
    SaveTrayList(kTrayOrderValue, keys);
}

void PlaceTrayIcon(HWND wnd, std::wstring const& key, int place) {
    TrayPlaces places = TrayPlaces::Load();
    places.Set(key, place);
    BuildTray();
    Layout(wnd);
    Redraw(wnd);
}

////////////////////////////////////////////////////////////////////////////////
// Dragging tray icons: along the tray to move them, onto System Icons' "show
// hidden icons" arrow or its open flyout to put them in the overflow; and
// icons dragged out of that flyout land here (see tray-shared.cpp).

POINT g_trayDragStart{};
bool g_trayDragMoved = false;
bool g_trayDragOnArrow = false;   // over the overflow arrow, where a drop hides it
DragGhost g_trayGhost;
UINT g_msgTrayDragOver = 0;

bool IsOverflowArrow(TrayItem const& item) {
    constexpr int kOverflowKind = 17 + 1;  // System Icons' kOverflow, as SystemIconKind counts
    return SystemIconKind(item.icon) == kOverflowKind;
}

// Where an icon dropped at a point would go among the tray's icons.
int TrayInsertIndex(POINT pt) {
    bool vertical = Vertical();
    int i = 0;
    for (; i < (int)g_tray.size(); i++) {
        RECT const& r = g_tray[i].rc;
        if ((vertical ? pt.y : pt.x) < (vertical ? (r.top + r.bottom) / 2 : (r.left + r.right) / 2)) break;
    }
    return i;
}

void DragTrayIcon(HWND wnd, POINT pt) {
    if (g_trayDrag < 0 || g_trayDrag >= (int)g_tray.size()) return;
    RECT client;
    GetClientRect(wnd, &client);
    POINT screen = pt;
    ClientToScreen(wnd, &screen);
    if (!PtInRect(&client, pt)) {
        // Off the bar it follows the pointer, toward the overflow flyout.
        g_trayDragOnArrow = false;
        g_hot = g_pressed = -2;
        g_trayGhost.Move(g_tray[g_trayDrag].icon, screen);
        Redraw(wnd);
        return;
    }
    g_trayGhost.Hide();
    int at = HitTest(pt) - kHitTray;
    bool onIcon = at >= 0 && at < (int)g_tray.size() && at != g_trayDrag;
    bool wasOnArrow = g_trayDragOnArrow;
    g_trayDragOnArrow = onIcon && IsOverflowArrow(g_tray[at]);
    // Resting on the arrow opens the overflow, to drop the icon in it.
    if (g_trayDragOnArrow && !wasOnArrow) SetTimer(wnd, kTimerDragOpen, 450, nullptr);
    else if (!g_trayDragOnArrow && wasOnArrow) KillTimer(wnd, kTimerDragOpen);
    if (onIcon && !g_trayDragOnArrow) {
        // The others make room: it takes the place of the icon it's over.
        TrayItem item = std::move(g_tray[g_trayDrag]);
        g_tray.erase(g_tray.begin() + g_trayDrag);
        g_tray.insert(g_tray.begin() + at, std::move(item));
        g_trayDrag = at;
        Layout(wnd);
    }
    g_hot = g_pressed = kHitTray + (g_trayDragOnArrow ? at : g_trayDrag);
    Redraw(wnd);
}

// Dragging a plugin item along the tray (a horizontal bar's): it takes its place among the icons as it
// goes, and keeps it in the tray's saved order when let go on the bar. As a tray icon does, it goes into
// the overflow when let go on System Icons' arrow (resting there opens the flyout) or on the open flyout.
void DragPluginItem(HWND wnd, POINT pt) {
    if (g_pluginDrag < 0 || g_pluginDrag >= (int)g_pluginItems.size()) return;
    RECT client;
    GetClientRect(wnd, &client);
    if (!PtInRect(&client, pt)) {
        // Off the bar it follows the pointer, toward the overflow flyout.
        if (g_trayDragOnArrow) KillTimer(wnd, kTimerDragOpen);
        g_trayDragOnArrow = false;
        static HiddenIcon ghost;
        if (!g_trayGhost.wnd) {
            ghost.size = Px(16);
            ghost.pixels = PluginItemPixels(g_pluginItems[g_pluginDrag], ghost.size, PaletteRef(g_settings.palette.text));
        }
        POINT screen = pt;
        ClientToScreen(wnd, &screen);
        g_trayGhost.Move(ghost, screen);
        g_hot = g_pressed = -2;
        Redraw(wnd);
        return;
    }
    g_trayGhost.Hide();
    int at = HitTest(pt) - kHitTray;
    bool wasOnArrow = g_trayDragOnArrow;
    g_trayDragOnArrow = at >= 0 && at < (int)g_tray.size() && IsOverflowArrow(g_tray[at]);
    if (g_trayDragOnArrow && !wasOnArrow) SetTimer(wnd, kTimerDragOpen, 450, nullptr);
    else if (!g_trayDragOnArrow && wasOnArrow) KillTimer(wnd, kTimerDragOpen);
    g_hot = g_pressed = g_trayDragOnArrow ? kHitTray + at : kHitPlugin + g_pluginDrag;
    if (g_trayDragOnArrow || wasOnArrow) Redraw(wnd);
    if (g_trayDragOnArrow) return;
    int slot = 0;
    for (; slot < (int)g_tray.size(); slot++)
        if (pt.x < (g_tray[slot].rc.left + g_tray[slot].rc.right) / 2) break;
    if (slot == g_pluginDragSlot) return;
    g_pluginDragSlot = slot;
    Layout(wnd);
    Redraw(wnd);
}

void EndPluginDrag(HWND wnd, bool keep) {
    if (keep) SaveTrayOrder();  // (as laid out now, with the item in its new place)
    KillTimer(wnd, kTimerDragOpen);
    g_trayDragOnArrow = false;
    g_trayGhost.Hide();
    g_pluginDrag = -1;
    g_pluginDragMoved = false;
    g_pluginDragSlot = -1;
    g_pressed = -2;
    Layout(wnd);
    Redraw(wnd);
}

// The overflow flyout, if it's open.
HWND OpenOverflowFlyout() {
    HWND found = nullptr;
    EnumWindows(
        [](HWND h, LPARAM param) __stdcall -> BOOL {
            WCHAR cls[64];
            if (IsWindowVisible(h) && GetClassNameW(h, cls, ARRAYSIZE(cls)) &&
                wcsncmp(cls, kFlyoutClassPrefix, wcslen(kFlyoutClassPrefix)) == 0) {
                *(HWND*)param = h;
                return FALSE;
            }
            return TRUE;
        },
        (LPARAM)&found);
    return found;
}

void OpenOverflowWhileDragging(HWND wnd) {
    KillTimer(wnd, kTimerDragOpen);
    if ((g_trayDrag < 0 && g_pluginDrag < 0) || !g_trayDragOnArrow || OpenOverflowFlyout()) return;
    for (int i = 0; i < (int)g_tray.size(); i++)
        if (IsOverflowArrow(g_tray[i])) return ClickTrayIcon(i, kClickLeft);
}

void CancelTrayDrag(HWND wnd) {
    KillTimer(wnd, kTimerDragOpen);
    g_trayDrag = -1;
    g_trayDragMoved = false;
    g_trayDragOnArrow = false;
    g_trayGhost.Hide();
    g_pressed = -2;
    BuildTray();  // back in the saved order
    Layout(wnd);
    Redraw(wnd);
}

void DropTrayIcon(HWND wnd, int index, POINT pt) {
    KillTimer(wnd, kTimerDragOpen);
    bool onArrow = g_trayDragOnArrow;
    g_trayDragOnArrow = false;
    g_trayGhost.Hide();
    g_pressed = -2;
    if (index < 0 || index >= (int)g_tray.size()) return CancelTrayDrag(wnd);
    std::wstring key = g_tray[index].key;
    RECT client;
    GetClientRect(wnd, &client);
    POINT screen = pt;
    ClientToScreen(wnd, &screen);
    if (onArrow) return PlaceTrayIcon(wnd, key, kPlaceOverflow);
    if (PtInRect(&client, pt)) {
        SaveTrayOrder();
        BuildTray();
        Layout(wnd);
        Redraw(wnd);
        return;
    }
    if (HWND flyout = DropTargetAt(screen, kFlyoutClassPrefix)) {
        if (SendTrayDrop(flyout, wnd, screen, key)) {
            BuildTray();
            Layout(wnd);
            Redraw(wnd);
            return;
        }
    }
    CancelTrayDrag(wnd);  // dropped anywhere else: back where it was
}

// Dragging a task button (pinned or running) along the bar: it takes the
// place of the button it's over, and the order is kept when it's dropped.
int g_buttonDrag = -1;  // pressed on a button: its index
std::wstring g_buttonDragKey;
POINT g_buttonDragStart{};
bool g_buttonDragMoved = false;

int ButtonIndexOf(std::wstring const& key) {
    for (int i = 0; i < (int)g_buttons.size(); i++)
        if (ButtonKey(g_buttons[i]) == key) return i;
    return -1;
}

void DragButton(HWND wnd, POINT pt) {
    int at = HitTest(pt);
    if (at >= 0 && at < (int)g_buttons.size() && ButtonKey(g_buttons[at]) != g_buttonDragKey) {
        std::wstring target = ButtonKey(g_buttons[at]);
        std::vector<std::wstring> keys;
        for (auto const& b : g_buttons) {
            std::wstring key = ButtonKey(b);
            if (!key.empty() && key != g_buttonDragKey && std::find(keys.begin(), keys.end(), key) == keys.end())
                keys.push_back(key);
        }
        // Before the button it's over when moving back, after it when moving on.
        auto it = std::find(keys.begin(), keys.end(), target);
        if (at > ButtonIndexOf(g_buttonDragKey) && it != keys.end()) ++it;
        keys.insert(it, g_buttonDragKey);
        for (auto const& other : g_buttonOrder)
            if (std::find(keys.begin(), keys.end(), other) == keys.end()) keys.push_back(other);
        g_buttonOrder = std::move(keys);
        Layout(wnd);
    }
    g_hot = g_pressed = ButtonIndexOf(g_buttonDragKey);
    Redraw(wnd);
}

void EndButtonDrag(HWND wnd, bool keep) {
    g_buttonDrag = -1;
    g_buttonDragMoved = false;
    g_pressed = -2;
    if (keep) {
        SaveTrayList(kButtonOrderValue, g_buttonOrder);
    } else {
        g_buttonOrderLoaded = false;  // back to the saved order
    }
    Layout(wnd);
    Redraw(wnd);
}

// An icon dragged out of System Icons' flyout onto the bar: into the tray,
// where it was dropped.
bool DropOnTray(HWND wnd, POINT screen, std::wstring const& key) {
    POINT pt = screen;
    ScreenToClient(wnd, &pt);
    int at = TrayInsertIndex(pt);
    std::vector<std::wstring> keys;
    for (auto const& item : g_tray)
        if (item.key != key) keys.push_back(item.key);
    keys.insert(keys.begin() + std::min(at, (int)keys.size()), key);
    for (auto const& other : LoadTrayList(kTrayOrderValue))
        if (std::find(keys.begin(), keys.end(), other) == keys.end()) keys.push_back(other);
    SaveTrayList(kTrayOrderValue, keys);
    g_trayDropAt = -1;
    PlaceTrayIcon(wnd, key, kPlaceTray);
    return true;
}

std::wstring TrayLabel(HiddenIcon const& icon) {
    std::wstring label = icon.tip.substr(0, icon.tip.find(L'\n'));
    if (label.empty()) label = OwnerFileName(icon.owner);
    if (label.size() > 48) label = label.substr(0, 47) + L"\x2026";
    std::wstring escaped;
    for (WCHAR ch : label) {
        if (ch == L'&') escaped += L'&';
        escaped += ch;
    }
    return escaped;
}

// Where an icon goes: its choices, as a menu or a submenu.
void AppendPlaceItems(HMENU menu, int base, int place) {
    AppendMenuW(menu, MF_STRING | (place == kPlaceTray ? MF_CHECKED : 0), base + kPlaceTray, L"Show in the tray");
    AppendMenuW(menu, MF_STRING | (place == kPlaceOverflow ? MF_CHECKED : 0), base + kPlaceOverflow,
                L"Move to the overflow");
    AppendMenuW(menu, MF_STRING | (place == kPlaceHidden ? MF_CHECKED : 0), base + kPlaceHidden, L"Hide");
    AppendMenuW(menu, MF_STRING | (place == kPlaceDefault ? MF_CHECKED : 0), base + kPlaceDefault,
                L"Follow Explorer's tray");
}

// Shift+right-click on an icon.
void TrayIconMenu(HWND wnd, int index) {
    if (index < 0 || index >= (int)g_tray.size()) return;
    std::wstring key = g_tray[index].key;
    HMENU menu = CreatePopupMenu();
    AppendMenuW(menu, MF_STRING | MF_GRAYED, 0, TrayLabel(g_tray[index].icon).c_str());
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendPlaceItems(menu, kCmdPlace, TrayPlaces::Load().Of(key));
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    bool vertical = Vertical();
    AppendMenuW(menu, MF_STRING | (index > 0 ? 0 : MF_GRAYED), kCmdMoveLeft, vertical ? L"Move up" : L"Move left");
    AppendMenuW(menu, MF_STRING | (index + 1 < (int)g_tray.size() ? 0 : MF_GRAYED), kCmdMoveRight,
                vertical ? L"Move down" : L"Move right");
    int cmd = RunMenu(wnd, menu);
    if (cmd == kCmdMoveLeft || cmd == kCmdMoveRight) {
        int other = index + (cmd == kCmdMoveLeft ? -1 : 1);
        if (other < 0 || other >= (int)g_tray.size()) return;
        std::swap(g_tray[index], g_tray[other]);
        SaveTrayOrder();
        Layout(wnd);
        Redraw(wnd);
    } else if (cmd >= kCmdPlace && cmd <= kCmdPlace + kPlaceHidden) {
        PlaceTrayIcon(wnd, key, cmd - kCmdPlace);
    }
}

bool HasExtension(std::wstring const& name, std::initializer_list<PCWSTR> extensions);

// The user's own icon for a plugin item: a picture (.ico, .png, .svg...), or a program or library and then
// one of its icons ("file,index"). Empty when nothing was picked.
std::wstring PickPluginIcon(HWND wnd) {
    IFileOpenDialog* dialog = nullptr;
    if (FAILED(CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&dialog)))) return L"";
    COMDLG_FILTERSPEC types[] = {{L"Icons and pictures", L"*.ico;*.png;*.svg;*.jpg;*.jpeg;*.bmp;*.gif;*.exe;*.dll;*.icl;*.cpl"},
                                 {L"All files", L"*.*"}};
    dialog->SetFileTypes(2, types);
    dialog->SetTitle(L"An icon for this item");
    DWORD options = 0;
    dialog->GetOptions(&options);
    dialog->SetOptions(options | FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST);
    std::wstring path;
    IShellItem* item = nullptr;
    LPWSTR chosen = nullptr;
    if (SUCCEEDED(dialog->Show(wnd)) && SUCCEEDED(dialog->GetResult(&item)) && SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &chosen))) {
        path = chosen;
        CoTaskMemFree(chosen);
    }
    if (item) item->Release();
    dialog->Release();
    if (path.empty() || !HasExtension(path, {L".exe", L".dll", L".icl", L".cpl", L".mun"})) return path;
    // A program or library: which of its icons, in Windows' own picker.
    WCHAR file[MAX_PATH];
    if (path.size() >= MAX_PATH) return L"";
    wcscpy_s(file, path.c_str());
    int index = 0;
    if (!PickIconDlg(wnd, file, ARRAYSIZE(file), &index)) return L"";
    return Expand(file) + L"," + std::to_wstring(index);
}

// Tells the plugin runtime the user's icons changed: it draws them again and sends the items back.
void PluginIconsChanged(HWND wnd) {
    if (HWND host = PluginHost()) PostMessageW(host, g_msgPluginsAsk, (WPARAM)wnd, kPluginAskIconsChanged);
}

// Shift+right-click on a plugin item: where it goes (tray, overflow or hidden; a hidden one comes back from the
// bar menu's tray icons list), as for a tray icon, and its icon. (Its own right-click
// menu, if it has one, is a plain right-click.)
void PluginItemMenu(HWND wnd, int index) {
    if (index < 0 || index >= (int)g_pluginItems.size()) return;
    PluginBarItem const& item = g_pluginItems[index];
    std::wstring key = L"plugin:" + item.key;
    std::wstring label = item.tip.substr(0, item.tip.find_first_of(L"\r\n"));
    if (label.empty()) label = item.text;
    if (label.empty()) label = item.key;
    if (label.size() > 48) label = label.substr(0, 47) + L"\x2026";
    for (size_t at = 0; (at = label.find(L'&', at)) != std::wstring::npos; at += 2) label.insert(at, 1, L'&');
    auto icons = LoadPluginIcons();
    bool own = std::any_of(icons.begin(), icons.end(), [&](auto const& row) { return row.first == key; });
    int place = TrayPlaces::Load().Of(key);
    HMENU menu = CreatePopupMenu();
    AppendMenuW(menu, MF_STRING | MF_GRAYED, 0, label.c_str());
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING | (place == kPlaceDefault || place == kPlaceTray ? MF_CHECKED : 0), kCmdPlace + kPlaceTray, L"Show in the tray");
    AppendMenuW(menu, MF_STRING | (place == kPlaceOverflow ? MF_CHECKED : 0), kCmdPlace + kPlaceOverflow, L"Move to the overflow");
    AppendMenuW(menu, MF_STRING | (place == kPlaceHidden ? MF_CHECKED : 0), kCmdPlace + kPlaceHidden, L"Hide");
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING, kCmdPluginIcon, L"Change icon...");
    AppendMenuW(menu, MF_STRING | (own ? 0 : MF_GRAYED), kCmdPluginIconBack, L"Put back original icon");
    int cmd = RunMenu(wnd, menu);
    if (cmd == kCmdPlace + kPlaceTray || cmd == kCmdPlace + kPlaceOverflow || cmd == kCmdPlace + kPlaceHidden) {
        PlaceTrayIcon(wnd, key, cmd - kCmdPlace);
    } else if (cmd == kCmdPluginIcon) {
        std::wstring file = PickPluginIcon(wnd);
        if (file.empty()) return;
        SavePluginIcon(key, file);
        PluginIconsChanged(wnd);
    } else if (cmd == kCmdPluginIconBack) {
        SavePluginIcon(key, L"");
        PluginIconsChanged(wnd);
    }
}

// The clock's clicks, as the Clock part's settings say. "What Windows does"
// and the clock's flyout open Windows' own date and time flyout.
void BarMenu(HWND wnd, bool fromClock = false);

////////////////////////////////////////////////////////////////////////////////
// Installing themes: a .msstyles, an orb picture, a folder of them (a downloaded theme pack) or a
// .zip goes where the bar looks for them, and the bar picks it up (the folders are watched).

std::wstring TourneOrbs() { return Expand(L"%LOCALAPPDATA%\\Tourne\\Orbs\\"); }

struct InstallResult {
    int styles = 0, orbs = 0, yasb = 0, fonts = 0;
    std::vector<std::wstring> names;
};

bool HasExtension(std::wstring const& name, std::initializer_list<PCWSTR> extensions) {
    size_t dot = name.find_last_of(L'.');
    if (dot == std::wstring::npos) return false;
    std::wstring ext = Lower(name.substr(dot));
    for (PCWSTR e : extensions)
        if (ext == e) return true;
    return false;
}

void InstallInto(std::wstring const& file, std::wstring const& folder, std::wstring const& name, InstallResult& result, bool style) {
    CreateDirectoryW(folder.c_str(), nullptr);
    std::wstring target = folder + name;
    if (_wcsicmp(file.c_str(), target.c_str()) == 0) return;
    if (!CopyFileW(file.c_str(), target.c_str(), FALSE)) return;
    (style ? result.styles : result.orbs)++;
    result.names.push_back(name.substr(0, name.find_last_of(L'.')));
}

void RemoveTree(std::wstring const& folder) {
    WIN32_FIND_DATAW fd;
    HANDLE find = FindFirstFileW((folder + L"\\*").c_str(), &fd);
    if (find != INVALID_HANDLE_VALUE) {
        do {
            std::wstring name = fd.cFileName;
            if (name == L"." || name == L"..") continue;
            std::wstring path = folder + L"\\" + name;
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) RemoveTree(path);
            else DeleteFileW(path.c_str());
        } while (FindNextFileW(find, &fd));
        FindClose(find);
    }
    RemoveDirectoryW(folder.c_str());
}

// A .zip opened with the tar that comes with Windows, into a folder to be looked through and removed.
std::wstring UnpackZip(std::wstring const& zip) {
    WCHAR temp[MAX_PATH];
    GetTempPathW(MAX_PATH, temp);
    std::wstring folder = std::wstring(temp) + L"TourneTheme_" + std::to_wstring(GetTickCount());
    CreateDirectoryW(folder.c_str(), nullptr);
    std::wstring command = L"tar.exe -xf \"" + zip + L"\" -C \"" + folder + L"\"";
    STARTUPINFOW si{sizeof(si)};
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    PROCESS_INFORMATION pi{};
    if (CreateProcessW(nullptr, command.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi)) {
        WaitForSingleObject(pi.hProcess, 30000);
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    }
    return folder;
}

// A folder with a styles.css and a config.yaml in it is a YASB theme.
bool IsYasbFolder(std::wstring const& folder) {
    return GetFileAttributesW((folder + L"\\styles.css").c_str()) != INVALID_FILE_ATTRIBUTES &&
           GetFileAttributesW((folder + L"\\config.yaml").c_str()) != INVALID_FILE_ATTRIBUTES;
}

void CopyTree(std::wstring const& from, std::wstring const& to, int depth = 0) {
    if (depth > 6) return;
    CreateDirectoryW(to.c_str(), nullptr);
    WIN32_FIND_DATAW fd;
    HANDLE find = FindFirstFileW((from + L"\\*").c_str(), &fd);
    if (find == INVALID_HANDLE_VALUE) return;
    do {
        std::wstring name = fd.cFileName;
        if (name == L"." || name == L"..") continue;
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) CopyTree(from + L"\\" + name, to + L"\\" + name, depth + 1);
        else if (fd.nFileSizeHigh == 0 && fd.nFileSizeLow < 32u * 1024 * 1024) CopyFileW((from + L"\\" + name).c_str(), (to + L"\\" + name).c_str(), FALSE);
    } while (FindNextFileW(find, &fd));
    FindClose(find);
}

void InstallPath(std::wstring const& path, InstallResult& result, int depth = 0, bool inOrbs = false, std::wstring const& asName = L"") {
    if (depth > 6 || result.styles + result.orbs + result.yasb + result.fonts > 200) return;
    DWORD attributes = GetFileAttributesW(path.c_str());
    if (attributes == INVALID_FILE_ATTRIBUTES) return;
    std::wstring name = path.substr(path.find_last_of(L'\\') + 1);
    if ((attributes & FILE_ATTRIBUTE_DIRECTORY) && IsYasbFolder(path)) {
        std::wstring theme = asName.empty() ? name : asName;
        for (auto& c : theme)
            if (wcschr(L"\\/:*?\"<>|", c)) c = L'_';
        CopyTree(path, TourneYasb() + theme);
        result.yasb++;
        result.names.push_back(theme);
        return;
    }
    if (attributes & FILE_ATTRIBUTE_DIRECTORY) {
        bool orbs = inOrbs || Lower(name).find(L"orb") != std::wstring::npos;
        WIN32_FIND_DATAW fd;
        HANDLE find = FindFirstFileW((path + L"\\*").c_str(), &fd);
        if (find == INVALID_HANDLE_VALUE) return;
        do {
            std::wstring child = fd.cFileName;
            if (child != L"." && child != L"..") InstallPath(path + L"\\" + child, result, depth + 1, orbs, asName);
        } while (FindNextFileW(find, &fd));
        FindClose(find);
    } else if (HasExtension(name, {L".ttf", L".otf", L".ttc"})) {
        // A font a theme names (a Nerd Font, say): used by the bar, for this program only.
        CreateDirectoryW(TourneFonts().c_str(), nullptr);
        if (CopyFileW(path.c_str(), (TourneFonts() + name).c_str(), FALSE)) {
            result.fonts++;
            result.names.push_back(name.substr(0, name.find_last_of(L'.')));
        }
    } else if (HasExtension(name, {L".msstyles"})) {
        InstallInto(path, TourneStyles(), name, result, true);
    } else if (HasExtension(name, {L".png", L".bmp", L".jpg", L".jpeg"}) && (inOrbs || depth == 0)) {
        InstallInto(path, TourneOrbs(), name, result, false);
    } else if (depth == 0 && HasExtension(name, {L".zip"})) {
        std::wstring folder = UnpackZip(path);
        InstallPath(folder, result, 1, false, name.substr(0, name.find_last_of(L'.')));  // a theme in it takes the .zip's name
        RemoveTree(folder);
    }
}

LRESULT CALLBACK TooltipProc(HWND tip, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR, DWORD_PTR);

// A small message beside the bar, in the tooltip look, for a few seconds.
HWND g_toast = nullptr;
void ShowToast(HWND wnd, std::wstring const& text) {
    if (!g_toast) {
        g_toast = CreateWindowExW(WS_EX_TOPMOST, TOOLTIPS_CLASSW, nullptr, WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP | TTS_NOANIMATE | TTS_NOFADE,
                                  CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, wnd, nullptr, nullptr, nullptr);
        if (!g_toast) return;
        SetWindowSubclass(g_toast, TooltipProc, 0, 0);
        DWORD square = 1;
        DwmSetWindowAttribute(g_toast, 33, &square, sizeof(square));
        BOOL noFade = TRUE;
        DwmSetWindowAttribute(g_toast, 3, &noFade, sizeof(noFade));
        TTTOOLINFOW ti{sizeof(ti)};
        ti.uFlags = TTF_TRACK | TTF_ABSOLUTE;
        ti.hwnd = wnd;
        ti.uId = 0x7001;
        ti.lpszText = (LPWSTR)L"";
        SendMessageW(g_toast, TTM_ADDTOOLW, 0, (LPARAM)&ti);
        SendMessageW(g_toast, TTM_SETMAXTIPWIDTH, 0, Px(420));
    }
    SendMessageW(g_toast, WM_SETFONT, (WPARAM)(g_tipFont ? g_tipFont : g_font), FALSE);
    RECT margin{Px(6), Px(4), Px(6), Px(4)};
    SendMessageW(g_toast, TTM_SETMARGIN, 0, (LPARAM)&margin);
    TTTOOLINFOW ti{sizeof(ti)};
    ti.hwnd = wnd;
    ti.uId = 0x7001;
    ti.lpszText = (LPWSTR)text.c_str();
    SendMessageW(g_toast, TTM_UPDATETIPTEXTW, 0, (LPARAM)&ti);
    RECT bar;
    GetWindowRect(wnd, &bar);
    // Above a bottom bar, below a top one, near its middle.
    int x = (bar.left + bar.right) / 2 - Px(120), y = g_settings.edge == Edge::Bottom ? bar.top - Px(48) : bar.bottom + Px(8);
    SendMessageW(g_toast, TTM_TRACKPOSITION, 0, MAKELPARAM(x, y));
    SendMessageW(g_toast, TTM_TRACKACTIVATE, TRUE, (LPARAM)&ti);
    SetTimer(wnd, kTimerToast, 3500, nullptr);
}

void InstallPaths(HWND wnd, std::vector<std::wstring> const& paths) {
    EnsureTourneFolders();
    InstallResult result;
    std::vector<std::wstring> done;  // a YASB theme's folder, when its styles.css or config.yaml was dropped instead of the folder
    for (auto const& path : paths) {
        std::wstring name = Lower(path.substr(path.find_last_of(L'\\') + 1));
        if (name == L"styles.css" || name == L"config.yaml") {
            std::wstring folder = path.substr(0, path.find_last_of(L'\\'));
            if (std::find(done.begin(), done.end(), folder) == done.end()) {
                done.push_back(folder);
                InstallPath(folder, result);
            }
        } else {
            InstallPath(path, result);
        }
    }
    std::wstring text;
    if (result.styles + result.orbs + result.yasb + result.fonts == 0) {
        text = L"Nothing to install: a .msstyles, an orb picture, a YASB theme (its folder or .zip), a font, or a folder or .zip with them.";
    } else {
        text = L"Installed";
        if (result.styles) text += L" " + std::to_wstring(result.styles) + (result.styles == 1 ? L" style" : L" styles");
        if (result.styles && (result.orbs || result.yasb)) text += L" and";
        if (result.orbs) text += L" " + std::to_wstring(result.orbs) + (result.orbs == 1 ? L" orb" : L" orbs");
        if (result.orbs && result.yasb) text += L" and";
        if (result.yasb) text += L" " + std::to_wstring(result.yasb) + (result.yasb == 1 ? L" YASB theme" : L" YASB themes");
        if (result.fonts) text += (result.styles + result.orbs + result.yasb ? L" and " : L" ") + std::to_wstring(result.fonts) + (result.fonts == 1 ? L" font" : L" fonts");
        text += L": ";
        for (size_t i = 0; i < result.names.size() && i < 4; i++) text += (i ? L", " : L"") + result.names[i];
        if (result.names.size() > 4) text += L"...";
    }
    ShowToast(wnd, text);
}

void InstallThemesDialog(HWND wnd) {
    IFileOpenDialog* dialog = nullptr;
    if (FAILED(CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&dialog)))) return;
    COMDLG_FILTERSPEC types[] = {{L"Themes, orbs and theme packs", L"*.msstyles;*.zip;*.png;*.bmp;*.jpg"}, {L"All files", L"*.*"}};
    dialog->SetFileTypes(2, types);
    dialog->SetTitle(L"Install a theme or orb");
    DWORD options = 0;
    dialog->GetOptions(&options);
    dialog->SetOptions(options | FOS_ALLOWMULTISELECT | FOS_FORCEFILESYSTEM);
    std::vector<std::wstring> paths;
    if (SUCCEEDED(dialog->Show(wnd))) {
        IShellItemArray* items = nullptr;
        if (SUCCEEDED(dialog->GetResults(&items))) {
            DWORD count = 0;
            items->GetCount(&count);
            for (DWORD i = 0; i < count; i++) {
                IShellItem* item = nullptr;
                LPWSTR path = nullptr;
                if (SUCCEEDED(items->GetItemAt(i, &item)) && SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &path))) {
                    paths.push_back(path);
                    CoTaskMemFree(path);
                }
                if (item) item->Release();
            }
            items->Release();
        }
    }
    dialog->Release();
    if (!paths.empty()) InstallPaths(wnd, paths);
}

////////////////////////////////////////////////////////////////////////////////
// YASB theme gallery: the community themes of yasb.dev/themes, which live in
// the amnweb/yasb-themes repository on GitHub (one folder per theme, with a
// theme.json, styles.css and config.yaml). Nothing is fetched until the user
// presses "Get the list"; a theme is fetched when Install is pressed. The
// files go to a temporary folder and are installed like a dropped theme.

namespace gallery {

constexpr UINT kMsgList = WM_APP + 71, kMsgDone = WM_APP + 72;
constexpr WCHAR kClass[] = L"TourneYasbGallery";
constexpr WCHAR kRaw[] = L"https://raw.githubusercontent.com/amnweb/yasb-themes/main/themes/";
constexpr WCHAR kListing[] = L"https://api.github.com/repos/amnweb/yasb-themes/contents/themes";
enum : int { kIdGet = 101, kIdList, kIdLink, kIdInstall, kIdShot, kIdClose };

struct Entry {
    std::wstring id, name, author, description, style, config, image;
};

struct ListJob {
    HWND wnd = nullptr;
    std::vector<std::wstring> ids;
    std::vector<Entry> entries;
    volatile LONG next = 0;
    bool ok = false;
};

struct DoneJob {
    std::wstring folder, name, error;
};

HWND g_win = nullptr, g_owner = nullptr, g_status = nullptr, g_get = nullptr, g_list = nullptr, g_detail = nullptr,
     g_link = nullptr, g_install = nullptr, g_shot = nullptr;
HFONT g_face = nullptr;
std::vector<Entry> g_entries;
bool g_busy = false;


bool StartWorker(void (*fn)(void*), void* arg, HANDLE* handle = nullptr) { return BgWork(fn, arg, handle); }

std::wstring WorkDir() {
    WCHAR temp[MAX_PATH];
    DWORD n = GetTempPathW(ARRAYSIZE(temp), temp);
    std::wstring dir = std::wstring(temp, n) + L"TourneYasbGallery\\";
    CreateDirectoryW(dir.c_str(), nullptr);
    return dir;
}

std::string ReadBytes(std::wstring const& path) {
    std::string data;
    HANDLE file = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE) return data;
    LARGE_INTEGER size{};
    if (GetFileSizeEx(file, &size) && size.QuadPart < 4 * 1024 * 1024) {
        data.resize((size_t)size.QuadPart);
        DWORD got = 0;
        if (!ReadFile(file, data.data(), (DWORD)data.size(), &got, nullptr)) got = 0;
        data.resize(got);
    }
    CloseHandle(file);
    return data;
}

std::wstring FromUtf8(std::string const& text) {
    if (text.empty()) return {};
    int n = MultiByteToWideChar(CP_UTF8, 0, text.data(), (int)text.size(), nullptr, 0);
    std::wstring out(n, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text.data(), (int)text.size(), out.data(), n);
    return out;
}

void AppendUtf8(std::string& out, unsigned cp) {
    if (cp < 0x80) {
        out += (char)cp;
    } else if (cp < 0x800) {
        out += (char)(0xC0 | (cp >> 6));
        out += (char)(0x80 | (cp & 0x3F));
    } else if (cp < 0x10000) {
        out += (char)(0xE0 | (cp >> 12));
        out += (char)(0x80 | ((cp >> 6) & 0x3F));
        out += (char)(0x80 | (cp & 0x3F));
    } else {
        out += (char)(0xF0 | (cp >> 18));
        out += (char)(0x80 | ((cp >> 12) & 0x3F));
        out += (char)(0x80 | ((cp >> 6) & 0x3F));
        out += (char)(0x80 | (cp & 0x3F));
    }
}

bool HexAt(std::string const& text, size_t at, unsigned* value) {
    if (at + 4 > text.size()) return false;
    unsigned v = 0;
    for (size_t i = at; i < at + 4; i++) {
        char c = text[i];
        unsigned d = c >= '0' && c <= '9' ? c - '0' : c >= 'a' && c <= 'f' ? c - 'a' + 10 : c >= 'A' && c <= 'F' ? c - 'A' + 10 : 16;
        if (d == 16) return false;
        v = v * 16 + d;
    }
    *value = v;
    return true;
}

// Every string value of a key in a JSON text: enough for GitHub's folder
// listing and a theme's own theme.json.
std::vector<std::string> JsonStrings(std::string const& text, std::string const& key) {
    std::vector<std::string> values;
    std::string needle = "\"" + key + "\"";
    auto skip = [&](size_t i) {
        while (i < text.size() && (text[i] == ' ' || text[i] == '\t' || text[i] == '\r' || text[i] == '\n')) i++;
        return i;
    };
    for (size_t at = text.find(needle); at != std::string::npos; at = text.find(needle, at + needle.size())) {
        size_t i = skip(at + needle.size());
        if (i >= text.size() || text[i] != ':') continue;
        i = skip(i + 1);
        if (i >= text.size() || text[i] != '"') continue;
        std::string value;
        for (i++; i < text.size() && text[i] != '"'; i++) {
            char c = text[i];
            if (c != '\\' || i + 1 >= text.size()) {
                value += c;
                continue;
            }
            c = text[++i];
            unsigned cp = 0;
            if (c == 'n' || c == 't' || c == 'r') {
                value += ' ';
            } else if (c == 'u' && HexAt(text, i + 1, &cp)) {
                i += 4;
                unsigned low = 0;
                if (cp >= 0xD800 && cp < 0xDC00 && i + 6 < text.size() && text[i + 1] == '\\' && text[i + 2] == 'u' &&
                    HexAt(text, i + 3, &low) && low >= 0xDC00 && low < 0xE000) {
                    cp = 0x10000 + ((cp - 0xD800) << 10) + (low - 0xDC00);
                    i += 6;
                }
                AppendUtf8(value, cp);
            } else {
                value += c;
            }
        }
        values.push_back(value);
    }
    return values;
}

std::wstring JsonFirst(std::string const& text, char const* key) {
    auto all = JsonStrings(text, key);
    return all.empty() ? L"" : FromUtf8(all[0]);
}

// A theme's id is a UUID (its folder's name in the repository).
bool IsId(std::wstring const& s, size_t at = 0) {
    if (at + 36 > s.size()) return false;
    for (size_t i = 0; i < 36; i++) {
        wchar_t c = s[at + i];
        bool dash = i == 8 || i == 13 || i == 18 || i == 23;
        if (dash ? c != L'-' : !((c >= L'0' && c <= L'9') || (c >= L'a' && c <= L'f') || (c >= L'A' && c <= L'F'))) return false;
    }
    return true;
}

// A pasted theme link (the gallery's, a GitHub folder or file, a raw file, a
// yasb-themes:// link) or a bare id: the id in it.
std::wstring FindId(std::wstring const& text) {
    for (size_t i = 0; i + 36 <= text.size(); i++)
        if (IsId(text, i)) return Lower(text.substr(i, 36));
    return {};
}

bool Fetch(std::wstring const& url, std::wstring const& file) {
    DeleteUrlCacheEntryW(url.c_str());
    return SUCCEEDED(URLDownloadToFileW(nullptr, url.c_str(), file.c_str(), 0, nullptr));
}

// Only files of the repository itself, whatever a theme.json says.
bool InRepo(std::wstring const& url) { return url.rfind(kRaw, 0) == 0 && url.find(L"..") == std::wstring::npos; }

void FillEntry(Entry& e, std::wstring const& work) {
    std::wstring file = work + e.id + L".json";
    if (!Fetch(std::wstring(kRaw) + e.id + L"/theme.json", file)) return;
    std::string json = ReadBytes(file);
    DeleteFileW(file.c_str());
    e.name = JsonFirst(json, "name");
    e.author = JsonFirst(json, "author");
    e.description = JsonFirst(json, "description");
    e.style = JsonFirst(json, "style");
    e.config = JsonFirst(json, "config");
    e.image = JsonFirst(json, "image");
}

void ListWorker(void* param) {
    auto* job = (ListJob*)param;
    std::wstring work = WorkDir();
    std::wstring file = work + L"listing.json";
    if (Fetch(kListing, file)) {
        for (auto const& name : JsonStrings(ReadBytes(file), "name")) {
            std::wstring id = FromUtf8(name);
            if (IsId(id) && id.size() == 36 && std::find(job->ids.begin(), job->ids.end(), id) == job->ids.end()) job->ids.push_back(id);
        }
        DeleteFileW(file.c_str());
    }
    job->entries.resize(job->ids.size());
    for (size_t i = 0; i < job->ids.size(); i++) job->entries[i].id = job->ids[i];
    // A handful of workers read the themes' descriptions side by side.
    struct Slice { ListJob* job; std::wstring work; };
    Slice slice{job, work};
    HANDLE threads[6];
    int started = 0;
    if (!job->ids.empty()) {
        for (int i = 0; i < 6; i++) {
            HANDLE h = nullptr;
            if (StartWorker(
                    [](void* p) {
                        auto* s = (Slice*)p;
                        for (;;) {
                            LONG n = InterlockedIncrement(&s->job->next) - 1;
                            if (n >= (LONG)s->job->entries.size()) break;
                            FillEntry(s->job->entries[n], s->work);
                        }
                    },
                    &slice, &h))
                threads[started++] = h;
        }
        if (started) WaitForMultipleObjects(started, threads, TRUE, 120000);
        for (int i = 0; i < started; i++) CloseHandle(threads[i]);
    }
    job->ok = !job->ids.empty();
    if (!job->wnd || !PostMessageW(job->wnd, kMsgList, 0, (LPARAM)job)) delete job;
}

struct InstallJob {
    HWND wnd = nullptr;
    Entry entry;
};

void InstallWorker(void* param) {
    std::unique_ptr<InstallJob> job((InstallJob*)param);
    auto* done = new DoneJob;
    Entry& e = job->entry;
    std::wstring work = WorkDir();
    if (e.style.empty() || e.config.empty() || e.name.empty()) FillEntry(e, work);
    std::wstring styleUrl = InRepo(e.style) ? e.style : std::wstring(kRaw) + e.id + L"/styles.css";
    std::wstring configUrl = InRepo(e.config) ? e.config : std::wstring(kRaw) + e.id + L"/config.yaml";
    std::wstring imageUrl = InRepo(e.image) ? e.image : std::wstring(kRaw) + e.id + L"/image.png";
    std::wstring folder = work + e.id;
    RemoveTree(folder);
    CreateDirectoryW(folder.c_str(), nullptr);
    done->name = e.name.empty() ? e.id : e.name;
    if (!Fetch(styleUrl, folder + L"\\styles.css") || !Fetch(configUrl, folder + L"\\config.yaml"))
        done->error = L"Could not download that theme (no connection, or it isn't in the repository).";
    else
        Fetch(imageUrl, folder + L"\\image.png");  // the theme's screenshot; optional
    done->folder = folder;
    if (!job->wnd || !PostMessageW(job->wnd, kMsgDone, 0, (LPARAM)done)) {
        RemoveTree(folder);
        delete done;
    }
}

void SetStatus(std::wstring const& text) { SetWindowTextW(g_status, text.c_str()); }

void UpdateButtons() {
    EnableWindow(g_get, !g_busy);
    EnableWindow(g_install, !g_busy);
}

Entry* Selected() {
    int i = (int)SendMessageW(g_list, LB_GETCURSEL, 0, 0);
    return i >= 0 && i < (int)g_entries.size() ? &g_entries[i] : nullptr;
}

void ShowDetail() {
    Entry* e = Selected();
    if (!e) {
        SetWindowTextW(g_detail, L"");
        return;
    }
    std::wstring text = e->name + (e->author.empty() ? L"" : L", by " + e->author);
    if (!e->description.empty()) text += L"\r\n" + e->description;
    SetWindowTextW(g_detail, text.c_str());
}

void BeginInstall(Entry entry) {
    auto* job = new InstallJob{g_win, std::move(entry)};
    SetStatus(L"Downloading " + (job->entry.name.empty() ? job->entry.id : job->entry.name) + L"...");
    g_busy = true;
    UpdateButtons();
    if (!StartWorker(InstallWorker, job)) {
        delete job;
        g_busy = false;
        UpdateButtons();
        SetStatus(L"Could not start the download.");
    }
}

LRESULT CALLBACK Proc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            UINT dpi = GetDpiForWindow(wnd);
            if (!dpi) dpi = 96;
            auto S = [dpi](int v) { return MulDiv(v, (int)dpi, 96); };
            g_face = CreateFontW(-MulDiv(9, (int)dpi, 72), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                                 CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
            auto make = [&](PCWSTR cls, PCWSTR text, DWORD style, DWORD ex, int x, int y, int w, int h, int id) {
                HWND c = CreateWindowExW(ex, cls, text, WS_CHILD | WS_VISIBLE | style, S(x), S(y), S(w), S(h), wnd, (HMENU)(INT_PTR)id, nullptr, nullptr);
                SendMessageW(c, WM_SETFONT, (WPARAM)g_face, TRUE);
                return c;
            };
            g_status = make(L"STATIC", L"The themes of yasb.dev/themes come from github.com/amnweb/yasb-themes. Nothing is downloaded until you press the button.",
                            0, 0, 12, 10, 536, 34, 0);
            g_get = make(L"BUTTON", L"Get the list", BS_PUSHBUTTON | WS_TABSTOP, 0, 12, 50, 150, 28, kIdGet);
            g_list = make(L"LISTBOX", L"", LBS_NOTIFY | LBS_NOINTEGRALHEIGHT | WS_VSCROLL | WS_TABSTOP, WS_EX_CLIENTEDGE, 12, 88, 536, 250, kIdList);
            g_detail = make(L"STATIC", L"", 0, 0, 12, 346, 536, 56, 0);
            make(L"STATIC", L"Or paste a link to a theme (gallery, GitHub) or its id:", 0, 0, 12, 410, 536, 18, 0);
            g_link = make(L"EDIT", L"", ES_AUTOHSCROLL | WS_TABSTOP, WS_EX_CLIENTEDGE, 12, 430, 536, 24, kIdLink);
            g_install = make(L"BUTTON", L"Install", BS_DEFPUSHBUTTON | WS_TABSTOP, 0, 12, 470, 120, 30, kIdInstall);
            g_shot = make(L"BUTTON", L"Screenshot", BS_PUSHBUTTON | WS_TABSTOP, 0, 142, 470, 120, 30, kIdShot);
            make(L"BUTTON", L"Close", BS_PUSHBUTTON | WS_TABSTOP, 0, 428, 470, 120, 30, kIdClose);
            return 0;
        }
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case kIdGet:
                    if (g_busy) break;
                    g_busy = true;
                    UpdateButtons();
                    SetStatus(L"Reading the list from GitHub...");
                    {
                        auto* job = new ListJob;
                        job->wnd = wnd;
                        if (!StartWorker(ListWorker, job)) {
                            delete job;
                            g_busy = false;
                            UpdateButtons();
                            SetStatus(L"Could not start the download.");
                        }
                    }
                    break;
                case kIdList:
                    if (HIWORD(wParam) == LBN_SELCHANGE) ShowDetail();
                    if (HIWORD(wParam) == LBN_DBLCLK && !g_busy)
                        if (Entry* e = Selected()) BeginInstall(*e);
                    break;
                case kIdInstall:
                case kIdShot: {
                    if (g_busy) break;
                    Entry entry;
                    WCHAR pasted[1024] = L"";
                    GetWindowTextW(g_link, pasted, ARRAYSIZE(pasted));
                    std::wstring id = FindId(pasted);
                    if (!id.empty()) {
                        for (auto const& e : g_entries)
                            if (e.id == id) entry = e;
                        entry.id = id;
                    } else if (Entry* e = Selected()) {
                        entry = *e;
                    } else {
                        SetStatus(wcslen(pasted) ? L"That link has no theme in it. Paste a link to one theme's folder, or pick one from the list."
                                                 : L"Get the list and pick a theme, or paste a link to one.");
                        break;
                    }
                    if (LOWORD(wParam) == kIdShot) {
                        std::wstring url = InRepo(entry.image) ? entry.image : std::wstring(kRaw) + entry.id + L"/image.png";
                        ShellExecuteW(nullptr, L"open", url.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
                    } else {
                        BeginInstall(entry);
                    }
                    break;
                }
                case kIdClose: DestroyWindow(wnd); break;
            }
            return 0;
        case kMsgList: {
            std::unique_ptr<ListJob> job((ListJob*)lParam);
            g_busy = false;
            UpdateButtons();
            if (!job->ok) {
                SetStatus(L"Could not read the list (no connection, or GitHub is limiting requests for now). A pasted link still works.");
                return 0;
            }
            g_entries = std::move(job->entries);
            for (auto& e : g_entries)
                if (e.name.empty()) e.name = e.id;
            std::sort(g_entries.begin(), g_entries.end(), [](Entry const& a, Entry const& b) { return _wcsicmp(a.name.c_str(), b.name.c_str()) < 0; });
            SendMessageW(g_list, LB_RESETCONTENT, 0, 0);
            for (auto const& e : g_entries) {
                std::wstring line = e.name + (e.author.empty() ? L"" : L"  -  " + e.author);
                SendMessageW(g_list, LB_ADDSTRING, 0, (LPARAM)line.c_str());
            }
            SetStatus(std::to_wstring(g_entries.size()) + L" themes. Pick one and press Install (or double-click it).");
            ShowDetail();
            return 0;
        }
        case kMsgDone: {
            std::unique_ptr<DoneJob> done((DoneJob*)lParam);
            g_busy = false;
            UpdateButtons();
            if (!done->error.empty()) {
                RemoveTree(done->folder);
                SetStatus(done->error);
                return 0;
            }
            EnsureTourneFolders();
            InstallResult result;
            InstallPath(done->folder, result, 0, false, done->name);
            RemoveTree(done->folder);
            if (result.yasb) {
                std::wstring text = L"Installed " + done->name + L". Pick it under YASB theme in the Taskbar settings.";
                SetStatus(text);
                if (g_owner) ShowToast(g_owner, text);
            } else {
                SetStatus(L"That theme has no styles.css and config.yaml, so it was not installed.");
            }
            return 0;
        }
        case WM_CLOSE: DestroyWindow(wnd); return 0;
        case WM_DESTROY:
            if (g_face) DeleteObject(g_face);
            g_face = nullptr;
            g_win = g_status = g_get = g_list = g_detail = g_link = g_install = g_shot = nullptr;
            return 0;
    }
    return DefWindowProcW(wnd, msg, wParam, lParam);
}

void Open(HWND owner) {
    if (g_win) {
        SetForegroundWindow(g_win);
        return;
    }
    WNDCLASSW wc{};
    wc.lpfnWndProc = Proc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName = kClass;
    RegisterClassW(&wc);  // already there after the first time
    g_owner = owner;
    UINT dpi = GetDpiForWindow(owner);
    if (!dpi) dpi = 96;
    RECT r{0, 0, MulDiv(560, (int)dpi, 96), MulDiv(512, (int)dpi, 96)};
    DWORD style = WS_POPUP | WS_CAPTION | WS_SYSMENU;
    AdjustWindowRectExForDpi(&r, style, FALSE, WS_EX_TOPMOST, dpi);
    MONITORINFO mi{sizeof(mi)};
    GetMonitorInfoW(MonitorFromWindow(owner, MONITOR_DEFAULTTONEAREST), &mi);
    int w = r.right - r.left, h = r.bottom - r.top;
    int x = (mi.rcWork.left + mi.rcWork.right - w) / 2, y = (mi.rcWork.top + mi.rcWork.bottom - h) / 2;
    g_win = CreateWindowExW(WS_EX_TOPMOST, kClass, L"Get a YASB theme", style, x, y, w, h, owner, nullptr, wc.hInstance, nullptr);
    if (!g_win) return;
    ShowWindow(g_win, SW_SHOW);
    SetForegroundWindow(g_win);
}

// The window closes with the bar's thread; a download still running finishes
// first (it holds the module).
void Close() {
    if (g_win) DestroyWindow(g_win);
    g_win = nullptr;
    UnregisterClassW(kClass, GetModuleHandleW(nullptr));
}

}  // namespace gallery

// Tourne's notification centre (System Icons'), opened by anchor; false when System Icons isn't running.
bool OpenTourneNotificationCentre(RECT const& anchor) {
    HWND host = nullptr;
    EnumWindows(
        [](HWND h, LPARAM param) __stdcall -> BOOL {
            WCHAR name[96];
            if (GetClassNameW(h, name, ARRAYSIZE(name)) && wcsncmp(name, L"TourneTraySystemIcons_", 22) == 0) {
                *(HWND*)param = h;
                return FALSE;
            }
            return TRUE;
        },
        (LPARAM)&host);
    if (!host) return false;
    DWORD pid = 0;
    GetWindowThreadProcessId(host, &pid);
    AllowSetForegroundWindow(pid);
    LeaveAnchor(host, anchor);  // so it opens by what was clicked
    PostMessageW(host, RegisterWindowMessageW(L"TourneTray_NotificationCenter"), 0, 0);
    return true;
}

// Windows' notification centre (Win+N); in Tourne Desktop, whose shell mode hides Windows' one, Tourne's.
void OpenNotificationCentre(RECT const& anchor) {
#ifdef TD_PREFIX
    if (OpenTourneNotificationCentre(anchor)) return;
#else
    (void)anchor;
#endif
    SendKeys({VK_LWIN, 'N'});
}

void ClockClick(HWND wnd, int button) {
    std::wstring const& action = g_clock.clicks[button];
    if (action == L"nothing") return;
    if (action == L"notificationCenter") {
        if (!OpenTourneNotificationCentre(g_clockRect)) SendKeys({VK_LWIN, 'N'});
    } else if (action == L"taskbarMenu") {
        BarMenu(wnd, true);
    } else if (action == L"notifications") {
        OpenNotificationCentre(g_clockRect);
    } else if (action == L"settings") {
        ShellExecuteW(nullptr, L"open", L"ms-settings:dateandtime", nullptr, nullptr, SW_SHOWNORMAL);
    } else if (action == L"alarms") {
        ShellExecuteW(nullptr, L"open", L"ms-clock:", nullptr, nullptr, SW_SHOWNORMAL);
    } else if (action == L"copy") {
        SYSTEMTIME now;
        GetLocalTime(&now);
        std::wstring text;
        for (size_t i = 0; i < g_clock.lines.size(); i++) text += (i ? L" " : L"") + ExpandClockLine(g_clock.lines[i], now);
        if (OpenClipboard(wnd)) {
            EmptyClipboard();
            if (HGLOBAL memory = GlobalAlloc(GMEM_MOVEABLE, (text.size() + 1) * sizeof(WCHAR))) {
                memcpy(GlobalLock(memory), text.c_str(), (text.size() + 1) * sizeof(WCHAR));
                GlobalUnlock(memory);
                if (!SetClipboardData(CF_UNICODETEXT, memory)) GlobalFree(memory);
            }
            CloseClipboard();
        }
    } else if (action == L"command") {
        std::wstring file = g_clock.commands[button], arguments;
        file.erase(0, file.find_first_not_of(L" \t"));
        if (!file.empty() && file[0] == L'"') {
            size_t close = file.find(L'"', 1);
            if (close != std::wstring::npos) {
                arguments = file.substr(close + 1);
                file = file.substr(1, close - 1);
            }
        }
        if (!file.empty())
            ShellExecuteW(nullptr, L"open", file.c_str(), arguments.empty() ? nullptr : arguments.c_str(), nullptr,
                          SW_SHOWNORMAL);
    } else {
        SendKeys({VK_LWIN, VK_MENU, 'D'});
    }
}

// Every icon Windows has seen (the list its own settings page shows, HKCU\Control Panel\
// NotifyIconSettings): the tray's key for it (program and icon id) and the tooltip it first had.
// Icons known only by a GUID are left out (the tray can't name them the same way).
struct KnownTrayIcon {
    std::wstring key, label;
};
std::vector<KnownTrayIcon> KnownTrayIcons() {
    std::vector<KnownTrayIcon> known;
    HKEY root;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Control Panel\\NotifyIconSettings", 0, KEY_READ, &root) != ERROR_SUCCESS)
        return known;
    for (DWORD i = 0;; i++) {
        WCHAR name[64];
        DWORD length = ARRAYSIZE(name);
        if (RegEnumKeyExW(root, i, name, &length, nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS) break;
        WCHAR path[MAX_PATH * 2] = L"", tip[256] = L"";
        DWORD uid = 0, size = sizeof(path);
        if (RegGetValueW(root, name, L"ExecutablePath", RRF_RT_REG_SZ, nullptr, path, &size) != ERROR_SUCCESS) continue;
        size = sizeof(uid);
        if (RegGetValueW(root, name, L"UID", RRF_RT_REG_DWORD, nullptr, &uid, &size) != ERROR_SUCCESS) continue;
        size = sizeof(tip);
        RegGetValueW(root, name, L"InitialTooltip", RRF_RT_REG_SZ, nullptr, tip, &size);
        // System Icons' own (they run in Windhawk) are its settings' to place.
        if (LowerFileName(path) == L"windhawk.exe") continue;
        std::wstring key = LowerFileName(path) + L"|" + std::to_wstring(uid);
        if (std::any_of(known.begin(), known.end(), [&](KnownTrayIcon const& k) { return k.key == key; })) continue;
        HiddenIcon named;
        named.tip = tip;
        std::wstring label = named.tip.substr(0, named.tip.find(L'\n'));
        if (label.empty()) label = LowerFileName(path);
        if (label.size() > 48) label = label.substr(0, 47) + L"\x2026";
        std::wstring escaped;
        for (WCHAR ch : label) {
            if (ch == L'&') escaped += L'&';
            escaped += ch;
        }
        known.push_back({key, escaped});
    }
    RegCloseKey(root);
    std::sort(known.begin(), known.end(),
              [](KnownTrayIcon const& a, KnownTrayIcon const& b) { return _wcsicmp(a.label.c_str(), b.label.c_str()) < 0; });
    return known;
}

void BarMenu(HWND wnd, bool fromClock) {
    HMENU menu = CreatePopupMenu();
    // Every tray icon: a click puts a running one in the tray or back in the overflow; ones not
    // running (any Windows has seen) are placed for when they start; and every choice, per icon.
    std::vector<std::wstring> trayKeys, knownKeys;
    std::vector<int> trayNow;
    HMENU icons = nullptr;
    if (g_settings.tray && (!g_trayAll.empty() || !g_pluginItems.empty())) {
        icons = CreatePopupMenu();
        TrayPlaces places = TrayPlaces::Load();
        HMENU each = CreatePopupMenu();
        // Plugin items first: in the tray, or in the overflow.
        for (auto const& item : g_pluginItems) {
            std::wstring key = L"plugin:" + item.key;
            if (std::find(trayKeys.begin(), trayKeys.end(), key) != trayKeys.end()) continue;
            int place = places.Of(key) == kPlaceDefault ? kPlaceTray : places.Of(key);
            std::wstring label = item.tip.substr(0, item.tip.find_first_of(L"\r\n"));
            if (label.empty()) label = item.text.empty() ? item.key : item.text;
            if (label.size() > 48) label = label.substr(0, 47) + L"\x2026";
            for (size_t at = 0; (at = label.find(L'&', at)) != std::wstring::npos; at += 2) label.insert(at, 1, L'&');
            AppendMenuW(icons, MF_STRING | (place == kPlaceTray ? MF_CHECKED : 0), kCmdTrayToggle + trayKeys.size(),
                        (label + (place == kPlaceHidden ? L"\t(hidden)" : L"")).c_str());
            trayKeys.push_back(key);
            trayNow.push_back(place);
        }
        if (!g_pluginItems.empty() && !g_trayAll.empty()) AppendMenuW(icons, MF_SEPARATOR, 0, nullptr);
        for (auto const& icon : g_trayAll) {
            std::wstring key = HiddenIconKey(icon);
            if (std::find(trayKeys.begin(), trayKeys.end(), key) != trayKeys.end()) continue;
            int place = places.For(icon, key);
            std::wstring label = TrayLabel(icon);
            AppendMenuW(icons, MF_STRING | (place == kPlaceTray ? MF_CHECKED : 0), kCmdTrayToggle + trayKeys.size(),
                        (label + (place == kPlaceHidden ? L"\t(hidden)" : L"")).c_str());
            HMENU choices = CreatePopupMenu();
            AppendPlaceItems(choices, kCmdPlace + (int)trayKeys.size() * 4, places.Of(key));
            AppendMenuW(each, MF_POPUP, (UINT_PTR)choices, label.c_str());
            trayKeys.push_back(key);
            trayNow.push_back(place);
        }
        HMENU idle = CreatePopupMenu();
        for (auto const& k : KnownTrayIcons()) {
            if (std::find(trayKeys.begin(), trayKeys.end(), k.key) != trayKeys.end()) continue;
            AppendMenuW(idle, MF_STRING | (places.Of(k.key) == kPlaceTray ? MF_CHECKED : 0),
                        kCmdTrayKnown + knownKeys.size(), k.label.c_str());
            knownKeys.push_back(k.key);
        }
        AppendMenuW(icons, MF_SEPARATOR, 0, nullptr);
        if (!knownKeys.empty()) AppendMenuW(icons, MF_POPUP, (UINT_PTR)idle, L"Not running now");
        else DestroyMenu(idle);
        AppendMenuW(icons, MF_POPUP, (UINT_PTR)each, L"Where each goes");
    }
    // From the clock, the tray icons come first.
    if (icons && fromClock) {
        AppendMenuW(menu, MF_POPUP, (UINT_PTR)icons, L"Tray icons");
        AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    }
    AppendMenuW(menu, MF_STRING, kCmdTaskManager, L"Task Manager");
    if (!WindhawkExe().empty()) AppendMenuW(menu, MF_STRING, kCmdWindhawk, L"Windhawk");
    AppendMenuW(menu, MF_STRING, kCmdDesktop, L"Show the desktop");
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    if (icons && !fromClock) {
        AppendMenuW(menu, MF_POPUP, (UINT_PTR)icons, L"Tray icons");
        AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    }
    AppendMenuW(menu, MF_STRING, kCmdTaskbarSettings, L"Taskbar settings");
    AppendMenuW(menu, MF_STRING, kCmdTourneSettings, L"Tourne taskbar settings");
    AppendMenuW(menu, MF_STRING, kCmdOpenStyles, L"Open the Tourne styles folder");
    AppendMenuW(menu, MF_STRING, kCmdInstallTheme, L"Install a theme or orb...");
    AppendMenuW(menu, MF_STRING, kCmdGetYasb, L"Get a YASB theme...");
    int cmd = RunMenu(wnd, menu);
    if (cmd >= kCmdPlace && cmd < kCmdPlace + (int)trayKeys.size() * 4) {
        PlaceTrayIcon(wnd, trayKeys[(cmd - kCmdPlace) / 4], (cmd - kCmdPlace) % 4);
        return;
    }
    if (cmd >= kCmdTrayToggle && cmd < kCmdTrayToggle + (int)trayKeys.size()) {
        int i = cmd - kCmdTrayToggle;
        PlaceTrayIcon(wnd, trayKeys[i], trayNow[i] == kPlaceTray ? kPlaceOverflow : kPlaceTray);
        return;
    }
    if (cmd >= kCmdTrayKnown && cmd < kCmdTrayKnown + (int)knownKeys.size()) {
        std::wstring const& key = knownKeys[cmd - kCmdTrayKnown];
        PlaceTrayIcon(wnd, key, TrayPlaces::Load().Of(key) == kPlaceTray ? kPlaceDefault : kPlaceTray);
        return;
    }
    switch (cmd) {
        case kCmdTaskManager: Launch(L"taskmgr.exe"); break;
        case kCmdWindhawk: Launch(WindhawkExe()); break;
        case kCmdDesktop: ToggleShowDesktop(); break;
        case kCmdTaskbarSettings: ShellExecuteW(nullptr, nullptr, L"ms-settings:taskbar", nullptr, nullptr, SW_SHOWNORMAL); break;
        case kCmdTourneSettings: OpenOwnSettings(); break;
        case kCmdInstallTheme: InstallThemesDialog(wnd); break;
        case kCmdGetYasb: gallery::Open(wnd); break;
        case kCmdOpenStyles:
            EnsureTourneFolders();
            ShellExecuteW(nullptr, nullptr, TourneStyles().c_str(), nullptr, nullptr, SW_SHOWNORMAL);
            break;
    }
}

////////////////////////////////////////////////////////////////////////////////
// The bar: an app bar on its edge of the main screen, so maximised windows
// make room for it. With automatic hiding it reserves nothing and slides;
// with dynamic shyness too, it only slides away while a window touches it.

// Either kind of hiding: the bar reserves no space and can slide away.
bool Hides() { return g_settings.autoHide != g_hideFlip || g_settings.shy; }
void ShyApply(HWND wnd);

void ArriveApply(HWND wnd);

void PlaceBar(HWND wnd) {
    RECT r = BarRect(g_slide > 0 || !Hides());
    if (g_appBar) {
        APPBARDATA abd{sizeof(abd)};
        abd.hWnd = wnd;
        abd.uEdge = g_settings.edge == Edge::Top ? ABE_TOP : g_settings.edge == Edge::Bottom ? ABE_BOTTOM
                  : g_settings.edge == Edge::Left ? ABE_LEFT : ABE_RIGHT;
        abd.rc = r;
        SHAppBarMessage(ABM_QUERYPOS, &abd);
        int t = Px(g_settings.thickness);
        switch (g_settings.edge) {
            case Edge::Top: abd.rc.bottom = abd.rc.top + t; break;
            case Edge::Bottom: abd.rc.top = abd.rc.bottom - t; break;
            case Edge::Left: abd.rc.right = abd.rc.left + t; break;
            case Edge::Right: abd.rc.left = abd.rc.right - t; break;
        }
        SHAppBarMessage(ABM_SETPOS, &abd);
        r = abd.rc;
    }
    if (CssActive()) r = CssBody(r);  // a YASB theme's bar floats inside the space it reserves
    // Logged when it moves (Tourne saw the bar's hit spots a pixel off after a screen snip).
    RECT was{};
    GetWindowRect(wnd, &was);
    if (!EqualRect(&was, &r))
        Wh_Log(L"Bar placed at %d,%d to %d,%d (was %d,%d to %d,%d)", r.left, r.top, r.right, r.bottom, was.left, was.top,
               was.right, was.bottom);
    SetWindowPos(wnd, g_underFullscreen ? HWND_BOTTOM : HWND_TOPMOST, r.left, r.top, r.right - r.left, r.bottom - r.top,
                 SWP_NOACTIVATE | SWP_SHOWWINDOW);
    if (Hides() && (g_slide < 1 || g_edgeHint)) ShyApply(wnd);  // midway, or the glow at the edge
    if (BarArrival& a = g_active->arrive; a.running && a.slide) {
        // Placed again while a new screen's bar slides in: it goes on to (and ends at) the new place.
        a.home = {r.left, r.top};
        ArriveApply(wnd);
    }
}

void SetAppBar(HWND wnd, bool on) {
    APPBARDATA abd{sizeof(abd)};
    abd.hWnd = wnd;
    abd.uCallbackMessage = WM_APP_BAR;
    if (on && !g_appBar) g_appBar = SHAppBarMessage(ABM_NEW, &abd) != 0;
    if (!on && g_appBar) {
        SHAppBarMessage(ABM_REMOVE, &abd);
        g_appBar = false;
    }
}

// The windows and pins read again (every bar takes its buttons from them).
void RescanWindows() {
    // Pins changed in Explorer (pinned, unpinned, reordered).
    if (LoadPins(false)) {
        for (size_t i = g_iconCache.size(); i-- > 0;)
            if (g_iconCache[i].id.starts_with(L"pin:")) {
                if (g_iconCache[i].icon) DestroyIcon(g_iconCache[i].icon);
                g_iconCache.erase(g_iconCache.begin() + i);
            }
    }
    Rescan();
}

// Every bar laid out and drawn again (or just the given one, when no bars are registered: a test bar).
void RelayoutBars(HWND fallback = nullptr) {
    if (g_bars.empty()) {
        if (fallback) {
            Layout(fallback);
            Redraw(fallback);
            SyncPreview(fallback);
        }
        return;
    }
    std::vector<BarState*> bars = g_bars;  // a bar coming or going meanwhile doesn't upset the walk
    for (BarState* bar : bars) {
        if (std::find(g_bars.begin(), g_bars.end(), bar) == g_bars.end() || !bar->hwnd) continue;
        BarScope scope(bar);
        Layout(bar->hwnd);
        Redraw(bar->hwnd);
        SyncPreview(bar->hwnd);
    }
}

void Refresh(HWND wnd) {
    RescanWindows();
    RelayoutBars(wnd);
}

// What the bars show that can change without a message saying so: the windows (their titles, icons,
// states and screens), the active one, progress, pins, the icons read, the tray, the desktops and the
// clock's text; and per bar its size, hover and fade. 0 when it can't be told: a YASB theme's widgets
// (live data), an open preview (its titles) or a drag; the bars are then drawn every second as before.
uint64_t BarsSignature(HWND fallback) {
    if (CssActive() || !g_settings.yasbTheme.empty()) return 0;
    if (g_preview.wnd && IsWindowVisible(g_preview.wnd)) return 0;
    if (g_buttonDrag >= 0 || g_trayDrag >= 0 || g_trayDropAt >= 0 || g_pagerDrag || g_pagerCarry) return 0;
    uint64_t hash = 14695981039346656037ull;
    auto mix = [&](void const* data, size_t size) {
        for (size_t i = 0; i < size; i++) hash = (hash ^ ((BYTE const*)data)[i]) * 1099511628211ull;
    };
    auto put = [&](auto value) { mix(&value, sizeof(value)); };
    auto text = [&](std::wstring const& s) {
        put(s.size());
        mix(s.data(), s.size() * sizeof(WCHAR));
    };
    put(ForegroundRoot());
    for (auto const& w : g_wins) {
        put(w.hwnd);
        put(w.icon);
        put(w.iconPx);
        put(w.flashing);
        put((bool)IsIconic(w.hwnd));
        put(MonitorFromWindow(w.hwnd, MONITOR_DEFAULTTONEAREST));
        text(Title(w.hwnd));
    }
    uint64_t progress = 0;  // (in any order)
    for (auto const& [h, pr] : g_progress)
        progress += ((uint64_t)(ULONG_PTR)h * 1099511628211ull) ^ ((uint64_t)pr.state << 32 | (uint32_t)pr.value);
    put(progress);
    put(g_pinsChanges);
    put(g_pins.size());
    put(g_shortcutsRead);
    put(g_iconCache.size());
    put(g_trayHash);
    put(ArrowOpen());
    put(g_pagerDesktops.count);
    put(g_pagerDesktops.current);
    for (auto const& name : g_pagerDesktops.names) text(name);
    SYSTEMTIME now;
    GetLocalTime(&now);
    auto bar = [&](HWND wnd) {
        RECT rc{};
        GetClientRect(wnd, &rc);
        put(wnd);
        put(rc);
        put(g_hot);
        put(g_pressed);
        put(g_touching);
        put(g_edgeHint);
        put(g_barAlpha);
        put(g_shown);
        for (auto const& item : g_tray) text(item.key);
        for (auto const& line : g_clock.lines) text(ExpandClockLine(line, now));  // as DrawClock shows it
    };
    put(g_bars.size());
    if (g_bars.empty() && fallback) bar(fallback);
    std::vector<BarState*> bars = g_bars;
    for (BarState* b : bars) {
        if (!b->hwnd) continue;
        BarScope scope(b);
        bar(b->hwnd);
    }
    return hash | 1;
}

void StyleTooltip();

////////////////////////////////////////////////////////////////////////////////
// Hiding StartAllBack's taskbar (or Windows 11's): Explorer's taskbars set to
// auto-hide, so they give their space back, and the suite's relay in Explorer
// keeps them hidden away. The auto-hide state they had is kept in the registry
// before it's changed, so it's put back even after a crash, on the next start.

constexpr WCHAR kSystemTaskbarStateValue[] = L"SystemTaskbarState";  // its ABM state + 1
bool g_systemTaskbarHidden = false;

std::vector<HWND> SystemTaskbars() {
    std::vector<HWND> bars;
    if (HWND tray = FindWindowW(L"Shell_TrayWnd", nullptr)) bars.push_back(tray);
    for (HWND h = nullptr; (h = FindWindowExW(nullptr, h, L"Shell_SecondaryTrayWnd", nullptr));) bars.push_back(h);
    return bars;
}

DWORD SavedSystemTaskbarState() {
    DWORD value = 0, size = sizeof(value);
    RegGetValueW(HKEY_CURRENT_USER, kTrayStoreKey, kSystemTaskbarStateValue, RRF_RT_REG_DWORD, nullptr, &value, &size);
    return value;
}

// StartAllBack switched off for this account, as its own "Disable program for
// current user" does: SOFTWARE\StartIsBack's Disabled, read as Explorer loads
// it. Only a change of the setting acts, and only what this did is undone: a
// StartAllBack the user switched off themselves stays off.
constexpr WCHAR kSabKey[] = L"Software\\StartIsBack";
constexpr WCHAR kSabOffValue[] = L"StartAllBackOff";  // in kTrayStoreKey: 1 while this has it off

DWORD ReadDword(PCWSTR key, PCWSTR name) {
    DWORD value = 0, size = sizeof(value);
    RegGetValueW(HKEY_CURRENT_USER, key, name, RRF_RT_REG_DWORD, nullptr, &value, &size);
    return value;
}

void WriteDword(PCWSTR key, PCWSTR name, DWORD value) {
    RegSetKeyValueW(HKEY_CURRENT_USER, key, name, REG_DWORD, &value, sizeof(value));
}

// Explorer closed as its own "Exit Explorer" does, then started again, off
// the bar's thread.
DWORD WINAPI RestartExplorerThread(LPVOID) {
    HWND tray = FindWindowW(L"Shell_TrayWnd", nullptr);
    DWORD pid = 0;
    if (tray) GetWindowThreadProcessId(tray, &pid);
    HANDLE process = pid ? OpenProcess(SYNCHRONIZE | PROCESS_TERMINATE, FALSE, pid) : nullptr;
    if (tray) PostMessageW(tray, WM_USER + 436, 0, 0);
    if (process) {
        if (WaitForSingleObject(process, 8000) == WAIT_TIMEOUT) TerminateProcess(process, 1);
        CloseHandle(process);
    }
    // Windows may bring it back by itself; otherwise it's started here.
    Sleep(1500);
    if (!FindWindowW(L"Shell_TrayWnd", nullptr)) {
        WCHAR path[MAX_PATH];
        GetWindowsDirectoryW(path, MAX_PATH);
        wcscat_s(path, L"\\explorer.exe");
        STARTUPINFOW si{sizeof(si)};
        PROCESS_INFORMATION pi{};
        if (CreateProcessW(path, nullptr, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) {
            CloseHandle(pi.hThread);
            CloseHandle(pi.hProcess);
        }
    }
    return 0;
}

void RestartExplorer() {
    if (HANDLE thread = CreateThread(nullptr, 0, RestartExplorerThread, nullptr, 0, nullptr)) CloseHandle(thread);
}

void TurnStartAllBackOff(bool off) {
    HKEY key;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, kSabKey, 0, KEY_READ, &key) != ERROR_SUCCESS) return;  // not installed
    RegCloseKey(key);
    bool ours = ReadDword(kTrayStoreKey, kSabOffValue) != 0;
    if (off && !ours) {
        if (ReadDword(kSabKey, L"Disabled")) return;  // already off, by the user
        WriteDword(kSabKey, L"Disabled", 1);
        WriteDword(kTrayStoreKey, kSabOffValue, 1);
        Wh_Log(L"StartAllBack switched off; restarting Explorer");
        RestartExplorer();
    } else if (!off && ours) {
        RegDeleteKeyValueW(HKEY_CURRENT_USER, kSabKey, L"Disabled");
        RegDeleteKeyValueW(HKEY_CURRENT_USER, kTrayStoreKey, kSabOffValue);
        Wh_Log(L"StartAllBack switched back on; restarting Explorer");
        RestartExplorer();
    }
}

// What this bar asks of Explorer's side of the suite (the relay, in Explorer): on the bar's own window,
// and, from Tourne Desktop (whose settings the relay can't read), also in the registry, where the relay
// looks as Explorer starts. Nothing is done for a bar that isn't there, so a crash here undoes it all.
constexpr WCHAR kShellHideProp[] = L"TourneShell_HideNative";  // Windows' taskbars kept hidden
constexpr WCHAR kShellStartProp[] = L"TourneShell_StartKey";   // this Start takes the Windows key
constexpr WCHAR kShellAskKey[] = L"Software\\Tourne\\Desktop\\Shell";

void ShellAsk(HWND wnd, PCWSTR prop, PCWSTR value, bool on) {
    if (wnd) {
        if (on) SetPropW(wnd, prop, (HANDLE)1);
        else RemovePropW(wnd, prop);
    }
    if (wcscmp(WH_MOD_ID, L"tourne-desktop") != 0) return;
    DWORD v = on ? 1 : 0;
    RegSetKeyValueW(HKEY_CURRENT_USER, kShellAskKey, value, REG_DWORD, &v, sizeof(v));
}

void HideSystemTaskbar(HWND wnd, bool hide) {
    ShellAsk(wnd, kShellHideProp, L"HideTaskbar", hide);
    HWND tray = FindWindowW(L"Shell_TrayWnd", nullptr);
    DWORD saved = SavedSystemTaskbarState();
    APPBARDATA abd{sizeof(abd)};
    abd.hWnd = tray;
    if (hide) {
        if (tray && !saved) {
            UINT state = (UINT)SHAppBarMessage(ABM_GETSTATE, &abd);
            DWORD value = state + 1;
            RegSetKeyValueW(HKEY_CURRENT_USER, kTrayStoreKey, kSystemTaskbarStateValue, REG_DWORD, &value, sizeof(value));
            if (!(state & ABS_AUTOHIDE)) {
                abd.lParam = state | ABS_AUTOHIDE;
                SHAppBarMessage(ABM_SETSTATE, &abd);
            }
        }
        // Only auto-hidden here: the suite's relay in Explorer keeps it hidden from there, as Explorer
        // itself sees it, and cloaks it once it's away. It's never hidden outright any more (Windows
        // 11's taskbar fails Explorer when its window is hidden from outside); one an older build hid
        // is shown again.
        for (HWND h : SystemTaskbars())
            if (!IsWindowVisible(h)) ShowWindow(h, SW_SHOWNA);
    } else {
        KillTimer(wnd, kTimerSystemTaskbar);
        if (tray && saved) {
            abd.lParam = saved - 1;
            SHAppBarMessage(ABM_SETSTATE, &abd);
            RegDeleteKeyValueW(HKEY_CURRENT_USER, kTrayStoreKey, kSystemTaskbarStateValue);
        }
        // Shown again if it was hidden here (now, or before a crash).
        if (g_systemTaskbarHidden || saved)
            for (HWND h : SystemTaskbars())
                if (!IsWindowVisible(h)) ShowWindow(h, SW_SHOWNA);
    }
    g_systemTaskbarHidden = hide;
}

// @inline phone-link.cpp

// @inline phone-map.cpp

// @inline phone-ancs.cpp

// @inline phone-files.cpp

// @inline start-menu.cpp

// @inline tourne-menu.cpp

// @inline desktop-menu.cpp

// Blur or acrylic behind a see-through bar (not with segments: the gaps
// between islands would blur too).
void SetBarBackdrop(HWND wnd) {
    struct AccentPolicy {
        int state, flags;
        DWORD color;
        int animation;
    };
    struct CompositionData {
        int attribute;
        void* data;
        SIZE_T size;
    };
    using SetWindowCompositionAttribute_t = BOOL(WINAPI*)(HWND, CompositionData*);
    static auto set =
        (SetWindowCompositionAttribute_t)GetProcAddress(GetModuleHandleW(L"user32.dll"), "SetWindowCompositionAttribute");
    if (!set) return;
    int blur = g_settings.coloring && EffSegments() == Segments::Off && !BackdropClear() ? g_settings.blur : 0;
    // None under the glow at the edge, or under a bar fading (the blur doesn't fade with it), a new screen's
    // bar coming in faded too.
    BarArrival const& arrival = g_active->arrive;
    bool away = g_edgeHint || (FadeOn() && (!g_shown || g_slide < 1)) || arrival.pending || (arrival.running && arrival.fade);
    if (away) blur = 0;
    if (CssActive()) {
        // A YASB theme's bar: acrylic when its config asks for the blur, and its window corners and border.
        blur = g_css.blur && !away ? 2 : 0;
        DWORD corners = g_css.roundCorners ? 2 : 1;  // DWMWCP_ROUND, DWMWCP_DONOTROUND
        DwmSetWindowAttribute(wnd, 33 /* DWMWA_WINDOW_CORNER_PREFERENCE */, &corners, sizeof(corners));
        COLORREF border = 0xFFFFFFFE;  // DWMWA_COLOR_NONE
        std::wstring want = css::Lower(g_css.borderColor);
        css::Rgba c;
        if (want == L"system") border = 0xFFFFFFFF;  // DWMWA_COLOR_DEFAULT
        else if (want.size() > 1 && want[0] == L'#' && css::ParseColor(want, &c)) border = RGB(c.r, c.g, c.b);
        DwmSetWindowAttribute(wnd, 34 /* DWMWA_BORDER_COLOR */, &border, sizeof(border));
    }
    // ACCENT_ENABLE_BLURBEHIND (3), ACCENT_ENABLE_ACRYLICBLURBEHIND (4), or ACCENT_DISABLED.
    // The drawn tint carries the colour; the accent's own is near-clear.
    AccentPolicy policy{blur == 2 ? 4 : blur == 1 ? 3 : 0, 0, 0x01000000, 0};
    CompositionData data{19 /* WCA_ACCENT_POLICY */, &policy, sizeof(policy)};
    set(wnd, &data);
}

// A YASB theme's button: its callback. The built-in ones always work; "exec ..." opens Windows settings pages, and
// runs any command only when the setting allows it (a theme is someone else's file).
void RunCssAction(HWND wnd, std::wstring action, RECT const* anchor = nullptr) {
    action = css::Trim(action);
    if (action.empty()) return;
    if (action == L"start_menu" || action == L"exec start_menu") {
        if (!ToggleStartMenu()) SendKeys({VK_LWIN});
    } else if (action == L"power_menu") {
        if (!PowerUserMenu(wnd)) SendKeys({VK_LWIN, 'X'});
    } else if (action == L"notification_center") {
        OpenNotificationCentre(anchor ? *anchor : g_clockRect);
    } else if (action == L"quick_settings") {
        SendKeys({VK_LWIN, 'A'});
    } else if (action == L"search") {
        SendKeys({VK_LWIN, 'S'});
    } else if (action == L"widget") {
        SendKeys({VK_LWIN, 'W'});
    } else if (action.rfind(L"popup:", 0) == 0) {
        // The theme's popups: the calendar, the volume menu, the power menu. Opened beside the widget that was pressed.
        RECT r = anchor ? *anchor : RECT{};
        POINT origin{0, 0};
        ClientToScreen(wnd, &origin);
        OffsetRect(&r, origin.x, origin.y);
        std::wstring which = action.substr(6);
        CssPopupToggle(which == L"calendar" ? CssPopupKind::Calendar : which == L"volume" ? CssPopupKind::Volume : CssPopupKind::Power, r, (double)g_scale);
    } else if (action.rfind(L"desktop:", 0) == 0) {
        CssGoToDesktop(_wtoi(action.c_str() + 8));  // a workspace button: that virtual desktop (straight there where Windows allows)
        RelayoutBars(wnd);
    } else if (action.rfind(L"toggle_label:", 0) == 0) {
        std::wstring widget = action.substr(13);
        if (!g_css.alt.erase(widget)) g_css.alt.insert(widget);
        RelayoutBars(wnd);
    } else if (action.rfind(L"media:", 0) == 0) {
        std::wstring which = action.substr(6);
        CssMediaKey(which == L"prev" ? VK_MEDIA_PREV_TRACK : which == L"next" ? VK_MEDIA_NEXT_TRACK : VK_MEDIA_PLAY_PAUSE);
    } else if (action == L"toggle_label" || action == L"toggle_menu" || action == L"do_nothing") {
        // no widget name to go by: nothing to switch here
    } else {
        // "exec <command>", or just a command line.
        std::wstring command = css::Trim(action.rfind(L"exec ", 0) == 0 ? action.substr(5) : action);
        size_t at = command.find(L"ms-settings:");
        if (at != std::wstring::npos) {
            size_t end = command.find_first_of(L" \"'", at);
            ShellExecuteW(nullptr, L"open", command.substr(at, end == std::wstring::npos ? end : end - at).c_str(), nullptr, nullptr, SW_SHOWNORMAL);
        } else if (g_settings.yasbCommands) {
            ShellExecuteW(nullptr, L"open", L"cmd.exe", (L"/c " + command).c_str(), nullptr, SW_HIDE);
        }
    }
}

bool WindowTouchesBar();
void MakeTipFont();
LRESULT CALLBACK WndProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam);

////////////////////////////////////////////////////////////////////////////////
// The hiding animation. The bar's finished picture is moved and faded by the
// compositor (UpdateLayeredWindow with only a position and an opacity), so a
// frame costs no drawing; frames come in step with the screen's refresh, and
// only while something moves. Blurring the bar as it goes was tried on paper
// and left out: it needs the picture drawn again every frame.

double NowMs() {
    static LARGE_INTEGER freq = [] {
        LARGE_INTEGER f;
        QueryPerformanceFrequency(&f);
        return f;
    }();
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return now.QuadPart * 1000.0 / freq.QuadPart;
}

// Where a reload's time goes, when the test bar asks (TB_perf); otherwise one test.
bool g_perfTrace = false;
double g_perfLast = 0;
void PerfMark(PCWSTR what) {
    if (!g_perfTrace) return;
    double now = NowMs();
    if (g_perfLast) Wh_Log(L"  %ls: %.1f ms", what, now - g_perfLast);
    g_perfLast = now;
}

// Smoothing: t from 0 to 1, eased (past 1 on the way, with a curve that overshoots).
double ShyEase(double t) {
    if (g_settings.motionBar.set) return motion::Ease(g_settings.motionBar, t);
    t = std::clamp(t, 0.0, 1.0);
    switch (g_settings.shyEase) {
        case 0: return t;
        case 1: return 0.5 - 0.5 * std::cos(t * 3.14159265358979);  // gentle: sine
        case 2: return 1 - std::pow(1 - t, 4);                        // soft landing: quartic out
        default: return t * t * t * (t * (t * 6 - 15) + 10);          // smooth: smootherstep
    }
}

// The animation's length (its own setting: Windows' animation effects are often off on purpose, for Windows' own).
double ShyMs() {
    if (g_settings.motionBar.set) return motion::Ms(g_settings.motionBar);
    return g_settings.shyAnim ? g_settings.shyDuration : 0;
}

// Something fades in at the edge once the bar is away: the glow, or the sliver after a fade.
bool NeedsHint() { return g_settings.shyEdge == 1 || (g_settings.shyEdge == 0 && FadeOn()); }

BYTE ShyAlpha() {
    double a;
    if (!g_shown && g_slide <= 0 && NeedsHint()) a = g_hint;
    else if (FadeOn()) a = g_slide;
    else if (g_settings.shyEdge == 0) a = 1;
    else a = std::min(1.0, g_slide * 6);  // sliding with no sliver kept: gone over the last bit
    return (BYTE)std::lround(255 * std::clamp(a, 0.0, 1.0));
}

// How much of the bar a screen's arrival shows: none until it sets off, then as it fades in.
double ArriveAlpha() {
    BarArrival const& a = g_active->arrive;
    return a.pending ? 0 : a.running && a.fade ? std::clamp(a.level, 0.0, 1.0) : 1;
}

// The whole bar's opacity: the hiding animation's, times fading when idle (and a new screen's bar coming in).
BYTE BarAlpha() { return (BYTE)std::lround(ShyAlpha() * std::clamp(g_idle.level, 0.0, 1.0) * ArriveAlpha()); }

// The bar where its arrival has it: on the way in from its edge, at its opacity, in one compositor update.
void ArriveApply(HWND wnd) {
    BarArrival const& a = g_active->arrive;
    POINT at = a.home;
    if (a.running && a.slide) {
        RECT out = BarRect(false);
        if (CssActive()) out = CssBody(out);
        double away = 1 - a.level;
        if (Vertical()) at.x += (LONG)std::lround((out.left - a.home.x) * away);
        else at.y += (LONG)std::lround((out.top - a.home.y) * away);
    }
    g_barAlpha = BarAlpha();
    BLENDFUNCTION blend{AC_SRC_OVER, 0, g_barAlpha, AC_SRC_ALPHA};
    UpdateLayeredWindow(wnd, nullptr, a.slide ? &at : nullptr, nullptr, nullptr, nullptr, 0, &blend, ULW_ALPHA);
}

// While it's away the bar takes no clicks (the pointer at the edge still brings it back).
void ClickThrough(HWND wnd, bool on) {
    LONG_PTR ex = GetWindowLongPtrW(wnd, GWL_EXSTYLE);
    LONG_PTR want = on ? ex | WS_EX_TRANSPARENT : ex & ~(LONG_PTR)WS_EX_TRANSPARENT;
    if (want != ex) SetWindowLongPtrW(wnd, GWL_EXSTYLE, want);
}

void PacerProc(void* arg) {
    auto* held = (std::shared_ptr<FramePacer>*)arg;
    std::shared_ptr<FramePacer> pacer = *held;
    delete held;
    while (pacer->run) {
        if (FAILED(DwmFlush())) Sleep(8);  // no compositor frame to wait for
        if (!pacer->run) break;
        if (!pacer->posted.exchange(true) && !PostMessageW(pacer->wnd, WM_APP_SHYFRAME, 0, 0)) break;  // the bar is gone
    }
}

void StopPacer(HWND wnd) {
    if (g_pacer) g_pacer->run = false;
    g_pacer.reset();
    KillTimer(wnd, kTimerShy);
    g_shyRunning = false;
    g_aura.running = false;
    g_idle.running = false;  // (the next idle check picks up where it stopped)
    if (BarArrival& a = g_active->arrive; a.running) {
        // Cut short: in place, as it would have ended.
        a.running = false;
        a.level = 1;
        if (IsWindow(wnd)) {
            ArriveApply(wnd);
            SetBarBackdrop(wnd);
        }
        a.slide = false;
    }
}

void StartPacer(HWND wnd) {
    if (g_pacer) return;
    auto pacer = std::make_shared<FramePacer>();
    pacer->wnd = wnd;
    auto* held = new std::shared_ptr<FramePacer>(pacer);
    if (BgWork(PacerProc, held)) {
        g_pacer = pacer;
    } else {
        delete held;
        SetTimer(wnd, kTimerShy, USER_TIMER_MINIMUM, nullptr);
    }
}

// The bar where the animation has it: slid between its places, faded, or both, in one compositor update.
void ShyApply(HWND wnd) {
    RECT wr;
    GetWindowRect(wnd, &wr);
    int w = wr.right - wr.left, h = wr.bottom - wr.top;
    RECT in = BarRect(true), out = BarRect(false);
    if (CssActive()) {
        in = CssBody(in);
        out = CssBody(out);
    }
    POINT at;
    if (g_edgeHint) {
        // Only the glow's rows in view at the screen's edge.
        RECT m = BarRect(true);
        int depth = Px(g_settings.shyGlowSize);
        at = {in.left, in.top};
        switch (g_settings.edge) {
            case Edge::Top: at.y = m.top + depth - h; break;
            case Edge::Bottom: at.y = m.bottom - depth; break;
            case Edge::Left: at.x = m.left + depth - w; break;
            case Edge::Right: at.x = m.right - depth; break;
        }
    } else {
        double s = SlideOn() ? g_slide : g_slide > 0 ? 1 : 0;
        at = {(LONG)std::lround(out.left + (in.left - out.left) * s), (LONG)std::lround(out.top + (in.top - out.top) * s)};
    }
    g_barAlpha = BarAlpha();
    BLENDFUNCTION blend{AC_SRC_OVER, 0, g_barAlpha, AC_SRC_ALPHA};
    UpdateLayeredWindow(wnd, nullptr, &at, nullptr, nullptr, nullptr, 0, &blend, ULW_ALPHA);
}

// A screen just added: its new bar comes in on the "monitorAdded" line once it's set up (ApplySettings),
// slid in from its edge (no style, or a slide), faded (a bar that hides only fades: its hiding moves it),
// or both. Bars made at startup, or for screens already there, just appear.
bool ArriveOn() {
    auto const& m = g_settings.motionScreenAdded;
    return m.set && m.on && motion::Ms(m) > 0;
}

void ArriveStart(HWND wnd) {
    BarArrival& a = g_active->arrive;
    if (!a.pending) return;
    a.pending = false;
    std::wstring style = motion::StyleWord(g_settings.motionScreenAdded);
    bool slideOnly = style.empty() || style == L"slide" || style == L"slidevert";
    a.slide = (slideOnly || style == L"slidefade" || style == L"slidefadevert") && !Hides();
    a.fade = !(a.slide && slideOnly);
    RECT wr;
    GetWindowRect(wnd, &wr);
    a.home = {wr.left, wr.top};
    a.level = 0;
    a.start = NowMs();
    a.running = ArriveOn();
    SetBarBackdrop(wnd);  // (the blur only once it's no longer fading)
    ArriveApply(wnd);
    if (a.running) StartPacer(wnd);
    else a.slide = false;
}

void ArriveFrame(HWND wnd) {
    BarArrival& a = g_active->arrive;
    auto const& m = g_settings.motionScreenAdded;
    double ms = std::min(motion::Ms(m), 3000.0);
    double t = ms > 0 ? (NowMs() - a.start) / ms : 1;
    a.level = t >= 1 ? 1 : motion::Ease(m, t);
    a.running = t < 1;
    ArriveApply(wnd);
    if (a.running) return;
    a.slide = false;
    SetBarBackdrop(wnd);  // (the blur back, after a fade)
}

// The slide has ended: in view (the blur back), or away (no clicks, and the hint starts to fade in).
void ShyArrived(HWND wnd, double now) {
    if (g_shown) {
        SetBarBackdrop(wnd);
        return;
    }
    ClickThrough(wnd, true);
    g_hint = 0;
    g_hintStart = now;
    if (g_settings.shyEdge == 1) {
        g_edgeHint = true;
        g_barAlpha = 0;
        Redraw(wnd);
        SetBarBackdrop(wnd);
    }
}

void ShyFrame(HWND wnd) {
    double now = NowMs(), to = g_shown ? 1 : 0;
    bool busy = false;
    if (g_slide != to) {
        double span = ShyMs() * std::abs(to - g_slideFrom);
        double t = span > 0 ? (now - g_slideStart) / span : 1;
        g_slide = t >= 1 ? to : g_slideFrom + (to - g_slideFrom) * ShyEase(t);
        busy = t < 1;
        if (!busy) ShyArrived(wnd, now);
    }
    if (!g_shown && g_slide <= 0 && NeedsHint() && g_hint < 1) {
        double span = std::min(ShyMs(), 180.0);
        double t = span > 0 ? (now - g_hintStart) / span : 1;
        g_hint = t >= 1 ? 1 : ShyEase(t);
        busy |= t < 1;
    }
    ShyApply(wnd);
    if (!busy) {
        g_shyRunning = false;
        if (!g_aura.running && !g_idle.running && !g_active->arrive.running) StopPacer(wnd);  // (the aura, idle fade or a new screen's bar may still be using the frame clock)
    }
}

// A frame of the dynamic aura: its centre moves towards the pointer, all the way at once
// without smoothing, else by the share the smoothing's half-life says for the time passed.
void AuraFrame(HWND wnd) {
    double now = NowMs();
    double dt = g_aura.last ? now - g_aura.last : 1000.0 / 60;
    g_aura.last = now;
    int halfLife = g_settings.auraFollow;
    double k = halfLife > 0 ? 1 - std::pow(0.5, dt / halfLife) : 1;
    g_aura.x += (g_aura.toX - g_aura.x) * k;
    g_aura.y += (g_aura.toY - g_aura.y) * k;
    if (std::abs(g_aura.toX - g_aura.x) < 0.25 && std::abs(g_aura.toY - g_aura.y) < 0.25) {
        g_aura.x = g_aura.toX;
        g_aura.y = g_aura.toY;
        g_aura.running = false;
    }
    Redraw(wnd);
}

// The pointer moved over a button with the dynamic aura on: the next screen frame draws the
// aura towards it (a burst of moves is one redraw). Onto a new button, it starts where the pointer is.
void AuraFollowPointer(HWND wnd, POINT pt, bool jump) {
    g_aura.toX = pt.x;
    g_aura.toY = pt.y;
    if (jump || !g_aura.placed) {
        g_aura.x = pt.x;
        g_aura.y = pt.y;
        g_aura.placed = true;
    }
    if (!g_aura.running) {
        g_aura.running = true;
        g_aura.last = 0;
    }
    StartPacer(wnd);
}

void HidePreview();

// The bar sets off to go or to come back, from wherever it is now.
void SlideTo(HWND wnd, bool shown) {
    if (g_shown == shown) return;
    g_shown = shown;
    g_wantSince = 0;
    if (shown) {
        ClickThrough(wnd, false);
        g_hint = 0;
        if (g_edgeHint) {
            g_edgeHint = false;
            g_barAlpha = BarAlpha();
            Redraw(wnd);
        }
        g_underFullscreen = false;
        SetWindowPos(wnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
    } else {
        HidePreview();
    }
    SetBarBackdrop(wnd);
    g_slideFrom = g_slide;
    g_slideStart = NowMs();
    g_shyRunning = true;
    if (ShyMs() > 0) StartPacer(wnd);
    ShyFrame(wnd);
}

// Automatic hiding as set (or as the hotkey turned it), and this bar redone.
bool FullscreenInFront(HWND bar);

void ApplyHiding(HWND wnd) {
    SetAppBar(wnd, !Hides());
    PerfMark(L"    app bar space");
    StopPacer(wnd);
    // A full-screen program in front: redone out of its way, not shown over it for a moment (the bar flashed on
    // and off over a full-screen video while something kept redoing it).
    bool away = FullscreenInFront(wnd);
    g_shown = !(away && Hides());
    g_slide = g_slideFrom = g_shown ? 1 : 0;
    g_hint = 0;
    g_wantSince = 0;
    g_edgeHint = false;
    g_idle = {};  // awake
    g_barAlpha = BarAlpha();  // (255, unless a new screen's bar waits to come in)
    g_underFullscreen = away;  // (on top otherwise: PlaceBar)
    ClickThrough(wnd, !g_shown);
    if (Hides()) SetTimer(wnd, kTimerAutoHide, 100, nullptr);
    else KillTimer(wnd, kTimerAutoHide);
    SetBarBackdrop(wnd);
    PerfMark(L"    backdrop");
    PlaceBar(wnd);
    PerfMark(L"    placed");
    Layout(wnd);
    PerfMark(L"    laid out");
    Redraw(wnd);
    PerfMark(L"    drawn");
    SyncPreview(wnd);
}

void ApplyHidingToBars(HWND fallback) {
    if (g_bars.empty()) {
        ApplyHiding(fallback);
        return;
    }
    std::vector<BarState*> bars = g_bars;
    for (BarState* bar : bars) {
        if (std::find(g_bars.begin(), g_bars.end(), bar) == g_bars.end() || !bar->hwnd) continue;
        BarScope scope(bar);
        ApplyHiding(bar->hwnd);
    }
}

// The DPI of the screen a bar is on: the bar is put on its screen first, since a window's DPI is where it is.
UINT BarDpi(HWND bar) {
    MONITORINFO mi{sizeof(mi)};
    if (g_curMonitor && GetMonitorInfoW(g_curMonitor, &mi))
        SetWindowPos(bar, nullptr, mi.rcMonitor.left, mi.rcMonitor.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
    UINT dpi = GetDpiForWindow(bar);
    return dpi ? dpi : 96;
}

BOOL CALLBACK CollectMonitor(HMONITOR monitor, HDC, LPRECT, LPARAM param) {
    ((std::vector<HMONITOR>*)param)->push_back(monitor);
    return TRUE;
}

// The screens as SyncBars last saw them, by device name (Windows may give the same screens new handles).
std::vector<std::wstring> g_screenNames;
bool g_screensSeen = false;

std::wstring ScreenName(HMONITOR monitor) {
    MONITORINFOEXW mi{};
    mi.cbSize = sizeof(mi);
    return GetMonitorInfoW(monitor, &mi) ? std::wstring(mi.szDevice) : std::wstring();
}

// One bar for every screen but the main one when there are to be several
// (the main bar follows the main screen by itself); the ones for screens that
// are gone, or no longer wanted, are closed.
void SyncBars() {
    if (g_bars.empty()) return;  // not a running bar (a test bar)
    // A menu is open on one of them: not now.
    if (g_inMenu) {
        SetTimer(g_bars[0]->hwnd, kTimerScreens, 400, nullptr);
        return;
    }
    // Which screens are new since last time: their bars come in (motion.monitorAdded), not the first time.
    std::vector<HMONITOR> all;
    EnumDisplayMonitors(nullptr, nullptr, CollectMonitor, (LPARAM)&all);
    std::vector<std::wstring> before = std::move(g_screenNames);
    g_screenNames.clear();
    for (HMONITOR m : all) g_screenNames.push_back(ScreenName(m));
    bool seenBefore = g_screensSeen;
    g_screensSeen = true;
    std::vector<HMONITOR> screens;
    if (g_settings.multiBars) EnumDisplayMonitors(nullptr, nullptr, CollectMonitor, (LPARAM)&screens);
    HMONITOR main = MonitorFromPoint({0, 0}, MONITOR_DEFAULTTOPRIMARY);
    screens.erase(std::remove(screens.begin(), screens.end(), main), screens.end());
    for (size_t i = g_bars.size(); i-- > 1;) {
        BarState* bar = g_bars[i];
        if (std::find(screens.begin(), screens.end(), bar->monitor) != screens.end()) continue;
        if (bar->hwnd) DestroyWindow(bar->hwnd);  // (its WM_DESTROY tidies up)
        g_bars.erase(g_bars.begin() + i);
        delete bar;
    }
    for (HMONITOR screen : screens) {
        bool have = false;
        for (size_t i = 1; i < g_bars.size(); i++) have = have || g_bars[i]->monitor == screen;
        if (have) continue;
        auto* bar = new BarState();
        bar->monitor = screen;
        std::wstring name = ScreenName(screen);
        bar->arrive.pending = seenBefore && ArriveOn() && std::find(before.begin(), before.end(), name) == before.end();
        g_bars.push_back(bar);
        HWND wnd = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE | WS_EX_LAYERED | WS_EX_ACCEPTFILES,
                                   kWindowClass, L"Tourne taskbar", WS_POPUP, 0, 0, 1, 1, nullptr, nullptr, g_instance, bar);
        if (!wnd) {
            g_bars.pop_back();
            delete bar;
            continue;
        }
        KeepOutOfPeek(wnd);
    }
}

// Everything read again (settings, styles, the Start menu), or, when only screens changed
// (plugged in, taken away, or waking from sleep), just the bars: the Start menu keeps its
// window, lists and icons, so the first open after waking is as quick as any.
// What the bar's look is taken from outside its settings: the visual style, dark or light,
// and the accent. Windows tells windows these changed when they may not have (after sleep,
// or unlocking), and the bar reloads only when one really did.
std::wstring g_lookSignature;

std::wstring LookSignature() {
    WCHAR style[MAX_PATH] = L"";
    GetCurrentThemeName(style, ARRAYSIZE(style), nullptr, 0, nullptr, 0);
    std::wstring s = style;
    auto value = [&](PCWSTR key, PCWSTR name) {
        DWORD v = 0, size = sizeof(v);
        if (RegGetValueW(HKEY_CURRENT_USER, key, name, RRF_RT_REG_DWORD, nullptr, &v, &size) != ERROR_SUCCESS) v = 0xFFFFFFFF;
        s += L"|" + std::to_wstring(v);
    };
    constexpr PCWSTR kPersonalize = L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize";
    value(kPersonalize, L"AppsUseLightTheme");
    value(kPersonalize, L"SystemUsesLightTheme");
    value(kPersonalize, L"ColorPrevalence");
    value(L"Software\\Microsoft\\Windows\\DWM", L"AccentColor");
    value(L"Software\\Microsoft\\Windows\\DWM", L"ColorizationColor");
    value(L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Accent", L"AccentColorMenu");
    return s;
}

void ApplySettings(HWND wnd, bool screensOnly = false, const wchar_t* why = L"started") {
    Wh_Log(L"Settings applied: %ls%ls", why, screensOnly ? L" (screens only)" : L"");
    PerfMark(L"(start)");
    g_menuSampled = nullptr;  // the menu theme's colours are sampled again
    int oldIconPx = IconPixels();
    if (!screensOnly) StartBeforeTheme();
    PerfMark(L"start menu closed");
    g_settings = LoadSettings();
    g_fullscreenPid = 0;  // the program list may have changed
    PerfMark(L"settings read");
    CssApply();  // a YASB theme, when one is chosen: it sets the bar's edge and thickness
    HideSystemTaskbar(wnd, g_settings.hideSystemTaskbar);
    TurnStartAllBackOff(g_settings.disableStartAllBack);
    PerfMark(L"yasb, system taskbar, SAB");
    // Button icons may have changed: yours are read again.
    for (size_t i = g_iconCache.size(); i-- > 0;)
        if (g_iconCache[i].id.starts_with(L"custom:")) {
            if (g_iconCache[i].icon) DestroyIcon(g_iconCache[i].icon);
            g_iconCache.erase(g_iconCache.begin() + i);
        }
    g_iconColors.clear();
    if (!screensOnly) PublishChoices();
    PerfMark(L"choices published");
    // The Start menu's look can live in the bar's theme file, which this replaces: let it go first, even when
    // only the screens changed (its handles would point into the closed file, and opening Start crashed).
    if (screensOnly) CloseStartLook();
    g_themed = !g_settings.style.empty() && g_theme.Load(g_settings.style);
    if (!g_themed) g_theme.Close();
    if (screensOnly) LoadStartLook();
    PerfMark(L"visual style loaded");
    g_orb.Load(g_settings.orb);
    PerfMark(L"orb loaded");
    UINT dpi = GetDpiForWindow(wnd);
    g_scale = std::max(1, (int)(dpi + 48) / 96);

    NONCLIENTMETRICSW ncm{sizeof(ncm)};
    SystemParametersInfoForDpi(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0, dpi);
    if (g_font) DeleteObject(g_font);
    g_font = CreateFontIndirectW(&ncm.lfMessageFont);
    MakeTipFont();
    PerfMark(L"fonts");
    ApplyMenuMotion();
    if (IsMainBar()) {
        PublishMenuLook();
#ifdef TD_PREFIX
        // The desktop's menu made ahead (desktop-menu.cpp): Tourne Desktop's desktop only.
        static bool listening;
        if (g_settings.desktopMenuReady && !listening) listening = AddClipboardFormatListener(wnd);
        ReadyMenuSoon(wnd, 2000);
        WarmDesktopMenusOnce();
#endif
    }
    if (!screensOnly) StartAfterTheme();
    PerfMark(L"start menu's style and settings");
    g_lookSignature = LookSignature();
    LoadClock(dpi);
    PerfMark(L"clock");
    g_trayHash = 0;  // placements or margins may have changed: rebuild
    FetchTray(wnd);
    PerfMark(L"tray");
    // The desktops pager: its choices and the desktops, before the bars are laid out (it may take room there).
    LoadPagerChoices();
    g_pagerDesktops = g_settings.desktops ? ReadPagerDesktops() : PagerDesktops();
    PlaceFloatingPager();
    // Icons are read for the size they're drawn at.
    if (IconPixels() != oldIconPx || g_shortcuts.empty()) {
        RefreshShortcuts(true);
        for (auto& w : g_wins) FetchIcon(w);
    }
    PerfMark(L"icons");

    g_hideFlip = false;
    UnregisterHotKey(wnd, kHotkeyHide);
    if (g_settings.hotkeyVk) RegisterHotKey(wnd, kHotkeyHide, g_settings.hotkeyMods | MOD_NOREPEAT, g_settings.hotkeyVk);
    UnregisterHotKey(wnd, kHotkeyMenu);
    if (g_settings.menuEnabled && g_settings.menuHotkeyVk)
        RegisterHotKey(wnd, kHotkeyMenu, g_settings.menuHotkeyMods | MOD_NOREPEAT, g_settings.menuHotkeyVk);
    RegisterDesktopKeys(wnd);
    // The bars: one per screen when set, each redone.
    SyncBars();
    PerfMark(L"bars synced");
    RescanWindows();
    PerfMark(L"windows scanned");
    auto applyBar = [](HWND bar) {
        if (!IsMainBar()) {
            // Another screen may be at another scale: its own sizes, fonts and clock.
            UINT dpi = BarDpi(bar);
            g_scale = std::max(1, (int)(dpi + 48) / 96);
            NONCLIENTMETRICSW ncm{sizeof(ncm)};
            SystemParametersInfoForDpi(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0, dpi);
            if (g_font) DeleteObject(g_font);
            g_font = CreateFontIndirectW(&ncm.lfMessageFont);
            MakeTipFont();
            LoadClock(dpi);
        }
        StyleTooltip();
        PerfMark(L"  tooltip styled");
        g_touching = false;
        bool dynamic = g_settings.dynamicTransparency || g_settings.dynamicSegments;
        if (dynamic) {
            g_touching = WindowTouchesBar();
            SetTimer(bar, kTimerDynamic, 250, nullptr);
        } else {
            KillTimer(bar, kTimerDynamic);
        }
        ApplyHiding(bar);
        if (g_settings.idleFade) SetTimer(bar, kTimerIdle, 100, nullptr);
        else KillTimer(bar, kTimerIdle);
        PerfMark(L"  placed (hiding applied)");
    };
    if (g_bars.empty()) {
        applyBar(wnd);
        return;
    }
    std::vector<BarState*> bars = g_bars;
    for (BarState* bar : bars) {
        if (std::find(g_bars.begin(), g_bars.end(), bar) == g_bars.end() || !bar->hwnd) continue;
        BarScope scope(bar);
        applyBar(bar->hwnd);
        ArriveStart(bar->hwnd);  // (a bar for a screen just added comes in, now it's set up)
    }
    PerfMark(L"bars placed and drawn");
}

// A window over the bar's space, for dynamic shyness.
bool WindowTouchesBar() {
    RECT bar = BarRect(true);
    // A window only reaching a little into it (as ones placed for another
    // taskbar do) doesn't count, as set.
    int tolerance = Px(g_settings.shyTolerance);
    InflateRect(&bar, -tolerance, -tolerance);
    if (IsRectEmpty(&bar)) return false;
    for (auto const& w : g_wins) {
        if (IsIconic(w.hwnd) || !IsWindowVisible(w.hwnd)) continue;
        // Cloaked: on another virtual desktop, or a suspended Store app.
        DWORD cloaked = 0;
        if (SUCCEEDED(DwmGetWindowAttribute(w.hwnd, DWMWA_CLOAKED, &cloaked, sizeof(cloaked))) && cloaked) continue;
        RECT r{}, overlap;
        if (FAILED(DwmGetWindowAttribute(w.hwnd, DWMWA_EXTENDED_FRAME_BOUNDS, &r, sizeof(r)))) GetWindowRect(w.hwnd, &r);
        if (IntersectRect(&overlap, &r, &bar)) return true;
    }
    return false;
}

// Dynamic transparency, segments and aura follow whether a window touches the bar.
void DynamicTick(HWND wnd) {
    bool touching = WindowTouchesBar();
    if (touching == g_touching) return;
    g_touching = touching;
    SetBarBackdrop(wnd);
    Layout(wnd);
    Redraw(wnd);
}

bool BarWantsAttention();

// A program in front on this bar's screen that the bar keeps out of the way
// of: one filling the whole screen (exclusive or borderless full screen), or
// one named in the settings. Windows' own taskbar, which ExplorerPatcher and
// StartAllBack keep, does this through the app bar notice ABN_FULLSCREENAPP;
// a hiding bar gives up its app bar (it reserves no space), so it looks itself.
bool FullscreenInFront(HWND bar) {
    HWND fg = GetForegroundWindow();
    if (!fg || fg == bar) return false;
    DWORD pid = 0;
    GetWindowThreadProcessId(fg, &pid);
    if (pid == GetCurrentProcessId()) return false;  // the suite's own Start menu, flyouts and previews
    WCHAR cls[64] = L"";
    GetClassNameW(fg, cls, ARRAYSIZE(cls));
    for (PCWSTR shell : {L"Progman", L"WorkerW", L"Shell_TrayWnd", L"Shell_SecondaryTrayWnd"})
        if (!wcscmp(cls, shell)) return false;  // the desktop and Windows' taskbars
    // The bar's own screen, as BarRect has it: a hidden bar hangs mostly off
    // its screen, so the screen nearest to it can be the neighbouring one.
    HMONITOR screen = g_curMonitor ? g_curMonitor : MonitorFromPoint({0, 0}, MONITOR_DEFAULTTOPRIMARY);
    if (MonitorFromWindow(fg, MONITOR_DEFAULTTONULL) != screen) return false;
    if (LeftAlone(fg)) return true;  // a program Tourne leaves alone (a game): kept away from, full screen or not
    if (!g_settings.fullscreenPrograms.empty()) {
        // The program's name is looked up once per process in front, not on
        // every tick (LoadSettings clears it when the list changes).
        if (pid != g_fullscreenPid) {
            g_fullscreenPid = pid;
            g_fullscreenNamed = false;
            WCHAR path[MAX_PATH];
            DWORD size = ARRAYSIZE(path);
            if (HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid)) {
                if (QueryFullProcessImageNameW(process, 0, path, &size)) {
                    std::wstring name = PathFindFileNameW(path);
                    CharLowerBuffW(name.data(), (DWORD)name.size());
                    for (auto const& program : g_settings.fullscreenPrograms)
                        if (name == program) g_fullscreenNamed = true;
                }
                CloseHandle(process);
            }
        }
        if (g_fullscreenNamed) return true;
    }
    if (!g_settings.fullscreenStayAway) return false;
    // A maximised window that can be sized or has a title bar can cover the
    // screen too, once nothing reserves the edge; it isn't full screen.
    LONG style = GetWindowLongW(fg, GWL_STYLE);
    if (IsZoomed(fg) && (style & (WS_CAPTION | WS_THICKFRAME))) return false;
    RECT wr;
    MONITORINFO mi{sizeof(mi)};
    if (!GetWindowRect(fg, &wr) || !GetMonitorInfoW(screen, &mi)) return false;
    if (wr.left > mi.rcMonitor.left || wr.top > mi.rcMonitor.top || wr.right < mi.rcMonitor.right || wr.bottom < mi.rcMonitor.bottom)
        return false;
    // (a window with a title bar just bigger than its screen, left floating at its size, isn't full screen)
    return EqualRect(&wr, &mi.rcMonitor) || !(style & WS_CAPTION);
}

// Behind a full-screen program, or back on top once it's gone.
void SetUnderFullscreen(HWND wnd, bool under) {
    if (g_underFullscreen == under) return;
    g_underFullscreen = under;
    Wh_Log(L"Bar %ls a full-screen program", under ? L"behind" : L"back over");
    SetWindowPos(wnd, under ? HWND_BOTTOM : HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}

void AutoHideTick(HWND wnd) {
    // In view and still: it takes clicks. (A slide cut short could leave a shown bar letting clicks through to
    // what's under it, the tray dead until it hid and came back.)
    if (g_shown && !g_shyRunning && g_slide >= 1 && (GetWindowLongPtrW(wnd, GWL_EXSTYLE) & WS_EX_TRANSPARENT)) {
        Wh_Log(L"Bar was shown but letting clicks through: fixed");
        ClickThrough(wnd, false);
    }
    POINT pt;
    GetCursorPos(&pt);
    // A full-screen program in front: the bar goes and stays away, whatever the
    // pointer does, unless the Start menu was asked for.
    bool startOpen = IsMainBar() && g_startWnd && IsWindowVisible(g_startWnd);
    if (!startOpen && FullscreenInFront(wnd)) {
        g_wantSince = 0;
        if (g_shown && !g_inMenu && g_pressed == -2) SlideTo(wnd, false);
        if (!g_shown) SetUnderFullscreen(wnd, true);
        return;
    }
    SetUnderFullscreen(wnd, false);
    RECT shownRect = BarRect(true);
    RECT peek = BarRect(false);
    bool over = PtInRect(&shownRect, pt) != 0;
    // The bar's own popups count as the bar: its previews, and the overflow
    // flyout it opened.
    RECT popup;
    if (g_preview.wnd && IsWindowVisible(g_preview.wnd) && GetWindowRect(g_preview.wnd, &popup) && PtInRect(&popup, pt))
        over = true;
    WCHAR cls[64];
    HWND fg = GetForegroundWindow();
    if (fg && GetClassNameW(fg, cls, ARRAYSIZE(cls)) && !wcsncmp(cls, kFlyoutClassPrefix, wcslen(kFlyoutClassPrefix)))
        over = true;
    // The reveal zone: the sliver, reaching further in from the edge as set.
    int zone = Px(g_settings.shyRevealZone);
    switch (g_settings.edge) {
        case Edge::Bottom: peek.top -= zone; break;
        case Edge::Top: peek.bottom += zone; break;
        case Edge::Left: peek.right += zone; break;
        case Edge::Right: peek.left -= zone; break;
    }
    bool reaching = PtInRect(&peek, pt) != 0;
    bool touching = g_settings.shy && WindowTouchesBar();
    // Back, and kept, for a window wanting attention or the Start menu, as set.
    bool wanted = (g_settings.shyRevealAttention && BarWantsAttention()) ||
                  (g_settings.shyRevealStart && IsMainBar() && g_startWnd && IsWindowVisible(g_startWnd));
    // Plain automatic hiding hides whenever the pointer is away; shy hiding
    // only while a window is in the way. Each waits its delay first, and the
    // pointer at the edge brings the bar back at once.
    bool change;
    int delay;
    if (g_shown) {
        change = !over && !wanted && !g_inMenu && g_pressed == -2 && (!g_settings.shy || touching);
        delay = g_settings.shyHideDelay;
    } else if (reaching || wanted) {
        change = true;
        delay = 0;
    } else {
        change = g_settings.shy && !touching;
        delay = g_settings.shyShowDelay;
    }
    // Nothing changes while a window is being dragged or sized past the bar.
    if (change && delay && g_settings.shy) {
        GUITHREADINFO gti{sizeof(gti)};
        if (GetGUIThreadInfo(0, &gti) && (gti.flags & GUI_INMOVESIZE)) change = false;
    }
    if (!change) {
        g_wantSince = 0;
        return;
    }
    ULONGLONG now = GetTickCount64();
    if (!g_wantSince) g_wantSince = now;
    if (now - g_wantSince + 50 < (ULONGLONG)delay) return;  // (within half a tick)
    Wh_Log(L"Bar %ls after %llu ms (delay %d): pointer %ld,%ld%ls%ls", g_shown ? L"going" : L"back", now - g_wantSince, delay, pt.x, pt.y, reaching ? L", at the edge" : L"",
           touching ? L", a window touches it" : L"");
    SlideTo(wnd, !g_shown);
}

// A window on this bar wants attention (its button flashes).
bool BarWantsAttention() {
    for (auto const& b : g_buttons)
        for (HWND h : b.windows)
            if (Win const* w = FindWin(h); w && w->flashing) return true;
    return false;
}

// A frame of fading when idle: the level towards its target (smoothstep), then only the compositor's
// opacity changes.
void IdleFrame(HWND wnd) {
    auto const& s = g_settings;
    double span = (g_idle.to < g_idle.from ? s.idleFadeTime : s.idleWakeTime) * std::abs(g_idle.to - g_idle.from);
    double t = span > 0 ? (NowMs() - g_idle.start) / span : 1;
    t = std::clamp(t, 0.0, 1.0);
    g_idle.level = g_idle.from + (g_idle.to - g_idle.from) * (t * t * (3 - 2 * t));
    if (t >= 1) {
        g_idle.level = g_idle.to;
        g_idle.running = false;
    }
    BYTE alpha = BarAlpha();
    if (alpha == g_barAlpha) return;
    g_barAlpha = alpha;
    BLENDFUNCTION blend{AC_SRC_OVER, 0, g_barAlpha, AC_SRC_ALPHA};
    UpdateLayeredWindow(wnd, nullptr, nullptr, nullptr, nullptr, nullptr, 0, &blend, ULW_ALPHA);
}

// Fading when idle, every tenth of a second: awake while the pointer is on the bar (or a menu, its
// previews, a press, the Start menu, a window wanting attention keeps it busy); once it has been left
// alone long enough, down to the idle opacity, rising again as the pointer comes near.
void IdleTick(HWND wnd) {
    auto const& s = g_settings;
    ULONGLONG now = GetTickCount64();
    POINT pt;
    GetCursorPos(&pt);
    RECT bar = BarRect(true);
    double dx = std::max({0L, bar.left - pt.x, pt.x - (bar.right - 1)});
    double dy = std::max({0L, bar.top - pt.y, pt.y - (bar.bottom - 1)});
    double dist = std::hypot(dx, dy);
    bool busy = dist == 0 || !g_shown || g_inMenu || g_pressed != -2 || (g_preview.wnd && g_preview.bar == wnd) ||
                (IsMainBar() && g_startWnd && IsWindowVisible(g_startWnd)) || (s.idleWakeAttention && BarWantsAttention());
    if (busy || !g_idle.lastActive) g_idle.lastActive = now;
    double target = 1;
    if (now - g_idle.lastActive >= (ULONGLONG)s.idleAfter * 1000 && (s.idleWhen == 0 || (s.idleWhen == 2) == WindowTouchesBar())) {
        target = s.idleOpacity / 100.0;
        int zone = Px(s.idleNear);
        if (zone > 0 && dist < zone) target += (1 - target) * (1 - dist / zone);
    }
    if (std::abs(target - g_idle.to) < 0.005 && (g_idle.running || g_idle.level == g_idle.to)) return;
    g_idle.from = g_idle.level;
    g_idle.to = target;
    g_idle.start = NowMs();
    g_idle.running = true;
    StartPacer(wnd);
    IdleFrame(wnd);
}

////////////////////////////////////////////////////////////////////////////////
// Resting. Hiding, idle fading and the dynamic look check the pointer and the windows ten (or four)
// times a second on every bar. Once nothing has been touched for a couple of seconds and every bar
// has settled (shown or away, faded or not, nothing waiting), those checks stop, so a PC left alone
// wakes for none of them. The first key or pointer move starts them again, through a low-level hook
// that exists only while resting (so it costs nothing while the PC is in use, and nothing while
// nobody touches it), as does any window news from the shell (opened, in front, flashing).

constexpr UINT WM_APP_REST_WAKE = WM_APP + 21;
constexpr DWORD kRestAfterMs = 2000;
HHOOK g_restMouse = nullptr, g_restKeys = nullptr;
bool g_resting = false;

LRESULT CALLBACK RestInputProc(int code, WPARAM wParam, LPARAM lParam) {
    if (code == HC_ACTION && g_resting && g_wnd) PostMessageW(g_wnd, WM_APP_REST_WAKE, 0, 0);
    return CallNextHookEx(nullptr, code, wParam, lParam);
}

// The bars to walk: every screen's, or just the one given when none are registered (a test bar).
template <typename F>
void ForEachBar(HWND fallback, F&& f) {
    if (g_bars.empty()) {
        if (fallback) f(fallback);
        return;
    }
    std::vector<BarState*> bars = g_bars;
    for (BarState* bar : bars) {
        if (!bar->hwnd) continue;
        BarScope scope(bar);
        f(bar->hwnd);
    }
}

// This bar (in the globals) has nothing under way that a check would move on.
bool BarSettled() {
    auto const& s = g_settings;
    if (g_pacer || g_inMenu || g_pressed != -2 || (g_preview.wnd && IsWindowVisible(g_preview.wnd))) return false;
    if (Hides() && (g_wantSince || g_shyRunning || g_slide != (g_shown ? 1 : 0))) return false;
    // Idle fading: only once the fade has been decided (the bar left alone long enough), so it still
    // happens on time without a check running meanwhile.
    if (s.idleFade && (g_idle.running || (g_idle.to >= 1 && GetTickCount64() - g_idle.lastActive < (ULONGLONG)s.idleAfter * 1000 + 200)))
        return false;
    return true;
}

void RestTimers(HWND wnd, bool on) {
    auto const& s = g_settings;
    if (on && Hides()) SetTimer(wnd, kTimerAutoHide, 100, nullptr);
    else KillTimer(wnd, kTimerAutoHide);
    if (on && s.idleFade) SetTimer(wnd, kTimerIdle, 100, nullptr);
    else KillTimer(wnd, kTimerIdle);
    if (on && (s.dynamicTransparency || s.dynamicSegments)) SetTimer(wnd, kTimerDynamic, 250, nullptr);
    else KillTimer(wnd, kTimerDynamic);
}

void RestUnhook() {
    if (g_restMouse) UnhookWindowsHookEx(g_restMouse), g_restMouse = nullptr;
    if (g_restKeys) UnhookWindowsHookEx(g_restKeys), g_restKeys = nullptr;
}

// After a check: rest if nothing was touched lately and every bar has settled.
void MaybeRest(HWND wnd) {
    if (g_resting || !g_wnd) return;
    LASTINPUTINFO li{sizeof(li)};
    if (!GetLastInputInfo(&li) || GetTickCount() - li.dwTime < kRestAfterMs) return;
    bool settled = true;
    ForEachBar(wnd, [&](HWND) { settled = settled && BarSettled(); });
    if (!settled) return;
    g_restMouse = SetWindowsHookExW(WH_MOUSE_LL, RestInputProc, g_instance, 0);
    g_restKeys = SetWindowsHookExW(WH_KEYBOARD_LL, RestInputProc, g_instance, 0);
    if (!g_restMouse || !g_restKeys) return RestUnhook();  // (no way to wake: keep checking)
    g_resting = true;
    ForEachBar(wnd, [](HWND bar) { RestTimers(bar, false); });
}

////////////////////////////////////////////////////////////////////////////////
// Drags over the bar, as on Windows' own taskbar: a file (or anything) held over a task button for half a second
// brings that window up to drop into; held over the rest of the bar (or the show desktop corner) it peeks the
// desktop: that screen's windows step aside (minimised, tiling or not) so the drop can land on the desktop, and
// come back when the button is let go. The bar itself takes no drops.

constexpr UINT_PTR kTimerDragHover = 61, kTimerDragEnd = 62, kTimerEmptyMiddle = 63;

// A click (client point) really on this bar: inside it and the window there. HitTest alone answers "nothing there"
// for any point that isn't on a button, outside the bar too (a click reaching the bar while it holds the mouse),
// which let middle clicks on the desktop switch desktops.
bool OnBarItself(HWND wnd, LPARAM lParam) {
    POINT pt{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
    RECT client;
    if (!GetClientRect(wnd, &client) || !PtInRect(&client, pt)) return false;
    ClientToScreen(wnd, &pt);
    HWND at = WindowFromPoint(pt);
    return at && GetAncestor(at, GA_ROOT) == wnd;
}
bool g_swallowMiddleUp = false;  // the up of a double middle click already handled
constexpr DWORD kDragHoverMs = 500;
std::vector<HWND> g_dragPeeked;  // windows the peek put aside, to bring back
bool g_dragPeeking = false;

struct DragHover {
    HWND bar = nullptr;
    int hit = -2;
    ULONGLONG since = 0;
    bool acted = false;
} g_dragHover;

void PeekDesktop(HWND bar, HMONITOR on = nullptr) {
    if (g_dragPeeking) return;
    if (!on) on = MonitorFromWindow(bar, MONITOR_DEFAULTTONEAREST);
    g_dragPeeked.clear();
    for (HWND h = GetTopWindow(nullptr); h; h = GetWindow(h, GW_HWNDNEXT))
        if (IsTaskWindow(h) && !IsIconic(h) && MonitorFromWindow(h, MONITOR_DEFAULTTONEAREST) == on) g_dragPeeked.push_back(h);
    for (HWND h : g_dragPeeked) ShowWindowAsync(h, SW_SHOWMINNOACTIVE);
    g_dragPeeking = true;
    SetTimer(bar, kTimerDragEnd, 100, nullptr);  // (watching for the drop)
}

// The button let go (the drop made, or the drag given up): the windows back as they were, front last.
void PeekEnd(HWND bar) {
    KillTimer(bar, kTimerDragEnd);
    if (!g_dragPeeking) return;
    g_dragPeeking = false;
    for (auto it = g_dragPeeked.rbegin(); it != g_dragPeeked.rend(); ++it)
        if (IsWindow(*it) && IsIconic(*it)) ShowWindowAsync(*it, SW_SHOWNOACTIVATE);
    g_dragPeeked.clear();
}

void DragHoverTick(HWND wnd) {
    KillTimer(wnd, kTimerDragHover);
    if (g_dragHover.bar != wnd || g_dragHover.acted) return;
    g_dragHover.acted = true;
    BarState* bar = BarOf(wnd);
    if (!bar) return;
    BarScope scope(bar);
    int hit = g_dragHover.hit;
    if (hit >= 0 && hit < (int)g_buttons.size() && !g_buttons[hit].windows.empty()) ActivateWindow(g_buttons[hit].windows.front());
    else if (hit < 0 || hit == kHitShowDesktop || hit == kHitClock) PeekDesktop(wnd);
}

class BarDropTarget : public IDropTarget {
   public:
    explicit BarDropTarget(HWND wnd) : wnd_(wnd) {}
    STDMETHODIMP QueryInterface(REFIID riid, void** out) override {
        if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IDropTarget)) {
            *out = static_cast<IDropTarget*>(this);
            AddRef();
            return S_OK;
        }
        *out = nullptr;
        return E_NOINTERFACE;
    }
    STDMETHODIMP_(ULONG) AddRef() override { return ++refs_; }
    STDMETHODIMP_(ULONG) Release() override {
        ULONG left = --refs_;
        if (!left) delete this;
        return left;
    }
    // The cursor keeps the source's own look (a "no" here swaps in Windows' stock cursor); Drop still takes nothing.
    STDMETHODIMP DragEnter(IDataObject*, DWORD, POINTL pt, DWORD* effect) override {
        Hover(pt);
        allowed_ = *effect;
        *effect = Shown();
        return S_OK;
    }
    STDMETHODIMP DragOver(DWORD, POINTL pt, DWORD* effect) override {
        Hover(pt);
        *effect = Shown();
        return S_OK;
    }
    STDMETHODIMP DragLeave() override {
        KillTimer(wnd_, kTimerDragHover);
        g_dragHover = {};
        return S_OK;
    }
    STDMETHODIMP Drop(IDataObject*, DWORD, POINTL, DWORD* effect) override {
        DragLeave();
        *effect = DROPEFFECT_NONE;
        return S_OK;
    }

   private:
    DWORD Shown() const {
        return allowed_ & DROPEFFECT_MOVE ? DROPEFFECT_MOVE : allowed_ & DROPEFFECT_COPY ? DROPEFFECT_COPY : allowed_ & DROPEFFECT_LINK;
    }
    void Hover(POINTL at) {
        BarState* bar = BarOf(wnd_);
        if (!bar) return;
        int hit;
        {
            BarScope scope(bar);
            POINT p{at.x, at.y};
            ScreenToClient(wnd_, &p);
            hit = HitTest(p);
        }
        if (g_dragHover.bar == wnd_ && g_dragHover.hit == hit) return;  // (still over the same thing: its timer runs)
        g_dragHover = {wnd_, hit, GetTickCount64(), false};
        SetTimer(wnd_, kTimerDragHover, kDragHoverMs, nullptr);
    }
    HWND wnd_;
    DWORD allowed_ = DROPEFFECT_NONE;
    ULONG refs_ = 1;
};

// Drags anywhere else: held still for half a second at a screen's edge, or over the desktop showing between two
// windows, they make room there: the windows beside it slide back from that edge (taskbar.dragRoom pixels, eased)
// so the drop can land on the desktop, and grow back once the button is let go. Nothing closes or minimises, and
// the tiling isn't asked (it only answers moves made by hand). Nothing runs between drags: a capture starting (any
// press in any program) starts a 50 ms check that ends with the button.
HWND g_dragWatchBar = nullptr;
UINT_PTR g_dragWatch = 0;
struct {
    POINT at{};
    ULONGLONG since = 0;
} g_dragRest;

struct RoomSlide {
    HWND wnd;
    RECT home;  // where it was (and goes back to)
    RECT room;  // pulled back
    RECT from, to;  // this slide
};
std::vector<RoomSlide> g_room;
bool g_roomOpen = false;  // made (or being made) for this drag
UINT_PTR g_roomTimer = 0;
ULONGLONG g_roomStart = 0;
constexpr DWORD kRoomMs = 170;
constexpr int kRoomNear = 48;  // how close a window's edge must be to the spot to count as beside it

// A window's edges as drawn (its invisible resize border left out: between two tiles the pointer is usually
// over one of those, not over the desktop).
RECT SeenRect(HWND h) {
    RECT r{};
    if (FAILED(DwmGetWindowAttribute(h, DWMWA_EXTENDED_FRAME_BOUNDS, &r, sizeof(r)))) GetWindowRect(h, &r);
    return r;
}

// The gap the pointer is in or near (within pad of it): two windows facing each other across it, one each side
// of the pointer's line. The seam's middle comes back in at (x for a gap running up and down, y across).
bool InGap(POINT pt, HMONITOR on, int pad, bool* vertical, LONG* at) {
    struct Side {
        LONG edge;
        bool found = false;
    } left{LONG_MIN}, right{LONG_MAX}, top{LONG_MIN}, bottom{LONG_MAX};
    for (HWND h = GetTopWindow(nullptr); h; h = GetWindow(h, GW_HWNDNEXT)) {
        if (!IsTaskWindow(h) || IsIconic(h) || MonitorFromWindow(h, MONITOR_DEFAULTTONEAREST) != on) continue;
        RECT r = SeenRect(h);
        bool rows = pt.y >= r.top && pt.y < r.bottom, cols = pt.x >= r.left && pt.x < r.right;
        // (an edge counts from pad inside the window to anywhere past it, nearest kept)
        if (rows && r.right <= pt.x + pad && r.right > left.edge) left = {r.right, true};
        if (rows && r.left >= pt.x - pad && r.left < right.edge) right = {r.left, true};
        if (cols && r.bottom <= pt.y + pad && r.bottom > top.edge) top = {r.bottom, true};
        if (cols && r.top >= pt.y - pad && r.top < bottom.edge) bottom = {r.top, true};
    }
    // A gap is narrow (the tiling's spacing); a wide stretch of desktop isn't one, it's already room.
    constexpr LONG kWidest = 120;
    bool across = left.found && right.found && right.edge - left.edge <= kWidest;
    bool down = top.found && bottom.found && bottom.edge - top.edge <= kWidest;
    if (!across && !down) return false;
    *vertical = across && (!down || right.edge - left.edge <= bottom.edge - top.edge);
    *at = *vertical ? (left.edge + right.edge) / 2 : (top.edge + bottom.edge) / 2;
    return true;
}

// How long a drag must rest in a gap or at an edge (a pointer passing through doesn't count), and how far off a
// gap the pointer may be and still be in it.
DWORD DragHoldMs() {
    int ms = Wh_GetIntSetting(L"dragHold");
    if (!ms && !td::IsSet(TD_PREFIX, L"dragHold")) ms = 250;
    return (DWORD)std::clamp(ms, 50, 3000);
}

int DragGapPad() {
    int px = Wh_GetIntSetting(L"dragGapPad");
    if (!px && !td::IsSet(TD_PREFIX, L"dragGapPad")) px = 4;
    return std::clamp(px, 0, 40);
}

int DragRoom() {
    int px = Wh_GetIntSetting(L"dragRoom");
    if (!px && !td::IsSet(TD_PREFIX, L"dragRoom")) px = 240;  // (never set: its default)
    return px <= 0 ? 0 : std::clamp(px, 40, 900);
}

bool DragImageShown() {
    for (HWND h = FindWindowW(L"SysDragImage", nullptr); h; h = FindWindowExW(nullptr, h, L"SysDragImage", nullptr))
        if (IsWindowVisible(h)) return true;
    return false;
}

// Held while the room is open or closing, so the tiling waits (tiling.cpp, DropRoomOpen).
void HoldTiling(bool hold) {
    static HANDLE held = CreateEventW(nullptr, TRUE, FALSE, L"Local\\TourneDropRoom");
    if (held) hold ? SetEvent(held) : ResetEvent(held);
}

void CALLBACK RoomTick(HWND, UINT, UINT_PTR, DWORD) {
    double t = std::min(1.0, (double)(GetTickCount64() - g_roomStart) / kRoomMs);
    double e = 1 - (1 - t) * (1 - t) * (1 - t);  // (ease out)
    auto mix = [e](LONG a, LONG b) { return (LONG)std::lround(a + (b - a) * e); };
    for (auto const& s : g_room) {
        if (!IsWindow(s.wnd)) continue;
        RECT r{mix(s.from.left, s.to.left), mix(s.from.top, s.to.top), mix(s.from.right, s.to.right), mix(s.from.bottom, s.to.bottom)};
        SetWindowPos(s.wnd, nullptr, r.left, r.top, r.right - r.left, r.bottom - r.top,
                     SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_ASYNCWINDOWPOS);
    }
    if (t < 1) return;
    KillTimer(nullptr, g_roomTimer);
    g_roomTimer = 0;
    if (!g_roomOpen) {
        g_room.clear();
        HoldTiling(false);
    }
}

void SlideRoom(bool open) {
    for (auto& s : g_room) {
        if (!IsWindow(s.wnd)) continue;
        GetWindowRect(s.wnd, &s.from);  // (from wherever it is: a slide cut short turns round smoothly)
        s.to = open ? s.room : s.home;
    }
    g_roomStart = GetTickCount64();
    if (!g_roomTimer) g_roomTimer = SetTimer(nullptr, 0, 15, RoomTick);
}

// The windows beside the spot pulled back from it. At a screen's edge: those touching that edge. Over the desktop:
// the gap's two sides, along whichever way it runs (the nearer edges decide).
void MakeRoom(POINT pt, HMONITOR on, bool edge, bool vertical, LONG at) {
    int room = DragRoom();
    if (!room) return;
    MONITORINFO mi{sizeof(mi)};
    GetMonitorInfoW(on, &mi);
    RECT const& m = mi.rcMonitor;
    RECT const& work = mi.rcWork;
    std::vector<std::pair<HWND, RECT>> wins;
    for (HWND h = GetTopWindow(nullptr); h; h = GetWindow(h, GW_HWNDNEXT)) {
        RECT r;
        if (IsTaskWindow(h) && !IsIconic(h) && MonitorFromWindow(h, MONITOR_DEFAULTTONEAREST) == on && GetWindowRect(h, &r))
            wins.push_back({h, r});
    }
    constexpr int kMin = 160;  // (no window made narrower than this)
    auto add = [](HWND h, RECT const& r, RECT n) {
        if (n.right - n.left < kMin || n.bottom - n.top < kMin || EqualRect(&r, &n)) return;
        g_room.push_back({h, r, n, r, r});
    };
    g_room.clear();
    if (edge) {
        int zone = DragGapPad() + 2;
        bool left = pt.x < m.left + zone, right = pt.x >= m.right - zone;
        for (auto const& [h, r] : wins) {
            RECT n = r;
            if (left && r.left - work.left < kRoomNear) n.left = std::max(r.left, work.left) + room;
            else if (right && work.right - r.right < kRoomNear) n.right = std::min(r.right, work.right) - room;
            else if (!left && !right && r.top - work.top < kRoomNear) n.top = std::max(r.top, work.top) + room;
            add(h, r, n);
        }
    } else {
        // The gap's two sides, as drawn, pulled back from its middle (the window moved by as much as its drawn
        // edge has to go, its invisible border going along).
        int half = room / 2;
        for (auto const& [h, r] : wins) {
            RECT v = SeenRect(h), n = r;
            if (vertical) {
                if (v.right <= at + kRoomNear / 2 && at - v.right < kRoomNear) n.right -= std::max<LONG>(0, v.right - (at - half));
                else if (v.left >= at - kRoomNear / 2 && v.left - at < kRoomNear) n.left += std::max<LONG>(0, at + half - v.left);
            } else {
                if (v.bottom <= at + kRoomNear / 2 && at - v.bottom < kRoomNear) n.bottom -= std::max<LONG>(0, v.bottom - (at - half));
                else if (v.top >= at - kRoomNear / 2 && v.top - at < kRoomNear) n.top += std::max<LONG>(0, at + half - v.top);
            }
            add(h, r, n);
        }
    }
    if (g_room.empty()) return;
    g_roomOpen = true;
    HoldTiling(true);
    Wh_Log(L"Drag held %s: room made beside %d window(s)", edge ? L"at a screen edge" : L"over the desktop", (int)g_room.size());
    SlideRoom(true);
}

void CALLBACK DragWatchTick(HWND, UINT, UINT_PTR, DWORD) {
    if (!(GetAsyncKeyState(GetSystemMetrics(SM_SWAPBUTTON) ? VK_RBUTTON : VK_LBUTTON) & 0x8000)) {
        KillTimer(nullptr, g_dragWatch);
        g_dragWatch = 0;
        g_dragRest = {};
        if (g_roomOpen) {
            g_roomOpen = false;
            SlideRoom(false);  // (the drop made, or given up: the windows fill the space again)
        }
        return;
    }
    if (g_roomOpen || g_dragPeeking || !IsWindow(g_dragWatchBar) || !DragImageShown()) return;
    POINT pt;
    GetCursorPos(&pt);
    HMONITOR on = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi{sizeof(mi)};
    GetMonitorInfoW(on, &mi);
    RECT const& m = mi.rcMonitor;
    // (a few pixels in from the edge count: where a screen meets another the pointer doesn't stop at the edge)
    int zone = DragGapPad() + 2;
    bool edge = pt.x < m.left + zone || pt.x >= m.right - zone || pt.y < m.top + zone;
    bool vertical = false;
    LONG at = 0;
    bool gap = !edge && InGap(pt, on, DragGapPad(), &vertical, &at);
    if (!edge && !gap) return void(g_dragRest = {});
    // Meant, not passing through: the pointer has to stay put (within a few pixels) for the whole wait; a drag
    // sweeping across a gap keeps restarting it.
    ULONGLONG now = GetTickCount64();
    if (!g_dragRest.since || abs(pt.x - g_dragRest.at.x) > 6 || abs(pt.y - g_dragRest.at.y) > 6)
        return void(g_dragRest = {pt, now});
    if (now - g_dragRest.since < DragHoldMs()) return;
    g_dragRest = {};
    MakeRoom(pt, on, edge, vertical, at);
}

void CALLBACK CaptureEventProc(HWINEVENTHOOK, DWORD, HWND, LONG, LONG, DWORD, DWORD) {
    if (!g_dragWatch) g_dragWatch = SetTimer(nullptr, 0, 50, DragWatchTick);
}

void DragTargetStart(HWND wnd) {
    static bool ole = SUCCEEDED(OleInitialize(nullptr));  // (the bars' thread, once)
    if (!ole) return;
    if (!g_dragWatchBar) g_dragWatchBar = wnd;
    static HWINEVENTHOOK hook = SetWinEventHook(EVENT_SYSTEM_CAPTURESTART, EVENT_SYSTEM_CAPTURESTART, nullptr, CaptureEventProc, 0, 0,
                                                WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);
    (void)hook;
    auto* target = new BarDropTarget(wnd);
    RegisterDragDrop(wnd, target);
    target->Release();
}

void DragTargetStop(HWND wnd) {
    RevokeDragDrop(wnd);
    if (g_dragWatchBar == wnd) g_dragWatchBar = nullptr;
    KillTimer(wnd, kTimerDragHover);
    PeekEnd(wnd);
}

// Something happened: the checks run again at once, and from then on as before.
void RestWake(HWND wnd) {
    if (!g_resting) return;
    g_resting = false;
    RestUnhook();
    ForEachBar(wnd, [](HWND bar) {
        RestTimers(bar, true);
        if (Hides()) AutoHideTick(bar);
        if (g_settings.idleFade) IdleTick(bar);
        if (g_settings.dynamicTransparency || g_settings.dynamicSegments) DynamicTick(bar);
    });
}

////////////////////////////////////////////////////////////////////////////////
// Tooltips that follow the theme: the visual style's own tooltip look when it
// has one, else the colour scheme (by default System Icons', so they match
// its flyouts and the tray's tooltips), in the Tourne pixel font, square and
// without a fade. Drawn from the current settings every time, so a changed
// theme or scheme shows at once.

HFONT TourneFont(int px) {
    for (PCWSTR face : {L"CozetteVector", L"scientifica"}) {
        LOGFONTW lf{};
        lf.lfHeight = -px;
        lf.lfCharSet = DEFAULT_CHARSET;
        lf.lfQuality = NONANTIALIASED_QUALITY;
        wcsncpy_s(lf.lfFaceName, face, _TRUNCATE);
        HFONT font = CreateFontIndirectW(&lf);
        // Windows substitutes a missing face; keep only a real match.
        HDC dc = GetDC(nullptr);
        HGDIOBJ old = SelectObject(dc, font);
        WCHAR got[LF_FACESIZE] = L"";
        GetTextFaceW(dc, LF_FACESIZE, got);
        SelectObject(dc, old);
        ReleaseDC(nullptr, dc);
        if (!_wcsicmp(got, face)) return font;
        DeleteObject(font);
    }
    return nullptr;
}

void PaintTooltip(HWND tip, HDC dc) {
    auto const& p = g_settings.palette;
    RECT client;
    GetClientRect(tip, &client);
    COLORREF text = PaletteRef(p.text);
    if (g_themed && g_theme.tip) {
        Fill(dc, client, p.ground);  // under a see-through edge
        DrawThemeBackground(g_theme.tip, dc, TTP_STANDARD, TTSS_NORMAL, &client, nullptr);
        GetThemeColor(g_theme.tip, TTP_STANDARD, TTSS_NORMAL, TMT_TEXTCOLOR, &text);
    } else {
        Fill(dc, client, p.ground);
        SetDCBrushColor(dc, PaletteRef(p.edge));
        FrameRect(dc, &client, (HBRUSH)GetStockObject(DC_BRUSH));
    }
    WCHAR buffer[1024];
    GetWindowTextW(tip, buffer, ARRAYSIZE(buffer));
    RECT margin{};
    SendMessageW(tip, TTM_GETMARGIN, 0, (LPARAM)&margin);
    RECT area{client.left + margin.left, client.top + margin.top, client.right - margin.right,
              client.bottom - margin.bottom};
    HGDIOBJ old = SelectObject(dc, (HFONT)SendMessageW(tip, WM_GETFONT, 0, 0));
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, text);
    DrawTextW(dc, buffer, -1, &area, DT_LEFT | DT_TOP | DT_NOPREFIX | DT_WORDBREAK | DT_EXPANDTABS);
    SelectObject(dc, old);
}

bool g_placingTip = false;  // PlaceTooltip is moving it (TooltipProc leaves that move alone)
bool TipPosition(HWND wnd, UINT_PTR id, int tw, int th, int* outX, int* outY, bool besideOnly);

LRESULT CALLBACK TooltipProc(HWND tip, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR, DWORD_PTR) {
    switch (msg) {
        case WM_WINDOWPOSCHANGING: {
            // The tooltip control moving a tooltip of ours that shows beside the bar (a refresh puts it
            // back where the control would): it goes straight to its own place instead, so it never
            // shows for a frame over the bar.
            auto* pos = (WINDOWPOS*)lParam;
            if (g_placingTip || (pos->flags & SWP_NOMOVE)) break;
            HWND bar = GetWindow(tip, GW_OWNER);
            BarState* state = BarOf(bar);
            if (!state) break;
            BarScope scope(state);
            TOOLINFOW ti{sizeof(ti)};
            if (tip != g_tooltip || !SendMessageW(tip, TTM_GETCURRENTTOOLW, 0, (LPARAM)&ti)) break;
            RECT now;
            GetWindowRect(tip, &now);
            int tw = (pos->flags & SWP_NOSIZE) ? now.right - now.left : pos->cx;
            int th = (pos->flags & SWP_NOSIZE) ? now.bottom - now.top : pos->cy;
            int x = pos->x, y = pos->y;
            if (TipPosition(bar, ti.uId, tw, th, &x, &y, true)) {
                pos->x = x;
                pos->y = y;
            }
            break;
        }
        case WM_ERASEBKGND:
            // Painted here too, so it never shows unpainted for a frame.
            PaintTooltip(tip, (HDC)wParam);
            return 1;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC dc = BeginPaint(tip, &ps);
            PaintTooltip(tip, dc);
            EndPaint(tip, &ps);
            return 0;
        }
        case WM_PRINTCLIENT:
            PaintTooltip(tip, (HDC)wParam);
            return 0;
        case WM_NCDESTROY:
            RemoveWindowSubclass(tip, TooltipProc, 0);
            break;
    }
    return DefSubclassProc(tip, msg, wParam, lParam);
}

// With the settings: the font and margins for the scale, and a repaint so an
// open tooltip takes a new theme or scheme at once.
void MakeTipFont() {
    if (g_tipFont) DeleteObject(g_tipFont);
    g_tipFont = TourneFont(Px(13));
}

void StyleTooltip() {
    if (!g_tooltip) return;
    SendMessageW(g_tooltip, WM_SETFONT, (WPARAM)(g_tipFont ? g_tipFont : g_font), FALSE);
    RECT margin{Px(6), Px(4), Px(6), Px(4)};
    SendMessageW(g_tooltip, TTM_SETMARGIN, 0, (LPARAM)&margin);
    // The delay as set, else Windows' own (from the double-click time).
    if (g_settings.tipDelay > 0) SendMessageW(g_tooltip, TTM_SETDELAYTIME, TTDT_INITIAL, g_settings.tipDelay);
    else SendMessageW(g_tooltip, TTM_SETDELAYTIME, TTDT_AUTOMATIC, -1);
    SendMessageW(g_tooltip, TTM_UPDATE, 0, 0);
    InvalidateRect(g_tooltip, nullptr, TRUE);
}

// The text a tool's tooltip shows.
std::wstring TooltipText(UINT_PTR id) {
    std::wstring text;
    size_t i = id - 1;
    if (id >= (UINT_PTR)kHitCss) {
        if (id - kHitCss < g_cssTargets.size()) text = g_cssTargets[id - kHitCss].tip;
    } else if (id >= (UINT_PTR)kHitTray) {
        if (id - kHitTray < g_tray.size()) text = g_tray[id - kHitTray].icon.tip;
    } else if (int item = PluginIndexOf((int)id); item >= 0) {
        text = g_pluginItems[item].tip;
        if (text.empty() && Vertical()) text = g_pluginItems[item].text;  // (a vertical bar shows only its glyph)
    } else if (id == (UINT_PTR)kHitShowDesktop) {
        text = L"Show desktop";
    } else if (id == (UINT_PTR)kHitClock) {
        SYSTEMTIME now;
        GetLocalTime(&now);
        text = FormatDateText(now, nullptr, DATE_LONGDATE) + L"\n" + FormatTimeText(now, nullptr);
    } else if (int desktop = PagerIndexOf((int)id); desktop >= 0) {
        if (!g_settings.pagerLook.previews) text = PagerLabel(desktop);  // (its preview says its name)
    } else if (i < g_buttons.size() && g_settings.previews && !g_buttons[i].windows.empty()) {
        // Running apps show their previews instead.
    } else if (i < g_buttons.size()) {
        if (g_buttons[i].windows.empty() && g_buttons[i].pin >= 0 && g_buttons[i].pin < (int)g_pins.size())
            text = g_pins[g_buttons[i].pin].name;
        for (HWND h : g_buttons[i].windows) {
            if (!text.empty()) text += L"\n";
            text += Title(h);
        }
    }
    return text;
}

// Where a tooltip shows, when it isn't left where Windows puts it: beside the bar over what's pointed
// at, or Windows' place (*outX, *outY on the way in); then moved away from the bar and along it as set,
// and kept on the bar's screen. False when it's left where Windows puts it. With besideOnly, only a
// place beside the bar over what's pointed at counts.
bool TipPosition(HWND wnd, UINT_PTR id, int tw, int th, int* outX, int* outY, bool besideOnly = false) {
    auto const& s = g_settings;
    if (s.tipPlace == 0 && (besideOnly || (!s.tipAway && !s.tipAlong))) return false;
    RECT bar;
    GetWindowRect(wnd, &bar);
    int gap = Px(4);
    int x = *outX, y = *outY;
    RECT item{};
    bool found = false;
    for (auto const& [placedId, r] : g_placed) {
        if (placedId == id) {
            item = r;
            found = true;
            break;
        }
    }
    if (besideOnly && !found) return false;
    if (s.tipPlace == 1 && found) {
        MapWindowPoints(wnd, nullptr, (POINT*)&item, 2);
        int midX = (item.left + item.right) / 2 - tw / 2, midY = (item.top + item.bottom) / 2 - th / 2;
        switch (s.edge) {
            case Edge::Bottom: x = midX; y = bar.top - gap - th; break;
            case Edge::Top: x = midX; y = bar.bottom + gap; break;
            case Edge::Left: x = bar.right + gap; y = midY; break;
            case Edge::Right: x = bar.left - gap - tw; y = midY; break;
        }
    }
    int away = Px(s.tipAway), along = Px(s.tipAlong);
    switch (s.edge) {
        case Edge::Bottom: y -= away; x += along; break;
        case Edge::Top: y += away; x += along; break;
        case Edge::Left: x += away; y += along; break;
        case Edge::Right: x -= away; y += along; break;
    }
    MONITORINFO mi{sizeof(mi)};
    GetMonitorInfoW(MonitorFromWindow(wnd, MONITOR_DEFAULTTONEAREST), &mi);
    x = std::clamp(x, (int)mi.rcMonitor.left, std::max((int)mi.rcMonitor.left, (int)mi.rcMonitor.right - tw));
    y = std::clamp(y, (int)mi.rcMonitor.top, std::max((int)mi.rcMonitor.top, (int)mi.rcMonitor.bottom - th));
    *outX = x;
    *outY = y;
    return true;
}

BOOL PlaceTooltip(HWND wnd, UINT_PTR id) {
    RECT tip;
    GetWindowRect(g_tooltip, &tip);
    int x = tip.left, y = tip.top;
    if (!TipPosition(wnd, id, tip.right - tip.left, tip.bottom - tip.top, &x, &y)) return FALSE;
    g_placingTip = true;
    SetWindowPos(g_tooltip, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
    g_placingTip = false;
    return TRUE;
}

LRESULT CALLBACK WndProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    // Which bar this is for: its state is in the globals for the message, and the bar before is back after.
    BarState* thisBar;
    if (msg == WM_CREATE) {
        thisBar = (BarState*)((CREATESTRUCTW*)lParam)->lpCreateParams;
        if (thisBar) {
            thisBar->hwnd = wnd;
            if (thisBar == &g_mainBar) g_wnd = wnd;
            if (thisBar == g_active) g_curWnd = wnd;
        }
    } else {
        thisBar = BarOf(wnd);
    }
    BarScope barScope(thisBar);
    if (g_shellHookMessage && msg == g_shellHookMessage) {
        RestWake(wnd);  // a window opened, came to the front or flashes: the bars may need to move
        HWND h = (HWND)lParam;
        if (wParam == (HSHELL_REDRAW | 0x8000)) {  // HSHELL_FLASH
            if (Win* w = FindWin(h)) w->flashing = true;
        } else if ((wParam & 0x7FFF) == HSHELL_WINDOWACTIVATED) {
            if (Win* w = FindWin(h)) w->flashing = false;
            PreviewActivated(h);
        } else if (wParam == HSHELL_REDRAW) {
            if (Win* w = FindWin(h)) FetchIcon(*w);
        }
        Refresh(wnd);
        return 0;
    }
    if (msg == WM_APP_REST_WAKE) {
        RestWake(wnd);
        return 0;
    }
    if (g_msgTrayChanged && msg == g_msgTrayChanged) {
        // One read covers a burst of changes, say a fast spin of the wheel.
        MSG queued;
        while (PeekMessageW(&queued, wnd, g_msgTrayChanged, g_msgTrayChanged, PM_REMOVE)) {
        }
        SetTimer(wnd, kTimerTrayAgain, 100, nullptr);
        // Moved in or out of the overflow from its flyout: the same icons, placed elsewhere, so the tray is
        // built again from where they're placed too.
        if (FetchTray(wnd) || RebuildTray()) {
            Layout(wnd);
            Redraw(wnd);
        }
        return 0;
    }
    if (g_msgPluginsChanged && msg == g_msgPluginsChanged) {
        if (IsMainBar()) AskForPlugins(wnd);
        return 0;
    }
    if (g_msgProgress && msg == g_msgProgress) {
        // An app's progress changed (wParam: its window; lParam: value, state). Every bar hears it; the
        // table is shared, and a bar redraws when it shows that window and the change can be seen.
        HWND h = (HWND)wParam;
        Progress now{HIWORD(lParam), LOWORD(lParam)};
        Progress before;
        if (auto it = g_progress.find(h); it != g_progress.end()) before = it->second;
        if (now.state) g_progress[h] = now;
        else g_progress.erase(h);
        bool seen = now.state != before.state || abs(now.value - before.value) >= 0x100 || now.value == 0xFFFF;
        if (seen && !CssActive())
            for (auto const& b : g_buttons)
                if (std::find(b.windows.begin(), b.windows.end(), h) != b.windows.end()) {
                    Redraw(wnd);
                    break;
                }
        return 0;
    }
    if (g_msgShellStart && msg == g_msgShellStart) {
        if (IsMainBar()) StartKeyHook(g_startConfig.enabled && g_startConfig.winKey);
        return 0;
    }
    if (g_msgCrashGuard && msg == g_msgCrashGuard) {
        if (IsMainBar()) {
            ShowToast(wnd, L"Explorer kept closing right after starting, so Tourne'Suite stopped loading its "
                           L"parts into it. Save any Tourne'Suite setting to try again.");
            SetTimer(wnd, kTimerToast, 12000, nullptr);
        }
        return 0;
    }
    if (g_msgTrayDragOver && msg == g_msgTrayDragOver) {
        // An icon dragged from System Icons' flyout is over the bar (wParam 1,
        // the screen point in lParam) or has left it: where it would land.
        int at = -1;
        if (wParam) {
            POINT pt{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            ScreenToClient(wnd, &pt);
            at = TrayInsertIndex(pt);
        }
        if (at != g_trayDropAt) {
            g_trayDropAt = at;
            Redraw(wnd);
        }
        return 0;
    }

    // Tourne Desktop made new theme colours: read again if the bar uses them.
    if (msg == TourneThemeMessage() && msg) {
        if (IsMainBar() && g_followsTourneTheme) ApplySettings(wnd, false, L"new theme colours");
        return 0;
    }

    // A program Tourne leaves alone (a game) came to the front, or went: the Start key's hook let go or back.
    if (msg == LeftAloneMessage() && msg) {
        if (IsMainBar()) StartKeyHook(g_startConfig.enabled && g_startConfig.winKey);
        return 0;
    }

    // Tourne Desktop's desktop was clicked with the Tourne Menu's button: it opens at the pointer.
    if (msg == TourneMenuMessage() && msg) {
        ShowTourneMenu(wnd);
        return 0;
    }

    // Tourne Desktop's desktop was right-clicked: its menu, in this look (desktop-menu.cpp). The request is
    // a string the desktop made for this message, to free here.
    if (msg == DesktopMenuMessage() && msg) {
        auto* request = (std::wstring*)lParam;
        if (request) {
            std::wstring text = *request;
            delete request;
            ShowDesktopMenu(wnd, (HWND)wParam, text);
        }
        return 0;
    }

    switch (msg) {
        case WM_CREATE:
            DragTargetStart(wnd);
            // The main bar watches the windows and the tray for all of them; the others only show what it finds.
            if (IsMainBar()) {
                g_shellHookMessage = RegisterWindowMessageW(L"SHELLHOOK");
                RegisterShellHookWindow(wnd);
                SetTimer(wnd, kTimerRescan, 1000, nullptr);
                // Tray icons change (animations, tooltips) without telling anyone.
                SetTimer(wnd, kTimerTray, 1000, nullptr);
                g_msgRelay = RegisterWindowMessageW(L"TourneSuite_RelayOverflow");
                g_msgTrayChanged = RegisterWindowMessageW(L"TourneSuite_TrayChanged");
                ChangeWindowMessageFilterEx(wnd, g_msgTrayChanged, MSGFLT_ALLOW, nullptr);
                g_msgTrayDragOver = RegisterWindowMessageW(kTrayDragOverMessage);
                g_msgPluginsChanged = RegisterWindowMessageW(kPluginBarChangedMessage);
                g_msgPluginsAsk = RegisterWindowMessageW(kPluginBarAskMessage);
                g_msgPluginsEvent = RegisterWindowMessageW(kPluginBarEventMessage);
                AskForPlugins(wnd);
            }
            // Progress and the crash guard's notice come from Explorer, to every bar.
            g_msgProgress = RegisterWindowMessageW(L"TourneSuite_Progress");
            g_msgCrashGuard = RegisterWindowMessageW(L"TourneSuite_CrashGuard");
            ChangeWindowMessageFilterEx(wnd, g_msgProgress, MSGFLT_ALLOW, nullptr);
            ChangeWindowMessageFilterEx(wnd, g_msgCrashGuard, MSGFLT_ALLOW, nullptr);
            g_msgShellStart = RegisterWindowMessageW(L"TourneSuite_ShellStart");
            ChangeWindowMessageFilterEx(wnd, g_msgShellStart, MSGFLT_ALLOW, nullptr);
            // What progress there is already: the relay sends it all.
            if (IsMainBar())
                if (HWND tray = FindWindowW(L"Shell_TrayWnd", nullptr))
                    PostMessageW(tray, RegisterWindowMessageW(L"TourneSuite_ProgressQuery"), (WPARAM)wnd, 0);
            g_tooltip = CreateWindowExW(WS_EX_TOPMOST, TOOLTIPS_CLASSW, nullptr,
                                        WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP | TTS_NOANIMATE | TTS_NOFADE, CW_USEDEFAULT,
                                        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, wnd, nullptr, nullptr, nullptr);
            if (g_tooltip) {
                SetWindowSubclass(g_tooltip, TooltipProc, 0, 0);
                KeepOutOfPeek(g_tooltip);
                DWORD square = 1;  // DWMWCP_DONOTROUND
                DwmSetWindowAttribute(g_tooltip, 33 /* DWMWA_WINDOW_CORNER_PREFERENCE */, &square, sizeof(square));
            }
            return 0;

        case WM_NOTIFY: {
            auto* nm = (NMHDR*)lParam;
            if (nm->hwndFrom == g_tooltip && nm->code == TTN_SHOW) return PlaceTooltip(wnd, nm->idFrom);
            if (nm->hwndFrom == g_tooltip && nm->code == TTN_GETDISPINFOW) {
                auto* di = (NMTTDISPINFOW*)lParam;
                static std::wstring text;
                text = TooltipText(di->hdr.idFrom);
                g_tipShownText = text;
                SendMessageW(g_tooltip, TTM_SETMAXTIPWIDTH, 0, Px(400));
                di->lpszText = text.data();
            }
            return 0;
        }

        case WM_APP_BAR:
            if (wParam == ABN_POSCHANGED) {
                PlaceBar(wnd);
            } else if (wParam == ABN_FULLSCREENAPP) {
                SetUnderFullscreen(wnd, lParam && g_settings.fullscreenStayAway);
            }
            return 0;

        case WM_APP_START:
            ToggleStartMenu();
            return 0;

        case WM_APP_ICONS:
            ForgetTourneIconsButtons();
            if (g_msgTrayChanged) PostMessageW(wnd, g_msgTrayChanged, 0, 0);  // (the tray's icons, read again)
            RefreshShortcuts(true);
            for (auto& w : g_wins) FetchIcon(w);
            RelayoutBars(wnd);
            return 0;

        case WM_APP_SETTINGS:
        case WM_DPICHANGED:
            if (IsMainBar()) ApplySettings(wnd, false, msg == WM_DPICHANGED ? L"scale changed" : L"settings changed");  // (the others get these too; one reload does for all)
            else if (msg == WM_DPICHANGED && g_wnd) SetTimer(g_wnd, kTimerScreens, 400, nullptr);  // its screen's scale
            return 0;

        case WM_THEMECHANGED:  // another visual style: its colours, for the schemes taken from it
            if (IsMainBar() && LookSignature() != g_lookSignature) ApplySettings(wnd, false, L"visual style changed");
            return 0;

        case WM_DISPLAYCHANGE:
            // Screens came or went, moved, or woke: the bars follow them once they settle (waking
            // sends several of these in a row).
            if (IsMainBar()) SetTimer(wnd, kTimerScreens, 1000, nullptr);
            return 0;

        case WM_SETTINGCHANGE:
            // Dark or light mode, or the accent colour, when one of them really changed.
            if (IsMainBar() && lParam && !wcscmp((PCWSTR)lParam, L"ImmersiveColorSet") &&
                LookSignature() != g_lookSignature)
                ApplySettings(wnd, false, L"colour mode changed");
            break;

        case WM_MEASUREITEM:
            if (MenuMeasureItem((MEASUREITEMSTRUCT*)lParam)) return TRUE;
            if (LRESULT result; g_shellMenu.menu && ShellMenuMessage(msg, wParam, lParam, &result)) return TRUE;
            break;

        case WM_DRAWITEM:
            if (MenuDrawItem((DRAWITEMSTRUCT*)lParam)) return TRUE;
            if (LRESULT result; g_shellMenu.menu && ShellMenuMessage(msg, wParam, lParam, &result)) return TRUE;
            break;

        case WM_MENURBUTTONUP:
            // The app in its jump list: its own shell menu.
            if (!g_jumpApp.empty() && !g_shellMenu.menu && GetMenuItemID((HMENU)lParam, (int)wParam) == kCmdNew)
                OpenShellMenu(wnd, g_jumpApp);
            return 0;

        case WM_INITMENUPOPUP:
            // A shell submenu ("Send to") filling in, then styled; once: again, it's as it was.
            if (g_shellMenu.open) {
                if (g_shellMenu.filled.count((HMENU)wParam)) {
                    StyleMenuItems((HMENU)wParam, true);  // (cheap for items already styled; new ones get styled)
                    return 0;
                }
                g_shellMenu.filled.insert((HMENU)wParam);
                LRESULT result;
                ShellMenuMessage(msg, wParam, lParam, &result);
                StyleMenuItems((HMENU)wParam, true);
                return 0;
            }
            break;

        case WM_MENUCHAR:
            if (LRESULT result; g_shellMenu.open && ShellMenuMessage(msg, wParam, lParam, &result)) return result;
            break;

        case WM_CLIPBOARDUPDATE:
            OnClipboardChanged(wnd);
            break;

        case WM_COPYDATA: {
            auto* copyData = (COPYDATASTRUCT const*)lParam;
            if (copyData && copyData->dwData == kHiddenIconsCopyId && copyData->lpData) {
                OnTrayPacket(copyData);
                return TRUE;
            }
            // The plugin items, from Tourne Desktop's plugin runtime.
            if (copyData && copyData->dwData == kPluginBarItemsId && copyData->lpData && IsMainBar()) {
                wchar_t from[64];
                if (GetClassNameW((HWND)wParam, from, 64) && !wcscmp(from, kPluginHostClass)) {
                    auto const* text = (wchar_t const*)copyData->lpData;
                    std::wstring_view packed(text, wcsnlen(text, copyData->cbData / sizeof(wchar_t)));
                    unsigned version = 0;
                    auto items = UnpackPluginBarItems(packed, &version);
                    g_pluginItems = std::move(items);
                    g_pluginPictures.clear();
                    for (auto const& item : g_pluginItems) g_pluginPictures.push_back(ReadPluginPicture(item.image));
                    g_pluginVersion = version;
                    ReadPluginAway(TrayPlaces::Load());
                    Layout(wnd);
                    Redraw(wnd);
                }
                return TRUE;
            }
            // An icon dropped here from System Icons' overflow flyout.
            POINT screen;
            std::wstring key;
            if (ReadTrayDrop(copyData, &screen, &key)) return DropOnTray(wnd, screen, key);
            return FALSE;
        }

        case WM_DROPFILES: {
            // A theme, an orb, a theme pack folder or .zip dropped on the bar.
            HDROP drop = (HDROP)wParam;
            std::vector<std::wstring> paths;
            UINT count = DragQueryFileW(drop, 0xFFFFFFFF, nullptr, 0);
            for (UINT i = 0; i < count; i++) {
                WCHAR path[MAX_PATH * 2];
                if (DragQueryFileW(drop, i, path, ARRAYSIZE(path))) paths.push_back(path);
            }
            DragFinish(drop);
            InstallPaths(wnd, paths);
            return 0;
        }

        case WM_HOTKEY:
            if (wParam == kHotkeyHide) {
                g_hideFlip = !g_hideFlip;
                ApplyHidingToBars(wnd);
            } else if (wParam == kHotkeyMenu) {
                ShowTourneMenu(wnd);
            } else if (wParam == kHotkeyNextDesktop || wParam == kHotkeyPrevDesktop) {
                int step = wParam == kHotkeyNextDesktop ? 1 : -1;
                PagerGoTo(std::clamp(g_pagerDesktops.current + step, 0, g_pagerDesktops.count - 1));
            } else if (wParam >= kHotkeyDesktop1 && wParam < kHotkeyDesktop1 + 9) {
                PagerGoTo((int)wParam - kHotkeyDesktop1);
            } else if (wParam == kHotkeyMoveNext || wParam == kHotkeyMovePrev) {
                int step = wParam == kHotkeyMoveNext ? 1 : -1;
                PagerSendWindow(std::clamp(g_pagerDesktops.current + step, 0, g_pagerDesktops.count - 1));
            } else if (wParam >= kHotkeyMove1 && wParam < kHotkeyMove1 + 9) {
                PagerSendWindow((int)wParam - kHotkeyMove1);
            }
            return 0;

        case WM_TIMER:
            if (wParam == kTimerDragHover) return DragHoverTick(wnd), 0;
            if (wParam == kTimerEmptyMiddle) {
                KillTimer(wnd, kTimerEmptyMiddle);  // (no second middle click came: the single one's)
                TmAction(L"tourne:" + g_settings.emptyMiddleClick);
                return 0;
            }
            if (wParam == kTimerDragEnd) {
                if (!(GetAsyncKeyState(GetSystemMetrics(SM_SWAPBUTTON) ? VK_RBUTTON : VK_LBUTTON) & 0x8000)) PeekEnd(wnd);
                return 0;
            }
            if (wParam == kTimerScreens) {
                KillTimer(wnd, kTimerScreens);
                // Only the bars, unless the main screen's scale changed (then sizes and icons are redone).
                UINT dpi = GetDpiForWindow(wnd);
                ApplySettings(wnd, std::max(1, (int)(dpi + 48) / 96) == g_scale, L"screens changed");
            } else if (wParam == kTimerReadyMenu) {
                OnReadyMenuTimer(wnd);
            } else if (wParam == kTimerBootLog) {
                KillTimer(wnd, kTimerBootLog);
                WriteBootLog();
            } else if (wParam == kTimerReload) {
                KillTimer(wnd, kTimerReload);
                ApplySettings(wnd, false, L"a theme file changed");  // a theme or orb file changed, or StartAllBack''s settings: read again
            } else if (wParam == kTimerToast) {
                KillTimer(wnd, kTimerToast);
                if (g_toast) {
                    TTTOOLINFOW ti{sizeof(ti)};
                    ti.hwnd = wnd;
                    ti.uId = 0x7001;
                    SendMessageW(g_toast, TTM_TRACKACTIVATE, FALSE, (LPARAM)&ti);
                }
            } else if (wParam == kTimerCssFade) {
                Redraw(wnd);  // a hover in the theme fading in or out
            } else if (wParam == kTimerGlow) {
                GlowStep(wnd);
            } else if (wParam == kTimerProgress) {
                Redraw(wnd);  // the busy sweep moving on
            } else if (wParam == kTimerRescan) {
                // Windows that closed while showing progress.
                if (!g_progress.empty()) std::erase_if(g_progress, [](auto const& e) { return !IsWindow(e.first); });
                // New windows, a title, the active window, the clock's next minute: the bars are drawn again
                // only when something they show changed (or was drawn since by anything else).
                RescanWindows();
                if (uint64_t shown = BarsSignature(wnd); !shown || shown != g_barsDrawn) {
                    RelayoutBars(wnd);
                    g_barsDrawn = shown;
                }
            } else if (wParam == kTimerAutoHide) {
                RestWake(wnd);  // (a bar placed again meanwhile started its checks: all of them run again)
                AutoHideTick(wnd);
                MaybeRest(wnd);
            } else if (wParam == kTimerShy) {
                // The frame clock couldn't start: the same frames, from a timer.
                if (g_shyRunning) ShyFrame(wnd);
                if (g_aura.running) AuraFrame(wnd);
                if (g_idle.running) IdleFrame(wnd);
                if (g_active->arrive.running) ArriveFrame(wnd);
                if (!g_shyRunning && !g_aura.running && !g_idle.running && !g_active->arrive.running) StopPacer(wnd);
            } else if (wParam == kTimerIdle) {
                RestWake(wnd);
                IdleTick(wnd);
                MaybeRest(wnd);
            } else if (wParam == kTimerDynamic) {
                RestWake(wnd);
                DynamicTick(wnd);
                MaybeRest(wnd);
            } else if (wParam == kTimerPreviewShow) {
                KillTimer(wnd, kTimerPreviewShow);
                if (g_hot >= 0 && !g_inMenu) ShowPreview(wnd, g_hot);
            } else if (wParam == kTimerDesktopPreview) {
                PagerPreviewTimer(wnd);
            } else if (wParam == kTimerPreviewHide) {
                KillTimer(wnd, kTimerPreviewHide);
                // Unless the pointer came back to the popup or its button.
                POINT pt;
                GetCursorPos(&pt);
                RECT popup{};
                if (g_preview.wnd) GetWindowRect(g_preview.wnd, &popup);
                POINT local = pt;
                ScreenToClient(wnd, &local);
                if (g_preview.dismiss ||
                    (!PtInRect(&popup, pt) && !(g_preview.button >= 0 && HitTest(local) == g_preview.button)))
                    HidePreview();
            } else if (wParam == kTimerSystemTaskbar) {
                KillTimer(wnd, kTimerSystemTaskbar);  // (no longer used: the relay keeps it hidden)
            } else if (wParam == kTimerDragOpen) {
                OpenOverflowWhileDragging(wnd);
            } else if (wParam == kTimerTray || wParam == kTimerTrayAgain) {
                if (wParam == kTimerTrayAgain) KillTimer(wnd, kTimerTrayAgain);
                bool changed = FetchTray(wnd);
                if (changed) Layout(wnd);
                // The arrow's flyout opened or closed: the style draws the arrow the other way round.
                if (changed || (TrayArrowTheme() && ArrowOpen() != g_arrowDrawnOpen)) Redraw(wnd);
            }
            return 0;

        case WM_SIZE:
            Layout(wnd);
            Redraw(wnd);
            return 0;

        case WM_CAPTURECHANGED:
            if (g_buttonDrag >= 0 && (HWND)lParam != wnd) {
                if (g_buttonDragMoved) EndButtonDrag(wnd, false);
                g_buttonDrag = -1;
            }
            if (g_trayDrag >= 0 && (HWND)lParam != wnd) {
                if (g_trayDragMoved) CancelTrayDrag(wnd);
                g_trayDrag = -1;
            }
            if (g_pluginDrag >= 0 && (HWND)lParam != wnd) EndPluginDrag(wnd, false);
            if (g_pagerDrag && (HWND)lParam != wnd) {
                g_pagerDrag = false;
                if (g_pagerCarry) CancelPagerCarry();
            }
            return 0;

        case WM_MOUSEMOVE: {
            if (g_settings.idleFade && g_idle.to < 1) IdleTick(wnd);  // woken at once, not at the next check
            if (g_buttonDrag >= 0 && (wParam & MK_LBUTTON)) {
                POINT pt{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
                if (!g_buttonDragMoved && (abs(pt.x - g_buttonDragStart.x) > GetSystemMetrics(SM_CXDRAG) ||
                                           abs(pt.y - g_buttonDragStart.y) > GetSystemMetrics(SM_CYDRAG))) {
                    g_buttonDragMoved = true;
                    KillTimer(wnd, kTimerPreviewShow);
                    HidePreview();
                    if (g_tooltip) SendMessageW(g_tooltip, TTM_POP, 0, 0);
                }
                if (g_buttonDragMoved) {
                    DragButton(wnd, pt);
                    return 0;
                }
            }
            if (g_trayDrag >= 0 && (wParam & MK_LBUTTON)) {
                POINT pt{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
                if (!g_trayDragMoved && (abs(pt.x - g_trayDragStart.x) > GetSystemMetrics(SM_CXDRAG) ||
                                         abs(pt.y - g_trayDragStart.y) > GetSystemMetrics(SM_CYDRAG))) {
                    g_trayDragMoved = true;
                    HidePreview();
                    if (g_tooltip) SendMessageW(g_tooltip, TTM_POP, 0, 0);
                }
                if (g_trayDragMoved) {
                    DragTrayIcon(wnd, pt);
                    return 0;
                }
            }
            if (g_pluginDrag >= 0 && (wParam & MK_LBUTTON)) {
                POINT pt{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
                if (!g_pluginDragMoved && (abs(pt.x - g_trayDragStart.x) > GetSystemMetrics(SM_CXDRAG) ||
                                           abs(pt.y - g_trayDragStart.y) > GetSystemMetrics(SM_CYDRAG))) {
                    g_pluginDragMoved = true;
                    HidePreview();
                    if (g_tooltip) SendMessageW(g_tooltip, TTM_POP, 0, 0);
                }
                if (g_pluginDragMoved) {
                    DragPluginItem(wnd, pt);
                    return 0;
                }
            }
            if (g_pagerDrag && (wParam & MK_LBUTTON)) {
                // The desktops pager, unlocked: carried off its place, to another on the bar or anywhere.
                POINT pt{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
                if (!g_pagerCarry && (abs(pt.x - g_pagerDragStart.x) > GetSystemMetrics(SM_CXDRAG) ||
                                      abs(pt.y - g_pagerDragStart.y) > GetSystemMetrics(SM_CYDRAG)))
                    StartPagerCarry(wnd, g_pagerDragStart);
                if (g_pagerCarry) {
                    ClientToScreen(wnd, &pt);
                    CarryPager(pt);
                    return 0;
                }
            }
            int hot = HitTest({GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)});
            if (!g_tracking) {
                TRACKMOUSEEVENT tme{sizeof(tme), TME_LEAVE, wnd, 0};
                TrackMouseEvent(&tme);
                g_tracking = true;
            }
            if (hot != g_hot) {
                g_hot = hot;
                GlowTo(wnd, hot);
                if (hot >= 0 && hot < (int)g_buttons.size()) {
                    // Read its jump list ahead, for the right-click that may follow.
                    Button const& hovered = g_buttons[hot];
                    Win* w = hovered.windows.empty() ? nullptr : FindWin(hovered.windows[0]);
                    bool pinned = hovered.pin >= 0 && hovered.pin < (int)g_pins.size();
                    std::wstring appId = w && !w->appId.empty() ? w->appId : pinned ? g_pins[hovered.pin].appId : std::wstring();
                    PrefetchJumpList(appId, 16 * g_scale);
                }
                if (hot >= kHitTray && hot < kHitCss) HoverTrayIcon(hot - kHitTray);
                PreviewHover(wnd, hot);
                if (IsMainBar() && !g_pagerCarry) PagerHoverOnBar(wnd, hot);
                if (hot >= 0 && AuraOn() && g_settings.dynamicAura)
                    AuraFollowPointer(wnd, {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)}, true);
                Redraw(wnd);
            } else if (hot >= 0 && AuraOn() && g_settings.dynamicAura) {
                // The glow follows the pointer, drawn once per screen frame.
                AuraFollowPointer(wnd, {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)}, false);
            }
            return 0;
        }

        case WM_MOUSEWHEEL: {
            // Scrolling over System Icons' icons: the volume, the main
            // screen's brightness.
            POINT pt{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            ScreenToClient(wnd, &pt);
            int at = HitTest(pt);
            if (PagerIndexOf(at) >= 0) {
                PagerStep(GET_WHEEL_DELTA_WPARAM(wParam));
                return 0;
            }
            if (int item = PluginIndexOf(at); item >= 0) {
                PluginEvent(item, GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? kPluginScrollUp : kPluginScrollDown);
                return 0;
            }
            if (at >= kHitTray && at - kHitTray < (int)g_tray.size()) {
                HiddenIcon const& icon = g_tray[at - kHitTray].icon;
                if (int kind = SystemIconKind(icon))
                    PostMessageW(icon.owner, RegisterWindowMessageW(L"TourneSuite_RelayWheel"),
                                 (WPARAM)(short)GET_WHEEL_DELTA_WPARAM(wParam), kind);
            }
            return 0;
        }

        case WM_MOUSELEAVE:
            g_tracking = false;
            g_hot = -2;
            g_aura.placed = g_aura.running = false;
            GlowTo(wnd, -2);
            PreviewHover(wnd, -2);
            if (IsMainBar()) HideDesktopPreviewSoon();
            Redraw(wnd);
            return 0;

        case WM_MBUTTONDBLCLK:
            // On the bar's empty space with a double middle click set: that, and the single one waiting is off.
            if (HitTest({GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)}) == -2 && OnBarItself(wnd, lParam) &&
                g_settings.emptyMiddleDoubleClick != L"none") {
                KillTimer(wnd, kTimerEmptyMiddle);
                TmAction(L"tourne:" + g_settings.emptyMiddleDoubleClick);
                g_swallowMiddleUp = true;
                return 0;
            }
            [[fallthrough]];
        case WM_LBUTTONDBLCLK: {
            // On a tray icon the second click of a double click reaches its
            // program as one (the first already went through as a click).
            int at = HitTest({GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)});
            if (msg == WM_LBUTTONDBLCLK && at >= kHitTray && at - kHitTray < (int)g_tray.size()) {
                if (!OpensOnDoubleClick(g_tray[at - kHitTray].icon.owner)) ClickTrayIcon(at - kHitTray, kClickSecondOfDouble);
                g_swallowUp = true;
                return 0;
            }
            // On the bar's empty space (between segments too): as set, the desktop shown by default.
            if (msg == WM_LBUTTONDBLCLK && at == -2 && OnBarItself(wnd, lParam) && g_settings.emptyDoubleClick != L"none") {  // (-2: nothing there; -1 is Start)
                TmAction(L"tourne:" + g_settings.emptyDoubleClick);
                g_swallowUp = true;
                return 0;
            }
            // Anywhere else it's a press like the first.
            [[fallthrough]];
        }
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
            KillTimer(wnd, kTimerPreviewShow);
            if (IsMainBar()) HideDesktopPreview();
            g_pressed = HitTest({GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)});
            // A press on a tray icon may turn into a drag to move it.
            g_trayDrag = msg == WM_LBUTTONDOWN && IsMainBar() && g_pressed >= kHitTray && g_pressed - kHitTray < (int)g_tray.size()  // (moved on the main bar only)
                             ? g_pressed - kHitTray
                             : -1;
            g_trayDragStart = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            g_trayDragMoved = false;
            // Or on a plugin item, to move it among the tray's icons (along a horizontal main bar).
            g_pluginDrag = msg == WM_LBUTTONDOWN && IsMainBar() && !Vertical() ? PluginIndexOf(g_pressed) : -1;
            g_pluginDragMoved = false;
            g_pluginDragSlot = -1;
            // Or on a task button, to move it along the bar.
            g_buttonDrag = msg == WM_LBUTTONDOWN && g_pressed >= 0 && g_pressed < (int)g_buttons.size() ? g_pressed : -1;
            g_buttonDragKey = g_buttonDrag >= 0 ? ButtonKey(g_buttons[g_buttonDrag]) : L"";
            if (g_buttonDragKey.empty()) g_buttonDrag = -1;
            g_buttonDragStart = g_trayDragStart;
            g_buttonDragMoved = false;
            // Or on the desktops pager, unlocked, to carry it somewhere else.
            g_pagerDrag = msg == WM_LBUTTONDOWN && PagerIndexOf(g_pressed) >= 0 && !g_pagerChoices.locked;
            g_pagerDragStart = g_trayDragStart;
            // Our Start menu opens on the press, as a menu does (the release then does nothing).
            g_startOnPress = msg == WM_LBUTTONDOWN && g_pressed == -1 && g_startConfig.onPress && ToggleStartMenu();
            if (g_startOnPress) {
                g_pressed = -2;
                Redraw(wnd);
                return 0;
            }
            SetCapture(wnd);
            Redraw(wnd);
            return 0;

        case WM_LBUTTONUP:
        case WM_MBUTTONUP: {
            int dragged = g_trayDragMoved ? g_trayDrag : -1;
            g_trayDrag = -1;  // before ReleaseCapture, so it isn't taken for a cancel
            g_trayDragMoved = false;
            bool pluginDragged = g_pluginDrag >= 0 && g_pluginDragMoved;
            if (pluginDragged) {
                // A plugin item let go: on the overflow arrow or the open flyout, into the overflow; on the bar,
                // in its new place; anywhere else, back where it was.
                RECT client;
                GetClientRect(wnd, &client);
                POINT pt{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)}, screen = pt;
                ClientToScreen(wnd, &screen);
                std::wstring key = g_pluginDrag < (int)g_pluginItems.size() ? L"plugin:" + g_pluginItems[g_pluginDrag].key : L"";
                HWND flyout = PtInRect(&client, pt) ? nullptr : DropTargetAt(screen, kFlyoutClassPrefix);
                if (g_trayDragOnArrow && !key.empty()) TrayPlaces::Load().Set(key, kPlaceOverflow);
                else if (PtInRect(&client, pt)) SaveTrayOrder();  // (as laid out now)
                else if (flyout && !key.empty()) SendTrayDrop(flyout, wnd, screen, key);
                BuildTray();  // (left out of the bar when it went to the overflow)
                g_pluginDrag = -1;  // before ReleaseCapture, so it isn't taken for a cancel
                ReleaseCapture();
                EndPluginDrag(wnd, false);
                return 0;
            }
            g_pluginDrag = -1;
            g_pluginDragSlot = -1;
            bool buttonDragged = g_buttonDragMoved && g_buttonDrag >= 0;
            g_buttonDrag = -1;
            bool pagerCarried = g_pagerDrag && g_pagerCarry;
            g_pagerDrag = false;
            ReleaseCapture();
            if (pagerCarried) {
                POINT screen{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
                ClientToScreen(wnd, &screen);
                g_pressed = -2;
                DropPager(screen);
                return 0;
            }
            if (buttonDragged) {
                // Dropped on the bar: that's the order now; off it, back as it was.
                RECT client;
                GetClientRect(wnd, &client);
                EndButtonDrag(wnd, PtInRect(&client, {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)}) != FALSE);
                return 0;
            }
            g_buttonDragMoved = false;
            if (dragged >= 0) {
                DropTrayIcon(wnd, dragged, {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)});
                return 0;
            }
            if (g_swallowUp && msg == WM_LBUTTONUP) {
                g_swallowUp = false;
                return 0;
            }
            int at = HitTest({GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)});
            int pressed = g_pressed;
            g_pressed = -2;
            Redraw(wnd);
            if (at != pressed) return 0;
            // The bar's empty space: the Tourne Menu, when it's set to open there too.
            if (at == -2 && msg == WM_MBUTTONUP && g_settings.menuEnabled && g_settings.menuOnBar && g_settings.menuButton == 1) {
                ShowTourneMenu(wnd);
                return 0;
            }
            // Else its middle click as set (waiting first when a double one could still come).
            if (at == -2 && msg == WM_MBUTTONUP && OnBarItself(wnd, lParam)) {
                if (g_swallowMiddleUp) {
                    g_swallowMiddleUp = false;
                } else if (g_settings.emptyMiddleDoubleClick != L"none" && g_settings.emptyMiddleClick != L"none") {
                    SetTimer(wnd, kTimerEmptyMiddle, GetDoubleClickTime(), nullptr);
                } else if (g_settings.emptyMiddleClick != L"none") {
                    TmAction(L"tourne:" + g_settings.emptyMiddleClick);
                }
                return 0;
            }
            if (int desktop = PagerIndexOf(at); desktop >= 0) {
                if (msg == WM_LBUTTONUP) PagerGoTo(desktop);
                else PagerRename(desktop, PagerMarkOnScreen(wnd, desktop), true);  // a middle click names it
                return 0;
            }
            if (int item = PluginIndexOf(at); item >= 0) {
                PluginEvent(item, msg == WM_MBUTTONUP ? kPluginMiddleClick : kPluginClick);
                return 0;
            }
            if (at >= kHitCss) {
                // One of a YASB theme's own buttons.
                int index = at - kHitCss;
                if (index < (int)g_cssTargets.size()) RunCssAction(wnd, msg == WM_MBUTTONUP ? g_cssTargets[index].middle : g_cssTargets[index].left, &g_cssTargets[index].rc);
                return 0;
            }
            if (at >= kHitTray) {
                int index = at - kHitTray;
                if (index < (int)g_tray.size()) {
                    int click = msg == WM_MBUTTONUP                            ? kClickMiddle
                              : OpensOnDoubleClick(g_tray[index].icon.owner) ? kClickDouble
                                                                               : kClickLeft;
                    ClickTrayIcon(index, click);
                }
                return 0;
            }
            if (at == kHitClock) {
                if (CssActive()) RunCssAction(wnd, CssClockAction(msg == WM_MBUTTONUP ? 2 : 0), &g_clockRect);
                else ClockClick(wnd, msg == WM_MBUTTONUP ? 2 : 0);
                return 0;
            }
            if (at == kHitShowDesktop) {
                if (msg == WM_LBUTTONUP) ToggleShowDesktop();  // shows the desktop, or brings the windows back
                return 0;
            }
            if (at == -1) {
                if (msg == WM_LBUTTONUP && !g_startOnPress && !ToggleStartMenu()) SendKeys({VK_LWIN});
            } else if (at >= 0 && at < (int)g_buttons.size()) {
                Button b = g_buttons[at];
                Win* w = b.windows.empty() ? nullptr : FindWin(b.windows[0]);
                // A pinned app that isn't running starts; middle click or
                // Shift+click starts another window (through its pin, which
                // keeps its arguments, like a browser profile).
                if (b.windows.empty() || msg == WM_MBUTTONUP || (wParam & MK_SHIFT)) {
                    HidePreview();
                    if (b.pin >= 0) LaunchPin(b.pin);
                    else if (w) Launch(w->exe);
                } else if ((b.windows.size() > 1 && ((wParam & MK_CONTROL) || !ActiveOfGroup(b))) ||
                           ((wParam & MK_CONTROL) && g_settings.previews)) {
                    // Ctrl+click (a group, or with previews any button), or a group with nothing of it
                    // to bring up: its windows to pick from.
                    if (!g_settings.previews) PickWindow(wnd, b);
                    else if (g_preview.button != at || !g_preview.wnd || g_preview.bar != wnd) ShowPreview(wnd, at);
                } else if (b.windows.size() > 1) {
                    // A group: the window of it used last comes up (or goes down, if it's the one in front).
                    HidePreview();
                    if (w) w->flashing = false;
                    ClickWindow(ActiveOfGroup(b));
                } else {
                    HidePreview();
                    if (w) w->flashing = false;
                    ClickWindow(b.windows[0]);
                }
            }
            return 0;
        }

        case WM_RBUTTONUP: {
            int at = HitTest({GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)});
            if (int desktop = PagerIndexOf(at); desktop >= 0) {
                PagerMenu(desktop, PagerMarkOnScreen(wnd, desktop));
            } else if (int item = PluginIndexOf(at); item >= 0) {
                // Shift+right-click: where it goes and its icon. Otherwise its own menu.
                if (GetKeyState(VK_SHIFT) < 0) PluginItemMenu(wnd, item);
                else PluginEvent(item, kPluginRightClick);
            } else if (at >= kHitCss) {
                int index = at - kHitCss;
                if (index < (int)g_cssTargets.size() && !g_cssTargets[index].right.empty()) RunCssAction(wnd, g_cssTargets[index].right, &g_cssTargets[index].rc);
                else BarMenu(wnd);
            } else if (at >= kHitTray) {
                // Shift+right-click: where the icon goes. Otherwise its own menu.
                if (GetKeyState(VK_SHIFT) < 0) TrayIconMenu(wnd, at - kHitTray);
                else ClickTrayIcon(at - kHitTray, kClickRight);
            } else if (at == kHitClock) {
                if (CssActive()) RunCssAction(wnd, CssClockAction(1), &g_clockRect);
                else ClockClick(wnd, 1);
            } else if (at == -1) {
                if (!PowerUserMenu(wnd)) SendKeys({VK_LWIN, 'X'});  // the power user menu
            } else if (at >= 0 && at < (int)g_buttons.size()) {
                // Shift+right-click: the window's commands. Otherwise its jump list.
                Button b = g_buttons[at];
                g_menuButton = b.rc;
                MapWindowPoints(wnd, nullptr, (POINT*)&g_menuButton, 2);
                g_menuFromButton = true;
                if (GetKeyState(VK_SHIFT) < 0) WindowMenu(wnd, b);
                else ButtonMenu(wnd, b);
                g_menuFromButton = false;
            } else {
                BarMenu(wnd);
            }
            return 0;
        }

        case WM_MOUSEACTIVATE:
            return MA_NOACTIVATE;

        case WM_APP_SHYFRAME:
            if (g_pacer) {
                g_pacer->posted = false;
                if (g_shyRunning) ShyFrame(wnd);
                if (g_aura.running) AuraFrame(wnd);
                if (g_idle.running) IdleFrame(wnd);
                if (g_active->arrive.running) ArriveFrame(wnd);
                if (!g_shyRunning && !g_aura.running && !g_idle.running && !g_active->arrive.running) StopPacer(wnd);
            }
            return 0;

        case WM_APP_EXIT:
            DestroyWindow(wnd);
            return 0;

        case WM_DESTROY:
            DragTargetStop(wnd);
            if (!IsMainBar()) {
                // Another screen's bar closing: its own timers, popup and space.
                for (UINT_PTR timer : {kTimerAutoHide, kTimerDynamic, kTimerGlow, kTimerPreviewShow, kTimerPreviewHide, kTimerIdle})
                    KillTimer(wnd, timer);
                StopPacer(wnd);
                if (g_preview.bar == wnd) HidePreview();
                SetAppBar(wnd, false);
                // Its own fonts and clock (the globals hold this bar's while it handles a message).
                FreeClockFonts();
                g_clock = Clock();
                if (g_font) DeleteObject(g_font);
                g_font = nullptr;
                if (g_tipFont) DeleteObject(g_tipFont);
                g_tipFont = nullptr;
                return 0;
            }
            // The bars of the other screens go first.
            for (size_t i = g_bars.size(); i-- > 1;) {
                BarState* other = g_bars[i];
                if (other->hwnd) DestroyWindow(other->hwnd);
                g_bars.erase(g_bars.begin() + i);
                delete other;
            }
            UnregisterHotKey(wnd, kHotkeyHide);
            UnregisterDesktopKeys(wnd);
            KillTimer(wnd, kTimerRescan);
            KillTimer(wnd, kTimerAutoHide);
            KillTimer(wnd, kTimerIdle);
            RestUnhook();
            g_resting = false;
            StopPacer(wnd);
            KillTimer(wnd, kTimerTray);
            KillTimer(wnd, kTimerTrayAgain);
            HidePreview();
            HideSystemTaskbar(wnd, false);  // StartAllBack's taskbar back as it was
            DeregisterShellHookWindow(wnd);
            SetAppBar(wnd, false);
            g_wnd = nullptr;
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(wnd, msg, wParam, lParam);
}

DWORD WINAPI ThreadProc(LPVOID parameter) {
    HANDLE ready = (HANDLE)parameter;
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    INITCOMMONCONTROLSEX icc{sizeof(icc), ICC_WIN95_CLASSES};
    InitCommonControlsEx(&icc);
    EnsureTourneFolders();

    HINSTANCE instance = nullptr;
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                       (LPCWSTR)&WndProc, &instance);
    WNDCLASSW wc{};
    wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = instance;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.lpszClassName = kWindowClass;
    RegisterClassW(&wc);
    WNDCLASSW preview{};
    preview.lpfnWndProc = PreviewProc;
    preview.hInstance = instance;
    preview.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    preview.lpszClassName = kPreviewClass;
    RegisterClassW(&preview);
    preview.style |= CS_DROPSHADOW;
    preview.lpszClassName = kPreviewShadowClass;
    RegisterClassW(&preview);
    RegisterPagerClass(instance);
    g_instance = instance;

    PerfMark(L"(thread started)");
    HWND wnd = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE | WS_EX_LAYERED | WS_EX_ACCEPTFILES, kWindowClass,
                               L"Tourne taskbar", WS_POPUP, 0, 0, 1, 1, nullptr, nullptr, instance, &g_mainBar);
    PerfMark(L"window created");
    g_wnd = wnd;
    if (wnd) g_bars.push_back(&g_mainBar);
    KeepOutOfPeek(wnd);
    BootMark(L"bar window made");
    // The rest of the suite starts now, instead of waiting for the bar; the bar sets itself up
    // once Windhawk has put every hook in (Icon Redirect's among them), so each icon is read once,
    // already themed.
    SetEvent(ready);
    if (g_hooksLive) WaitForSingleObject(g_hooksLive, 3000);
    BootMark(L"hooks in");
    if (wnd) ApplySettings(wnd);
    BootMark(L"bar shown");
    if (wnd) SetTimer(wnd, kTimerBootLog, 30000, nullptr);  // written sooner once the Start menu is warm

    // Things that change without telling anyone, watched so the bar follows them live: StartAllBack's
    // style, orb and colours (registry values), and the folders themes and orbs are kept in (a theme
    // file being saved, or one installed, or removed). A burst of changes is one reload.
    HKEY sabKey = nullptr;
    std::vector<HANDLE> watches;  // [0] the registry key, then the folders
    watches.push_back(CreateEventW(nullptr, FALSE, FALSE, nullptr));
    HANDLE sabChanged = watches[0];
    // [1] Windows' virtual desktops (Win+Tab): a desktop added, removed or switched to, so the bar follows at once.
    watches.push_back(CreateEventW(nullptr, FALSE, FALSE, nullptr));
    HANDLE desktopsChanged = watches[1];
    HKEY desktopsKey = nullptr;
    auto watchDesktops = [&] {
        if (!desktopsKey && RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\VirtualDesktops", 0, KEY_NOTIFY, &desktopsKey) != ERROR_SUCCESS)
            desktopsKey = nullptr;
        if (desktopsKey) RegNotifyChangeKeyValue(desktopsKey, TRUE, REG_NOTIFY_CHANGE_LAST_SET | REG_NOTIFY_CHANGE_NAME, desktopsChanged, TRUE);
    };
    watchDesktops();
    auto watchSab = [&] {
        if (!sabKey && RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\StartIsBack", 0, KEY_NOTIFY, &sabKey) != ERROR_SUCCESS)
            sabKey = nullptr;
        if (sabKey) RegNotifyChangeKeyValue(sabKey, TRUE, REG_NOTIFY_CHANGE_LAST_SET | REG_NOTIFY_CHANGE_NAME, sabChanged, TRUE);
    };
    watchSab();
    auto watchFolder = [&](std::wstring folder, bool subtree = false) {
        if (folder.empty() || GetFileAttributesW(folder.c_str()) == INVALID_FILE_ATTRIBUTES) return;
        HANDLE h = FindFirstChangeNotificationW(folder.c_str(), subtree,
                                                FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_SIZE);
        if (h != INVALID_HANDLE_VALUE) watches.push_back(h);
    };
    watchFolder(TourneYasb(), true);  // YASB themes: a theme's files, or a theme folder added
    watchFolder(TourneStyles());
    watchFolder(SabStyles());
    for (auto const& folder : OrbFolders()) watchFolder(folder);

    MSG msg;
    while (wnd) {
        DWORD woke = MsgWaitForMultipleObjects((DWORD)watches.size(), watches.data(), FALSE, INFINITE, QS_ALLINPUT);
        if (woke < WAIT_OBJECT_0 + watches.size()) {
            size_t which = woke - WAIT_OBJECT_0;
            if (which == 1) {
                watchDesktops();
                CssDesktopsChanged();
                PagerDesktopsChanged();
                if (g_css.loaded) Refresh(wnd);  // the workspace buttons and the windows of the new desktop
                continue;
            }
            if (which == 0) watchSab();
            else FindNextChangeNotification(watches[which]);
            SetTimer(wnd, kTimerReload, 400, nullptr);  // StartAllBack writes several values in a row; a file takes a moment to be saved
            continue;
        }
        bool quit = false;
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) { quit = true; break; }
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        if (quit) break;
    }    if (sabKey) RegCloseKey(sabKey);
    CloseHandle(sabChanged);
    if (desktopsKey) RegCloseKey(desktopsKey);
    CloseHandle(desktopsChanged);
    for (size_t i = 2; i < watches.size(); i++) FindCloseChangeNotification(watches[i]);
    cssdraw::FontCache().Clear();
    gallery::Close();
    ClosePager();
    CssPopupShutdown();
    CssUnloadFonts();
    if (g_toast) DestroyWindow(g_toast);
    g_toast = nullptr;

    for (auto& w : g_wins)
        if (w.ownsIcon && w.icon) DestroyIcon(w.icon);
    g_wins.clear();
    ClearIconCache();
    g_shortcuts.clear();
    if (g_resolver) g_resolver->Release();
    g_resolver = nullptr;
    g_tray.clear();
    g_trayAll.clear();
    FreeClockFonts();
    if (g_tipFont) DeleteObject(g_tipFont);
    g_tipFont = nullptr;
    g_buttons.clear();
    g_bars.clear();
    g_mainBar.hwnd = nullptr;
    g_curWnd = nullptr;
    FreeJumpCache();
    FreeAlphaScratch();
    if (g_menuIconFont) DeleteObject(g_menuIconFont);
    g_menuIconFont = nullptr;
    DestroyStartMenu();
    g_theme.Close();
    g_orb.Close();
    if (g_font) DeleteObject(g_font);
    g_font = nullptr;
    UnregisterClassW(kWindowClass, instance);
    UnregisterClassW(kPreviewClass, instance);
    UnregisterClassW(kPreviewShadowClass, instance);
    CoUninitialize();
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Mod lifecycle

BOOL Wh_ModInit() {
    Wh_Log(L">");
    BootMark(L"taskbar starting");
    g_hooksLive = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    HANDLE ready = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    g_thread = CreateThread(nullptr, 0, ThreadProc, ready, 0, nullptr);
    if (!g_thread) {
        CloseHandle(ready);
        return FALSE;
    }
    WaitForSingleObject(ready, 5000);
    CloseHandle(ready);
    return TRUE;
}

// Hooks go live only now, Icon Redirect's among them: the bar has been
// waiting for this to set itself up.
void Wh_ModAfterInit() {
    if (g_hooksLive) SetEvent(g_hooksLive);
}

void Wh_ModUninit() {
    Wh_Log(L">");
    if (g_hooksLive) SetEvent(g_hooksLive);  // (unloaded before Windhawk finished loading it)
    if (HWND wnd = g_wnd) PostMessageW(wnd, WM_APP_EXIT, 0, 0);
    if (g_thread) {
        WaitForSingleObject(g_thread, 10000);
        CloseHandle(g_thread);
        g_thread = nullptr;
    }
    if (g_hooksLive) CloseHandle(g_hooksLive);
    g_hooksLive = nullptr;
}

void Wh_ModSettingsChanged() {
    Wh_Log(L">");
    if (HWND wnd = g_wnd) PostMessageW(wnd, WM_APP_SETTINGS, 0, 0);
}
