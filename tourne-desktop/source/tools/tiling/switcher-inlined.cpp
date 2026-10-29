// ==WindhawkMod==
// @id              tourne-switcher
// @name            Tourne'Switcher
// @description     An Alt+Tab window switcher in the Tourne style, themed like the rest of the suite
// @version         0.2.0
// @author          USER-TOURNE
// @github          https://github.com/USER-TOURNE
// @include         windhawk.exe
// @architecture    x86-64
// @compilerOptions -luxtheme -ldwmapi -lole32 -lshell32 -lshlwapi -lgdi32 -luser32 -lversion
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
    - monocle: Monocle (every window fills the screen)
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
    - monocle: Monocle (every window fills the screen)
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
  - tooSmall: force
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
      alpha, like 64aa89cc) or the word accent for Windows' accent colour.
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
      For the spin and the fade. Nothing is drawn while nothing moves; with
      a spin on, the borders that spin are redrawn this often.
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

// Colour schemes: Tourne's own, and the palettes the ricing community lives
// in, so the flyouts, tray icons and taskbar can match a system themed with
// any of them. Placed into System Icons and the taskbar by the build
// ("// @inline palettes.cpp").
//
// Each scheme fills the same roles:
//   ground  the flyout and bar background
//   raised  a selected row, a pressed or active button
//   line    dividers
//   edge    borders
//   text    main text, and the "ink" icon tone
//   accent  links, headings, switches, the "green" tone
//   muted   secondary text, the "sage" tone
//   dim     faint text, the "alt" tone
//   warn    warnings and values, the "coral" tone
//   bad     errors, the "red" tone
// Hover is text at 16% over ground, as in the original design.

struct Scheme {
    PCWSTR key;
    uint32_t ground, raised, line, edge, text, accent, muted, dim, warn, bad;
};

constexpr Scheme kSchemes[] = {
    {L"tourne",              0x121C21, 0x1B2A31, 0x1E2F36, 0x4F8F6A, 0x66EA97, 0x15FFA2, 0x64AA89, 0x5D8D6F, 0xFF6F77, 0xFF8F8F},
    {L"gruvboxDarkHard",     0x1D2021, 0x3C3836, 0x32302F, 0x665C54, 0xEBDBB2, 0xB8BB26, 0xA89984, 0x7C6F64, 0xFE8019, 0xFB4934},
    {L"gruvboxDark",         0x282828, 0x3C3836, 0x3C3836, 0x665C54, 0xEBDBB2, 0xB8BB26, 0xA89984, 0x7C6F64, 0xFE8019, 0xFB4934},
    {L"gruvboxDarkSoft",     0x32302F, 0x504945, 0x3C3836, 0x665C54, 0xEBDBB2, 0xB8BB26, 0xA89984, 0x7C6F64, 0xFE8019, 0xFB4934},
    {L"gruvboxLight",        0xFBF1C7, 0xEBDBB2, 0xD5C4A1, 0xBDAE93, 0x3C3836, 0x79740E, 0x7C6F64, 0x928374, 0xAF3A03, 0x9D0006},
    {L"monokai",             0x272822, 0x3E3D32, 0x3E3D32, 0x75715E, 0xF8F8F2, 0xA6E22E, 0xCFCFC2, 0x75715E, 0xFD971F, 0xF92672},
    {L"catppuccinMocha",     0x1E1E2E, 0x313244, 0x313244, 0x45475A, 0xCDD6F4, 0xCBA6F7, 0xA6ADC8, 0x6C7086, 0xFAB387, 0xF38BA8},
    {L"catppuccinMacchiato", 0x24273A, 0x363A4F, 0x363A4F, 0x494D64, 0xCAD3F5, 0xC6A0F6, 0xA5ADCB, 0x6E738D, 0xF5A97F, 0xED8796},
    {L"catppuccinFrappe",    0x303446, 0x414559, 0x414559, 0x51576D, 0xC6D0F5, 0xCA9EE6, 0xA5ADCE, 0x737994, 0xEF9F76, 0xE78284},
    {L"catppuccinLatte",     0xEFF1F5, 0xCCD0DA, 0xCCD0DA, 0xBCC0CC, 0x4C4F69, 0x8839EF, 0x6C6F85, 0x9CA0B0, 0xFE640B, 0xD20F39},
    {L"everforestDarkHard",  0x272E33, 0x374145, 0x2E383C, 0x495156, 0xD3C6AA, 0xA7C080, 0x9DA9A0, 0x7A8478, 0xE69875, 0xE67E80},
    {L"everforestDark",      0x2D353B, 0x3D484D, 0x343F44, 0x4F585E, 0xD3C6AA, 0xA7C080, 0x9DA9A0, 0x7A8478, 0xE69875, 0xE67E80},
    {L"everforestDarkSoft",  0x333C43, 0x434F55, 0x3A464C, 0x555F66, 0xD3C6AA, 0xA7C080, 0x9DA9A0, 0x7A8478, 0xE69875, 0xE67E80},
    {L"everforestLightHard", 0xFFFBEF, 0xF2EFDF, 0xF8F5E4, 0xBEC5B2, 0x5C6A72, 0x8DA101, 0x829181, 0xA6B0A0, 0xF57D26, 0xF85552},
    {L"everforestLight",     0xFDF6E3, 0xEFEBD4, 0xF4F0D9, 0xBDC3AF, 0x5C6A72, 0x8DA101, 0x829181, 0xA6B0A0, 0xF57D26, 0xF85552},
    {L"everforestLightSoft", 0xF3EAD3, 0xE5DFC5, 0xEAE4CA, 0xB9C0AB, 0x5C6A72, 0x8DA101, 0x829181, 0xA6B0A0, 0xF57D26, 0xF85552},
    {L"nord",                0x2E3440, 0x3B4252, 0x3B4252, 0x4C566A, 0xECEFF4, 0x88C0D0, 0x81A1C1, 0x616E88, 0xD08770, 0xBF616A},
    {L"dracula",             0x282A36, 0x44475A, 0x44475A, 0x6272A4, 0xF8F8F2, 0xBD93F9, 0x8BE9FD, 0x6272A4, 0xFFB86C, 0xFF5555},
    {L"tokyoNight",          0x1A1B26, 0x292E42, 0x292E42, 0x414868, 0xC0CAF5, 0x7AA2F7, 0xA9B1D6, 0x565F89, 0xFF9E64, 0xF7768E},
    {L"tokyoNightStorm",     0x24283B, 0x292E42, 0x292E42, 0x414868, 0xC0CAF5, 0x7AA2F7, 0xA9B1D6, 0x565F89, 0xFF9E64, 0xF7768E},
    {L"rosePine",            0x191724, 0x26233A, 0x1F1D2E, 0x403D52, 0xE0DEF4, 0xC4A7E7, 0x908CAA, 0x6E6A86, 0xF6C177, 0xEB6F92},
    {L"rosePineMoon",        0x232136, 0x393552, 0x2A273F, 0x44415A, 0xE0DEF4, 0xC4A7E7, 0x908CAA, 0x6E6A86, 0xF6C177, 0xEB6F92},
    {L"rosePineDawn",        0xFAF4ED, 0xF2E9E1, 0xFFFAF3, 0xDFDAD9, 0x575279, 0x907AA9, 0x797593, 0x9893A5, 0xEA9D34, 0xB4637A},
    {L"kanagawa",            0x1F1F28, 0x2A2A37, 0x2A2A37, 0x54546D, 0xDCD7BA, 0x7E9CD8, 0xC8C093, 0x727169, 0xFFA066, 0xFF5D62},
    {L"oneDark",             0x282C34, 0x2C313A, 0x2C313A, 0x3E4451, 0xABB2BF, 0x61AFEF, 0x828997, 0x5C6370, 0xD19A66, 0xE06C75},
    {L"solarizedDark",       0x002B36, 0x073642, 0x073642, 0x586E75, 0x93A1A1, 0x268BD2, 0x839496, 0x586E75, 0xCB4B16, 0xDC322F},
    {L"solarizedLight",      0xFDF6E3, 0xEEE8D5, 0xEEE8D5, 0x93A1A1, 0x586E75, 0x268BD2, 0x657B83, 0x93A1A1, 0xCB4B16, 0xDC322F},
};

struct Palette {
    uint32_t ground, raised, line, edge, text, accent, muted, dim, warn, bad, hover;
    bool tourne;
};

// b over a at t (0..1), per channel.
constexpr uint32_t MixRgb(uint32_t a, uint32_t b, double t) {
    auto ch = [&](int shift) {
        double x = ((a >> shift) & 0xFF) * (1 - t) + ((b >> shift) & 0xFF) * t;
        return (uint32_t)(x + 0.5) << shift;
    };
    return ch(16) | ch(8) | ch(0);
}

Palette SchemePalette(std::wstring_view key) {
    Scheme const* s = &kSchemes[0];
    for (auto const& each : kSchemes) {
        if (key == each.key) s = &each;
    }
    return {s->ground, s->raised, s->line,  s->edge, s->text,
            s->accent, s->muted,  s->dim,   s->warn, s->bad,
            MixRgb(s->ground, s->text, 0.16), s == &kSchemes[0]};
}

// The one-step darker shade drawn under icon pixels. Tourne keeps its
// hand-picked shades; other schemes take the colour halfway to the ground.
uint32_t PaletteShade(Palette const& p, uint32_t color) {
    if (p.tourne) {
        switch (color) {
            case 0x64AA89: return 0x3D6657;
            case 0x66EA97: return 0x3C865C;
            case 0x15FFA2: return 0x1B9650;
            case 0xFF8F8F: return 0x8D585A;
            case 0xFF6F77: return 0xA95D5D;
            case 0x5D8D6F: return 0x366E53;
        }
        return color;
    }
    return MixRgb(p.ground, color, 0.5);
}

constexpr COLORREF PaletteRef(uint32_t rgb) {
    return RGB((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
}
// Theme files, and colour schemes taken from them. Shared by System Icons, the
// taskbar and the Switcher ("// @inline theme-palette.cpp", after
// palettes.cpp).
//
// Windows only offers visual styles for the whole system, so a theme file is
// loaded the way theme tools do it: uxtheme #92 (LoaderLoadTheme) parses it
// into two memory sections, and #16 (OpenThemeDataFromFile) opens a class
// from them. A class like "Taskbar2Composited::TaskBar" is the app name
// "Taskbar2Composited", set on a window with SetWindowTheme, plus "TaskBar".
//
// Two schemes come from a theme instead of the list in palettes.cpp:
//   visualStyle   the .msstyles Windows is using
//   taskbarStyle  the taskbar's theme (its Visual style setting, else the one
//                 StartAllBack is set to), else the visual style
// Both read the theme's menu look (its popup background, border, hot item and
// text), as Windows draws menus and flyouts with it. A theme without one, or
// no theme, gives Windows' own colours.

using LoaderLoadTheme_t = HRESULT(WINAPI*)(HANDLE themeFile, HINSTANCE instance, LPCWSTR fileName, LPCWSTR color,
                                           LPCWSTR size, HANDLE* sharableSection, LPWSTR sharableName,
                                           int sharableChars, HANDLE* nonSharableSection, LPWSTR nonSharableName,
                                           int nonSharableChars, PVOID customLoadHandler, HANDLE* reuseSection,
                                           int a, int b, BOOL emulateGlobal);

struct ThemeFileData {
    char header[7];
    void* sharableView;
    HANDLE sharableSection;
    void* nonSharableView;
    HANDLE nonSharableSection;
    char end[3];
};

using OpenThemeDataFromFile_t = HTHEME(WINAPI*)(ThemeFileData* themeFile, HWND wnd, LPCWSTR classList, DWORD flags);

OpenThemeDataFromFile_t ThemeFileOpener() {
    HMODULE ux = GetModuleHandleW(L"uxtheme.dll");
    return ux ? (OpenThemeDataFromFile_t)GetProcAddress(ux, MAKEINTRESOURCEA(16)) : nullptr;
}

bool LoadThemeFile(std::wstring const& path, ThemeFileData* file) {
    *file = {};
    HMODULE ux = GetModuleHandleW(L"uxtheme.dll");
    auto load = ux ? (LoaderLoadTheme_t)GetProcAddress(ux, MAKEINTRESOURCEA(92)) : nullptr;
    if (!load || !ThemeFileOpener() || path.empty() || GetFileAttributesW(path.c_str()) == INVALID_FILE_ATTRIBUTES)
        return false;
    HANDLE sharable = nullptr, nonSharable = nullptr;
    // No section names: with names it tries to create named sections and
    // gets access denied.
    HRESULT hr = load(nullptr, nullptr, path.c_str(), L"NormalColor", L"NormalSize", &sharable, nullptr, 0,
                      &nonSharable, nullptr, 0, nullptr, nullptr, 0, 0, FALSE);
    if (FAILED(hr) || !sharable || !nonSharable) {
        Wh_Log(L"Couldn't load %s: 0x%08X", path.c_str(), hr);
        if (sharable) CloseHandle(sharable);
        if (nonSharable) CloseHandle(nonSharable);
        return false;
    }
    memcpy(file->header, "thmfile", 7);
    memcpy(file->end, "end", 3);
    file->sharableSection = sharable;
    file->nonSharableSection = nonSharable;
    file->sharableView = MapViewOfFile(sharable, FILE_MAP_READ, 0, 0, 0);
    file->nonSharableView = MapViewOfFile(nonSharable, FILE_MAP_READ, 0, 0, 0);
    return true;
}

void CloseThemeFile(ThemeFileData* file) {
    if (file->sharableView) UnmapViewOfFile(file->sharableView);
    if (file->nonSharableView) UnmapViewOfFile(file->nonSharableView);
    if (file->sharableSection) CloseHandle(file->sharableSection);
    if (file->nonSharableSection) CloseHandle(file->nonSharableSection);
    *file = {};
}

// One pixel of a part drawn into a small bitmap, or the fallback when the
// part isn't defined or draws nothing there.
uint32_t PartPixel(HTHEME theme, int part, int state, int x, int y, uint32_t fallback) {
    constexpr int kSize = 9;
    if (!IsThemePartDefined(theme, part, 0)) return fallback;
    HDC dc = CreateCompatibleDC(nullptr);
    BITMAPINFO bi{};
    bi.bmiHeader = {sizeof(BITMAPINFOHEADER), kSize, -kSize, 1, 32, BI_RGB};
    void* bits = nullptr;
    HBITMAP bitmap = CreateDIBSection(dc, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
    uint32_t result = fallback;
    if (bitmap) {
        HGDIOBJ old = SelectObject(dc, bitmap);
        memset(bits, 0, kSize * kSize * 4);
        RECT r{0, 0, kSize, kSize};
        if (SUCCEEDED(DrawThemeBackground(theme, dc, part, state, &r, nullptr))) {
            GdiFlush();
            uint32_t pixel = ((uint32_t*)bits)[y * kSize + x];
            if (pixel) result = pixel & 0xFFFFFF;
        }
        SelectObject(dc, old);
        DeleteObject(bitmap);
    }
    DeleteDC(dc);
    return result;
}

// A theme file's menu class. In dark mode, the style's dark menus first (as
// Windows draws them for apps in dark mode), then its plain ones; in light
// mode the other way round. A style made for one mode may only have that
// one. Null when the style has no menu look of its own.
HTHEME OpenThemeFileMenu(ThemeFileData* file, HWND host, bool dark) {
    auto open = ThemeFileOpener();
    if (!open) return nullptr;
    PCWSTR darkFirst[] = {L"DarkMode_ImmersiveStart", L"ImmersiveStartDark", L"DarkMode", L""};
    PCWSTR lightFirst[] = {L"", L"DarkMode_ImmersiveStart", L"ImmersiveStartDark", L"DarkMode"};
    HTHEME found = nullptr;
    for (PCWSTR app : dark ? darkFirst : lightFirst) {
        SetWindowTheme(host, app, nullptr);
        HTHEME menu = open(file, host, L"Menu", 0);
        if (menu && IsThemePartDefined(menu, MENU_POPUPITEM, 0)) {
            found = menu;
            break;
        }
        if (menu) CloseThemeData(menu);
    }
    SetWindowTheme(host, L"", nullptr);
    return found;
}

// The visual style Windows is using now.
std::wstring CurrentThemeFile() {
    WCHAR file[MAX_PATH] = L"";
    if (FAILED(GetCurrentThemeName(file, ARRAYSIZE(file), nullptr, 0, nullptr, 0))) return {};
    return file;
}

// The taskbar and its flyouts follow the system mode, not the apps' one.
bool SystemUsesLightTheme() {
    DWORD light = 0, size = sizeof(light);
    RegGetValueW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
                 L"SystemUsesLightTheme", RRF_RT_REG_DWORD, nullptr, &light, &size);
    return light != 0;
}

// Windows' accent colour, as RGB.
uint32_t WindowsAccent() {
    DWORD abgr = 0, size = sizeof(abgr);
    if (RegGetValueW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\DWM", L"AccentColor", RRF_RT_REG_DWORD,
                     nullptr, &abgr, &size) != ERROR_SUCCESS)
        return 0x0078D4;
    return ((abgr & 0xFF) << 16) | (abgr & 0xFF00) | ((abgr >> 16) & 0xFF);
}

double PaletteLuma(uint32_t rgb) {
    return (0.299 * ((rgb >> 16) & 0xFF) + 0.587 * ((rgb >> 8) & 0xFF) + 0.114 * (rgb & 0xFF)) / 255;
}

// Windows' own colours, for what no theme sets.
Palette NativePalette(bool light) {
    uint32_t accent = WindowsAccent();
    Palette p = light ? Palette{0xF3F3F3, 0xE5E5E5, 0xD6D6D6, 0xB8B8B8, 0x1A1A1A, accent,
                                0x5C5C5C, 0x8A8A8A, 0x9D5D00, 0xC42B1C, 0xE5E5E5, false}
                      : Palette{0x202020, 0x2D2D2D, 0x3A3A3A, 0x4A4A4A, 0xFFFFFF, accent,
                                0xC8C8C8, 0x8A8A8A, 0xFCE100, 0xFF99A4, 0x2D2D2D, false};
    if (std::abs(PaletteLuma(p.accent) - PaletteLuma(p.ground)) < 0.25) p.accent = MixRgb(p.accent, p.text, 0.5);
    return p;
}

// A theme file's colours, from its menu look. False when it has none.
bool PaletteFromThemeFile(std::wstring const& path, bool light, Palette* out) {
    // Loading a theme takes a moment, and settings are read often: the last
    // one read is kept, until its file or the mode changes.
    static SRWLOCK lock = SRWLOCK_INIT;
    static std::wstring cachedKey;
    static bool cachedOk = false;
    static Palette cached{};
    WIN32_FILE_ATTRIBUTE_DATA info{};
    if (path.empty() || !GetFileAttributesExW(path.c_str(), GetFileExInfoStandard, &info)) return false;
    std::wstring key = path + L"|" + std::to_wstring(info.ftLastWriteTime.dwLowDateTime) + L"|" +
                       std::to_wstring(info.ftLastWriteTime.dwHighDateTime) + (light ? L"|light" : L"|dark") +
                       L"|" + std::to_wstring(WindowsAccent());
    AcquireSRWLockExclusive(&lock);
    if (key == cachedKey) {
        bool ok = cachedOk;
        if (ok) *out = cached;
        ReleaseSRWLockExclusive(&lock);
        return ok;
    }
    ReleaseSRWLockExclusive(&lock);

    ThemeFileData file;
    if (!LoadThemeFile(path, &file)) return false;
    HWND host = CreateWindowExW(0, L"STATIC", L"", 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, nullptr, nullptr);
    HTHEME menu = host ? OpenThemeFileMenu(&file, host, !light) : nullptr;
    Palette p = NativePalette(light);
    bool ok = menu != nullptr;
    if (menu) {
        // The style's own system colours (its sysmetrics, which become Windows' colours when it's applied)
        // fill in what its menu look doesn't say. Only a full style has a menu class, and only a full
        // style's system colours mean anything: a StartAllBack-only style carries Plain8's light defaults.
        auto sys = [&](int index) {
            COLORREF c = GetThemeSysColor(menu, index);
            return (uint32_t)((GetRValue(c) << 16) | (GetGValue(c) << 8) | GetBValue(c));
        };
        uint32_t sysGround = sys(COLOR_MENU), sysText = sys(COLOR_MENUTEXT);
        bool sysUsable = std::abs(PaletteLuma(sysGround) - PaletteLuma(sysText)) >= 0.25;
        if (sysUsable) {
            p.ground = sysGround;
            p.text = sysText;
            p.edge = sys(COLOR_ACTIVEBORDER);
        }
        p.ground = PartPixel(menu, MENU_POPUPBACKGROUND, 0, 4, 4, p.ground);
        p.edge = PartPixel(menu, MENU_POPUPBORDERS, 0, 0, 0, p.edge);
        uint32_t hot = PartPixel(menu, MENU_POPUPITEM, MPI_HOT, 4, 4, MixRgb(p.ground, p.text, 0.16));
        COLORREF text;
        if (SUCCEEDED(GetThemeColor(menu, MENU_POPUPITEM, MPI_NORMAL, TMT_TEXTCOLOR, &text))) {
            p.text = (GetRValue(text) << 16) | (GetGValue(text) << 8) | GetBValue(text);
        } else if (!sysUsable) {
            p.text = PaletteLuma(p.ground) < 0.5 ? 0xFFFFFF : 0x000000;
        }
        // The theme's own hot item for hovering, unless it hardly shows on
        // the ground; a quieter step of it for a selected row or a pressed
        // button.
        if (std::abs(PaletteLuma(hot) - PaletteLuma(p.ground)) < 0.06) hot = MixRgb(p.ground, p.text, 0.16);
        p.hover = hot;
        p.raised = MixRgb(p.ground, hot, 0.5);
        p.line = PartPixel(menu, MENU_POPUPSEPARATOR, 0, 4, 4, MixRgb(p.ground, p.text, 0.2));
        if (p.line == p.ground) p.line = MixRgb(p.ground, p.text, 0.2);
        p.muted = MixRgb(p.ground, p.text, 0.72);
        p.dim = MixRgb(p.ground, p.text, 0.45);
        // Its own grey text for faint text, when that can be read on the ground.
        if (sysUsable && std::abs(PaletteLuma(sys(COLOR_GRAYTEXT)) - PaletteLuma(p.ground)) >= 0.18)
            p.dim = sys(COLOR_GRAYTEXT);
        // Warnings and errors keep their meaning; only their lightness
        // follows the ground.
        bool dark = PaletteLuma(p.ground) < 0.5;
        p.warn = dark ? 0xF5A97F : 0xB35A00;
        p.bad = dark ? 0xF38BA8 : 0xC42B1C;
        // Its own highlight as the accent, when it stands out from the
        // ground; then the style's Highlight system colour; Windows' accent
        // colour last.
        uint32_t styleAccent = sysUsable ? sys(COLOR_HIGHLIGHT) : WindowsAccent();
        if (std::abs(PaletteLuma(styleAccent) - PaletteLuma(p.ground)) < 0.25) styleAccent = WindowsAccent();
        p.accent = std::abs(PaletteLuma(hot) - PaletteLuma(p.ground)) >= 0.25 ? hot : styleAccent;
        if (std::abs(PaletteLuma(p.accent) - PaletteLuma(p.ground)) < 0.25) p.accent = MixRgb(p.accent, p.text, 0.5);
        p.tourne = false;
        CloseThemeData(menu);
    }
    if (host) DestroyWindow(host);
    CloseThemeFile(&file);

    AcquireSRWLockExclusive(&lock);
    cachedKey = key;
    cachedOk = ok;
    cached = p;
    ReleaseSRWLockExclusive(&lock);
    if (ok) *out = p;
    return ok;
}

// A theme name, as StartAllBack stores it, to its file in StartAllBack's
// Styles folder or Tourne's; a full path stays as it is.
std::wstring ThemeStyleFile(std::wstring const& name) {
    if (name.empty() || name == L"none") return {};
    auto exists = [](std::wstring const& path) {
        DWORD a = GetFileAttributesW(path.c_str());
        return a != INVALID_FILE_ATTRIBUTES && !(a & FILE_ATTRIBUTE_DIRECTORY);
    };
    if (name.find(L'\\') != std::wstring::npos) return exists(name) ? name : L"";
    for (PCWSTR folder : {L"%ProgramFiles%\\StartAllBack\\Styles\\", L"%LOCALAPPDATA%\\Tourne\\Styles\\"}) {
        WCHAR expanded[MAX_PATH * 2];
        if (!ExpandEnvironmentStringsW(folder, expanded, ARRAYSIZE(expanded))) continue;
        std::wstring path = expanded + name;
        if (exists(path)) return path;
    }
    return {};
}

// The taskbar's theme file: its Visual style setting (read by its full name,
// through a variable, so the build doesn't file it under the part reading
// it), else the one StartAllBack is set to. Empty for none.
std::wstring TaskbarThemeFile() {
    constexpr PCWSTR kTaskbarStyleSetting = L"taskbar.style";
    PCWSTR value = Wh_GetStringSetting(kTaskbarStyleSetting);
    std::wstring name = value ? value : L"";
    Wh_FreeStringSetting(value);
    if (name.empty() || name == L"startallback") {
        WCHAR sab[MAX_PATH * 2] = L"";
        DWORD size = sizeof(sab);
        RegGetValueW(HKEY_CURRENT_USER, L"Software\\StartIsBack", L"TaskbarStyle", RRF_RT_REG_SZ, nullptr, sab, &size);
        name = sab;
    }
    return ThemeStyleFile(name);
}

bool IsThemeScheme(std::wstring_view key) {
    return key == L"visualStyle" || key == L"taskbarStyle";
}

// A colour scheme by its key, including the two taken from a theme.
Palette SuiteSchemePalette(std::wstring_view key) {
    if (!IsThemeScheme(key)) return SchemePalette(key);
    bool light = SystemUsesLightTheme();
    Palette p = NativePalette(light);
    if (key == L"taskbarStyle" && PaletteFromThemeFile(TaskbarThemeFile(), light, &p)) return p;
    PaletteFromThemeFile(CurrentThemeFile(), light, &p);
    return p;
}
// Peek, as the taskbar's thumbnails do ("// @inline live-peek.cpp"): every
// other window faded away, the peeked one shown where it is, and the window
// passed as "above" (a preview popup, the switcher) kept over it. DWM's own
// private call (dwmapi ordinal 113), whose fifth argument came with Windows
// 10.

using DwmpActivateLivePreview_t = HRESULT(WINAPI*)(BOOL, HWND, HWND, UINT, UINT_PTR);
[[maybe_unused]] constexpr UINT kPeekTaskbar = 1;
[[maybe_unused]] constexpr UINT kPeekAltTab = 3;

HWND g_peeking;  // the window being peeked at, or null

// Stays in view through any peek, as Windows' own taskbar does.
[[maybe_unused]] void KeepOutOfPeek(HWND wnd) {
    BOOL exclude = TRUE;
    if (wnd) DwmSetWindowAttribute(wnd, DWMWA_EXCLUDED_FROM_PEEK, &exclude, sizeof(exclude));
}

// Peeks at a window, or ends the peek (target null).
[[maybe_unused]] void LivePeek(HWND target, HWND above, UINT type) {
    static auto activate =
        (DwmpActivateLivePreview_t)GetProcAddress(LoadLibraryW(L"dwmapi.dll"), MAKEINTRESOURCEA(113));
    if (!activate || target == g_peeking) return;
    if (!target) {
        g_peeking = nullptr;
        activate(FALSE, nullptr, nullptr, type, 0);
        return;
    }
    g_peeking = target;
    activate(TRUE, target, above, type, 0);
}

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

// A program's icon everywhere ("// @inline program-icons.cpp", in Icon Redirect, System Icons and the
// Switcher): the taskbar's Button icons rows marked "Use everywhere", read by each part that shows
// programs' icons, so one row gives a program the same icon on the taskbar, in the tray, in Alt+Tab and,
// through Icon Redirect, wherever Windows takes the icon from its file. Or one of System Icons' designs
// (tourne:screenshot), on its buttons, in the tray and in Alt+Tab. A row's program is a file name,
// a full path or an app id, and may hold * and ? (like *\NVIDIA Corporation\*.exe).

struct ProgramIcon {
    std::wstring program;  // lower case
    std::wstring icon;     // a file, or "file,index"; variables expanded; or tourne:design
};

// One of System Icons' own designs, by name (tourne:screenshot), in place of a file: drawn by System Icons
// for the tray, the taskbar's buttons and Alt+Tab; Icon Redirect, which needs an .ico, passes over it.
bool IsTourneDesign(std::wstring const& icon) {
    return _wcsnicmp(icon.c_str(), L"tourne:", 7) == 0;
}

// * matches any run of characters, ? any one. Both sides lower case.
bool ProgramWildcard(std::wstring_view pattern, std::wstring_view text) {
    size_t p = 0, t = 0, star = std::wstring_view::npos, resume = 0;
    while (t < text.size()) {
        if (p < pattern.size() && (pattern[p] == L'?' || pattern[p] == text[t])) {
            p++;
            t++;
        } else if (p < pattern.size() && pattern[p] == L'*') {
            star = p++;
            resume = t;
        } else if (star != std::wstring_view::npos) {
            p = star + 1;
            t = ++resume;
        } else {
            return false;
        }
    }
    while (p < pattern.size() && pattern[p] == L'*') {
        p++;
    }
    return p == pattern.size();
}

// A path from a setting: trimmed, without outer quotes or a stray leading *, variables expanded.
std::wstring ProgramIconPath(std::wstring s) {
    auto trim = [&] {
        while (!s.empty() && iswspace(s.front())) s.erase(0, 1);
        while (!s.empty() && iswspace(s.back())) s.pop_back();
    };
    trim();
    if (s.size() >= 2 && s.front() == L'"' && s.back() == L'"') s = s.substr(1, s.size() - 2);
    while (!s.empty() && s.front() == L'*') s.erase(0, 1);
    trim();
    if (s.find(L'%') == std::wstring::npos) return s;
    WCHAR expanded[MAX_PATH * 2];
    DWORD length = ExpandEnvironmentStringsW(s.c_str(), expanded, ARRAYSIZE(expanded));
    return length && length <= ARRAYSIZE(expanded) ? std::wstring(expanded) : s;
}

std::vector<ProgramIcon> EverywhereIcons() {
    // The taskbar part's settings, by their full names (through variables, so the build doesn't file
    // them under the part reading them).
    constexpr PCWSTR kProgram = L"taskbar.buttonIcons[%d].program";
    constexpr PCWSTR kIcon = L"taskbar.buttonIcons[%d].icon";
    constexpr PCWSTR kEverywhere = L"taskbar.buttonIcons[%d].everywhere";
    std::vector<ProgramIcon> rows;
    for (int i = 0; i < 64; i++) {
        PCWSTR program = Wh_GetStringSetting(kProgram, i);
        std::wstring match = program ? program : L"";
        Wh_FreeStringSetting(program);
        while (!match.empty() && iswspace(match.front())) match.erase(0, 1);
        while (!match.empty() && iswspace(match.back())) match.pop_back();
        if (match.empty()) {
            if (i > 0) break;
            continue;
        }
        PCWSTR icon = Wh_GetStringSetting(kIcon, i);
        std::wstring file = ProgramIconPath(icon ? icon : L"");
        Wh_FreeStringSetting(icon);
        // (A design goes on the program's buttons, tray icon and Alt+Tab alike, without "Use everywhere".)
        if (!Wh_GetIntSetting(kEverywhere, i) && !IsTourneDesign(file)) continue;
        CharLowerBuffW(match.data(), (DWORD)match.size());
        if (!file.empty()) rows.push_back({match, file});
    }
    return rows;
}

// Whether a row stands for a program, by its app id, full path or file name (all lower case).
bool ProgramIconMatches(std::wstring const& row, std::wstring const& appId, std::wstring const& path) {
    std::wstring_view name = path;
    if (size_t slash = name.find_last_of(L"\\/"); slash != std::wstring_view::npos) name = name.substr(slash + 1);
    if (row.find_first_of(L"*?") != std::wstring::npos)
        return (!path.empty() && ProgramWildcard(row, path)) || (!name.empty() && ProgramWildcard(row, name)) ||
               (!appId.empty() && ProgramWildcard(row, appId));
    return (!path.empty() && row == path) || (!name.empty() && row == name) || (!appId.empty() && row == appId);
}

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
    bool backtick = false;         // Alt+` is taken at all
    bool backtickSameApp = false;
    bool wheelOpens = false, wheelPages = false, wheelModifierCtrl = false, reverseWheel = false;
    bool allDesktops = true, thisMonitorOnly = false;
    Minimized minimized = Minimized::InPlace;
    std::wstring excludeTitles, excludeApps;
    std::vector<CustomHeader> headers;
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
constexpr UINT_PTR kTimerShow = 1;
constexpr UINT_PTR kTimerPeek = 2;
// STEP's lParam: LOWORD what pressed it, HIWORD flags.
constexpr WORD kFromTab = 0, kFromBacktick = 1, kFromWheel = 2;
constexpr WORD kFlagSticky = 1, kFlagAllMonitors = 2;
constexpr WCHAR kWindowClass[] = L"TourneSwitcher_" WH_MOD_ID;
constexpr WCHAR kOverlayClass[] = L"TourneSwitcherOverlay_" WH_MOD_ID;
// An unassigned key, pressed while switching: Alt let go then isn't a lone
// Alt tap (which opens the active app's menu bar), and a process that sent
// the last input may set the foreground window.
constexpr WORD kDummyKey = 0xE8;

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

// A program's icon from the taskbar's Button icons marked "Use everywhere",
// matched by its app id, path or file name; the caller destroys it.
HICON EverywhereIcon(HWND h, std::wstring const& program) {
    if (g_settings.programIcons.empty()) return nullptr;
    std::wstring path = Lower(program), id = Lower(AppIdOf(h));
    for (auto const& row : g_settings.programIcons)
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

// Peeks at the selection once it has rested there a moment, or at once when
// already peeking.
HWND g_peekPending;  // the selection the peek timer is for

void SchedulePeek(HWND wnd) {
    if (!g_settings.peek || g_sel < 0 || g_sel >= (int)g_entries.size() || g_entries[g_sel].windows.empty()) return;
    HWND target = g_entries[g_sel].windows[0];
    if (g_peeking) {
        LivePeek(target, wnd, kPeekAltTab);
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
    if (IsIconic(h)) ShowWindow(h, SW_RESTORE);
    PressDummyKey();
    if (!SetForegroundWindow(h)) SwitchToThisWindow(h, TRUE);
}

LRESULT CALLBACK MouseProc(int code, WPARAM wParam, LPARAM lParam);

// The wheel hook, on the hook thread, only while it's wanted.
void WatchWheel(bool on) {
    if (g_hookThreadId) PostThreadMessageW(g_hookThreadId, WM_APP_WATCHWHEEL, on || g_settings.wheelOpens, 0);
}

void Hide(HWND wnd) {
    KillTimer(wnd, kTimerShow);
    KillTimer(wnd, kTimerPeek);
    LivePeek(nullptr, nullptr, kPeekAltTab);
    g_peekPending = nullptr;
    g_open = g_shown = g_sticky = false;
    g_hover = -1;
    ShowWindow(wnd, SW_HIDE);
    if (g_overlay) ShowWindow(g_overlay, SW_HIDE);
    ClearEntries(g_entries);
    ClearEntries(g_outer);
    WatchWheel(false);
}

void Refit(HWND wnd) {
    if (!g_shown) return;
    Arrange(wnd);
    UpdateThumbnails(wnd);
    Render(wnd);
}

void Show(HWND wnd) {
    KillTimer(wnd, kTimerShow);
    if (!g_open || g_shown) return;
    g_shown = true;
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
    g_sticky = sticky;
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

////////////////////////////////////////////////////////////////////////////////
// Window tiling, in the manner of komorebi ("// @inline tiling.cpp", in the
// Switcher, whose keyboard hook it shares): the windows on a screen laid out
// by a layout, with gaps around and between them, clear of the taskbar. By
// hand with chords on a key the user picks (the tiling key, none until set),
// or automatically as windows come and go. Each screen keeps the layout
// picked for it on each of Windows' desktops, across restarts too.
//
// Windows are placed by their visible frames (DWM's extended frame bounds),
// so the gaps are what the eye sees, not the invisible resize borders.

enum class TileLayout {
    Off,
    Same,  // other screens: as the main screen
    Columns,
    Rows,
    VerticalStack,
    RightMainVerticalStack,
    HorizontalStack,
    UltrawideVerticalStack,
    Grid,
    Bsp,
    Monocle,
};

constexpr TileLayout kLayoutCycle[] = {
    TileLayout::Columns,        TileLayout::Rows,
    TileLayout::VerticalStack,  TileLayout::RightMainVerticalStack,
    TileLayout::HorizontalStack, TileLayout::UltrawideVerticalStack,
    TileLayout::Grid,           TileLayout::Bsp,
    TileLayout::Monocle,
};

struct TilingSettings {
    UINT leaderMods = 0, leaderVk = 0;  // the tiling key (MOD_ flags and a key); none when 0
    bool autoTile = false;
    TileLayout layout = TileLayout::Columns;
    TileLayout otherLayout = TileLayout::Same;
    int ratio = 50;
    int gapTop = 4, gapBottom = 2, gapLeft = 6, gapRight = 6, gapInner = 6;
    bool reserveTaskbar = true;
    bool restoreMaximized = true;
    bool tileElevated = false;
    bool keepResized = true;
    bool rememberAll = true;
    int fit = 0;  // kFitForce, kFitOverlap or kFitFloat
    int minWidth = 0, minHeight = 0;
    std::wstring floatApps, floatTitles, floatClasses;
    int focusBorder = 0;        // kBorderNone, kBorderTiled or kBorderAll
    bool focusAccent = true;    // Windows' accent colour, or focusColor
    COLORREF focusColor = 0;
};

enum { kBorderNone, kBorderTiled, kBorderAll };

// A window that won't shrink to its tile (its own minimum size): pushed onto
// it anyway, as dwm with resizehints = 0 does; left at its own size over its
// neighbours; or left floating and the rest tiled without it.
enum { kFitForce, kFitOverlap, kFitFloat };

// Tiling's helper in programs running as administrator (see the suite's
// suite_tile_agent, which these must match).
constexpr WCHAR kTileAgentClass[] = L"TourneTileAgent_" WH_MOD_ID;
constexpr ULONG_PTR kTilePlaceCopyId = 0x54504C43;  // "TPLC"
struct TilePlaceRequest {
    uint64_t hwnd;
    RECT frame;
    int force;
};

TilingSettings g_tiling;
std::atomic<UINT> g_leaderVk, g_leaderMods;  // read by the hook thread

// What the chords ask for (WM_APP_TILE's wParam; lParam the direction or step).
enum TileCommand { kTileNow, kTileMove, kTileFocus, kTileFloat, kTileLayout, kTileRatio, kTileAuto, kTileLayouts, kTileApplyLayout };
enum TileDirection { kLeft, kRight, kUp, kDown };
constexpr UINT WM_APP_TILE = WM_APP + 12;
constexpr UINT_PTR kTimerTile = 3;

// The running state, on the switcher's thread.
std::vector<HWND> g_tileOrder;  // every tiled window, in its order
std::vector<HWND> g_floating;   // set floating with the tiling key and Space
bool g_autoTile = false;
bool g_userMoving = false;
HWND g_moving;                  // the window being dragged or resized by hand
RECT g_movingFrame;             // and its frame when that began
HWINEVENTHOOK g_tileHooks[4];

// Windows resized by hand: left floating where they were put, and the place
// remembered for the program (its file name and window class), so its next
// window opens there too. One window at a time holds a program's place.
struct KeptPlace {
    std::wstring key;
    RECT frame;
};
std::vector<KeptPlace> g_keptPlaces;                      // saved, newest last
std::vector<std::pair<HWND, std::wstring>> g_keptWindows;  // the windows holding them
constexpr size_t kMostKeptPlaces = 64;
constexpr WCHAR kKeptPlacesValue[] = L"switcher.tiling.keptPlaces";

// Each screen's layout and main window's share as picked (the tiling key
// and N, [ or ], or a layout applied), one for each of Windows' desktops,
// kept across restarts. A screen is known by its monitor's device name, so
// it keeps its choice when the screens are rearranged.
struct LayoutChoice {
    std::wstring key;                      // the screen's and the desktop's names, as "screen|desktop"
    TileLayout layout = TileLayout::Same;  // Same: none picked, the settings' layout
    int ratioStep = 0;                     // added to the settings' share
};
std::vector<LayoutChoice> g_layoutChoices;  // newest last
std::vector<std::pair<std::wstring, std::wstring>> g_screenKeys;  // names found, by the screen's GDI name
constexpr size_t kMostLayoutChoices = 64;
constexpr WCHAR kLayoutChoicesValue[] = L"switcher.tiling.choices";

// The layouts window and the places (tiling-layouts.cpp).
void OpenLayoutsWindow();
void ApplyLayoutNumber(int index);
PCWSTR LayoutName(TileLayout layout);
bool Placeable(HWND h);

TileLayout ParseLayout(std::wstring const& v, TileLayout fallback) {
    if (v == L"off") return TileLayout::Off;
    if (v == L"same") return TileLayout::Same;
    if (v == L"columns") return TileLayout::Columns;
    if (v == L"rows") return TileLayout::Rows;
    if (v == L"verticalStack") return TileLayout::VerticalStack;
    if (v == L"rightMainVerticalStack") return TileLayout::RightMainVerticalStack;
    if (v == L"horizontalStack") return TileLayout::HorizontalStack;
    if (v == L"ultrawideVerticalStack") return TileLayout::UltrawideVerticalStack;
    if (v == L"grid") return TileLayout::Grid;
    if (v == L"bsp") return TileLayout::Bsp;
    if (v == L"monocle") return TileLayout::Monocle;
    return fallback;
}

// "alt+win+84": modifiers, then the virtual-key code in decimal (Windhawk's
// hotkey setting, as the taskbar reads its own).
void ParseHotkey(std::wstring const& key, UINT* mods, UINT* vk) {
    *mods = *vk = 0;
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
    // A modifier on its own isn't a key to hold.
    switch (*vk) {
        case VK_SHIFT: case VK_LSHIFT: case VK_RSHIFT: case VK_CONTROL: case VK_LCONTROL: case VK_RCONTROL:
        case VK_MENU: case VK_LMENU: case VK_RMENU: case VK_LWIN: case VK_RWIN:
            *vk = 0;
    }
    if (*vk > 0xFE) *vk = 0;
}

// "#RRGGBB" (or without the #). False if it isn't one.
bool ParseColor(std::wstring v, COLORREF* out) {
    if (!v.empty() && v[0] == L'#') v.erase(0, 1);
    if (v.size() != 6 || v.find_first_not_of(L"0123456789abcdefABCDEF") != std::wstring::npos) return false;
    unsigned long rgb = wcstoul(v.c_str(), nullptr, 16);
    *out = RGB((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
    return true;
}

void LoadLayoutChoices();

void LoadTilingSettings() {
    TilingSettings s;
    ParseHotkey(TextSetting(L"tiling.leader"), &s.leaderMods, &s.leaderVk);
    s.autoTile = Wh_GetIntSetting(L"tiling.auto") != 0;
    s.layout = ParseLayout(TextSetting(L"tiling.layout"), TileLayout::Columns);
    if (s.layout == TileLayout::Same || s.layout == TileLayout::Off) s.layout = TileLayout::Columns;
    s.otherLayout = ParseLayout(TextSetting(L"tiling.otherLayout"), TileLayout::Same);
    int ratio = Wh_GetIntSetting(L"tiling.mainRatio");
    s.ratio = ratio > 0 ? std::clamp(ratio, 10, 90) : 50;
    s.gapTop = IntSetting(L"tiling.gapTop", 0, 400);
    s.gapBottom = IntSetting(L"tiling.gapBottom", 0, 400);
    s.gapLeft = IntSetting(L"tiling.gapLeft", 0, 400);
    s.gapRight = IntSetting(L"tiling.gapRight", 0, 400);
    s.gapInner = IntSetting(L"tiling.gapInner", 0, 400);
    s.reserveTaskbar = Wh_GetIntSetting(L"tiling.reserveTaskbar") != 0;
    s.restoreMaximized = Wh_GetIntSetting(L"tiling.restoreMaximized") != 0;
    s.floatApps = TextSetting(L"tiling.floatApps");
    s.floatTitles = TextSetting(L"tiling.floatTitles");
    s.floatClasses = TextSetting(L"tiling.floatClasses");
    s.tileElevated = Wh_GetIntSetting(L"tiling.tileElevated") != 0;
    s.keepResized = Wh_GetIntSetting(L"tiling.keepResized") != 0;
    s.rememberAll = Wh_GetIntSetting(L"tiling.rememberAll") != 0;
    std::wstring fit = TextSetting(L"tiling.tooSmall");
    s.fit = fit == L"overlap" ? kFitOverlap : fit == L"float" ? kFitFloat : kFitForce;
    s.minWidth = IntSetting(L"tiling.minWidth", 0, 4000);
    s.minHeight = IntSetting(L"tiling.minHeight", 0, 4000);
    std::wstring border = TextSetting(L"tiling.focusBorder");
    s.focusBorder = border == L"tiled" ? kBorderTiled : border == L"all" ? kBorderAll : kBorderNone;
    s.focusAccent = TextSetting(L"tiling.focusColors") != L"custom" || !ParseColor(TextSetting(L"tiling.focusColor"), &s.focusColor);
    g_tiling = s;
    g_leaderVk = s.leaderVk;
    g_leaderMods = s.leaderMods;
    g_autoTile = s.autoTile;
    g_screenKeys.clear();
    LoadLayoutChoices();
}

////////////////////////////////////////////////////////////////////////////////
// Which windows, and where

RECT FrameOf(HWND h) {
    RECT r{};
    if (FAILED(DwmGetWindowAttribute(h, DWMWA_EXTENDED_FRAME_BOUNDS, &r, sizeof(r)))) GetWindowRect(h, &r);
    return r;
}

bool IsFloating(HWND h) {
    return std::find(g_floating.begin(), g_floating.end(), h) != g_floating.end();
}

// A window the tiling looks after: an app's own resizable top-level window,
// in view, not set floating.
bool Tileable(HWND h) {
    if (!IsWindowVisible(h) || IsIconic(h) || GetWindow(h, GW_OWNER)) return false;
    LONG style = GetWindowLongW(h, GWL_STYLE), ex = GetWindowLongW(h, GWL_EXSTYLE);
    if ((style & WS_CHILD) || !(style & WS_THICKFRAME) || (ex & (WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE))) return false;
    if (IsZoomed(h) && !g_tiling.restoreMaximized) return false;
    DWORD cloaked = 0;
    if (SUCCEEDED(DwmGetWindowAttribute(h, DWMWA_CLOAKED, &cloaked, sizeof(cloaked))) && cloaked) return false;
    DWORD pid = 0;
    GetWindowThreadProcessId(h, &pid);
    if (pid == GetCurrentProcessId()) return false;
    WCHAR cls[64] = L"";
    GetClassNameW(h, cls, ARRAYSIZE(cls));
    if (!wcsncmp(cls, L"Tourne", 6)) return false;
    if (IsFloating(h) || IsHungAppWindow(h)) return false;
    if (!g_tiling.floatApps.empty()) {
        std::wstring program = ProgramOf(h);
        if (MatchesList(g_tiling.floatApps, program.substr(program.find_last_of(L'\\') + 1))) return false;
    }
    if (!g_tiling.floatTitles.empty() && MatchesList(g_tiling.floatTitles, Title(h))) return false;
    if (!g_tiling.floatClasses.empty() && MatchesList(g_tiling.floatClasses, cls)) return false;
    // Smaller than the thresholds (as komorebi's minimum_window_width and
    // _height): a little window of its own, not one to tile.
    if (g_tiling.minWidth || g_tiling.minHeight) {
        RECT r = FrameOf(h);
        UINT dpi = GetDpiForWindow(h);
        if (r.right - r.left < MulDiv(g_tiling.minWidth, dpi ? dpi : 96, 96) ||
            r.bottom - r.top < MulDiv(g_tiling.minHeight, dpi ? dpi : 96, 96))
            return false;
    }
    return true;
}

UINT MonitorDpi(HMONITOR monitor) {
    using GetDpiForMonitor_t = HRESULT(WINAPI*)(HMONITOR, int, UINT*, UINT*);
    static auto getDpi = (GetDpiForMonitor_t)GetProcAddress(LoadLibraryW(L"shcore.dll"), "GetDpiForMonitor");
    UINT x = 96, y = 96;
    if (getDpi) getDpi(monitor, 0 /* MDT_EFFECTIVE_DPI */, &x, &y);
    return x ? x : 96;
}

// The space the windows share on a screen: its work area, clear of the
// suite's taskbar where that doesn't reserve its space (hiding), less the
// outer gaps.
RECT TileArea(HMONITOR monitor) {
    MONITORINFO mi{sizeof(mi)};
    GetMonitorInfoW(monitor, &mi);
    RECT area = mi.rcWork;
    UINT dpi = MonitorDpi(monitor);
    auto scaled = [dpi](int v) { return MulDiv(v, dpi, 96); };
    // The taskbar's own settings (through variables, so the build doesn't
    // file them under this part). It sits on the main screen.
    constexpr PCWSTR kTaskbarEnabled = L"taskbar.enabled", kTaskbarThickness = L"taskbar.thickness";
    constexpr PCWSTR kTaskbarLocation = L"taskbar.location";
    if (g_tiling.reserveTaskbar && (mi.dwFlags & MONITORINFOF_PRIMARY) && Wh_GetIntSetting(kTaskbarEnabled)) {
        int thickness = Wh_GetIntSetting(kTaskbarThickness);
        thickness = scaled(thickness <= 0 ? 40 : std::clamp(thickness, 20, 96));
        PCWSTR value = Wh_GetStringSetting(kTaskbarLocation);
        std::wstring location = value ? value : L"";
        Wh_FreeStringSetting(value);
        RECT const& m = mi.rcMonitor;
        if (location == L"top") {
            if (area.top == m.top) area.top += thickness;
        } else if (location == L"left") {
            if (area.left == m.left) area.left += thickness;
        } else if (location == L"right") {
            if (area.right == m.right) area.right -= thickness;
        } else if (area.bottom == m.bottom) {
            area.bottom -= thickness;
        }
    }
    area.left += scaled(g_tiling.gapLeft);
    area.top += scaled(g_tiling.gapTop);
    area.right -= scaled(g_tiling.gapRight);
    area.bottom -= scaled(g_tiling.gapBottom);
    return area;
}

// A screen's lasting name: its monitor's device path (which names the
// monitor and the port it's on), or its GDI name if there's none.
std::wstring ScreenKey(HMONITOR monitor) {
    MONITORINFOEXW mi{};
    mi.cbSize = sizeof(mi);
    if (!GetMonitorInfoW(monitor, (MONITORINFO*)&mi)) return L"";
    for (auto const& [gdi, key] : g_screenKeys)
        if (gdi == mi.szDevice) return key;
    DISPLAY_DEVICEW dd{};
    dd.cb = sizeof(dd);
    std::wstring key = EnumDisplayDevicesW(mi.szDevice, 0, &dd, EDD_GET_DEVICE_INTERFACE_NAME) && dd.DeviceID[0]
                           ? dd.DeviceID
                           : mi.szDevice;
    CharLowerBuffW(key.data(), (DWORD)key.size());
    std::replace_if(key.begin(), key.end(), [](WCHAR c) { return c == L';' || c == L'=' || c == L'|' || c == L','; }, L'_');
    g_screenKeys.push_back({mi.szDevice, key});
    return key;
}

// The desktop showing (Windows' own, of Win+Tab), as its id; empty while
// there has only ever been one.
std::wstring DesktopKey() {
    GUID id{};
    DWORD size = sizeof(id);
    if (RegGetValueW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\VirtualDesktops",
                     L"CurrentVirtualDesktop", RRF_RT_REG_BINARY, nullptr, &id, &size) != ERROR_SUCCESS ||
        size != sizeof(id))
        return L"";
    WCHAR text[40] = L"";
    StringFromGUID2(id, text, ARRAYSIZE(text));
    return text;
}

std::wstring ChoiceKey(HMONITOR monitor) {
    return ScreenKey(monitor) + L"|" + DesktopKey();
}

LayoutChoice const* FindLayoutChoice(HMONITOR monitor) {
    std::wstring key = ChoiceKey(monitor);
    for (auto const& c : g_layoutChoices)
        if (c.key == key) return &c;
    return nullptr;
}

// What the choices were made against: when the settings' layouts or share
// change, the choices are let go, so the new settings show.
std::wstring ChoicesMadeWith() {
    return std::wstring(LayoutName(g_tiling.layout)) + L"," +
           (g_tiling.otherLayout == TileLayout::Same ? L"same" : LayoutName(g_tiling.otherLayout)) + L"," +
           std::to_wstring(g_tiling.ratio);
}

void SaveLayoutChoices() {
    if (g_layoutChoices.size() > kMostLayoutChoices)
        g_layoutChoices.erase(g_layoutChoices.begin(), g_layoutChoices.end() - kMostLayoutChoices);
    std::wstring text = L"~settings=" + ChoicesMadeWith() + L";";
    for (auto const& c : g_layoutChoices)
        text += c.key + L"=" + (c.layout == TileLayout::Same ? L"same" : LayoutName(c.layout)) + L"," +
                std::to_wstring(c.ratioStep) + L";";
    Wh_SetStringValue(kLayoutChoicesValue, text.c_str());
}

void LoadLayoutChoices() {
    g_layoutChoices.clear();
    std::wstring text(16384, L'\0');
    Wh_GetStringValue(kLayoutChoicesValue, text.data(), text.size());
    text.resize(wcslen(text.c_str()));
    bool current = false;
    std::wstring_view rest = text;
    while (!rest.empty()) {
        size_t end = rest.find(L';');
        std::wstring item(rest.substr(0, end));
        rest = end == std::wstring_view::npos ? std::wstring_view() : rest.substr(end + 1);
        size_t eq = item.rfind(L'=');
        if (eq == std::wstring::npos) continue;
        std::wstring key = item.substr(0, eq), value = item.substr(eq + 1);
        if (key == L"~settings") {
            current = value == ChoicesMadeWith();
            continue;
        }
        size_t comma = value.find(L',');
        LayoutChoice c{key, ParseLayout(value.substr(0, comma), TileLayout::Same)};
        if (comma != std::wstring::npos) c.ratioStep = std::clamp(_wtoi(value.c_str() + comma + 1), -80, 80);
        g_layoutChoices.push_back(c);
    }
    if (!current && !g_layoutChoices.empty()) {
        g_layoutChoices.clear();
        SaveLayoutChoices();
    }
}

// This screen's choice on the desktop showing, made if there's none yet,
// and moved to the newest.
LayoutChoice& ChoiceFor(HMONITOR monitor) {
    std::wstring key = ChoiceKey(monitor);
    LayoutChoice c{key};
    auto it = std::find_if(g_layoutChoices.begin(), g_layoutChoices.end(), [&](LayoutChoice const& x) { return x.key == key; });
    if (it != g_layoutChoices.end()) {
        c = *it;
        g_layoutChoices.erase(it);
    }
    g_layoutChoices.push_back(c);
    return g_layoutChoices.back();
}

void SetLayoutChoice(HMONITOR monitor, TileLayout layout) {
    ChoiceFor(monitor).layout = layout;
    SaveLayoutChoices();
}

// The main window's share (percent) on a screen.
int RatioFor(HMONITOR monitor) {
    LayoutChoice const* c = FindLayoutChoice(monitor);
    return std::clamp(g_tiling.ratio + (c ? c->ratioStep : 0), 10, 90);
}

TileLayout LayoutFor(HMONITOR monitor) {
    if (LayoutChoice const* c = FindLayoutChoice(monitor); c && c->layout != TileLayout::Same) return c->layout;
    MONITORINFO mi{sizeof(mi)};
    GetMonitorInfoW(monitor, &mi);
    if (mi.dwFlags & MONITORINFOF_PRIMARY) return g_tiling.layout;
    return g_tiling.otherLayout == TileLayout::Same ? g_tiling.layout : g_tiling.otherLayout;
}

////////////////////////////////////////////////////////////////////////////////
// Layouts

// A length cut into pieces with gaps between; the odd pixels go to the first.
std::vector<std::pair<int, int>> Cut(int start, int length, int count, int gap) {
    std::vector<std::pair<int, int>> pieces;
    int room = std::max(0, length - gap * (count - 1));
    int each = room / count, extra = room % count;
    for (int i = 0, at = start; i < count; i++) {
        int size = each + (i < extra ? 1 : 0);
        pieces.push_back({at, at + size});
        at += size + gap;
    }
    return pieces;
}

// A length cut in two by a share (percent) of it for the first part.
std::pair<std::pair<int, int>, std::pair<int, int>> CutShare(int start, int length, int percent, int gap) {
    int room = std::max(0, length - gap);
    int first = room * percent / 100;
    return {{start, start + first}, {start + first + gap, start + length}};
}

void Bsp(RECT area, int count, bool across, int percent, int gap, std::vector<RECT>* out) {
    if (count == 1) {
        out->push_back(area);
        return;
    }
    RECT a = area, b = area;
    if (across) {
        auto [l, r] = CutShare(area.left, area.right - area.left, percent, gap);
        a.left = l.first, a.right = l.second, b.left = r.first, b.right = r.second;
    } else {
        auto [t, u] = CutShare(area.top, area.bottom - area.top, percent, gap);
        a.top = t.first, a.bottom = t.second, b.top = u.first, b.bottom = u.second;
    }
    out->push_back(a);
    Bsp(b, count - 1, !across, 50, gap, out);
}

std::vector<RECT> LayoutRects(TileLayout layout, RECT area, int n, int gap, int percent) {
    std::vector<RECT> rects;
    if (n <= 0) return rects;
    int w = area.right - area.left, h = area.bottom - area.top;
    auto column = [&](int left, int right, int count) {
        for (auto [t, b] : Cut(area.top, h, count, gap)) rects.push_back({left, t, right, b});
    };
    auto row = [&](int top, int bottom, int count) {
        for (auto [l, r] : Cut(area.left, w, count, gap)) rects.push_back({l, top, r, bottom});
    };
    if (n == 1 || layout == TileLayout::Monocle) {
        rects.assign(n, area);
        return rects;
    }
    switch (layout) {
        case TileLayout::Rows:
            column(area.left, area.right, n);
            break;
        case TileLayout::VerticalStack: {
            auto [main, stack] = CutShare(area.left, w, percent, gap);
            rects.push_back({main.first, area.top, main.second, area.bottom});
            column(stack.first, stack.second, n - 1);
            break;
        }
        case TileLayout::RightMainVerticalStack: {
            auto [stack, main] = CutShare(area.left, w, 100 - percent, gap);
            rects.push_back({main.first, area.top, main.second, area.bottom});
            column(stack.first, stack.second, n - 1);
            break;
        }
        case TileLayout::HorizontalStack: {
            auto [main, stack] = CutShare(area.top, h, percent, gap);
            rects.push_back({area.left, main.first, area.right, main.second});
            row(stack.first, stack.second, n - 1);
            break;
        }
        case TileLayout::UltrawideVerticalStack: {
            if (n == 2) return LayoutRects(TileLayout::VerticalStack, area, n, gap, percent);
            // The main window in the middle, one to its left, the rest stacked
            // to its right.
            int mainW = std::max(0, w - 2 * gap) * percent / 100;
            int side = std::max(0, w - 2 * gap - mainW) / 2;
            int mainLeft = area.left + side + gap;
            rects.push_back({mainLeft, area.top, mainLeft + mainW, area.bottom});
            rects.push_back({area.left, area.top, area.left + side, area.bottom});
            column(mainLeft + mainW + gap, area.right, n - 2);
            break;
        }
        case TileLayout::Grid: {
            int columns = (int)std::ceil(std::sqrt((double)n));
            auto spans = Cut(area.left, w, columns, gap);
            for (int c = 0; c < columns; c++) {
                int count = n / columns + (c < n % columns ? 1 : 0);
                if (count) column(spans[c].first, spans[c].second, count);
            }
            break;
        }
        case TileLayout::Bsp:
            Bsp(area, n, true, percent, gap, &rects);
            break;
        default:  // Columns
            row(area.top, area.bottom, n);
            break;
    }
    return rects;
}

////////////////////////////////////////////////////////////////////////////////
// Tiling

// A program running as administrator places its own window, through the
// suite's helper in it. 1: placed; 0: it wouldn't shrink; -1: no helper.
int PlaceThroughAgent(HWND h, RECT const& want, bool force) {
    if (!g_tiling.tileElevated) return -1;
    DWORD pid = 0;
    GetWindowThreadProcessId(h, &pid);
    for (HWND agent = nullptr; (agent = FindWindowExW(HWND_MESSAGE, agent, kTileAgentClass, nullptr));) {
        DWORD agentPid = 0;
        if (!GetWindowThreadProcessId(agent, &agentPid) || agentPid != pid) continue;
        TilePlaceRequest request{(uint64_t)(ULONG_PTR)h, want, force ? 1 : 0};
        COPYDATASTRUCT data{kTilePlaceCopyId, sizeof(request), &request};
        DWORD_PTR result = 0;
        if (!SendMessageTimeoutW(agent, WM_COPYDATA, 0, (LPARAM)&data, SMTO_ABORTIFHUNG, 500, &result) || !result)
            return -1;
        return result == 2 ? 1 : 0;
    }
    return -1;
}

// Puts a window's visible frame on a rect. False: it won't shrink to it and
// is to be left floating.
bool PlaceFrame(HWND h, RECT const& want) {
    bool force = g_tiling.fit == kFitForce;
    if (IsZoomed(h)) ShowWindow(h, SW_SHOWNOACTIVATE);
    RECT window{}, frame = FrameOf(h);
    GetWindowRect(h, &window);
    int il = frame.left - window.left, it = frame.top - window.top;
    int ir = window.right - frame.right, ib = window.bottom - frame.bottom;
    int x = want.left - il, y = want.top - it;
    int cx = (want.right - want.left) + il + ir, cy = (want.bottom - want.top) + it + ib;
    UINT flags = SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE;
    if (!SetWindowPos(h, nullptr, x, y, cx, cy, flags)) {
        // Refused outright: a program running as administrator.
        if (GetLastError() == ERROR_ACCESS_DENIED) {
            int placed = PlaceThroughAgent(h, want, force);
            return placed != 0 || g_tiling.fit != kFitFloat;
        }
        return true;
    }
    RECT now = FrameOf(h);
    if (EqualRect(&now, &want)) return true;
    // Bigger than asked: its own minimum size, or the size it was snapped at.
    bool bigger = now.right - now.left > want.right - want.left || now.bottom - now.top > want.bottom - want.top;
    if (bigger && g_tiling.fit == kFitFloat) return false;
    if (bigger && g_tiling.fit == kFitOverlap) return true;
    // Placed again without being asked.
    SetWindowPos(h, nullptr, x, y, cx, cy, flags | SWP_NOSENDCHANGING);
    return true;
}

struct TileScan {
    std::vector<std::pair<HWND, HMONITOR>> windows;
};

std::vector<std::pair<HWND, HMONITOR>> ScanTileable() {
    TileScan scan;
    EnumWindows(
        [](HWND h, LPARAM param) __stdcall -> BOOL {
            if (Tileable(h))
                ((TileScan*)param)->windows.push_back({h, MonitorFromWindow(h, MONITOR_DEFAULTTONEAREST)});
            return TRUE;
        },
        (LPARAM)&scan);
    return scan.windows;
}

// A screen's tiled windows in their order: the known ones as they were, new
// ones after them by where they are (or all by where they are, when asked).
std::vector<HWND> WindowsOn(HMONITOR monitor, bool byPosition) {
    std::erase_if(g_tileOrder, [](HWND h) { return !IsWindow(h); });
    std::vector<HWND> here, fresh;
    for (auto const& [h, m] : ScanTileable())
        if (m == monitor) here.push_back(h);
    std::vector<HWND> ordered;
    if (!byPosition)
        for (HWND h : g_tileOrder)
            if (std::find(here.begin(), here.end(), h) != here.end()) ordered.push_back(h);
    for (HWND h : here)
        if (std::find(ordered.begin(), ordered.end(), h) == ordered.end()) fresh.push_back(h);
    bool vertical = LayoutFor(monitor) == TileLayout::Rows;
    std::sort(fresh.begin(), fresh.end(), [vertical](HWND a, HWND b) {
        RECT ra = FrameOf(a), rb = FrameOf(b);
        int ax = (ra.left + ra.right) / 2, ay = (ra.top + ra.bottom) / 2;
        int bx = (rb.left + rb.right) / 2, by = (rb.top + rb.bottom) / 2;
        return vertical ? std::tie(ay, ax) < std::tie(by, bx) : std::tie(ax, ay) < std::tie(bx, by);
    });
    ordered.insert(ordered.end(), fresh.begin(), fresh.end());
    // The order kept: this screen's, after the others'.
    std::erase_if(g_tileOrder, [&](HWND h) { return std::find(ordered.begin(), ordered.end(), h) != ordered.end(); });
    g_tileOrder.insert(g_tileOrder.end(), ordered.begin(), ordered.end());
    return ordered;
}

std::vector<RECT> RectsFor(HMONITOR monitor, int n) {
    UINT dpi = MonitorDpi(monitor);
    return LayoutRects(LayoutFor(monitor), TileArea(monitor), n, MulDiv(g_tiling.gapInner, dpi, 96), RatioFor(monitor));
}

////////////////////////////////////////////////////////////////////////////////
// Places kept for windows resized by hand

void LoadKeptPlaces() {
    g_keptPlaces.clear();
    std::wstring text(16384, L'\0');
    Wh_GetStringValue(kKeptPlacesValue, text.data(), text.size());
    text.resize(wcslen(text.c_str()));
    std::wstring_view rest = text;
    while (!rest.empty()) {
        size_t end = rest.find(L';');
        std::wstring_view item = rest.substr(0, end);
        size_t eq = item.rfind(L'=');
        RECT r;
        if (eq != std::wstring_view::npos &&
            swscanf(std::wstring(item.substr(eq + 1)).c_str(), L"%ld,%ld,%ld,%ld", &r.left, &r.top, &r.right, &r.bottom) == 4)
            g_keptPlaces.push_back({std::wstring(item.substr(0, eq)), r});
        rest = end == std::wstring_view::npos ? std::wstring_view() : rest.substr(end + 1);
    }
}

void SaveKeptPlaces() {
    if (g_keptPlaces.size() > kMostKeptPlaces)
        g_keptPlaces.erase(g_keptPlaces.begin(), g_keptPlaces.end() - kMostKeptPlaces);
    std::wstring text;
    for (auto const& [key, r] : g_keptPlaces)
        text += key + L"=" + std::to_wstring(r.left) + L"," + std::to_wstring(r.top) + L"," + std::to_wstring(r.right) +
                L"," + std::to_wstring(r.bottom) + L";";
    Wh_SetStringValue(kKeptPlacesValue, text.c_str());
}

// A program's key: its file name and the window's class, without the ; and =
// the saved list uses.
std::wstring KeptKey(HWND h) {
    std::wstring program = ProgramOf(h);
    WCHAR cls[128] = L"";
    GetClassNameW(h, cls, ARRAYSIZE(cls));
    std::wstring key = program.substr(program.find_last_of(L'\\') + 1) + L"|" + cls;
    CharLowerBuffW(key.data(), (DWORD)key.size());
    std::replace_if(key.begin(), key.end(), [](WCHAR c) { return c == L';' || c == L'='; }, L'_');
    return key;
}

bool IsKept(HWND h) {
    return std::any_of(g_keptWindows.begin(), g_keptWindows.end(), [h](auto const& w) { return w.first == h; });
}

// A window left floating where it is, its program's place kept.
void KeepPlace(HWND h) {
    std::wstring key = KeptKey(h);
    if (!IsFloating(h)) g_floating.push_back(h);
    std::erase_if(g_keptWindows, [&](auto const& w) { return w.first == h || w.second == key || !IsWindow(w.first); });
    g_keptWindows.push_back({h, key});
    std::erase_if(g_keptPlaces, [&](KeptPlace const& p) { return p.key == key; });
    g_keptPlaces.push_back({key, FrameOf(h)});
    SaveKeptPlaces();
}

// Put back in the tiling (the tiling key and Space): its program's place is
// let go too.
void ForgetPlace(HWND h) {
    auto it = std::find_if(g_keptWindows.begin(), g_keptWindows.end(), [h](auto const& w) { return w.first == h; });
    if (it == g_keptWindows.end()) return;
    std::wstring key = it->second;
    g_keptWindows.erase(it);
    std::erase_if(g_keptPlaces, [&](KeptPlace const& p) { return p.key == key; });
    SaveKeptPlaces();
}

// The end of a drag or resize by hand. A tiled window that was resized, or
// one already kept, stays where it was put. True when it did.
bool MovedByHand(HWND h) {
    RECT now = FrameOf(h);
    bool resized = now.right - now.left != g_movingFrame.right - g_movingFrame.left ||
                   now.bottom - now.top != g_movingFrame.bottom - g_movingFrame.top;
    bool tiled = std::find(g_tileOrder.begin(), g_tileOrder.end(), h) != g_tileOrder.end() && !IsFloating(h);
    if (!IsKept(h) && !(resized && tiled)) return false;
    KeepPlace(h);
    return true;
}

// Windows not seen before whose program has a kept place (and no window
// holding it): put there, floating.
void PlaceKeptWindows() {
    if (!g_tiling.keepResized || g_keptPlaces.empty()) return;
    std::erase_if(g_keptWindows, [](auto const& w) { return !IsWindow(w.first); });
    for (auto const& [h, monitor] : ScanTileable()) {
        if (std::find(g_tileOrder.begin(), g_tileOrder.end(), h) != g_tileOrder.end()) continue;
        std::wstring key = KeptKey(h);
        auto place = std::find_if(g_keptPlaces.begin(), g_keptPlaces.end(), [&](KeptPlace const& p) { return p.key == key; });
        if (place == g_keptPlaces.end()) continue;
        if (std::any_of(g_keptWindows.begin(), g_keptWindows.end(), [&](auto const& w) { return w.second == key; })) continue;
        if (!MonitorFromRect(&place->frame, MONITOR_DEFAULTTONULL)) continue;  // its screen is gone
        PlaceFrame(h, place->frame);
        g_floating.push_back(h);
        g_keptWindows.push_back({h, key});
    }
}

void TileMonitor(HMONITOR monitor, bool byPosition) {
    if (!monitor || LayoutFor(monitor) == TileLayout::Off) return;
    // A window that won't fit is left floating and the rest tiled again
    // without it (a few times at most).
    for (int pass = 0; pass < 4; pass++) {
        std::vector<HWND> windows = WindowsOn(monitor, byPosition && pass == 0);
        std::vector<RECT> rects = RectsFor(monitor, (int)windows.size());
        bool again = false;
        for (size_t i = 0; i < windows.size() && i < rects.size(); i++) {
            if (!PlaceFrame(windows[i], rects[i])) {
                g_floating.push_back(windows[i]);
                again = true;
            }
        }
        if (!again) break;
    }
}

void TileAll() {
    PlaceKeptWindows();
    EnumDisplayMonitors(
        nullptr, nullptr,
        [](HMONITOR monitor, HDC, LPRECT, LPARAM) -> BOOL {
            TileMonitor(monitor, false);
            return TRUE;
        },
        0);
}

HMONITOR ActiveMonitor() {
    if (HWND fg = GetForegroundWindow()) return MonitorFromWindow(fg, MONITOR_DEFAULTTONEAREST);
    POINT pt;
    GetCursorPos(&pt);
    return MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
}

// The tiled window next to one, in a direction: the nearest whose middle lies
// that way.
int Neighbour(std::vector<RECT> const& rects, int from, int direction) {
    if (from < 0 || from >= (int)rects.size()) return -1;
    auto mid = [](RECT const& r) { return POINT{(r.left + r.right) / 2, (r.top + r.bottom) / 2}; };
    POINT a = mid(rects[from]);
    int best = -1;
    long long bestDistance = 0;
    for (int i = 0; i < (int)rects.size(); i++) {
        if (i == from) continue;
        POINT b = mid(rects[i]);
        int dx = b.x - a.x, dy = b.y - a.y;
        bool that = direction == kLeft ? dx < 0 && std::abs(dx) >= std::abs(dy) / 2
                  : direction == kRight ? dx > 0 && std::abs(dx) >= std::abs(dy) / 2
                  : direction == kUp    ? dy < 0 && std::abs(dy) >= std::abs(dx) / 2
                                        : dy > 0 && std::abs(dy) >= std::abs(dx) / 2;
        if (!that) continue;
        long long distance = (long long)dx * dx + (long long)dy * dy;
        if (best < 0 || distance < bestDistance) best = i, bestDistance = distance;
    }
    return best;
}

void SwapInOrder(HWND a, HWND b) {
    auto ia = std::find(g_tileOrder.begin(), g_tileOrder.end(), a);
    auto ib = std::find(g_tileOrder.begin(), g_tileOrder.end(), b);
    if (ia != g_tileOrder.end() && ib != g_tileOrder.end()) std::iter_swap(ia, ib);
}

void TileCommandRun(WPARAM command, LPARAM arg) {
    HWND fg = GetForegroundWindow();
    HMONITOR monitor = ActiveMonitor();
    switch (command) {
        case kTileNow:
            PlaceKeptWindows();
            TileMonitor(monitor, true);
            break;
        case kTileMove:
        case kTileFocus: {
            std::vector<HWND> windows = WindowsOn(monitor, false);
            std::vector<RECT> rects = RectsFor(monitor, (int)windows.size());
            int n = (int)windows.size();
            int from = (int)(std::find(windows.begin(), windows.end(), fg) - windows.begin());
            int to = Neighbour(rects, from, (int)arg);
            // Monocle: every window has the whole screen, so none lies any
            // way of another. Left and up go to the one before, right and
            // down to the one after, round and round.
            if (LayoutFor(monitor) == TileLayout::Monocle && from < n && n > 1)
                to = (from + (arg == kLeft || arg == kUp ? n - 1 : 1)) % n;
            if (to < 0) break;
            if (command == kTileFocus) {
                Activate(windows[to]);
            } else {
                SwapInOrder(windows[from], windows[to]);
                TileMonitor(monitor, false);
            }
            break;
        }
        case kTileFloat:
            if (!fg) break;
            if (IsFloating(fg)) {
                std::erase(g_floating, fg);
                ForgetPlace(fg);
            } else {
                g_floating.push_back(fg);
            }
            TileMonitor(monitor, false);
            break;
        case kTileLayout: {
            TileLayout now = LayoutFor(monitor);
            auto it = std::find(std::begin(kLayoutCycle), std::end(kLayoutCycle), now);
            TileLayout next = it == std::end(kLayoutCycle) || it + 1 == std::end(kLayoutCycle) ? kLayoutCycle[0] : *(it + 1);
            SetLayoutChoice(monitor, next);
            TileMonitor(monitor, false);
            break;
        }
        case kTileRatio: {
            LayoutChoice& c = ChoiceFor(monitor);
            c.ratioStep = std::clamp(c.ratioStep + (int)arg, 10 - g_tiling.ratio, 90 - g_tiling.ratio);
            SaveLayoutChoices();
            TileMonitor(monitor, false);
            break;
        }
        case kTileLayouts:
            OpenLayoutsWindow();
            break;
        case kTileApplyLayout:
            ApplyLayoutNumber((int)arg);
            break;
        case kTileAuto:
            g_autoTile = !g_autoTile;
            if (g_autoTile) TileAll();
            break;
    }
}

////////////////////////////////////////////////////////////////////////////////
// Automatic tiling: windows opening, closing, minimising and being dragged.

// A dragged tiled window dropped on another's place swaps with it.
void DropTiled(HWND h) {
    if (std::find(g_tileOrder.begin(), g_tileOrder.end(), h) == g_tileOrder.end()) return;
    POINT pt;
    GetCursorPos(&pt);
    HMONITOR monitor = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
    std::vector<HWND> windows = WindowsOn(monitor, false);
    std::vector<RECT> rects = RectsFor(monitor, (int)windows.size());
    for (size_t i = 0; i < rects.size() && i < windows.size(); i++) {
        if (windows[i] != h && PtInRect(&rects[i], pt)) {
            SwapInOrder(h, windows[i]);
            break;
        }
    }
}

// Where windows go when they open and close (tiling-layouts.cpp).
void PlacesEvent(DWORD event, HWND h);
void TrackOpenWindows();

////////////////////////////////////////////////////////////////////////////////
// The focused window's border: Windows 11 draws a window's thin border in a
// colour asked of it (DWMWA_BORDER_COLOR), so the window in front wears the
// accent and the one before is given back Windows' own border.

constexpr DWORD kDwmBorderColor = 34;               // DWMWA_BORDER_COLOR
constexpr COLORREF kDwmColorDefault = 0xFFFFFFFF;   // DWMWA_COLOR_DEFAULT
HWND g_bordered = nullptr;                          // the window wearing it

COLORREF FocusColor() {
    if (!g_tiling.focusAccent) return g_tiling.focusColor;
    DWORD abgr = 0, size = sizeof(abgr);
    if (RegGetValueW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\DWM", L"AccentColor", RRF_RT_REG_DWORD, nullptr,
                     &abgr, &size) == ERROR_SUCCESS)
        return abgr & 0xFFFFFF;
    DWORD argb = 0;
    BOOL opaque = FALSE;
    if (SUCCEEDED(DwmGetColorizationColor(&argb, &opaque))) return RGB((argb >> 16) & 0xFF, (argb >> 8) & 0xFF, argb & 0xFF);
    return RGB(0x00, 0x78, 0xD4);
}

void ClearFocusBorder() {
    if (g_bordered && IsWindow(g_bordered)) {
        COLORREF none = kDwmColorDefault;
        DwmSetWindowAttribute(g_bordered, kDwmBorderColor, &none, sizeof(none));
    }
    g_bordered = nullptr;
}

// The window in front now (or none): it wears the border if it's one the
// setting names, and whichever wore it before gives it back.
void UpdateFocusBorder(HWND h) {
    bool wear = false;
    if (h && g_tiling.focusBorder != kBorderNone) {
        if (g_tiling.focusBorder == kBorderAll)
            wear = Placeable(h);
        else
            wear = std::find(g_tileOrder.begin(), g_tileOrder.end(), h) != g_tileOrder.end() && Tileable(h) &&
                   LayoutFor(MonitorFromWindow(h, MONITOR_DEFAULTTONEAREST)) != TileLayout::Off;
    }
    if (wear && h == g_bordered) return;
    ClearFocusBorder();
    if (!wear) return;
    COLORREF color = FocusColor();
    if (SUCCEEDED(DwmSetWindowAttribute(h, kDwmBorderColor, &color, sizeof(color)))) g_bordered = h;
}

void CALLBACK TileEventProc(HWINEVENTHOOK, DWORD event, HWND h, LONG object, LONG child, DWORD, DWORD) {
    if (!h || object != OBJID_WINDOW || child != CHILDID_SELF) return;
    PlacesEvent(event, h);
    if (event == EVENT_SYSTEM_FOREGROUND) {
        UpdateFocusBorder(h);
        return;
    }
    if (event == EVENT_OBJECT_DESTROY && h == g_bordered) g_bordered = nullptr;
    // Drags and resizes are followed with or without automatic tiling, so a
    // window resized by hand isn't put back by the next tiling either.
    if (event == EVENT_SYSTEM_MOVESIZESTART) {
        g_userMoving = true;
        g_moving = h;
        g_movingFrame = FrameOf(h);
        return;
    }
    if (event == EVENT_SYSTEM_MOVESIZEEND) {
        g_userMoving = false;
        bool kept = h == g_moving && g_tiling.keepResized && MovedByHand(h);
        g_moving = nullptr;
        if (!g_autoTile) return;
        if (!kept) DropTiled(h);
    } else if (!g_autoTile || GetAncestor(h, GA_ROOT) != h) {
        return;
    }
    if (HWND wnd = g_wnd) SetTimer(wnd, kTimerTile, 150, nullptr);
}

void StopTileEvents() {
    for (auto& hook : g_tileHooks) {
        if (hook) UnhookWinEvent(hook);
        hook = nullptr;
    }
    ClearFocusBorder();
}

// On the switcher's thread, whose messages carry the events. Only what's
// turned on is followed, so with the tiling, the places and the border all
// off nothing is: no event wakes the suite as windows open, close or move.
void StartTileEvents() {
    StopTileEvents();
    DWORD flags = WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS;
    bool tiling = g_tiling.leaderVk || g_tiling.autoTile;
    if (tiling || g_tiling.rememberAll) {
        g_tileHooks[0] = SetWinEventHook(EVENT_SYSTEM_MOVESIZESTART, EVENT_SYSTEM_MINIMIZEEND, nullptr, TileEventProc, 0, 0, flags);
        g_tileHooks[1] = SetWinEventHook(EVENT_OBJECT_DESTROY, EVENT_OBJECT_HIDE, nullptr, TileEventProc, 0, 0, flags);
        g_tileHooks[2] = SetWinEventHook(EVENT_OBJECT_CLOAKED, EVENT_OBJECT_UNCLOAKED, nullptr, TileEventProc, 0, 0, flags);
    }
    // The focus moving: for the border, and for the places, whose frames are
    // noted as windows are left (rather than at every move any window makes).
    if (g_tiling.rememberAll || g_tiling.focusBorder != kBorderNone)
        g_tileHooks[3] = SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, nullptr, TileEventProc, 0, 0, flags);
    if (g_tiling.rememberAll) TrackOpenWindows();
    UpdateFocusBorder(GetForegroundWindow());
}

void TileTimer(HWND wnd) {
    KillTimer(wnd, kTimerTile);
    if (g_autoTile && !g_userMoving) {
        TileAll();
        UpdateFocusBorder(GetForegroundWindow());
    }
}

////////////////////////////////////////////////////////////////////////////////
// The chords, on the hook thread, held with the tiling key (a key and its
// modifiers, as the user set it; none until then). The key, pressed with
// exactly its modifiers, is kept back until it's clear what it's for: a chord
// key runs a command, anything else (typing) sends the key first after all,
// and letting go of it alone sends it then. Held down past the key repeat
// delay it goes through too, so holding it still works (as games need), and
// in a fullscreen app it's never kept back.

constexpr ULONG_PTR kTileReplayTag = 0x54494C45;  // "TILE": input this sent
bool g_leaderDown = false, g_leaderUsed = false;
DWORD g_leaderScan = 0;
std::vector<DWORD> g_chordKeysDown;  // chord keys whose key up is kept back too

void ReplayKeys(std::initializer_list<std::pair<DWORD, DWORD>> keys, bool up) {
    std::vector<INPUT> in;
    for (auto [vk, scan] : keys) {
        INPUT i{INPUT_KEYBOARD};
        i.ki.wVk = (WORD)vk;
        i.ki.wScan = (WORD)scan;
        i.ki.dwExtraInfo = kTileReplayTag;
        in.push_back(i);
    }
    if (up) {
        INPUT i{INPUT_KEYBOARD};
        i.ki.wVk = (WORD)keys.begin()->first;
        i.ki.wScan = (WORD)keys.begin()->second;
        i.ki.dwFlags = KEYEVENTF_KEYUP;
        i.ki.dwExtraInfo = kTileReplayTag;
        in.push_back(i);
    }
    SendInput((UINT)in.size(), in.data(), sizeof(INPUT));
}

bool ForegroundIsFullscreen() {
    HWND fg = GetForegroundWindow();
    if (!fg || fg == GetShellWindow()) return false;
    RECT r;
    MONITORINFO mi{sizeof(mi)};
    return GetWindowRect(fg, &r) && GetMonitorInfoW(MonitorFromWindow(fg, MONITOR_DEFAULTTONEAREST), &mi) &&
           r.left <= mi.rcMonitor.left && r.top <= mi.rcMonitor.top && r.right >= mi.rcMonitor.right &&
           r.bottom >= mi.rcMonitor.bottom;
}

bool ChordFor(DWORD vk, WPARAM* command, LPARAM* arg) {
    switch (vk) {
        case VK_TAB: *command = kTileNow; return true;
        case VK_LEFT: *command = kTileMove, *arg = kLeft; return true;
        case VK_RIGHT: *command = kTileMove, *arg = kRight; return true;
        case VK_UP: *command = kTileMove, *arg = kUp; return true;
        case VK_DOWN: *command = kTileMove, *arg = kDown; return true;
        case 'H': *command = kTileFocus, *arg = kLeft; return true;
        case 'L': *command = kTileFocus, *arg = kRight; return true;
        case 'K': *command = kTileFocus, *arg = kUp; return true;
        case 'J': *command = kTileFocus, *arg = kDown; return true;
        case VK_SPACE: *command = kTileFloat; return true;
        case 'N': *command = kTileLayout; return true;
        case VK_OEM_4: *command = kTileRatio, *arg = -5; return true;  // [
        case VK_OEM_6: *command = kTileRatio, *arg = 5; return true;   // ]
        case 'A': *command = kTileAuto; return true;
        case 'M': *command = kTileLayouts; return true;
        case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
            *command = kTileApplyLayout, *arg = (LPARAM)(vk - '1');
            return true;
    }
    return false;
}

// -1: not the chords' business; otherwise the key is kept back.
int TilingKeyHook(WPARAM wParam, KBDLLHOOKSTRUCT const* k) {
    if (k->dwExtraInfo == kTileReplayTag) return -1;
    bool down = wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN;
    DWORD vk = k->vkCode;
    if (!down) {
        if (auto it = std::find(g_chordKeysDown.begin(), g_chordKeysDown.end(), vk); it != g_chordKeysDown.end()) {
            g_chordKeysDown.erase(it);
            return 1;
        }
    }
    DWORD leader = g_leaderVk;
    if (leader && vk == leader) {
        if (down) {
            if (g_leaderDown) {
                // Held: the key repeat. Unless a chord was used, it's the key
                // held, which goes through from here.
                if (!g_leaderUsed) {
                    g_leaderDown = false;
                    ReplayKeys({{leader, g_leaderScan}}, false);
                    return -1;
                }
                return 1;
            }
            // With exactly the modifiers set, no more and no fewer.
            UINT mods = g_leaderMods;
            bool shift = GetAsyncKeyState(VK_SHIFT) < 0, ctrl = GetAsyncKeyState(VK_CONTROL) < 0;
            bool alt = (k->flags & LLKHF_ALTDOWN) != 0, win = GetAsyncKeyState(VK_LWIN) < 0 || GetAsyncKeyState(VK_RWIN) < 0;
            bool exact = shift == ((mods & MOD_SHIFT) != 0) && ctrl == ((mods & MOD_CONTROL) != 0) &&
                         alt == ((mods & MOD_ALT) != 0) && win == ((mods & MOD_WIN) != 0);
            if (exact && !ForegroundIsFullscreen()) {
                g_leaderDown = true;
                g_leaderUsed = false;
                g_leaderScan = k->scanCode;
                // Win or Alt let go after a key that was kept back would open
                // Start or the program's menu bar: a key nothing uses is
                // pressed between, as the switcher does.
                if (mods & (MOD_WIN | MOD_ALT)) ReplayKeys({{kDummyKey, 0}}, true);
                return 1;
            }
            return -1;
        }
        if (g_leaderDown) {
            g_leaderDown = false;
            if (!g_leaderUsed) ReplayKeys({{leader, g_leaderScan}}, true);
            return 1;
        }
        return -1;
    }
    if (!g_leaderDown || !down) return -1;
    if (vk == VK_SHIFT || vk == VK_LSHIFT || vk == VK_RSHIFT || vk == VK_CONTROL || vk == VK_LCONTROL ||
        vk == VK_RCONTROL || vk == VK_MENU || vk == VK_LMENU || vk == VK_RMENU || vk == VK_LWIN || vk == VK_RWIN)
        return -1;
    WPARAM command = 0;
    LPARAM arg = 0;
    if (ChordFor(vk, &command, &arg)) {
        g_leaderUsed = true;
        g_chordKeysDown.push_back(vk);
        if (HWND wnd = g_wnd) PostMessageW(wnd, WM_APP_TILE, command, arg);
        return 1;
    }
    // Typing: the tiling key first, then this key.
    g_leaderDown = false;
    if (g_leaderUsed) return -1;
    ReplayKeys({{leader, g_leaderScan}, {vk, k->scanCode}}, false);
    return 1;
}
////////////////////////////////////////////////////////////////////////////////
// Where windows go ("// @inline tiling-layouts.cpp", after tiling.cpp, in the
// Switcher): every window's place when it closed, given back when one of its
// kind opens, and layouts (the windows' places and each screen's tiling,
// saved under a name), kept in a window of their own (the tiling key and M).

////////////////////////////////////////////////////////////////////////////////
// Places on close

struct SavedPlace {
    std::wstring key;
    RECT frame;
    bool maximized;
};
struct TrackedWindow {
    std::wstring key;
    RECT frame{};
    RECT insets{};  // its invisible resize borders (the window's rect less its frame), as last seen
    bool have = false, maximized = false;
};
std::vector<SavedPlace> g_places;  // newest last
std::unordered_map<HWND, TrackedWindow> g_tracked;
HWND g_placesFront = nullptr;  // the window in front, whose frame is noted when it's left
constexpr size_t kMostPlaces = 256;
constexpr WCHAR kPlacesValue[] = L"switcher.tiling.places";
constexpr UINT_PTR kTimerPlaces = 4;
bool g_placesDirty = false;

// Without the characters the saved lists use between their parts.
std::wstring Sanitized(std::wstring s) {
    std::replace_if(s.begin(), s.end(), [](WCHAR c) { return c == L';' || c == L'=' || c == L'\n' || c == L'\r' || c == 0x1E; }, L'_');
    return s;
}

// A window's kind: its program and class (as for kept places), and for a
// dialog or an owned window its title too, as one program has many.
std::wstring PlaceKey(HWND h, std::wstring const& kept) {
    WCHAR cls[64] = L"";
    GetClassNameW(h, cls, ARRAYSIZE(cls));
    if (!GetWindow(h, GW_OWNER) && wcscmp(cls, L"#32770")) return kept;
    std::wstring title = Title(h);
    CharLowerBuffW(title.data(), (DWORD)title.size());
    return kept + L"|" + Sanitized(title);
}

// Any window with a title bar or a resizing border: programs' windows,
// dialogs, tool palettes. Not menus, tips, the shell's own or the suite's.
bool Placeable(HWND h) {
    if (!IsWindowVisible(h) || GetAncestor(h, GA_ROOT) != h) return false;
    LONG style = GetWindowLongW(h, GWL_STYLE), ex = GetWindowLongW(h, GWL_EXSTYLE);
    if ((style & WS_CHILD) || (ex & WS_EX_NOACTIVATE)) return false;
    if ((style & WS_CAPTION) != WS_CAPTION && !(style & WS_THICKFRAME)) return false;
    DWORD pid = 0;
    GetWindowThreadProcessId(h, &pid);
    if (pid == GetCurrentProcessId()) return false;
    DWORD cloaked = 0;
    if (SUCCEEDED(DwmGetWindowAttribute(h, DWMWA_CLOAKED, &cloaked, sizeof(cloaked))) && cloaked) return false;
    WCHAR cls[64] = L"";
    GetClassNameW(h, cls, ARRAYSIZE(cls));
    for (PCWSTR shell : {L"Shell_TrayWnd", L"Shell_SecondaryTrayWnd", L"Progman", L"WorkerW", L"#32768"})
        if (!wcscmp(cls, shell)) return false;
    return wcsncmp(cls, L"Tourne", 6) != 0;
}

void LoadPlaces() {
    g_places.clear();
    std::wstring text(65536, L'\0');
    Wh_GetStringValue(kPlacesValue, text.data(), text.size());
    text.resize(wcslen(text.c_str()));
    std::wstring_view rest = text;
    while (!rest.empty()) {
        size_t end = rest.find(L';');
        std::wstring_view item = rest.substr(0, end);
        size_t eq = item.rfind(L'=');
        RECT r;
        int maximized = 0;
        if (eq != std::wstring_view::npos &&
            swscanf(std::wstring(item.substr(eq + 1)).c_str(), L"%ld,%ld,%ld,%ld,%d", &r.left, &r.top, &r.right, &r.bottom,
                    &maximized) >= 4)
            g_places.push_back({std::wstring(item.substr(0, eq)), r, maximized != 0});
        rest = end == std::wstring_view::npos ? std::wstring_view() : rest.substr(end + 1);
    }
}

void SavePlaces() {
    g_placesDirty = false;
    if (g_places.size() > kMostPlaces) g_places.erase(g_places.begin(), g_places.end() - kMostPlaces);
    std::wstring text;
    for (auto const& p : g_places) {
        WCHAR numbers[64];
        swprintf_s(numbers, L"=%ld,%ld,%ld,%ld,%d;", p.frame.left, p.frame.top, p.frame.right, p.frame.bottom,
                   p.maximized ? 1 : 0);
        text += p.key + numbers;
    }
    Wh_SetStringValue(kPlacesValue, text.c_str());
}

// Written a little later, once for a burst of windows closing.
void SavePlacesSoon() {
    g_placesDirty = true;
    if (HWND wnd = g_wnd) SetTimer(wnd, kTimerPlaces, 2000, nullptr);
}

void PlacesTimer(HWND wnd) {
    KillTimer(wnd, kTimerPlaces);
    if (g_placesDirty) SavePlaces();
}

// A window's frame as it is, noted when it opens, is left for another,
// minimises, is let go after a drag, or hides (no hook follows every move
// any window makes, which would wake the suite all the time). Minimised or
// maximised, it's the frame it goes back to (and a maximised one is
// maximised again); hidden, the one it had.
void NoteFrame(HWND h) {
    auto it = g_tracked.find(h);
    if (it == g_tracked.end() || !IsWindow(h)) return;
    TrackedWindow& t = it->second;
    RECT r{};
    if (!IsIconic(h) && !IsZoomed(h)) {
        if (!GetWindowRect(h, &r)) return;
        if (IsWindowVisible(h)) {
            RECT frame = FrameOf(h);
            t.insets = {frame.left - r.left, frame.top - r.top, r.right - frame.right, r.bottom - frame.bottom};
        }
        t.maximized = false;
    } else {
        WINDOWPLACEMENT wp{sizeof(wp)};
        if (!GetWindowPlacement(h, &wp)) return;
        r = wp.rcNormalPosition;
        // In workspace coordinates (the work area's, not the screen's),
        // unless it's a tool window.
        if (!(GetWindowLongW(h, GWL_EXSTYLE) & WS_EX_TOOLWINDOW)) {
            MONITORINFO mi{sizeof(mi)};
            if (GetMonitorInfoW(MonitorFromRect(&r, MONITOR_DEFAULTTONEAREST), &mi))
                OffsetRect(&r, mi.rcWork.left - mi.rcMonitor.left, mi.rcWork.top - mi.rcMonitor.top);
        }
        t.maximized = IsZoomed(h) || (wp.flags & WPF_RESTORETOMAXIMIZED);
    }
    t.frame = {r.left + t.insets.left, r.top + t.insets.top, r.right - t.insets.right, r.bottom - t.insets.bottom};
    t.have = true;
}

// Puts a window's visible frame on a rect: moved and sized, or only moved if
// it can't be resized.
void MoveFrameTo(HWND h, RECT const& want) {
    if (IsZoomed(h) || IsIconic(h)) ShowWindow(h, SW_SHOWNOACTIVATE);
    RECT window{}, frame = FrameOf(h);
    GetWindowRect(h, &window);
    int il = frame.left - window.left, it = frame.top - window.top;
    int ir = window.right - frame.right, ib = window.bottom - frame.bottom;
    UINT flags = SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE;
    if (!(GetWindowLongW(h, GWL_STYLE) & WS_THICKFRAME)) flags |= SWP_NOSIZE;
    if (!SetWindowPos(h, nullptr, want.left - il, want.top - it, (want.right - want.left) + il + ir,
                      (want.bottom - want.top) + it + ib, flags) &&
        GetLastError() == ERROR_ACCESS_DENIED)
        PlaceThroughAgent(h, want, false);  // a program running as administrator
}

bool WillBeTiled(HWND h) {
    return g_autoTile && Tileable(h) && LayoutFor(MonitorFromWindow(h, MONITOR_DEFAULTTONEAREST)) != TileLayout::Off;
}

// A window opening: back where its kind was when one last closed, unless the
// tiling places it (or a place kept for it by hand does).
void WindowShown(HWND h) {
    if (g_tracked.count(h)) {
        NoteFrame(h);
        return;
    }
    if (!Placeable(h)) return;
    std::wstring kept = KeptKey(h);
    std::wstring key = PlaceKey(h, kept);
    g_tracked[h].key = key;
    bool keptByHand = g_tiling.keepResized &&
                      std::any_of(g_keptPlaces.begin(), g_keptPlaces.end(), [&](KeptPlace const& p) { return p.key == kept; });
    auto place = std::find_if(g_places.begin(), g_places.end(), [&](SavedPlace const& p) { return p.key == key; });
    if (place != g_places.end() && !keptByHand && !WillBeTiled(h) && MonitorFromRect(&place->frame, MONITOR_DEFAULTTONULL)) {
        MoveFrameTo(h, place->frame);
        if (place->maximized && (GetWindowLongW(h, GWL_STYLE) & WS_MAXIMIZEBOX)) ShowWindow(h, SW_MAXIMIZE);
    }
    NoteFrame(h);
}

// Closed (or hidden, as programs that close to the tray do): its place is
// kept for the next of its kind.
void WindowGone(HWND h, bool destroyed) {
    auto it = g_tracked.find(h);
    if (it == g_tracked.end()) return;
    TrackedWindow const& t = it->second;
    if (t.have) {
        std::erase_if(g_places, [&](SavedPlace const& p) { return p.key == t.key; });
        g_places.push_back({t.key, t.frame, t.maximized});
        SavePlacesSoon();
    }
    if (destroyed) g_tracked.erase(it);
}

// The windows already open, followed from now (not moved).
void TrackOpenWindows() {
    EnumWindows(
        [](HWND h, LPARAM) __stdcall -> BOOL {
            if (!g_tracked.count(h) && Placeable(h)) {
                g_tracked[h].key = PlaceKey(h, KeptKey(h));
                NoteFrame(h);
            }
            return TRUE;
        },
        0);
}

void PlacesEvent(DWORD event, HWND h) {
    if (!g_tiling.rememberAll) return;
    switch (event) {
        case EVENT_OBJECT_SHOW:
        case EVENT_OBJECT_UNCLOAKED:
            WindowShown(h);
            break;
        case EVENT_SYSTEM_FOREGROUND:
            if (g_placesFront && g_placesFront != h) NoteFrame(g_placesFront);
            g_placesFront = h;
            NoteFrame(h);
            break;
        case EVENT_SYSTEM_MOVESIZEEND:
        case EVENT_SYSTEM_MINIMIZESTART:
        case EVENT_SYSTEM_MINIMIZEEND:
            NoteFrame(h);
            break;
        case EVENT_OBJECT_HIDE:
            NoteFrame(h);  // (if it's still there: the event comes a little after)
            WindowGone(h, false);
            break;
        case EVENT_OBJECT_DESTROY:
            if (h == g_placesFront) g_placesFront = nullptr;
            WindowGone(h, true);
            break;
    }
}

////////////////////////////////////////////////////////////////////////////////
// Layouts: each window's place and whether it's tiled, and each screen's
// tiling layout, as text that can be edited:
//   screen 0 0 2560 1440 verticalStack
//   tile 6 4 1274 1396 chrome.exe|chrome_widgetwin_1
//   float 300 200 1100 800 notepad.exe|notepad

struct SavedLayout {
    std::wstring name, text;
};
std::vector<SavedLayout> g_layouts;
constexpr WCHAR kLayoutsValue[] = L"switcher.tiling.layouts";

constexpr std::pair<TileLayout, PCWSTR> kLayoutNames[] = {
    {TileLayout::Off, L"off"},
    {TileLayout::Columns, L"columns"},
    {TileLayout::Rows, L"rows"},
    {TileLayout::VerticalStack, L"verticalStack"},
    {TileLayout::RightMainVerticalStack, L"rightMainVerticalStack"},
    {TileLayout::HorizontalStack, L"horizontalStack"},
    {TileLayout::UltrawideVerticalStack, L"ultrawideVerticalStack"},
    {TileLayout::Grid, L"grid"},
    {TileLayout::Bsp, L"bsp"},
    {TileLayout::Monocle, L"monocle"},
};

PCWSTR LayoutName(TileLayout layout) {
    for (auto [l, name] : kLayoutNames)
        if (l == layout) return name;
    return L"columns";
}

void LoadLayouts() {
    g_layouts.clear();
    std::wstring text(262144, L'\0');
    Wh_GetStringValue(kLayoutsValue, text.data(), text.size());
    text.resize(wcslen(text.c_str()));
    std::wstring_view rest = text;
    while (!rest.empty()) {
        size_t end = rest.find(L'\x1E');
        std::wstring_view item = rest.substr(0, end);
        size_t line = item.find(L'\n');
        if (!item.empty())
            g_layouts.push_back({std::wstring(item.substr(0, line)),
                                 line == std::wstring_view::npos ? L"" : std::wstring(item.substr(line + 1))});
        rest = end == std::wstring_view::npos ? std::wstring_view() : rest.substr(end + 1);
    }
}

void SaveLayouts() {
    std::wstring text;
    for (auto const& l : g_layouts) {
        if (!text.empty()) text += L'\x1E';
        std::wstring body = l.text;
        std::erase(body, L'\x1E');
        text += Sanitized(l.name) + L"\n" + body;
    }
    Wh_SetStringValue(kLayoutsValue, text.c_str());
}

// The windows open now, as a layout's text: the tiled ones in their order,
// then the rest from the top down.
std::wstring SnapshotLayout() {
    std::wstring text =
        L"# A screen: its edges (left top right bottom) and its tiling layout\r\n"
        L"# (off, columns, rows, verticalStack, rightMainVerticalStack, horizontalStack,\r\n"
        L"# ultrawideVerticalStack, grid, bsp, monocle).\r\n"
        L"# A window: tile or float, its edges, then program|class.\r\n";
    struct Screens {
        std::wstring* text;
    } screens{&text};
    EnumDisplayMonitors(
        nullptr, nullptr,
        [](HMONITOR monitor, HDC, LPRECT, LPARAM param) -> BOOL {
            MONITORINFO mi{sizeof(mi)};
            GetMonitorInfoW(monitor, &mi);
            WCHAR line[160];
            swprintf_s(line, L"screen %ld %ld %ld %ld %ls\r\n", mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right,
                       mi.rcMonitor.bottom, LayoutName(LayoutFor(monitor)));
            *((Screens*)param)->text += line;
            return TRUE;
        },
        (LPARAM)&screens);
    std::vector<HWND> windows;
    EnumWindows(
        [](HWND h, LPARAM param) __stdcall -> BOOL {
            if (Placeable(h) && !IsIconic(h)) ((std::vector<HWND>*)param)->push_back(h);
            return TRUE;
        },
        (LPARAM)&windows);
    auto line = [&](HWND h, bool tiled) {
        RECT r = FrameOf(h);
        WCHAR numbers[80];
        swprintf_s(numbers, L"%ls %ld %ld %ld %ld ", tiled ? L"tile" : L"float", r.left, r.top, r.right, r.bottom);
        text += numbers + KeptKey(h) + L"\r\n";
    };
    std::vector<HWND> tiled;
    for (HWND h : g_tileOrder)
        if (std::find(windows.begin(), windows.end(), h) != windows.end() && Tileable(h) &&
            LayoutFor(MonitorFromWindow(h, MONITOR_DEFAULTTONEAREST)) != TileLayout::Off)
            tiled.push_back(h);
    for (HWND h : tiled) line(h, true);
    for (HWND h : windows)
        if (std::find(tiled.begin(), tiled.end(), h) == tiled.end()) line(h, false);
    return text;
}

// Puts the open windows where a layout has them: each line takes the first
// window of its program and class not taken yet. False if nothing matched.
bool ApplyLayout(std::wstring const& text) {
    struct Candidate {
        HWND h;
        std::wstring key;
        bool used;
    };
    std::vector<Candidate> candidates;
    EnumWindows(
        [](HWND h, LPARAM param) __stdcall -> BOOL {
            if (Placeable(h) || (IsIconic(h) && GetAncestor(h, GA_ROOT) == h && IsWindowVisible(h)))
                ((std::vector<Candidate>*)param)->push_back({h, {}, false});
            return TRUE;
        },
        (LPARAM)&candidates);
    for (auto& c : candidates) c.key = KeptKey(c.h);
    std::vector<HWND> tiledOrder;
    bool any = false;
    size_t at = 0;
    while (at < text.size()) {
        size_t end = text.find(L'\n', at);
        std::wstring line = text.substr(at, end == std::wstring::npos ? std::wstring::npos : end - at);
        at = end == std::wstring::npos ? text.size() : end + 1;
        while (!line.empty() && iswspace(line.back())) line.pop_back();
        WCHAR kind[16] = L"";
        RECT r{};
        int used = 0;
        if (line.empty() || line[0] == L'#' ||
            swscanf_s(line.c_str(), L"%15s %ld %ld %ld %ld %n", kind, (unsigned)ARRAYSIZE(kind), &r.left, &r.top,
                      &r.right, &r.bottom, &used) < 5 ||
            !used)
            continue;
        std::wstring rest = line.substr(used);
        if (!wcscmp(kind, L"screen")) {
            HMONITOR monitor = MonitorFromRect(&r, MONITOR_DEFAULTTONULL);
            if (!monitor) continue;
            SetLayoutChoice(monitor, ParseLayout(rest, TileLayout::Columns));
            continue;
        }
        bool tile = !wcscmp(kind, L"tile");
        if (!tile && wcscmp(kind, L"float")) continue;
        CharLowerBuffW(rest.data(), (DWORD)rest.size());
        auto c = std::find_if(candidates.begin(), candidates.end(), [&](Candidate const& c) { return !c.used && c.key == rest; });
        if (c == candidates.end()) continue;
        c->used = true;
        any = true;
        if (IsIconic(c->h)) ShowWindow(c->h, SW_SHOWNOACTIVATE);
        if (tile) {
            std::erase(g_floating, c->h);
            ForgetPlace(c->h);
            tiledOrder.push_back(c->h);
        } else {
            if (!IsFloating(c->h)) g_floating.push_back(c->h);
            if (MonitorFromRect(&r, MONITOR_DEFAULTTONULL)) MoveFrameTo(c->h, r);
        }
    }
    // The tiled ones first, in the layout's order.
    std::erase_if(g_tileOrder, [&](HWND h) { return std::find(tiledOrder.begin(), tiledOrder.end(), h) != tiledOrder.end(); });
    g_tileOrder.insert(g_tileOrder.begin(), tiledOrder.begin(), tiledOrder.end());
    EnumDisplayMonitors(
        nullptr, nullptr,
        [](HMONITOR monitor, HDC, LPRECT, LPARAM) -> BOOL {
            TileMonitor(monitor, false);
            return TRUE;
        },
        0);
    return any;
}

////////////////////////////////////////////////////////////////////////////////
// The layouts window

constexpr WCHAR kLayoutsClass[] = L"TourneTileLayouts_" WH_MOD_ID;
enum LayoutControl : int { kLyList = 201, kLyName, kLyText, kLyNew, kLyApply, kLyCapture, kLySave, kLyDelete, kLyStatus };

struct LayoutsWindow {
    HWND wnd = nullptr;
    COLORREF raised = 0;  // the fields: the ground a little toward the text
    HFONT font = nullptr;
    HBRUSH ground = nullptr, field = nullptr;
    int selected = -1;
} g_ly;

std::wstring ControlText(HWND parent, int id) {
    HWND control = GetDlgItem(parent, id);
    std::wstring text(GetWindowTextLengthW(control), L'\0');
    if (!text.empty()) GetWindowTextW(control, text.data(), (int)text.size() + 1);
    return text;
}

void LayoutsStatus(PCWSTR text) {
    if (g_ly.wnd) SetDlgItemTextW(g_ly.wnd, kLyStatus, text);
}

void FillLayoutsList(int select) {
    HWND list = GetDlgItem(g_ly.wnd, kLyList);
    SendMessageW(list, LB_RESETCONTENT, 0, 0);
    for (auto const& l : g_layouts) SendMessageW(list, LB_ADDSTRING, 0, (LPARAM)l.name.c_str());
    g_ly.selected = g_layouts.empty() ? -1 : std::clamp(select, 0, (int)g_layouts.size() - 1);
    SendMessageW(list, LB_SETCURSEL, g_ly.selected, 0);
    if (g_ly.selected >= 0) {
        SetDlgItemTextW(g_ly.wnd, kLyName, g_layouts[g_ly.selected].name.c_str());
        SetDlgItemTextW(g_ly.wnd, kLyText, g_layouts[g_ly.selected].text.c_str());
    } else {
        SetDlgItemTextW(g_ly.wnd, kLyName, L"");
        SetDlgItemTextW(g_ly.wnd, kLyText, L"");
    }
}

std::wstring NewLayoutName() {
    for (int n = (int)g_layouts.size() + 1;; n++) {
        std::wstring name = L"Layout " + std::to_wstring(n);
        if (std::none_of(g_layouts.begin(), g_layouts.end(), [&](SavedLayout const& l) { return l.name == name; })) return name;
    }
}

void LayoutsCommand(HWND wnd, int id, int code) {
    switch (id) {
        case kLyList:
            if (code == LBN_SELCHANGE) {
                FillLayoutsList((int)SendDlgItemMessageW(wnd, kLyList, LB_GETCURSEL, 0, 0));
                LayoutsStatus(L"");
            }
            break;
        case kLyNew:
            g_layouts.push_back({NewLayoutName(), SnapshotLayout()});
            SaveLayouts();
            FillLayoutsList((int)g_layouts.size() - 1);
            SetFocus(GetDlgItem(wnd, kLyName));
            SendDlgItemMessageW(wnd, kLyName, EM_SETSEL, 0, -1);
            LayoutsStatus(L"Saved the windows open now as a new layout. Type a name and press Save to rename it.");
            break;
        case kLyApply:
            LayoutsStatus(ApplyLayout(ControlText(wnd, kLyText)) ? L"Applied." : L"None of its windows are open.");
            break;
        case kLyCapture:
            if (g_ly.selected < 0) break;
            g_layouts[g_ly.selected].text = SnapshotLayout();
            SaveLayouts();
            FillLayoutsList(g_ly.selected);
            LayoutsStatus(L"Updated from the windows open now.");
            break;
        case kLySave: {
            std::wstring name = Sanitized(ControlText(wnd, kLyName));
            if (name.empty()) {
                LayoutsStatus(L"Give it a name first.");
                break;
            }
            if (g_ly.selected < 0) {
                g_layouts.push_back({});
                g_ly.selected = (int)g_layouts.size() - 1;
            }
            g_layouts[g_ly.selected] = {name, ControlText(wnd, kLyText)};
            SaveLayouts();
            FillLayoutsList(g_ly.selected);
            LayoutsStatus(L"Saved.");
            break;
        }
        case kLyDelete: {
            if (g_ly.selected < 0) break;
            std::wstring ask = L"Delete the layout \"" + g_layouts[g_ly.selected].name + L"\"?";
            if (MessageBoxW(wnd, ask.c_str(), L"Window layouts", MB_YESNO | MB_ICONQUESTION) != IDYES) break;
            g_layouts.erase(g_layouts.begin() + g_ly.selected);
            SaveLayouts();
            FillLayoutsList(g_ly.selected);
            LayoutsStatus(L"Deleted.");
            break;
        }
        case IDCANCEL:
            DestroyWindow(wnd);
            break;
    }
}

LRESULT CALLBACK LayoutsWndProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    Look const& look = g_settings.look;
    switch (msg) {
        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORLISTBOX:
            SetTextColor((HDC)wParam, Rgb(look.text));
            SetBkColor((HDC)wParam, g_ly.raised);
            return (LRESULT)g_ly.field;
        case WM_CTLCOLORSTATIC:
            SetTextColor((HDC)wParam, Rgb(look.text));
            SetBkColor((HDC)wParam, Rgb(look.ground));
            return (LRESULT)g_ly.ground;
        case WM_ERASEBKGND: {
            RECT rc;
            GetClientRect(wnd, &rc);
            FillRect((HDC)wParam, &rc, g_ly.ground);
            return 1;
        }
        case WM_MEASUREITEM: {
            auto* item = (MEASUREITEMSTRUCT*)lParam;
            HDC dc = GetDC(wnd);
            HGDIOBJ old = SelectObject(dc, g_ly.font);
            TEXTMETRICW tm;
            GetTextMetricsW(dc, &tm);
            SelectObject(dc, old);
            ReleaseDC(wnd, dc);
            item->itemHeight = tm.tmHeight + tm.tmHeight / 2;
            return TRUE;
        }
        case WM_DRAWITEM: {
            auto* item = (DRAWITEMSTRUCT*)lParam;
            bool selected = item->itemState & ODS_SELECTED, focused = item->itemState & ODS_FOCUS;
            std::wstring text;
            if (item->CtlType == ODT_LISTBOX) {
                if (item->itemID == (UINT)-1) return TRUE;
                text.resize(SendMessageW(item->hwndItem, LB_GETTEXTLEN, item->itemID, 0));
                SendMessageW(item->hwndItem, LB_GETTEXT, item->itemID, (LPARAM)text.data());
                HBRUSH fill = CreateSolidBrush(selected ? Rgb(look.highlight) : g_ly.raised);
                FillRect(item->hDC, &item->rcItem, fill);
                DeleteObject(fill);
                RECT r = item->rcItem;
                r.left += 6;
                SetBkMode(item->hDC, TRANSPARENT);
                SetTextColor(item->hDC, Rgb(selected ? look.selectedText : look.text));
                HGDIOBJ old = SelectObject(item->hDC, g_ly.font);
                DrawTextW(item->hDC, text.c_str(), -1, &r, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
                SelectObject(item->hDC, old);
                return TRUE;
            }
            text.resize(GetWindowTextLengthW(item->hwndItem));
            GetWindowTextW(item->hwndItem, text.data(), (int)text.size() + 1);
            HBRUSH fill = CreateSolidBrush(selected ? Rgb(look.highlight) : g_ly.raised);
            FillRect(item->hDC, &item->rcItem, fill);
            DeleteObject(fill);
            HBRUSH frame = CreateSolidBrush(Rgb(focused ? look.highlightBorder : look.border));
            FrameRect(item->hDC, &item->rcItem, frame);
            DeleteObject(frame);
            SetBkMode(item->hDC, TRANSPARENT);
            SetTextColor(item->hDC, Rgb(selected ? look.selectedText : look.text));
            HGDIOBJ old = SelectObject(item->hDC, g_ly.font);
            DrawTextW(item->hDC, text.c_str(), -1, &item->rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            SelectObject(item->hDC, old);
            return TRUE;
        }
        case WM_COMMAND:
            LayoutsCommand(wnd, LOWORD(wParam), HIWORD(wParam));
            return 0;
        case WM_CLOSE:
            DestroyWindow(wnd);
            return 0;
        case WM_DESTROY:
            if (g_ly.font) DeleteObject(g_ly.font);
            if (g_ly.ground) DeleteObject(g_ly.ground);
            if (g_ly.field) DeleteObject(g_ly.field);
            g_ly = {};
            return 0;
    }
    return DefWindowProcW(wnd, msg, wParam, lParam);
}

// For the thread's message loop: Tab and Escape in the layouts window.
bool LayoutsDialogMessage(MSG* msg) {
    return g_ly.wnd && IsDialogMessageW(g_ly.wnd, msg);
}

void OpenLayoutsWindow() {
    if (g_ly.wnd) {
        SetForegroundWindow(g_ly.wnd);
        return;
    }
    HINSTANCE instance = nullptr;
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                       (LPCWSTR)&LayoutsWndProc, &instance);
    static bool registered = false;
    if (!registered) {
        WNDCLASSW wc{};
        wc.lpfnWndProc = LayoutsWndProc;
        wc.hInstance = instance;
        wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        wc.lpszClassName = kLayoutsClass;
        registered = RegisterClassW(&wc) != 0;
    }
    HMONITOR monitor = ActiveMonitor();
    UINT dpi = MonitorDpi(monitor);
    auto px = [dpi](int v) { return MulDiv(v, dpi, 96); };
    Look const& look = g_settings.look;
    LOGFONTW lf{};
    lf.lfHeight = -MulDiv(std::max(g_settings.fontSize, 9), dpi, 72);
    lf.lfWeight = FW_NORMAL;
    lf.lfQuality = NONANTIALIASED_QUALITY;
    wcsncpy_s(lf.lfFaceName, g_settings.font.c_str(), _TRUNCATE);
    g_ly.font = CreateFontIndirectW(&lf);
    g_ly.ground = CreateSolidBrush(Rgb(look.ground));
    auto toward = [](uint32_t from, uint32_t to, int part) {  // part in 16ths
        auto mix = [&](int shift) { return (((from >> shift) & 0xFF) * (16 - part) + ((to >> shift) & 0xFF) * part) / 16; };
        return RGB(mix(16), mix(8), mix(0));
    };
    g_ly.raised = toward(look.ground, look.text, 2);
    g_ly.field = CreateSolidBrush(g_ly.raised);
    int width = px(820), height = px(500), pad = px(12), listWidth = px(200), row = px(26), button = px(24);
    RECT frame{0, 0, width, height};
    AdjustWindowRectExForDpi(&frame, WS_OVERLAPPEDWINDOW, FALSE, 0, dpi);
    MONITORINFO mi{sizeof(mi)};
    GetMonitorInfoW(monitor, &mi);
    int w = frame.right - frame.left, h = frame.bottom - frame.top;
    RECT const& work = mi.rcWork;
    int x = work.left + std::max(0, (int)(work.right - work.left - w) / 2);
    int y = work.top + std::max(0, (int)(work.bottom - work.top - h) / 2);
    g_ly.wnd = CreateWindowExW(WS_EX_CONTROLPARENT, kLayoutsClass, L"Window layouts",
                               WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_POPUP, x, y, w, h, nullptr, nullptr,
                               instance, nullptr);
    if (!g_ly.wnd) return;
    bool dark = (GetRValue(Rgb(look.ground)) + GetGValue(Rgb(look.ground)) + GetBValue(Rgb(look.ground))) < 3 * 128;
    BOOL darkFlag = dark;
    DwmSetWindowAttribute(g_ly.wnd, 20 /* DWMWA_USE_IMMERSIVE_DARK_MODE */, &darkFlag, sizeof(darkFlag));
    COLORREF caption = Rgb(look.ground), captionText = Rgb(look.text), border = Rgb(look.border);
    DwmSetWindowAttribute(g_ly.wnd, 35 /* DWMWA_CAPTION_COLOR */, &caption, sizeof(caption));
    DwmSetWindowAttribute(g_ly.wnd, 36 /* DWMWA_TEXT_COLOR */, &captionText, sizeof(captionText));
    DwmSetWindowAttribute(g_ly.wnd, 34 /* DWMWA_BORDER_COLOR */, &border, sizeof(border));

    auto add = [&](PCWSTR cls, PCWSTR text, DWORD style, DWORD ex, int id, int left, int top, int right, int bottom) {
        HWND control = CreateWindowExW(ex, cls, text, WS_CHILD | WS_VISIBLE | style, left, top, right - left,
                                       bottom - top, g_ly.wnd, (HMENU)(INT_PTR)id, instance, nullptr);
        SendMessageW(control, WM_SETFONT, (WPARAM)g_ly.font, FALSE);
        if (dark) SetWindowTheme(control, L"DarkMode_Explorer", nullptr);
        return control;
    };
    int right = width - pad, textLeft = pad + listWidth + pad;
    int bottomRow = height - pad - button - row;  // the buttons, above the status line
    add(L"STATIC", L"Layouts", SS_LEFT, 0, -1, pad, pad, pad + listWidth, pad + px(18));
    add(L"LISTBOX", L"", WS_TABSTOP | WS_VSCROLL | LBS_NOTIFY | LBS_OWNERDRAWFIXED | LBS_HASSTRINGS | LBS_NOINTEGRALHEIGHT,
        0, kLyList, pad, pad + px(22), pad + listWidth, bottomRow - pad);
    add(L"STATIC", L"Name", SS_LEFT | SS_CENTERIMAGE, 0, -1, textLeft, pad, textLeft + px(50), pad + px(22));
    add(L"EDIT", L"", WS_TABSTOP | ES_AUTOHSCROLL, 0, kLyName, textLeft + px(54), pad, right, pad + px(22));
    add(L"EDIT", L"", WS_TABSTOP | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_WANTRETURN,
        0, kLyText, textLeft, pad + px(30), right, bottomRow - pad);
    struct Button {
        int id;
        PCWSTR text;
        int width;
    } const buttons[] = {{kLyNew, L"Save current as new", 160}, {kLyApply, L"Apply", 70},
                         {kLyCapture, L"Update from windows", 160}, {kLySave, L"Save", 70},
                         {kLyDelete, L"Delete", 70},         {IDCANCEL, L"Close", 70}};
    int left = pad;
    for (auto const& b : buttons) {
        int bw = px(b.width);
        if (b.id == IDCANCEL) left = right - bw;
        add(L"BUTTON", b.text, WS_TABSTOP | BS_OWNERDRAW, 0, b.id, left, bottomRow, left + bw, bottomRow + button);
        left += bw + px(8);
    }
    add(L"STATIC", L"", SS_LEFT | SS_CENTERIMAGE | SS_ENDELLIPSIS, 0, kLyStatus, pad, height - pad - row, right,
        height - pad);
    FillLayoutsList(0);
    if (g_layouts.empty())
        LayoutsStatus(L"No layouts yet: arrange your windows, then Save current as new.");
    else
        LayoutsStatus(L"The tiling key and 1 to 9 apply the first nine layouts.");
    ShowWindow(g_ly.wnd, SW_SHOW);
    SetForegroundWindow(g_ly.wnd);
    SetFocus(GetDlgItem(g_ly.wnd, kLyList));
}

// The tiling key and a digit: the layout in that place in the list.
void ApplyLayoutNumber(int index) {
    if (index >= 0 && index < (int)g_layouts.size()) ApplyLayout(g_layouts[index].text);
}
////////////////////////////////////////////////////////////////////////////////
// Drawn window borders ("// @inline borders.cpp", after tiling-layouts.cpp, in
// the Switcher), in the manner of Tacky Borders: a border of any width round
// each window, its own colour or gradient for the window in front and for the
// rest, rounded as the window is, set off from its edge, with a glow and a
// shadow, a fade between the two looks and a gradient that turns.
//
// A border is four see-through strips (top, bottom, left, right) kept beside
// the window in the z-order, not one window the size of the window it frames:
// only the pixels near the edge are drawn, kept and sent to DWM. The window's
// moves are followed through Windows' events while the borders are on (as
// Tacky Borders does); a move only moves the strips, and they're drawn again
// only when the size, the look or an animation changes.

struct BorderPaint {
    uint32_t lut[256]{};  // 0xAARRGGBB along the gradient (straight alpha)
    bool gradient = false;
    float sx = 0, sy = 0, ex = 1, ey = 0;  // the gradient's start and end, as fractions of the border's box
};

struct BorderSettings {
    bool on = false;
    int width = 3, offset = -1;
    int radius = -1;  // px at 100%; -1: as the window's own corners
    bool below = false;
    bool followNative = true;
    int initDelay = 200, unminimizeDelay = 150;
    std::wstring activeText, inactiveText;
    int spinActive = 0, spinInactive = 0;  // 1 clockwise, -1 the other way, 0 none
    int spinMs = 1800;
    bool fade = false;
    int fadeMs = 200;
    int fps = 60;
    int glow = 0, shadow = 0;  // kEffectNone, kEffectActive or kEffectAll
    int glowBlur = 8, glowSpread = 0, glowOpacity = 100;
    int shadowBlur = 8, shadowOpacity = 100, shadowX = 0, shadowY = 0;
    std::wstring skipApps, skipClasses, skipTitles;
};

enum { kEffectNone, kEffectActive, kEffectAll };

constexpr WCHAR kBorderClass[] = L"TourneBorder_" WH_MOD_ID;
constexpr UINT_PTR kTimerBorders = 5;  // pending borders and animations
constexpr UINT_PTR kTimerBorderZ = 6;  // windows reordered: borders put back beside theirs

BorderSettings g_borderSettings;
BorderPaint g_activePaint, g_inactivePaint;
HWINEVENTHOOK g_borderHooks[5];
std::unordered_set<HWND> g_borderSkipped;  // windows left out, not asked again while they're open
HWND g_borderFront = nullptr;  // the window in front, wearing the active look
bool g_borderTimer = false;
ULONGLONG g_borderTick = 0;
bool g_borderClass = false;

struct BorderStrip {
    HWND wnd = nullptr;
    RECT rc{};  // in the border's canvas
    std::vector<uint8_t> ring, glow, shadow;  // coverage of the border itself, its glow and its shadow, 0 to 255
    HDC dc = nullptr;
    HBITMAP bitmap = nullptr;
    HGDIOBJ old = nullptr;
    uint32_t* bits = nullptr;
    int bw = 0, bh = 0;  // the bitmap's size
};

struct Border {
    HWND target = nullptr;
    BorderStrip strips[4];
    bool shown = false;
    ULONGLONG showAt = 0;    // shown at this tick (after a delay); 0: not waiting
    RECT frame{};            // the window's frame when last placed
    RECT canvas{};           // the strips' whole area, on the screen
    RECT box{};              // the border's outer edge, in the canvas
    int dpi = 96, w = 0, pad = 0, band = 0;
    float outerR = 0, innerR = 0;
    float mix = 0, mixTarget = 0;  // 1: the active look, 0: the inactive one
    float appear = 1;              // 0 to 1 as it fades in
    float turn = 0;                // the gradient's turn, 0 to 1
};

std::unordered_map<HWND, std::unique_ptr<Border>> g_borders;

////////////////////////////////////////////////////////////////////////////////
// Colours: "accent", "#RGB", "#RGBA", "#RRGGBB" or "#RRGGBBAA"; several make
// a gradient, followed by its direction: an angle ("45deg") or a start and an
// end point ("0,1 1,0", 0,0 the top left and 1,1 the bottom right).

uint32_t BorderAccent() {
    DWORD abgr = 0, size = sizeof(abgr);
    if (RegGetValueW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\DWM", L"AccentColor", RRF_RT_REG_DWORD, nullptr,
                     &abgr, &size) == ERROR_SUCCESS)
        return 0xFF000000 | ((abgr & 0xFF) << 16) | (abgr & 0xFF00) | ((abgr >> 16) & 0xFF);
    DWORD argb = 0;
    BOOL opaque = FALSE;
    if (SUCCEEDED(DwmGetColorizationColor(&argb, &opaque))) return 0xFF000000 | (argb & 0xFFFFFF);
    return 0xFF0078D4;
}

bool ParseBorderColor(std::wstring v, uint32_t* out) {
    CharLowerBuffW(v.data(), (DWORD)v.size());
    if (v == L"accent") {
        *out = BorderAccent();
        return true;
    }
    if (v.empty() || v[0] != L'#') return false;
    v.erase(0, 1);
    if (v.find_first_not_of(L"0123456789abcdef") != std::wstring::npos) return false;
    if (v.size() == 3 || v.size() == 4) {
        std::wstring longer;
        for (WCHAR c : v) longer += std::wstring(2, c);
        v = longer;
    }
    if (v.size() == 6) v += L"ff";
    if (v.size() != 8) return false;
    unsigned long rgba = wcstoul(v.c_str(), nullptr, 16);
    *out = (rgba >> 8) | ((rgba & 0xFF) << 24);
    return true;
}

BorderPaint ParseBorderPaint(std::wstring const& text, uint32_t fallback) {
    std::vector<uint32_t> colors;
    std::vector<std::pair<float, float>> points;
    BorderPaint p;
    bool angled = false;
    float degrees = 0;
    size_t at = 0;
    while (at < text.size()) {
        size_t start = text.find_first_not_of(L" \t,;", at);
        if (start == std::wstring::npos) break;
        size_t end = text.find_first_of(L" \t;", start);
        std::wstring token = text.substr(start, end == std::wstring::npos ? std::wstring::npos : end - start);
        at = end == std::wstring::npos ? text.size() : end;
        while (!token.empty() && token.back() == L',') token.pop_back();
        uint32_t c;
        float x, y;
        if (ParseBorderColor(token, &c)) {
            colors.push_back(c);
        } else if (token.size() > 3 && !_wcsicmp(token.c_str() + token.size() - 3, L"deg")) {
            angled = true;
            degrees = (float)_wtof(token.c_str());
        } else if (swscanf(token.c_str(), L"%f,%f", &x, &y) == 2) {
            points.push_back({x, y});
        }
    }
    if (colors.empty()) colors.push_back(fallback);
    if (colors.size() == 1) {
        std::fill(std::begin(p.lut), std::end(p.lut), colors[0]);
        return p;
    }
    p.gradient = true;
    if (points.size() >= 2) {
        p.sx = points[0].first, p.sy = points[0].second, p.ex = points[1].first, p.ey = points[1].second;
    } else {
        // As Tacky Borders reads an angle: 0deg left to right, 90deg bottom
        // to top, reaching the box's edges.
        double rad = (angled ? degrees : 0.0) * 3.14159265358979 / 180.0;
        double x0 = std::cos(rad), y0 = -std::sin(rad);
        double scale = 0.5 / std::max(std::abs(x0), std::abs(y0));
        p.sx = (float)(0.5 - x0 * scale), p.sy = (float)(0.5 - y0 * scale);
        p.ex = (float)(0.5 + x0 * scale), p.ey = (float)(0.5 + y0 * scale);
    }
    // The stops evenly spread, mixed as they're written (sRGB).
    int n = (int)colors.size();
    for (int i = 0; i < 256; i++) {
        float t = i / 255.0f * (n - 1);
        int k = std::min((int)t, n - 2);
        float f = t - k;
        uint32_t a = colors[k], b = colors[k + 1], out = 0;
        for (int shift = 0; shift < 32; shift += 8) {
            float va = (float)((a >> shift) & 0xFF), vb = (float)((b >> shift) & 0xFF);
            out |= (uint32_t)std::lround(va + (vb - va) * f) << shift;
        }
        p.lut[i] = out;
    }
    return p;
}

void LoadBorderPaints() {
    g_activePaint = ParseBorderPaint(g_borderSettings.activeText, BorderAccent());
    g_inactivePaint = ParseBorderPaint(g_borderSettings.inactiveText, 0x00000000);
}

void LoadBorderSettings() {
    BorderSettings s;
    s.on = Wh_GetIntSetting(L"borders.enabled") != 0;
    s.width = IntSetting(L"borders.width", 0, 64);
    s.offset = IntSetting(L"borders.offset", -64, 64);
    std::wstring radius = TextSetting(L"borders.radius");
    s.radius = radius == L"square" ? 0 : radius == L"round" ? 8 : radius == L"roundSmall" ? 4
             : radius == L"custom" ? IntSetting(L"borders.radiusCustom", 0, 200) : -1;
    s.below = TextSetting(L"borders.zOrder") == L"below";
    s.followNative = Wh_GetIntSetting(L"borders.followNative") != 0;
    s.initDelay = IntSetting(L"borders.initDelay", 0, 5000);
    s.unminimizeDelay = IntSetting(L"borders.unminimizeDelay", 0, 5000);
    s.activeText = TextSetting(L"borders.activeColor");
    s.inactiveText = TextSetting(L"borders.inactiveColor");
    auto spin = [](std::wstring const& v) { return v == L"spiral" ? 1 : v == L"reverseSpiral" ? -1 : 0; };
    s.spinActive = spin(TextSetting(L"borders.activeSpin"));
    s.spinInactive = spin(TextSetting(L"borders.inactiveSpin"));
    s.spinMs = IntSetting(L"borders.spinDuration", 100, 60000);
    s.fade = Wh_GetIntSetting(L"borders.fade") != 0;
    s.fadeMs = IntSetting(L"borders.fadeDuration", 1, 5000);
    s.fps = IntSetting(L"borders.fps", 1, 240);
    auto effect = [](std::wstring const& v) { return v == L"active" ? kEffectActive : v == L"all" ? kEffectAll : kEffectNone; };
    s.glow = effect(TextSetting(L"borders.glow"));
    s.glowBlur = IntSetting(L"borders.glowBlur", 1, 64);
    s.glowSpread = IntSetting(L"borders.glowSpread", 0, 64);
    s.glowOpacity = IntSetting(L"borders.glowOpacity", 0, 100);
    s.shadow = effect(TextSetting(L"borders.shadow"));
    s.shadowBlur = IntSetting(L"borders.shadowBlur", 1, 64);
    s.shadowOpacity = IntSetting(L"borders.shadowOpacity", 0, 100);
    s.shadowX = IntSetting(L"borders.shadowX", -200, 200);
    s.shadowY = IntSetting(L"borders.shadowY", -200, 200);
    s.skipApps = TextSetting(L"borders.skipApps");
    s.skipClasses = TextSetting(L"borders.skipClasses");
    s.skipTitles = TextSetting(L"borders.skipTitles");
    g_borderSettings = s;
    LoadBorderPaints();
}

////////////////////////////////////////////////////////////////////////////////
// Which windows

// A window that may wear a border: an app's own top-level window, not a tool
// window, a menu, the shell's or the suite's, and not left out by the rules.
bool BorderEligible(HWND h) {
    if (!IsWindow(h) || GetAncestor(h, GA_ROOT) != h) return false;
    LONG style = GetWindowLongW(h, GWL_STYLE), ex = GetWindowLongW(h, GWL_EXSTYLE);
    if ((style & WS_CHILD) || (ex & (WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE))) return false;
    DWORD pid = 0;
    GetWindowThreadProcessId(h, &pid);
    if (pid == GetCurrentProcessId()) return false;
    WCHAR cls[128] = L"";
    GetClassNameW(h, cls, ARRAYSIZE(cls));
    if (!wcsncmp(cls, L"Tourne", 6)) return false;
    for (PCWSTR shell : {L"Shell_TrayWnd", L"Shell_SecondaryTrayWnd", L"Progman", L"WorkerW", L"#32768"})
        if (!wcscmp(cls, shell)) return false;
    auto const& s = g_borderSettings;
    if (!s.skipClasses.empty() && MatchesList(s.skipClasses, cls)) return false;
    if (!s.skipTitles.empty() && MatchesList(s.skipTitles, Title(h))) return false;
    if (!s.skipApps.empty()) {
        std::wstring program = ProgramOf(h);
        if (MatchesList(s.skipApps, program.substr(program.find_last_of(L'\\') + 1))) return false;
    }
    return true;
}

// Whether its border shows now: in view, and (following Windows) only while
// Windows would draw a border of its own, so not maximised or full screen.
bool BorderWanted(HWND h) {
    if (!IsWindowVisible(h) || IsIconic(h)) return false;
    DWORD cloaked = 0;
    if (SUCCEEDED(DwmGetWindowAttribute(h, DWMWA_CLOAKED, &cloaked, sizeof(cloaked))) && cloaked) return false;
    if (g_borderSettings.followNative &&
        ((GetWindowLongW(h, GWL_STYLE) & WS_MAXIMIZE) || !(GetWindowLongW(h, GWL_EXSTYLE) & WS_EX_WINDOWEDGE)))
        return false;
    return true;
}

// The radius at 100% the window's own corners have: Windows 11 rounds them
// (8 px, or 4 for small ones) unless a window asks it not to, and squares
// them while a window is snapped.
int AutoRadius(HWND h) {
    using IsArranged_t = BOOL(WINAPI*)(HWND);
    static auto isArranged = (IsArranged_t)GetProcAddress(GetModuleHandleW(L"user32.dll"), "IsWindowArranged");
    if (isArranged && isArranged(h)) return 0;
    DWORD corner = 0;  // DWMWCP_DEFAULT
    if (FAILED(DwmGetWindowAttribute(h, 33 /* DWMWA_WINDOW_CORNER_PREFERENCE */, &corner, sizeof(corner))))
        return 0;  // (Windows 10: square)
    return corner == 1 /* DONOTROUND */ ? 0 : corner == 3 /* ROUNDSMALL */ ? 4 : 8;
}

////////////////////////////////////////////////////////////////////////////////
// Drawing

// The signed distance from a point to a rounded rectangle: negative inside.
float RoundRectDistance(float px, float py, float l, float t, float r, float b, float radius) {
    float hx = (r - l) / 2, hy = (b - t) / 2;
    if (hx <= 0 || hy <= 0) return 1e9f;  // nothing there
    float rr = std::clamp(radius, 0.0f, std::min(hx, hy));
    float qx = std::abs(px - (l + r) / 2) - (hx - rr), qy = std::abs(py - (t + b) / 2) - (hy - rr);
    float outside = std::hypot(std::max(qx, 0.0f), std::max(qy, 0.0f));
    return outside + std::min(std::max(qx, qy), 0.0f) - rr;
}

void FreeStripBitmap(BorderStrip& s) {
    if (s.dc) {
        SelectObject(s.dc, s.old);
        DeleteDC(s.dc);
    }
    if (s.bitmap) DeleteObject(s.bitmap);
    s.dc = nullptr, s.bitmap = nullptr, s.old = nullptr, s.bits = nullptr, s.bw = s.bh = 0;
}

// A band's coverage once blurred (a Gaussian blur, as Tacky Borders' glow
// and shadow): m is the distance from the band's middle, half its half width.
inline float BlurredBand(float m, float half, float sigma) {
    float k = 1.0f / (sigma * 1.41421356f);
    return 0.5f * (std::erf((m + half) * k) - std::erf((m - half) * k));
}

// The border's shape for the window's frame now: the canvas, the strips and
// how much of each pixel the border, its glow and its shadow cover. True if
// it changed.
bool LayOutBorder(Border& b, RECT const& frame) {
    auto const& s = g_borderSettings;
    UINT dpi = MonitorDpi(MonitorFromWindow(b.target, MONITOR_DEFAULTTONEAREST));
    int radius = s.radius < 0 ? AutoRadius(b.target) : s.radius;
    auto scaled = [dpi](float v) { return v * (float)dpi / 96.0f; };
    int w = (int)std::lround(scaled((float)s.width)), offset = (int)std::lround(scaled((float)s.offset));
    float innerR = radius ? scaled((float)radius) : 0, outerR = radius ? innerR + w : 0;
    bool glows = s.glow != kEffectNone && s.glowOpacity > 0, shadows = s.shadow != kEffectNone && s.shadowOpacity > 0;
    float glowSigma = scaled((float)s.glowBlur), glowSpread = scaled((float)s.glowSpread);
    float shadowSigma = scaled((float)s.shadowBlur), shadowX = scaled((float)s.shadowX), shadowY = scaled((float)s.shadowY);
    // Room for the glow and the shadow around the border (outside it and in).
    int pad = 0;
    if (glows) pad = std::max(pad, (int)std::ceil(3 * glowSigma + glowSpread));
    if (shadows) pad = std::max(pad, (int)std::ceil(3 * shadowSigma + std::max(std::abs(shadowX), std::abs(shadowY))));
    int cw = (frame.right - frame.left) + 2 * (w + offset + pad), ch = (frame.bottom - frame.top) + 2 * (w + offset + pad);
    bool same = b.dpi == (int)dpi && b.w == w && b.pad == pad && b.outerR == outerR &&
                b.canvas.right - b.canvas.left == cw && b.canvas.bottom - b.canvas.top == ch;
    b.frame = frame;
    b.canvas = {frame.left - (w + offset + pad), frame.top - (w + offset + pad), 0, 0};
    b.canvas.right = b.canvas.left + cw, b.canvas.bottom = b.canvas.top + ch;
    if (same && !b.strips[0].ring.empty()) return false;
    b.dpi = (int)dpi, b.w = w, b.pad = pad, b.outerR = outerR, b.innerR = innerR;
    b.box = {pad, pad, cw - pad, ch - pad};
    // The strips: deep enough to hold the border where its corners curve in,
    // and its glow and shadow on both sides.
    int band = pad + w + (int)std::ceil(outerR * 0.293f) + pad + 2;
    band = std::max(1, std::min({band, cw / 2, ch / 2}));
    b.band = band;
    b.strips[0].rc = {0, 0, cw, band};
    b.strips[1].rc = {0, ch - band, cw, ch};
    b.strips[2].rc = {0, band, band, ch - band};
    b.strips[3].rc = {cw - band, band, cw, ch - band};
    RECT const& o = b.box;
    float ol = (float)o.left, ot = (float)o.top, orr = (float)o.right, ob = (float)o.bottom;
    float il = ol + w, it = ot + w, ir = orr - w, ib = ob - w;
    float half = w / 2.0f;
    float glowOpacity = s.glowOpacity / 100.0f, shadowOpacity = s.shadowOpacity / 100.0f;
    for (auto& strip : b.strips) {
        int sw = std::max(0, (int)(strip.rc.right - strip.rc.left)), sh = std::max(0, (int)(strip.rc.bottom - strip.rc.top));
        size_t count = (size_t)sw * sh;
        strip.ring.assign(count, 0);
        strip.glow.assign(glows ? count : 0, 0);
        strip.shadow.assign(shadows ? count : 0, 0);
        for (int y = 0; y < sh; y++) {
            float py = strip.rc.top + y + 0.5f;
            for (int x = 0; x < sw; x++) {
                float px = strip.rc.left + x + 0.5f;
                size_t i = (size_t)y * sw + x;
                float dOuter = RoundRectDistance(px, py, ol, ot, orr, ob, outerR);
                float dInner = RoundRectDistance(px, py, il, it, ir, ib, innerR);
                float ring = std::clamp(0.5f - dOuter, 0.0f, 1.0f) * std::clamp(0.5f + dInner, 0.0f, 1.0f);
                strip.ring[i] = (uint8_t)std::lround(ring * 255);
                if (glows) {
                    float g = BlurredBand(std::abs(dOuter + half), half + glowSpread, glowSigma);
                    strip.glow[i] = (uint8_t)std::lround(std::clamp(g * glowOpacity, 0.0f, 1.0f) * 255);
                }
                if (shadows) {
                    float d = RoundRectDistance(px - shadowX, py - shadowY, ol, ot, orr, ob, outerR);
                    float g = BlurredBand(std::abs(d + half), half, shadowSigma);
                    strip.shadow[i] = (uint8_t)std::lround(std::clamp(g * shadowOpacity, 0.0f, 1.0f) * 255);
                }
            }
        }
    }
    return true;
}

// The colour (0xAARRGGBB, straight) a paint gives a point of the canvas, its
// gradient turned by the border's turn.
inline uint32_t PaintAt(BorderPaint const& p, Border const& b, float px, float py, float cosT, float sinT) {
    if (!p.gradient) return p.lut[0];
    float bw = (float)(b.box.right - b.box.left), bh = (float)(b.box.bottom - b.box.top);
    float cx = b.box.left + bw / 2, cy = b.box.top + bh / 2;
    // The point turned back, so the gradient turns forward.
    float dx = px - cx, dy = py - cy;
    float rx = cx + dx * cosT + dy * sinT, ry = cy - dx * sinT + dy * cosT;
    float sx = b.box.left + p.sx * bw, sy = b.box.top + p.sy * bh;
    float ex = b.box.left + p.ex * bw, ey = b.box.top + p.ey * bh;
    float vx = ex - sx, vy = ey - sy, len = vx * vx + vy * vy;
    float t = len > 0 ? ((rx - sx) * vx + (ry - sy) * vy) / len : 0;
    return p.lut[(int)std::lround(std::clamp(t, 0.0f, 1.0f) * 255)];
}

// Draws the strips as the border looks now and gives them to DWM (moved and
// sized too).
void PaintBorder(Border& b) {
    auto const& s = g_borderSettings;
    HDC screen = GetDC(nullptr);
    float turn = b.turn * 6.28318531f, cosT = std::cos(turn), sinT = std::sin(turn);
    float mix = b.mix;
    if (s.fade) mix = mix < 0.5f ? 2 * mix * mix : 1 - 2 * (1 - mix) * (1 - mix);  // ease in and out
    bool glowActive = s.glow != kEffectNone, glowInactive = s.glow == kEffectAll;
    bool shadowActive = s.shadow != kEffectNone, shadowInactive = s.shadow == kEffectAll;
    for (auto& strip : b.strips) {
        int sw = strip.rc.right - strip.rc.left, sh = strip.rc.bottom - strip.rc.top;
        if (!strip.wnd || sw <= 0 || sh <= 0 || strip.ring.size() != (size_t)sw * sh) continue;
        if (strip.bw != sw || strip.bh != sh) {
            FreeStripBitmap(strip);
            BITMAPINFO bi{};
            bi.bmiHeader = {sizeof(BITMAPINFOHEADER), sw, -sh, 1, 32, BI_RGB};
            void* bits = nullptr;
            strip.dc = CreateCompatibleDC(screen);
            strip.bitmap = CreateDIBSection(strip.dc, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
            if (!strip.bitmap) {
                FreeStripBitmap(strip);
                continue;
            }
            strip.old = SelectObject(strip.dc, strip.bitmap);
            strip.bits = (uint32_t*)bits;
            strip.bw = sw, strip.bh = sh;
        }
        for (int y = 0; y < sh; y++) {
            float py = strip.rc.top + y + 0.5f;
            for (int x = 0; x < sw; x++) {
                size_t i = (size_t)y * sw + x;
                float ring = strip.ring[i] / 255.0f;
                float glow = strip.glow.empty() ? 0 : strip.glow[i] / 255.0f;
                float shadow = strip.shadow.empty() ? 0 : strip.shadow[i] / 255.0f;
                if (ring <= 0 && glow <= 0 && shadow <= 0) {
                    strip.bits[i] = 0;
                    continue;
                }
                float px = strip.rc.left + x + 0.5f;
                float r = 0, g = 0, bl = 0, a = 0;
                // One look's pixel, weighted: the border over its glow (both
                // in its colour) over its shadow (black), premultiplied.
                auto add = [&](BorderPaint const& p, float weight, bool glows, bool shadows) {
                    if (weight <= 0) return;
                    float cover = ring + (glows ? glow * (1 - ring) : 0);
                    float dark = shadows ? shadow * (1 - cover) : 0;
                    if (cover > 0) {
                        uint32_t c = PaintAt(p, b, px, py, cosT, sinT);
                        float ca = (c >> 24) / 255.0f * cover * weight;
                        r += ((c >> 16) & 0xFF) * ca, g += ((c >> 8) & 0xFF) * ca, bl += (c & 0xFF) * ca, a += ca;
                    }
                    a += dark * weight;
                };
                add(g_activePaint, mix, glowActive, shadowActive);
                add(g_inactivePaint, 1 - mix, glowInactive, shadowInactive);
                float k = b.appear;
                strip.bits[i] = ((uint32_t)std::lround(std::min(a * k, 1.0f) * 255) << 24) |
                                ((uint32_t)std::lround(std::min(r * k, 255.0f)) << 16) |
                                ((uint32_t)std::lround(std::min(g * k, 255.0f)) << 8) | (uint32_t)std::lround(std::min(bl * k, 255.0f));
            }
        }
        GdiFlush();
        POINT src{0, 0}, dst{b.canvas.left + strip.rc.left, b.canvas.top + strip.rc.top};
        SIZE size{sw, sh};
        BLENDFUNCTION blend{AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
        UpdateLayeredWindow(strip.wnd, screen, &dst, &size, strip.dc, &src, 0, &blend, ULW_ALPHA);
    }
    ReleaseDC(nullptr, screen);
}

////////////////////////////////////////////////////////////////////////////////
// Following the windows

bool IsBorderStrip(Border const& b, HWND h) {
    for (auto const& strip : b.strips)
        if (strip.wnd && strip.wnd == h) return true;
    return false;
}

// The strips put beside the window in the z-order (just above it, or just
// below), moved to the canvas and shown; or hidden.
void PlaceStrips(Border& b, bool show, bool move) {
    auto const& s = g_borderSettings;
    HWND insertAfter = b.target;
    UINT zFlag = 0;
    if (show && !s.below) {
        HWND above = GetWindow(b.target, GW_HWNDPREV);
        bool topmost = GetWindowLongW(b.target, GWL_EXSTYLE) & WS_EX_TOPMOST;
        if (above && IsBorderStrip(b, above))
            zFlag = SWP_NOZORDER;
        else if (!above || (!topmost && (GetWindowLongW(above, GWL_EXSTYLE) & WS_EX_TOPMOST)))
            insertAfter = topmost ? HWND_TOPMOST : HWND_TOP;  // (not among the always-on-top windows above it)
        else
            insertAfter = above;
    } else if (show) {
        HWND next = GetWindow(b.target, GW_HWNDNEXT);
        if (next && IsBorderStrip(b, next)) zFlag = SWP_NOZORDER;
    }
    // Already there: nothing to do (the usual case, as windows reorder).
    if (show && b.shown && !move && zFlag) return;
    auto flagsFor = [&](BorderStrip const& strip) {
        int sw = strip.rc.right - strip.rc.left, sh = strip.rc.bottom - strip.rc.top;
        bool here = show && sw > 0 && sh > 0;
        return SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOSENDCHANGING | zFlag |
               (here ? SWP_SHOWWINDOW : SWP_HIDEWINDOW | SWP_NOZORDER) | (move ? 0 : SWP_NOMOVE);
    };
    // Together, so they move as one; one by one if that's refused. Next to a
    // program running as administrator, Windows won't let them be put in the
    // z-order beside it (Tacky Borders' "Access is denied"): they're moved
    // and shown without it then.
    HDWP defer = BeginDeferWindowPos(4);
    for (auto& strip : b.strips) {
        if (!strip.wnd || !defer) continue;
        defer = DeferWindowPos(defer, strip.wnd, insertAfter, b.canvas.left + strip.rc.left, b.canvas.top + strip.rc.top, 0, 0,
                               flagsFor(strip));
    }
    if (!defer || !EndDeferWindowPos(defer)) {
        for (auto& strip : b.strips) {
            if (!strip.wnd) continue;
            int x = b.canvas.left + strip.rc.left, y = b.canvas.top + strip.rc.top;
            if (!SetWindowPos(strip.wnd, insertAfter, x, y, 0, 0, flagsFor(strip)))
                SetWindowPos(strip.wnd, nullptr, x, y, 0, 0, flagsFor(strip) | SWP_NOZORDER);
        }
    }
    b.shown = show;
}

void StartBorderTimer();

// Brought up to date with its window: hidden if it's not to show, else laid
// out again (drawn again if its shape changed) and put beside it.
void RefreshBorder(Border& b) {
    if (b.showAt) return;  // waiting to show
    if (!BorderWanted(b.target)) {
        if (b.shown) PlaceStrips(b, false, false);
        return;
    }
    RECT frame{};
    if (FAILED(DwmGetWindowAttribute(b.target, DWMWA_EXTENDED_FRAME_BOUNDS, &frame, sizeof(frame))))
        GetWindowRect(b.target, &frame);
    bool wasShown = b.shown;
    bool moved = !EqualRect(&frame, &b.frame);
    if (LayOutBorder(b, frame) || !wasShown) {
        PaintBorder(b);
        PlaceStrips(b, true, true);
    } else if (moved) {
        PlaceStrips(b, true, true);
    } else {
        PlaceStrips(b, true, false);
    }
}

void DestroyBorder(Border& b) {
    for (auto& strip : b.strips) {
        FreeStripBitmap(strip);
        if (strip.wnd) DestroyWindow(strip.wnd);
        strip.wnd = nullptr;
    }
}

Border* FindBorder(HWND h) {
    auto it = g_borders.find(h);
    return it == g_borders.end() ? nullptr : it->second.get();
}

// A window shown (or its border called for): its border made if it has none,
// and shown after the delay given (for Windows' opening animation).
void ShowBorderFor(HWND h, int delay) {
    Border* b = FindBorder(h);
    if (!b) {
        if (g_borderSkipped.contains(h) || GetAncestor(h, GA_ROOT) != h || !BorderWanted(h)) return;
        if (!BorderEligible(h)) {
            if (IsWindow(h)) g_borderSkipped.insert(h);
            return;
        }
        auto made = std::make_unique<Border>();
        made->target = h;
        HINSTANCE instance = (HINSTANCE)GetWindowLongPtrW(g_wnd, GWLP_HINSTANCE);
        for (auto& strip : made->strips) {
            strip.wnd = CreateWindowExW(WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE, kBorderClass,
                                        L"", WS_POPUP, 0, 0, 1, 1, nullptr, nullptr, instance, nullptr);
            if (strip.wnd) KeepOutOfPeek(strip.wnd);
        }
        bool front = h == g_borderFront;
        made->mix = made->mixTarget = front ? 1.0f : 0.0f;
        b = made.get();
        g_borders[h] = std::move(made);
    }
    if (g_borderSettings.fade && !b->shown) b->appear = 0;
    if (delay > 0) {
        b->showAt = GetTickCount64() + delay;
        if (b->shown) PlaceStrips(*b, false, false);
        StartBorderTimer();
        return;
    }
    b->showAt = 0;
    RefreshBorder(*b);
    if (b->appear < 1 || g_borderSettings.spinActive || g_borderSettings.spinInactive) StartBorderTimer();
}

void HideBorderFor(HWND h) {
    if (Border* b = FindBorder(h)) {
        b->showAt = 0;
        if (b->shown) PlaceStrips(*b, false, false);
    }
}

// The window in front changed: the border that wore the active look takes
// the inactive one, and the window now in front's the active one (faded, if
// set).
void SetBorderFront(HWND h) {
    HWND front = h ? GetAncestor(h, GA_ROOT) : nullptr;
    if (front == g_borderFront) return;
    HWND before = g_borderFront;
    g_borderFront = front;
    bool fade = g_borderSettings.fade;
    for (HWND which : {before, front}) {
        Border* b = which ? FindBorder(which) : nullptr;
        if (!b) continue;
        b->mixTarget = which == front ? 1.0f : 0.0f;
        if (!fade) b->mix = b->mixTarget;
        if (b->shown) {
            // The glow may differ between the two looks; the window in front
            // also comes up in the z-order, and its border with it.
            PaintBorder(*b);
            PlaceStrips(*b, true, false);
        }
    }
    if (fade || g_borderSettings.spinActive || g_borderSettings.spinInactive) StartBorderTimer();
}

////////////////////////////////////////////////////////////////////////////////
// Animations and waiting borders, on one timer that runs only while there are
// any (a turning gradient keeps it running).

void StartBorderTimer() {
    if (g_borderTimer || !g_wnd) return;
    g_borderTimer = true;
    g_borderTick = GetTickCount64();
    SetTimer(g_wnd, kTimerBorders, std::max(1, 1000 / g_borderSettings.fps), nullptr);
}

void BordersTimer(HWND wnd) {
    auto const& s = g_borderSettings;
    ULONGLONG now = GetTickCount64();
    float dt = (float)(now - g_borderTick);
    g_borderTick = now;
    bool busy = false;
    for (auto& [h, b] : g_borders) {
        if (b->showAt) {
            if (now >= b->showAt) {
                b->showAt = 0;
                RefreshBorder(*b);
            }
            busy = true;
            continue;
        }
        if (!b->shown) {
            b->mix = b->mixTarget;  // (no fade to see)
            continue;
        }
        bool again = false;
        float step = s.fade ? dt / s.fadeMs : 1.0f;
        if (b->mix != b->mixTarget) {
            b->mix = b->mix < b->mixTarget ? std::min(b->mixTarget, b->mix + step) : std::max(b->mixTarget, b->mix - step);
            again = true;
        }
        if (b->appear < 1) {
            b->appear = std::min(1.0f, b->appear + step);
            again = true;
        }
        int spin = b->mixTarget >= 1 ? s.spinActive : s.spinInactive;
        if (spin) {
            b->turn = std::fmod(b->turn + spin * dt / s.spinMs + 1.0f, 1.0f);
            again = true;
        }
        if (again) {
            PaintBorder(*b);
            busy = true;
        }
    }
    if (!busy) {
        KillTimer(wnd, kTimerBorders);
        g_borderTimer = false;
    }
}

// Windows reordered somewhere: each border shown is put back beside its
// window if it was left behind (a little later, once for a burst).
void BordersReorderTimer(HWND wnd) {
    KillTimer(wnd, kTimerBorderZ);
    for (auto& [h, b] : g_borders)
        if (b->shown) PlaceStrips(*b, true, false);
}

void CALLBACK BorderEventProc(HWINEVENTHOOK, DWORD event, HWND h, LONG object, LONG child, DWORD, DWORD) {
    if (object == OBJID_CURSOR) return;
    switch (event) {
        case EVENT_OBJECT_LOCATIONCHANGE:
            if (object != OBJID_WINDOW || child != CHILDID_SELF || !h) return;
            if (Border* b = FindBorder(h)) RefreshBorder(*b);
            else ShowBorderFor(h, 0);  // (back from full screen, say)
            return;
        case EVENT_OBJECT_REORDER:
            if (object == OBJID_CLIENT && !g_borders.empty() && g_wnd) SetTimer(g_wnd, kTimerBorderZ, 16, nullptr);
            return;
        case EVENT_SYSTEM_FOREGROUND: {
            HWND fg = GetForegroundWindow();
            SetBorderFront(fg ? fg : h);
            return;
        }
        case EVENT_OBJECT_SHOW:
        case EVENT_OBJECT_UNCLOAKED:
            if (object == OBJID_WINDOW && child == CHILDID_SELF && h)
                ShowBorderFor(h, FindBorder(h) ? 0 : g_borderSettings.initDelay);
            return;
        case EVENT_OBJECT_HIDE:
        case EVENT_OBJECT_CLOAKED:
            if (object == OBJID_WINDOW && child == CHILDID_SELF && h) HideBorderFor(h);
            return;
        case EVENT_SYSTEM_MINIMIZESTART:
            HideBorderFor(h);
            return;
        case EVENT_SYSTEM_MINIMIZEEND:
            ShowBorderFor(h, g_borderSettings.unminimizeDelay);
            return;
        case EVENT_OBJECT_DESTROY:
            if (object == OBJID_WINDOW && child == CHILDID_SELF && h) {
                auto it = g_borders.find(h);
                if (it != g_borders.end()) {
                    DestroyBorder(*it->second);
                    g_borders.erase(it);
                }
                if (h == g_borderFront) g_borderFront = nullptr;
                g_borderSkipped.erase(h);
            }
            return;
    }
}

void StopBorders() {
    for (auto& hook : g_borderHooks) {
        if (hook) UnhookWinEvent(hook);
        hook = nullptr;
    }
    if (g_wnd) {
        KillTimer(g_wnd, kTimerBorders);
        KillTimer(g_wnd, kTimerBorderZ);
    }
    g_borderTimer = false;
    for (auto& [h, b] : g_borders) DestroyBorder(*b);
    g_borders.clear();
    g_borderSkipped.clear();
    g_borderFront = nullptr;
}

// On the switcher's thread. Nothing is followed while the borders are off.
void StartBorders() {
    StopBorders();
    if (!g_borderSettings.on || !g_wnd) return;
    if (!g_borderClass) {
        WNDCLASSW wc{};
        wc.lpfnWndProc = DefWindowProcW;  // (nothing of this module's: see-through, never clicked)
        wc.hInstance = (HINSTANCE)GetWindowLongPtrW(g_wnd, GWLP_HINSTANCE);
        wc.lpszClassName = kBorderClass;
        g_borderClass = RegisterClassW(&wc) != 0;
    }
    DWORD flags = WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS;
    g_borderHooks[0] = SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, nullptr, BorderEventProc, 0, 0, flags);
    g_borderHooks[1] = SetWinEventHook(EVENT_SYSTEM_MINIMIZESTART, EVENT_SYSTEM_MINIMIZEEND, nullptr, BorderEventProc, 0, 0, flags);
    g_borderHooks[2] = SetWinEventHook(EVENT_OBJECT_DESTROY, EVENT_OBJECT_REORDER, nullptr, BorderEventProc, 0, 0, flags);
    g_borderHooks[3] = SetWinEventHook(EVENT_OBJECT_LOCATIONCHANGE, EVENT_OBJECT_LOCATIONCHANGE, nullptr, BorderEventProc, 0, 0, flags);
    g_borderHooks[4] = SetWinEventHook(EVENT_OBJECT_CLOAKED, EVENT_OBJECT_UNCLOAKED, nullptr, BorderEventProc, 0, 0, flags);
    HWND fg = GetForegroundWindow();
    g_borderFront = fg ? GetAncestor(fg, GA_ROOT) : nullptr;
    // The windows open now, at once (no opening animation to wait for).
    std::vector<HWND> open;
    EnumWindows(
        [](HWND h, LPARAM param) __stdcall -> BOOL {
            ((std::vector<HWND>*)param)->push_back(h);
            return TRUE;
        },
        (LPARAM)&open);
    for (HWND h : open) ShowBorderFor(h, 0);
}

// Windows' accent colour changed: borders that use it take the new one.
void BordersAccentChanged() {
    if (g_borders.empty()) return;
    LoadBorderPaints();
    for (auto& [h, b] : g_borders)
        if (b->shown) PaintBorder(*b);
}

// The switcher closing: the borders go, and their window class with them.
void EndBorders() {
    StopBorders();
    if (g_borderClass && g_wnd) UnregisterClassW(kBorderClass, (HINSTANCE)GetWindowLongPtrW(g_wnd, GWLP_HINSTANCE));
    g_borderClass = false;
}

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
    if (TilingKeyHook(wParam, k) > 0) return 1;
    bool down = wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN;
    bool alt = k->flags & LLKHF_ALTDOWN;
    bool shift = GetAsyncKeyState(VK_SHIFT) < 0;
    bool ctrl = GetAsyncKeyState(VK_CONTROL) < 0;
    DWORD vk = k->vkCode;
    WORD flags = (ctrl ? kFlagSticky : 0) | (WinDown() ? kFlagAllMonitors : 0);

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
    g_keyHook = SetWindowsHookExW(WH_KEYBOARD_LL, KeyboardProc, instance, 0);
    if (!g_keyHook) Wh_Log(L"Couldn't catch Alt+Tab: %u", GetLastError());
    SetEvent(ready);
    // Windows drops a hook that was too slow a few times, without a word; it's
    // put back now and then, which also keeps it ahead of hooks set since.
    UINT_PTR renew = SetTimer(nullptr, 0, 5000, nullptr);
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        if (!msg.hwnd && msg.message == WM_TIMER && msg.wParam == renew) {
            if (!g_open) {
                if (HHOOK fresh = SetWindowsHookExW(WH_KEYBOARD_LL, KeyboardProc, instance, 0)) {
                    if (g_keyHook) UnhookWindowsHookEx(g_keyHook);
                    g_keyHook = fresh;
                }
            }
            continue;
        }
        if (msg.hwnd || msg.message != WM_APP_WATCHWHEEL) continue;
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
    PublishChoices();
    SetBackdrop(wnd);
    DWORD corner = g_settings.acrylic && g_settings.radius > 0 ? 2 /* DWMWCP_ROUND */ : 1 /* DWMWCP_DONOTROUND */;
    DwmSetWindowAttribute(wnd, 33 /* DWMWA_WINDOW_CORNER_PREFERENCE */, &corner, sizeof(corner));
    COLORREF none = 0xFFFFFFFE;  // DWMWA_COLOR_NONE: the border is drawn here
    DwmSetWindowAttribute(wnd, 34 /* DWMWA_BORDER_COLOR */, &none, sizeof(none));
    WatchWheel(false);
}

LRESULT CALLBACK WndProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_APP_STEP: {
            int dir = (int)(INT_PTR)wParam;
            WORD from = LOWORD(lParam), flags = HIWORD(lParam);
            bool backtick = from == kFromBacktick;
            if (!g_open) {
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
            int dir = ((short)wParam > 0) != g_settings.reverseWheel ? -1 : 1;
            bool page = g_settings.wheelPages != (lParam != 0);
            Show(wnd);
            Step(wnd, page ? dir * std::max(1, g_m.rows * g_m.columns) : dir);
            return 0;
        }
        case WM_APP_MOVE: {
            if (!g_open) return 0;
            int dx = (int)(INT_PTR)wParam, dy = (int)lParam;
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
            return 0;
        case WM_TIMER:
            if (wParam == kTimerTile) TileTimer(wnd);
            if (wParam == kTimerPlaces) PlacesTimer(wnd);
            if (wParam == kTimerBorders) BordersTimer(wnd);
            if (wParam == kTimerBorderZ) BordersReorderTimer(wnd);
            if (wParam == kTimerShow) Show(wnd);
            if (wParam == kTimerPeek) {
                KillTimer(wnd, kTimerPeek);
                if (g_shown && g_sel >= 0 && g_sel < (int)g_entries.size())
                    LivePeek(g_entries[g_sel].windows[0], wnd, kPeekAltTab);
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
            StopTileEvents();
            EndBorders();
            if (g_placesDirty) SavePlaces();
            if (g_ly.wnd) DestroyWindow(g_ly.wnd);
            Hide(wnd);
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
    UnregisterClassW(kWindowClass, instance);
    UnregisterClassW(kOverlayClass, instance);
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
