// ==WindhawkMod==
// @id              tourne-switcher
// @name            Tourne'Switcher
// @description     An Alt+Tab window switcher in the Tourne style, themed like the rest of the suite
// @version         0.2.0
// @author          USER-TOURNE
// @github          https://github.com/USER-TOURNE
// @include         windhawk.exe
// @architecture    x86-64
// @compilerOptions -luxtheme -ldwmapi -lole32 -lshell32 -lshlwapi -lgdi32 -luser32 -lversion -ld3d11 -ldxgi -ld2d1 -ldwrite -lruntimeobject
// @license         GPL-3.0
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# `Tourne'Switcher`

Our own Alt+Tab: your open windows, one entry per app or per window, drawn in
the suite's style. Hold Alt and press Tab to move through them, let go of Alt
to switch.

- **Tab / Shift+Tab**, the **arrow keys** or the **mouse wheel** move the
  selection; **Enter** or a click switches; **Esc** closes it.
- **Alt+`** goes backwards, or lists only the current app's windows.
- **Ctrl** (tapped while it's open) opens an app's entry into its windows;
  Esc goes back out.
- **Q**, **Delete** or **Ctrl+W** closes the selected window; a middle click
  closes the one under the pointer.
- **Alt+Ctrl+Tab** keeps it open after Alt is let go.
- **Win+Alt+Tab** shows every screen's windows when it's set to show one
  screen's.

**Style** picks the colours: *Tourne'Style* (the preset these settings start
from), the suite's colour scheme, the taskbar's visual style, or any theme in
StartAllBack's Styles folder or `%LOCALAPPDATA%\Tourne\Styles`. Any colour can
be set by hand over it, for dark and light mode.

While a window running as administrator has the keyboard, Windows keeps the
keys from the switcher and its own Alt+Tab shows. Turn off other Alt+Tab mods
(Simple Window Switcher) while this is on.
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- theme:
  - style: auto
    $name: Style
    $description: >-
      Automatic starts from what you already have: Simple Window Switcher's
      colours if you've used it, otherwise your visual style's (the .msstyles
      Windows is using), otherwise Windows' own look. Or pick Tourne'Style,
      the suite's colour scheme, the taskbar's visual style, or any theme's
      menu colours.
    #! $dynamicSelect: true
  - backdrop: none
    $name: Backdrop
    $description: Acrylic blurs what's behind the switcher; lower the background opacity to see it.
    $options:
    - none: None (solid or see-through)
    - acrylic: Acrylic
  - colorMode: system
    $name: Light or dark
    $description: Which of the two colour sets below is used.
    $options:
    - system: Follow Windows
    - dark: Dark
    - light: Light
  - highlight: fillAndBorder
    $name: Selection style
    $options:
    - border: Border only
    - fill: Background fill
    - fillAndBorder: Background fill and border
  - opacity: 100
    $name: Background opacity (%)
    #! $min: 0
    #! $max: 100
  $name: Theme
- colors:
  - background: ''
    $name: Background
  - border: ''
    $name: Border
  - selection: ''
    $name: Selection
  - selectionBorder: ''
    $name: Selection border
  - text: ''
    $name: Text
  - selectedText: ''
    $name: Selected text
  - iconBackground: ''
    $name: Badge icon background
  - iconBackgroundOpacity: 55
    $name: Badge icon background opacity (%)
  - count: ''
    $name: Window count background
  - countOpacity: 25
    $name: Window count background opacity (%)
  - countText: ''
    $name: Window count text
  $name: Colours (dark mode)
  $description: >-
    Hex colours, like #66EA97, over the style's own. Leave one empty to keep
    the style's.
- lightColors:
  - background: ''
    $name: Background
  - border: ''
    $name: Border
  - selection: ''
    $name: Selection
  - selectionBorder: ''
    $name: Selection border
  - text: ''
    $name: Text
  - selectedText: ''
    $name: Selected text
  - iconBackground: ''
    $name: Badge icon background
  - iconBackgroundOpacity: 55
    $name: Badge icon background opacity (%)
  - count: ''
    $name: Window count background
  - countOpacity: 85
    $name: Window count background opacity (%)
  - countText: ''
    $name: Window count text
  $name: Colours (light mode)
  $description: >-
    Hex colours for light mode. Tourne'Style has a light set of its own; the
    other styles use their one look in both modes.
- corners:
  - radius: 0
    $name: Corner radius (px)
    $description: 0 is square.
  - roundItems: false
    $name: Round the items and close button
  - roundCount: false
    $name: Round the window count
  - roundIconBackground: false
    $name: Round the badge icon background
  $name: Corners
- thumbnails:
  - show: false
    $name: Show thumbnails
    $description: A live preview of each window.
  - position: bottom
    $name: Thumbnail position
    $options:
    - bottom: Below the title
    - top: Above the title
    #! $showIf: {show: true}
  - alignment: center
    $name: Thumbnail alignment
    $options:
    - left: Left
    - center: Centre
    - right: Right
    #! $showIf: {show: true}
  - stretch: true
    $name: Stretch thumbnails to the item's width
    #! $showIf: {show: true}
  - closeButton: true
    $name: Close button
    $description: On the item under the pointer.
  - hoverBorder: true
    $name: Border on the thumbnail under the pointer
    #! $showIf: {show: true}
  - shadow: true
    $name: Thumbnail shadow
    #! $showIf: {show: true}
  $name: Thumbnails
- header:
  - showTitle: true
    $name: Show the title
  - showIcon: false
    $name: Show the icon
  - iconSize: small
    $name: Icon size
    $options:
    - small: Small (16)
    - medium: Medium (24)
    - large: Large (32)
  - centered: true
    $name: Centre the icon and title
  - orientation: vertical
    $name: Icon and title
    $options:
    - vertical: Icon above the title
    - horizontal: Icon beside the title
  $name: Title and icon
- view:
  - style: list
    $name: View
    $description: >-
      How Alt+Tab shows your windows. Flip 3D stacks them on a slant with the
      chosen one in front; Cover Flow lines them up with the chosen one facing
      you and the rest turned to the sides. Grid shows them all at once;
      Mission Control does too, keeping them roughly where they sit on the
      screen. Ring stands them in a turning circle; Shelf in one row like a
      dock, magnified under the pointer. All fly your windows in from where they are, over a
      dimmed, blurred copy of the screen; the motion settings' "switcher" line
      sets how they move.
    $options:
    - list: The list (the settings below)
    - flip3d: Flip 3D
    - coverflow: Cover Flow
    - grid: Grid
    - mission: Mission Control
    - ring: Ring
    - shelf: Shelf
    - canvas: Canvas (PrinceWM's infinite canvas)
  - overview: canvas
    $name: Overview (Win+Tab and the overview command)
    $description: >-
      The canvas (PrinceWM's): every window live on a canvas you pan, flick and
      zoom, tiles kept where you put them, an app's windows in one stack
      (middle click opens it), a tile dropped on another puts the two side by
      side, notes and pasted pictures pinned, and paint. Or Mission Control.
    $options:
    - canvas: Canvas
    - mission: Mission Control
  - live: true
    $name: Live windows
    $description: >-
      The windows keep moving in the view (Windows' own screen capture,
      Windows 10 1903 and later). Off: a still of each, taken as it opens.
  - dim: 55
    $name: Background dim (%)
  - blur: 6
    $name: Background blur
    $description: 0 to 10.
  $name: View
- layout:
  - orientation: vertical
    $name: Task list
    $options:
    - vertical: Vertical list
    - horizontal: Horizontal grid
  - rowHeight: 230
    $name: Item height with thumbnails (px)
    #! $showIf: {thumbnails.show: true}
  - rowWidth: 0
    $name: Item width (px)
    $description: 0 fits the content.
  - maxWidth: 25
    $name: Largest width (% of the screen)
  - maxHeight: 35
    $name: Largest height (% of the screen)
  - shrinkToFit: true
    $name: Shrink the items to fit
    $description: Instead of scrolling, when there are more than fit.
  - overflowIndicator: true
    $name: Arrows where more items are out of view
  $name: Layout
- position:
  - place: center
    $name: Place on the screen
    $options:
    - center: Centre
    - top: Top
    - bottom: Bottom
    - left: Left
    - right: Right
    - topLeft: Top left
    - topRight: Top right
    - bottomLeft: Bottom left
    - bottomRight: Bottom right
  - offsetX: 0
    $name: Horizontal distance (px)
    $description: >-
      From the left or right edge it sits against; when centred across, a
      shift to the right (negative for left).
    #! $min: -4000
    #! $max: 4000
  - offsetY: 0
    $name: Vertical distance (px)
    $description: >-
      From the top or bottom edge it sits against; when centred up and down, a
      shift downwards (negative for up).
    #! $min: -4000
    #! $max: 4000
  - monitor: cursor
    $name: Show on
    $options:
    - cursor: The screen with the pointer
    - active: The screen with the active window
    - primary: The main screen
  $name: Position
- badgeLayout:
  - use: false
    $name: Badge layout (macOS style)
    $description: With thumbnails, the app's icon sits on its thumbnail as a badge.
  - iconPosition: center
    $name: Badge position
    $options:
    - topLeft: Top left
    - topCenter: Top centre
    - topRight: Top right
    - center: Centre
    - bottomLeft: Bottom left
    - bottomCenter: Bottom centre
    - bottomRight: Bottom right
    #! $showIf: {use: true}
  - titlePosition: above
    $name: Title
    $options:
    - above: Above the thumbnail
    - below: Below the thumbnail
    #! $showIf: {use: true}
  - iconBackground: false
    $name: Badge icon background
    #! $showIf: {use: true}
  - iconShadow: true
    $name: Badge icon shadow
    #! $showIf: {use: true}
  - padding: 4
    $name: Badge padding (px)
    #! $showIf: {use: true}
  - offsetX: 0
    $name: Badge offset across (px)
    #! $showIf: {use: true}
  - offsetY: 0
    $name: Badge offset down (px)
    #! $showIf: {use: true}
  $name: Badge layout
- font:
  - family: Scientifica
    $name: Font
  - size: 8
    $name: Size (pt)
  - style: bold
    $name: Style
    $options:
    - regular: Regular
    - bold: Bold
    - italic: Italic
    - boldItalic: Bold italic
  - forCount: true
    $name: Use it for the window count too
  $name: Font
- grouping:
  - apps: true
    $name: One entry per app
  - titles: appName
    $name: Titles
    $options:
    - appName: The app's name
    - windowTitle: The window's title
  - restoreAll: true
    $name: Bring back all of an app's windows
  - showCount: true
    $name: Window count on apps with several
  - countShadow: true
    $name: Window count shadow
  - closeBehavior: closeRecent
    $name: Closing an app's entry closes
    $options:
    - closeRecent: Its last used window
    - closeAll: All its windows
  $name: Grouping
- keys:
  - showDelay: 0
    $name: Show after (ms)
    $description: A quick Alt+Tab switches without the list flashing up.
  - replaceWindows: true
    $name: Replace Windows' own Alt+Tab
    $description: >-
      Windows' Alt+Tab never shows while this runs; an Alt+Tab that reaches it
      opens this instead. Needs the suite's Explorer relay. Win+Tab is
      untouched.
  - winTab: false
    $name: Win+Tab opens Mission Control
    $description: >-
      Win+Tab opens this as Mission Control (every window laid out where it
      sits) instead of Windows' Task view, and it stays open until you pick a
      window, click away or press Esc.
  - peek: false
    $name: Peek at the selected window
    $description: >-
      The other windows fade away and the selected one shows where it is, as
      the taskbar's previews do.
  - peekDelay: 500
    $name: Peek after (ms)
    #! $min: 0
    #! $max: 3000
  - backtick: none
    $name: Alt+`
    $description: >-
      Off until you pick what it does. On, the suite takes Alt+` for itself:
      the program in front never hears it, so any shortcut it had on the same
      keys silently stops working.
    $options:
    - none: Off (left to the program in front)
    - backward: Goes backwards
    - sameApp: Only the current app's windows
  - wheelOpens: never
    $name: Scroll wheel opens it
    $options:
    - never: Never
    - withAlt: With Alt held
  - wheelAction: selection
    $name: Scroll wheel
    $options:
    - selection: Moves the selection
    - page: Moves a page
  - wheelModifier: shift
    $name: Key for the other scroll action
    $options:
    - shift: Shift
    - ctrl: Ctrl
  - reverseWheel: false
    $name: Reverse the scroll direction
  $name: Keys and scrolling
- windows:
  - desktops: all
    $name: Virtual desktops
    $options:
    - all: Windows on every desktop
    - current: Only this desktop's
  - thisMonitorOnly: false
    $name: Only the windows on the screen it shows on
    $description: Win+Alt+Tab still shows every screen's.
  - minimized: inPlace
    $name: Minimized windows
    $options:
    - inPlace: In their place
    - last: At the end
    - hidden: Left out
  - excludeTitles: ''
    $name: Leave out windows titled
    $description: Separated by commas; * and ? match any text.
  - excludeApps: ''
    $name: Leave out programs
    $description: Program file names, separated by commas, like notepad.exe; * and ? match any text.
  $name: Windows listed
- tiling:
  - leader: ''
    $name: Tiling key
    $description: >-
      The key you hold for the tiling's shortcuts. None until you set one.
      Hold it and press Tab to tile the screen with the active window; the
      arrows to move the active window that way; H, J, K, L to focus the
      window left, down, up, right (in Monocle, left and right go through the
      windows in turn); Space to float or tile the active window; N for the
      next layout; [ and ] to shrink or grow the main window; A to turn
      automatic tiling on or off; M to open Window layouts, where layouts are
      saved, applied, edited, renamed and deleted; 1 to 9 to apply the first
      nine. The layout and size you pick are kept for that screen on that
      desktop, across restarts too. Pressed on its own the key still does
      what it did, and in a fullscreen app the keys are left alone. Once set,
      the suite takes that key press for itself while it's held: the program
      in front never hears it or the keys pressed with it, so any shortcut it
      had on the same keys silently stops working. Pick a combination with
      two modifiers (like Ctrl+Alt or Win+Alt) that nothing else you use
      takes; with just Ctrl, Ctrl+S could stop saving without any sign. With
      Win in it, Win+L still locks the PC, as nothing can take that.
    #! $format: hotkey
  - auto: false
    $name: Tile automatically
    $description: >-
      Windows are tiled as they open, close, minimise and restore; a dragged
      window dropped on another's place swaps with it.
  - layout: columns
    $name: Layout
    $options:
    - columns: Columns
    - rows: Rows
    - verticalStack: Vertical stack (main on the left)
    - rightMainVerticalStack: Vertical stack (main on the right)
    - horizontalStack: Horizontal stack (main on top)
    - ultrawideVerticalStack: Ultrawide vertical stack (main in the middle)
    - grid: Grid
    - bsp: BSP (each window halves the space left)
    - spiral: Spiral (each window takes a side of what's left, winding inwards)
    - threeColumn: Three columns (main in the middle, a column each side)
    - slice: Slice (one window keeps a strip along an edge, the rest laid out in what's left)
    - screenSplit: Split screen (named regions, each with its own layout and its own windows)
    - script: Your own layout (a .luau file in the Layouts folder)
    - monocle: Monocle (every window fills the screen)
    - layers: Layers of three (one fills, two halves, three main and two; the 4th to 6th lie over them the same way)
  - otherLayout: same
    $name: Layout on other screens
    $options:
    - same: As the main screen
    - off: Don't tile them
    - columns: Columns
    - rows: Rows
    - verticalStack: Vertical stack (main on the left)
    - rightMainVerticalStack: Vertical stack (main on the right)
    - horizontalStack: Horizontal stack (main on top)
    - ultrawideVerticalStack: Ultrawide vertical stack (main in the middle)
    - grid: Grid
    - bsp: BSP (each window halves the space left)
    - spiral: Spiral (each window takes a side of what's left, winding inwards)
    - threeColumn: Three columns (main in the middle, a column each side)
    - slice: Slice (one window keeps a strip along an edge, the rest laid out in what's left)
    - screenSplit: Split screen (named regions, each with its own layout and its own windows)
    - script: Your own layout (a .luau file in the Layouts folder)
    - monocle: Monocle (every window fills the screen)
    - layers: Layers of three (one fills, two halves, three main and two; the 4th to 6th lie over them the same way)
  - layoutRules: ''
    $name: Layout by window count
    $description: >-
      From so many windows on a screen, another layout, as komorebi's
      layout_rules: 1: bsp, 4: grid is BSP from one window and Grid from
      four. Layout names as above (columns, rows, verticalStack, grid, bsp,
      monocle...). Empty: the layout above whatever the count. A layout you
      pick with the Tiling key's N stays put on that screen.
  - mainRatio: 50
    $name: Main window's share (%)
    $description: For the stacks and BSP.
    #! $min: 10
    #! $max: 90
  - gapTop: 4
    $name: Gap at the top (px)
    #! $min: 0
    #! $max: 400
  - gapBottom: 2
    $name: Gap at the bottom (px)
    #! $min: 0
    #! $max: 400
  - gapLeft: 6
    $name: Gap at the left (px)
    #! $min: 0
    #! $max: 400
  - gapRight: 6
    $name: Gap at the right (px)
    #! $min: 0
    #! $max: 400
  - gapInner: 6
    $name: Gap between windows (px)
    #! $min: 0
    #! $max: 400
  - reserveTaskbar: true
    $name: Keep clear of the taskbar
    $description: The suite's taskbar's space is left free even while it hides.
  - restoreMaximized: true
    $name: Tile maximised windows too
  - floatApps: ''
    $name: Programs left floating
    $description: Program file names, separated by commas, like SnippingTool.exe; * and ? match any text.
  - floatTitles: ''
    $name: Window titles left floating
    $description: Separated by commas; * and ? match any text, like *Properties.
  - floatClasses: ''
    $name: Window classes left floating
    $description: Separated by commas; * and ? match any text.
  - minWidth: 0
    $name: Leave out windows narrower than (px)
    $description: Little windows of their own stay floating; 0 for no limit.
    #! $min: 0
    #! $max: 4000
  - minHeight: 0
    $name: Leave out windows shorter than (px)
    #! $min: 0
    #! $max: 4000
  - tooSmall: float
    $name: When a window won't shrink to its tile
    $description: >-
      Some programs have a minimum size. Force ignores it, as dwm does with
      resizehints = 0 (its content may be cut off); Overlap lets it keep its
      size over its neighbours, as resizehints = 1 does; Float leaves it out
      and tiles the rest without it.
    $options:
    - force: Force it to fit
    - overlap: Let it overlap its neighbours
    - float: Leave it floating
  - newWindow: end
    $name: Where a window that opens goes
    $description: >-
      A new window joins the screen's order at the end, which is the last tile
      in the layout. First makes it the main window instead; the other two put
      it either side of the window you were in, as qtile's
      new_client_position does.
    $options:
    - end: At the end
    - first: First (the main window, in layouts that have one)
    - afterFocused: After the window you were in
    - beforeFocused: Before the window you were in
  - sliceWindow: ''
    $name: 'Slice: the window with the strip'
    $description: >-
      For the Slice layout. The program, class or title of the window that
      keeps the strip along the edge, with * and ? matching any text
      (Spotify.exe, or Discord). Empty: whichever window is first on the
      screen. Several, separated by commas: the first one found takes it.
  - sliceEdge: left
    $name: 'Slice: which edge'
    $options:
    - left: Left
    - right: Right
    - top: Top
    - bottom: Bottom
  - sliceSize: 0
    $name: 'Slice: how much of the screen the strip takes (%)'
    $description: 0 uses the main window's share above.
  - sliceLayout: verticalStack
    $name: 'Slice: the rest of the windows'
    $description: The layout for everything that isn't in the strip.
    $options:
    - columns: Columns
    - rows: Rows
    - verticalStack: Vertical stack (main on the left)
    - rightMainVerticalStack: Vertical stack (main on the right)
    - horizontalStack: Horizontal stack (main on top)
    - ultrawideVerticalStack: Ultrawide vertical stack (main in the middle)
    - threeColumn: Three columns (main in the middle, a column each side)
    - grid: Grid
    - bsp: BSP (each window halves the space left)
    - spiral: Spiral (each window takes a side of what's left, winding inwards)
    - monocle: Monocle (every window fills what's left)
    - layers: Layers of three
  - split: ['']
    $name: 'Split screen: the regions'
    $description: >-
      For the Split screen layout, one region a line, as qtile's ScreenSplit
      does it: a name, the region's share of the screen as four numbers
      (left top width height, in percent), its own layout, then the programs,
      classes or titles whose windows go in it (* and ? match any text).

      region = sidebar, 0 0 25 100, verticalStack, Spotify.exe, Discord.exe

      region = main, 25 0 75 100, grid

      The "region =" in front is optional. A region that names no windows
      takes everything left over; with every region naming something, the last
      one does. The gap between tiles is the gap between regions too, so the
      shares can add up to 100 without the tiles touching.
  - script: ''
    $name: 'Your own layout: which file'
    $description: >-
      For the Your own layout choice. The name of a .luau file in
      %LOCALAPPDATA%\Tourne\Layouts (without the .luau), and after a comma
      anything you want the layout itself to read: spread, 3. The folder is
      made with three commented examples in it the first time Tourne looks for
      it, and saving a file is all it takes: the next window that moves uses
      the new one. A layout is handed the screen, the gap, the ratio and the
      windows (with each one's program, class and title) and answers with where
      each window goes; it can't reach anything else, and a layout that stops
      or answers with the wrong thing lays out in columns and says so in the
      log. Tourne Desktop only: in the Windhawk suite this lays out in columns.
  - keepResized: true
    $name: Leave a window you resize where you put it
    $description: >-
      Resizing a tiled window by hand takes it out of the tiling, so it isn't
      snapped back, and its place is remembered for that program: its next
      window opens there. The tiling key and Space put it back in the tiling
      and forget the place.
  - rememberAll: false
    $name: Remember where every window closed
    $description: >-
      Every kind of window (programs' windows, dialogs, tool windows) opens
      where the last of its kind was when it closed, unless the tiling places
      it. A dialog is told apart by its title too. Off until you turn it on,
      so windows open where their programs put them.
  - focusBorder: none
    $name: Border on the focused window
    $description: >-
      The window in front draws its thin border in a colour, and gives it
      back when the focus moves on (Windows 11).
    $options:
    - none: None
    - tiled: Tiled windows
    - all: Every window
  - focusColors: accent
    $name: Focus border colour
    $options:
    - accent: Windows' accent colour
    - custom: My own colour
    #! $showIf: {focusBorder: [tiled, all]}
  - focusColor: '66EA97'
    $name: Focus border's own colour
    #! $format: colorRgb
    #! $showIf: {focusColors: custom}
  - tileElevated: false
    $name: Tile programs running as administrator
    $description: >-
      Through a small helper the suite starts inside each such program, so
      the tiling itself never needs administrator rights.
  $name: Window tiling
- borders:
  - enabled: false
    $name: Draw borders around windows
    $description: >-
      A border of your own drawn around each window, as Tacky Borders does:
      gradients, a spin, a fade between the window in front and the rest,
      and a glow and shadow. Off until you turn it on. Turn Tacky Borders
      itself off first, or both draw. While this is on, the suite follows
      windows as they move, open and close; while it's off, it doesn't
      watch them at all. The settings start as yours from Tacky Borders.
  - width: 1
    $name: Width
    $description: In pixels at 100% scale; it grows with the screen's scale.
  - offset: -1
    $name: Offset
    $description: >-
      How far out from the window's edge the border sits, in pixels.
      Negative values draw it over the window's edge (-1 covers Windows' own
      thin border).
  - radius: square
    $name: Corners
    $options:
    - auto: As the window's own corners
    - square: Square
    - round: Round
    - roundSmall: Slightly round
    - custom: My own radius
  - radiusCustom: 8
    $name: Corner radius
    $description: In pixels at 100% scale.
    #! $showIf: {radius: custom}
  - zOrder: above
    $name: Draw it
    $options:
    - above: Above the window
    - below: Below the window
  - followNative: true
    $name: Only where Windows draws its own border
    $description: >-
      No border on maximised windows, or on windows that have no border of
      their own.
  - initDelay: 200
    $name: Delay for a new window (ms)
    $description: Lets the window's opening animation finish first; 0 for none.
  - unminimizeDelay: 150
    $name: Delay for a restored window (ms)
    $description: Lets the window's restore animation finish first; 0 for none.
  - activeColor: '#64aa89 #121c21 0,1 1,0'
    $name: Colour of the window in front
    $description: >-
      One colour, or several for a gradient: a hex colour (RGB or with
      alpha, like 64aa89cc), the word accent for Windows' accent colour, or
      accentInactive for the other windows' one (AccentColorInactive, as the
      Windows 11 Accent Window Border mod reads it).
      After the colours, the gradient's direction as an angle (like 45deg)
      or as start and end points from 0,0 (top left) to 1,1 (bottom right).
  - inactiveColor: '#121c21 #64aa89 0,1 1,0'
    $name: Colour of the other windows
    $description: Written the same way. Empty for no border on them.
  - activeSpin: spiral
    $name: Spin on the window in front
    $description: The gradient turns around the window.
    $options:
    - none: None
    - spiral: Clockwise
    - reverseSpiral: Anticlockwise
  - inactiveSpin: none
    $name: Spin on the other windows
    $options:
    - none: None
    - spiral: Clockwise
    - reverseSpiral: Anticlockwise
  - spinDuration: 2500
    $name: Time for one turn (ms)
  - spinRepeat: once
    $name: How long it spins
    $description: >-
      Once: one turn each time a window comes to the front or leaves it, then
      it stands still and nothing is drawn. Always: it keeps turning, and the
      borders that spin are redrawn every frame while they show.
    $options:
    - once: One turn, then still
    - always: Always
  - fade: true
    $name: Fade between the two colours
    $description: >-
      As the focus moves, the window it leaves fades to the other windows'
      colour and the window it reaches fades to the front's.
  - fadeDuration: 1500
    $name: Fade time (ms)
  - fps: 60
    $name: Frames per second
    $description: >-
      For the spin and the fade. Nothing is drawn while nothing moves; while
      a border spins or fades, it is redrawn this often.
  - followMoves: front
    $name: Follow window moves
    $description: >-
      Window in front: borders follow the window in front as it moves, a
      window dragged anywhere, and the windows the tiling moves; a window
      behind that moves itself is caught up when the windows' order or the
      front changes. Every window: every move on the PC is followed, which
      costs a little work all the time, even while you only move the mouse.
    $options:
    - front: Window in front (lighter)
    - all: Every window
  - glow: all
    $name: Glow
    $description: The border's colours, blurred outward.
    $options:
    - none: None
    - active: On the window in front
    - all: On every window
  - glowBlur: 1
    $name: Glow blur
    $description: In pixels at 100% scale.
  - glowSpread: 0
    $name: Glow spread
    $description: How much wider than the border the glow starts, in pixels.
  - glowOpacity: 100
    $name: Glow opacity (%)
  - shadow: all
    $name: Shadow
    $options:
    - none: None
    - active: On the window in front
    - all: On every window
  - shadowBlur: 1
    $name: Shadow blur
    $description: In pixels at 100% scale.
  - shadowOpacity: 100
    $name: Shadow opacity (%)
  - shadowX: 0
    $name: Shadow offset across
  - shadowY: 0
    $name: Shadow offset down
  - skipApps: 'tacky-borders.exe, zebar.exe'
    $name: Programs without borders
    $description: File names, separated by commas; * and ? match any text.
  - skipClasses: 'Windows.UI.Core.CoreWindow, XamlExplorerHostIslandWindow'
    $name: Window classes without borders
  - skipTitles: 'komorebi-bar, keyviz, Picture-in-Picture'
    $name: Window titles without borders
  $name: Window borders
- binds:
  - enabled: false
    $name: Key binds
    $description: >-
      Hyprland's binds and commands, on Windows. Off until turned on, and
      nothing is bound until you write a line. A bound key is taken by
      Tourne while it runs: the program in front never hears it, so any
      shortcut it had on the same keys silently stops working. Pick
      combinations with Win, or two modifiers (like Ctrl+Alt), that nothing
      else you use takes; with just Ctrl, Ctrl+S could stop saving without
      any sign. Win+L still locks the PC, as nothing can take that. Binds are
      left alone in fullscreen programs unless written bindl.
  - lines: ['']
    $name: Binds
    $description: >-
      One a line, written as in Hyprland's config: bind = MODS, key, command,
      argument. MODS are WIN (or SUPER), CTRL, ALT and SHIFT, with spaces
      between, or none. Keys are letters, digits, F1 to F24 and Hyprland's
      names (space, return, escape, tab, left, right, up, down, minus, equal,
      bracketleft, XF86AudioRaiseVolume...), or code:NN for a Windows key
      code. binde repeats while held, bindr runs on release, bindl works in
      fullscreen programs too, bindn lets the program hear the key as well,
      bindd takes what it does in words after the key (bindd = WIN, Q, Close
      the window, killactive), which the Keybind Guide shows.
      bindm = WIN, mouse:272, movewindow drags the window under the pointer
      with the left button (273 right, 274 middle; resizewindow resizes from
      the nearest corner), floating it; it needs a modifier.
      bindk runs on a double press (the key twice within the double press
      time below) and bindo when held (for the hold time below, or bindo800
      for its own); a key can have a bind, a bindk and a bindo at once, and
      then a single press waits out the double press time before it runs.
      "$mod = WIN" lines name a value for later lines; "submap = resize" puts
      the lines after it in a mode of their own (submap = reset ends it), and
      "submap, resize" as a bind's command enters it; catchall as the key
      takes every other key in a mode. Commands: run (several steps in one,
      separated by semicolons, with wait between them: run, workspace 2;
      wait 200; exec notepad. A wait takes 200, 200ms or 0.2s; write \; for
      a semicolon of your own. The same line is a menu item's command when
      written tourne:do workspace 2; wait 200; exec notepad, and tournectl
      run does it from a script, while tournectl check prints what it would
      do without doing any of it), exec, killactive,
      forcekillactive, closewindow, killwindow, togglefloating, setfloating,
      settiled, tileall (every window on the screen tiled, an even split),
      monocle (one window given the screen, again puts it back; tileothers
      after it tiles the rest behind it), fullscreen, minimize, pin, movefocus, movewindow, swapwindow,
      resizeactive, moveactive, resizewindowpixel, movewindowpixel,
      centerwindow, cyclenext, focuswindow, focusmonitor, focuscurrentorlast,
      workspace, movetoworkspace, movetoworkspacesilent, renameworkspace
      (Windows' desktops, made when a number has none yet, up to 20; movetoworkspace, special or special:name puts a
      window away in a scratchpad, and togglespecialworkspace [name] brings
      it back floating over any desktop, or puts it away again), layoutmsg (cycle, tile, auto, layouts, apply N,
      mfact N), splitratio, submap, sendshortcut, alterzorder,
      bringactivetotop, movecursor, movecursortocorner, dpms, lockscreen,
      flyout NAME (a system icons flyout: mixer, notifications, network, power,
      bluetooth, devices, brightness, vpn, security; again closes it),
      explorerbar (Explorer's own taskbar in place of Tourne's bar, and
      back: explorer, tourne, or nothing to swap; shell mode only),
      overview (every window on every screen, as the switcher's Mission
      Control; again closes it), pickcolor (the colour under the pointer
      copied as #rrggbb),
      and groups (windows stacked in one tile with a bar of tabs):
      togglegroup, changegroupactive (f, b or N), moveintogroup (l, r, u,
      d), moveoutofgroup.
      Windows are named class:, title:, exe: or pid:, with * and ? matching
      any text.
      "idle = 300, dpms, off" runs a command once you've been away that
      long (as hypridle; any command, like lockscreen; not while a
      fullscreen program is in front). Window rules go here too, as in Hyprland: windowrule = float,
      class:Calculator (rules: float, tile, center, size W H, move X Y,
      workspace N [silent], monitor N, pin, maximize, minimize, opacity A,
      noborder, keep [S]; each for a window as it opens). exec takes rules for the
      window it opens: exec, [float; size 50% 50%] notepad.
  - file: ''
    $name: Binds file
    $description: >-
      A file of more lines written the same way (like a hyprland.conf's
      binds), read with the settings, for example
      %APPDATA%\Tourne\binds.conf. Empty for none.
  - doubleMs: 300
    $name: Double press time (ms)
    $description: >-
      How quickly the second press must follow the first for a bindk. A
      single press on a key that also has one waits this long before it runs.
  - holdMs: 500
    $name: Hold time (ms)
    $description: How long a key is held before its bindo runs (bindo800 sets its own).
  - control: false
    $name: Let scripts control Tourne (tournectl)
    $description: >-
      Programs you run (tournectl, a bar, a script) can run these commands
      and hear events (activewindow, openwindow, closewindow, workspace,
      submap), as Hyprland's hyprctl and event socket do. Only programs of
      yours on this PC can reach it. Nothing runs while nothing calls. The
      Tourne Menu's own items ask through this as well, so an item whose
      command starts with tourne:do needs it on.
  $name: Key binds and control
- customHeaders:
  - - program: ''
      $name: Program
      $description: Its file name, like notepad.exe, or full path.
    - name: ''
      $name: Name shown
    - icon: ''
      $name: Icon
      $description: An .ico file, or an .exe or .dll with ,index (like shell32.dll,4).
  $name: Custom names and icons
*/
// ==/WindhawkModSettings==

#include <windows.h>
#include <dwmapi.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <shobjidl.h>
#include <propsys.h>
#include <uxtheme.h>
#include <vssym32.h>
#include <windowsx.h>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <cwctype>
#include <mutex>
#include <thread>

// Programs Tourne Desktop leaves alone (its exclusions.cpp: shell.exclude, games, Windhawk's exclusions):
// no border or tiling for their windows; no hooks while one of shell.excludeKeys is in front. None in the suite, where
// Windhawk's own exclusions keep it out of them.
#ifdef TD_PREFIX
inline bool LeftAlone(HWND h) { return td::ProgramExcluded(h); }
inline bool LeftAloneInFront() { return td::ExcludedInFront(); }
#else
inline bool LeftAlone(HWND) { return false; }
inline bool LeftAloneInFront() { return false; }
#endif

// @inline palettes.cpp
// @inline theme-palette.cpp
// @inline live-peek.cpp
// @inline motion.cpp

////////////////////////////////////////////////////////////////////////////////
// Colours

struct Look {
    uint32_t ground, border, highlight, highlightBorder, text, selectedText, iconBack, count, countText;
    int iconBackOpacity, countOpacity;  // %
};

// Tourne'Style: the preset, dark and light.
constexpr Look kTourneDark = {0x121C21, 0xA95D5D, 0x64AA89, 0xA95D5D, 0xFFFFFF,
                              0xFFFFFF, 0x64AA89, 0x64AA89, 0xA95D5D, 55, 25};
constexpr Look kTourneLight = {0xF3F3F3, 0x8A8A8A, 0xDADADA, 0x8A8A8A, 0x000000,
                               0x000000, 0xFFFFFF, 0xEAEAEA, 0x000000, 55, 85};

Look LookFromPalette(Palette const& p) {
    return {p.ground, p.edge, p.hover, p.accent, p.text, p.text, p.raised, p.accent, p.text, 55, 25};
}

constexpr COLORREF Rgb(uint32_t rgb) {
    return RGB((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
}

uint32_t FromColorRef(COLORREF c) {
    return (GetRValue(c) << 16) | (GetGValue(c) << 8) | GetBValue(c);
}

// Windows' own look, for what nothing else sets.
constexpr Look kNativeDark = {0x202020, 0x3A3A3A, 0x3A3A3A, 0x5A5A5A, 0xFFFFFF,
                              0xFFFFFF, 0x3A3A3A, 0x333333, 0xFFFFFF, 55, 85};
constexpr Look kNativeLight = {0xF3F3F3, 0xCCCCCC, 0xDADADA, 0xA0A0A0, 0x000000,
                               0x000000, 0xFFFFFF, 0xEAEAEA, 0x000000, 55, 85};

// A theme file's menu colours: its ground, border, hot item and text.
bool LookFromTheme(std::wstring const& path, Look* look, bool dark = false) {
    ThemeFileData file;
    if (!LoadThemeFile(path, &file)) return false;
    HWND host = CreateWindowExW(0, L"STATIC", L"", 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, nullptr, nullptr);
    HTHEME menu = host ? OpenThemeFileMenu(&file, host, dark) : nullptr;
    if (menu) {
        Look l = *look;
        l.ground = PartPixel(menu, MENU_POPUPBACKGROUND, 0, 4, 4, l.ground);
        l.border = PartPixel(menu, MENU_POPUPBORDERS, 0, 0, 0, l.border);
        l.highlight = PartPixel(menu, MENU_POPUPITEM, MPI_HOT, 4, 4, l.highlight);
        COLORREF text;
        if (SUCCEEDED(GetThemeColor(menu, MENU_POPUPITEM, MPI_NORMAL, TMT_TEXTCOLOR, &text))) l.text = FromColorRef(text);
        l.selectedText = l.text;
        if (SUCCEEDED(GetThemeColor(menu, MENU_POPUPITEM, MPI_HOT, TMT_TEXTCOLOR, &text)))
            l.selectedText = FromColorRef(text);
        l.highlightBorder = l.border;
        l.iconBack = l.highlight;
        l.count = l.highlight;
        l.countText = l.text;
        *look = l;
        CloseThemeData(menu);
    }
    if (host) DestroyWindow(host);
    CloseThemeFile(&file);
    return menu != nullptr;
}

////////////////////////////////////////////////////////////////////////////////
// Settings

enum class Highlight { Border, Fill, FillAndBorder };
enum class Spot { TopLeft, TopCenter, TopRight, Center, BottomLeft, BottomCenter, BottomRight };
enum class Align { Left, Center, Right };
enum class Where { Center, Top, Bottom, Left, Right, TopLeft, TopRight, BottomLeft, BottomRight };
enum class ShowOn { Cursor, Active, Primary };
enum class Minimized { InPlace, Last, Hidden };

// @inline program-icons.cpp

struct CustomHeader {
    std::wstring program;  // lower case: a file name or a full path
    std::wstring name;
    std::wstring icon;
};

struct Settings {
    std::vector<ProgramIcon> programIcons;  // the taskbar's Button icons used everywhere
    Look look = kTourneDark;
    bool acrylic = false;
    Highlight highlight = Highlight::FillAndBorder;
    int opacity = 100;
    int radius = 0;
    bool roundItems = false, roundCount = false, roundIconBack = false;
    bool thumbnails = false;
    bool thumbnailsOnTop = false;
    Align thumbAlign = Align::Center;
    bool stretch = true, closeButton = true, hoverBorder = true, thumbShadow = true;
    bool showTitle = true, showIcon = false;
    int iconSize = 16;
    bool centered = true;
    bool iconAbove = true;
    bool grid = false;
    int rowHeight = 230, rowWidth = 0, maxWidth = 25, maxHeight = 35;
    bool shrinkToFit = true, overflowIndicator = true;
    Where where = Where::Center;
    int offsetX = 0, offsetY = 0;  // px, see the Position settings
    ShowOn showOn = ShowOn::Cursor;
    bool badges = false;
    Spot badgeSpot = Spot::Center;
    bool badgeTitleAbove = true, badgeBack = false, badgeShadow = true;
    int badgePadding = 4, badgeX = 0, badgeY = 0;
    std::wstring font = L"Scientifica";
    int fontSize = 8;
    bool bold = true, italic = false, fontForCount = true;
    bool groupApps = true, appTitles = true, restoreAll = true, showCount = true, countShadow = true, closeAll = false;
    int showDelay = 0;
    bool peek = false;
    int peekDelay = 500;
    bool winTab = false;           // Win+Tab opens Mission Control instead of Task view
    bool backtick = false;         // Alt+` is taken at all
    bool backtickSameApp = false;
    bool wheelOpens = false, wheelPages = false, wheelModifierCtrl = false, reverseWheel = false;
    bool allDesktops = true, thisMonitorOnly = false;
    Minimized minimized = Minimized::InPlace;
    std::wstring excludeTitles, excludeApps;
    std::vector<CustomHeader> headers;
    int view = 0;  // 0 the list, 1 Flip 3D, 2 Cover Flow, 3 Grid, 4 Mission Control, 5 Ring, 6 Shelf (switcher-views.cpp), 7 Canvas
    int overviewView = 7;  // the overview's (Win+Tab, the overview command): 4 Mission Control or 7 Canvas
    bool viewLive = true;
    int viewDim = 55, viewBlur = 6;
};

std::wstring TextSetting(PCWSTR name) {
    PCWSTR value = Wh_GetStringSetting(name);
    std::wstring s = value ? value : L"";
    Wh_FreeStringSetting(value);
    return s;
}

// Other parts' settings, read by their full names (through a variable, so the
// build doesn't file them under this part).
constexpr PCWSTR kIconsScheme = L"systemIcons.scheme";

std::wstring Expand(std::wstring const& s) {
    WCHAR out[MAX_PATH * 2];
    DWORD n = ExpandEnvironmentStringsW(s.c_str(), out, ARRAYSIZE(out));
    return n && n <= ARRAYSIZE(out) ? out : s;
}

std::wstring Lower(std::wstring s) {
    CharLowerW(s.data());
    return s;
}

std::wstring SabStyles() { return Expand(L"%ProgramFiles%\\StartAllBack\\Styles\\"); }
std::wstring TourneStyles() { return Expand(L"%LOCALAPPDATA%\\Tourne\\Styles\\"); }

bool AppsUseLightTheme() {
    DWORD light = 0, size = sizeof(light);
    RegGetValueW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
                 L"AppsUseLightTheme", RRF_RT_REG_DWORD, nullptr, &light, &size);
    return light != 0;
}

void HexColor(std::wstring hex, uint32_t* out);

// Simple Window Switcher's own colours, where it has them set (its settings
// stay in Windhawk's registry even once it's turned off). Its "default"
// colours leave the look passed in.
bool LookFromSimpleWindowSwitcher(bool light, Look* look) {
    HKEY key;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Windhawk\\Engine\\Mods\\simple-window-switcher\\Settings", 0,
                      KEY_READ, &key) != ERROR_SUCCESS)
        return false;
    std::wstring mode = light ? L"Style.LightMode." : L"Style.DarkMode.";
    auto text = [&](PCWSTR name) {
        WCHAR value[64] = L"";
        DWORD size = sizeof(value);
        RegGetValueW(key, nullptr, (mode + name).c_str(), RRF_RT_REG_SZ, nullptr, value, &size);
        return std::wstring(value);
    };
    auto number = [&](PCWSTR name, int* out) {
        DWORD value = 0, size = sizeof(value);
        if (RegGetValueW(key, nullptr, (mode + name).c_str(), RRF_RT_REG_DWORD, nullptr, &value, &size) == ERROR_SUCCESS)
            *out = std::clamp((int)value, 0, 100);
    };
    // A colour it has set by hand ("custom"); the accent colour too.
    auto color = [&](PCWSTR modeName, PCWSTR customName, uint32_t* out) {
        std::wstring how = text(modeName);
        if (how == L"custom") {
            HexColor(text(customName), out);
        } else if (how == L"accent") {
            DWORD accent = 0;
            BOOL opaque = FALSE;
            if (SUCCEEDED(DwmGetColorizationColor(&accent, &opaque))) *out = accent & 0xFFFFFF;
        }
    };
    Look l = *look;
    color(L"bgColorMode", L"customBgColor", &l.ground);
    color(L"borderColorMode", L"customBorderColor", &l.border);
    color(L"highlightFillColorMode", L"customHighlightFillColor", &l.highlight);
    color(L"iconBgColorMode", L"customIconBgColor", &l.iconBack);
    color(L"indicatorBgColorMode", L"customIndicatorBgColor", &l.count);
    color(L"indicatorTextColorMode", L"customIndicatorTextColor", &l.countText);
    number(L"iconBgOpacity", &l.iconBackOpacity);
    number(L"indicatorBgOpacity", &l.countOpacity);
    // It draws the selection's border in its border colour, and its text in
    // white (dark mode) or black.
    l.highlightBorder = l.border;
    l.text = l.selectedText = light ? 0x000000 : 0xFFFFFF;
    RegCloseKey(key);
    *look = l;
    return true;
}

// Your visual style's colours (the .msstyles Windows is using), else
// Windows' own look.
Look VisualStyleLook(bool light) {
    Look look = light ? kNativeLight : kNativeDark;
    std::wstring file = CurrentThemeFile();
    if (!file.empty()) LookFromTheme(file, &look, !light);
    return look;
}

Palette SuitePalette() {
    PCWSTR scheme = Wh_GetStringSetting(kIconsScheme);
    Palette p = SuiteSchemePalette(scheme ? scheme : L"");
    Wh_FreeStringSetting(scheme);
    return p;
}

int IntSetting(PCWSTR name, int low, int high) {
    return std::clamp(Wh_GetIntSetting(name), low, high);
}

// "#RRGGBB" (or without the #) over a colour; anything else leaves it.
void HexColor(std::wstring hex, uint32_t* out) {
    hex.erase(0, hex.find_first_not_of(L" \t#"));
    hex.erase(hex.find_last_not_of(L" \t") + 1);
    wchar_t* end = nullptr;
    unsigned long value = wcstoul(hex.c_str(), &end, 16);
    if (hex.size() == 6 && end && !*end) *out = value & 0xFFFFFF;
}

Settings LoadSettings() {
    Settings s;
    std::wstring mode = TextSetting(L"theme.colorMode");
    bool light = mode == L"light" || (mode != L"dark" && AppsUseLightTheme());
    std::wstring style = TextSetting(L"theme.style");
    if (style.empty() || style == L"auto") {
        // What you already have: Simple Window Switcher's colours, else your
        // visual style's, else Windows' own.
        s.look = light ? kNativeLight : kNativeDark;
        if (!LookFromSimpleWindowSwitcher(light, &s.look)) s.look = VisualStyleLook(light);
    } else if (style == L"visualStyle") {
        s.look = VisualStyleLook(light);
    } else if (style == L"scheme") {
        s.look = LookFromPalette(SuitePalette());
    } else if (style == L"matchTaskbar") {
        if (!LookFromTheme(TaskbarThemeFile(), &s.look, !light)) s.look = LookFromPalette(SuitePalette());
    } else if (!style.empty() && style != L"tourneStyle") {
        LookFromTheme(ThemeStyleFile(style), &s.look, !light);
    } else {
        s.look = light ? kTourneLight : kTourneDark;
    }
    // The colours set by hand, for the mode in use.
    std::wstring set = light ? L"lightColors." : L"colors.";
    auto hex = [&](PCWSTR name, uint32_t* out) { HexColor(TextSetting((set + name).c_str()), out); };
    auto percent = [&](PCWSTR name, int* out) { *out = std::clamp(Wh_GetIntSetting((set + name).c_str()), 0, 100); };
    // Read by variable, so each is read with this part's prefix put in by
    // hand: the build only prefixes literals.
    set = L"switcher." + set;
    hex(L"background", &s.look.ground);
    hex(L"border", &s.look.border);
    hex(L"selection", &s.look.highlight);
    hex(L"selectionBorder", &s.look.highlightBorder);
    hex(L"text", &s.look.text);
    hex(L"selectedText", &s.look.selectedText);
    hex(L"iconBackground", &s.look.iconBack);
    hex(L"count", &s.look.count);
    hex(L"countText", &s.look.countText);
    percent(L"iconBackgroundOpacity", &s.look.iconBackOpacity);
    percent(L"countOpacity", &s.look.countOpacity);

    s.acrylic = TextSetting(L"theme.backdrop") == L"acrylic";
    std::wstring highlight = TextSetting(L"theme.highlight");
    s.highlight = highlight == L"border" ? Highlight::Border : highlight == L"fill" ? Highlight::Fill
                                                                                 : Highlight::FillAndBorder;
    s.opacity = IntSetting(L"theme.opacity", 0, 100);

    s.radius = IntSetting(L"corners.radius", 0, 48);
    s.roundItems = Wh_GetIntSetting(L"corners.roundItems") != 0;
    s.roundCount = Wh_GetIntSetting(L"corners.roundCount") != 0;
    s.roundIconBack = Wh_GetIntSetting(L"corners.roundIconBackground") != 0;

    s.thumbnails = Wh_GetIntSetting(L"thumbnails.show") != 0;
    s.thumbnailsOnTop = TextSetting(L"thumbnails.position") == L"top";
    std::wstring align = TextSetting(L"thumbnails.alignment");
    s.thumbAlign = align == L"left" ? Align::Left : align == L"right" ? Align::Right : Align::Center;
    s.stretch = Wh_GetIntSetting(L"thumbnails.stretch") != 0;
    s.closeButton = Wh_GetIntSetting(L"thumbnails.closeButton") != 0;
    s.hoverBorder = Wh_GetIntSetting(L"thumbnails.hoverBorder") != 0;
    s.thumbShadow = Wh_GetIntSetting(L"thumbnails.shadow") != 0;

    s.showTitle = Wh_GetIntSetting(L"header.showTitle") != 0;
    s.showIcon = Wh_GetIntSetting(L"header.showIcon") != 0;
    std::wstring iconSize = TextSetting(L"header.iconSize");
    s.iconSize = iconSize == L"large" ? 32 : iconSize == L"medium" ? 24 : 16;
    s.centered = Wh_GetIntSetting(L"header.centered") != 0;
    s.iconAbove = TextSetting(L"header.orientation") != L"horizontal";

    std::wstring view = TextSetting(L"view.style");
    s.view = view == L"flip3d" ? 1 : view == L"coverflow" ? 2 : view == L"grid" ? 3 : view == L"mission" ? 4 : view == L"ring" ? 5
           : view == L"shelf"  ? 6
           : view == L"canvas" ? 7
                               : 0;
    s.overviewView = TextSetting(L"view.overview") == L"mission" ? 4 : 7;
    s.viewLive = Wh_GetIntSetting(L"view.live") != 0;
    s.viewDim = IntSetting(L"view.dim", 0, 95);
    s.viewBlur = IntSetting(L"view.blur", 0, 10);

    s.grid = TextSetting(L"layout.orientation") == L"horizontal";
    int rowHeight = Wh_GetIntSetting(L"layout.rowHeight");
    s.rowHeight = rowHeight > 0 ? std::clamp(rowHeight, 60, 800) : 230;
    s.rowWidth = IntSetting(L"layout.rowWidth", 0, 1600);
    int maxWidth = Wh_GetIntSetting(L"layout.maxWidth"), maxHeight = Wh_GetIntSetting(L"layout.maxHeight");
    s.maxWidth = maxWidth > 0 ? std::clamp(maxWidth, 10, 100) : 25;
    s.maxHeight = maxHeight > 0 ? std::clamp(maxHeight, 10, 100) : 35;
    s.shrinkToFit = Wh_GetIntSetting(L"layout.shrinkToFit") != 0;
    s.overflowIndicator = Wh_GetIntSetting(L"layout.overflowIndicator") != 0;

    std::wstring place = TextSetting(L"position.place");
    s.where = place == L"top"     ? Where::Top
            : place == L"bottom" ? Where::Bottom
            : place == L"left"   ? Where::Left
            : place == L"right"  ? Where::Right
            : place == L"topLeft" ? Where::TopLeft
            : place == L"topRight" ? Where::TopRight
            : place == L"bottomLeft" ? Where::BottomLeft
            : place == L"bottomRight" ? Where::BottomRight
                                 : Where::Center;
    s.offsetX = IntSetting(L"position.offsetX", -4000, 4000);
    s.offsetY = IntSetting(L"position.offsetY", -4000, 4000);
    // The old single distance, for the edges it applied to, until these are set.
    int margin = IntSetting(L"position.margin", 0, 2000);
    if (!s.offsetX && (s.where == Where::Left || s.where == Where::Right)) s.offsetX = margin;
    if (!s.offsetY && (s.where == Where::Top || s.where == Where::Bottom)) s.offsetY = margin;
    std::wstring monitor = TextSetting(L"position.monitor");
    s.showOn = monitor == L"active" ? ShowOn::Active : monitor == L"primary" ? ShowOn::Primary : ShowOn::Cursor;

    s.badges = Wh_GetIntSetting(L"badgeLayout.use") != 0;
    std::wstring spot = TextSetting(L"badgeLayout.iconPosition");
    s.badgeSpot = spot == L"topLeft"      ? Spot::TopLeft
                : spot == L"topCenter"    ? Spot::TopCenter
                : spot == L"topRight"     ? Spot::TopRight
                : spot == L"bottomLeft"   ? Spot::BottomLeft
                : spot == L"bottomCenter" ? Spot::BottomCenter
                : spot == L"bottomRight"  ? Spot::BottomRight
                                          : Spot::Center;
    s.badgeTitleAbove = TextSetting(L"badgeLayout.titlePosition") != L"below";
    s.badgeBack = Wh_GetIntSetting(L"badgeLayout.iconBackground") != 0;
    s.badgeShadow = Wh_GetIntSetting(L"badgeLayout.iconShadow") != 0;
    s.badgePadding = IntSetting(L"badgeLayout.padding", 0, 64);
    s.badgeX = IntSetting(L"badgeLayout.offsetX", -400, 400);
    s.badgeY = IntSetting(L"badgeLayout.offsetY", -400, 400);

    std::wstring font = TextSetting(L"font.family");
    if (!font.empty()) s.font = font;
    int size = Wh_GetIntSetting(L"font.size");
    s.fontSize = size > 0 ? std::clamp(size, 5, 48) : 8;
    std::wstring fontStyle = TextSetting(L"font.style");
    s.bold = fontStyle == L"bold" || fontStyle == L"boldItalic";
    s.italic = fontStyle == L"italic" || fontStyle == L"boldItalic";
    s.fontForCount = Wh_GetIntSetting(L"font.forCount") != 0;

    s.groupApps = Wh_GetIntSetting(L"grouping.apps") != 0;
    s.appTitles = TextSetting(L"grouping.titles") != L"windowTitle";
    s.restoreAll = Wh_GetIntSetting(L"grouping.restoreAll") != 0;
    s.showCount = Wh_GetIntSetting(L"grouping.showCount") != 0;
    s.countShadow = Wh_GetIntSetting(L"grouping.countShadow") != 0;
    s.closeAll = TextSetting(L"grouping.closeBehavior") == L"closeAll";

    s.showDelay = IntSetting(L"keys.showDelay", 0, 2000);
    s.peek = Wh_GetIntSetting(L"keys.peek") != 0;
    s.peekDelay = IntSetting(L"keys.peekDelay", 0, 3000);
    s.winTab = Wh_GetIntSetting(L"keys.winTab") != 0;
    std::wstring backtick = TextSetting(L"keys.backtick");
    s.backtick = backtick == L"backward" || backtick == L"sameApp";
    s.backtickSameApp = backtick == L"sameApp";
    s.wheelOpens = TextSetting(L"keys.wheelOpens") == L"withAlt";
    s.wheelPages = TextSetting(L"keys.wheelAction") == L"page";
    s.wheelModifierCtrl = TextSetting(L"keys.wheelModifier") == L"ctrl";
    s.reverseWheel = Wh_GetIntSetting(L"keys.reverseWheel") != 0;

    s.allDesktops = TextSetting(L"windows.desktops") != L"current";
    s.thisMonitorOnly = Wh_GetIntSetting(L"windows.thisMonitorOnly") != 0;
    std::wstring minimized = TextSetting(L"windows.minimized");
    s.minimized = minimized == L"last" ? Minimized::Last : minimized == L"hidden" ? Minimized::Hidden : Minimized::InPlace;
    s.excludeTitles = TextSetting(L"windows.excludeTitles");
    s.excludeApps = TextSetting(L"windows.excludeApps");

    for (int i = 0; i < 64; i++) {
        PCWSTR program = Wh_GetStringSetting(L"customHeaders[%d].program", i);
        CustomHeader h{program ? Lower(program) : L""};
        Wh_FreeStringSetting(program);
        if (h.program.empty()) {
            if (i > 0) break;
            continue;
        }
        PCWSTR name = Wh_GetStringSetting(L"customHeaders[%d].name", i);
        PCWSTR icon = Wh_GetStringSetting(L"customHeaders[%d].icon", i);
        h.name = name ? name : L"";
        h.icon = icon ? Expand(icon) : L"";
        Wh_FreeStringSetting(name);
        Wh_FreeStringSetting(icon);
        s.headers.push_back(std::move(h));
    }
    s.programIcons = EverywhereIcons();
    return s;
}

// The Style dropdown (Windhawk 2.0's $dynamicSelect): the preset, the suite's
// colours, and every theme installed.
void PublishChoices() {
    auto option = [](std::wstring const& value, std::wstring const& label) {
        std::wstring key = L"::wh_select_option::switcher.theme.style::" + value;
        Wh_SetStringValue(key.c_str(), label.c_str());
    };
    option(L"auto", L"Automatic (what you already use)");
    option(L"visualStyle", L"Your visual style (the .msstyles in use)");
    option(L"tourneStyle", L"Tourne'Style");
    option(L"scheme", L"Colour scheme (same as System Icons)");
    option(L"matchTaskbar", L"Same as the taskbar's visual style");
    auto each = [&](std::wstring const& folder, bool byPath, PCWSTR suffix) {
        WIN32_FIND_DATAW fd;
        HANDLE find = FindFirstFileW((folder + L"*.msstyles").c_str(), &fd);
        if (find == INVALID_HANDLE_VALUE) return;
        do {
            std::wstring name = fd.cFileName;
            option(byPath ? folder + name : name, name.substr(0, name.rfind(L'.')) + suffix);
        } while (FindNextFileW(find, &fd));
        FindClose(find);
    };
    each(SabStyles(), false, L"");
    each(TourneStyles(), true, L" (Tourne)");
}

////////////////////////////////////////////////////////////////////////////////
// The windows listed: what Alt+Tab lists, in the order they were last used.

struct Entry {
    std::vector<HWND> windows;  // most recently used first
    std::wstring key;           // what groups them: the app id or program file
    std::wstring label;
    HICON icon = nullptr;
    bool ownsIcon = false;      // a custom icon, loaded here
    HTHUMBNAIL thumbnail = nullptr;
    RECT rc{};                  // the item
    RECT title{};               // its title
    RECT iconRc{};              // its icon (or badge)
    RECT thumb{};               // the thumbnail's area
    RECT shown{};               // where the thumbnail is drawn in it
    RECT count{};               // the window count
    RECT close{};               // the close button
};

Settings g_settings;
std::atomic<HWND> g_wnd;
HANDLE g_thread;
// The hooks live on a thread of their own that does nothing else: Windows
// gives a low-level hook only a moment to answer and otherwise lets the key
// through (to its own Alt+Tab), so they can't wait on the switcher's drawing.
HANDLE g_hookThread;
DWORD g_hookThreadId;
HHOOK g_keyHook, g_mouseHook;  // hook thread only
// What the hooks read, kept by the switcher's thread.
std::atomic<bool> g_wheelOpens, g_wheelModifierCtrl, g_reverseWheel, g_backtick;
// Tab held down: its repeats add up here and go as one step per message, so
// they can't queue up behind the drawing.
std::atomic<int> g_tabSteps;
std::atomic<bool> g_tabStepPosted;
std::vector<Entry> g_entries;
std::vector<Entry> g_outer;   // while drilled into an app: the full list
int g_sel = 0;
int g_first = 0;              // the first item shown, when they don't all fit
int g_hover = -1;
bool g_hoverClose = false;
std::atomic<bool> g_open;     // Alt+Tab is under way (shown or not yet)
bool g_shown = false;
std::atomic<bool> g_sticky;
bool g_allMonitors = false;   // Win+Alt+Tab
HFONT g_font, g_countFont;
int g_dpi = 96;
IVirtualDesktopManager* g_desktops;

constexpr UINT WM_APP_EXIT = WM_APP + 1;
constexpr UINT WM_APP_SETTINGS = WM_APP + 2;
constexpr UINT WM_APP_STEP = WM_APP + 3;     // wParam: 1 forward, -1 back; lParam: see Begin
constexpr UINT WM_APP_COMMIT = WM_APP + 4;
constexpr UINT WM_APP_CANCEL = WM_APP + 5;   // Esc
constexpr UINT WM_APP_CLOSEONE = WM_APP + 6;
constexpr UINT WM_APP_DRILL = WM_APP + 7;
constexpr UINT WM_APP_MOVE = WM_APP + 8;     // arrows: wParam dx, lParam dy
constexpr UINT WM_APP_WHEEL = WM_APP + 9;    // wParam: the wheel delta; lParam: the other action's key held
constexpr UINT WM_APP_TABSTEPS = WM_APP + 10;  // Tab again while switching: steps in g_tabSteps
constexpr UINT WM_APP_WATCHWHEEL = WM_APP + 11;  // to the hook thread; wParam: on
constexpr UINT WM_APP_LEFTALONE = WM_APP + 12;   // to the hook thread; wParam: a program left alone is in front
constexpr UINT_PTR kTimerShow = 1;
constexpr UINT_PTR kTimerPeek = 2;
// STEP's lParam: LOWORD what pressed it, HIWORD flags.
constexpr WORD kFromTab = 0, kFromBacktick = 1, kFromWheel = 2, kFromWinTab = 3;
constexpr WORD kFlagSticky = 1, kFlagAllMonitors = 2;
constexpr WCHAR kWindowClass[] = L"TourneSwitcher_" WH_MOD_ID;
constexpr WCHAR kOverlayClass[] = L"TourneSwitcherOverlay_" WH_MOD_ID;
// An unassigned key, pressed while switching: Alt let go then isn't a lone
// Alt tap (which opens the active app's menu bar), and a process that sent
// the last input may set the foreground window.
constexpr WORD kDummyKey = 0xE8;
// The view for this opening only, or -1 for the one set (Win+Tab: Mission Control, 4).
int g_viewFor = -1;

int Px(int v) { return MulDiv(v, g_dpi, 96); }

std::wstring ProgramOf(HWND h) {
    DWORD pid = 0;
    GetWindowThreadProcessId(h, &pid);
    WCHAR path[MAX_PATH] = L"";
    auto pathOf = [&](DWORD process) -> std::wstring {
        HANDLE p = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, process);
        if (!p) return {};
        DWORD size = ARRAYSIZE(path);
        std::wstring result = QueryFullProcessImageNameW(p, 0, path, &size) ? path : L"";
        CloseHandle(p);
        return result;
    };
    std::wstring program = pathOf(pid);
    // A packaged app's frame belongs to ApplicationFrameHost; the app is its
    // core window's process.
    if (program.size() >= 24 && _wcsicmp(program.c_str() + program.size() - 24, L"ApplicationFrameHost.exe") == 0) {
        if (HWND core = FindWindowExW(h, nullptr, L"Windows.UI.Core.CoreWindow", nullptr)) {
            DWORD corePid = 0;
            GetWindowThreadProcessId(core, &corePid);
            if (std::wstring inner = pathOf(corePid); !inner.empty()) program = inner;
        }
    }
    return program;
}

std::wstring AppIdOf(HWND h) {
    constexpr PROPERTYKEY kAppUserModelId = {{0x9F4C2855, 0x9F79, 0x4B39, {0xA8, 0xD0, 0xE1, 0xD4, 0x2D, 0xE1, 0xD5, 0xF3}}, 5};
    IPropertyStore* store = nullptr;
    std::wstring id;
    if (SUCCEEDED(SHGetPropertyStoreForWindow(h, __uuidof(IPropertyStore), (void**)&store)) && store) {
        PROPVARIANT pv;
        PropVariantInit(&pv);
        if (SUCCEEDED(store->GetValue(kAppUserModelId, &pv)) && pv.vt == VT_LPWSTR && pv.pwszVal) id = pv.pwszVal;
        PropVariantClear(&pv);
        store->Release();
    }
    return id;
}

// A program's name as it describes itself, else its file name.
std::wstring ProgramName(std::wstring const& program) {
    DWORD handle = 0;
    DWORD size = GetFileVersionInfoSizeW(program.c_str(), &handle);
    if (size) {
        std::vector<BYTE> data(size);
        if (GetFileVersionInfoW(program.c_str(), 0, size, data.data())) {
            struct Translation { WORD language, codePage; }* translations = nullptr;
            UINT bytes = 0;
            if (VerQueryValueW(data.data(), L"\\VarFileInfo\\Translation", (void**)&translations, &bytes) && bytes) {
                WCHAR query[64];
                swprintf_s(query, L"\\StringFileInfo\\%04x%04x\\FileDescription", translations[0].language,
                           translations[0].codePage);
                WCHAR* text = nullptr;
                UINT chars = 0;
                if (VerQueryValueW(data.data(), query, (void**)&text, &chars) && chars > 1 && text[0]) return text;
            }
        }
    }
    std::wstring file = program.substr(program.find_last_of(L'\\') + 1);
    if (file.size() > 4 && _wcsicmp(file.c_str() + file.size() - 4, L".exe") == 0) file.resize(file.size() - 4);
    return file;
}

std::wstring Title(HWND h) {
    WCHAR text[256] = L"";
    GetWindowTextW(h, text, ARRAYSIZE(text));
    return text;
}

HICON WindowIcon(HWND h, bool big) {
    DWORD_PTR result = 0;
    WPARAM order[3] = {(WPARAM)ICON_SMALL2, (WPARAM)ICON_SMALL, (WPARAM)ICON_BIG};
    if (big) std::swap(order[0], order[2]);
    for (WPARAM kind : order) {
        if (SendMessageTimeoutW(h, WM_GETICON, kind, 0, SMTO_ABORTIFHUNG | SMTO_BLOCK, 50, &result) && result)
            return (HICON)result;
    }
    if (HICON icon = (HICON)GetClassLongPtrW(h, big ? GCLP_HICON : GCLP_HICONSM)) return icon;
    return (HICON)GetClassLongPtrW(h, big ? GCLP_HICONSM : GCLP_HICON);
}

// "file.ico" or "file.dll,4": an icon at a size.
HICON LoadIconSpec(std::wstring spec, int size) {
    if (IsTourneDesign(spec)) return part_system_icons::DesignIconAt(spec, size);  // a design (tourne:)
    int index = 0;
    size_t comma = spec.rfind(L',');
    if (comma != std::wstring::npos) {
        index = _wtoi(spec.c_str() + comma + 1);
        spec.resize(comma);
    }
    HICON icon = nullptr;
    UINT id = 0;
    if (PrivateExtractIconsW(spec.c_str(), index, size, size, &icon, &id, 1, LR_DEFAULTCOLOR) == 1) return icon;
    return nullptr;
}

// "a, b*, c" against a name: * and ? match any text.
bool MatchesList(std::wstring const& list, std::wstring const& name) {
    if (list.empty() || name.empty()) return false;
    size_t start = 0;
    while (start <= list.size()) {
        size_t comma = list.find(L',', start);
        std::wstring pattern = list.substr(start, comma == std::wstring::npos ? std::wstring::npos : comma - start);
        pattern.erase(0, pattern.find_first_not_of(L" \t"));
        pattern.erase(pattern.find_last_not_of(L" \t") + 1);
        if (!pattern.empty() && PathMatchSpecW(name.c_str(), pattern.c_str())) return true;
        if (comma == std::wstring::npos) break;
        start = comma + 1;
    }
    return false;
}

HMONITOR TargetMonitor() {
    POINT pt;
    switch (g_settings.showOn) {
        case ShowOn::Active:
            if (HWND fg = GetForegroundWindow()) return MonitorFromWindow(fg, MONITOR_DEFAULTTOPRIMARY);
            break;
        case ShowOn::Primary:
            return MonitorFromPoint({0, 0}, MONITOR_DEFAULTTOPRIMARY);
        case ShowOn::Cursor:
            GetCursorPos(&pt);
            return MonitorFromPoint(pt, MONITOR_DEFAULTTOPRIMARY);
    }
    return MonitorFromPoint({0, 0}, MONITOR_DEFAULTTOPRIMARY);
}

// The windows Alt+Tab lists: shown, not owned by another, not a tool window,
// titled, and on this desktop (or any, as set).
bool IsSwitchWindow(HWND h) {
    if (!IsWindowVisible(h)) return false;
    LONG_PTR ex = GetWindowLongPtrW(h, GWL_EXSTYLE);
    bool app = ex & WS_EX_APPWINDOW;
    if (!app && ((ex & WS_EX_TOOLWINDOW) || (ex & WS_EX_NOACTIVATE))) return false;
    if (HWND owner = GetWindow(h, GW_OWNER); owner && !app && IsWindowVisible(owner)) return false;
    if (GetWindowTextLengthW(h) == 0) return false;
    WCHAR cls[64];
    if (GetClassNameW(h, cls, ARRAYSIZE(cls)) && (!wcsncmp(cls, L"Tourne", 6) || !wcscmp(cls, L"Progman") ||
                                                  !wcscmp(cls, L"Shell_TrayWnd") || !wcscmp(cls, L"WorkerW")))
        return false;
    DWORD cloaked = 0;
    DwmGetWindowAttribute(h, DWMWA_CLOAKED, &cloaked, sizeof(cloaked));
    if (cloaked) {
        // Cloaked because it's on another desktop: listed when every
        // desktop's windows are. Otherwise (a suspended app's frame, say) not.
        GUID desktop{};
        BOOL here = TRUE;
        if (!g_settings.allDesktops || !g_desktops || FAILED(g_desktops->GetWindowDesktopId(h, &desktop)) ||
            desktop == GUID{} || FAILED(g_desktops->IsWindowOnCurrentVirtualDesktop(h, &here)) || here)
            return false;
    } else if (!g_settings.allDesktops && g_desktops) {
        BOOL here = TRUE;
        if (SUCCEEDED(g_desktops->IsWindowOnCurrentVirtualDesktop(h, &here)) && !here) return false;
    }
    return true;
}

void ClearEntries(std::vector<Entry>& entries) {
    for (auto& e : entries) {
        if (e.thumbnail) DwmUnregisterThumbnail(e.thumbnail);
        if (e.ownsIcon && e.icon) DestroyIcon(e.icon);
    }
    entries.clear();
}

CustomHeader const* HeaderFor(std::wstring const& program) {
    std::wstring path = Lower(program), file = path.substr(path.find_last_of(L'\\') + 1);
    for (auto const& h : g_settings.headers)
        if (h.program == path || h.program == file) return &h;
    return nullptr;
}

// A program's icon from Tourne'Icons' Taskbar buttons and the taskbar's Button
// icons marked "Use everywhere", matched by its app id, path or file name; the
// caller destroys it.
HICON EverywhereIcon(HWND h, std::wstring const& program) {
    std::vector<ProgramIcon> rows = WithTourneIcons(g_settings.programIcons);
    if (rows.empty()) return nullptr;
    std::wstring path = Lower(program), id = Lower(AppIdOf(h));
    for (auto const& row : rows)
        if (ProgramIconMatches(row.program, id, path)) return LoadIconSpec(row.icon, Px(std::max(32, g_settings.iconSize)));
    return nullptr;
}

// Every window, grouped by app (or not), most recently used first. With
// onlyApp, just the windows of the app with that key.
std::vector<Entry> BuildEntries(std::wstring const* onlyApp) {
    std::vector<HWND> windows;
    EnumWindows(
        [](HWND h, LPARAM param) __stdcall -> BOOL {
            if (IsSwitchWindow(h)) ((std::vector<HWND>*)param)->push_back(h);
            return TRUE;
        },
        (LPARAM)&windows);
    if (g_settings.thisMonitorOnly && !g_allMonitors) {
        HMONITOR monitor = TargetMonitor();
        std::erase_if(windows, [&](HWND h) { return MonitorFromWindow(h, MONITOR_DEFAULTTONEAREST) != monitor; });
    }
    if (g_settings.minimized == Minimized::Hidden) std::erase_if(windows, [](HWND h) { return IsIconic(h); });
    if (g_settings.minimized == Minimized::Last)
        std::stable_partition(windows.begin(), windows.end(), [](HWND h) { return !IsIconic(h); });

    std::vector<Entry> entries;
    bool group = g_settings.groupApps && !onlyApp;
    bool bigIcon = g_settings.iconSize > 16 || g_settings.badges;
    for (HWND h : windows) {
        std::wstring program = ProgramOf(h);
        std::wstring file = program.substr(program.find_last_of(L'\\') + 1);
        std::wstring title = Title(h);
        if (MatchesList(g_settings.excludeTitles, title) || MatchesList(g_settings.excludeApps, file)) continue;
        std::wstring key = AppIdOf(h);
        if (key.empty()) key = program;
        key = Lower(key);
        if (onlyApp && key != *onlyApp) continue;
        if (group) {
            auto it = std::find_if(entries.begin(), entries.end(), [&](Entry const& e) { return e.key == key; });
            if (it != entries.end()) {
                it->windows.push_back(h);
                continue;
            }
        }
        Entry e;
        e.windows = {h};
        e.key = key;
        CustomHeader const* custom = HeaderFor(program);
        if (group && g_settings.appTitles) e.label = custom && !custom->name.empty() ? custom->name : ProgramName(program);
        else e.label = title;
        if (e.label.empty()) e.label = title;
        if (custom && !custom->icon.empty()) {
            e.icon = LoadIconSpec(custom->icon, Px(std::max(32, g_settings.iconSize)));
            e.ownsIcon = e.icon != nullptr;
        }
        if (!e.icon) {
            e.icon = EverywhereIcon(h, program);
            e.ownsIcon = e.icon != nullptr;
        }
        if (!e.icon) e.icon = WindowIcon(h, bigIcon);
        entries.push_back(std::move(e));
    }
    // The active window's entry first, whatever is stacked above it (another
    // screen's always-on-top window), so the first Tab goes to the one before.
    HWND fg = GetForegroundWindow();
    if (fg) fg = GetAncestor(fg, GA_ROOTOWNER);
    for (size_t i = 0; i < entries.size(); i++) {
        auto& list = entries[i].windows;
        auto it = std::find(list.begin(), list.end(), fg);
        if (it == list.end()) continue;
        std::rotate(list.begin(), it, it + 1);
        std::rotate(entries.begin(), entries.begin() + i, entries.begin() + i + 1);
        break;
    }
    return entries;
}

////////////////////////////////////////////////////////////////////////////////
// Layout

struct Metrics {
    int pad, gap, itemW, itemH, headerH, textH, icon, columns, rows;
    SIZE window;
};
Metrics g_m{};

void MakeFonts() {
    if (g_font) DeleteObject(g_font);
    if (g_countFont) DeleteObject(g_countFont);
    LOGFONTW lf{};
    lf.lfHeight = -MulDiv(g_settings.fontSize, g_dpi, 72);
    lf.lfWeight = g_settings.bold ? FW_BOLD : FW_NORMAL;
    lf.lfItalic = g_settings.italic;
    lf.lfQuality = NONANTIALIASED_QUALITY;
    wcsncpy_s(lf.lfFaceName, g_settings.font.c_str(), _TRUNCATE);
    g_font = CreateFontIndirectW(&lf);
    if (g_settings.fontForCount) {
        g_countFont = CreateFontIndirectW(&lf);
    } else {
        NONCLIENTMETRICSW ncm{sizeof(ncm)};
        SystemParametersInfoForDpi(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0, g_dpi);
        g_countFont = CreateFontIndirectW(&ncm.lfMessageFont);
    }
}

bool ShowsCount(Entry const& e) {
    return g_settings.showCount && g_outer.empty() && e.windows.size() > 1;
}

int CountWidth(HDC dc, Entry const& e) {
    if (!ShowsCount(e)) return 0;
    std::wstring count = std::to_wstring(e.windows.size());
    HGDIOBJ old = SelectObject(dc, g_countFont);
    SIZE size{};
    GetTextExtentPoint32W(dc, count.c_str(), (int)count.size(), &size);
    SelectObject(dc, old);
    return size.cx + Px(10);
}

// Does the item show its icon in its header (not as a badge)?
bool HeaderIcon() {
    return g_settings.showIcon && !(g_settings.badges && g_settings.thumbnails);
}

// Where each part of an item goes, for an item of the given size at (x, y).
void PlaceItem(HDC dc, Entry& e, int x, int y, int w, int h) {
    auto const& s = g_settings;
    int pad = g_m.pad;
    e.rc = {x, y, x + w, y + h};
    bool thumbs = s.thumbnails;
    // The header: title (and icon), above or below the thumbnail.
    int headerH = g_m.headerH;
    bool headerOnTop = thumbs ? (s.badges ? s.badgeTitleAbove : !s.thumbnailsOnTop) : true;
    RECT header = headerOnTop ? RECT{x + pad, y + pad / 2, x + w - pad, y + pad / 2 + headerH}
                              : RECT{x + pad, y + h - pad / 2 - headerH, x + w - pad, y + h - pad / 2};
    if (!thumbs) header = {x + pad, y, x + w - pad, y + h};
    e.thumb = thumbs ? (headerOnTop ? RECT{x + pad, header.bottom, x + w - pad, y + h - pad}
                                    : RECT{x + pad, y + pad, x + w - pad, header.top})
                     : RECT{};

    // The window count and the close button, at the header's end.
    int count = CountWidth(dc, e);
    int closeSize = Px(14);
    e.close = {};
    if (s.closeButton) {
        if (thumbs) {
            e.close = {e.thumb.right - closeSize - Px(2), e.thumb.top + Px(2), e.thumb.right - Px(2),
                       e.thumb.top + Px(2) + closeSize};
        } else {
            e.close = {header.right - closeSize, (header.top + header.bottom - closeSize) / 2, header.right,
                       (header.top + header.bottom + closeSize) / 2};
        }
    }
    RECT title = header;
    e.count = {};
    if (count) {
        int ch = std::min<int>(header.bottom - header.top - Px(6), g_m.textH + Px(4));
        e.count = {title.right - count, (header.top + header.bottom - ch) / 2, title.right, (header.top + header.bottom + ch) / 2};
        if (!thumbs && s.closeButton) OffsetRect(&e.count, -(closeSize + Px(4)), 0);
        title.right = e.count.left - Px(4);
        if (s.centered) title.left += title.right == header.right ? 0 : header.right - e.count.left + Px(4);
    } else if (!thumbs && s.closeButton) {
        title.right -= closeSize + Px(4);
        if (s.centered) title.left += closeSize + Px(4);
    }

    // The icon: beside or above the title, or as a badge on the thumbnail.
    int icon = g_m.icon;
    e.iconRc = {};
    if (s.showIcon && s.badges && thumbs) {
        int size = Px(std::max(32, s.iconSize)), bp = Px(s.badgePadding);
        int bw = size + 2 * bp;
        int left = e.thumb.left, right = e.thumb.right - bw, top = e.thumb.top, bottom = e.thumb.bottom - bw;
        int cx = (e.thumb.left + e.thumb.right - bw) / 2, cy = (e.thumb.top + e.thumb.bottom - bw) / 2;
        POINT at = s.badgeSpot == Spot::TopLeft      ? POINT{left, top}
                 : s.badgeSpot == Spot::TopCenter    ? POINT{cx, top}
                 : s.badgeSpot == Spot::TopRight     ? POINT{right, top}
                 : s.badgeSpot == Spot::BottomLeft   ? POINT{left, bottom}
                 : s.badgeSpot == Spot::BottomCenter ? POINT{cx, bottom}
                 : s.badgeSpot == Spot::BottomRight  ? POINT{right, bottom}
                                                     : POINT{cx, cy};
        at.x += Px(s.badgeX);
        at.y += Px(s.badgeY);
        e.iconRc = {at.x, at.y, at.x + bw, at.y + bw};
    } else if (HeaderIcon()) {
        if (s.iconAbove && !thumbs) {
            int cx = s.centered ? (header.left + header.right - icon) / 2 : header.left;
            int top = header.top + Px(4);
            e.iconRc = {cx, top, cx + icon, top + icon};
            title.top = e.iconRc.bottom;
        } else {
            e.iconRc = {title.left, (title.top + title.bottom - icon) / 2, title.left + icon, (title.top + title.bottom + icon) / 2};
            title.left += icon + Px(6);
            if (s.centered) title.right -= icon + Px(6);  // the title stays in the middle
        }
    }
    e.title = s.showTitle ? title : RECT{};
}

// Sizes the items and the switcher for the screen, keeps the selection in
// view, and places everything.
void Arrange(HWND wnd) {
    auto const& s = g_settings;
    HMONITOR monitor = TargetMonitor();
    UINT dpiX = 96, dpiY = 96;
    using GetDpiForMonitor_t = HRESULT(WINAPI*)(HMONITOR, int, UINT*, UINT*);
    static auto getDpi = (GetDpiForMonitor_t)GetProcAddress(LoadLibraryW(L"shcore.dll"), "GetDpiForMonitor");
    if (getDpi && SUCCEEDED(getDpi(monitor, 0, &dpiX, &dpiY))) g_dpi = dpiX;
    MakeFonts();
    MONITORINFO mi{sizeof(mi)};
    GetMonitorInfoW(monitor, &mi);
    RECT work = mi.rcWork;
    int maxW = (work.right - work.left) * s.maxWidth / 100;
    int maxH = (work.bottom - work.top) * s.maxHeight / 100;

    HDC dc = CreateCompatibleDC(nullptr);
    HGDIOBJ old = SelectObject(dc, g_font);
    TEXTMETRICW tm{};
    GetTextMetricsW(dc, &tm);
    g_m.pad = Px(6);
    g_m.gap = Px(6);
    g_m.textH = tm.tmHeight;
    g_m.icon = Px(s.iconSize);
    bool iconInHeader = HeaderIcon();
    bool stacked = iconInHeader && s.iconAbove && !s.thumbnails;
    int textRow = s.showTitle ? g_m.textH : 0;
    g_m.headerH = stacked ? g_m.icon + Px(4) + textRow + Px(8)
                          : std::max(textRow, iconInHeader ? g_m.icon : 0) + Px(10);

    // The widest item's content, for items that fit their content.
    int widest = 0;
    for (auto const& e : g_entries) {
        SIZE size{};
        if (s.showTitle) GetTextExtentPoint32W(dc, e.label.c_str(), (int)e.label.size(), &size);
        int count = CountWidth(dc, e);
        int extra = count ? count + Px(4) : 0;
        if (!s.thumbnails && s.closeButton) extra += Px(18);
        int w = size.cx + (s.centered ? 2 * extra : extra);
        if (iconInHeader && !stacked) w += (s.centered ? 2 : 1) * (g_m.icon + Px(6));
        if (stacked) w = std::max<int>(w, g_m.icon);
        widest = std::max(widest, w);
    }

    int n = std::max(1, (int)g_entries.size());
    int avail = maxW - 2 * g_m.pad;
    auto sizeItems = [&](double scale) {
        if (s.thumbnails) {
            g_m.itemH = std::max(Px(60), (int)(Px(s.rowHeight) * scale));
            int thumbH = g_m.itemH - g_m.headerH - g_m.pad * 2;
            g_m.itemW = s.rowWidth ? (int)(Px(s.rowWidth) * scale) : std::max(Px(80), thumbH * 16 / 10 + 2 * g_m.pad);
        } else {
            g_m.itemH = g_m.headerH + (stacked ? 0 : Px(4));
            g_m.itemW = s.rowWidth ? Px(s.rowWidth) : std::max(s.grid ? Px(120) : Px(160), widest + 2 * g_m.pad + Px(8));
            g_m.itemW = std::min(g_m.itemW, std::max(Px(80), avail));
        }
        g_m.columns = s.grid ? std::clamp((avail + g_m.gap) / (g_m.itemW + g_m.gap), 1, n) : 1;
        int allRows = (n + g_m.columns - 1) / g_m.columns;
        int fitRows = std::max(1, (maxH - g_m.pad) / (g_m.itemH + g_m.gap));
        g_m.rows = std::min(allRows, fitRows);
        return allRows <= fitRows;
    };
    // Shrinking only helps items with thumbnails (text keeps its size), and
    // stops while a thumbnail can still be made out: past that it scrolls.
    bool fits = sizeItems(1.0);
    if (!fits && s.shrinkToFit && s.thumbnails) {
        int smallest = g_m.headerH + 2 * g_m.pad + Px(70);
        for (double scale = 0.95; !fits && Px(s.rowHeight) * scale >= smallest; scale -= 0.05) fits = sizeItems(scale);
        if (!fits) sizeItems(std::max((double)smallest / Px(s.rowHeight), 0.1));
    }

    int allRows = (n + g_m.columns - 1) / g_m.columns;
    int selRow = g_sel / g_m.columns, firstRow = g_first / g_m.columns;
    if (selRow < firstRow) firstRow = selRow;
    if (selRow >= firstRow + g_m.rows) firstRow = selRow - g_m.rows + 1;
    firstRow = std::clamp(firstRow, 0, std::max(0, allRows - g_m.rows));
    g_first = firstRow * g_m.columns;

    int shownColumns = std::min(g_m.columns, n);
    int width = shownColumns * (g_m.itemW + g_m.gap) - g_m.gap + 2 * g_m.pad;
    int height = g_m.rows * (g_m.itemH + g_m.gap) - g_m.gap + 2 * g_m.pad;
    if (s.overflowIndicator && allRows > g_m.rows) height += Px(8);
    int top = g_m.pad + (s.overflowIndicator && allRows > g_m.rows ? Px(4) : 0);
    for (int i = 0; i < (int)g_entries.size(); i++) {
        auto& e = g_entries[i];
        int slot = i - g_first;
        if (slot < 0 || slot >= g_m.rows * g_m.columns) {
            e.rc = e.thumb = e.title = e.iconRc = e.count = e.close = {};
            continue;
        }
        PlaceItem(dc, e, g_m.pad + (slot % g_m.columns) * (g_m.itemW + g_m.gap),
                  top + (slot / g_m.columns) * (g_m.itemH + g_m.gap), g_m.itemW, g_m.itemH);
    }
    SelectObject(dc, old);
    DeleteDC(dc);

    // Each axis: against an edge (the distance from it) or centred (the
    // distance as a shift), then kept on the screen.
    bool atLeft = s.where == Where::Left || s.where == Where::TopLeft || s.where == Where::BottomLeft;
    bool atRight = s.where == Where::Right || s.where == Where::TopRight || s.where == Where::BottomRight;
    bool atTop = s.where == Where::Top || s.where == Where::TopLeft || s.where == Where::TopRight;
    bool atBottom = s.where == Where::Bottom || s.where == Where::BottomLeft || s.where == Where::BottomRight;
    int dx = Px(s.offsetX), dy = Px(s.offsetY);
    int x = atLeft ? work.left + dx : atRight ? work.right - width - dx : (work.left + work.right - width) / 2 + dx;
    int y = atTop ? work.top + dy : atBottom ? work.bottom - height - dy : (work.top + work.bottom - height) / 2 + dy;
    // A negative distance may take it past the work area's edge, over the
    // taskbar, but never off the screen.
    RECT screen = mi.rcMonitor;
    x = std::clamp(x, (int)screen.left, std::max((int)screen.left, (int)screen.right - width));
    y = std::clamp(y, (int)screen.top, std::max((int)screen.top, (int)screen.bottom - height));
    g_m.window = {width, height};
    SetWindowPos(wnd, HWND_TOPMOST, x, y, width, height, SWP_NOACTIVATE | SWP_NOREDRAW);
}

// Live thumbnails for the items in view, fitted to their areas.
void UpdateThumbnails(HWND wnd) {
    auto const& s = g_settings;
    for (auto& e : g_entries) {
        e.shown = {};
        if (IsRectEmpty(&e.thumb) || !g_shown) {
            if (e.thumbnail) DwmUnregisterThumbnail(e.thumbnail);
            e.thumbnail = nullptr;
            continue;
        }
        if (!e.thumbnail && FAILED(DwmRegisterThumbnail(wnd, e.windows[0], &e.thumbnail))) {
            e.thumbnail = nullptr;
            continue;
        }
        SIZE source{};
        DwmQueryThumbnailSourceSize(e.thumbnail, &source);
        int areaW = e.thumb.right - e.thumb.left, areaH = e.thumb.bottom - e.thumb.top;
        if (source.cx <= 0 || source.cy <= 0 || areaW <= 0 || areaH <= 0) continue;
        double scale = std::min((double)areaW / source.cx, (double)areaH / source.cy);
        if (!s.stretch) scale = std::min(scale, 1.0);
        int w = (int)(source.cx * scale), h = (int)(source.cy * scale);
        int x = s.thumbAlign == Align::Left    ? e.thumb.left
              : s.thumbAlign == Align::Right   ? e.thumb.right - w
                                               : e.thumb.left + (areaW - w) / 2;
        e.shown = {x, e.thumb.top + (areaH - h) / 2, 0, 0};
        e.shown.right = e.shown.left + w;
        e.shown.bottom = e.shown.top + h;
        DWM_THUMBNAIL_PROPERTIES props{};
        props.dwFlags = DWM_TNP_RECTDESTINATION | DWM_TNP_VISIBLE | DWM_TNP_OPACITY | DWM_TNP_SOURCECLIENTAREAONLY;
        props.rcDestination = e.shown;
        props.fVisible = TRUE;
        props.opacity = 255;
        props.fSourceClientAreaOnly = FALSE;
        DwmUpdateThumbnailProperties(e.thumbnail, &props);
    }
}

////////////////////////////////////////////////////////////////////////////////
// Drawing: into a 32-bit bitmap given to Windows with its transparency (a
// layered window), so the background can be see-through with the rest solid.

void Fill(HDC dc, RECT const& r, uint32_t rgb, bool round = false) {
    if (round) {
        SetDCBrushColor(dc, Rgb(rgb));
        SetDCPenColor(dc, Rgb(rgb));
        HGDIOBJ brush = SelectObject(dc, GetStockObject(DC_BRUSH)), pen = SelectObject(dc, GetStockObject(DC_PEN));
        int d = Px(8);
        RoundRect(dc, r.left, r.top, r.right, r.bottom, d, d);
        SelectObject(dc, brush);
        SelectObject(dc, pen);
        return;
    }
    SetDCBrushColor(dc, Rgb(rgb));
    FillRect(dc, &r, (HBRUSH)GetStockObject(DC_BRUSH));
}

void Frame(HDC dc, RECT const& r, uint32_t rgb, bool round = false) {
    if (round) {
        SetDCPenColor(dc, Rgb(rgb));
        HGDIOBJ brush = SelectObject(dc, GetStockObject(NULL_BRUSH)), pen = SelectObject(dc, GetStockObject(DC_PEN));
        int d = Px(8);
        RoundRect(dc, r.left, r.top, r.right, r.bottom, d, d);
        SelectObject(dc, brush);
        SelectObject(dc, pen);
        return;
    }
    SetDCBrushColor(dc, Rgb(rgb));
    FrameRect(dc, &r, (HBRUSH)GetStockObject(DC_BRUSH));
}

// A soft-edged shadow: the shape, darker, a little down and to the right.
void Shadow(HDC dc, RECT r, bool round) {
    uint32_t ground = g_settings.look.ground;
    OffsetRect(&r, Px(2), Px(2));
    Fill(dc, r, MixRgb(ground, 0x000000, 0.45), round);
}

void DrawChevron(HDC dc, int cx, int y, bool up, uint32_t rgb) {
    int h = Px(4);
    for (int i = 0; i < h; i++) {
        int half = up ? i : h - 1 - i;
        RECT line{cx - half - 1, y + i, cx + half + 1, y + i + 1};
        Fill(dc, line, rgb);
    }
}

void DrawCross(HDC dc, RECT const& r, uint32_t rgb) {
    int k = Px(8), x = (r.left + r.right - k) / 2, y = (r.top + r.bottom - k) / 2;
    for (int j = 0; j < k; j++) {
        RECT a{x + j, y + j, x + j + Px(1), y + j + Px(1)};
        RECT b{x + k - Px(1) - j, y + j, x + k - j, y + j + Px(1)};
        Fill(dc, a, rgb);
        Fill(dc, b, rgb);
    }
}

void DrawItem(HDC dc, Entry const& e, bool selected, bool hovered) {
    auto const& s = g_settings;
    Look const& look = s.look;
    bool fill = s.highlight != Highlight::Border, border = s.highlight != Highlight::Fill;
    if (selected && fill) Fill(dc, e.rc, look.highlight, s.roundItems);
    if (selected && border) Frame(dc, e.rc, look.highlightBorder, s.roundItems);

    // The thumbnail's frame: its shadow, and a border under the pointer.
    if (!IsRectEmpty(&e.shown)) {
        if (s.thumbShadow) Shadow(dc, e.shown, false);
        if (hovered && s.hoverBorder) {
            RECT around = e.shown;
            InflateRect(&around, Px(1), Px(1));
            Frame(dc, around, look.highlightBorder);
        }
    } else if (!IsRectEmpty(&e.thumb) && e.icon) {
        // No thumbnail (DWM has nothing for this window): its icon, large.
        int size = Px(48);
        DrawIconEx(dc, (e.thumb.left + e.thumb.right - size) / 2, (e.thumb.top + e.thumb.bottom - size) / 2, e.icon,
                   size, size, 0, nullptr, DI_NORMAL);
    }

    // The icon in the header. (A badge goes on the overlay: see DrawOverlay.)
    if (!IsRectEmpty(&e.iconRc) && e.icon && !(s.badges && s.thumbnails)) {
        RECT const& box = e.iconRc;
        DrawIconEx(dc, box.left, box.top, e.icon, box.right - box.left, box.bottom - box.top, 0, nullptr, DI_NORMAL);
    }

    if (!IsRectEmpty(&e.title)) {
        SelectObject(dc, g_font);
        SetTextColor(dc, Rgb(selected ? look.selectedText : look.text));
        UINT align = s.centered ? DT_CENTER : DT_LEFT;
        RECT title = e.title;
        DrawTextW(dc, e.label.c_str(), -1, &title, DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX | align);
    }

    if (!IsRectEmpty(&e.count)) {
        if (s.countShadow) Shadow(dc, e.count, s.roundCount);
        Fill(dc, e.count, MixRgb(look.ground, look.count, look.countOpacity / 100.0), s.roundCount);
        SelectObject(dc, g_countFont);
        SetTextColor(dc, Rgb(look.countText));
        std::wstring count = std::to_wstring(e.windows.size());
        RECT box = e.count;
        DrawTextW(dc, count.c_str(), -1, &box, DT_SINGLELINE | DT_VCENTER | DT_CENTER | DT_NOPREFIX);
    }

    if (hovered && !IsRectEmpty(&e.close) && !s.thumbnails) {
        if (g_hoverClose) Fill(dc, e.close, look.border, s.roundItems);
        DrawCross(dc, e.close, g_hoverClose ? look.ground : look.text);
    }
}

// What goes over a thumbnail, which DWM draws above the switcher's own
// pixels: the badge and the close button, on a see-through window on top.
void DrawOverlay(HDC dc, Entry const& e, bool hovered) {
    auto const& s = g_settings;
    Look const& look = s.look;
    if (s.badges && !IsRectEmpty(&e.iconRc) && e.icon) {
        RECT box = e.iconRc;
        if (s.badgeShadow) Shadow(dc, box, s.roundIconBack);
        if (s.badgeBack) Fill(dc, box, MixRgb(look.ground, look.iconBack, look.iconBackOpacity / 100.0), s.roundIconBack);
        InflateRect(&box, -Px(s.badgePadding), -Px(s.badgePadding));
        DrawIconEx(dc, box.left, box.top, e.icon, box.right - box.left, box.bottom - box.top, 0, nullptr, DI_NORMAL);
    }
    if (hovered && !IsRectEmpty(&e.close)) {
        Fill(dc, e.close, g_hoverClose ? look.border : look.ground, s.roundItems);
        DrawCross(dc, e.close, g_hoverClose ? look.ground : look.text);
    }
}

HWND g_overlay;

void RenderOverlay(HWND wnd) {
    if (!g_overlay) return;
    if (!g_shown || !g_settings.thumbnails) {
        ShowWindow(g_overlay, SW_HIDE);
        return;
    }
    int w = g_m.window.cx, h = g_m.window.cy;
    HDC screen = GetDC(nullptr);
    HDC dc = CreateCompatibleDC(screen);
    BITMAPINFO bi{};
    bi.bmiHeader = {sizeof(BITMAPINFOHEADER), w, -h, 1, 32, BI_RGB};
    void* bits = nullptr;
    HBITMAP bitmap = CreateDIBSection(dc, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
    if (bitmap) {
        HGDIOBJ old = SelectObject(dc, bitmap);
        memset(bits, 0, (size_t)w * h * 4);
        for (int i = 0; i < (int)g_entries.size(); i++)
            if (!IsRectEmpty(&g_entries[i].rc)) DrawOverlay(dc, g_entries[i], i == g_hover);
        GdiFlush();
        // Icons blend in with their own transparency; shapes GDI drew are
        // solid (it leaves their alpha at 0).
        auto* px = (uint32_t*)bits;
        for (size_t i = 0, count = (size_t)w * h; i < count; i++)
            if (px[i] && !(px[i] >> 24)) px[i] |= 0xFF000000;
        RECT rc;
        GetWindowRect(wnd, &rc);
        POINT src{0, 0}, dst{rc.left, rc.top};
        SIZE size{w, h};
        BLENDFUNCTION blend{AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
        UpdateLayeredWindow(g_overlay, screen, &dst, &size, dc, &src, 0, &blend, ULW_ALPHA);
        SelectObject(dc, old);
        DeleteObject(bitmap);
    }
    DeleteDC(dc);
    ReleaseDC(nullptr, screen);
    SetWindowPos(g_overlay, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
}

// While peeking, a see-through sheet over every screen, just under the switcher: Windows' peek only fades the
// other windows, which still take the mouse (one could be dragged or resized, unseen, by its edge). A click on
// it puts the switcher away, as a click outside Windows' own does.
constexpr WCHAR kPeekSheetClass[] = L"TourneSwitcherPeekSheet_" WH_MOD_ID;
HWND g_peekSheet;

LRESULT CALLBACK PeekSheetProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_MOUSEACTIVATE:
            return MA_NOACTIVATE;
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN:
            if (g_wnd) PostMessageW(g_wnd, WM_APP_CANCEL, 0, 0);
            return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

void ShowPeekSheet(HWND above, bool show) {
    if (!show) {
        if (g_peekSheet) ShowWindow(g_peekSheet, SW_HIDE);
        return;
    }
    if (!g_peekSheet) {
        HINSTANCE instance = nullptr;
        GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                           (LPCWSTR)&PeekSheetProc, &instance);
        WNDCLASSW wc{};
        wc.lpfnWndProc = PeekSheetProc;
        wc.hInstance = instance;
        wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        wc.lpszClassName = kPeekSheetClass;
        RegisterClassW(&wc);
        g_peekSheet = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE | WS_EX_LAYERED, kPeekSheetClass, L"",
                                      WS_POPUP, 0, 0, 1, 1, nullptr, nullptr, instance, nullptr);
        if (!g_peekSheet) return;
        SetLayeredWindowAttributes(g_peekSheet, 0, 1, LWA_ALPHA);  // (1 of 255: unseen, but it takes the clicks)
    }
    int x = GetSystemMetrics(SM_XVIRTUALSCREEN), y = GetSystemMetrics(SM_YVIRTUALSCREEN);
    int w = GetSystemMetrics(SM_CXVIRTUALSCREEN), h = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    SetWindowPos(g_peekSheet, HWND_TOPMOST, x, y, w, h, SWP_NOACTIVATE | SWP_SHOWWINDOW);
    if (above) SetWindowPos(above, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}

// The switcher's peek: Windows' (live-peek.cpp), with the sheet under the switcher while it lasts.
void SwitcherPeek(HWND target, HWND above) {
    LivePeek(target, above, kPeekAltTab);
    ShowPeekSheet(above, target != nullptr);
}

// Peeks at the selection once it has rested there a moment, or at once when
// already peeking.
HWND g_peekPending;  // the selection the peek timer is for

void SchedulePeek(HWND wnd) {
    if (!g_settings.peek || g_sel < 0 || g_sel >= (int)g_entries.size() || g_entries[g_sel].windows.empty()) return;
    HWND target = g_entries[g_sel].windows[0];
    if (g_peeking) {
        SwitcherPeek(target, wnd);
    } else if (target != g_peekPending) {
        g_peekPending = target;
        SetTimer(wnd, kTimerPeek, std::max(1, g_settings.peekDelay), nullptr);
    }
}

void Render(HWND wnd) {
    if (!g_shown) return;
    SchedulePeek(wnd);
    auto const& s = g_settings;
    int w = g_m.window.cx, h = g_m.window.cy;
    if (w <= 0 || h <= 0) return;
    HDC screen = GetDC(nullptr);
    HDC dc = CreateCompatibleDC(screen);
    BITMAPINFO bi{};
    bi.bmiHeader = {sizeof(BITMAPINFOHEADER), w, -h, 1, 32, BI_RGB};
    void* bits = nullptr;
    HBITMAP bitmap = CreateDIBSection(dc, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
    HDC maskDc = CreateCompatibleDC(screen);
    void* maskBits = nullptr;
    HBITMAP mask = CreateDIBSection(maskDc, &bi, DIB_RGB_COLORS, &maskBits, nullptr, 0);
    if (!bitmap || !mask) {
        if (bitmap) DeleteObject(bitmap);
        if (mask) DeleteObject(mask);
        DeleteDC(dc);
        DeleteDC(maskDc);
        ReleaseDC(nullptr, screen);
        return;
    }
    HGDIOBJ oldBitmap = SelectObject(dc, bitmap), oldMask = SelectObject(maskDc, mask);
    Look const& look = s.look;

    // The switcher's shape, and its ground and border.
    int d = Px(s.radius) * 2;
    HRGN shape = d ? CreateRoundRectRgn(0, 0, w + 1, h + 1, d, d) : CreateRectRgn(0, 0, w, h);
    memset(maskBits, 0, (size_t)w * h * 4);
    FillRgn(maskDc, shape, (HBRUSH)GetStockObject(WHITE_BRUSH));
    RECT all{0, 0, w, h};
    Fill(dc, all, look.ground);
    SetDCBrushColor(dc, Rgb(look.border));
    FrameRgn(dc, shape, (HBRUSH)GetStockObject(DC_BRUSH), Px(1), Px(1));
    DeleteObject(shape);

    SetBkMode(dc, TRANSPARENT);
    HGDIOBJ oldFont = SelectObject(dc, g_font);
    for (int i = 0; i < (int)g_entries.size(); i++) {
        if (IsRectEmpty(&g_entries[i].rc)) continue;
        DrawItem(dc, g_entries[i], i == g_sel, i == g_hover);
    }
    // More above or below.
    if (s.overflowIndicator) {
        int per = g_m.rows * g_m.columns;
        if (g_first > 0) DrawChevron(dc, w / 2, Px(2), true, look.border);
        if (g_first + per < (int)g_entries.size()) DrawChevron(dc, w / 2, h - Px(6), false, look.border);
    }
    SelectObject(dc, oldFont);
    GdiFlush();

    // Its transparency: nothing outside the shape; the ground at the set
    // opacity; everything drawn on it solid.
    // (A pixel drawn in RGB(r, g, b) reads back as 0x00RRGGBB, the same form
    // the colours are kept in.)
    auto* px = (uint32_t*)bits;
    auto* in = (uint32_t*)maskBits;
    uint32_t ground = look.ground;
    int alpha = std::max(1, s.opacity * 255 / 100);  // never 0: clicks still land
    for (size_t i = 0, count = (size_t)w * h; i < count; i++) {
        if (!(in[i] & 0xFFFFFF)) {
            px[i] = 0;
            continue;
        }
        uint32_t c = px[i] & 0xFFFFFF;
        if (c == ground && alpha < 255) {
            uint32_t r = ((c >> 16) & 0xFF) * alpha / 255, g = ((c >> 8) & 0xFF) * alpha / 255, b = (c & 0xFF) * alpha / 255;
            px[i] = ((uint32_t)alpha << 24) | (r << 16) | (g << 8) | b;
        } else {
            px[i] = 0xFF000000 | c;
        }
    }

    POINT src{0, 0};
    SIZE size{w, h};
    RECT rc;
    GetWindowRect(wnd, &rc);
    POINT dst{rc.left, rc.top};
    BLENDFUNCTION blend{AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
    UpdateLayeredWindow(wnd, screen, &dst, &size, dc, &src, 0, &blend, ULW_ALPHA);

    SelectObject(dc, oldBitmap);
    SelectObject(maskDc, oldMask);
    DeleteObject(bitmap);
    DeleteObject(mask);
    DeleteDC(dc);
    DeleteDC(maskDc);
    ReleaseDC(nullptr, screen);
    RenderOverlay(wnd);
}

// Acrylic behind the switcher, where its background lets it through.
void SetBackdrop(HWND wnd) {
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
    uint32_t g = g_settings.look.ground;
    // ACCENT_ENABLE_ACRYLICBLURBEHIND, tinted (barely: the drawn ground
    // carries the colour), or ACCENT_DISABLED.
    AccentPolicy policy{g_settings.acrylic ? 4 : 0, 0,
                        0x01000000u | ((g & 0xFF) << 16) | (g & 0xFF00) | ((g >> 16) & 0xFF), 0};
    CompositionData data{19 /* WCA_ACCENT_POLICY */, &policy, sizeof(policy)};
    set(wnd, &data);
}

////////////////////////////////////////////////////////////////////////////////
// Switching

void PressDummyKey() {
    INPUT in[2] = {};
    in[0].type = in[1].type = INPUT_KEYBOARD;
    in[0].ki.wVk = in[1].ki.wVk = kDummyKey;
    in[1].ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(2, in, sizeof(INPUT));
}

void Activate(HWND h) {
    // (a minimised admin window ignores the restore, as Windows keeps our messages from it: SwitchToThisWindow
    // restores it itself)
    if (IsIconic(h)) {
        ShowWindow(h, SW_RESTORE);
        if (IsIconic(h)) SwitchToThisWindow(h, TRUE);
    }
    PressDummyKey();
    if (!SetForegroundWindow(h) || IsIconic(h)) SwitchToThisWindow(h, TRUE);
}

LRESULT CALLBACK MouseProc(int code, WPARAM wParam, LPARAM lParam);
extern std::atomic<bool> g_mouseBindsOn;  // (binds.cpp)
int MouseBindsHook(WPARAM wParam, MSLLHOOKSTRUCT const* m);

// The mouse hook, on the hook thread, only while it's wanted (the wheel, or a mouse bind).
void WatchWheel(bool on) {
    if (g_hookThreadId) PostThreadMessageW(g_hookThreadId, WM_APP_WATCHWHEEL, on || g_settings.wheelOpens || g_mouseBindsOn, 0);
}

// The 3D views (switcher-views.cpp), shown instead of the list when chosen.
bool ViewOpen();
int ViewKind();
bool ViewShown();
void ViewRefit();
void ViewClose();
void ViewFree();
void ViewPrepare();

void Hide(HWND wnd) {
    ViewClose();
    KillTimer(wnd, kTimerShow);
    KillTimer(wnd, kTimerPeek);
    SwitcherPeek(nullptr, nullptr);
    g_peekPending = nullptr;
    g_open = g_shown = g_sticky = false;
    g_viewFor = -1;
    g_hover = -1;
    ShowWindow(wnd, SW_HIDE);
    if (g_overlay) ShowWindow(g_overlay, SW_HIDE);
    ClearEntries(g_entries);
    ClearEntries(g_outer);
    WatchWheel(false);
}

void Refit(HWND wnd) {
    if (!g_shown) return;
    if (ViewShown()) return ViewRefit();
    Arrange(wnd);
    UpdateThumbnails(wnd);
    Render(wnd);
}

void Show(HWND wnd) {
    KillTimer(wnd, kTimerShow);
    if (!g_open || g_shown) return;
    g_shown = true;
    if (ViewOpen()) return;  // a 3D view instead of the list
    Arrange(wnd);
    Render(wnd);
    SetWindowPos(wnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
    UpdateThumbnails(wnd);
    Render(wnd);
}

void Begin(HWND wnd, int dir, bool sameApp, bool sticky, bool allMonitors) {
    std::wstring key;
    if (sameApp) {
        HWND fg = GetForegroundWindow();
        key = AppIdOf(fg);
        if (key.empty()) key = ProgramOf(fg);
        key = Lower(key);
    }
    g_allMonitors = allMonitors;
    g_entries = BuildEntries(sameApp ? &key : nullptr);
    if (g_entries.empty()) return;
    g_open = true;
    g_sticky = sticky || ViewKind() == 7;  // (the canvas stays until something is picked, as PrinceWM's does)
    g_first = 0;
    int n = (int)g_entries.size();
    g_sel = n > 1 ? (dir > 0 ? 1 : n - 1) : 0;
    PressDummyKey();
    WatchWheel(true);
    if (g_settings.showDelay > 0 && !sticky) SetTimer(wnd, kTimerShow, g_settings.showDelay, nullptr);
    else Show(wnd);
}

void Step(HWND wnd, int dir) {
    int n = (int)g_entries.size();
    if (!n) return;
    g_sel = ((g_sel + dir) % n + n) % n;
    Refit(wnd);
}

// Switches to the selection: an app's most recent window, and (as set) its
// other windows too, behind it.
void Commit(HWND wnd, int index) {
    if (index < 0 || index >= (int)g_entries.size()) {
        Hide(wnd);
        return;
    }
    std::vector<HWND> windows = g_entries[index].windows;
    bool grouped = windows.size() > 1 && g_outer.empty();
    Hide(wnd);
    if (grouped && g_settings.restoreAll) {
        for (auto it = windows.rbegin(); it != windows.rend() - 1; ++it) {
            if (IsIconic(*it)) ShowWindow(*it, SW_SHOWNOACTIVATE);
            SetWindowPos(*it, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        }
    }
    Activate(windows[0]);
}

// Closes an item: an app's last used window, or all of them (as set).
void CloseItem(HWND wnd, int index) {
    if (index < 0 || index >= (int)g_entries.size()) return;
    auto& e = g_entries[index];
    size_t closing = g_settings.closeAll && g_outer.empty() ? e.windows.size() : 1;
    for (size_t i = 0; i < closing; i++) PostMessageW(e.windows[i], WM_SYSCOMMAND, SC_CLOSE, 0);
    e.windows.erase(e.windows.begin(), e.windows.begin() + closing);
    if (e.thumbnail) DwmUnregisterThumbnail(e.thumbnail);
    e.thumbnail = nullptr;
    if (e.windows.empty()) {
        if (e.ownsIcon && e.icon) DestroyIcon(e.icon);
        g_entries.erase(g_entries.begin() + index);
    }
    if (g_entries.empty()) {
        Hide(wnd);
        return;
    }
    g_sel = std::min(g_sel, (int)g_entries.size() - 1);
    g_hover = -1;
    Refit(wnd);
}

// Ctrl: into the selected app's windows. Esc: back out.
void Drill(HWND wnd) {
    if (!g_outer.empty() || g_sel < 0 || g_sel >= (int)g_entries.size() || g_entries[g_sel].windows.size() < 2) return;
    std::vector<Entry> inner;
    for (HWND h : g_entries[g_sel].windows) {
        Entry e;
        e.windows = {h};
        e.key = g_entries[g_sel].key;
        e.label = Title(h);
        e.icon = EverywhereIcon(h, ProgramOf(h));
        e.ownsIcon = e.icon != nullptr;
        if (!e.icon) e.icon = WindowIcon(h, g_settings.iconSize > 16 || g_settings.badges);
        inner.push_back(std::move(e));
    }
    for (auto& e : g_entries) {
        if (e.thumbnail) DwmUnregisterThumbnail(e.thumbnail);
        e.thumbnail = nullptr;
    }
    g_outer = std::move(g_entries);
    g_entries = std::move(inner);
    g_sel = 0;
    g_first = 0;
    Refit(wnd);
}

bool Undrill(HWND wnd) {
    if (g_outer.empty()) return false;
    ClearEntries(g_entries);
    g_entries = std::move(g_outer);
    g_outer.clear();
    g_first = 0;
    Refit(wnd);
    return true;
}

// @inline switcher-views.cpp
// @inline switcher-canvas.cpp
// @inline tiling.cpp
// @inline tiling-layouts.cpp
// @inline borders.cpp
// @inline virtual-desktops.cpp
// @inline binds.cpp
// @inline window-rules.cpp
// @inline tiling-groups.cpp
// @inline window-motion.cpp

////////////////////////////////////////////////////////////////////////////////
// The keyboard and the wheel: Alt+Tab and the keys used while switching,
// caught before anything else sees them.

bool g_ctrlTap = false;  // Ctrl went down with nothing else pressed yet

bool WinDown() {
    return GetAsyncKeyState(VK_LWIN) < 0 || GetAsyncKeyState(VK_RWIN) < 0;
}

LRESULT CALLBACK KeyboardProc(int code, WPARAM wParam, LPARAM lParam) {
    if (code != HC_ACTION) return CallNextHookEx(nullptr, code, wParam, lParam);
    auto* k = (KBDLLHOOKSTRUCT*)lParam;
    HWND wnd = g_wnd;
    if (!wnd || k->vkCode == kDummyKey) return CallNextHookEx(nullptr, code, wParam, lParam);
    if (BindsKeyHook(wParam, k) > 0) return 1;
    if (TilingKeyHook(wParam, k) > 0) return 1;
    bool down = wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN;
    bool alt = k->flags & LLKHF_ALTDOWN;
    bool shift = GetAsyncKeyState(VK_SHIFT) < 0;
    bool ctrl = GetAsyncKeyState(VK_CONTROL) < 0;
    DWORD vk = k->vkCode;
    WORD flags = (ctrl ? kFlagSticky : 0) | (WinDown() ? kFlagAllMonitors : 0);

    // Win+Tab, as set: Mission Control, open until something is picked, instead of Windows' Task view. The
    // unassigned key goes in so Win let go isn't a lone tap (Start, Windows' or Tourne's).
    if (vk == VK_TAB && !alt && !g_open && g_settings.winTab && WinDown()) {
        if (down) {
            PressDummyKey();
            PostMessageW(wnd, WM_APP_STEP, 1, MAKELPARAM(kFromWinTab, kFlagSticky));
        }
        return 1;
    }
    if (down && vk == VK_TAB && (alt || g_sticky)) {
        if (g_open) {
            g_tabSteps += shift ? -1 : 1;
            if (!g_tabStepPosted.exchange(true)) PostMessageW(wnd, WM_APP_TABSTEPS, 0, 0);
        } else {
            PostMessageW(wnd, WM_APP_STEP, shift ? (WPARAM)-1 : 1, MAKELPARAM(kFromTab, flags));
        }
        g_ctrlTap = false;
        return 1;
    }
    if (down && vk == VK_OEM_3 && (alt || g_sticky) && g_backtick) {
        PostMessageW(wnd, WM_APP_STEP, shift ? 1 : (WPARAM)-1, MAKELPARAM(kFromBacktick, flags & ~kFlagSticky));
        g_ctrlTap = false;
        return 1;
    }
    if (!g_open) return CallNextHookEx(nullptr, code, wParam, lParam);

    bool isAlt = vk == VK_MENU || vk == VK_LMENU || vk == VK_RMENU;
    bool isCtrl = vk == VK_CONTROL || vk == VK_LCONTROL || vk == VK_RCONTROL;
    if (!down) {
        if (isAlt && !g_sticky) PostMessageW(wnd, WM_APP_COMMIT, 0, 0);
        if (isCtrl && g_ctrlTap) PostMessageW(wnd, WM_APP_DRILL, 0, 0);
        g_ctrlTap = false;
        return CallNextHookEx(nullptr, code, wParam, lParam);
    }
    if (isCtrl) {
        g_ctrlTap = true;
        return CallNextHookEx(nullptr, code, wParam, lParam);
    }
    if (isAlt || vk == VK_SHIFT || vk == VK_LSHIFT || vk == VK_RSHIFT || vk == VK_LWIN || vk == VK_RWIN)
        return CallNextHookEx(nullptr, code, wParam, lParam);
    g_ctrlTap = false;
    // Print Screen and Win's keys still reach Windows and the screenshot tools (the canvas can be captured).
    if (CanvasShown() && (vk == VK_SNAPSHOT || WinDown())) return CallNextHookEx(nullptr, code, wParam, lParam);
    if (CanvasShown()) {  // (every key: the canvas has its own, and notes are typed into)
        PostMessageW(wnd, WM_APP_CANVASKEY, vk, (k->scanCode & 0xFFFF) | (shift ? 1 << 16 : 0) | (ctrl ? 1 << 17 : 0));
        return 1;
    }
    switch (vk) {
        case VK_ESCAPE: PostMessageW(wnd, WM_APP_CANCEL, 0, 0); break;
        case VK_RETURN:
        case VK_SPACE: PostMessageW(wnd, WM_APP_COMMIT, 1, 0); break;
        case VK_LEFT: PostMessageW(wnd, WM_APP_MOVE, (WPARAM)-1, 0); break;
        case VK_RIGHT: PostMessageW(wnd, WM_APP_MOVE, 1, 0); break;
        case VK_UP: PostMessageW(wnd, WM_APP_MOVE, 0, -1); break;
        case VK_DOWN: PostMessageW(wnd, WM_APP_MOVE, 0, 1); break;
        case 'Q':
        case VK_DELETE: PostMessageW(wnd, WM_APP_CLOSEONE, 0, 0); break;
        case 'W':
            if (ctrl) PostMessageW(wnd, WM_APP_CLOSEONE, 0, 0);
            break;
    }
    // Everything else is kept from the app underneath while switching.
    return 1;
}

// The wheel, anywhere: moves the selection while switching, and (as set)
// opens the switcher with Alt held.
LRESULT CALLBACK MouseProc(int code, WPARAM wParam, LPARAM lParam) {
    if (code == HC_ACTION && wParam != WM_MOUSEWHEEL && MouseBindsHook(wParam, (MSLLHOOKSTRUCT const*)lParam) > 0) return 1;
    if (code == HC_ACTION && wParam == WM_MOUSEWHEEL) {
        HWND wnd = g_wnd;
        auto* m = (MSLLHOOKSTRUCT*)lParam;
        short delta = (short)HIWORD(m->mouseData);
        bool other = GetAsyncKeyState(g_wheelModifierCtrl ? VK_CONTROL : VK_SHIFT) < 0;
        if (wnd && g_open) {
            PostMessageW(wnd, WM_APP_WHEEL, (WPARAM)(INT_PTR)delta, other);
            return 1;
        }
        if (wnd && g_wheelOpens && GetAsyncKeyState(VK_MENU) < 0) {
            int dir = (delta > 0) != g_reverseWheel ? -1 : 1;
            PostMessageW(wnd, WM_APP_STEP, (WPARAM)(INT_PTR)dir, MAKELPARAM(kFromWheel, WinDown() ? kFlagAllMonitors : 0));
            return 1;
        }
    }
    return CallNextHookEx(nullptr, code, wParam, lParam);
}

DWORD WINAPI HookThreadProc(LPVOID ready) {
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
    HINSTANCE instance = nullptr;
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                       (LPCWSTR)&KeyboardProc, &instance);
    MSG msg;
    PeekMessageW(&msg, nullptr, 0, 0, PM_NOREMOVE);  // the queue, for WatchWheel's messages
    // While a program Tourne leaves alone is in front (a game), no hook at all: let go, and put back after.
    bool leftAlone = LeftAloneInFront(), wheelWanted = false;
#ifdef TD_PREFIX
    static bool watching = false;  // (once: it tells whichever hook thread is running)
    if (!watching) {
        watching = true;
        td::WatchExcludedFront([](bool inFront) {
            if (g_hookThreadId) PostThreadMessageW(g_hookThreadId, WM_APP_LEFTALONE, inFront, 0);
        });
    }
#endif
    if (!leftAlone) g_keyHook = SetWindowsHookExW(WH_KEYBOARD_LL, KeyboardProc, instance, 0);
    if (!g_keyHook && !leftAlone) Wh_Log(L"Couldn't catch Alt+Tab: %u", GetLastError());
    SetEvent(ready);
    // Windows drops a hook that was too slow a few times, without a word; it's
    // put back now and then, which also keeps it ahead of hooks set since.
    UINT_PTR renew = SetTimer(nullptr, 0, 5000, nullptr);
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        if (!msg.hwnd && msg.message == WM_APP_LEFTALONE) {
            leftAlone = msg.wParam != 0;
            if (leftAlone) {
                if (g_keyHook) UnhookWindowsHookEx(g_keyHook);
                if (g_mouseHook) UnhookWindowsHookEx(g_mouseHook);
                g_keyHook = g_mouseHook = nullptr;
            } else {
                if (!g_keyHook) g_keyHook = SetWindowsHookExW(WH_KEYBOARD_LL, KeyboardProc, instance, 0);
                if (wheelWanted && !g_mouseHook) g_mouseHook = SetWindowsHookExW(WH_MOUSE_LL, MouseProc, instance, 0);
            }
            continue;
        }
        if (!msg.hwnd && msg.message == WM_TIMER && msg.wParam == renew) {
            if (!g_open && !leftAlone) {
                if (HHOOK fresh = SetWindowsHookExW(WH_KEYBOARD_LL, KeyboardProc, instance, 0)) {
                    if (g_keyHook) UnhookWindowsHookEx(g_keyHook);
                    g_keyHook = fresh;
                }
            }
            continue;
        }
        if (msg.hwnd || msg.message != WM_APP_WATCHWHEEL) continue;
        wheelWanted = msg.wParam != 0;
        if (leftAlone) continue;
        if (msg.wParam && !g_mouseHook) {
            g_mouseHook = SetWindowsHookExW(WH_MOUSE_LL, MouseProc, instance, 0);
        } else if (!msg.wParam && g_mouseHook) {
            UnhookWindowsHookEx(g_mouseHook);
            g_mouseHook = nullptr;
        }
    }
    KillTimer(nullptr, renew);
    if (g_keyHook) UnhookWindowsHookEx(g_keyHook);
    g_keyHook = nullptr;
    if (g_mouseHook) UnhookWindowsHookEx(g_mouseHook);
    g_mouseHook = nullptr;
    return 0;
}

void StartHooks() {
    HANDLE ready = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    g_hookThread = CreateThread(nullptr, 0, HookThreadProc, ready, 0, &g_hookThreadId);
    if (g_hookThread) WaitForSingleObject(ready, 5000);
    CloseHandle(ready);
}

void StopHooks() {
    if (!g_hookThread) return;
    PostThreadMessageW(g_hookThreadId, WM_QUIT, 0, 0);
    WaitForSingleObject(g_hookThread, 5000);
    CloseHandle(g_hookThread);
    g_hookThread = nullptr;
    g_hookThreadId = 0;
}

////////////////////////////////////////////////////////////////////////////////
// The window

int ItemAt(POINT pt, bool* onClose) {
    for (int i = 0; i < (int)g_entries.size(); i++) {
        auto const& e = g_entries[i];
        if (!PtInRect(&e.rc, pt)) continue;
        if (onClose) *onClose = !IsRectEmpty(&e.close) && PtInRect(&e.close, pt);
        return i;
    }
    return -1;
}

void ApplySettings(HWND wnd) {
    if (g_open) Hide(wnd);
    g_settings = LoadSettings();
    if (!g_settings.view) ViewFree();
    else ViewPrepare();
    g_wheelOpens = g_settings.wheelOpens;
    g_backtick = g_settings.backtick;
    g_wheelModifierCtrl = g_settings.wheelModifierCtrl;
    g_reverseWheel = g_settings.reverseWheel;
    LoadTilingSettings();
    LoadKeptPlaces();
    if (g_placesDirty) SavePlaces();
    LoadPlaces();
    LoadLayouts();
    StartTileEvents();
    LoadBorderSettings();
    StartBorders();
    ApplyBindSettings();
    StartWindowMotion();
    PublishChoices();
    SetBackdrop(wnd);
    DWORD corner = g_settings.acrylic && g_settings.radius > 0 ? 2 /* DWMWCP_ROUND */ : 1 /* DWMWCP_DONOTROUND */;
    DwmSetWindowAttribute(wnd, 33 /* DWMWA_WINDOW_CORNER_PREFERENCE */, &corner, sizeof(corner));
    COLORREF none = 0xFFFFFFFE;  // DWMWA_COLOR_NONE: the border is drawn here
    DwmSetWindowAttribute(wnd, 34 /* DWMWA_BORDER_COLOR */, &none, sizeof(none));
    WatchWheel(false);
}

LRESULT CALLBACK WndProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    // Tourne Desktop made new theme colours.
    if (msg == TourneThemeMessage() && msg) {
        if (g_followsTourneTheme) ApplySettings(wnd);
        return 0;
    }
    if (LRESULT result = 0; BindsMessage(msg, wParam, lParam, &result)) return result;
    switch (msg) {
        case WM_APP_STEP: {
            int dir = (int)(INT_PTR)wParam;
            WORD from = LOWORD(lParam), flags = HIWORD(lParam);
            bool backtick = from == kFromBacktick;
            if (!g_open) {
                if (from == kFromWinTab) g_viewFor = g_settings.overviewView;  // the canvas, or Mission Control
                // Alt+`: backwards, or this app's windows only.
                bool sameApp = backtick && g_settings.backtickSameApp;
                Begin(wnd, sameApp ? 1 : dir, sameApp, flags & kFlagSticky, flags & kFlagAllMonitors);
            } else {
                // In "this app's windows" mode, ` goes forward and Shift+` back.
                Step(wnd, backtick && g_settings.backtickSameApp ? -dir : dir);
            }
            // Alt was let go before this arrived (a quick Alt+Tab, where the
            // key up can overtake the message): switch now.
            if (g_open && !g_sticky && GetAsyncKeyState(VK_MENU) >= 0) Commit(wnd, g_sel);
            return 0;
        }
        case WM_APP_TABSTEPS: {
            g_tabStepPosted = false;
            int steps = g_tabSteps.exchange(0);
            if (!steps) return 0;
            if (!g_open) {
                // Alt let go and pressed again before the switch went through:
                // a new Alt+Tab.
                if (GetAsyncKeyState(VK_MENU) >= 0) return 0;
                Begin(wnd, steps > 0 ? 1 : -1, false, false, WinDown());
                if (!g_open) return 0;
                steps -= steps > 0 ? 1 : -1;
            }
            if (steps) Step(wnd, steps);
            if (!g_sticky && GetAsyncKeyState(VK_MENU) >= 0) Commit(wnd, g_sel);
            return 0;
        }
        case WM_APP_WHEEL: {
            if (!g_open) return 0;
            if (CanvasShown()) {
                CanvasWheel((short)wParam);
                return 0;
            }
            int dir = ((short)wParam > 0) != g_settings.reverseWheel ? -1 : 1;
            bool page = g_settings.wheelPages != (lParam != 0);
            Show(wnd);
            Step(wnd, page ? dir * std::max(1, g_m.rows * g_m.columns) : dir);
            return 0;
        }
        case WM_APP_CANVASKEY:
            if (g_open) CanvasKey((UINT)wParam, LOWORD(lParam), (lParam >> 16) & 1, (lParam >> 17) & 1);
            return 0;
        case WM_APP_MOVE: {
            if (!g_open) return 0;
            int dx = (int)(INT_PTR)wParam, dy = (int)lParam;
            if (CanvasShown()) {
                CanvasMove(dx, dy, GetAsyncKeyState(VK_SHIFT) < 0);
                return 0;
            }
            Step(wnd, g_settings.grid ? dx + dy * std::max(1, g_m.columns) : dx + dy);
            return 0;
        }
        case WM_APP_COMMIT:
            if (g_open) Commit(wnd, g_sel);
            return 0;
        case WM_APP_CANCEL:
            if (g_open && !Undrill(wnd)) Hide(wnd);
            return 0;
        case WM_APP_CLOSEONE:
            if (g_open) CloseItem(wnd, g_sel);
            return 0;
        case WM_APP_DRILL:
            if (g_open) {
                Show(wnd);
                Drill(wnd);
            }
            return 0;
        case WM_APP_TILE:
            TileCommandRun(wParam, lParam);
            UpdateFocusBorder(GetForegroundWindow());
            if (g_borderSettings.frontMoves) RefreshAllBorders();
            return 0;
        case WM_TIMER:
            if (wParam == kTimerTile) TileTimer(wnd);
            if (wParam == kTimerPlaces) PlacesTimer(wnd);
            if (wParam == kTimerBorders) BordersTimer(wnd);
            if (wParam == kTimerBorderZ) BordersReorderTimer(wnd);
            if (wParam == kTimerWindowMoves) WindowMovesTimer(wnd);
            if ((wParam == kTimerTile || wParam == kTimerWindowMoves) && g_borderSettings.frontMoves) RefreshAllBorders();
            if (wParam == kTimerShow) Show(wnd);
            if (wParam == kTimerPeek) {
                KillTimer(wnd, kTimerPeek);
                if (g_shown && g_sel >= 0 && g_sel < (int)g_entries.size())
                    SwitcherPeek(g_entries[g_sel].windows[0], wnd);
            }
            return 0;
        case WM_MOUSEACTIVATE:
            return MA_NOACTIVATE;
        case WM_MOUSEMOVE: {
            TRACKMOUSEEVENT tme{sizeof(tme), TME_LEAVE, wnd, 0};
            TrackMouseEvent(&tme);
            bool onClose = false;
            int at = ItemAt({GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)}, &onClose);
            if (at != g_hover || onClose != g_hoverClose) {
                g_hover = at;
                g_hoverClose = onClose;
                if (at >= 0) g_sel = at;
                Render(wnd);
            }
            return 0;
        }
        case WM_MOUSELEAVE:
            g_hover = -1;
            Render(wnd);
            return 0;
        case WM_LBUTTONUP: {
            bool onClose = false;
            int at = ItemAt({GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)}, &onClose);
            if (at >= 0 && onClose) CloseItem(wnd, at);
            else if (at >= 0) Commit(wnd, at);
            return 0;
        }
        case WM_MBUTTONUP: {
            int at = ItemAt({GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)}, nullptr);
            if (at >= 0) CloseItem(wnd, at);
            return 0;
        }
        case WM_SETTINGCHANGE:
            // Windows switching between light and dark.
            if (lParam && !wcscmp((PCWSTR)lParam, L"ImmersiveColorSet")) ApplySettings(wnd);
            return 0;
        case WM_THEMECHANGED:
            // Another visual style: its colours, for the styles that follow it.
            ApplySettings(wnd);
            return 0;
        case WM_APP_SETTINGS:
            ApplySettings(wnd);
            return 0;
        case WM_APP_EXIT:
            DestroyWindow(wnd);
            return 0;
        case WM_DWMCOLORIZATIONCOLORCHANGED:
            // The accent colour, for borders drawn in it.
            BordersAccentChanged();
            return 0;
        case WM_DESTROY:
            EndWindowMoves();
            StopTileEvents();
            EndBorders();
            EndBinds();
            EndWindowMotion();
            EndGroups();
            if (g_placesDirty) SavePlaces();
            if (g_ly.wnd) DestroyWindow(g_ly.wnd);
            Hide(wnd);
            ViewFree();
            g_wnd = nullptr;
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(wnd, msg, wParam, lParam);
}

DWORD WINAPI ThreadProc(LPVOID ready) {
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    CoCreateInstance(CLSID_VirtualDesktopManager, nullptr, CLSCTX_ALL, IID_PPV_ARGS(&g_desktops));

    HINSTANCE instance = nullptr;
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                       (LPCWSTR)&WndProc, &instance);
    WNDCLASSW wc{};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = instance;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.lpszClassName = kWindowClass;
    RegisterClassW(&wc);
    HWND wnd = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE | WS_EX_LAYERED, kWindowClass,
                               L"Tourne switcher", WS_POPUP, 0, 0, 1, 1, nullptr, nullptr, instance, nullptr);
    g_wnd = wnd;
    WNDCLASSW overlay{};
    overlay.lpfnWndProc = DefWindowProcW;
    overlay.hInstance = instance;
    overlay.lpszClassName = kOverlayClass;
    RegisterClassW(&overlay);
    // Above the switcher (it owns it), and clicks go through to it.
    if (wnd)
        g_overlay = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE | WS_EX_LAYERED | WS_EX_TRANSPARENT,
                                    kOverlayClass, L"", WS_POPUP, 0, 0, 1, 1, wnd, nullptr, instance, nullptr);
    KeepOutOfPeek(wnd);
    KeepOutOfPeek(g_overlay);
    if (wnd) {
        StartHooks();
        ApplySettings(wnd);
    }
    SetEvent(ready);

    MSG msg;
    while (wnd && GetMessageW(&msg, nullptr, 0, 0) > 0) {
        if (LayoutsDialogMessage(&msg)) continue;
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    StopHooks();
    if (g_font) DeleteObject(g_font);
    if (g_countFont) DeleteObject(g_countFont);
    g_font = g_countFont = nullptr;
    if (g_desktops) g_desktops->Release();
    g_desktops = nullptr;
    g_overlay = nullptr;  // destroyed with its owner
    if (g_peekSheet) DestroyWindow(g_peekSheet);
    g_peekSheet = nullptr;
    UnregisterClassW(kPeekSheetClass, instance);
    UnregisterClassW(kWindowClass, instance);
    UnregisterClassW(kOverlayClass, instance);
    UnregisterClassW(kViewClass, instance);
    CoUninitialize();
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Mod lifecycle

BOOL Wh_ModInit() {
    Wh_Log(L">");
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

void Wh_ModUninit() {
    Wh_Log(L">");
    if (HWND wnd = g_wnd) PostMessageW(wnd, WM_APP_EXIT, 0, 0);
    if (g_thread) {
        WaitForSingleObject(g_thread, 10000);
        CloseHandle(g_thread);
        g_thread = nullptr;
    }
}

void Wh_ModSettingsChanged() {
    Wh_Log(L">");
    if (HWND wnd = g_wnd) PostMessageW(wnd, WM_APP_SETTINGS, 0, 0);
}
