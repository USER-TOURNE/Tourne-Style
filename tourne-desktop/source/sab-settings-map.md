# StartAllBack's settings, mapped

Taken from StartAllBack 3.9's own files: the page forms and translation
table in `StartAllBackCfg.exe`, and the value names in it and
`StartAllBackX64.dll`. Values live in `HKCU\Software\StartIsBack` unless
noted. Text is SAB's exact English. "Ours" is where the suite has it.

## Welcome ("Where do you want to go today?" / "Select theme to apply")

Each preset writes OrbBitmap, AlterStyle, TaskbarStyle, FrameStyle,
SysTrayStyle, BottomDetails.

| Preset | Lines | Styles (Start, taskbar) | Orb | FrameStyle | SysTrayStyle | BottomDetails |
|---|---|---|---|---|---|---|
| Proper 11 | - Round Start menu and taskbar elements / - Corner taskbar icons open Win11 flyouts if possible / - New Command Bar in Windows Explorer | Default (none) | default | 0 | 0 | 0 |
| Kinda 10 | - Square Start Menu and taskbar elements / - Corner taskbar icons open Win10 flyouts if possible / - Ribbon UI in Windows Explorer | Plain8.msstyles | w8logo.svg | 1 | 1 | 0 |
| Remastered 7 | - Aero Start Menu and taskbar style / - Corner taskbar icons open Win7 flyouts if possible / - Simple UI and details on bottom in Windows Explorer | Windows 7.msstyles | Windows 7.orb | 2 | 2 | 1 |

Footer: "You can tweak functionality and change style separately on other
pages." / "To get back to these settings, right-click Start button and
choose Properties."

(The Default/none values for Proper 11 are inferred: the constants in the
preset code are only Plain8, Windows 7, w8logo.svg and Windows 7.orb.)

## Start Menu ("Configure Start menu items and behavior")

- Use enhanced classic start menu
- Visual style: (AlterStyle; "Default" or a Styles\*.msstyles)
- Icon size: S / L / M (list order as in the form)
- Icon count: 0-60, default 10 (Start_MinMFU)
- My Programs: Use large icons (Start_LargeMFUIcons)
- All Programs: Use large icons (Start_LargeAllAppsIcons); Display as flyout
  menu (Windows XP style) (AllProgramsFlyout); Display Modern Apps in a
  folder (StartMetroAppsFolder); Override sort order (Start_SortOverride):
  Items first / Folders first / No preference
- Search: Use large icons (Start_LargeSearchIcons); Search programs and
  settings; Search public folders; Use search flyout for more results
  (Start_SearchPrograms, Start_SearchPCSettings, Start_SearchFiles,
  Start_AskCortana: exact pairing unconfirmed)
- General: Highlight newly installed programs (Start_NotifyNewApps); Open
  submenus when I pause on them with the mouse pointer (Start_AutoCascade)
- Right side items: Display user account picture (HideUserFrame, inverted);
  Display glyph icons (Start_RightPaneIcons); Use multi-column flyout menus
  for right side items (MultiColumnFlyout); "Custom folders..." (StartPage2
  CustomFolders; dialog "Add custom folders to the right side of the Start
  Menu", Add... / Delete / Change glyph icon... / Use large icons
  (Start_LargeFlyoutIcons))
- Right side items, each Don't display / Link / Menu (0 / 1 / 2):
  (user) Start_ShowUser, Documents Start_ShowMyDocs, Pictures
  Start_ShowMyPics, Music Start_ShowMyMusic, Videos Start_ShowVideos,
  Downloads Start_ShowDownloads, OneDrive Start_ShowSkyDrive, Favorites
  StartMenuFavorites (no Link), Recent Items Start_ShowRecentDocs (no Link),
  Network Start_ShowNetPlaces (no Menu), Network Connections
  Start_ShowNetConn, This PC Start_ShowMyComputer, Control Panel
  Start_ShowControlPanel, Settings Start_ShowPCSettings, Windows Tools
  Start_AdminToolsRoot (no Link), Devices and Printers Start_ShowPrinters,
  Default Programs Start_ShowSetProgramAccessAndDefaults (no Menu), Windows
  Terminal Start_ShowTerminal (no Menu), Command Prompt
  Start_ShowCommandPrompt (no Menu), Run Start_ShowRun (no Menu)
- Power button default action: (Start_PowerButtonAction)
- Set default values ("Resets Start menu displayed items and settings to
  default values")

## Taskbar ("Choose Taskbar style" / "Tweak behavior and superpowers")

- Use enhanced classic taskbar (LegacyTaskbar)
- Visual style: Default / Styles\*.msstyles (TaskbarStyle)
- Icon size: S / M / L / XL (TaskbarIconSize)
- Icon margins: XS / S / M / L / XL (TaskbarSpacierIcons)
- Orbs list, Add... / Delete (OrbBitmap)
- Combine taskbar buttons: Always, hide labels / When taskbar is full /
  Never / Never, hide labels / Never group (TaskbarGlomLevel)
- On secondary taskbars: the same less "Never group" (MMTaskbarGlomLevel)
- Dynamic transparency (TaskbarTranslucentEffect)
- Centered task icons: Separately from start button / Together with start
  button (TaskbarCenterIcons)
- Segments: Up to three segments / Just central segment
  (TaskbarOneSegment); Dynamic segments
- Taskbar location on screen: Left / Top / Right / Bottom (TaskbarPosition)
- Automatic hiding (Win+Alt+T); Dynamic shyness
- Plumpness: Buffer / Aura (FatTaskbar); Dynamic aura
- Taskbar settings (link)
- "Changing taskbar type requires restarting Explorer shell" / Restart now

## Tray icons ("Choose flyouts style")

- Flyouts: Windows 11 / Windows 10 / Windows 7 (SysTrayStyle); "Corner
  icons open:" Windows 11 flyouts / Windows 10 flyouts if possible /
  Windows 7 flyouts if possible
- System icons, each on/off (search: box / icon / hidden; touch keyboard:
  auto / always / never; clock: format Day, Short Date, Seconds; font
  default/custom; position left/right; battery "New icons")
- Always show all app icons (EnableAutoTray), per-app list
- Icon margins: XS..XL (SysTraySpacierIcons)
- On secondary taskbars: on / off (MinimalSecondarySysTray)

## Explorer ("Choose Explorer style" / "Settings apply to new windows")

- FrameStyle: Win11 Command Bar / Win10 Ribbon UI / Win7 Command Bar (0/1/2)
- Mica effect on top (NavBarGlass)
- Classic search box (OldSearch)
- Details pane on bottom (BottomDetails)
- New icons (RestyleIcons)
- Classic drive grouping (DriveGrouping)
- Make it top notch: Classic context menus (NoXAMLMenus); Colorize
  everything with accent color (RestyleControls); Restore Control Panel
  applets (UndeadControlPanel)

## Advanced

- Customize appearance: Use custom start menu coloring (StartMenuColoring,
  StartMenuColor, StartMenuAlpha 0-255, StartMenuBlur: off / on / more /
  most / acrylic); Use custom taskbar coloring (TaskbarColoring,
  TaskbarColor, TaskbarAlpha, TaskbarBlur)
- Tweak Windows settings: Store and display recently opened programs
  (Start_TrackProgs) + Clear list; Store and display recently opened items
  (Start_TrackDocs) + Clear list; Use new font for classic UI; Number of
  jumplist items to display: 0-30, default 10 (Start_JumpListItems)
- Disable StartAllBack: Disable program for current user (Disabled)
