# Shared by Build-TourneSuiteRedirect.ps1 (this PC's StartAllBack, for the build's defaults) and the
# tester kit's Fill-MySettings.ps1 (the tester's StartAllBack, on their PC).

# StartAllBack's Start menu settings (HKCU\Software\StartIsBack), as this
# suite's taskbar settings name them.
function Get-SabDefaults {
    $values = @{}
    $sab = Get-ItemProperty 'HKCU:\Software\StartIsBack' -ErrorAction SilentlyContinue
    if (-not $sab) { return $values }
    $get = { param($name) $p = $sab.PSObject.Properties[$name]; if ($p) { $p.Value } else { $null } }
    $flag = { param($key, $name, [switch]$invert) $v = & $get $name; if ($null -ne $v) { $values[$key] = [int]((([int]$v) -ne 0) -xor $invert.IsPresent) } }
    $v = & $get 'Start_LargeMFUIcons'; if ($null -ne $v) { $values['startMenu.iconSize'] = @('small', 'large', 'medium')[[Math]::Min([int]$v, 2)] }
    $v = & $get 'Start_MinMFU'; if ($null -ne $v) { $values['startMenu.iconCount'] = [int]$v }
    & $flag 'startMenu.allPrograms.largeIcons' 'Start_LargeAllAppsIcons'
    & $flag 'startMenu.allPrograms.flyout' 'AllProgramsFlyout'
    & $flag 'startMenu.allPrograms.modernAppsFolder' 'StartMetroAppsFolder'
    & $flag 'startMenu.search.largeIcons' 'Start_LargeSearchIcons'
    & $flag 'startMenu.general.highlightNew' 'Start_NotifyNewApps'
    & $flag 'startMenu.general.autoCascade' 'Start_AutoCascade'
    & $flag 'startMenu.rightSide.userPicture' 'HideUserFrame' -invert
    & $flag 'startMenu.rightSide.glyphIcons' 'Start_RightPaneIcons'
    & $flag 'startMenu.rightSide.multiColumn' 'MultiColumnFlyout'
    & $flag 'startMenu.rightSide.largeFlyoutIcons' 'Start_LargeFlyoutIcons'
    # The right side: 0 Don't display, 1 Link, 2 Menu; items with only one
    # way to show take any other value as that.
    $items = [ordered]@{
        user = 'Start_ShowUser'; documents = 'Start_ShowMyDocs'; pictures = 'Start_ShowMyPics'; music = 'Start_ShowMyMusic'
        videos = 'Start_ShowVideos'; downloads = 'Start_ShowDownloads'; oneDrive = 'Start_ShowSkyDrive'
        favorites = 'StartMenuFavorites'; recentItems = 'Start_ShowRecentDocs'; network = 'Start_ShowNetPlaces'
        networkConnections = 'Start_ShowNetConn'; computer = 'Start_ShowMyComputer'; controlPanel = 'Start_ShowControlPanel'
        settings = 'Start_ShowPCSettings'; windowsTools = 'Start_AdminToolsRoot'; devicesAndPrinters = 'Start_ShowPrinters'
        defaultPrograms = 'Start_ShowSetProgramAccessAndDefaults'; terminal = 'Start_ShowTerminal'
        commandPrompt = 'Start_ShowCommandPrompt'; run = 'Start_ShowRun'
    }
    $menuOnly = 'favorites', 'recentItems', 'windowsTools'
    $linkOnly = 'network', 'defaultPrograms', 'terminal', 'commandPrompt', 'run'
    foreach ($item in $items.Keys) {
        $v = & $get $items[$item]; if ($null -eq $v) { continue }
        $v = [int]$v
        $values["startMenu.rightSide.$item"] = if ($v -eq 0) { 'dontDisplay' } elseif ($item -in $menuOnly) { 'menu' } elseif ($item -in $linkOnly) { 'link' } elseif ($v -eq 2) { 'menu' } else { 'link' }
    }
    # Taskbar page: TaskbarTranslucentEffect is bits (1 transparency, 2 segments, 4 aura); FatTaskbar 0/1/2 = none/buffer/aura.
    $v = & $get 'TaskbarTranslucentEffect'
    if ($null -ne $v) { $values['dynamicTransparency'] = [int](([int]$v -band 1) -ne 0); $values['dynamicSegments'] = [int](([int]$v -band 2) -ne 0); $values['dynamicAura'] = [int](([int]$v -band 4) -ne 0) }
    $v = & $get 'FatTaskbar'; if ($null -ne $v) { $values['plumpness'] = @('none', 'buffer', 'aura')[[Math]::Min([int]$v, 2)] }
    $v = & $get 'EnableAutoTray'; if ($null -ne $v) { $values['allAppIcons'] = [int](([int]$v) -ne 0) }
    # Windows 7's power button values.
    $v = & $get 'Start_PowerButtonAction'
    if ($null -ne $v) {
        $values['startMenu.powerButton'] = switch ([int]$v) { 1 { 'signOut' } 4 { 'restart' } 16 { 'sleep' } 64 { 'hibernate' } 256 { 'switchUser' } 512 { 'lock' } default { 'shutDown' } }
    }
    $values
}
