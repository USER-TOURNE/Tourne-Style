# Which part of Windows a visual style class paints. A class "App::Base" is the Base class as the program
# App asks for it (SetWindowTheme(window, "App", ...)); properties it doesn't set come from Base (the style's
# BCMAP), so a derived class holds only what differs. Classes whose App starts with DarkMode are the ones
# programs switch to in dark mode; they're marked [dark].

function Get-Surface([string]$cls) {
    $parts = $cls -split '::'
    $base = $parts[-1]
    $app = if ($parts.Count -gt 1) { $parts[0] } else { '' }
    $dark = $app -match '^DarkMode' -or $cls -match '^DarkMode'
    $tag = if ($dark) { ' [dark]' } else { '' }

    if ($cls -match '^(globals|sysmetrics|documentation)$' -or $cls -match '^(sizevariant|colorvariant)\.') {
        return 'Theme-wide: system colours, fonts, sizes'
    }
    if ($base -match '^DWM(Window|Pen|Touch)$') { return 'Window frames: DWM title bars and buttons' }
    if ($base -match '^(Window|WindowStyle)$') { return 'Window frames: classic (DWM off, MDI, some apps)' }
    if ($app -match 'HC(Black|White)$') { return 'High contrast' }
    if ($base -match '^(TaskBar|TaskBand|TaskBand2|Taskband2|TaskbandExtendedUI|TaskbarPearl|TaskbarShowDesktop|TrayNotify|TrayNotifyFlyout)$' -or
        $app -match '^(TaskBar|TaskBand|TrayNotify)') {
        return 'Taskbar and tray (classic: StartAllBack, ours)'
    }
    if ($base -match '^(StartPanel|StartPanelPriv|StartPanelComposited|StartPanelDark)$' -or $app -match '^(StartMenu|StartPanel)' -or
        $app -match 'ImmersiveStart') {
        return "Start menu$tag"
    }
    if ($base -match '^(TaskManager|ChartView)$') { return "Task Manager$tag" }
    if ($app -match '^(Media|Communications)' -or $base -match '^(MediaStyle|CommunicationsStyle)$') { return 'Old Media Player and Messenger' }
    if ($base -match '^(Menu|MenuStyle)$') { return "Menus (every program)$tag" }
    if ($app -eq 'Desktop') { return 'Desktop icons' }
    $explorerBases = '^(ItemsView|Navigation|PreviewPane|ReadingPane|CommandModule|AddressBand|BreadcrumbBar|SearchBox|SearchEditBox|' +
                     'SearchHome|HelpSearchBox|ExplorerBar|ExplorerBarStyle|ExplorerNavPane|ExplorerStatusBar|FileExplorerBannerContainer|' +
                     'ProperTree|InfoBar|EmptyMarkup|ShellStatusBarSeparator|CommonItemsDialog|ControlPanel|ControlPanelStyle|BarrierPage|' +
                     'TryHarder|MenuBand)$'
    $explorerApps = '(Explorer|ItemsView|Address|^BB|BBComposited|^Go|GoComposited|Navbar|SearchBox|SearchButton|^AB|ABComposited|InfoPane|' +
                    'PreviewPaneControl|Placesbar|LVPopup|CFD|ITBarBase|BrowserTab|UISubsetFooter|CPLCommandModule|AutoComplete|^Help$|' +
                    '^Default$|^Alternate|AccessibleToolbar|Breadcrumb|MaxBB|InactiveBB)'
    if ($base -match $explorerBases -or $app -match $explorerApps) { return "Explorer, Open/Save, Control Panel$tag" }
    if ($base -match 'Style$') { return 'Control animations (*Style classes)' }
    $controls = '^(Button|Edit|Combobox|ComboBox|Listbox|ListView|Header|Tab|Toolbar|Rebar|Progress|TrackBar|Spin|Status|Tooltip|TreeView|' +
                'ScrollBar|DatePicker|MonthCal|Link|Static|TaskDialog|AeroWizard|TextStyle|DragDrop|Pause|Flyout|TextGlow|' +
                'TextSelectionGripper|ListViewPopup|DropListControl|CopyClose)$'
    if ($base -match $controls) { return "Controls in programs$tag" }
    return "Other$tag"
}
