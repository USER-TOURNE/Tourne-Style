# How StartAllBack themes the system

Taken from StartAllBack 3.9 on this machine: strings and resources in
`StartAllBackX64.dll` (runs inside Explorer) and `DarkMagicX64.dll` (injected
into other programs), your `HKCU\Software\StartIsBack` values, and the two
styles rendered part by part (`Bouquet SAB.msstyles`,
`Bouquet MAC - Night.msstyles`).

## 1. Two styles, never one

- **The SAB style** (`AlterStyle` for the Start menu, `TaskbarStyle` for the
  bar) holds only three classes: `StartPanelPriv`, `TaskBar`/`TaskBand2` and
  the tray. It is opened under app names picked by mode and edge:
  `StartPanelDark`, `StartPanelLight`, `StartPanelComposited`,
  `StartPanelCompositedBottom`, `StartMenuComposited`, and
  `Taskbar2Composited::TaskBar`, `Taskband2Composited::TaskBand2`,
  `TaskBand2CompositedSmallIcons::TaskBand2`, `TaskbarVert(Light)`,
  `TaskBar(Light)`, `TaskBarSmallIcons(Light)` and the HC variants,
  `TrayNotify%ws%ws::Button`, `TrayNotify::Clock`, `ShowDesktop::Button`,
  `VerticalShowDesktop::Button`.
- **Everything else comes from the Windows style you're running**, even
  inside the Start menu:
  - **Menus:** `ImmersiveStart::Menu` and `DarkMode::Menu`. Part 14 is the
    item (pink hover), 15 the separator, 11 the tick, 12 the tick's
    background.
  - **Start menu cascades:** `StartMenu::Toolbar` and `StartMenu::MenuBand`.
  - **Start search box:** `DarkMode::SearchBox`, where part 1 draws nothing
    in state 1, a border in 2, hatching in 3, and hatching with the pink
    underline in 4. `EditComposited` and `SearchBoxEditComposited::Edit` are
    also used here.
  - **Pane names:** `MoreProgramsPane`, `AllProgramsPane`, `LogoffPane`.
  - **Tooltips:** `Explorer::Tooltip`, `tooltips_class32`.

## 2. Start menu specifics

- **Text:** the system icon title font (here scientificaCAPS 12, hence the
  capitals), not the style's font (that's Segoe UI Bold on parts 2, 3, 18
  and 19).
- **Icons:** from the shell's system image lists at their own size
  (`Shell_GetCachedImageIndexW`, `ImageList_Draw`).
- **Layout:** it asks for sizes by name: `StartPane_LeftWidth`,
  `StartPane_RightWidth`, `StartPane_JumpListWidth`, `SearchX`, `SearchY`,
  `SearchRightPad`, `SearchHeight`, `ContentRight`, `PartHeight`,
  `StartPanel.BottomBar`, `StartPane_MinHeight`.
- **Cached in `Software\StartIsBack\Cache`:** `IdealWidth.%d` and
  `IdealHeight.%d`, plus `OrbWidth` and `OrbHeight` per DPI (54 px at 96).
- **Right column:** policies (`NoSMMyDocs`, `NoControlPanel` and the rest)
  are honoured next to each `Start_Show*` value.
- **Colours:** `StartMenuColor` / `StartMenuAlpha` and `CustomColors`
  (`ColorA=211C12` here), `StartMenuBlur`.
- **Fades:** `StartPanel_FadeDelay`, `FadeIn` and `FadeOut`.
- **Measured against the video (100% scale):**
  - The window starts at the screen edge. It has an 18 px see-through margin
    on the left and 3 px on top.
  - The panes have a 3 px bottom border, and the bottom bar is 45 px.
  - The left pane is 244 px from its border; the right pane is 150 px.
  - The bottom sits 4 px above the bar top (the bar is at 1042 in its own
    `Taskbaz\Settings` too).

## 3. Explorer (inside Explorer)

- **Layouts:** it swaps them through dui70 `SetXMLFromResourceWithTheme`.
  It carries its own `UIFILE_DETAILS` 3 to 6 and 19 to 21, and uses them
  only when `BottomDetails = 1`:
  - The details pane is along the bottom at 53 rp.
  - The reading pane sits at top level on the right, with no width of its
    own. Its sizer has `GrowTargetFirst`, `PrioritySizer =
    PageSpaceControlSizer` and `FramePersistType = 2`. That is what makes
    Alt+P resizable both ways.
- **Folder band:** its own UIFILE 32, 1032 and 2032 (Overflow, Help and
  Preview buttons).
- **Classes it styles:**
  - Toolbars and frames: `BBComposited`, `InactiveBBComposited`,
    `MaxBBComposited`, `MaxInactiveBBComposited`, `BBComposited::Toolbar`,
    `BB::Toolbar`, `CompositedWindow::Window`.
  - Address and search: `AddressComposited`, `BreadcrumbBar`, `AddressBand`,
    `SearchBox`, `SearchBoxHost`, `NativeSearchBox`.
  - Lists and trees: `Explorer::TreeView`, `Explorer::ItemsView`,
    `Explorer::ListView`, `ItemsView::ListView`, `ItemsView::Header`,
    `Explorer::Button`.
  - Dark variants: `DarkMode::PreviewPane`, `DarkMode_Explorer::ScrollBar`,
    `DarkMode_ItemsView`.
  - Control Panel and dialogs: `CPLCommandModule::CommandModule`,
    `ControlPanelStyle`, `TaskDialogStyle`, `AeroWizardStyle`, `TextStyle`.
- **The hatched hover:** it is the system style's own ItemsView/ListView part
  1 (hot state hatched, pressed state framed pink) and TreeView part 1 (a
  coloured bar on the left). StartAllBack only makes sure those classes are
  used.
- **Ribbon and command bar:** `UIRibbon.dll` with SVG icons from
  `StartAllBack\Ribbon\theme-dark|light`. Command names: `windows.*`,
  `Windows.MultiVerb.*`.
- **Drive grouping:** localised names for Hard Disk Drives, Devices with
  Removable Storage, Other, Scanners and Cameras, Portable (Media) Devices.
- **Values:** `FrameStyle`, `NoCommandBar`, `BottomDetails`, `NavBarGlass`,
  `ClientAreaOnlyRibbon`, `NoXAMLMenus`, `RestyleIcons`, `UndeadControlPanel`.

## 4. DarkMagic (every other program)

- **Injection:** loaded by `DarkMagicLoaderX64/X86.exe` through a shell hook,
  and passed on to children by hooking `CreateProcessInternalW`.
- **Hooks:**
  - Theme calls: `OpenThemeData(ForDpi)`, `SetWindowTheme`, `GetWindowTheme`,
    `DrawThemeBackground`, `DrawThemeText(Ex)`, `GetThemeMargins`,
    `GetThemeFont`, `GetThemeInt`, `GetThemeMetric`, `GetThemeBitmap`,
    `IsThemeBackgroundPartiallyTransparent`, `SetWindowThemeAttribute`.
  - Text and colour: `ExtTextOutW`, `DrawTextW/ExW`, `SetTextColor`,
    `GetSysColor`, `FillRect`, `DrawEdge`.
  - Windows and metrics: `GetSystemMetrics(ForDpi)`, `CreateWindowExW`,
    `RegisterClass(Ex)W`, `SetWindowRgn`, `AnimateWindow`.
  - Menus and icons: menu info calls, `LoadIconWithScaleDown`.
- **Classes it forces:** `DarkMode_Explorer::Button`,
  `DarkMode_Explorer::ScrollBar`, `Explorer::Scrollbar`, `DarkMode_ItemsView`,
  `AddressComposited`, `BBComposited`, tooltips, `TaskDialogStyle`,
  `Progress`, and the TrackBar, Tab, StatusBar, Toolbar, ReBar, ComboBoxEx
  and ComboLBox controls.
- **Settings:**
  - `Software\StartIsBack\DarkMagic` (empty here, so defaults):
    `FadeDisabled`, `CompactMenus` (SIBTouchMenu), `ImmersiveMenus`,
    `NoDarkTooltips`, `Unround`, `DynamicScrollbars`.
  - Shared with the main values: `RestyleControls`, `RestyleIcons`,
    `DriveGrouping`.
  - Colour overrides: `Hilight`, `HotTrackingColor` and `MenuHilight`
    through `Software\StartIsBack\Recolor` (absent here).
- **Theme check:** it reads `ThemeManager\DllName` to see whether you're on
  aero.msstyles.

## 5. What this machine adds on top (not StartAllBack)

- **`remove-context-menu-items`** (explorer.exe only): removes Pin to Quick
  access, Include in library, Paste, Display settings, Personalize, Edit with
  Photos/Paint, and the Copilot/OneDrive/Designer/Clipchamp items.
- **`dark-menus`** (every process, ForceDark) and **`classic-menus`** (every
  process): these are what make Win32 menus dark and classic, ours included.
- **`explorer-visual-tweaks-dark`:** draws the red selection boxes over the
  style's hatched hover.
- **Tourne Explorer Style** has `detailsPane = right`. StartAllBack runs with
  `BottomDetails = 1`, which is the layout with the resizable reading pane.

## 6. Where Tourne stands

| Area | Status |
| --- | --- |
| StartPanelPriv parts, frame, bottom bar | matched (measured) |
| Search box (`DarkMode::SearchBox` states 1/2/4) | matched |
| Start text font (icon title font) | matched |
| Start icons (system image list) | matched |
| Right-click menus (Menu parts 14/15/11, menu font, text colour, no icons) | matched, drawn by us so Text Color and Menu No Icons apply |
| StartAllBack's shorter item list on folder menus | not yet: its rule isn't in its strings |
| Cascading flyouts as MenuBand toolbars (`StartMenu::Toolbar`) | not yet (ours are menus) |
| Reading pane sizer | matched; needs Details pane = Bottom, as StartAllBack's |
| DarkMagic-style restyling of other programs | built: Dark Magic part (controls, tooltips, title bars, menu bars, dialogs, menu mode), untested live |
| Recolor (`Hilight`/`HotTrackingColor`/`MenuHilight`) | not started |
