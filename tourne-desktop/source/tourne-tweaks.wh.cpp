// ==WindhawkMod==
// @id              tourne-tweaks
// @name            Tourne'Suite Tweaks
// @description     Winaero Tweaker's options as settings (a part of Tourne'Suite)
// @version         1.0.0
// @author          USER-TOURNE
// @include         windhawk.exe
// @license         MIT
// ==/WindhawkMod==

// This file is generated from tourne-tweaks.engine.cpp and tweaks.def by
// Build-TourneSuiteRedirect.ps1; edit those, not this.

// ==WindhawkModSettings==
/*
- general:
  - restartExplorer: false
    $name: 'Restart Explorer when a change needs it'
    $description: 'Some changes only show once Explorer (the taskbar, desktop and File Explorer) restarts. On: the suite restarts it for you after a Save that needs it. Off: they show the next time you sign in.'
  - liveFontSwap: false
    $name: 'Show a new system font straight away'
    $description: 'Goes with Fonts and sizes > System font. Programs already running switch to the new font at once, instead of after you sign out. Only programs that draw text with GDI follow; newer ones still need a sign-out.'
  $name: 'How tweaks are applied'
  $description: 'Every group below starts switched off and changes nothing. To use one, switch on "Use these tweaks" at its top, then change what you want in it and Save. Switching a group on applies nothing by itself: the suite only notes how each tweak is set. Settings that Windows already offers in Settings, Folder Options or Edge aren''t repeated here.'
- privacy:
  - use: false
    $name: 'Use these tweaks'
    $description: 'Off: this group changes nothing in Windows, and its tweaks stay hidden. Switching it on shows them and applies nothing by itself; change a tweak and Save to apply it.'
  - adsSilentApps: true
    $name: 'Don''t install suggested apps without asking'
    $description: 'Stops Windows quietly installing the apps it suggests, such as games pinned to Start after setup.'
    #! $showIf: {use: true}
  - adsConsumer: true
    $name: 'No suggested apps and content (policy)'
    $description: 'The policy behind the one above, which also stops Windows promoting apps and content. Windows only follows it on the Enterprise and Education editions. Takes effect after you sign out and back in. Asks for admin.'
    #! $showIf: {use: true}
  - copilot: false
    $name: 'Turn off Copilot'
    $description: 'Turns Copilot off for this PC and removes its taskbar button. (Settings can only hide the button.) Takes effect after you sign out and back in. Asks for admin.'
    #! $showIf: {use: true}
  - noWebSearch: true
    $name: 'No web results in Windows search'
    $description: 'Search on the taskbar and in Start only looks on this PC; Bing results leave.'
    #! $showIf: {use: true}
  - noTelemetry: false
    $name: 'Send Microsoft as little diagnostic data as allowed'
    $description: 'Lowers diagnostic data to the minimum this edition of Windows allows. Settings can only turn off the optional part. Takes effect after a restart. Asks for admin.'
    #! $showIf: {use: true}
  - noErrorReporting: false
    $name: 'Turn off Windows Error Reporting'
    $description: 'Crashes are no longer reported to Microsoft, and the "Looking for a solution" dialog stops. Asks for admin.'
    #! $showIf: {use: true}
  - backgroundApps: true
    $name: 'Stop all Store apps running in the background'
    $description: 'One switch for every Store app at once. Settings only offers this per app. Takes effect after you sign out and back in. Asks for admin.'
    #! $showIf: {use: true}
  - noStoreLookup: false
    $name: 'No "Look for an app in the Store" when opening a file'
    $description: 'Opening a file no program handles goes straight to the list of programs on this PC. Asks for admin.'
    #! $showIf: {use: true}
  - noNewAppAlert: false
    $name: 'No "You have new apps that can open this type of file"'
    $description: 'Stops the notice that pops up after installing a program that can open a file type you already use. Asks for admin.'
    #! $showIf: {use: true}
  $name: 'Privacy, ads and suggestions'
  $description: 'Windows'' own switches for ads and suggestions are in Settings > Privacy & security > General, Settings > Personalization > Start and Lock screen, and Settings > System > Notifications > Additional settings. These are the ones Settings doesn''t have.'
- start:
  - use: false
    $name: 'Use these tweaks'
    $description: 'Off: this group changes nothing in Windows, and its tweaks stay hidden. Switching it on shows them and applies nothing by itself; change a tweak and Save to apply it.'
  - hideRecommended: false
    $name: 'Hide Recommended in Start completely'
    $description: 'Removes the whole Recommended section. Windows only allows it on education PCs, so this marks the PC as one; Windows Spotlight may stop working while it''s on. To just empty the section, use Settings > Personalization > Start instead. Takes effect after a restart. Asks for admin.'
    #! $showIf: {use: true}
  - colorizeTaskbar: 'both'
    $name: 'Accent colour on the taskbar only'
    $description: 'Settings can paint the accent colour on Start and the taskbar together. "Taskbar only" uses a value Windows 11 doesn''t document; if it shows no change, pick another option.'
    $options:
    - 'disabled': 'Nowhere'
    - 'both': 'Taskbar and Start'
    - 'taskbar': 'Taskbar only'
    #! $showIf: {use: true}
  - notificationsStayOpen: true
    $name: 'Notification Center stays open'
    $description: 'Clicking away no longer closes it; click the clock again to close it, as in Windows 10''s Action Center.'
    #! $showIf: {use: true}
  - spotlightIcon: false
    $name: 'Hide the Windows Spotlight desktop icon'
    $description: 'Removes the "Learn about this picture" icon Windows Spotlight puts on the desktop. Shows after Explorer restarts.'
    #! $showIf: {use: true}
  - stickers: false
    $name: 'Desktop stickers'
    $description: 'Adds "Add or edit stickers" to the desktop''s right-click menu, for stickers on top of the wallpaper. Takes effect after a restart. Asks for admin.'
    #! $showIf: {use: true}
  - desktopVersion: false
    $name: 'Show the Windows version on the desktop'
    $description: 'Writes the Windows edition and build number in the bottom corner of the desktop. Takes effect after you sign out and back in.'
    #! $showIf: {use: true}
  - wallpaperQuality: 100
    $name: 'JPEG wallpaper quality (%)'
    $description: 'Windows re-encodes JPEG wallpapers at 85%. 100 keeps them sharp. Applies to the next wallpaper you set.'
    #! $min: 60
    #! $max: 100
    #! $showIf: {use: true}
  - shutdownDefault: 'default'
    $name: 'Alt + F4 on the desktop selects'
    $description: 'What the Shut Down Windows box has picked when you press Alt + F4 on the desktop. Shows after Explorer restarts.'
    $options:
    - 'default': 'Windows default'
    - 'shutdown': 'Shut down'
    - 'restart': 'Restart'
    - 'signout': 'Sign out'
    - 'sleep': 'Sleep'
    - 'hibernate': 'Hibernate'
    - 'switch': 'Switch user'
    #! $showIf: {use: true}
  - runAsUserStart: false
    $name: '"Run as different user" in Start'
    $description: 'Adds it to the right-click menu of programs in Start, for running one with another account. Shows after Explorer restarts.'
    #! $showIf: {use: true}
  $name: 'Start, taskbar and desktop'
  $description: 'For Windows'' own Start menu, taskbar and desktop. The suite''s Taskbar and Start menu parts have their own settings.'
- look:
  - use: false
    $name: 'Use these tweaks'
    $description: 'Off: this group changes nothing in Windows, and its tweaks stay hidden. Switching it on shows them and applies nothing by itself; change a tweak and Save to apply it.'
  - visualStyle: 'keep'
    $name: 'Window style'
    $description: 'Switches the style windows are drawn in, live. "Aero Lite" is the flat style Windows keeps hidden. "Leave it as it is" keeps whatever is in use, including a third-party style.'
    $options:
    - 'keep': 'Leave it as it is'
    - 'aero': 'Windows default'
    - 'lite': 'Aero Lite'
    #! $showIf: {use: true}
  - accentColors: ''
    $name: 'Extra accent colours'
    $description: 'Up to eight colours added to the choices in Settings > Personalization > Colors, as #RRGGBB separated by commas. Windows 11 may not show them all. Asks for admin.'
    #! $showIf: {use: true}
  - inactiveTitleColor: '4D8671'
    $name: 'Title bar colour of windows in the background'
    $description: 'As RRGGBB. Used when Settings > Personalization > Colors > "Show accent colour on title bars" is on. Empty: the Windows default.'
    #! $format: colorRgb
    #! $showIf: {use: true}
  - displayAnimation: true
    $name: 'No animation when a display changes'
    $description: 'Skips the fade Windows 11 plays when a screen is connected or its resolution changes. Takes effect after a restart. Asks for admin.'
    #! $showIf: {use: true}
  - slowAnimations: false
    $name: 'Slow window animations while Shift is held'
    $description: 'Minimising or opening a window with Shift held plays it in slow motion, as in Windows 7. For fun, or for checking animations. Takes effect after you sign out and back in.'
    #! $showIf: {use: true}
  $name: 'Colours, style and animations'
  $description: 'Dark mode, accent colour and transparency are in Settings > Personalization > Colors.'
- metrics:
  - use: false
    $name: 'Use these tweaks'
    $description: 'Off: this group changes nothing in Windows, and its tweaks stay hidden. Switching it on shows them and applies nothing by itself; change a tweak and Save to apply it.'
  - systemFont: 'scientifica'
    $name: 'System font'
    $description: 'Replaces Segoe UI everywhere, the way Winaero Tweaker does it: the font is substituted and Segoe UI''s own files are unregistered, so programs can''t pick it directly. Empty restores Segoe UI. To see it before signing out, see How tweaks are applied > Show a new system font straight away. Takes effect after you sign out and back in. Asks for admin.'
    #! $format: fontFamily
    #! $showIf: {use: true}
  - captionFont: 'scientificaCAPS, 12'
    $name: 'Title bar font'
    $description: 'In window title bars.'
    #! $showIf: {use: true}
  - menuFont: 'scientifica, 11'
    $name: 'Menu font'
    $description: 'In menu bars and right-click menus.'
    #! $showIf: {use: true}
  - messageFont: 'scientificaCAPS, 12'
    $name: 'Message box font'
    $description: 'In message boxes and many dialogs.'
    #! $showIf: {use: true}
  - statusFont: 'scientifica, 11, bold'
    $name: 'Status bar and tooltip font'
    $description: 'In status bars and tooltips.'
    #! $showIf: {use: true}
  - iconFont: 'scientifica, 11, bold'
    $name: 'Icon label font'
    $description: 'Under desktop icons and in File Explorer''s lists.'
    #! $showIf: {use: true}
  - captionHeight: 22
    $name: 'Title bar height (px)'
    $description: 'The height of title bars and the size of their buttons.'
    #! $min: 10
    #! $max: 100
    #! $showIf: {use: true}
  - menuHeight: 19
    $name: 'Menu height (px)'
    $description: 'The height of each row in menu bars.'
    #! $min: 10
    #! $max: 100
    #! $showIf: {use: true}
  - scrollWidth: 17
    $name: 'Scroll bar width (px)'
    $description: 'How wide vertical scroll bars are.'
    #! $min: 8
    #! $max: 100
    #! $showIf: {use: true}
  - scrollHeight: 17
    $name: 'Scroll bar button size (px)'
    $description: 'The size of the arrow buttons at the ends of scroll bars.'
    #! $min: 8
    #! $max: 100
    #! $showIf: {use: true}
  - borderWidth: 1
    $name: 'Window border width (px)'
    $description: 'Shows with the Aero Lite style; the default style draws its own borders.'
    #! $min: 1
    #! $max: 10
    #! $showIf: {use: true}
  - borderPadding: 4
    $name: 'Window border padding (px)'
    $description: 'Extra space around the window border, which also widens the area you can grab to resize.'
    #! $min: 0
    #! $max: 10
    #! $showIf: {use: true}
  - iconSpacingH: 75
    $name: 'Desktop icon spacing, across (px)'
    #! $min: 32
    #! $max: 182
    #! $showIf: {use: true}
  - iconSpacingV: 75
    $name: 'Desktop icon spacing, down (px)'
    #! $min: 32
    #! $max: 182
    #! $showIf: {use: true}
  $name: 'Fonts and sizes'
  $description: 'The fonts and sizes of classic window parts, as Windows 7''s Window Color and Appearance dialog set them. A font is written "Face, size in pixels", with ", bold" or ", italic" if wanted, e.g. Segoe UI, 12. Empty: the Windows default. Newer programs draw their own and don''t follow these. They apply live. Windows defaults: icon spacing 75, menu height 19, scroll bars 17, title bars 22, border 1, border padding 4.'
- input:
  - use: false
    $name: 'Use these tweaks'
    $description: 'Off: this group changes nothing in Windows, and its tweaks stay hidden. Switching it on shows them and applies nothing by itself; change a tweak and Save to apply it.'
  - snapDockMoving: true
    $name: 'Snap: dragging to a side fills half the screen'
    $description: 'Needs Snap windows on in Settings. Off: dragging a window to the left or right edge leaves it where it is.'
    #! $showIf: {use: true}
  - snapSizing: true
    $name: 'Snap: dragging the top edge to the screen stretches the window'
    $description: 'Dragging a window''s top or bottom border to the screen''s edge makes it full height, keeping its width.'
    #! $showIf: {use: true}
  - snapDragMaximized: true
    $name: 'Snap: drag a maximised window to restore it'
    $description: 'Off: a maximised window can''t be dragged off the top of the screen.'
    #! $showIf: {use: true}
  - xmouseTracking: false
    $name: 'Focus follows the mouse'
    $description: 'The window under the pointer becomes the active one, without a click.'
    #! $showIf: {use: true}
  - xmouseDelay: 0
    $name: 'Focus follows the mouse: delay (ms)'
    $description: 'How long the pointer rests on a window before it becomes active.'
    #! $min: 0
    #! $max: 9000
    #! $showIf: {use: true}
  - xmouseRaise: false
    $name: 'Focus follows the mouse: bring the window to the front'
    $description: 'Off: the window becomes active but stays behind the others.'
    #! $showIf: {use: true}
  - wheelRouting: 'pointer'
    $name: 'Mouse wheel scrolls'
    $description: 'Settings > Bluetooth & devices > Mouse only switches between the first and last options.'
    $options:
    - 'focus': 'The active window only'
    - 'hybrid': 'The active window, and Store apps under the pointer'
    - 'pointer': 'Whatever window is under the pointer'
    #! $showIf: {use: true}
  - menuShowDelay: 15
    $name: 'Submenu delay (ms)'
    $description: 'How long the pointer rests on a menu item before its submenu opens.'
    #! $min: 0
    #! $max: 1000
    #! $showIf: {use: true}
  - dragThreshold: 4
    $name: 'Drag starts after moving (px)'
    $description: 'How far the pointer moves with the button held before a drag begins. Higher stops accidental drags.'
    #! $min: 1
    #! $max: 100
    #! $showIf: {use: true}
  - dragDrop: 'default'
    $name: 'Dragging files with the left button'
    $description: 'Windows moves files dragged within a drive and copies them between drives. These make it always do one thing (dragging with the right button still asks).'
    $options:
    - 'default': 'Windows default (move on the same drive, copy between drives)'
    - 'copy': 'Always copy'
    - 'move': 'Always move'
    - 'link': 'Always make a shortcut'
    #! $showIf: {use: true}
  - emojiAllLanguages: false
    $name: 'Emoji panel (Win + .) for every keyboard language'
    $description: 'Windows only offers it for some keyboard languages. Takes effect after you sign out and back in. Asks for admin.'
    #! $showIf: {use: true}
  - printScreenSound: 'C:\windows\media\miraclei-sample_soft_alert02_kofi_by_miraclei-360125.wav'
    $name: 'Print Screen sound'
    $description: 'A .wav file played when you press Print Screen. Empty: no sound.'
    #! $format: filePath
    #! $showIf: {use: true}
  - appKeyMail: ''
    $name: 'Keyboard Mail key opens'
    $description: 'For keyboards with extra keys. Empty: the Windows default. "none": the key does nothing. Anything else: that program or command. The same goes for the keys below.'
    #! $showIf: {use: true}
  - appKeyCalculator: ''
    $name: 'Keyboard Calculator key opens'
    #! $showIf: {use: true}
  - appKeyComputer: ''
    $name: 'Keyboard My Computer key opens'
    #! $showIf: {use: true}
  - appKeyBrowser: ''
    $name: 'Keyboard Browser home key opens'
    #! $showIf: {use: true}
  - appKeyMedia: ''
    $name: 'Keyboard Media key opens'
    #! $showIf: {use: true}
  - appKeySearch: ''
    $name: 'Keyboard Search key opens'
    #! $showIf: {use: true}
  $name: 'Mouse, keyboard and windows'
  $description: 'Snap windows itself and title bar shake are in Settings > System > Multitasking.'
- explorer:
  - use: false
    $name: 'Use these tweaks'
    $description: 'Off: this group changes nothing in Windows, and its tweaks stay hidden. Switching it on shows them and applies nothing by itself; change a tweak and Save to apply it.'
  - ribbon: false
    $name: 'Windows 10 ribbon instead of the command bar'
    $description: 'File Explorer gets the Windows 10 ribbon back in place of Windows 11''s command bar. Shows after Explorer restarts.'
    #! $showIf: {use: true}
  - startFolder: 'thispc'
    $name: 'File Explorer opens to'
    $description: 'Folder Options offers Home and This PC; this adds Downloads and a folder of your own. Shows after Explorer restarts.'
    $options:
    - 'home': 'Home'
    - 'thispc': 'This PC'
    - 'downloads': 'Downloads'
    - 'custom': 'The folder below'
    #! $showIf: {use: true}
  - startFolderPath: ''
    $name: 'File Explorer opens to: your own folder'
    $description: 'Used when "File Explorer opens to" is "The folder below".'
    #! $format: folderPath
    #! $showIf: {use: true, startFolder: [custom]}
  - classicSearch: true
    $name: 'Classic search'
    $description: 'File Explorer''s search box looks only on this PC, with the old search filters, and stops suggesting web results. Shows after Explorer restarts.'
    #! $showIf: {use: true}
  - noSearchHistory: true
    $name: 'No search history in File Explorer'
    $description: 'The search box stops remembering and suggesting what you searched for. Shows after Explorer restarts.'
    #! $showIf: {use: true}
  - autoComplete: true
    $name: 'Complete paths as you type'
    $description: 'In the Run box, the address bar, and Open and Save dialogs.'
    #! $showIf: {use: true}
  - noFolderTypeDiscovery: true
    $name: 'Same view in every folder'
    $description: 'Every folder uses the same general view, instead of a pictures or music layout Windows guesses from its contents. Views you set yourself are kept. Shows after Explorer restarts.'
    #! $showIf: {use: true}
  - folderViews: 5000
    $name: 'Folder views to remember'
    $description: 'How many folders keep their own view (icons, details, sorting) before the oldest are forgotten.'
    #! $min: 5000
    #! $max: 20000
    #! $showIf: {use: true}
  - doForAll: false
    $name: '"Do this for all current items" ticked by default'
    $description: 'When copying runs into several conflicts, the box to do the same for all of them starts ticked.'
    #! $showIf: {use: true}
  - removableRecycleBin: false
    $name: 'Recycle Bin on USB drives'
    $description: 'Files deleted from USB sticks and memory cards go to the Recycle Bin instead of being deleted straight away. Shows after Explorer restarts.'
    #! $showIf: {use: true}
  - jumpListItems: 10
    $name: 'Items in jump lists'
    $description: 'How many recent files show when you right-click a program on the taskbar. Shows after Explorer restarts.'
    #! $min: 2
    #! $max: 30
    #! $showIf: {use: true}
  - keepThumbnailCache: true
    $name: 'Keep picture thumbnails between restarts'
    $description: 'Windows clears its thumbnail cache now and then, so folders of pictures load slowly again; this keeps it. Asks for admin.'
    #! $showIf: {use: true}
  $name: 'File Explorer'
  $description: 'Folder Options (View > Options in File Explorer) has hidden files, file extensions, drive letters on or off, Libraries in the navigation pane and what File Explorer opens to. These are the ones it doesn''t have.'
- navpane:
  - use: false
    $name: 'Use these tweaks'
    $description: 'Off: this group changes nothing in Windows, and its tweaks stay hidden. Switching it on shows them and applies nothing by itself; change a tweak and Save to apply it.'
  - navHome: false
    $name: 'Navigation pane: Home'
    $description: 'Shows after Explorer restarts.'
    #! $showIf: {use: true}
  - navGallery: false
    $name: 'Navigation pane: Gallery'
    $description: 'Shows after Explorer restarts.'
    #! $showIf: {use: true}
  - navOneDrive: false
    $name: 'Navigation pane: OneDrive'
    $description: 'Shows after Explorer restarts.'
    #! $showIf: {use: true}
  - navThisPc: true
    $name: 'Navigation pane: This PC'
    $description: 'Shows after Explorer restarts.'
    #! $showIf: {use: true}
  - navNetwork: false
    $name: 'Navigation pane: Network'
    $description: 'Shows after Explorer restarts.'
    #! $showIf: {use: true}
  - navRemovable: false
    $name: 'Navigation pane: USB drives on their own'
    $description: 'On: USB drives also show as their own entries. Off: only under This PC. Shows after Explorer restarts. Asks for admin.'
    #! $showIf: {use: true}
  - navPaneFolders: ''
    $name: 'Navigation pane: your own folders'
    $description: 'Folders added to the left pane, as Name > Path > Icon, with | between them. The icon is optional, e.g. Projects > D:\Projects Shows after Explorer restarts.'
    #! $showIf: {use: true}
  - thisPcDesktop: false
    $name: 'This PC: Desktop folder'
    $description: 'Shows after Explorer restarts. Asks for admin.'
    #! $showIf: {use: true}
  - thisPcDocuments: false
    $name: 'This PC: Documents folder'
    $description: 'Shows after Explorer restarts. Asks for admin.'
    #! $showIf: {use: true}
  - thisPcDownloads: false
    $name: 'This PC: Downloads folder'
    $description: 'Shows after Explorer restarts. Asks for admin.'
    #! $showIf: {use: true}
  - thisPcMusic: false
    $name: 'This PC: Music folder'
    $description: 'Shows after Explorer restarts. Asks for admin.'
    #! $showIf: {use: true}
  - thisPcPictures: false
    $name: 'This PC: Pictures folder'
    $description: 'Shows after Explorer restarts. Asks for admin.'
    #! $showIf: {use: true}
  - thisPcVideos: false
    $name: 'This PC: Videos folder'
    $description: 'Shows after Explorer restarts. Asks for admin.'
    #! $showIf: {use: true}
  - thisPcFolders: 'Downloads > %USERPROFILE%\Downloads > %USERPROFILE%\Downloads\Bouquet\Folder Icons\1615555226_bouquet_icon_theme_by_niivu_dc3uqnw\bouquet Icon Theme\Extra Icons\Folder Downloads_16x16-32.ico,0'
    $name: 'This PC: your own folders'
    $description: 'Folders added to This PC, as Name > Path > Icon, with | between them. The icon is optional, e.g. Downloads > X:\Downloads > C:\Icons\dl.ico Shows after Explorer restarts.'
    #! $showIf: {use: true}
  - driveLetters: 'before'
    $name: 'Drive letters in This PC'
    $description: 'Folder Options can only show or hide them; this can also put them first. Shows after Explorer restarts. Asks for admin.'
    $options:
    - 'after': 'After the name (Windows default)'
    - 'before': 'Before the name'
    - 'network': 'Before the name for network drives only'
    - 'hidden': 'Hidden'
    #! $showIf: {use: true}
  - hiddenDrives: 'F,H,I,J,K,L'
    $name: 'Hidden drives'
    $description: 'Drive letters hidden from This PC, e.g. F,H,I. They still open when typed in the address bar. Empty: all drives show. Shows after Explorer restarts. Asks for admin.'
    #! $showIf: {use: true}
  - librariesName: 'Libraries'
    $name: 'Libraries: name'
    $description: 'Empty: the Windows default. Libraries themselves show once Folder Options > Show libraries is on. Shows after Explorer restarts.'
    #! $showIf: {use: true}
  - librariesIcon: 'C:\ProgramData\WinaeroTweaker\ShellIcons\-336311243.ico'
    $name: 'Libraries: icon'
    $description: 'An .ico file. Empty: the Windows default. Shows after Explorer restarts.'
    #! $format: filePath
    #! $showIf: {use: true}
  $name: 'File Explorer: navigation pane and This PC'
  $description: 'What shows in File Explorer''s left pane, and in This PC.'
- icons:
  - use: false
    $name: 'Use these tweaks'
    $description: 'Off: this group changes nothing in Windows, and its tweaks stay hidden. Switching it on shows them and applies nothing by itself; change a tweak and Save to apply it.'
  - shortcutArrow: 'none'
    $name: 'Shortcut arrow'
    $description: 'The arrow on shortcut icons. Empty: the Windows default. "none": no arrow. "classic": Windows XP''s arrow. Or an .ico file. Shows after Explorer restarts. Asks for admin.'
    #! $showIf: {use: true}
  - noShortcutText: true
    $name: 'No " - Shortcut" on new shortcuts'
    #! $showIf: {use: true}
  - compressedOverlay: 'none'
    $name: 'Compressed file arrows'
    $description: 'The blue arrows on files NTFS has compressed. Empty: the Windows default. "none": no arrows. Or an .ico file. Shows after Explorer restarts. Asks for admin.'
    #! $showIf: {use: true}
  - driveIcon: 'C:\ProgramData\WinaeroTweaker\DriveIcons\-912001537.ico'
    $name: 'One icon for every drive'
    $description: 'An .ico file used for all drives. Empty: the Windows default. Shows after Explorer restarts. Asks for admin.'
    #! $format: filePath
    #! $showIf: {use: true}
  - driveIcons: 'C=C:\ProgramData\WinaeroTweaker\DriveIcons\-1671516419.ico|D=C:\ProgramData\WinaeroTweaker\DriveIcons\1816881822.ico|E=C:\ProgramData\WinaeroTweaker\DriveIcons\-912001537.ico|G=C:\ProgramData\WinaeroTweaker\DriveIcons\-1671516419.ico|I=C:\ProgramData\WinaeroTweaker\DriveIcons\-1671516419.ico|J=C:\ProgramData\WinaeroTweaker\DriveIcons\1816881822.ico|X=C:\ProgramData\WinaeroTweaker\DriveIcons\1816881822.ico'
    $name: 'Icons for single drives'
    $description: 'Letter and .ico pairs, e.g. D=C:\Icons\games.ico|E=C:\Icons\media.ico. These win over the one above. Shows after Explorer restarts. Asks for admin.'
    #! $showIf: {use: true}
  - noFolderThumbnails: false
    $name: 'Plain folder icons'
    $description: 'Folders show a plain icon instead of a preview of what''s in them; files keep their thumbnails. Shows after Explorer restarts.'
    #! $showIf: {use: true}
  - thumbnailFrame: 'default'
    $name: 'Picture thumbnail frame'
    $description: 'The frame drawn around picture thumbnails. Shows after Explorer restarts.'
    $options:
    - 'default': 'Windows default'
    - 'none': 'None'
    - 'shadow': 'Drop shadow'
    - 'photo': 'Photo border'
    - 'film': 'Film strip'
    #! $showIf: {use: true}
  - iconCacheKB: 2000
    $name: 'Icon cache size (KB)'
    $description: 'Raise it if icons go blank or turn into the wrong ones. Takes effect after a restart. Asks for admin.'
    #! $min: 100
    #! $max: 100000
    #! $showIf: {use: true}
  $name: 'Icons, shortcuts and thumbnails'
- menu:
  - use: false
    $name: 'Use these tweaks'
    $description: 'Off: this group changes nothing in Windows, and its tweaks stay hidden. Switching it on shows them and applies nothing by itself; change a tweak and Save to apply it.'
  - classicMenus: true
    $name: 'Full right-click menus'
    $description: 'Right-click menus open in full, as in Windows 10, without "Show more options". The entries below show in either menu. Shows after Explorer restarts.'
    #! $showIf: {use: true}
  - multiSelectLimit: 25
    $name: 'Open, Print and Edit for up to this many files'
    $description: 'With more files selected than this, Windows leaves Open, Print and Edit out of the menu.'
    #! $min: 5
    #! $max: 200
    #! $showIf: {use: true}
  - editImageApp: 'X:\Program Files\Krita (x64)\bin\krita.exe "%1"'
    $name: 'Program for Edit on pictures'
    $description: 'What right-click > Edit opens pictures with, as a command line with "%1" for the file. Empty: Paint. Asks for admin.'
    #! $showIf: {use: true}
  - runAsUserAlways: false
    $name: '"Run as different user" without Shift'
    $description: 'Windows only shows it on programs while Shift is held.'
    #! $showIf: {use: true}
  - newProcessAlways: false
    $name: '"Open in new process" without Shift'
    $description: 'Opens a folder in a separate Explorer, so a crash there can''t take the taskbar down. Windows only shows it while Shift is held.'
    #! $showIf: {use: true}
  - removeAskCopilot: false
    $name: 'Remove "Ask Copilot"'
    $description: 'Asks for admin.'
    #! $showIf: {use: true}
  - removeClipchamp: false
    $name: 'Remove "Edit in Clipchamp"'
    $description: 'Asks for admin.'
    #! $showIf: {use: true}
  - removeEditNotepad: false
    $name: 'Remove "Edit in Notepad"'
    $description: 'Asks for admin.'
    #! $showIf: {use: true}
  - removeEditPhotos: false
    $name: 'Remove "Edit with Photos"'
    $description: 'Asks for admin.'
    #! $showIf: {use: true}
  - removePaint3d: false
    $name: 'Remove "Edit with Paint 3D"'
    #! $showIf: {use: true}
  - removeCreateVideo: false
    $name: 'Remove "Create a new video"'
    $description: 'Asks for admin.'
    #! $showIf: {use: true}
  - removeRotate: false
    $name: 'Remove "Rotate left/right"'
    $description: 'On pictures. Asks for admin.'
    #! $showIf: {use: true}
  - removeMediaPlayer: false
    $name: 'Remove Windows Media Player entries'
    $description: 'Add to playlist and Play with Windows Media Player. Asks for admin.'
    #! $showIf: {use: true}
  - removeCast: false
    $name: 'Remove "Cast to device"'
    $description: 'Asks for admin.'
    #! $showIf: {use: true}
  - removeShare: false
    $name: 'Remove "Share"'
    $description: 'Asks for admin.'
    #! $showIf: {use: true}
  - removeGiveAccess: false
    $name: 'Remove "Give access to"'
    $description: 'Sharing with other people on the network. Asks for admin.'
    #! $showIf: {use: true}
  - removeOneDrive: false
    $name: 'Remove OneDrive entries'
    $description: 'Asks for admin.'
    #! $showIf: {use: true}
  - removeFavorites: false
    $name: 'Remove "Add to Favorites"'
    #! $showIf: {use: true}
  - removeIncludeLibrary: false
    $name: 'Remove "Include in library"'
    $description: 'Asks for admin.'
    #! $showIf: {use: true}
  - removePinTaskbar: false
    $name: 'Remove "Pin to taskbar"'
    $description: 'Asks for admin.'
    #! $showIf: {use: true}
  - removePinStart: false
    $name: 'Remove "Pin to Start"'
    $description: 'Asks for admin.'
    #! $showIf: {use: true}
  - removeOpenInTerminal: false
    $name: 'Remove "Open in Terminal"'
    $description: 'Windows Terminal''s own entry. Folders > Windows Terminal, below, can add a better one. Asks for admin.'
    #! $showIf: {use: true}
  - removePrint: false
    $name: 'Remove "Print" (images, text and scripts)'
    #! $showIf: {use: true}
  - removeCompatibility: false
    $name: 'Remove "Troubleshoot compatibility"'
    $description: 'On programs. Asks for admin.'
    #! $showIf: {use: true}
  - removePreviousVersions: false
    $name: 'Remove "Restore previous versions"'
    $description: 'Asks for admin.'
    #! $showIf: {use: true}
  - removeBurnImage: false
    $name: 'Remove "Burn disc image"'
    $description: 'On .iso files.'
    #! $showIf: {use: true}
  - removeExtractAll: false
    $name: 'Remove "Extract all"'
    $description: 'On .zip files. Asks for admin.'
    #! $showIf: {use: true}
  - removeEfsOwnership: false
    $name: 'Remove "File ownership" (EFS)'
    $description: 'Only shows on PCs using the Encrypting File System.'
    #! $showIf: {use: true}
  - removeBitlocker: false
    $name: 'Remove BitLocker entries from drives'
    #! $showIf: {use: true}
  - removeDefenderScan: false
    $name: 'Remove "Scan with Microsoft Defender"'
    $description: 'Asks for admin.'
    #! $showIf: {use: true}
  $name: 'Right-click menu: Windows'' own entries'
  $description: 'Hide the entries Windows and its apps add to right-click menus, or change how they behave.'
- menuFiles:
  - use: false
    $name: 'Use these tweaks'
    $description: 'Off: this group changes nothing in Windows, and its tweaks stay hidden. Switching it on shows them and applies nothing by itself; change a tweak and Save to apply it.'
  - cmdHere: 'disabled'
    $name: 'Folders: Command prompt here'
    $description: '"With Shift held" keeps it out of the way until you hold Shift while right-clicking. The same goes for the ones below.'
    $options:
    - 'disabled': 'Off'
    - 'enabled': 'On'
    - 'shift': 'On, with Shift held'
    #! $showIf: {use: true}
  - cmdHereAdmin: 'disabled'
    $name: 'Folders: Command prompt here as administrator'
    $options:
    - 'disabled': 'Off'
    - 'enabled': 'On'
    - 'shift': 'On, with Shift held'
    #! $showIf: {use: true}
  - psHereAdmin: 'disabled'
    $name: 'Folders: PowerShell here as administrator'
    $options:
    - 'disabled': 'Off'
    - 'enabled': 'On'
    - 'shift': 'On, with Shift held'
    #! $showIf: {use: true}
  - terminalMenu: 'disabled'
    $name: 'Folders: Windows Terminal'
    $description: 'Opens Windows Terminal in the folder, with a choice of profile.'
    $options:
    - 'disabled': 'Off'
    - 'enabled': 'On'
    - 'shift': 'On, with Shift held'
    #! $showIf: {use: true}
  - terminalAdminMenu: 'disabled'
    $name: 'Folders: Windows Terminal as administrator'
    $options:
    - 'disabled': 'Off'
    - 'enabled': 'On'
    - 'shift': 'On, with Shift held'
    #! $showIf: {use: true}
  - bashHere: false
    $name: 'Folders: Linux shell here'
    $description: 'Opens WSL in the folder, when it''s installed.'
    #! $showIf: {use: true}
  - caseSensitivity: false
    $name: 'Folders: Case sensitivity'
    $description: 'Makes a folder treat File.txt and file.txt as two files, or not, for Linux tools.'
    #! $showIf: {use: true}
  - attributes: false
    $name: 'Files and folders: Attributes'
    $description: 'Hide, show, make read-only and the like, without opening Properties.'
    #! $showIf: {use: true}
  - viewOwner: false
    $name: 'Files and folders: Show owner'
    $description: 'Shows which account owns it.'
    #! $showIf: {use: true}
  - takeOwnership: false
    $name: 'Files and folders: Take ownership'
    $description: 'Makes your account the owner, so you can change or delete files Windows protects.'
    #! $showIf: {use: true}
  - changeOwner: false
    $name: 'Files and folders: Change owner'
    $description: 'Hands ownership to Administrators, Everyone, SYSTEM, TrustedInstaller or you.'
    #! $showIf: {use: true}
  - resetPermissions: false
    $name: 'Files and folders: Reset permissions'
    $description: 'Puts back the permissions it would get from its folder.'
    #! $showIf: {use: true}
  - inheritanceMenu: false
    $name: 'Files and folders: Inherited permissions'
    $description: 'Turns taking permissions from the folder above on or off.'
    #! $showIf: {use: true}
  - encryptMenu: false
    $name: 'Files and folders: Encrypt and Decrypt'
    $description: 'Uses the Encrypting File System, on editions of Windows that have it.'
    #! $showIf: {use: true}
  - hashMenu: false
    $name: 'Files: Hash'
    $description: 'Shows a file''s SHA256, SHA1 or MD5 checksum, for checking a download.'
    #! $showIf: {use: true}
  - unblockMenu: false
    $name: 'Files: Unblock'
    $description: 'Removes the "came from the internet" mark that makes Windows warn before opening it.'
    #! $showIf: {use: true}
  - runWithPriority: false
    $name: 'Programs: Run with priority'
    $description: 'Starts a program at a CPU priority you pick.'
    #! $showIf: {use: true}
  - runAsPs1: false
    $name: 'PowerShell scripts: Run as administrator'
    #! $showIf: {use: true}
  - ps1Menu: 'disabled'
    $name: 'PowerShell scripts: "Run or modify" menu'
    $description: 'Run, run as administrator, or edit a .ps1 file.'
    $options:
    - 'disabled': 'Off'
    - 'enabled': 'On'
    - 'shift': 'On, with Shift held'
    #! $showIf: {use: true}
  - ps1MenuPwsh: false
    $name: 'PowerShell scripts: PowerShell 7 in that menu'
    $description: 'Adds "Run with PowerShell 7" to the menu above, when PowerShell 7 is installed.'
    #! $showIf: {use: true, ps1Menu: [enabled, shift]}
  - runAsVbs: false
    $name: 'VBScript files: Run as administrator'
    #! $showIf: {use: true}
  - openWithBatch: true
    $name: 'Batch files: Open with'
    $description: 'Windows leaves Open with out for .bat and .cmd files.'
    #! $showIf: {use: true}
  - runAsMsi: false
    $name: 'Installers (.msi): Install as administrator'
    #! $showIf: {use: true}
  - msiExtract: true
    $name: 'Installers (.msi): Extract the contents'
    $description: 'Unpacks an installer into a folder beside it, without installing. Asks for admin.'
    #! $showIf: {use: true}
  - cabInstall: false
    $name: 'Update packages (.cab): Install'
    $description: 'Installs a Windows update or language pack from a .cab file.'
    #! $showIf: {use: true}
  - bitlockerLock: false
    $name: 'Drives: Lock with BitLocker'
    $description: 'Locks an unlocked BitLocker drive again without restarting.'
    #! $showIf: {use: true}
  - advancedSystemProps: true
    $name: 'This PC: Advanced system properties'
    $description: 'Performance, environment variables, startup and recovery.'
    #! $showIf: {use: true}
  - classicSystemProps: false
    $name: 'This PC: System properties (classic)'
    $description: 'The old System Properties window: computer name, hardware, remote.'
    #! $showIf: {use: true}
  $name: 'Right-click menu: files, folders and programs'
  $description: 'Entries added to the menus of files, folders, drives and programs. Each one is off until you switch it on. New entries are for your account only and show straight away; entries Winaero Tweaker added are taken over, so turning one off removes it too.'
- menuDesktop:
  - use: false
    $name: 'Use these tweaks'
    $description: 'Off: this group changes nothing in Windows, and its tweaks stay hidden. Switching it on shows them and applies nothing by itself; change a tweak and Save to apply it.'
  - settingsMenu: false
    $name: 'Settings'
    #! $showIf: {use: true}
  - controlPanelMenu: false
    $name: 'Control Panel'
    #! $showIf: {use: true}
  - personalizeMenu: false
    $name: 'Personalization'
    $description: 'The classic Personalization window.'
    #! $showIf: {use: true}
  - taskManagerMenu: true
    $name: 'Task Manager'
    $description: 'Asks for admin.'
    #! $showIf: {use: true}
  - windowsToolsMenu: false
    $name: 'Windows Tools'
    $description: 'The folder of administrative tools.'
    #! $showIf: {use: true}
  - envVarsMenu: false
    $name: 'Environment variables'
    #! $showIf: {use: true}
  - gpeditMenu: false
    $name: 'Group Policy'
    $description: 'The Local Group Policy Editor, on Pro and higher.'
    #! $showIf: {use: true}
  - defaultAppsMenu: false
    $name: 'Default apps'
    #! $showIf: {use: true}
  - accountsMenu: false
    $name: 'User accounts'
    #! $showIf: {use: true}
  - securityMenu: false
    $name: 'Windows Security'
    #! $showIf: {use: true}
  - firewallMenu: false
    $name: 'Firewall'
    #! $showIf: {use: true}
  - updateMenu: false
    $name: 'Windows Update'
    #! $showIf: {use: true}
  - appModeMenu: false
    $name: 'Light or dark mode for apps'
    $description: 'Switches apps between light and dark.'
    #! $showIf: {use: true}
  - windowsModeMenu: false
    $name: 'Light or dark mode for Windows'
    $description: 'Switches the taskbar and Start between light and dark.'
    #! $showIf: {use: true}
  - wallpaperLocation: false
    $name: 'Open the wallpaper''s location'
    $description: 'Opens the folder of the picture on the desktop now.'
    #! $showIf: {use: true}
  - bluetoothMenu: false
    $name: 'Bluetooth'
    #! $showIf: {use: true}
  - networkLocation: false
    $name: 'Network location'
    $description: 'Switches the current network between Private and Public.'
    #! $showIf: {use: true}
  - projectMenu: false
    $name: 'Project display'
    $description: 'Duplicate, extend or switch screens.'
    #! $showIf: {use: true}
  - clipboardMenu: false
    $name: 'Clipboard'
    $description: 'Opens the clipboard history, or clears it.'
    #! $showIf: {use: true}
  - screenSnipMenu: false
    $name: 'Screen snip'
    $description: 'Takes a screenshot of part of the screen.'
    #! $showIf: {use: true}
  - snippingToolMenu: false
    $name: 'Snipping Tool'
    #! $showIf: {use: true}
  - copilotMenu: false
    $name: 'Copilot'
    #! $showIf: {use: true}
  - speechMenu: false
    $name: 'Speech recognition'
    #! $showIf: {use: true}
  - killHung: false
    $name: 'Close programs that aren''t responding'
    $description: 'Ends every program Windows marks as not responding, at once.'
    #! $showIf: {use: true}
  - restartExplorerMenu: true
    $name: 'Restart Explorer'
    $description: 'Restarts the taskbar, desktop and File Explorer. Asks for admin.'
    #! $showIf: {use: true}
  - restartStartMenu: false
    $name: 'Restart Start menu'
    #! $showIf: {use: true}
  - powerPlanMenu: false
    $name: 'Switch power plan'
    #! $showIf: {use: true}
  - powerMenu: false
    $name: 'Power options'
    $description: 'Shut down, restart, sleep and the rest.'
    #! $showIf: {use: true}
  - shutdownMenu: false
    $name: 'Shut down'
    #! $showIf: {use: true}
  - safeModeMenu: false
    $name: 'Safe Mode'
    $description: 'Sets the next start to Safe Mode and restarts.'
    #! $showIf: {use: true}
  - advancedStartup: false
    $name: 'Advanced startup'
    $description: 'Restarts into the recovery options: Safe Mode, firmware settings, repair.'
    #! $showIf: {use: true}
  - firmwareMenu: false
    $name: 'Restart to firmware (UEFI)'
    $description: 'Restarts straight into the PC''s firmware settings.'
    #! $showIf: {use: true}
  - restorePointMenu: false
    $name: 'Create restore point'
    #! $showIf: {use: true}
  - sfcMenu: false
    $name: 'Check system files (sfc)'
    $description: 'Runs sfc /scannow, which repairs damaged Windows files.'
    #! $showIf: {use: true}
  - repairImage: false
    $name: 'Repair Windows image'
    $description: 'Runs DISM to check or repair the Windows image that sfc repairs from.'
    #! $showIf: {use: true}
  - compactOs: false
    $name: 'Compact OS'
    $description: 'Compresses or decompresses Windows'' own files to save space.'
    #! $showIf: {use: true}
  $name: 'Right-click menu: desktop'
  $description: 'Shortcuts added to the menu you get by right-clicking the desktop. Each one is off until you switch it on. New entries are for your account only and show straight away; entries Winaero Tweaker added are taken over, so turning one off removes it too.'
- menuNew:
  - use: false
    $name: 'Use these tweaks'
    $description: 'Off: this group changes nothing in Windows, and its tweaks stay hidden. Switching it on shows them and applies nothing by itself; change a tweak and Save to apply it.'
  - newBat: false
    $name: 'Batch file (.bat)'
    #! $showIf: {use: true}
  - newCmd: false
    $name: 'Command script (.cmd)'
    #! $showIf: {use: true}
  - newPs1: false
    $name: 'PowerShell script (.ps1)'
    #! $showIf: {use: true}
  - newVbs: false
    $name: 'VBScript file (.vbs)'
    #! $showIf: {use: true}
  - newReg: false
    $name: 'Registration entries (.reg)'
    #! $showIf: {use: true}
  $name: 'Right-click menu: New'
  $description: 'File types added to New in right-click menus.'
- signin:
  - use: false
    $name: 'Use these tweaks'
    $description: 'Off: this group changes nothing in Windows, and its tweaks stay hidden. Switching it on shows them and applies nothing by itself; change a tweak and Save to apply it.'
  - noLockScreen: false
    $name: 'Skip the lock screen'
    $description: 'Waking or starting the PC goes straight to the sign-in screen. Asks for admin.'
    #! $showIf: {use: true}
  - lockScreenImage: '%USERPROFILE%\Downloads\Images\Tourne Moss 3840x2160.png'
    $name: 'Lock screen picture for everyone'
    $description: 'A picture shown on the lock and sign-in screens for every account. While it''s set, Settings can''t change the lock screen picture. Empty: off. Takes effect after you sign out and back in. Asks for admin.'
    #! $format: filePath
    #! $showIf: {use: true}
  - slideshowDuration: 0
    $name: 'Lock screen slideshow length (s)'
    $description: 'How long a lock screen slideshow plays before the screen turns off. 0: the Windows default.'
    #! $min: 0
    #! $max: 86400
    #! $showIf: {use: true}
  - hideLockedUser: false
    $name: 'Hide your name on the lock screen'
    $description: 'Asks for admin.'
    #! $showIf: {use: true}
  - noLockNetworkIcon: false
    $name: 'No network icon on the lock screen'
    $description: 'Asks for admin.'
    #! $showIf: {use: true}
  - noLogonBlur: false
    $name: 'Sign-in screen: sharp background'
    $description: 'The lock screen picture stays sharp behind the sign-in box instead of blurred. Asks for admin.'
    #! $showIf: {use: true}
  - noLogonImage: false
    $name: 'Sign-in screen: plain colour background'
    $description: 'The accent colour instead of the lock screen picture. Wins over the one above. Asks for admin.'
    #! $showIf: {use: true}
  - hideLastUser: false
    $name: 'Sign-in screen: don''t show the last user'
    $description: 'Everyone types their user name. Asks for admin.'
    #! $showIf: {use: true}
  - noLogonPowerButton: false
    $name: 'Sign-in screen: no power button'
    $description: 'Asks for admin.'
    #! $showIf: {use: true}
  - numLockAtLogon: false
    $name: 'Sign-in screen: Num Lock on'
    $description: 'Asks for admin.'
    #! $showIf: {use: true}
  - requireCtrlAltDel: false
    $name: 'Press Ctrl + Alt + Del to sign in'
    $description: 'Asks for admin.'
    #! $showIf: {use: true}
  - autoLogonCheckbox: false
    $name: 'Bring back automatic sign-in in netplwiz'
    $description: 'Shows the "Users must enter a user name and password" box in netplwiz again, which is how you set up signing in without a password. Asks for admin.'
    #! $showIf: {use: true}
  - signInTitle: ''
    $name: 'Message before sign-in: title'
    $description: 'Shown to everyone before they sign in. Empty: no message. Asks for admin.'
    #! $showIf: {use: true}
  - signInText: ''
    $name: 'Message before sign-in: text'
    $description: 'Asks for admin.'
    #! $showIf: {use: true}
  - verboseStatus: false
    $name: 'Detailed status messages'
    $description: 'Signing in and shutting down show what Windows is doing ("Applying user settings...") instead of "Please wait". Asks for admin.'
    #! $showIf: {use: true}
  - noPasswordReveal: false
    $name: 'No password reveal button'
    $description: 'Removes the eye button that shows a password as you type it. Asks for admin.'
    #! $showIf: {use: true}
  $name: 'Lock screen and sign-in'
  $description: 'The lock screen picture and its fun facts are in Settings > Personalization > Lock screen; sign-in options are in Settings > Accounts.'
- boot:
  - use: false
    $name: 'Use these tweaks'
    $description: 'Off: this group changes nothing in Windows, and its tweaks stay hidden. Switching it on shows them and applies nothing by itself; change a tweak and Save to apply it.'
  - autoRepair: true
    $name: 'Automatic repair after failed starts'
    $description: 'After two failed starts Windows tries to repair itself. The build can''t read this without admin, so it starts on, the Windows default. Takes effect after a restart. Asks for admin.'
    #! $showIf: {use: true}
  - legacyBootMenu: false
    $name: 'F8 boot menu'
    $description: 'The Windows 7 style menu: press F8 while the PC starts for Safe Mode and the other startup options. Starts off, the Windows default. Takes effect after a restart. Asks for admin.'
    #! $showIf: {use: true}
  - bootLogo: true
    $name: 'Boot logo and spinner'
    $description: 'Takes effect after a restart. Asks for admin.'
    #! $showIf: {use: true}
  - chkdskTimeout: 8
    $name: 'Disk check countdown at startup (s)'
    $description: 'How long you have to skip a scheduled disk check. Asks for admin.'
    #! $min: 0
    #! $max: 259200
    #! $showIf: {use: true}
  - bsodNoSmiley: false
    $name: 'Stop screen without the sad face'
    $description: 'Takes effect after a restart. Asks for admin.'
    #! $showIf: {use: true}
  - bsodDetails: false
    $name: 'Stop screen shows technical details'
    $description: 'The blue screen lists the error''s parameters, for tracking down a crash. Takes effect after a restart. Asks for admin.'
    #! $showIf: {use: true}
  - crashOnScroll: false
    $name: 'Crash on purpose with Ctrl + Scroll Lock'
    $description: 'For debugging: hold right Ctrl and press Scroll Lock twice to blue screen and write a crash dump. Takes effect after a restart. Asks for admin.'
    #! $showIf: {use: true}
  - registryBackup: false
    $name: 'Automatic registry backup'
    $description: 'Windows stopped backing up the registry in 2018; this turns it back on (to C:\Windows\System32\config\RegBack). Takes effect after a restart. Asks for admin.'
    #! $showIf: {use: true}
  - restorePointAnyTime: true
    $name: 'Create restore points without the 24-hour limit'
    $description: 'Windows skips a new restore point when one was made in the last 24 hours. Asks for admin.'
    #! $showIf: {use: true}
  $name: 'Startup, recovery and crashes'
- updates:
  - use: false
    $name: 'Use these tweaks'
    $description: 'Off: this group changes nothing in Windows, and its tweaks stay hidden. Switching it on shows them and applies nothing by itself; change a tweak and Save to apply it.'
  - noWindowsUpdate: false
    $name: 'No automatic Windows updates'
    $description: 'Stops automatic downloads, installs and restarts. You can still check for updates by hand in Settings. Asks for admin.'
    #! $showIf: {use: true}
  - noDriverUpdates: false
    $name: 'No driver updates from Windows Update'
    $description: 'Windows Update stops replacing drivers, such as the graphics driver you installed yourself. Asks for admin.'
    #! $showIf: {use: true}
  - noMrt: false
    $name: 'Don''t install the Malicious Software Removal Tool'
    $description: 'Windows Update stops installing and running the monthly Malicious Software Removal Tool. Asks for admin.'
    #! $showIf: {use: true}
  - noMaintenance: false
    $name: 'Turn off automatic maintenance'
    $description: 'Stops the cleanup, scans and updates Windows runs while the PC is idle. Asks for admin.'
    #! $showIf: {use: true}
  $name: 'Updates and maintenance'
  $description: 'Pausing updates is in Settings > Windows Update.'
- security:
  - use: false
    $name: 'Use these tweaks'
    $description: 'Off: this group changes nothing in Windows, and its tweaks stay hidden. Switching it on shows them and applies nothing by itself; change a tweak and Save to apply it.'
  - uac: 'enabled'
    $name: 'User Account Control'
    $description: 'Off lets every program run with full rights, silently. The slider in Control Panel can''t turn it fully off; this can. Takes effect after a restart. Asks for admin.'
    $options:
    - 'enabled': 'On (Windows default)'
    - 'disabled': 'Off'
    #! $showIf: {use: true}
  - adminApproval: false
    $name: 'UAC for the built-in Administrator'
    $description: 'Whether the built-in Administrator account also gets UAC prompts. Takes effect after a restart. Asks for admin.'
    #! $showIf: {use: true}
  - builtinAdmin: false
    $name: 'Built-in Administrator account'
    $description: 'Turns on the hidden account named Administrator. Asks for admin.'
    #! $showIf: {use: true}
  - noDefender: false
    $name: 'Turn off Microsoft Defender Antivirus'
    $description: 'Only works with Tamper Protection off (Windows Security > Virus & threat protection > Manage settings). Takes effect after a restart. Asks for admin.'
    #! $showIf: {use: true}
  - noDownloadBlocking: false
    $name: 'Don''t mark downloaded files'
    $description: 'New downloads skip the "this file came from another computer" warning.'
    #! $showIf: {use: true}
  - usbWriteProtect: false
    $name: 'Write-protect USB drives'
    $description: 'Nothing can be written to USB drives, only read. Takes effect after a restart. Asks for admin.'
    #! $showIf: {use: true}
  - msiSafeMode: false
    $name: 'Windows Installer works in Safe Mode'
    $description: 'Lets you uninstall programs while in Safe Mode. Asks for admin.'
    #! $showIf: {use: true}
  - adminShares: true
    $name: 'Administrative shares (C$, ADMIN$)'
    $description: 'The hidden C$ and ADMIN$ shares remote administration tools use. Takes effect after a restart. Asks for admin.'
    #! $showIf: {use: true}
  - linkedConnections: false
    $name: 'Mapped drives in programs run as administrator'
    $description: 'Mapped network drives also show in programs run as administrator. Takes effect after a restart. Asks for admin.'
    #! $showIf: {use: true}
  - rdpPort: 3389
    $name: 'Remote Desktop port'
    $description: 'Open the new port in the firewall too. Takes effect after a restart. Asks for admin.'
    #! $min: 1
    #! $max: 65535
    #! $showIf: {use: true}
  - ipRouting: false
    $name: 'IP routing (forwarding)'
    $description: 'Lets this PC pass network traffic between its connections, like a router. Takes effect after a restart. Asks for admin.'
    #! $showIf: {use: true}
  $name: 'Security, accounts and network'
  $description: 'Windows Security has SmartScreen and blocking potentially unwanted apps. Turning any of these off makes the PC less safe.'
- power:
  - use: false
    $name: 'Use these tweaks'
    $description: 'Off: this group changes nothing in Windows, and its tweaks stay hidden. Switching it on shows them and applies nothing by itself; change a tweak and Save to apply it.'
  - hibernation: 'disabled'
    $name: 'Hibernation'
    $description: 'Asks for admin.'
    $options:
    - 'disabled': 'Off (also turns off fast startup)'
    - 'enabled': 'On'
    - 'reduced': 'Fast startup only (small file)'
    #! $showIf: {use: true}
  - hiberFileSize: 0
    $name: 'Hibernation file size (% of RAM)'
    $description: '0: leave it as it is (with hibernation off there''s no file). 40 to 100: the file''s size as a share of RAM, used when hibernation is on. Asks for admin.'
    #! $min: 0
    #! $max: 100
    #! $showIf: {use: true}
  - noPowerThrottling: false
    $name: 'Turn off power throttling'
    $description: 'Windows slows down programs in the background to save battery; this stops it. Takes effect after a restart. Asks for admin.'
    #! $showIf: {use: true}
  - wakePassword: false
    $name: 'Show "Require a password on wakeup" in Power Options'
    $description: 'Windows hides this choice in a power plan''s advanced settings; this shows it. Asks for admin.'
    #! $showIf: {use: true}
  - screenSaverGrace: 5
    $name: 'Screen saver lock grace period (s)'
    $description: 'Seconds a password-protected screen saver runs before it locks. Asks for admin.'
    #! $min: 0
    #! $max: 3600
    #! $showIf: {use: true}
  - svchostSplitKB: 33554432
    $name: 'Svchost split threshold (KB)'
    $description: 'Above this much RAM, each service gets its own svchost.exe. Set it above your RAM (e.g. 33554432 for 32 GB) for fewer svchost processes. Asks for admin.'
    #! $min: 0
    #! $max: 2147483647
    #! $showIf: {use: true}
  $name: 'Power and performance'
- apps:
  - use: false
    $name: 'Use these tweaks'
    $description: 'Off: this group changes nothing in Windows, and its tweaks stay hidden. Switching it on shows them and applies nothing by itself; change a tweak and Save to apply it.'
  - photoViewer: false
    $name: 'Windows Photo Viewer in "Open with"'
    $description: 'Registers the old viewer for common image types; choose it once in Open with or Default apps. Asks for admin.'
    #! $showIf: {use: true}
  - noInk: true
    $name: 'Turn off Windows Ink Workspace'
    $description: 'Removes the Windows Ink Workspace pen menu. Asks for admin.'
    #! $showIf: {use: true}
  - noOneDriveBackup: true
    $name: 'Don''t move Desktop, Documents and Pictures to OneDrive'
    $description: 'Stops OneDrive offering to move these folders into OneDrive. Folders it already moved stay where they are. Asks for admin.'
    #! $showIf: {use: true}
  $name: 'Built-in apps'
  $description: 'Store app updates are in the Microsoft Store''s own settings.'
- panels:
  - use: false
    $name: 'Use these tweaks'
    $description: 'Off: this group changes nothing in Windows, and its tweaks stay hidden. Switching it on shows them and applies nothing by itself; change a tweak and Save to apply it.'
  - netplwiz: false
    $name: 'Control Panel: User Accounts (classic)'
    $description: 'Shows after Explorer restarts. Asks for admin.'
    #! $showIf: {use: true}
  - classicBackground: false
    $name: 'Control Panel: Desktop Background'
    $description: 'Shows after Explorer restarts. Asks for admin.'
    #! $showIf: {use: true}
  - classicColor: false
    $name: 'Control Panel: Color and Appearance'
    $description: 'Shows after Explorer restarts. Asks for admin.'
    #! $showIf: {use: true}
  - classicPersonalization: true
    $name: 'Control Panel: Personalization (classic)'
    $description: 'Shows after Explorer restarts. Asks for admin.'
    #! $showIf: {use: true}
  - modernPersonalization: false
    $name: 'Control Panel: Personalization (Settings)'
    $description: 'A link to Settings > Personalization in Control Panel. Shows after Explorer restarts. Asks for admin.'
    #! $showIf: {use: true}
  - windowsUpdate: false
    $name: 'Control Panel: Windows Update'
    $description: 'A link to Settings > Windows Update in Control Panel. Shows after Explorer restarts. Asks for admin.'
    #! $showIf: {use: true}
  - noOnlineTips: false
    $name: 'No online tips and videos in Settings'
    $description: 'Settings stops showing tips and videos it fetches from the internet. Asks for admin.'
    #! $showIf: {use: true}
  - settingsPages: 'hide:home'
    $name: 'Settings pages to hide or show'
    $description: '"hide:" or "showonly:" followed by page names separated by ";", e.g. hide:home;gaming-gamebar. Page names are the ms-settings: links without the prefix. Empty: every page shows. Asks for admin.'
    #! $showIf: {use: true}
  - hideInsider: true
    $name: 'Hide the Windows Insider page'
    $description: 'Asks for admin.'
    #! $showIf: {use: true}
  $name: 'Control Panel and Settings app'
  $description: 'Classic pages added back to Control Panel, and pages taken out of Settings.'
- edge:
  - use: false
    $name: 'Use these tweaks'
    $description: 'Off: this group changes nothing in Windows, and its tweaks stay hidden. Switching it on shows them and applies nothing by itself; change a tweak and Save to apply it.'
  - firstRun: true
    $name: 'No first run experience'
    $description: 'Asks for admin.'
    #! $showIf: {use: true}
  - autoImport: false
    $name: 'No import from other browsers on first run'
    $description: 'Asks for admin.'
    #! $showIf: {use: true}
  - defaultPrompts: true
    $name: 'No "make Edge your default browser" prompts'
    $description: 'Asks for admin.'
    #! $showIf: {use: true}
  - implicitSignIn: false
    $name: 'No automatic sign-in with the Windows account'
    $description: 'Asks for admin.'
    #! $showIf: {use: true}
  - personalization: true
    $name: 'No personalisation of ads, news and the browser'
    $description: 'Asks for admin.'
    #! $showIf: {use: true}
  - recommendations: true
    $name: 'No recommendations and desktop notifications'
    $description: 'Asks for admin.'
    #! $showIf: {use: true}
  - follow: false
    $name: 'No Follow creators'
    $description: 'Asks for admin.'
    #! $showIf: {use: true}
  - standaloneSidebar: false
    $name: 'No standalone sidebar'
    $description: 'The Copilot sidebar Edge can pin to the side of the screen, outside the browser. Asks for admin.'
    #! $showIf: {use: true}
  - collections: true
    $name: 'No Collections'
    $description: 'Asks for admin.'
    #! $showIf: {use: true}
  - splitScreen: false
    $name: 'No split screen'
    $description: 'Asks for admin.'
    #! $showIf: {use: true}
  - feedback: true
    $name: 'No feedback prompts'
    $description: 'Asks for admin.'
    #! $showIf: {use: true}
  - searchBar: false
    $name: 'No floating Bing search bar'
    $description: 'Asks for admin.'
    #! $showIf: {use: true}
  - restoreDialog: false
    $name: 'No "Restore pages" prompt after a crash'
    $description: 'Asks for admin.'
    #! $showIf: {use: true}
  - ntpTopSites: false
    $name: 'New tab: hide Microsoft''s pinned sites'
    $description: 'Asks for admin.'
    #! $showIf: {use: true}
  - desktopShortcut: true
    $name: 'No desktop shortcut after Edge updates'
    $description: 'Asks for admin.'
    #! $showIf: {use: true}
  - noUpdates: false
    $name: 'No Edge updates'
    $description: 'Edge stops updating itself. Security fixes stop too. Asks for admin.'
    #! $showIf: {use: true}
  $name: 'Microsoft Edge'
  $description: 'Policies for Edge, which it shows as "managed by your organization". Edge''s own settings page has the sidebar, shopping, Microsoft Rewards, sync, SmartScreen, startup boost and the new tab page. Edge picks these up at its next start.'
- system:
  - use: false
    $name: 'Use these tweaks'
    $description: 'Off: this group changes nothing in Windows, and its tweaks stay hidden. Switching it on shows them and applies nothing by itself; change a tweak and Save to apply it.'
  - oemManufacturer: 'Alienware'
    $name: 'Manufacturer'
    $description: 'Asks for admin.'
    #! $showIf: {use: true}
  - oemModel: 'Alienware Aurora ACT1250 '
    $name: 'Model'
    $description: 'Asks for admin.'
    #! $showIf: {use: true}
  - oemSupportHours: 'Also available at www.dell.com/support/home'
    $name: 'Support hours'
    $description: 'Asks for admin.'
    #! $showIf: {use: true}
  - oemSupportPhone: '1 800 Alienware'
    $name: 'Support phone'
    $description: 'Asks for admin.'
    #! $showIf: {use: true}
  - oemSupportUrl: 'http://www.dell.com/support/home'
    $name: 'Support website'
    $description: 'Asks for admin.'
    #! $showIf: {use: true}
  - oemLogo: 'c:\windows\system32\oem\SystemLogo.bmp'
    $name: 'Logo (.bmp, 120 x 120)'
    $description: 'Asks for admin.'
    #! $format: filePath
    #! $showIf: {use: true}
  - registeredOwner: ''
    $name: 'Registered owner'
    $description: 'Shown in winver. Empty: left as it is. Asks for admin.'
    #! $showIf: {use: true}
  - registeredOrganization: ''
    $name: 'Registered organisation'
    $description: 'Empty: left as it is. Asks for admin.'
    #! $showIf: {use: true}
  $name: 'About this PC'
  $description: 'What Settings > System > About and winver show. Empty leaves each one as it is.'
*/
// ==/WindhawkModSettings==

#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>

#include <algorithm>
#include <atomic>
#include <map>
#include <string>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
// The table

// A setting and what applying each of its values does. Ops are lines of the
// small language described at the top of tweaks.def.
struct Tweak {
    PCWSTR id;
    PCWSTR title;          // as the settings page names it
    char kind;             // 't' toggle, 'n' number, 's' text, 'c' choice
    char after;            // 'e' Explorer restart, 's' sign-out, 'r' restart
    bool admin;            // some of its ops need admin
    bool keepEmpty;        // text: empty, number: 0, means leave the system alone
    PCWSTR windowsDefault; // number: the value that runs the unset ops
    PCWSTR a;              // on / set / choice options
    PCWSTR b;              // off / unset
    PCWSTR uses;           // other settings the ops read, space separated
};

constexpr Tweak kTweaks[] = {
    {L"general.restartExplorer", L"Restart Explorer when a change needs it", 't', 0, false, false, nullptr,
     L"",
     L"",
     L""},
    {L"general.liveFontSwap", L"Show a new system font straight away", 't', 0, false, false, nullptr,
     L"",
     L"",
     L""},
    {L"privacy.adsSilentApps", L"Don't install suggested apps without asking", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\ContentDeliveryManager | SilentInstalledAppsEnabled = dword:0\nHKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\ContentDeliveryManager | PreInstalledAppsEnabled = dword:0\nHKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\ContentDeliveryManager | OemPreInstalledAppsEnabled = dword:0",
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\ContentDeliveryManager | SilentInstalledAppsEnabled = dword:1\nHKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\ContentDeliveryManager | PreInstalledAppsEnabled = dword:1\nHKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\ContentDeliveryManager | OemPreInstalledAppsEnabled = dword:1",
     L""},
    {L"privacy.adsConsumer", L"No suggested apps and content (policy)", 't', 's', true, false, nullptr,
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\Windows\\CloudContent | DisableWindowsConsumerFeatures = dword:1",
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\Windows\\CloudContent | DisableWindowsConsumerFeatures -",
     L""},
    {L"privacy.copilot", L"Turn off Copilot", 't', 's', true, false, nullptr,
     L"HKCU\\Software\\Policies\\Microsoft\\Windows\\WindowsCopilot | TurnOffWindowsCopilot = dword:1\nHKLM\\SOFTWARE\\Policies\\Microsoft\\Windows\\WindowsCopilot | TurnOffWindowsCopilot = dword:1\nHKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced | ShowCopilotButton = dword:0",
     L"HKCU\\Software\\Policies\\Microsoft\\Windows\\WindowsCopilot | TurnOffWindowsCopilot -\nHKLM\\SOFTWARE\\Policies\\Microsoft\\Windows\\WindowsCopilot | TurnOffWindowsCopilot -\nHKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced | ShowCopilotButton -",
     L""},
    {L"privacy.noWebSearch", L"No web results in Windows search", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Search | BingSearchEnabled = dword:0\nHKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Search | CortanaConsent = dword:0",
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Search | BingSearchEnabled -\nHKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Search | CortanaConsent -",
     L""},
    {L"privacy.noTelemetry", L"Send Microsoft as little diagnostic data as allowed", 't', 'r', true, false, nullptr,
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\Windows\\DataCollection | AllowTelemetry = dword:0",
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\Windows\\DataCollection | AllowTelemetry -",
     L""},
    {L"privacy.noErrorReporting", L"Turn off Windows Error Reporting", 't', 0, true, false, nullptr,
     L"HKLM\\SOFTWARE\\Microsoft\\Windows\\Windows Error Reporting | Disabled = dword:1",
     L"HKLM\\SOFTWARE\\Microsoft\\Windows\\Windows Error Reporting | Disabled -",
     L""},
    {L"privacy.backgroundApps", L"Stop all Store apps running in the background", 't', 's', true, false, nullptr,
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\BackgroundAccessApplications | GlobalUserDisabled = dword:1\nHKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Search | BackgroundAppGlobalToggle = dword:0\nHKLM\\SOFTWARE\\Policies\\Microsoft\\Windows\\AppPrivacy | LetAppsRunInBackground = dword:2",
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\BackgroundAccessApplications | GlobalUserDisabled -\nHKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Search | BackgroundAppGlobalToggle -\nHKLM\\SOFTWARE\\Policies\\Microsoft\\Windows\\AppPrivacy | LetAppsRunInBackground -",
     L""},
    {L"privacy.noStoreLookup", L"No \"Look for an app in the Store\" when opening a file", 't', 0, true, false, nullptr,
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\Windows\\Explorer | NoUseStoreOpenWith = dword:1",
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\Windows\\Explorer | NoUseStoreOpenWith -",
     L""},
    {L"privacy.noNewAppAlert", L"No \"You have new apps that can open this type of file\"", 't', 0, true, false, nullptr,
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\Windows\\Explorer | NoNewAppAlert = dword:1",
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\Windows\\Explorer | NoNewAppAlert -",
     L""},
    {L"start.hideRecommended", L"Hide Recommended in Start completely", 't', 'r', true, false, nullptr,
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\Windows\\Explorer | HideRecommendedSection = dword:1\nHKLM\\SOFTWARE\\Microsoft\\PolicyManager\\current\\device\\Start | HideRecommendedSection = dword:1\nHKLM\\SOFTWARE\\Microsoft\\PolicyManager\\current\\device\\Education | IsEducationEnvironment = dword:1",
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\Windows\\Explorer | HideRecommendedSection -\nHKLM\\SOFTWARE\\Microsoft\\PolicyManager\\current\\device\\Start | HideRecommendedSection -\nHKLM\\SOFTWARE\\Microsoft\\PolicyManager\\current\\device\\Education | IsEducationEnvironment -",
     L""},
    {L"start.colorizeTaskbar", L"Accent colour on the taskbar only", 'c', 0, false, false, nullptr,
     L"disabled\036HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize | ColorPrevalence = dword:0\nnotify ImmersiveColorSet\037both\036HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize | ColorPrevalence = dword:1\nnotify ImmersiveColorSet\037taskbar\036HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize | ColorPrevalence = dword:2\nnotify ImmersiveColorSet",
     L"",
     L""},
    {L"start.notificationsStayOpen", L"Notification Center stays open", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\ImmersiveShell\\Launcher | DisableLightDismiss = dword:1",
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\ImmersiveShell\\Launcher | DisableLightDismiss -",
     L""},
    {L"start.spotlightIcon", L"Hide the Windows Spotlight desktop icon", 't', 'e', false, false, nullptr,
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\HideDesktopIcons\\NewStartPanel | {2cc5ca98-6485-489a-920e-b3e88a6ccce3} = dword:1",
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\HideDesktopIcons\\NewStartPanel | {2cc5ca98-6485-489a-920e-b3e88a6ccce3} -",
     L""},
    {L"start.stickers", L"Desktop stickers", 't', 'r', true, false, nullptr,
     L"HKLM\\SOFTWARE\\Microsoft\\PolicyManager\\current\\device\\Stickers | EnableStickers = dword:1",
     L"HKLM\\SOFTWARE\\Microsoft\\PolicyManager\\current\\device\\Stickers | EnableStickers -",
     L""},
    {L"start.desktopVersion", L"Show the Windows version on the desktop", 't', 's', false, false, nullptr,
     L"HKCU\\Control Panel\\Desktop | PaintDesktopVersion = dword:1",
     L"HKCU\\Control Panel\\Desktop | PaintDesktopVersion = dword:0",
     L""},
    {L"start.wallpaperQuality", L"JPEG wallpaper quality (%)", 'n', 0, false, false, L"85",
     L"HKCU\\Control Panel\\Desktop | JPEGImportQuality = dword:{}",
     L"HKCU\\Control Panel\\Desktop | JPEGImportQuality -",
     L""},
    {L"start.shutdownDefault", L"Alt + F4 on the desktop selects", 'c', 'e', false, false, nullptr,
     L"default\036HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced | Start_PowerButtonAction -\037shutdown\036HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced | Start_PowerButtonAction = dword:2\037restart\036HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced | Start_PowerButtonAction = dword:4\037signout\036HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced | Start_PowerButtonAction = dword:1\037sleep\036HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced | Start_PowerButtonAction = dword:16\037hibernate\036HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced | Start_PowerButtonAction = dword:64\037switch\036HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced | Start_PowerButtonAction = dword:256",
     L"",
     L""},
    {L"start.runAsUserStart", L"\"Run as different user\" in Start", 't', 'e', false, false, nullptr,
     L"HKCU\\Software\\Policies\\Microsoft\\Windows\\Explorer | ShowRunAsDifferentUserInStart = dword:1",
     L"HKCU\\Software\\Policies\\Microsoft\\Windows\\Explorer | ShowRunAsDifferentUserInStart -",
     L""},
    {L"look.visualStyle", L"Window style", 'c', 0, false, false, nullptr,
     L"keep\036\037aero\036style %SystemRoot%\\Resources\\Themes\\aero\\aero.msstyles\037lite\036style %SystemRoot%\\Resources\\Themes\\aero\\aerolite.msstyles",
     L"",
     L""},
    {L"look.accentColors", L"Extra accent colours", 's', 0, true, false, nullptr,
     L"accents {}",
     L"accents",
     L""},
    {L"look.inactiveTitleColor", L"Title bar colour of windows in the background", 's', 0, false, false, nullptr,
     L"HKCU\\Software\\Microsoft\\Windows\\DWM | AccentColorInactive = dword:{abgr}\nnotify ImmersiveColorSet",
     L"HKCU\\Software\\Microsoft\\Windows\\DWM | AccentColorInactive -\nnotify ImmersiveColorSet",
     L""},
    {L"look.displayAnimation", L"No animation when a display changes", 't', 'r', true, false, nullptr,
     L"HKLM\\SOFTWARE\\Microsoft\\Windows\\Dwm | ForceDisableModeChangeAnimation = dword:1",
     L"HKLM\\SOFTWARE\\Microsoft\\Windows\\Dwm | ForceDisableModeChangeAnimation -",
     L""},
    {L"look.slowAnimations", L"Slow window animations while Shift is held", 't', 's', false, false, nullptr,
     L"HKCU\\Software\\Microsoft\\Windows\\DWM | AnimationsShiftKey = dword:1",
     L"HKCU\\Software\\Microsoft\\Windows\\DWM | AnimationsShiftKey -",
     L""},
    {L"metrics.systemFont", L"System font", 's', 's', true, false, nullptr,
     L"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\FontSubstitutes | Segoe UI = sz:{}\nHKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts | Segoe UI (TrueType) = sz:\nHKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts | Segoe UI Bold (TrueType) = sz:\nHKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts | Segoe UI Bold Italic (TrueType) = sz:\nHKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts | Segoe UI Italic (TrueType) = sz:\nHKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts | Segoe UI Light (TrueType) = sz:\nHKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts | Segoe UI Semibold (TrueType) = sz:",
     L"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\FontSubstitutes | Segoe UI -\nHKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts | Segoe UI (TrueType) = sz:segoeui.ttf\nHKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts | Segoe UI Bold (TrueType) = sz:segoeuib.ttf\nHKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts | Segoe UI Bold Italic (TrueType) = sz:segoeuiz.ttf\nHKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts | Segoe UI Italic (TrueType) = sz:segoeuii.ttf\nHKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts | Segoe UI Light (TrueType) = sz:segoeuil.ttf\nHKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts | Segoe UI Semibold (TrueType) = sz:seguisb.ttf",
     L""},
    {L"metrics.captionFont", L"Title bar font", 's', 0, false, false, nullptr,
     L"font caption {}",
     L"font caption Segoe UI, 12",
     L""},
    {L"metrics.menuFont", L"Menu font", 's', 0, false, false, nullptr,
     L"font menu {}",
     L"font menu Segoe UI, 12",
     L""},
    {L"metrics.messageFont", L"Message box font", 's', 0, false, false, nullptr,
     L"font message {}",
     L"font message Segoe UI, 12",
     L""},
    {L"metrics.statusFont", L"Status bar and tooltip font", 's', 0, false, false, nullptr,
     L"font status {}",
     L"font status Segoe UI, 12",
     L""},
    {L"metrics.iconFont", L"Icon label font", 's', 0, false, false, nullptr,
     L"font icon {}",
     L"font icon Segoe UI, 12",
     L""},
    {L"metrics.captionHeight", L"Title bar height (px)", 'n', 0, false, false, L"22",
     L"metric captionHeight {}",
     L"",
     L""},
    {L"metrics.menuHeight", L"Menu height (px)", 'n', 0, false, false, L"19",
     L"metric menuHeight {}",
     L"",
     L""},
    {L"metrics.scrollWidth", L"Scroll bar width (px)", 'n', 0, false, false, L"17",
     L"metric scrollWidth {}",
     L"",
     L""},
    {L"metrics.scrollHeight", L"Scroll bar button size (px)", 'n', 0, false, false, L"17",
     L"metric scrollHeight {}",
     L"",
     L""},
    {L"metrics.borderWidth", L"Window border width (px)", 'n', 0, false, false, L"1",
     L"metric borderWidth {}",
     L"",
     L""},
    {L"metrics.borderPadding", L"Window border padding (px)", 'n', 0, false, false, L"4",
     L"metric borderPadding {}",
     L"",
     L""},
    {L"metrics.iconSpacingH", L"Desktop icon spacing, across (px)", 'n', 0, false, false, L"75",
     L"metric iconSpacingH {}",
     L"",
     L""},
    {L"metrics.iconSpacingV", L"Desktop icon spacing, down (px)", 'n', 0, false, false, L"75",
     L"metric iconSpacingV {}",
     L"",
     L""},
    {L"input.snapDockMoving", L"Snap: dragging to a side fills half the screen", 't', 0, false, false, nullptr,
     L"spi ui 0x0091 1",
     L"spi ui 0x0091 0",
     L""},
    {L"input.snapSizing", L"Snap: dragging the top edge to the screen stretches the window", 't', 0, false, false, nullptr,
     L"spi ui 0x008F 1",
     L"spi ui 0x008F 0",
     L""},
    {L"input.snapDragMaximized", L"Snap: drag a maximised window to restore it", 't', 0, false, false, nullptr,
     L"spi ui 0x008D 1",
     L"spi ui 0x008D 0",
     L""},
    {L"input.xmouseTracking", L"Focus follows the mouse", 't', 0, false, false, nullptr,
     L"spi pv 0x1001 1",
     L"spi pv 0x1001 0",
     L""},
    {L"input.xmouseDelay", L"Focus follows the mouse: delay (ms)", 'n', 0, false, false, L"0",
     L"spi pv 0x2003 {}",
     L"",
     L""},
    {L"input.xmouseRaise", L"Focus follows the mouse: bring the window to the front", 't', 0, false, false, nullptr,
     L"spi pv 0x100D 1",
     L"spi pv 0x100D 0",
     L""},
    {L"input.wheelRouting", L"Mouse wheel scrolls", 'c', 0, false, false, nullptr,
     L"focus\036spi pv 0x201D 0\037hybrid\036spi pv 0x201D 1\037pointer\036spi pv 0x201D 2",
     L"",
     L""},
    {L"input.menuShowDelay", L"Submenu delay (ms)", 'n', 0, false, false, L"400",
     L"spi ui 0x006B {}",
     L"",
     L""},
    {L"input.dragThreshold", L"Drag starts after moving (px)", 'n', 0, false, false, L"4",
     L"spi ui 0x004C {}\nspi ui 0x004D {}",
     L"",
     L""},
    {L"input.dragDrop", L"Dragging files with the left button", 'c', 0, false, false, nullptr,
     L"default\036HKCU\\Software\\Classes\\* | DefaultDropEffect -\nHKCU\\Software\\Classes\\AllFilesystemObjects | DefaultDropEffect -\037copy\036HKCU\\Software\\Classes\\* | DefaultDropEffect = dword:1\nHKCU\\Software\\Classes\\AllFilesystemObjects | DefaultDropEffect = dword:1\037move\036HKCU\\Software\\Classes\\* | DefaultDropEffect = dword:2\nHKCU\\Software\\Classes\\AllFilesystemObjects | DefaultDropEffect = dword:2\037link\036HKCU\\Software\\Classes\\* | DefaultDropEffect = dword:4\nHKCU\\Software\\Classes\\AllFilesystemObjects | DefaultDropEffect = dword:4",
     L"",
     L""},
    {L"input.emojiAllLanguages", L"Emoji panel (Win + .) for every keyboard language", 't', 's', true, false, nullptr,
     L"HKLM\\SOFTWARE\\Microsoft\\Input\\Settings | EnableExpressiveInputShellHotkey = dword:1",
     L"HKLM\\SOFTWARE\\Microsoft\\Input\\Settings | EnableExpressiveInputShellHotkey -",
     L""},
    {L"input.printScreenSound", L"Print Screen sound", 's', 0, false, false, nullptr,
     L"HKCU\\AppEvents\\Schemes\\Apps\\.Default\\SnapShot\\.Current | @ = sz:{}",
     L"HKCU\\AppEvents\\Schemes\\Apps\\.Default\\SnapShot\\.Current | @ = sz:",
     L""},
    {L"input.appKeyMail", L"Keyboard Mail key opens", 's', 0, false, false, nullptr,
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\AppKey\\15 | ShellExecute = sz:{cmd}",
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\AppKey\\15 | -",
     L""},
    {L"input.appKeyCalculator", L"Keyboard Calculator key opens", 's', 0, false, false, nullptr,
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\AppKey\\18 | ShellExecute = sz:{cmd}",
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\AppKey\\18 | -",
     L""},
    {L"input.appKeyComputer", L"Keyboard My Computer key opens", 's', 0, false, false, nullptr,
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\AppKey\\17 | ShellExecute = sz:{cmd}",
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\AppKey\\17 | -",
     L""},
    {L"input.appKeyBrowser", L"Keyboard Browser home key opens", 's', 0, false, false, nullptr,
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\AppKey\\7 | ShellExecute = sz:{cmd}",
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\AppKey\\7 | -",
     L""},
    {L"input.appKeyMedia", L"Keyboard Media key opens", 's', 0, false, false, nullptr,
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\AppKey\\16 | ShellExecute = sz:{cmd}",
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\AppKey\\16 | -",
     L""},
    {L"input.appKeySearch", L"Keyboard Search key opens", 's', 0, false, false, nullptr,
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\AppKey\\5 | ShellExecute = sz:{cmd}",
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\AppKey\\5 | -",
     L""},
    {L"explorer.ribbon", L"Windows 10 ribbon instead of the command bar", 't', 'e', false, false, nullptr,
     L"HKCU\\Software\\Classes\\CLSID\\{2aa9162e-c906-4dd9-ad0b-3d24a8eef5a0} | @ = sz:CLSID_ItemsViewAdapter\nHKCU\\Software\\Classes\\CLSID\\{2aa9162e-c906-4dd9-ad0b-3d24a8eef5a0}\\InProcServer32 | @ = sz:C:\\Windows\\System32\\Windows.UI.FileExplorer.dll_\nHKCU\\Software\\Classes\\CLSID\\{2aa9162e-c906-4dd9-ad0b-3d24a8eef5a0}\\InProcServer32 | ThreadingModel = sz:Apartment\nHKCU\\Software\\Classes\\CLSID\\{6480100b-5a83-4d1e-9f69-8ae5a88e9a33} | @ = sz:File Explorer Xaml Island View Adapter\nHKCU\\Software\\Classes\\CLSID\\{6480100b-5a83-4d1e-9f69-8ae5a88e9a33}\\InProcServer32 | @ = sz:C:\\Windows\\System32\\Windows.UI.FileExplorer.dll_\nHKCU\\Software\\Classes\\CLSID\\{6480100b-5a83-4d1e-9f69-8ae5a88e9a33}\\InProcServer32 | ThreadingModel = sz:Apartment",
     L"HKCU\\Software\\Classes\\CLSID\\{2aa9162e-c906-4dd9-ad0b-3d24a8eef5a0} | -\nHKCU\\Software\\Classes\\CLSID\\{6480100b-5a83-4d1e-9f69-8ae5a88e9a33} | -",
     L""},
    {L"explorer.startFolder", L"File Explorer opens to", 'c', 'e', false, false, nullptr,
     L"home\036HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced | LaunchTo = dword:2\nHKCU\\Software\\Classes\\CLSID\\{52205fd8-5dfb-447d-801a-d0b52f2e83e1} | -\037thispc\036HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced | LaunchTo = dword:1\nHKCU\\Software\\Classes\\CLSID\\{52205fd8-5dfb-447d-801a-d0b52f2e83e1} | -\037downloads\036HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced | LaunchTo = dword:3\nHKCU\\Software\\Classes\\CLSID\\{52205fd8-5dfb-447d-801a-d0b52f2e83e1} | -\037custom\036HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced | LaunchTo = dword:1\nHKCU\\Software\\Classes\\CLSID\\{52205fd8-5dfb-447d-801a-d0b52f2e83e1}\\shell\\opennewwindow\\command | @ = sz:explorer.exe \"{@explorer.startFolderPath}\"\nHKCU\\Software\\Classes\\CLSID\\{52205fd8-5dfb-447d-801a-d0b52f2e83e1}\\shell\\opennewwindow\\command | DelegateExecute = sz:",
     L"",
     L"explorer.startFolderPath"},
    {L"explorer.startFolderPath", L"File Explorer opens to: your own folder", 's', 0, false, false, nullptr,
     L"",
     L"",
     L""},
    {L"explorer.classicSearch", L"Classic search", 't', 'e', false, false, nullptr,
     L"HKCU\\Software\\Classes\\CLSID\\{1d64637d-31e9-4b06-9124-e83fb178ac6e}\\TreatAs | @ = sz:{64bc32b5-4eec-4de7-972d-bd8bd0324537}\nHKCU\\Software\\Classes\\WOW6432Node\\CLSID\\{1d64637d-31e9-4b06-9124-e83fb178ac6e}\\TreatAs | @ = sz:{64bc32b5-4eec-4de7-972d-bd8bd0324537}",
     L"HKCU\\Software\\Classes\\CLSID\\{1d64637d-31e9-4b06-9124-e83fb178ac6e} | -\nHKCU\\Software\\Classes\\WOW6432Node\\CLSID\\{1d64637d-31e9-4b06-9124-e83fb178ac6e} | -",
     L""},
    {L"explorer.noSearchHistory", L"No search history in File Explorer", 't', 'e', false, false, nullptr,
     L"HKCU\\Software\\Policies\\Microsoft\\Windows\\Explorer | DisableSearchBoxSuggestions = dword:1",
     L"HKCU\\Software\\Policies\\Microsoft\\Windows\\Explorer | DisableSearchBoxSuggestions -",
     L""},
    {L"explorer.autoComplete", L"Complete paths as you type", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\AutoComplete | Append Completion = sz:yes",
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\AutoComplete | Append Completion = sz:no",
     L""},
    {L"explorer.noFolderTypeDiscovery", L"Same view in every folder", 't', 'e', false, false, nullptr,
     L"HKCU\\Software\\Classes\\Local Settings\\Software\\Microsoft\\Windows\\Shell\\Bags\\AllFolders\\Shell | FolderType = sz:NotSpecified",
     L"HKCU\\Software\\Classes\\Local Settings\\Software\\Microsoft\\Windows\\Shell\\Bags\\AllFolders\\Shell | FolderType -",
     L""},
    {L"explorer.folderViews", L"Folder views to remember", 'n', 0, false, false, L"5000",
     L"HKCU\\Software\\Classes\\Local Settings\\Software\\Microsoft\\Windows\\Shell | BagMRU Size = dword:{}",
     L"HKCU\\Software\\Classes\\Local Settings\\Software\\Microsoft\\Windows\\Shell | BagMRU Size -",
     L""},
    {L"explorer.doForAll", L"\"Do this for all current items\" ticked by default", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\OperationStatusManager | ConfirmationCheckBoxDoForAll = dword:1",
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\OperationStatusManager | ConfirmationCheckBoxDoForAll -",
     L""},
    {L"explorer.removableRecycleBin", L"Recycle Bin on USB drives", 't', 'e', false, false, nullptr,
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer | RecycleBinDrives = dword:0xffffffff",
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer | RecycleBinDrives -",
     L""},
    {L"explorer.jumpListItems", L"Items in jump lists", 'n', 'e', false, false, L"12",
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced | JumpListItems_Maximum = dword:{}",
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced | JumpListItems_Maximum -",
     L""},
    {L"explorer.keepThumbnailCache", L"Keep picture thumbnails between restarts", 't', 0, true, false, nullptr,
     L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\VolumeCaches\\Thumbnail Cache | Autorun = dword:0\nHKLM\\SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Explorer\\VolumeCaches\\Thumbnail Cache | Autorun = dword:0",
     L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\VolumeCaches\\Thumbnail Cache | Autorun = dword:1\nHKLM\\SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Explorer\\VolumeCaches\\Thumbnail Cache | Autorun = dword:1",
     L""},
    {L"navpane.navHome", L"Navigation pane: Home", 't', 'e', false, false, nullptr,
     L"HKCU\\Software\\Classes\\CLSID\\{f874310e-b6b7-47dc-bc84-b9e6b38f5903} | System.IsPinnedToNameSpaceTree -",
     L"HKCU\\Software\\Classes\\CLSID\\{f874310e-b6b7-47dc-bc84-b9e6b38f5903} | System.IsPinnedToNameSpaceTree = dword:0",
     L""},
    {L"navpane.navGallery", L"Navigation pane: Gallery", 't', 'e', false, false, nullptr,
     L"HKCU\\Software\\Classes\\CLSID\\{e88865ea-0e1c-4e20-9aa6-edcd0212c87c} | System.IsPinnedToNameSpaceTree -",
     L"HKCU\\Software\\Classes\\CLSID\\{e88865ea-0e1c-4e20-9aa6-edcd0212c87c} | System.IsPinnedToNameSpaceTree = dword:0",
     L""},
    {L"navpane.navOneDrive", L"Navigation pane: OneDrive", 't', 'e', false, false, nullptr,
     L"HKCU\\Software\\Classes\\CLSID\\{018D5C66-4533-4307-9B53-224DE2ED1FE6} | System.IsPinnedToNameSpaceTree = dword:1",
     L"HKCU\\Software\\Classes\\CLSID\\{018D5C66-4533-4307-9B53-224DE2ED1FE6} | System.IsPinnedToNameSpaceTree = dword:0",
     L""},
    {L"navpane.navThisPc", L"Navigation pane: This PC", 't', 'e', false, false, nullptr,
     L"HKCU\\Software\\Classes\\CLSID\\{20D04FE0-3AEA-1069-A2D8-08002B30309D} | System.IsPinnedToNameSpaceTree -",
     L"HKCU\\Software\\Classes\\CLSID\\{20D04FE0-3AEA-1069-A2D8-08002B30309D} | System.IsPinnedToNameSpaceTree = dword:0",
     L""},
    {L"navpane.navNetwork", L"Navigation pane: Network", 't', 'e', false, false, nullptr,
     L"HKCU\\Software\\Classes\\CLSID\\{F02C1A0D-BE21-4350-88B0-7367FC96EF3C} | System.IsPinnedToNameSpaceTree -",
     L"HKCU\\Software\\Classes\\CLSID\\{F02C1A0D-BE21-4350-88B0-7367FC96EF3C} | System.IsPinnedToNameSpaceTree = dword:0",
     L""},
    {L"navpane.navRemovable", L"Navigation pane: USB drives on their own", 't', 'e', true, false, nullptr,
     L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Desktop\\NameSpace\\DelegateFolders\\{F5FB2C77-0E2F-4A16-A381-3E560C68BC83} | @ = sz:Removable Drives\nHKLM\\SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Desktop\\NameSpace\\DelegateFolders\\{F5FB2C77-0E2F-4A16-A381-3E560C68BC83} | @ = sz:Removable Drives",
     L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Desktop\\NameSpace\\DelegateFolders\\{F5FB2C77-0E2F-4A16-A381-3E560C68BC83} | -\nHKLM\\SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Desktop\\NameSpace\\DelegateFolders\\{F5FB2C77-0E2F-4A16-A381-3E560C68BC83} | -",
     L""},
    {L"navpane.navPaneFolders", L"Navigation pane: your own folders", 's', 'e', false, false, nullptr,
     L"folders Desktop {}",
     L"folders Desktop",
     L""},
    {L"navpane.thisPcDesktop", L"This PC: Desktop folder", 't', 'e', true, false, nullptr,
     L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MyComputer\\NameSpace\\{B4BFCC3A-DB2C-424C-B029-7FE99A87C641} | HiddenByDefault -\nHKLM\\SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MyComputer\\NameSpace\\{B4BFCC3A-DB2C-424C-B029-7FE99A87C641} | HiddenByDefault -",
     L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MyComputer\\NameSpace\\{B4BFCC3A-DB2C-424C-B029-7FE99A87C641} | HiddenByDefault = dword:1\nHKLM\\SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MyComputer\\NameSpace\\{B4BFCC3A-DB2C-424C-B029-7FE99A87C641} | HiddenByDefault = dword:1",
     L""},
    {L"navpane.thisPcDocuments", L"This PC: Documents folder", 't', 'e', true, false, nullptr,
     L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MyComputer\\NameSpace\\{A8CDFF1C-4878-43be-B5FD-F8091C1C60D0} | HiddenByDefault -\nHKLM\\SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MyComputer\\NameSpace\\{A8CDFF1C-4878-43be-B5FD-F8091C1C60D0} | HiddenByDefault -",
     L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MyComputer\\NameSpace\\{A8CDFF1C-4878-43be-B5FD-F8091C1C60D0} | HiddenByDefault = dword:1\nHKLM\\SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MyComputer\\NameSpace\\{A8CDFF1C-4878-43be-B5FD-F8091C1C60D0} | HiddenByDefault = dword:1",
     L""},
    {L"navpane.thisPcDownloads", L"This PC: Downloads folder", 't', 'e', true, false, nullptr,
     L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MyComputer\\NameSpace\\{374DE290-123F-4565-9164-39C4925E467B} | HiddenByDefault -\nHKLM\\SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MyComputer\\NameSpace\\{374DE290-123F-4565-9164-39C4925E467B} | HiddenByDefault -",
     L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MyComputer\\NameSpace\\{374DE290-123F-4565-9164-39C4925E467B} | HiddenByDefault = dword:1\nHKLM\\SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MyComputer\\NameSpace\\{374DE290-123F-4565-9164-39C4925E467B} | HiddenByDefault = dword:1",
     L""},
    {L"navpane.thisPcMusic", L"This PC: Music folder", 't', 'e', true, false, nullptr,
     L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MyComputer\\NameSpace\\{1CF1260C-4DD0-4ebb-811F-33C572699FDE} | HiddenByDefault -\nHKLM\\SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MyComputer\\NameSpace\\{1CF1260C-4DD0-4ebb-811F-33C572699FDE} | HiddenByDefault -",
     L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MyComputer\\NameSpace\\{1CF1260C-4DD0-4ebb-811F-33C572699FDE} | HiddenByDefault = dword:1\nHKLM\\SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MyComputer\\NameSpace\\{1CF1260C-4DD0-4ebb-811F-33C572699FDE} | HiddenByDefault = dword:1",
     L""},
    {L"navpane.thisPcPictures", L"This PC: Pictures folder", 't', 'e', true, false, nullptr,
     L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MyComputer\\NameSpace\\{3ADD1653-EB32-4cb0-BBD7-DFA0ABB5ACCA} | HiddenByDefault -\nHKLM\\SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MyComputer\\NameSpace\\{3ADD1653-EB32-4cb0-BBD7-DFA0ABB5ACCA} | HiddenByDefault -",
     L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MyComputer\\NameSpace\\{3ADD1653-EB32-4cb0-BBD7-DFA0ABB5ACCA} | HiddenByDefault = dword:1\nHKLM\\SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MyComputer\\NameSpace\\{3ADD1653-EB32-4cb0-BBD7-DFA0ABB5ACCA} | HiddenByDefault = dword:1",
     L""},
    {L"navpane.thisPcVideos", L"This PC: Videos folder", 't', 'e', true, false, nullptr,
     L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MyComputer\\NameSpace\\{A0953C92-50DC-43bf-BE83-3742FED03C9C} | HiddenByDefault -\nHKLM\\SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MyComputer\\NameSpace\\{A0953C92-50DC-43bf-BE83-3742FED03C9C} | HiddenByDefault -",
     L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MyComputer\\NameSpace\\{A0953C92-50DC-43bf-BE83-3742FED03C9C} | HiddenByDefault = dword:1\nHKLM\\SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MyComputer\\NameSpace\\{A0953C92-50DC-43bf-BE83-3742FED03C9C} | HiddenByDefault = dword:1",
     L""},
    {L"navpane.thisPcFolders", L"This PC: your own folders", 's', 'e', false, false, nullptr,
     L"folders MyComputer {}",
     L"folders MyComputer",
     L""},
    {L"navpane.driveLetters", L"Drive letters in This PC", 'c', 'e', true, false, nullptr,
     L"after\036HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer | ShowDriveLettersFirst -\nHKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer | ShowDriveLettersFirst -\037before\036HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer | ShowDriveLettersFirst = dword:4\037network\036HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer | ShowDriveLettersFirst = dword:1\037hidden\036HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer | ShowDriveLettersFirst = dword:2",
     L"",
     L""},
    {L"navpane.hiddenDrives", L"Hidden drives", 's', 'e', true, false, nullptr,
     L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer | NoDrives = dword:{drives}",
     L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer | NoDrives -",
     L""},
    {L"navpane.librariesName", L"Libraries: name", 's', 'e', false, false, nullptr,
     L"HKCU\\Software\\Classes\\CLSID\\{031E4825-7B94-4dc3-B131-E946B44C8DD5} | LocalizedString = sz:{}",
     L"HKCU\\Software\\Classes\\CLSID\\{031E4825-7B94-4dc3-B131-E946B44C8DD5} | LocalizedString -",
     L""},
    {L"navpane.librariesIcon", L"Libraries: icon", 's', 'e', false, false, nullptr,
     L"HKCU\\Software\\Classes\\CLSID\\{031E4825-7B94-4dc3-B131-E946B44C8DD5}\\DefaultIcon | @ = sz:{}",
     L"HKCU\\Software\\Classes\\CLSID\\{031E4825-7B94-4dc3-B131-E946B44C8DD5}\\DefaultIcon | -",
     L""},
    {L"icons.shortcutArrow", L"Shortcut arrow", 's', 'e', true, false, nullptr,
     L"shellicon 29 {}",
     L"shellicon 29",
     L""},
    {L"icons.noShortcutText", L"No \" - Shortcut\" on new shortcuts", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\NamingTemplates | ShortcutNameTemplate = sz:\"%s.lnk\"",
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\NamingTemplates | ShortcutNameTemplate -",
     L""},
    {L"icons.compressedOverlay", L"Compressed file arrows", 's', 'e', true, false, nullptr,
     L"shellicon 179 {}",
     L"shellicon 179",
     L""},
    {L"icons.driveIcon", L"One icon for every drive", 's', 'e', true, false, nullptr,
     L"shellicon 8 {}",
     L"shellicon 8",
     L""},
    {L"icons.driveIcons", L"Icons for single drives", 's', 'e', true, false, nullptr,
     L"driveicons {}",
     L"driveicons",
     L""},
    {L"icons.noFolderThumbnails", L"Plain folder icons", 't', 'e', false, false, nullptr,
     L"HKCU\\Software\\Classes\\Folder\\ShellEx\\{e357fccd-a995-4576-b01f-234630154e96} | @ = sz:",
     L"HKCU\\Software\\Classes\\Folder\\ShellEx\\{e357fccd-a995-4576-b01f-234630154e96} | -",
     L""},
    {L"icons.thumbnailFrame", L"Picture thumbnail frame", 'c', 'e', false, false, nullptr,
     L"default\036HKCU\\Software\\Classes\\SystemFileAssociations\\image | Treatment -\037none\036HKCU\\Software\\Classes\\SystemFileAssociations\\image | Treatment = dword:0\037shadow\036HKCU\\Software\\Classes\\SystemFileAssociations\\image | Treatment = dword:1\037photo\036HKCU\\Software\\Classes\\SystemFileAssociations\\image | Treatment = dword:2\037film\036HKCU\\Software\\Classes\\SystemFileAssociations\\image | Treatment = dword:3",
     L"",
     L""},
    {L"icons.iconCacheKB", L"Icon cache size (KB)", 'n', 'r', true, false, L"500",
     L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer | Max Cached Icons = sz:{}",
     L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer | Max Cached Icons -",
     L""},
    {L"menu.classicMenus", L"Full right-click menus", 't', 'e', false, false, nullptr,
     L"HKCU\\Software\\Classes\\CLSID\\{86ca1aa0-34aa-4e8b-a509-50c905bae2a2}\\InprocServer32 | @ = sz:",
     L"HKCU\\Software\\Classes\\CLSID\\{86ca1aa0-34aa-4e8b-a509-50c905bae2a2} | -",
     L""},
    {L"menu.multiSelectLimit", L"Open, Print and Edit for up to this many files", 'n', 0, false, false, L"15",
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer | MultipleInvokePromptMinimum = dword:{}",
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer | MultipleInvokePromptMinimum -",
     L""},
    {L"menu.editImageApp", L"Program for Edit on pictures", 's', 0, true, false, nullptr,
     L"HKLM\\SOFTWARE\\Classes\\SystemFileAssociations\\image\\shell\\edit\\command | @ = expand:{}",
     L"HKLM\\SOFTWARE\\Classes\\SystemFileAssociations\\image\\shell\\edit\\command | @ = expand:\"%systemroot%\\system32\\mspaint.exe\" \"%1\"",
     L""},
    {L"menu.runAsUserAlways", L"\"Run as different user\" without Shift", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\exefile\\shell\\runasuser | @ = sz:@shell32.dll,-50944\nHKCU\\Software\\Classes\\exefile\\shell\\runasuser\\command | DelegateExecute = sz:{ea72d00e-4960-42fa-ba92-7792a7944c1d}\nHKCU\\Software\\Classes\\Msi.Package\\shell\\runasuser | @ = sz:@shell32.dll,-50944\nHKCU\\Software\\Classes\\Msi.Package\\shell\\runasuser\\command | DelegateExecute = sz:{ea72d00e-4960-42fa-ba92-7792a7944c1d}\nHKCU\\Software\\Classes\\batfile\\shell\\runasuser | @ = sz:@shell32.dll,-50944\nHKCU\\Software\\Classes\\batfile\\shell\\runasuser\\command | DelegateExecute = sz:{ea72d00e-4960-42fa-ba92-7792a7944c1d}\nHKCU\\Software\\Classes\\cmdfile\\shell\\runasuser | @ = sz:@shell32.dll,-50944\nHKCU\\Software\\Classes\\cmdfile\\shell\\runasuser\\command | DelegateExecute = sz:{ea72d00e-4960-42fa-ba92-7792a7944c1d}",
     L"HKCU\\Software\\Classes\\exefile\\shell\\runasuser | -\nHKCU\\Software\\Classes\\Msi.Package\\shell\\runasuser | -\nHKCU\\Software\\Classes\\batfile\\shell\\runasuser | -\nHKCU\\Software\\Classes\\cmdfile\\shell\\runasuser | -",
     L""},
    {L"menu.newProcessAlways", L"\"Open in new process\" without Shift", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\Folder\\shell\\opennewprocess | MUIVerb = sz:@shell32.dll,-8518\nHKCU\\Software\\Classes\\Folder\\shell\\opennewprocess | MultiSelectModel = sz:Document\nHKCU\\Software\\Classes\\Folder\\shell\\opennewprocess | LaunchExplorerFlags = dword:3\nHKCU\\Software\\Classes\\Folder\\shell\\opennewprocess | ExplorerHost = sz:{ceff45ee-c862-41de-aee2-a022c81eda92}\nHKCU\\Software\\Classes\\Folder\\shell\\opennewprocess\\command | DelegateExecute = sz:{11dbb47c-a525-400b-9e80-a54615a090c0}",
     L"HKCU\\Software\\Classes\\Folder\\shell\\opennewprocess | -",
     L""},
    {L"menu.removeAskCopilot", L"Remove \"Ask Copilot\"", 't', 0, true, false, nullptr,
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {CB3B0003-8088-4EDE-8769-8B354AB2FF8C} = sz:",
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {CB3B0003-8088-4EDE-8769-8B354AB2FF8C} -\nHKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {CB3B0003-8088-4EDE-8769-8B354AB2FF8C} -",
     L""},
    {L"menu.removeClipchamp", L"Remove \"Edit in Clipchamp\"", 't', 0, true, false, nullptr,
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {8AB635F8-9A67-4698-AB99-784AD929F3B4} = sz:",
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {8AB635F8-9A67-4698-AB99-784AD929F3B4} -\nHKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {8AB635F8-9A67-4698-AB99-784AD929F3B4} -",
     L""},
    {L"menu.removeEditNotepad", L"Remove \"Edit in Notepad\"", 't', 0, true, false, nullptr,
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {CA6CC9F1-867A-481E-951E-A28C5E4F01EA} = sz:",
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {CA6CC9F1-867A-481E-951E-A28C5E4F01EA} -\nHKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {CA6CC9F1-867A-481E-951E-A28C5E4F01EA} -",
     L""},
    {L"menu.removeEditPhotos", L"Remove \"Edit with Photos\"", 't', 0, true, false, nullptr,
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {BFE0E2A4-C70C-4AD7-AC3D-10D1ECEBB5B4} = sz:",
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {BFE0E2A4-C70C-4AD7-AC3D-10D1ECEBB5B4} -\nHKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {BFE0E2A4-C70C-4AD7-AC3D-10D1ECEBB5B4} -",
     L""},
    {L"menu.removePaint3d", L"Remove \"Edit with Paint 3D\"", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\SystemFileAssociations\\.bmp\\Shell\\3D Edit | ProgrammaticAccessOnly = sz:\nHKCU\\Software\\Classes\\SystemFileAssociations\\.gif\\Shell\\3D Edit | ProgrammaticAccessOnly = sz:\nHKCU\\Software\\Classes\\SystemFileAssociations\\.jpg\\Shell\\3D Edit | ProgrammaticAccessOnly = sz:\nHKCU\\Software\\Classes\\SystemFileAssociations\\.jpeg\\Shell\\3D Edit | ProgrammaticAccessOnly = sz:\nHKCU\\Software\\Classes\\SystemFileAssociations\\.png\\Shell\\3D Edit | ProgrammaticAccessOnly = sz:\nHKCU\\Software\\Classes\\SystemFileAssociations\\.tif\\Shell\\3D Edit | ProgrammaticAccessOnly = sz:\nHKCU\\Software\\Classes\\SystemFileAssociations\\.tiff\\Shell\\3D Edit | ProgrammaticAccessOnly = sz:",
     L"HKCU\\Software\\Classes\\SystemFileAssociations\\.bmp\\Shell\\3D Edit | -\nHKCU\\Software\\Classes\\SystemFileAssociations\\.gif\\Shell\\3D Edit | -\nHKCU\\Software\\Classes\\SystemFileAssociations\\.jpg\\Shell\\3D Edit | -\nHKCU\\Software\\Classes\\SystemFileAssociations\\.jpeg\\Shell\\3D Edit | -\nHKCU\\Software\\Classes\\SystemFileAssociations\\.png\\Shell\\3D Edit | -\nHKCU\\Software\\Classes\\SystemFileAssociations\\.tif\\Shell\\3D Edit | -\nHKCU\\Software\\Classes\\SystemFileAssociations\\.tiff\\Shell\\3D Edit | -",
     L""},
    {L"menu.removeCreateVideo", L"Remove \"Create a new video\"", 't', 0, true, false, nullptr,
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {1100CBCD-B822-43F0-84CB-16814C2F6B44} = sz:",
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {1100CBCD-B822-43F0-84CB-16814C2F6B44} -\nHKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {1100CBCD-B822-43F0-84CB-16814C2F6B44} -",
     L""},
    {L"menu.removeRotate", L"Remove \"Rotate left/right\"", 't', 0, true, false, nullptr,
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {FFE2A43C-56B9-4bf5-9A79-CC6D4285608A} = sz:",
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {FFE2A43C-56B9-4bf5-9A79-CC6D4285608A} -\nHKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {FFE2A43C-56B9-4bf5-9A79-CC6D4285608A} -",
     L""},
    {L"menu.removeMediaPlayer", L"Remove Windows Media Player entries", 't', 0, true, false, nullptr,
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {F1B9284F-E9DC-4e68-9D7E-42362A59F0FD} = sz:\nHKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {CE3FB1D1-02AE-4a5f-A6E9-D9F1B4073E6C} = sz:\nHKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {8A734961-C4AA-4741-AC1E-791ACEBF5B39} = sz:",
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {F1B9284F-E9DC-4e68-9D7E-42362A59F0FD} -\nHKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {CE3FB1D1-02AE-4a5f-A6E9-D9F1B4073E6C} -\nHKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {8A734961-C4AA-4741-AC1E-791ACEBF5B39} -\nHKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {F1B9284F-E9DC-4e68-9D7E-42362A59F0FD} -\nHKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {CE3FB1D1-02AE-4a5f-A6E9-D9F1B4073E6C} -\nHKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {8A734961-C4AA-4741-AC1E-791ACEBF5B39} -",
     L""},
    {L"menu.removeCast", L"Remove \"Cast to device\"", 't', 0, true, false, nullptr,
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {7AD84985-87B4-4a16-BE58-8B72A5B390F7} = sz:",
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {7AD84985-87B4-4a16-BE58-8B72A5B390F7} -\nHKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {7AD84985-87B4-4a16-BE58-8B72A5B390F7} -",
     L""},
    {L"menu.removeShare", L"Remove \"Share\"", 't', 0, true, false, nullptr,
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {e2bf9676-5f8f-435c-97eb-11607a5bedf7} = sz:",
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {e2bf9676-5f8f-435c-97eb-11607a5bedf7} -\nHKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {e2bf9676-5f8f-435c-97eb-11607a5bedf7} -",
     L""},
    {L"menu.removeGiveAccess", L"Remove \"Give access to\"", 't', 0, true, false, nullptr,
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {f81e9010-6ea4-11ce-a7ff-00aa003ca9f6} = sz:",
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {f81e9010-6ea4-11ce-a7ff-00aa003ca9f6} -\nHKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {f81e9010-6ea4-11ce-a7ff-00aa003ca9f6} -",
     L""},
    {L"menu.removeOneDrive", L"Remove OneDrive entries", 't', 0, true, false, nullptr,
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {CB3D0F55-BC2C-4C1A-85ED-23ED75B5106B} = sz:",
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {CB3D0F55-BC2C-4C1A-85ED-23ED75B5106B} -\nHKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {CB3D0F55-BC2C-4C1A-85ED-23ED75B5106B} -",
     L""},
    {L"menu.removeFavorites", L"Remove \"Add to Favorites\"", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\*\\shell\\pintohomefile | ProgrammaticAccessOnly = sz:",
     L"HKCU\\Software\\Classes\\*\\shell\\pintohomefile | -",
     L""},
    {L"menu.removeIncludeLibrary", L"Remove \"Include in library\"", 't', 0, true, false, nullptr,
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {3dad6c5d-2167-4cae-9914-f99e41c12cfa} = sz:",
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {3dad6c5d-2167-4cae-9914-f99e41c12cfa} -\nHKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {3dad6c5d-2167-4cae-9914-f99e41c12cfa} -",
     L""},
    {L"menu.removePinTaskbar", L"Remove \"Pin to taskbar\"", 't', 0, true, false, nullptr,
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {90AA3A4E-1CBA-4233-B8BB-535773D48449} = sz:",
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {90AA3A4E-1CBA-4233-B8BB-535773D48449} -\nHKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {90AA3A4E-1CBA-4233-B8BB-535773D48449} -",
     L""},
    {L"menu.removePinStart", L"Remove \"Pin to Start\"", 't', 0, true, false, nullptr,
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {470C0EBD-5D73-4d58-9CED-E91E22E23282} = sz:",
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {470C0EBD-5D73-4d58-9CED-E91E22E23282} -\nHKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {470C0EBD-5D73-4d58-9CED-E91E22E23282} -",
     L""},
    {L"menu.removeOpenInTerminal", L"Remove \"Open in Terminal\"", 't', 0, true, false, nullptr,
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {9F156763-7844-4DC4-B2B1-901F640F5155} = sz:\nHKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {02DB545A-3E20-46DE-83A5-1329B1E88B6B} = sz:",
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {9F156763-7844-4DC4-B2B1-901F640F5155} -\nHKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {02DB545A-3E20-46DE-83A5-1329B1E88B6B} -\nHKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {9F156763-7844-4DC4-B2B1-901F640F5155} -\nHKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {02DB545A-3E20-46DE-83A5-1329B1E88B6B} -",
     L""},
    {L"menu.removePrint", L"Remove \"Print\" (images, text and scripts)", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\SystemFileAssociations\\image\\shell\\print | ProgrammaticAccessOnly = sz:\nHKCU\\Software\\Classes\\txtfile\\shell\\print | ProgrammaticAccessOnly = sz:\nHKCU\\Software\\Classes\\batfile\\shell\\print | ProgrammaticAccessOnly = sz:\nHKCU\\Software\\Classes\\cmdfile\\shell\\print | ProgrammaticAccessOnly = sz:\nHKCU\\Software\\Classes\\regfile\\shell\\print | ProgrammaticAccessOnly = sz:\nHKCU\\Software\\Classes\\VBSFile\\shell\\Print | ProgrammaticAccessOnly = sz:",
     L"HKCU\\Software\\Classes\\SystemFileAssociations\\image\\shell\\print | -\nHKCU\\Software\\Classes\\txtfile\\shell\\print | -\nHKCU\\Software\\Classes\\batfile\\shell\\print | -\nHKCU\\Software\\Classes\\cmdfile\\shell\\print | -\nHKCU\\Software\\Classes\\regfile\\shell\\print | -\nHKCU\\Software\\Classes\\VBSFile\\shell\\Print | -",
     L""},
    {L"menu.removeCompatibility", L"Remove \"Troubleshoot compatibility\"", 't', 0, true, false, nullptr,
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {1d27f844-3a1f-4410-85ac-14651078412d} = sz:",
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {1d27f844-3a1f-4410-85ac-14651078412d} -\nHKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {1d27f844-3a1f-4410-85ac-14651078412d} -",
     L""},
    {L"menu.removePreviousVersions", L"Remove \"Restore previous versions\"", 't', 0, true, false, nullptr,
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {596AB062-B4D2-4215-9F74-E9109B0A8153} = sz:",
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {596AB062-B4D2-4215-9F74-E9109B0A8153} -\nHKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {596AB062-B4D2-4215-9F74-E9109B0A8153} -",
     L""},
    {L"menu.removeBurnImage", L"Remove \"Burn disc image\"", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\Windows.IsoFile\\shell\\burn | ProgrammaticAccessOnly = sz:",
     L"HKCU\\Software\\Classes\\Windows.IsoFile\\shell\\burn | -",
     L""},
    {L"menu.removeExtractAll", L"Remove \"Extract all\"", 't', 0, true, false, nullptr,
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {b8cdcb65-b1bf-4b42-9428-1dfdb7ee92af} = sz:",
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {b8cdcb65-b1bf-4b42-9428-1dfdb7ee92af} -\nHKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {b8cdcb65-b1bf-4b42-9428-1dfdb7ee92af} -",
     L""},
    {L"menu.removeEfsOwnership", L"Remove \"File ownership\" (EFS)", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\*\\shell\\UpdateEncryptionSettingsWork | ProgrammaticAccessOnly = sz:\nHKCU\\Software\\Classes\\Directory\\shell\\UpdateEncryptionSettings | ProgrammaticAccessOnly = sz:",
     L"HKCU\\Software\\Classes\\*\\shell\\UpdateEncryptionSettingsWork | -\nHKCU\\Software\\Classes\\Directory\\shell\\UpdateEncryptionSettings | -",
     L""},
    {L"menu.removeBitlocker", L"Remove BitLocker entries from drives", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\Drive\\shell\\encrypt-bde | ProgrammaticAccessOnly = sz:\nHKCU\\Software\\Classes\\Drive\\shell\\encrypt-bde-elev | ProgrammaticAccessOnly = sz:\nHKCU\\Software\\Classes\\Drive\\shell\\manage-bde | ProgrammaticAccessOnly = sz:\nHKCU\\Software\\Classes\\Drive\\shell\\resume-bde | ProgrammaticAccessOnly = sz:\nHKCU\\Software\\Classes\\Drive\\shell\\resume-bde-elev | ProgrammaticAccessOnly = sz:\nHKCU\\Software\\Classes\\Drive\\shell\\unlock-bde | ProgrammaticAccessOnly = sz:\nHKCU\\Software\\Classes\\Drive\\shell\\change-passphrase | ProgrammaticAccessOnly = sz:\nHKCU\\Software\\Classes\\Drive\\shell\\change-pin | ProgrammaticAccessOnly = sz:",
     L"HKCU\\Software\\Classes\\Drive\\shell\\encrypt-bde | -\nHKCU\\Software\\Classes\\Drive\\shell\\encrypt-bde-elev | -\nHKCU\\Software\\Classes\\Drive\\shell\\manage-bde | -\nHKCU\\Software\\Classes\\Drive\\shell\\resume-bde | -\nHKCU\\Software\\Classes\\Drive\\shell\\resume-bde-elev | -\nHKCU\\Software\\Classes\\Drive\\shell\\unlock-bde | -\nHKCU\\Software\\Classes\\Drive\\shell\\change-passphrase | -\nHKCU\\Software\\Classes\\Drive\\shell\\change-pin | -",
     L""},
    {L"menu.removeDefenderScan", L"Remove \"Scan with Microsoft Defender\"", 't', 0, true, false, nullptr,
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {09A47860-11B0-4DA5-AFA5-26D86198A780} = sz:",
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {09A47860-11B0-4DA5-AFA5-26D86198A780} -\nHKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked | {09A47860-11B0-4DA5-AFA5-26D86198A780} -",
     L""},
    {L"menuFiles.cmdHere", L"Folders: Command prompt here", 'c', 0, false, false, nullptr,
     L"disabled\036HKCU\\Software\\Classes\\Directory\\shell\\TourneCmd | -\nHKCU\\Software\\Classes\\Directory\\Background\\shell\\TourneCmd | -\nHKCU\\Software\\Classes\\Drive\\shell\\TourneCmd | -\037enabled\036HKCU\\Software\\Classes\\Directory\\shell\\TourneCmd | MUIVerb = sz:Open command window here\nHKCU\\Software\\Classes\\Directory\\shell\\TourneCmd | Extended -\nHKCU\\Software\\Classes\\Directory\\shell\\TourneCmd | Icon = sz:cmd.exe\nHKCU\\Software\\Classes\\Directory\\shell\\TourneCmd\\command | @ = sz:cmd.exe /s /k pushd \"%V\"\nHKCU\\Software\\Classes\\Directory\\Background\\shell\\TourneCmd | MUIVerb = sz:Open command window here\nHKCU\\Software\\Classes\\Directory\\Background\\shell\\TourneCmd | Extended -\nHKCU\\Software\\Classes\\Directory\\Background\\shell\\TourneCmd | Icon = sz:cmd.exe\nHKCU\\Software\\Classes\\Directory\\Background\\shell\\TourneCmd\\command | @ = sz:cmd.exe /s /k pushd \"%V\"\nHKCU\\Software\\Classes\\Drive\\shell\\TourneCmd | MUIVerb = sz:Open command window here\nHKCU\\Software\\Classes\\Drive\\shell\\TourneCmd | Extended -\nHKCU\\Software\\Classes\\Drive\\shell\\TourneCmd | Icon = sz:cmd.exe\nHKCU\\Software\\Classes\\Drive\\shell\\TourneCmd\\command | @ = sz:cmd.exe /s /k pushd \"%V\"\037shift\036HKCU\\Software\\Classes\\Directory\\shell\\TourneCmd | MUIVerb = sz:Open command window here\nHKCU\\Software\\Classes\\Directory\\shell\\TourneCmd | Extended = sz:\nHKCU\\Software\\Classes\\Directory\\shell\\TourneCmd | Icon = sz:cmd.exe\nHKCU\\Software\\Classes\\Directory\\shell\\TourneCmd\\command | @ = sz:cmd.exe /s /k pushd \"%V\"\nHKCU\\Software\\Classes\\Directory\\Background\\shell\\TourneCmd | MUIVerb = sz:Open command window here\nHKCU\\Software\\Classes\\Directory\\Background\\shell\\TourneCmd | Extended = sz:\nHKCU\\Software\\Classes\\Directory\\Background\\shell\\TourneCmd | Icon = sz:cmd.exe\nHKCU\\Software\\Classes\\Directory\\Background\\shell\\TourneCmd\\command | @ = sz:cmd.exe /s /k pushd \"%V\"\nHKCU\\Software\\Classes\\Drive\\shell\\TourneCmd | MUIVerb = sz:Open command window here\nHKCU\\Software\\Classes\\Drive\\shell\\TourneCmd | Extended = sz:\nHKCU\\Software\\Classes\\Drive\\shell\\TourneCmd | Icon = sz:cmd.exe\nHKCU\\Software\\Classes\\Drive\\shell\\TourneCmd\\command | @ = sz:cmd.exe /s /k pushd \"%V\"",
     L"",
     L""},
    {L"menuFiles.cmdHereAdmin", L"Folders: Command prompt here as administrator", 'c', 0, false, false, nullptr,
     L"disabled\036HKCU\\Software\\Classes\\Directory\\shell\\TourneCmdAdmin | -\nHKCU\\Software\\Classes\\Directory\\Background\\shell\\TourneCmdAdmin | -\nHKCU\\Software\\Classes\\Drive\\shell\\TourneCmdAdmin | -\037enabled\036HKCU\\Software\\Classes\\Directory\\shell\\TourneCmdAdmin | MUIVerb = sz:Command prompt here (administrator)\nHKCU\\Software\\Classes\\Directory\\shell\\TourneCmdAdmin | Extended -\nHKCU\\Software\\Classes\\Directory\\shell\\TourneCmdAdmin | HasLUAShield = sz:\nHKCU\\Software\\Classes\\Directory\\shell\\TourneCmdAdmin\\command | @ = sz:powershell.exe -NoProfile -WindowStyle Hidden -Command \"Start-Process cmd.exe -Verb RunAs -ArgumentList '/s','/k','pushd','\\\"%V\\\"'\"\nHKCU\\Software\\Classes\\Directory\\Background\\shell\\TourneCmdAdmin | MUIVerb = sz:Command prompt here (administrator)\nHKCU\\Software\\Classes\\Directory\\Background\\shell\\TourneCmdAdmin | Extended -\nHKCU\\Software\\Classes\\Directory\\Background\\shell\\TourneCmdAdmin | HasLUAShield = sz:\nHKCU\\Software\\Classes\\Directory\\Background\\shell\\TourneCmdAdmin\\command | @ = sz:powershell.exe -NoProfile -WindowStyle Hidden -Command \"Start-Process cmd.exe -Verb RunAs -ArgumentList '/s','/k','pushd','\\\"%V\\\"'\"\nHKCU\\Software\\Classes\\Drive\\shell\\TourneCmdAdmin | MUIVerb = sz:Command prompt here (administrator)\nHKCU\\Software\\Classes\\Drive\\shell\\TourneCmdAdmin | Extended -\nHKCU\\Software\\Classes\\Drive\\shell\\TourneCmdAdmin | HasLUAShield = sz:\nHKCU\\Software\\Classes\\Drive\\shell\\TourneCmdAdmin\\command | @ = sz:powershell.exe -NoProfile -WindowStyle Hidden -Command \"Start-Process cmd.exe -Verb RunAs -ArgumentList '/s','/k','pushd','\\\"%V\\\"'\"\037shift\036HKCU\\Software\\Classes\\Directory\\shell\\TourneCmdAdmin | MUIVerb = sz:Command prompt here (administrator)\nHKCU\\Software\\Classes\\Directory\\shell\\TourneCmdAdmin | Extended = sz:\nHKCU\\Software\\Classes\\Directory\\shell\\TourneCmdAdmin | HasLUAShield = sz:\nHKCU\\Software\\Classes\\Directory\\shell\\TourneCmdAdmin\\command | @ = sz:powershell.exe -NoProfile -WindowStyle Hidden -Command \"Start-Process cmd.exe -Verb RunAs -ArgumentList '/s','/k','pushd','\\\"%V\\\"'\"\nHKCU\\Software\\Classes\\Directory\\Background\\shell\\TourneCmdAdmin | MUIVerb = sz:Command prompt here (administrator)\nHKCU\\Software\\Classes\\Directory\\Background\\shell\\TourneCmdAdmin | Extended = sz:\nHKCU\\Software\\Classes\\Directory\\Background\\shell\\TourneCmdAdmin | HasLUAShield = sz:\nHKCU\\Software\\Classes\\Directory\\Background\\shell\\TourneCmdAdmin\\command | @ = sz:powershell.exe -NoProfile -WindowStyle Hidden -Command \"Start-Process cmd.exe -Verb RunAs -ArgumentList '/s','/k','pushd','\\\"%V\\\"'\"\nHKCU\\Software\\Classes\\Drive\\shell\\TourneCmdAdmin | MUIVerb = sz:Command prompt here (administrator)\nHKCU\\Software\\Classes\\Drive\\shell\\TourneCmdAdmin | Extended = sz:\nHKCU\\Software\\Classes\\Drive\\shell\\TourneCmdAdmin | HasLUAShield = sz:\nHKCU\\Software\\Classes\\Drive\\shell\\TourneCmdAdmin\\command | @ = sz:powershell.exe -NoProfile -WindowStyle Hidden -Command \"Start-Process cmd.exe -Verb RunAs -ArgumentList '/s','/k','pushd','\\\"%V\\\"'\"",
     L"",
     L""},
    {L"menuFiles.psHereAdmin", L"Folders: PowerShell here as administrator", 'c', 0, false, false, nullptr,
     L"disabled\036HKCU\\Software\\Classes\\Directory\\shell\\TournePsAdmin | -\nHKCU\\Software\\Classes\\Directory\\Background\\shell\\TournePsAdmin | -\nHKCU\\Software\\Classes\\Drive\\shell\\TournePsAdmin | -\037enabled\036HKCU\\Software\\Classes\\Directory\\shell\\TournePsAdmin | MUIVerb = sz:PowerShell here (administrator)\nHKCU\\Software\\Classes\\Directory\\shell\\TournePsAdmin | Extended -\nHKCU\\Software\\Classes\\Directory\\shell\\TournePsAdmin | HasLUAShield = sz:\nHKCU\\Software\\Classes\\Directory\\shell\\TournePsAdmin\\command | @ = sz:powershell.exe -NoProfile -WindowStyle Hidden -Command \"Start-Process powershell.exe -Verb RunAs -ArgumentList '-NoExit','-Command','Set-Location -LiteralPath ''%V'''\"\nHKCU\\Software\\Classes\\Directory\\Background\\shell\\TournePsAdmin | MUIVerb = sz:PowerShell here (administrator)\nHKCU\\Software\\Classes\\Directory\\Background\\shell\\TournePsAdmin | Extended -\nHKCU\\Software\\Classes\\Directory\\Background\\shell\\TournePsAdmin | HasLUAShield = sz:\nHKCU\\Software\\Classes\\Directory\\Background\\shell\\TournePsAdmin\\command | @ = sz:powershell.exe -NoProfile -WindowStyle Hidden -Command \"Start-Process powershell.exe -Verb RunAs -ArgumentList '-NoExit','-Command','Set-Location -LiteralPath ''%V'''\"\nHKCU\\Software\\Classes\\Drive\\shell\\TournePsAdmin | MUIVerb = sz:PowerShell here (administrator)\nHKCU\\Software\\Classes\\Drive\\shell\\TournePsAdmin | Extended -\nHKCU\\Software\\Classes\\Drive\\shell\\TournePsAdmin | HasLUAShield = sz:\nHKCU\\Software\\Classes\\Drive\\shell\\TournePsAdmin\\command | @ = sz:powershell.exe -NoProfile -WindowStyle Hidden -Command \"Start-Process powershell.exe -Verb RunAs -ArgumentList '-NoExit','-Command','Set-Location -LiteralPath ''%V'''\"\037shift\036HKCU\\Software\\Classes\\Directory\\shell\\TournePsAdmin | MUIVerb = sz:PowerShell here (administrator)\nHKCU\\Software\\Classes\\Directory\\shell\\TournePsAdmin | Extended = sz:\nHKCU\\Software\\Classes\\Directory\\shell\\TournePsAdmin | HasLUAShield = sz:\nHKCU\\Software\\Classes\\Directory\\shell\\TournePsAdmin\\command | @ = sz:powershell.exe -NoProfile -WindowStyle Hidden -Command \"Start-Process powershell.exe -Verb RunAs -ArgumentList '-NoExit','-Command','Set-Location -LiteralPath ''%V'''\"\nHKCU\\Software\\Classes\\Directory\\Background\\shell\\TournePsAdmin | MUIVerb = sz:PowerShell here (administrator)\nHKCU\\Software\\Classes\\Directory\\Background\\shell\\TournePsAdmin | Extended = sz:\nHKCU\\Software\\Classes\\Directory\\Background\\shell\\TournePsAdmin | HasLUAShield = sz:\nHKCU\\Software\\Classes\\Directory\\Background\\shell\\TournePsAdmin\\command | @ = sz:powershell.exe -NoProfile -WindowStyle Hidden -Command \"Start-Process powershell.exe -Verb RunAs -ArgumentList '-NoExit','-Command','Set-Location -LiteralPath ''%V'''\"\nHKCU\\Software\\Classes\\Drive\\shell\\TournePsAdmin | MUIVerb = sz:PowerShell here (administrator)\nHKCU\\Software\\Classes\\Drive\\shell\\TournePsAdmin | Extended = sz:\nHKCU\\Software\\Classes\\Drive\\shell\\TournePsAdmin | HasLUAShield = sz:\nHKCU\\Software\\Classes\\Drive\\shell\\TournePsAdmin\\command | @ = sz:powershell.exe -NoProfile -WindowStyle Hidden -Command \"Start-Process powershell.exe -Verb RunAs -ArgumentList '-NoExit','-Command','Set-Location -LiteralPath ''%V'''\"",
     L"",
     L""},
    {L"menuFiles.terminalMenu", L"Folders: Windows Terminal", 'c', 0, false, false, nullptr,
     L"disabled\036HKCU\\Software\\Classes\\Directory\\shell\\TourneTerminal | -\nHKCU\\Software\\Classes\\Directory\\Background\\shell\\TourneTerminal | -\037enabled\036HKCU\\Software\\Classes\\Directory\\shell\\TourneTerminal | -\nHKCU\\Software\\Classes\\Directory\\Background\\shell\\TourneTerminal | -\nterminal Directory {}\nterminal Directory\\Background {}\037shift\036HKCU\\Software\\Classes\\Directory\\shell\\TourneTerminal | -\nHKCU\\Software\\Classes\\Directory\\Background\\shell\\TourneTerminal | -\nterminal Directory {}\nterminal Directory\\Background {}",
     L"",
     L""},
    {L"menuFiles.terminalAdminMenu", L"Folders: Windows Terminal as administrator", 'c', 0, false, false, nullptr,
     L"disabled\036HKCU\\Software\\Classes\\Directory\\shell\\TourneTerminalAdmin | -\nHKCU\\Software\\Classes\\Directory\\Background\\shell\\TourneTerminalAdmin | -\037enabled\036HKCU\\Software\\Classes\\Directory\\shell\\TourneTerminalAdmin | MUIVerb = sz:Terminal here (administrator)\nHKCU\\Software\\Classes\\Directory\\shell\\TourneTerminalAdmin | Extended -\nHKCU\\Software\\Classes\\Directory\\shell\\TourneTerminalAdmin | HasLUAShield = sz:\nHKCU\\Software\\Classes\\Directory\\shell\\TourneTerminalAdmin\\command | @ = sz:powershell.exe -NoProfile -WindowStyle Hidden -Command \"Start-Process wt.exe -Verb RunAs -ArgumentList '-d','\\\"%V\\\"'\"\nHKCU\\Software\\Classes\\Directory\\Background\\shell\\TourneTerminalAdmin | MUIVerb = sz:Terminal here (administrator)\nHKCU\\Software\\Classes\\Directory\\Background\\shell\\TourneTerminalAdmin | Extended -\nHKCU\\Software\\Classes\\Directory\\Background\\shell\\TourneTerminalAdmin | HasLUAShield = sz:\nHKCU\\Software\\Classes\\Directory\\Background\\shell\\TourneTerminalAdmin\\command | @ = sz:powershell.exe -NoProfile -WindowStyle Hidden -Command \"Start-Process wt.exe -Verb RunAs -ArgumentList '-d','\\\"%V\\\"'\"\037shift\036HKCU\\Software\\Classes\\Directory\\shell\\TourneTerminalAdmin | MUIVerb = sz:Terminal here (administrator)\nHKCU\\Software\\Classes\\Directory\\shell\\TourneTerminalAdmin | Extended = sz:\nHKCU\\Software\\Classes\\Directory\\shell\\TourneTerminalAdmin | HasLUAShield = sz:\nHKCU\\Software\\Classes\\Directory\\shell\\TourneTerminalAdmin\\command | @ = sz:powershell.exe -NoProfile -WindowStyle Hidden -Command \"Start-Process wt.exe -Verb RunAs -ArgumentList '-d','\\\"%V\\\"'\"\nHKCU\\Software\\Classes\\Directory\\Background\\shell\\TourneTerminalAdmin | MUIVerb = sz:Terminal here (administrator)\nHKCU\\Software\\Classes\\Directory\\Background\\shell\\TourneTerminalAdmin | Extended = sz:\nHKCU\\Software\\Classes\\Directory\\Background\\shell\\TourneTerminalAdmin | HasLUAShield = sz:\nHKCU\\Software\\Classes\\Directory\\Background\\shell\\TourneTerminalAdmin\\command | @ = sz:powershell.exe -NoProfile -WindowStyle Hidden -Command \"Start-Process wt.exe -Verb RunAs -ArgumentList '-d','\\\"%V\\\"'\"",
     L"",
     L""},
    {L"menuFiles.bashHere", L"Folders: Linux shell here", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\Directory\\shell\\TourneWsl | MUIVerb = sz:Open Linux shell here\nHKCU\\Software\\Classes\\Directory\\shell\\TourneWsl | Icon = sz:wsl.exe\nHKCU\\Software\\Classes\\Directory\\shell\\TourneWsl\\command | @ = sz:wsl.exe --cd \"%V\"\nHKCU\\Software\\Classes\\Directory\\Background\\shell\\TourneWsl | MUIVerb = sz:Open Linux shell here\nHKCU\\Software\\Classes\\Directory\\Background\\shell\\TourneWsl | Icon = sz:wsl.exe\nHKCU\\Software\\Classes\\Directory\\Background\\shell\\TourneWsl\\command | @ = sz:wsl.exe --cd \"%V\"",
     L"HKCU\\Software\\Classes\\Directory\\shell\\TourneWsl | -\nHKCU\\Software\\Classes\\Directory\\Background\\shell\\TourneWsl | -",
     L""},
    {L"menuFiles.caseSensitivity", L"Folders: Case sensitivity", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\Directory\\shell\\TourneCase | MUIVerb = sz:Case sensitivity\nHKCU\\Software\\Classes\\Directory\\shell\\TourneCase | SubCommands = sz:\nHKCU\\Software\\Classes\\Directory\\shell\\TourneCase\\shell\\1on | MUIVerb = sz:Turn on\nHKCU\\Software\\Classes\\Directory\\shell\\TourneCase\\shell\\1on | HasLUAShield = sz:\nHKCU\\Software\\Classes\\Directory\\shell\\TourneCase\\shell\\1on\\command | @ = sz:powershell.exe -NoProfile -WindowStyle Hidden -Command \"Start-Process fsutil.exe -Verb RunAs -ArgumentList 'file','setCaseSensitiveInfo','\\\"%1\\\"','enable'\"\nHKCU\\Software\\Classes\\Directory\\shell\\TourneCase\\shell\\2off | MUIVerb = sz:Turn off\nHKCU\\Software\\Classes\\Directory\\shell\\TourneCase\\shell\\2off | HasLUAShield = sz:\nHKCU\\Software\\Classes\\Directory\\shell\\TourneCase\\shell\\2off\\command | @ = sz:powershell.exe -NoProfile -WindowStyle Hidden -Command \"Start-Process fsutil.exe -Verb RunAs -ArgumentList 'file','setCaseSensitiveInfo','\\\"%1\\\"','disable'\"\nHKCU\\Software\\Classes\\Directory\\shell\\TourneCase\\shell\\3query | MUIVerb = sz:Show state\nHKCU\\Software\\Classes\\Directory\\shell\\TourneCase\\shell\\3query\\command | @ = sz:cmd.exe /k fsutil.exe file queryCaseSensitiveInfo \"%1\"",
     L"HKCU\\Software\\Classes\\Directory\\shell\\TourneCase | -",
     L""},
    {L"menuFiles.attributes", L"Files and folders: Attributes", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\AllFilesystemObjects\\shell\\TourneAttributes | MUIVerb = sz:Attributes\nHKCU\\Software\\Classes\\AllFilesystemObjects\\shell\\TourneAttributes | SubCommands = sz:\nHKCU\\Software\\Classes\\AllFilesystemObjects\\shell\\TourneAttributes\\shell\\1hide | MUIVerb = sz:Hide\nHKCU\\Software\\Classes\\AllFilesystemObjects\\shell\\TourneAttributes\\shell\\1hide\\command | @ = sz:attrib.exe +h \"%1\"\nHKCU\\Software\\Classes\\AllFilesystemObjects\\shell\\TourneAttributes\\shell\\2show | MUIVerb = sz:Unhide\nHKCU\\Software\\Classes\\AllFilesystemObjects\\shell\\TourneAttributes\\shell\\2show\\command | @ = sz:attrib.exe -h \"%1\"\nHKCU\\Software\\Classes\\AllFilesystemObjects\\shell\\TourneAttributes\\shell\\3ro | MUIVerb = sz:Make read-only\nHKCU\\Software\\Classes\\AllFilesystemObjects\\shell\\TourneAttributes\\shell\\3ro\\command | @ = sz:attrib.exe +r \"%1\"\nHKCU\\Software\\Classes\\AllFilesystemObjects\\shell\\TourneAttributes\\shell\\4rw | MUIVerb = sz:Make writable\nHKCU\\Software\\Classes\\AllFilesystemObjects\\shell\\TourneAttributes\\shell\\4rw\\command | @ = sz:attrib.exe -r \"%1\"\nHKCU\\Software\\Classes\\AllFilesystemObjects\\shell\\TourneAttributes\\shell\\5sys | MUIVerb = sz:Mark as system\nHKCU\\Software\\Classes\\AllFilesystemObjects\\shell\\TourneAttributes\\shell\\5sys\\command | @ = sz:attrib.exe +s \"%1\"\nHKCU\\Software\\Classes\\AllFilesystemObjects\\shell\\TourneAttributes\\shell\\6nosys | MUIVerb = sz:Unmark as system\nHKCU\\Software\\Classes\\AllFilesystemObjects\\shell\\TourneAttributes\\shell\\6nosys\\command | @ = sz:attrib.exe -s \"%1\"",
     L"HKCU\\Software\\Classes\\AllFilesystemObjects\\shell\\TourneAttributes | -",
     L""},
    {L"menuFiles.viewOwner", L"Files and folders: Show owner", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\AllFilesystemObjects\\shell\\TourneViewOwner | MUIVerb = sz:Show owner\nHKCU\\Software\\Classes\\AllFilesystemObjects\\shell\\TourneViewOwner\\command | @ = sz:powershell.exe -NoProfile -NoExit -Command \"(Get-Acl -LiteralPath '%1').Owner\"",
     L"HKCU\\Software\\Classes\\AllFilesystemObjects\\shell\\TourneViewOwner | -",
     L""},
    {L"menuFiles.takeOwnership", L"Files and folders: Take ownership", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\*\\shell\\TourneTakeOwn | MUIVerb = sz:Take ownership\nHKCU\\Software\\Classes\\*\\shell\\TourneTakeOwn | HasLUAShield = sz:\nHKCU\\Software\\Classes\\*\\shell\\TourneTakeOwn\\command | @ = sz:powershell.exe -NoProfile -WindowStyle Hidden -Command \"Start-Process cmd.exe -Verb RunAs -ArgumentList '/c','takeown /f \\\"%1\\\" && icacls \\\"%1\\\" /grant *S-1-5-32-544:F'\"\nHKCU\\Software\\Classes\\Directory\\shell\\TourneTakeOwn | MUIVerb = sz:Take ownership\nHKCU\\Software\\Classes\\Directory\\shell\\TourneTakeOwn | HasLUAShield = sz:\nHKCU\\Software\\Classes\\Directory\\shell\\TourneTakeOwn\\command | @ = sz:powershell.exe -NoProfile -WindowStyle Hidden -Command \"Start-Process cmd.exe -Verb RunAs -ArgumentList '/c','takeown /f \\\"%1\\\" /r /d y && icacls \\\"%1\\\" /grant *S-1-5-32-544:F /t'\"",
     L"HKCU\\Software\\Classes\\*\\shell\\TourneTakeOwn | -\nHKCU\\Software\\Classes\\Directory\\shell\\TourneTakeOwn | -",
     L""},
    {L"menuFiles.changeOwner", L"Files and folders: Change owner", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\AllFilesystemObjects\\shell\\TourneOwner | MUIVerb = sz:Change owner\nHKCU\\Software\\Classes\\AllFilesystemObjects\\shell\\TourneOwner | SubCommands = sz:\nHKCU\\Software\\Classes\\AllFilesystemObjects\\shell\\TourneOwner | HasLUAShield = sz:\nHKCU\\Software\\Classes\\AllFilesystemObjects\\shell\\TourneOwner\\shell\\1admins | MUIVerb = sz:Administrators\nHKCU\\Software\\Classes\\AllFilesystemObjects\\shell\\TourneOwner\\shell\\1admins\\command | @ = sz:powershell.exe -NoProfile -WindowStyle Hidden -Command \"Start-Process icacls.exe -Verb RunAs -ArgumentList '\\\"%1\\\"','/setowner','*S-1-5-32-544','/T','/C'\"\nHKCU\\Software\\Classes\\AllFilesystemObjects\\shell\\TourneOwner\\shell\\2everyone | MUIVerb = sz:Everyone\nHKCU\\Software\\Classes\\AllFilesystemObjects\\shell\\TourneOwner\\shell\\2everyone\\command | @ = sz:powershell.exe -NoProfile -WindowStyle Hidden -Command \"Start-Process icacls.exe -Verb RunAs -ArgumentList '\\\"%1\\\"','/setowner','*S-1-1-0','/T','/C'\"\nHKCU\\Software\\Classes\\AllFilesystemObjects\\shell\\TourneOwner\\shell\\3system | MUIVerb = sz:SYSTEM\nHKCU\\Software\\Classes\\AllFilesystemObjects\\shell\\TourneOwner\\shell\\3system\\command | @ = sz:powershell.exe -NoProfile -WindowStyle Hidden -Command \"Start-Process icacls.exe -Verb RunAs -ArgumentList '\\\"%1\\\"','/setowner','*S-1-5-18','/T','/C'\"\nHKCU\\Software\\Classes\\AllFilesystemObjects\\shell\\TourneOwner\\shell\\4ti | MUIVerb = sz:TrustedInstaller\nHKCU\\Software\\Classes\\AllFilesystemObjects\\shell\\TourneOwner\\shell\\4ti\\command | @ = sz:powershell.exe -NoProfile -WindowStyle Hidden -Command \"Start-Process icacls.exe -Verb RunAs -ArgumentList '\\\"%1\\\"','/setowner','\\\"NT SERVICE\\TrustedInstaller\\\"','/T','/C'\"\nHKCU\\Software\\Classes\\AllFilesystemObjects\\shell\\TourneOwner\\shell\\5me | MUIVerb = sz:Me\nHKCU\\Software\\Classes\\AllFilesystemObjects\\shell\\TourneOwner\\shell\\5me\\command | @ = sz:powershell.exe -NoProfile -WindowStyle Hidden -Command \"Start-Process takeown.exe -Verb RunAs -ArgumentList '/F','\\\"%1\\\"','/R','/D','Y'\"",
     L"HKCU\\Software\\Classes\\AllFilesystemObjects\\shell\\TourneOwner | -",
     L""},
    {L"menuFiles.resetPermissions", L"Files and folders: Reset permissions", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\AllFilesystemObjects\\shell\\TourneResetAcl | MUIVerb = sz:Reset permissions\nHKCU\\Software\\Classes\\AllFilesystemObjects\\shell\\TourneResetAcl | HasLUAShield = sz:\nHKCU\\Software\\Classes\\AllFilesystemObjects\\shell\\TourneResetAcl\\command | @ = sz:powershell.exe -NoProfile -WindowStyle Hidden -Command \"Start-Process icacls.exe -Verb RunAs -ArgumentList '\\\"%1\\\"','/reset','/T','/C','/Q'\"",
     L"HKCU\\Software\\Classes\\AllFilesystemObjects\\shell\\TourneResetAcl | -",
     L""},
    {L"menuFiles.inheritanceMenu", L"Files and folders: Inherited permissions", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\AllFilesystemObjects\\shell\\TourneInherit | MUIVerb = sz:Inherited permissions\nHKCU\\Software\\Classes\\AllFilesystemObjects\\shell\\TourneInherit | SubCommands = sz:\nHKCU\\Software\\Classes\\AllFilesystemObjects\\shell\\TourneInherit | HasLUAShield = sz:\nHKCU\\Software\\Classes\\AllFilesystemObjects\\shell\\TourneInherit\\shell\\1on | MUIVerb = sz:Turn on\nHKCU\\Software\\Classes\\AllFilesystemObjects\\shell\\TourneInherit\\shell\\1on\\command | @ = sz:powershell.exe -NoProfile -WindowStyle Hidden -Command \"Start-Process icacls.exe -Verb RunAs -ArgumentList '\\\"%1\\\"','/inheritance:e'\"\nHKCU\\Software\\Classes\\AllFilesystemObjects\\shell\\TourneInherit\\shell\\2copy | MUIVerb = sz:Turn off, keep a copy\nHKCU\\Software\\Classes\\AllFilesystemObjects\\shell\\TourneInherit\\shell\\2copy\\command | @ = sz:powershell.exe -NoProfile -WindowStyle Hidden -Command \"Start-Process icacls.exe -Verb RunAs -ArgumentList '\\\"%1\\\"','/inheritance:d'\"\nHKCU\\Software\\Classes\\AllFilesystemObjects\\shell\\TourneInherit\\shell\\3remove | MUIVerb = sz:Turn off and remove\nHKCU\\Software\\Classes\\AllFilesystemObjects\\shell\\TourneInherit\\shell\\3remove\\command | @ = sz:powershell.exe -NoProfile -WindowStyle Hidden -Command \"Start-Process icacls.exe -Verb RunAs -ArgumentList '\\\"%1\\\"','/inheritance:r'\"",
     L"HKCU\\Software\\Classes\\AllFilesystemObjects\\shell\\TourneInherit | -",
     L""},
    {L"menuFiles.encryptMenu", L"Files and folders: Encrypt and Decrypt", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced | EncryptionContextMenu = dword:1",
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced | EncryptionContextMenu -",
     L""},
    {L"menuFiles.hashMenu", L"Files: Hash", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\*\\shell\\TourneHash | MUIVerb = sz:Hash\nHKCU\\Software\\Classes\\*\\shell\\TourneHash | SubCommands = sz:\nHKCU\\Software\\Classes\\*\\shell\\TourneHash\\shell\\1sha256 | MUIVerb = sz:SHA256\nHKCU\\Software\\Classes\\*\\shell\\TourneHash\\shell\\1sha256\\command | @ = sz:powershell.exe -NoProfile -NoExit -Command \"Get-FileHash -LiteralPath '%1' -Algorithm SHA256 | Format-List\"\nHKCU\\Software\\Classes\\*\\shell\\TourneHash\\shell\\2sha1 | MUIVerb = sz:SHA1\nHKCU\\Software\\Classes\\*\\shell\\TourneHash\\shell\\2sha1\\command | @ = sz:powershell.exe -NoProfile -NoExit -Command \"Get-FileHash -LiteralPath '%1' -Algorithm SHA1 | Format-List\"\nHKCU\\Software\\Classes\\*\\shell\\TourneHash\\shell\\3sha384 | MUIVerb = sz:SHA384\nHKCU\\Software\\Classes\\*\\shell\\TourneHash\\shell\\3sha384\\command | @ = sz:powershell.exe -NoProfile -NoExit -Command \"Get-FileHash -LiteralPath '%1' -Algorithm SHA384 | Format-List\"\nHKCU\\Software\\Classes\\*\\shell\\TourneHash\\shell\\4sha512 | MUIVerb = sz:SHA512\nHKCU\\Software\\Classes\\*\\shell\\TourneHash\\shell\\4sha512\\command | @ = sz:powershell.exe -NoProfile -NoExit -Command \"Get-FileHash -LiteralPath '%1' -Algorithm SHA512 | Format-List\"\nHKCU\\Software\\Classes\\*\\shell\\TourneHash\\shell\\5md5 | MUIVerb = sz:MD5\nHKCU\\Software\\Classes\\*\\shell\\TourneHash\\shell\\5md5\\command | @ = sz:powershell.exe -NoProfile -NoExit -Command \"Get-FileHash -LiteralPath '%1' -Algorithm MD5 | Format-List\"\nHKCU\\Software\\Classes\\*\\shell\\TourneHash\\shell\\6all | MUIVerb = sz:All of them\nHKCU\\Software\\Classes\\*\\shell\\TourneHash\\shell\\6all\\command | @ = sz:powershell.exe -NoProfile -NoExit -Command \"'SHA256','SHA1','SHA384','SHA512','MD5' | ForEach-Object { Get-FileHash -LiteralPath '%1' -Algorithm $_ } | Format-List\"",
     L"HKCU\\Software\\Classes\\*\\shell\\TourneHash | -",
     L""},
    {L"menuFiles.unblockMenu", L"Files: Unblock", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\*\\shell\\TourneUnblock | MUIVerb = sz:Unblock\nHKCU\\Software\\Classes\\*\\shell\\TourneUnblock\\command | @ = sz:powershell.exe -NoProfile -WindowStyle Hidden -Command \"Unblock-File -LiteralPath '%1'\"\nHKCU\\Software\\Classes\\Directory\\shell\\TourneUnblock | MUIVerb = sz:Unblock everything inside\nHKCU\\Software\\Classes\\Directory\\shell\\TourneUnblock\\command | @ = sz:powershell.exe -NoProfile -WindowStyle Hidden -Command \"Get-ChildItem -LiteralPath '%1' -Recurse -File | Unblock-File\"",
     L"HKCU\\Software\\Classes\\*\\shell\\TourneUnblock | -\nHKCU\\Software\\Classes\\Directory\\shell\\TourneUnblock | -",
     L""},
    {L"menuFiles.runWithPriority", L"Programs: Run with priority", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\exefile\\shell\\TournePriority | MUIVerb = sz:Run with priority\nHKCU\\Software\\Classes\\exefile\\shell\\TournePriority | SubCommands = sz:\nHKCU\\Software\\Classes\\exefile\\shell\\TournePriority\\shell\\1realtime | MUIVerb = sz:Realtime\nHKCU\\Software\\Classes\\exefile\\shell\\TournePriority\\shell\\1realtime\\command | @ = sz:cmd.exe /c start \"\" /realtime \"%1\"\nHKCU\\Software\\Classes\\exefile\\shell\\TournePriority\\shell\\2high | MUIVerb = sz:High\nHKCU\\Software\\Classes\\exefile\\shell\\TournePriority\\shell\\2high\\command | @ = sz:cmd.exe /c start \"\" /high \"%1\"\nHKCU\\Software\\Classes\\exefile\\shell\\TournePriority\\shell\\3above | MUIVerb = sz:Above normal\nHKCU\\Software\\Classes\\exefile\\shell\\TournePriority\\shell\\3above\\command | @ = sz:cmd.exe /c start \"\" /abovenormal \"%1\"\nHKCU\\Software\\Classes\\exefile\\shell\\TournePriority\\shell\\4normal | MUIVerb = sz:Normal\nHKCU\\Software\\Classes\\exefile\\shell\\TournePriority\\shell\\4normal\\command | @ = sz:cmd.exe /c start \"\" /normal \"%1\"\nHKCU\\Software\\Classes\\exefile\\shell\\TournePriority\\shell\\5below | MUIVerb = sz:Below normal\nHKCU\\Software\\Classes\\exefile\\shell\\TournePriority\\shell\\5below\\command | @ = sz:cmd.exe /c start \"\" /belownormal \"%1\"\nHKCU\\Software\\Classes\\exefile\\shell\\TournePriority\\shell\\6low | MUIVerb = sz:Low\nHKCU\\Software\\Classes\\exefile\\shell\\TournePriority\\shell\\6low\\command | @ = sz:cmd.exe /c start \"\" /low \"%1\"",
     L"HKCU\\Software\\Classes\\exefile\\shell\\TournePriority | -",
     L""},
    {L"menuFiles.runAsPs1", L"PowerShell scripts: Run as administrator", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\SystemFileAssociations\\.ps1\\Shell\\runas | HasLUAShield = sz:\nHKCU\\Software\\Classes\\SystemFileAssociations\\.ps1\\Shell\\runas | MUIVerb = sz:Run as administrator\nHKCU\\Software\\Classes\\SystemFileAssociations\\.ps1\\Shell\\runas\\command | @ = sz:powershell.exe -NoProfile -ExecutionPolicy Bypass -NoExit -File \"%1\"",
     L"HKCU\\Software\\Classes\\SystemFileAssociations\\.ps1\\Shell\\runas | -",
     L""},
    {L"menuFiles.ps1Menu", L"PowerShell scripts: \"Run or modify\" menu", 'c', 0, false, false, nullptr,
     L"disabled\036HKCU\\Software\\Classes\\SystemFileAssociations\\.ps1\\Shell\\TourneRunModify | -\037enabled\036HKCU\\Software\\Classes\\SystemFileAssociations\\.ps1\\Shell\\TourneRunModify | -\nHKCU\\Software\\Classes\\SystemFileAssociations\\.ps1\\Shell\\TourneRunModify | MUIVerb = sz:Run or modify\nHKCU\\Software\\Classes\\SystemFileAssociations\\.ps1\\Shell\\TourneRunModify | SubCommands = sz:\nHKCU\\Software\\Classes\\SystemFileAssociations\\.ps1\\Shell\\TourneRunModify | Icon = sz:powershell.exe\nHKCU\\Software\\Classes\\SystemFileAssociations\\.ps1\\Shell\\TourneRunModify\\shell\\1run | MUIVerb = sz:Run with PowerShell\nHKCU\\Software\\Classes\\SystemFileAssociations\\.ps1\\Shell\\TourneRunModify\\shell\\1run\\command | @ = sz:powershell.exe -NoProfile -ExecutionPolicy Bypass -NoExit -File \"%1\"\nHKCU\\Software\\Classes\\SystemFileAssociations\\.ps1\\Shell\\TourneRunModify\\shell\\2admin | MUIVerb = sz:Run as administrator\nHKCU\\Software\\Classes\\SystemFileAssociations\\.ps1\\Shell\\TourneRunModify\\shell\\2admin | HasLUAShield = sz:\nHKCU\\Software\\Classes\\SystemFileAssociations\\.ps1\\Shell\\TourneRunModify\\shell\\2admin\\command | @ = sz:powershell.exe -NoProfile -WindowStyle Hidden -Command \"Start-Process powershell.exe -Verb RunAs -ArgumentList '-NoProfile','-ExecutionPolicy','Bypass','-NoExit','-File','\\\"%1\\\"'\"\nHKCU\\Software\\Classes\\SystemFileAssociations\\.ps1\\Shell\\TourneRunModify\\shell\\3ise | MUIVerb = sz:Edit with PowerShell ISE\nHKCU\\Software\\Classes\\SystemFileAssociations\\.ps1\\Shell\\TourneRunModify\\shell\\3ise\\command | @ = sz:powershell_ise.exe \"%1\"\nHKCU\\Software\\Classes\\SystemFileAssociations\\.ps1\\Shell\\TourneRunModify\\shell\\4notepad | MUIVerb = sz:Edit in Notepad\nHKCU\\Software\\Classes\\SystemFileAssociations\\.ps1\\Shell\\TourneRunModify\\shell\\4notepad\\command | @ = sz:notepad.exe \"%1\"\npwsh {@menuFiles.ps1MenuPwsh}\037shift\036HKCU\\Software\\Classes\\SystemFileAssociations\\.ps1\\Shell\\TourneRunModify | -\nHKCU\\Software\\Classes\\SystemFileAssociations\\.ps1\\Shell\\TourneRunModify | MUIVerb = sz:Run or modify\nHKCU\\Software\\Classes\\SystemFileAssociations\\.ps1\\Shell\\TourneRunModify | Extended = sz:\nHKCU\\Software\\Classes\\SystemFileAssociations\\.ps1\\Shell\\TourneRunModify | SubCommands = sz:\nHKCU\\Software\\Classes\\SystemFileAssociations\\.ps1\\Shell\\TourneRunModify | Icon = sz:powershell.exe\nHKCU\\Software\\Classes\\SystemFileAssociations\\.ps1\\Shell\\TourneRunModify\\shell\\1run | MUIVerb = sz:Run with PowerShell\nHKCU\\Software\\Classes\\SystemFileAssociations\\.ps1\\Shell\\TourneRunModify\\shell\\1run\\command | @ = sz:powershell.exe -NoProfile -ExecutionPolicy Bypass -NoExit -File \"%1\"\nHKCU\\Software\\Classes\\SystemFileAssociations\\.ps1\\Shell\\TourneRunModify\\shell\\2admin | MUIVerb = sz:Run as administrator\nHKCU\\Software\\Classes\\SystemFileAssociations\\.ps1\\Shell\\TourneRunModify\\shell\\2admin | HasLUAShield = sz:\nHKCU\\Software\\Classes\\SystemFileAssociations\\.ps1\\Shell\\TourneRunModify\\shell\\2admin\\command | @ = sz:powershell.exe -NoProfile -WindowStyle Hidden -Command \"Start-Process powershell.exe -Verb RunAs -ArgumentList '-NoProfile','-ExecutionPolicy','Bypass','-NoExit','-File','\\\"%1\\\"'\"\nHKCU\\Software\\Classes\\SystemFileAssociations\\.ps1\\Shell\\TourneRunModify\\shell\\3ise | MUIVerb = sz:Edit with PowerShell ISE\nHKCU\\Software\\Classes\\SystemFileAssociations\\.ps1\\Shell\\TourneRunModify\\shell\\3ise\\command | @ = sz:powershell_ise.exe \"%1\"\nHKCU\\Software\\Classes\\SystemFileAssociations\\.ps1\\Shell\\TourneRunModify\\shell\\4notepad | MUIVerb = sz:Edit in Notepad\nHKCU\\Software\\Classes\\SystemFileAssociations\\.ps1\\Shell\\TourneRunModify\\shell\\4notepad\\command | @ = sz:notepad.exe \"%1\"\npwsh {@menuFiles.ps1MenuPwsh}",
     L"",
     L"menuFiles.ps1MenuPwsh"},
    {L"menuFiles.ps1MenuPwsh", L"PowerShell scripts: PowerShell 7 in that menu", 't', 0, false, false, nullptr,
     L"",
     L"",
     L""},
    {L"menuFiles.runAsVbs", L"VBScript files: Run as administrator", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\VBSFile\\Shell\\runas | HasLUAShield = sz:\nHKCU\\Software\\Classes\\VBSFile\\Shell\\runas\\command | @ = sz:WScript.exe \"%1\" %*",
     L"HKCU\\Software\\Classes\\VBSFile\\Shell\\runas | -",
     L""},
    {L"menuFiles.openWithBatch", L"Batch files: Open with", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\SystemFileAssociations\\.bat\\shell\\TourneOpenWith | MUIVerb = sz:Open with...\nHKCU\\Software\\Classes\\SystemFileAssociations\\.bat\\shell\\TourneOpenWith\\command | @ = expand:\"%SystemRoot%\\System32\\rundll32.exe\" \"%SystemRoot%\\System32\\shell32.dll\",OpenAs_RunDLL %1\nHKCU\\Software\\Classes\\SystemFileAssociations\\.cmd\\shell\\TourneOpenWith | MUIVerb = sz:Open with...\nHKCU\\Software\\Classes\\SystemFileAssociations\\.cmd\\shell\\TourneOpenWith\\command | @ = expand:\"%SystemRoot%\\System32\\rundll32.exe\" \"%SystemRoot%\\System32\\shell32.dll\",OpenAs_RunDLL %1",
     L"HKCU\\Software\\Classes\\SystemFileAssociations\\.bat\\shell\\TourneOpenWith | -\nHKCU\\Software\\Classes\\SystemFileAssociations\\.cmd\\shell\\TourneOpenWith | -\nHKCU\\Software\\Classes\\batfile\\shell\\TourneOpenWith | -\nHKCU\\Software\\Classes\\cmdfile\\shell\\TourneOpenWith | -",
     L""},
    {L"menuFiles.runAsMsi", L"Installers (.msi): Install as administrator", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\Msi.Package\\shell\\runas | MUIVerb = sz:Install as administrator\nHKCU\\Software\\Classes\\Msi.Package\\shell\\runas | HasLUAShield = sz:\nHKCU\\Software\\Classes\\Msi.Package\\shell\\runas\\command | @ = sz:msiexec.exe /i \"%1\"",
     L"HKCU\\Software\\Classes\\Msi.Package\\shell\\runas | -",
     L""},
    {L"menuFiles.msiExtract", L"Installers (.msi): Extract the contents", 't', 0, true, false, nullptr,
     L"HKCU\\Software\\Classes\\Msi.Package\\shell\\Extract | MUIVerb = sz:Extract the contents\nHKCU\\Software\\Classes\\Msi.Package\\shell\\Extract\\command | @ = sz:msiexec.exe /a \"%1\" /qb TARGETDIR=\"%1 Contents\"",
     L"HKCU\\Software\\Classes\\Msi.Package\\shell\\Extract | -\nHKLM\\SOFTWARE\\Classes\\Msi.Package\\shell\\Extract | -",
     L""},
    {L"menuFiles.cabInstall", L"Update packages (.cab): Install", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\CABFolder\\Shell\\TourneInstall | MUIVerb = sz:Install this update\nHKCU\\Software\\Classes\\CABFolder\\Shell\\TourneInstall | HasLUAShield = sz:\nHKCU\\Software\\Classes\\CABFolder\\Shell\\TourneInstall\\command | @ = sz:powershell.exe -NoProfile -WindowStyle Hidden -Command \"Start-Process dism.exe -Verb RunAs -ArgumentList '/Online','/Add-Package','/PackagePath:\\\"%1\\\"'\"",
     L"HKCU\\Software\\Classes\\CABFolder\\Shell\\TourneInstall | -",
     L""},
    {L"menuFiles.bitlockerLock", L"Drives: Lock with BitLocker", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\Drive\\shell\\TourneLockBde | MUIVerb = sz:Lock drive\nHKCU\\Software\\Classes\\Drive\\shell\\TourneLockBde | AppliesTo = sz:System.Volume.BitLockerProtection:=1\nHKCU\\Software\\Classes\\Drive\\shell\\TourneLockBde | HasLUAShield = sz:\nHKCU\\Software\\Classes\\Drive\\shell\\TourneLockBde\\command | @ = sz:powershell.exe -NoProfile -WindowStyle Hidden -Command \"Start-Process manage-bde.exe -Verb RunAs -ArgumentList '-lock','%1','-ForceDismount'\"",
     L"HKCU\\Software\\Classes\\Drive\\shell\\TourneLockBde | -",
     L""},
    {L"menuFiles.advancedSystemProps", L"This PC: Advanced system properties", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\CLSID\\{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\shell\\advproperties | MUIVerb = sz:Advanced system properties\nHKCU\\Software\\Classes\\CLSID\\{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\shell\\advproperties | Icon = sz:SystemPropertiesAdvanced.exe\nHKCU\\Software\\Classes\\CLSID\\{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\shell\\advproperties\\command | @ = sz:SystemPropertiesAdvanced.exe",
     L"HKCU\\Software\\Classes\\CLSID\\{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\shell\\advproperties | -",
     L""},
    {L"menuFiles.classicSystemProps", L"This PC: System properties (classic)", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\CLSID\\{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\shell\\TourneSysProps | MUIVerb = sz:System properties (classic)\nHKCU\\Software\\Classes\\CLSID\\{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\shell\\TourneSysProps | Icon = sz:sysdm.cpl\nHKCU\\Software\\Classes\\CLSID\\{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\shell\\TourneSysProps\\command | @ = sz:rundll32.exe shell32.dll,Control_RunDLL sysdm.cpl",
     L"HKCU\\Software\\Classes\\CLSID\\{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\shell\\TourneSysProps | -",
     L""},
    {L"menuDesktop.settingsMenu", L"Settings", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSettings | MUIVerb = sz:Settings\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSettings | SubCommands = sz:\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSettings | Icon = sz:shell32.dll,-16826\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSettings\\shell\\01home | MUIVerb = sz:Settings\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSettings\\shell\\01home\\command | @ = sz:explorer.exe ms-settings:\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSettings\\shell\\02system | MUIVerb = sz:System\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSettings\\shell\\02system\\command | @ = sz:explorer.exe ms-settings:display\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSettings\\shell\\03devices | MUIVerb = sz:Bluetooth and devices\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSettings\\shell\\03devices\\command | @ = sz:explorer.exe ms-settings:bluetooth\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSettings\\shell\\04network | MUIVerb = sz:Network and internet\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSettings\\shell\\04network\\command | @ = sz:explorer.exe ms-settings:network\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSettings\\shell\\05personal | MUIVerb = sz:Personalization\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSettings\\shell\\05personal\\command | @ = sz:explorer.exe ms-settings:personalization\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSettings\\shell\\06apps | MUIVerb = sz:Apps\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSettings\\shell\\06apps\\command | @ = sz:explorer.exe ms-settings:appsfeatures\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSettings\\shell\\07accounts | MUIVerb = sz:Accounts\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSettings\\shell\\07accounts\\command | @ = sz:explorer.exe ms-settings:yourinfo\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSettings\\shell\\08time | MUIVerb = sz:Time and language\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSettings\\shell\\08time\\command | @ = sz:explorer.exe ms-settings:dateandtime\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSettings\\shell\\09gaming | MUIVerb = sz:Gaming\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSettings\\shell\\09gaming\\command | @ = sz:explorer.exe ms-settings:gaming-gamebar\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSettings\\shell\\10access | MUIVerb = sz:Accessibility\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSettings\\shell\\10access\\command | @ = sz:explorer.exe ms-settings:easeofaccess\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSettings\\shell\\11privacy | MUIVerb = sz:Privacy and security\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSettings\\shell\\11privacy\\command | @ = sz:explorer.exe ms-settings:privacy\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSettings\\shell\\12update | MUIVerb = sz:Windows Update\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSettings\\shell\\12update\\command | @ = sz:explorer.exe ms-settings:windowsupdate",
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSettings | -",
     L""},
    {L"menuDesktop.controlPanelMenu", L"Control Panel", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneControlPanel | MUIVerb = sz:Control Panel\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneControlPanel | SubCommands = sz:\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneControlPanel | Icon = sz:imageres.dll,-27\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneControlPanel\\shell\\1category | MUIVerb = sz:Category view\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneControlPanel\\shell\\1category\\command | @ = sz:explorer.exe shell:::{26EE0668-A00A-44D7-9371-BEB064C98683}\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneControlPanel\\shell\\2icons | MUIVerb = sz:All items\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneControlPanel\\shell\\2icons\\command | @ = sz:explorer.exe shell:::{21EC2020-3AEA-1069-A2DD-08002B30309D}\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneControlPanel\\shell\\3all | MUIVerb = sz:All tasks\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneControlPanel\\shell\\3all\\command | @ = sz:explorer.exe shell:::{ED7BA470-8E54-465E-825C-99712043E01C}",
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneControlPanel | -",
     L""},
    {L"menuDesktop.personalizeMenu", L"Personalization", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TournePersonalize | MUIVerb = sz:Appearance\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TournePersonalize | SubCommands = sz:\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TournePersonalize | Icon = sz:themecpl.dll,-1\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TournePersonalize\\shell\\1background | MUIVerb = sz:Desktop background\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TournePersonalize\\shell\\1background\\command | @ = sz:explorer.exe shell:::{ED834ED6-4B5A-4bfe-8F11-A626DCB6A921} -Microsoft.Personalization\\pageWallpaper\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TournePersonalize\\shell\\2colour | MUIVerb = sz:Colour and appearance\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TournePersonalize\\shell\\2colour\\command | @ = sz:explorer.exe shell:::{ED834ED6-4B5A-4bfe-8F11-A626DCB6A921} -Microsoft.Personalization\\pageColorization\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TournePersonalize\\shell\\3icons | MUIVerb = sz:Desktop icons\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TournePersonalize\\shell\\3icons\\command | @ = sz:rundll32.exe shell32.dll,Control_RunDLL desk.cpl,,0\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TournePersonalize\\shell\\4saver | MUIVerb = sz:Screen saver\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TournePersonalize\\shell\\4saver\\command | @ = sz:rundll32.exe shell32.dll,Control_RunDLL desk.cpl,screensaver,@screensaver\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TournePersonalize\\shell\\5sounds | MUIVerb = sz:Sounds\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TournePersonalize\\shell\\5sounds\\command | @ = sz:rundll32.exe shell32.dll,Control_RunDLL mmsys.cpl,,2\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TournePersonalize\\shell\\6pointers | MUIVerb = sz:Mouse pointers\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TournePersonalize\\shell\\6pointers\\command | @ = sz:rundll32.exe shell32.dll,Control_RunDLL main.cpl,,1",
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TournePersonalize | -",
     L""},
    {L"menuDesktop.taskManagerMenu", L"Task Manager", 't', 0, true, false, nullptr,
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\taskmgr | MUIVerb = sz:Task Manager\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\taskmgr | Icon = sz:taskmgr.exe\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\taskmgr\\command | @ = sz:taskmgr.exe",
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\taskmgr | -\nHKLM\\SOFTWARE\\Classes\\DesktopBackground\\Shell\\taskmgr | -",
     L""},
    {L"menuDesktop.windowsToolsMenu", L"Windows Tools", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneWindowsTools | MUIVerb = sz:Windows Tools\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneWindowsTools | Icon = sz:imageres.dll,-114\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneWindowsTools\\command | @ = sz:explorer.exe shell:::{D20EA4E1-3957-11d2-A40B-0C5020524153}",
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneWindowsTools | -",
     L""},
    {L"menuDesktop.envVarsMenu", L"Environment variables", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneEnvVars | MUIVerb = sz:Environment variables\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneEnvVars\\command | @ = sz:rundll32.exe sysdm.cpl,EditEnvironmentVariables",
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneEnvVars | -",
     L""},
    {L"menuDesktop.gpeditMenu", L"Group Policy", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneGpedit | MUIVerb = sz:Group Policy\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneGpedit | Icon = sz:gpedit.dll\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneGpedit\\command | @ = sz:mmc.exe gpedit.msc",
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneGpedit | -",
     L""},
    {L"menuDesktop.defaultAppsMenu", L"Default apps", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneDefaultApps | MUIVerb = sz:Default apps\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneDefaultApps\\command | @ = sz:explorer.exe ms-settings:defaultapps",
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneDefaultApps | -",
     L""},
    {L"menuDesktop.accountsMenu", L"User accounts", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneAccounts | MUIVerb = sz:User accounts\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneAccounts | SubCommands = sz:\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneAccounts | Icon = sz:imageres.dll,-88\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneAccounts\\shell\\1info | MUIVerb = sz:Your info\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneAccounts\\shell\\1info\\command | @ = sz:explorer.exe ms-settings:yourinfo\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneAccounts\\shell\\2email | MUIVerb = sz:Email and accounts\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneAccounts\\shell\\2email\\command | @ = sz:explorer.exe ms-settings:emailandaccounts\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneAccounts\\shell\\3signin | MUIVerb = sz:Sign-in options\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneAccounts\\shell\\3signin\\command | @ = sz:explorer.exe ms-settings:signinoptions\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneAccounts\\shell\\4work | MUIVerb = sz:Access work or school\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneAccounts\\shell\\4work\\command | @ = sz:explorer.exe ms-settings:workplace\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneAccounts\\shell\\5others | MUIVerb = sz:Other users\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneAccounts\\shell\\5others\\command | @ = sz:explorer.exe ms-settings:otherusers\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneAccounts\\shell\\6netplwiz | MUIVerb = sz:User accounts (classic)\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneAccounts\\shell\\6netplwiz\\command | @ = sz:netplwiz.exe",
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneAccounts | -",
     L""},
    {L"menuDesktop.securityMenu", L"Windows Security", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSecurity | MUIVerb = sz:Windows Security\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSecurity | SubCommands = sz:\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSecurity | Icon = sz:%ProgramFiles%\\Windows Defender\\EppManifest.dll,-101\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSecurity\\shell\\1home | MUIVerb = sz:Security at a glance\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSecurity\\shell\\1home\\command | @ = sz:explorer.exe windowsdefender:\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSecurity\\shell\\2virus | MUIVerb = sz:Virus and threat protection\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSecurity\\shell\\2virus\\command | @ = sz:explorer.exe windowsdefender://threat\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSecurity\\shell\\3firewall | MUIVerb = sz:Firewall and network protection\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSecurity\\shell\\3firewall\\command | @ = sz:explorer.exe windowsdefender://network\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSecurity\\shell\\4apps | MUIVerb = sz:App and browser control\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSecurity\\shell\\4apps\\command | @ = sz:explorer.exe windowsdefender://appbrowser\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSecurity\\shell\\5device | MUIVerb = sz:Device security\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSecurity\\shell\\5device\\command | @ = sz:explorer.exe windowsdefender://devicesecurity\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSecurity\\shell\\6perf | MUIVerb = sz:Device performance and health\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSecurity\\shell\\6perf\\command | @ = sz:explorer.exe windowsdefender://perfcenter",
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSecurity | -",
     L""},
    {L"menuDesktop.firewallMenu", L"Firewall", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneFirewall | MUIVerb = sz:Firewall\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneFirewall | SubCommands = sz:\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneFirewall | Icon = sz:FirewallControlPanel.dll,-1\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneFirewall\\shell\\1basic | MUIVerb = sz:Windows Defender Firewall\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneFirewall\\shell\\1basic\\command | @ = sz:control.exe firewall.cpl\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneFirewall\\shell\\2advanced | MUIVerb = sz:Advanced security\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneFirewall\\shell\\2advanced\\command | @ = sz:mmc.exe wf.msc\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneFirewall\\shell\\3apps | MUIVerb = sz:Allowed apps\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneFirewall\\shell\\3apps\\command | @ = sz:explorer.exe shell:::{4026492F-2F69-46B8-B9BF-5654FC07E423} -Microsoft.WindowsFirewall\\pageConfigureApps",
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneFirewall | -",
     L""},
    {L"menuDesktop.updateMenu", L"Windows Update", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneUpdate | MUIVerb = sz:Windows Update\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneUpdate | SubCommands = sz:\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneUpdate | Icon = sz:shell32.dll,-47\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneUpdate\\shell\\1open | MUIVerb = sz:Windows Update\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneUpdate\\shell\\1open\\command | @ = sz:explorer.exe ms-settings:windowsupdate\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneUpdate\\shell\\2check | MUIVerb = sz:Check for updates\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneUpdate\\shell\\2check\\command | @ = sz:explorer.exe ms-settings:windowsupdate-action\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneUpdate\\shell\\3history | MUIVerb = sz:Update history\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneUpdate\\shell\\3history\\command | @ = sz:explorer.exe ms-settings:windowsupdate-history\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneUpdate\\shell\\4options | MUIVerb = sz:Advanced options\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneUpdate\\shell\\4options\\command | @ = sz:explorer.exe ms-settings:windowsupdate-options",
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneUpdate | -",
     L""},
    {L"menuDesktop.appModeMenu", L"Light or dark mode for apps", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneAppMode | MUIVerb = sz:App mode\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneAppMode | SubCommands = sz:\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneAppMode | Icon = sz:themecpl.dll,-1\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneAppMode\\shell\\1light | MUIVerb = sz:Light\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneAppMode\\shell\\1light\\command | @ = sz:reg.exe add HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize /v AppsUseLightTheme /t REG_DWORD /d 1 /f\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneAppMode\\shell\\2dark | MUIVerb = sz:Dark\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneAppMode\\shell\\2dark\\command | @ = sz:reg.exe add HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize /v AppsUseLightTheme /t REG_DWORD /d 0 /f",
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneAppMode | -",
     L""},
    {L"menuDesktop.windowsModeMenu", L"Light or dark mode for Windows", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneWindowsMode | MUIVerb = sz:Windows mode\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneWindowsMode | SubCommands = sz:\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneWindowsMode | Icon = sz:themecpl.dll,-1\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneWindowsMode\\shell\\1light | MUIVerb = sz:Light\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneWindowsMode\\shell\\1light\\command | @ = sz:reg.exe add HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize /v SystemUsesLightTheme /t REG_DWORD /d 1 /f\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneWindowsMode\\shell\\2dark | MUIVerb = sz:Dark\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneWindowsMode\\shell\\2dark\\command | @ = sz:reg.exe add HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize /v SystemUsesLightTheme /t REG_DWORD /d 0 /f",
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneWindowsMode | -",
     L""},
    {L"menuDesktop.wallpaperLocation", L"Open the wallpaper's location", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneWallpaper | MUIVerb = sz:Open wallpaper location\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneWallpaper | Icon = sz:imageres.dll,-5346\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneWallpaper\\command | @ = sz:powershell.exe -NoProfile -WindowStyle Hidden -Command \"$p = (Get-ItemProperty 'HKCU:\\Control Panel\\Desktop').WallPaper; if ($p -and (Test-Path -LiteralPath $p)) { explorer.exe /select,$p }\"",
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneWallpaper | -",
     L""},
    {L"menuDesktop.bluetoothMenu", L"Bluetooth", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneBluetooth | MUIVerb = sz:Bluetooth\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneBluetooth | SubCommands = sz:\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneBluetooth | Icon = sz:bthprops.cpl,-1\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneBluetooth\\shell\\1devices | MUIVerb = sz:Bluetooth devices\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneBluetooth\\shell\\1devices\\command | @ = sz:explorer.exe ms-settings:bluetooth\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneBluetooth\\shell\\2send | MUIVerb = sz:Send or receive files\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneBluetooth\\shell\\2send\\command | @ = sz:fsquirt.exe\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneBluetooth\\shell\\3options | MUIVerb = sz:More Bluetooth options\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneBluetooth\\shell\\3options\\command | @ = sz:rundll32.exe shell32.dll,Control_RunDLL bthprops.cpl,,1\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneBluetooth\\shell\\4com | MUIVerb = sz:COM ports\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneBluetooth\\shell\\4com\\command | @ = sz:rundll32.exe shell32.dll,Control_RunDLL bthprops.cpl,,2\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneBluetooth\\shell\\5hw | MUIVerb = sz:Hardware\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneBluetooth\\shell\\5hw\\command | @ = sz:rundll32.exe shell32.dll,Control_RunDLL bthprops.cpl,,3",
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneBluetooth | -",
     L""},
    {L"menuDesktop.networkLocation", L"Network location", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneNetLocation | MUIVerb = sz:Network location\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneNetLocation | SubCommands = sz:\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneNetLocation | HasLUAShield = sz:\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneNetLocation\\shell\\1private | MUIVerb = sz:Private\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneNetLocation\\shell\\1private\\command | @ = sz:powershell.exe -NoProfile -WindowStyle Hidden -Command \"Start-Process powershell.exe -Verb RunAs -WindowStyle Hidden -ArgumentList '-NoProfile','-Command','Get-NetConnectionProfile | Set-NetConnectionProfile -NetworkCategory Private'\"\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneNetLocation\\shell\\2public | MUIVerb = sz:Public\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneNetLocation\\shell\\2public\\command | @ = sz:powershell.exe -NoProfile -WindowStyle Hidden -Command \"Start-Process powershell.exe -Verb RunAs -WindowStyle Hidden -ArgumentList '-NoProfile','-Command','Get-NetConnectionProfile | Set-NetConnectionProfile -NetworkCategory Public'\"",
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneNetLocation | -",
     L""},
    {L"menuDesktop.projectMenu", L"Project display", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneProject | MUIVerb = sz:Project\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneProject | SubCommands = sz:\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneProject | Icon = sz:DisplaySwitch.exe\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneProject\\shell\\1internal | MUIVerb = sz:PC screen only\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneProject\\shell\\1internal\\command | @ = sz:DisplaySwitch.exe /internal\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneProject\\shell\\2clone | MUIVerb = sz:Duplicate\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneProject\\shell\\2clone\\command | @ = sz:DisplaySwitch.exe /clone\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneProject\\shell\\3extend | MUIVerb = sz:Extend\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneProject\\shell\\3extend\\command | @ = sz:DisplaySwitch.exe /extend\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneProject\\shell\\4external | MUIVerb = sz:Second screen only\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneProject\\shell\\4external\\command | @ = sz:DisplaySwitch.exe /external",
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneProject | -",
     L""},
    {L"menuDesktop.clipboardMenu", L"Clipboard", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneClipboard | MUIVerb = sz:Clipboard\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneClipboard | SubCommands = sz:\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneClipboard | Icon = sz:shell32.dll,-16763\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneClipboard\\shell\\1settings | MUIVerb = sz:Clipboard history settings\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneClipboard\\shell\\1settings\\command | @ = sz:explorer.exe ms-settings:clipboard\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneClipboard\\shell\\2clear | MUIVerb = sz:Clear the clipboard\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneClipboard\\shell\\2clear\\command | @ = sz:powershell.exe -NoProfile -WindowStyle Hidden -Command \"Set-Clipboard -Value $null; [Windows.ApplicationModel.DataTransfer.Clipboard,Windows.ApplicationModel.DataTransfer,ContentType=WindowsRuntime]::ClearHistory() | Out-Null\"",
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneClipboard | -",
     L""},
    {L"menuDesktop.screenSnipMenu", L"Screen snip", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneScreenSnip | MUIVerb = sz:Screen snip\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneScreenSnip | Icon = sz:SnippingTool.exe\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneScreenSnip\\command | @ = sz:explorer.exe ms-screenclip:",
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneScreenSnip | -",
     L""},
    {L"menuDesktop.snippingToolMenu", L"Snipping Tool", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSnippingTool | MUIVerb = sz:Snipping Tool\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSnippingTool | Icon = sz:SnippingTool.exe\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSnippingTool\\command | @ = sz:explorer.exe ms-screensketch:",
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSnippingTool | -",
     L""},
    {L"menuDesktop.copilotMenu", L"Copilot", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneCopilot | MUIVerb = sz:Copilot\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneCopilot\\command | @ = sz:explorer.exe ms-copilot:",
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneCopilot | -",
     L""},
    {L"menuDesktop.speechMenu", L"Speech recognition", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSpeech | MUIVerb = sz:Speech recognition\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSpeech | Icon = sz:%windir%\\Speech\\Common\\sapisvr.exe\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSpeech\\command | @ = sz:%windir%\\Speech\\Common\\sapisvr.exe -SpeechUX",
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSpeech | -",
     L""},
    {L"menuDesktop.killHung", L"Close programs that aren't responding", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneKillHung | MUIVerb = sz:Close programs that aren't responding\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneKillHung | Icon = sz:taskmgr.exe,-30651\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneKillHung\\command | @ = sz:taskkill.exe /f /fi \"status eq NOT RESPONDING\"",
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneKillHung | -",
     L""},
    {L"menuDesktop.restartExplorerMenu", L"Restart Explorer", 't', 0, true, false, nullptr,
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\RestartExplorer | MUIVerb = sz:Restart Explorer\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\RestartExplorer | SubCommands = sz:\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\RestartExplorer | Icon = sz:explorer.exe\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\RestartExplorer | Position = sz:Bottom\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\RestartExplorer\\shell\\1now | MUIVerb = sz:Restart now\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\RestartExplorer\\shell\\1now\\command | @ = sz:cmd.exe /c taskkill.exe /f /im explorer.exe & start explorer.exe\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\RestartExplorer\\shell\\2pause | MUIVerb = sz:Stop, then restart with a key press\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\RestartExplorer\\shell\\2pause\\command | @ = sz:cmd.exe /c taskkill.exe /f /im explorer.exe & echo Explorer stopped. Press any key to start it again. & pause >nul & start explorer.exe",
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\RestartExplorer | -\nHKLM\\SOFTWARE\\Classes\\DesktopBackground\\Shell\\RestartExplorer | -",
     L""},
    {L"menuDesktop.restartStartMenu", L"Restart Start menu", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneRestartStart | MUIVerb = sz:Restart Start menu\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneRestartStart\\command | @ = sz:taskkill.exe /f /im StartMenuExperienceHost.exe",
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneRestartStart | -",
     L""},
    {L"menuDesktop.powerPlanMenu", L"Switch power plan", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TournePowerPlan | MUIVerb = sz:Power plan\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TournePowerPlan | SubCommands = sz:\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TournePowerPlan | Icon = sz:powercpl.dll,-1\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TournePowerPlan\\shell\\1saver | MUIVerb = sz:Power saver\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TournePowerPlan\\shell\\1saver\\command | @ = sz:powercfg.exe /setactive a1841308-3541-4fab-bc81-f71556f20b4a\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TournePowerPlan\\shell\\2balanced | MUIVerb = sz:Balanced\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TournePowerPlan\\shell\\2balanced\\command | @ = sz:powercfg.exe /setactive 381b4222-f694-41f0-9685-ff5bb260df2e\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TournePowerPlan\\shell\\3high | MUIVerb = sz:High performance\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TournePowerPlan\\shell\\3high\\command | @ = sz:powercfg.exe /setactive 8c5e7fda-e8bf-4a96-9a85-a6e23a8c635c\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TournePowerPlan\\shell\\4ultimate | MUIVerb = sz:Ultimate performance\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TournePowerPlan\\shell\\4ultimate\\command | @ = sz:powercfg.exe /setactive e9a42b02-d5df-448d-aa00-03f14749eb61",
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TournePowerPlan | -",
     L""},
    {L"menuDesktop.powerMenu", L"Power options", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TournePower | MUIVerb = sz:Power options\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TournePower | SubCommands = sz:\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TournePower | Icon = sz:powercpl.dll,-1\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TournePower\\shell\\1options | MUIVerb = sz:Power options\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TournePower\\shell\\1options\\command | @ = sz:control.exe powercfg.cpl\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TournePower\\shell\\2plan | MUIVerb = sz:Edit current plan\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TournePower\\shell\\2plan\\command | @ = sz:control.exe powercfg.cpl,,3\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TournePower\\shell\\3buttons | MUIVerb = sz:What the power buttons do\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TournePower\\shell\\3buttons\\command | @ = sz:explorer.exe shell:::{025A5937-A6BE-4686-A844-36FE4BEC8B6D} -Microsoft.PowerOptions\\pageGlobalSettings\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TournePower\\shell\\4settings | MUIVerb = sz:Power settings\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TournePower\\shell\\4settings\\command | @ = sz:explorer.exe ms-settings:powersleep",
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TournePower | -",
     L""},
    {L"menuDesktop.shutdownMenu", L"Shut down", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneShutdown | MUIVerb = sz:Shut down\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneShutdown | SubCommands = sz:\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneShutdown | Icon = sz:shell32.dll,-28\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneShutdown | Position = sz:Bottom\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneShutdown\\shell\\1off | MUIVerb = sz:Shut down\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneShutdown\\shell\\1off\\command | @ = sz:shutdown.exe /s /t 0\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneShutdown\\shell\\2offwarn | MUIVerb = sz:Shut down in 30 seconds\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneShutdown\\shell\\2offwarn\\command | @ = sz:shutdown.exe /s /t 30\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneShutdown\\shell\\3restart | MUIVerb = sz:Restart\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneShutdown\\shell\\3restart\\command | @ = sz:shutdown.exe /r /t 0\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneShutdown\\shell\\4restartwarn | MUIVerb = sz:Restart in 30 seconds\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneShutdown\\shell\\4restartwarn\\command | @ = sz:shutdown.exe /r /t 30\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneShutdown\\shell\\5hybrid | MUIVerb = sz:Shut down (fast startup)\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneShutdown\\shell\\5hybrid\\command | @ = sz:shutdown.exe /s /hybrid /t 0\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneShutdown\\shell\\6signout | MUIVerb = sz:Sign out\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneShutdown\\shell\\6signout\\command | @ = sz:shutdown.exe /l\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneShutdown\\shell\\7lock | MUIVerb = sz:Lock\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneShutdown\\shell\\7lock\\command | @ = sz:rundll32.exe user32.dll,LockWorkStation\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneShutdown\\shell\\8cancel | MUIVerb = sz:Cancel a timed shutdown\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneShutdown\\shell\\8cancel\\command | @ = sz:shutdown.exe /a",
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneShutdown | -",
     L""},
    {L"menuDesktop.safeModeMenu", L"Safe Mode", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSafeMode | MUIVerb = sz:Safe Mode\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSafeMode | SubCommands = sz:\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSafeMode | HasLUAShield = sz:\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSafeMode | Position = sz:Bottom\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSafeMode\\shell\\1min | MUIVerb = sz:Restart in Safe Mode\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSafeMode\\shell\\1min\\command | @ = sz:powershell.exe -NoProfile -WindowStyle Hidden -Command \"Start-Process cmd.exe -Verb RunAs -ArgumentList '/c','bcdedit /set {current} safeboot minimal && shutdown /r /t 0'\"\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSafeMode\\shell\\2net | MUIVerb = sz:Restart in Safe Mode with networking\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSafeMode\\shell\\2net\\command | @ = sz:powershell.exe -NoProfile -WindowStyle Hidden -Command \"Start-Process cmd.exe -Verb RunAs -ArgumentList '/c','bcdedit /set {current} safeboot network && shutdown /r /t 0'\"\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSafeMode\\shell\\3cmd | MUIVerb = sz:Restart in Safe Mode with command prompt\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSafeMode\\shell\\3cmd\\command | @ = sz:powershell.exe -NoProfile -WindowStyle Hidden -Command \"Start-Process cmd.exe -Verb RunAs -ArgumentList '/c','bcdedit /set {current} safeboot minimal && bcdedit /set {current} safebootalternateshell yes && shutdown /r /t 0'\"\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSafeMode\\shell\\4normal | MUIVerb = sz:Restart normally\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSafeMode\\shell\\4normal\\command | @ = sz:powershell.exe -NoProfile -WindowStyle Hidden -Command \"Start-Process cmd.exe -Verb RunAs -ArgumentList '/c','bcdedit /deletevalue {current} safeboot & bcdedit /deletevalue {current} safebootalternateshell & shutdown /r /t 0'\"",
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSafeMode | -",
     L""},
    {L"menuDesktop.advancedStartup", L"Advanced startup", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneAdvancedStartup | MUIVerb = sz:Advanced startup\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneAdvancedStartup | Icon = sz:bootux.dll,-1032\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneAdvancedStartup | Position = sz:Bottom\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneAdvancedStartup\\command | @ = sz:shutdown.exe /r /o /f /t 0",
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneAdvancedStartup | -",
     L""},
    {L"menuDesktop.firmwareMenu", L"Restart to firmware (UEFI)", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneFirmware | MUIVerb = sz:Restart to firmware settings\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneFirmware | HasLUAShield = sz:\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneFirmware | Position = sz:Bottom\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneFirmware\\command | @ = sz:powershell.exe -NoProfile -WindowStyle Hidden -Command \"Start-Process shutdown.exe -Verb RunAs -ArgumentList '/r','/fw','/t','0'\"",
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneFirmware | -",
     L""},
    {L"menuDesktop.restorePointMenu", L"Create restore point", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneRestorePoint | MUIVerb = sz:Create restore point\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneRestorePoint | Icon = sz:rstrui.exe\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneRestorePoint | HasLUAShield = sz:\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneRestorePoint\\command | @ = sz:powershell.exe -NoProfile -WindowStyle Hidden -Command \"Start-Process powershell.exe -Verb RunAs -ArgumentList '-NoProfile','-Command','Checkpoint-Computer -Description (Get-Date -Format g) -RestorePointType MODIFY_SETTINGS; pause'\"",
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneRestorePoint | -",
     L""},
    {L"menuDesktop.sfcMenu", L"Check system files (sfc)", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSfc | MUIVerb = sz:Check system files\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSfc | HasLUAShield = sz:\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSfc\\command | @ = sz:powershell.exe -NoProfile -WindowStyle Hidden -Command \"Start-Process cmd.exe -Verb RunAs -ArgumentList '/k','sfc /scannow'\"",
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneSfc | -",
     L""},
    {L"menuDesktop.repairImage", L"Repair Windows image", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneDism | MUIVerb = sz:Repair Windows image\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneDism | SubCommands = sz:\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneDism | HasLUAShield = sz:\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneDism\\shell\\1check | MUIVerb = sz:Check health\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneDism\\shell\\1check\\command | @ = sz:powershell.exe -NoProfile -WindowStyle Hidden -Command \"Start-Process cmd.exe -Verb RunAs -ArgumentList '/k','DISM /Online /Cleanup-Image /CheckHealth'\"\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneDism\\shell\\2scan | MUIVerb = sz:Scan health\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneDism\\shell\\2scan\\command | @ = sz:powershell.exe -NoProfile -WindowStyle Hidden -Command \"Start-Process cmd.exe -Verb RunAs -ArgumentList '/k','DISM /Online /Cleanup-Image /ScanHealth'\"\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneDism\\shell\\3repair | MUIVerb = sz:Repair\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneDism\\shell\\3repair\\command | @ = sz:powershell.exe -NoProfile -WindowStyle Hidden -Command \"Start-Process cmd.exe -Verb RunAs -ArgumentList '/k','DISM /Online /Cleanup-Image /RestoreHealth'\"",
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneDism | -",
     L""},
    {L"menuDesktop.compactOs", L"Compact OS", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneCompactOs | MUIVerb = sz:Compact OS\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneCompactOs | SubCommands = sz:\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneCompactOs | HasLUAShield = sz:\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneCompactOs\\shell\\1query | MUIVerb = sz:Show state\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneCompactOs\\shell\\1query\\command | @ = sz:powershell.exe -NoProfile -WindowStyle Hidden -Command \"Start-Process cmd.exe -Verb RunAs -ArgumentList '/k','compact /compactos:query'\"\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneCompactOs\\shell\\2on | MUIVerb = sz:Compress Windows\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneCompactOs\\shell\\2on\\command | @ = sz:powershell.exe -NoProfile -WindowStyle Hidden -Command \"Start-Process cmd.exe -Verb RunAs -ArgumentList '/k','compact /compactos:always'\"\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneCompactOs\\shell\\3off | MUIVerb = sz:Decompress Windows\nHKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneCompactOs\\shell\\3off\\command | @ = sz:powershell.exe -NoProfile -WindowStyle Hidden -Command \"Start-Process cmd.exe -Verb RunAs -ArgumentList '/k','compact /compactos:never'\"",
     L"HKCU\\Software\\Classes\\DesktopBackground\\Shell\\TourneCompactOs | -",
     L""},
    {L"menuNew.newBat", L"Batch file (.bat)", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\.bat\\ShellNew | NullFile = sz:",
     L"HKCU\\Software\\Classes\\.bat\\ShellNew | -",
     L""},
    {L"menuNew.newCmd", L"Command script (.cmd)", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\.cmd\\ShellNew | NullFile = sz:",
     L"HKCU\\Software\\Classes\\.cmd\\ShellNew | -",
     L""},
    {L"menuNew.newPs1", L"PowerShell script (.ps1)", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\.ps1\\ShellNew | NullFile = sz:",
     L"HKCU\\Software\\Classes\\.ps1\\ShellNew | -",
     L""},
    {L"menuNew.newVbs", L"VBScript file (.vbs)", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\.vbs\\ShellNew | NullFile = sz:",
     L"HKCU\\Software\\Classes\\.vbs\\ShellNew | -",
     L""},
    {L"menuNew.newReg", L"Registration entries (.reg)", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Classes\\.reg\\ShellNew | NullFile = sz:",
     L"HKCU\\Software\\Classes\\.reg\\ShellNew | -",
     L""},
    {L"signin.noLockScreen", L"Skip the lock screen", 't', 0, true, false, nullptr,
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\Windows\\Personalization | NoLockScreen = dword:1",
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\Windows\\Personalization | NoLockScreen -",
     L""},
    {L"signin.lockScreenImage", L"Lock screen picture for everyone", 's', 's', true, false, nullptr,
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\Windows\\Personalization | LockScreenImage = sz:{}",
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\Windows\\Personalization | LockScreenImage -",
     L""},
    {L"signin.slideshowDuration", L"Lock screen slideshow length (s)", 'n', 0, false, false, L"0",
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Lock Screen | SlideshowDuration = dword:{ms}",
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Lock Screen | SlideshowDuration -",
     L""},
    {L"signin.hideLockedUser", L"Hide your name on the lock screen", 't', 0, true, false, nullptr,
     L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System | DontDisplayLockedUserId = dword:3",
     L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System | DontDisplayLockedUserId -",
     L""},
    {L"signin.noLockNetworkIcon", L"No network icon on the lock screen", 't', 0, true, false, nullptr,
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\Windows\\System | DontDisplayNetworkSelectionUI = dword:1",
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\Windows\\System | DontDisplayNetworkSelectionUI -",
     L""},
    {L"signin.noLogonBlur", L"Sign-in screen: sharp background", 't', 0, true, false, nullptr,
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\Windows\\System | DisableAcrylicBackgroundOnLogon = dword:1",
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\Windows\\System | DisableAcrylicBackgroundOnLogon -",
     L""},
    {L"signin.noLogonImage", L"Sign-in screen: plain colour background", 't', 0, true, false, nullptr,
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\Windows\\System | DisableLogonBackgroundImage = dword:1",
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\Windows\\System | DisableLogonBackgroundImage = dword:0",
     L""},
    {L"signin.hideLastUser", L"Sign-in screen: don't show the last user", 't', 0, true, false, nullptr,
     L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System | dontdisplaylastusername = dword:1",
     L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System | dontdisplaylastusername = dword:0",
     L""},
    {L"signin.noLogonPowerButton", L"Sign-in screen: no power button", 't', 0, true, false, nullptr,
     L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System | shutdownwithoutlogon = dword:0",
     L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System | shutdownwithoutlogon = dword:1",
     L""},
    {L"signin.numLockAtLogon", L"Sign-in screen: Num Lock on", 't', 0, true, false, nullptr,
     L"HKU\\.DEFAULT\\Control Panel\\Keyboard | InitialKeyboardIndicators = sz:2",
     L"HKU\\.DEFAULT\\Control Panel\\Keyboard | InitialKeyboardIndicators = sz:2147483648",
     L""},
    {L"signin.requireCtrlAltDel", L"Press Ctrl + Alt + Del to sign in", 't', 0, true, false, nullptr,
     L"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon | DisableCAD = dword:0",
     L"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon | DisableCAD = dword:1",
     L""},
    {L"signin.autoLogonCheckbox", L"Bring back automatic sign-in in netplwiz", 't', 0, true, false, nullptr,
     L"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\PasswordLess\\Device | DevicePasswordLessBuildVersion = dword:0",
     L"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\PasswordLess\\Device | DevicePasswordLessBuildVersion = dword:2",
     L""},
    {L"signin.signInTitle", L"Message before sign-in: title", 's', 0, true, false, nullptr,
     L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System | legalnoticecaption = sz:{}",
     L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System | legalnoticecaption = sz:",
     L""},
    {L"signin.signInText", L"Message before sign-in: text", 's', 0, true, false, nullptr,
     L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System | legalnoticetext = sz:{}",
     L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System | legalnoticetext = sz:",
     L""},
    {L"signin.verboseStatus", L"Detailed status messages", 't', 0, true, false, nullptr,
     L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System | VerboseStatus = dword:1",
     L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System | VerboseStatus -",
     L""},
    {L"signin.noPasswordReveal", L"No password reveal button", 't', 0, true, false, nullptr,
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\Windows\\CredUI | DisablePasswordReveal = dword:1",
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\Windows\\CredUI | DisablePasswordReveal -",
     L""},
    {L"boot.autoRepair", L"Automatic repair after failed starts", 't', 'r', true, false, nullptr,
     L"admin bcdedit /set {current} recoveryenabled Yes",
     L"admin bcdedit /set {current} recoveryenabled No",
     L""},
    {L"boot.legacyBootMenu", L"F8 boot menu", 't', 'r', true, false, nullptr,
     L"admin bcdedit /set {current} bootmenupolicy Legacy",
     L"admin bcdedit /set {current} bootmenupolicy Standard",
     L""},
    {L"boot.bootLogo", L"Boot logo and spinner", 't', 'r', true, false, nullptr,
     L"admin bcdedit /deletevalue {current} quietboot",
     L"admin bcdedit /set {current} quietboot Yes",
     L""},
    {L"boot.chkdskTimeout", L"Disk check countdown at startup (s)", 'n', 0, true, false, L"8",
     L"HKLM\\SYSTEM\\CurrentControlSet\\Control\\Session Manager | AutoChkTimeout = dword:{}",
     L"",
     L""},
    {L"boot.bsodNoSmiley", L"Stop screen without the sad face", 't', 'r', true, false, nullptr,
     L"HKLM\\SYSTEM\\CurrentControlSet\\Control\\CrashControl | DisableEmoticon = dword:1",
     L"HKLM\\SYSTEM\\CurrentControlSet\\Control\\CrashControl | DisableEmoticon -",
     L""},
    {L"boot.bsodDetails", L"Stop screen shows technical details", 't', 'r', true, false, nullptr,
     L"HKLM\\SYSTEM\\CurrentControlSet\\Control\\CrashControl | DisplayParameters = dword:1",
     L"HKLM\\SYSTEM\\CurrentControlSet\\Control\\CrashControl | DisplayParameters -",
     L""},
    {L"boot.crashOnScroll", L"Crash on purpose with Ctrl + Scroll Lock", 't', 'r', true, false, nullptr,
     L"HKLM\\SYSTEM\\CurrentControlSet\\Services\\kbdhid\\Parameters | CrashOnCtrlScroll = dword:1\nHKLM\\SYSTEM\\CurrentControlSet\\Services\\i8042prt\\Parameters | CrashOnCtrlScroll = dword:1",
     L"HKLM\\SYSTEM\\CurrentControlSet\\Services\\kbdhid\\Parameters | CrashOnCtrlScroll -\nHKLM\\SYSTEM\\CurrentControlSet\\Services\\i8042prt\\Parameters | CrashOnCtrlScroll -",
     L""},
    {L"boot.registryBackup", L"Automatic registry backup", 't', 'r', true, false, nullptr,
     L"HKLM\\SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Configuration Manager | EnablePeriodicBackup = dword:1",
     L"HKLM\\SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Configuration Manager | EnablePeriodicBackup -",
     L""},
    {L"boot.restorePointAnyTime", L"Create restore points without the 24-hour limit", 't', 0, true, false, nullptr,
     L"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\SystemRestore | SystemRestorePointCreationFrequency = dword:0",
     L"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\SystemRestore | SystemRestorePointCreationFrequency -",
     L""},
    {L"updates.noWindowsUpdate", L"No automatic Windows updates", 't', 0, true, false, nullptr,
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\Windows\\WindowsUpdate\\AU | NoAutoUpdate = dword:1\nHKLM\\SOFTWARE\\Policies\\Microsoft\\Windows\\WindowsUpdate\\AU | AUOptions = dword:1\nHKLM\\SOFTWARE\\Policies\\Microsoft\\Windows\\WindowsUpdate\\AU | NoAutoRebootWithLoggedOnUsers = dword:1",
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\Windows\\WindowsUpdate\\AU | NoAutoUpdate -\nHKLM\\SOFTWARE\\Policies\\Microsoft\\Windows\\WindowsUpdate\\AU | AUOptions -\nHKLM\\SOFTWARE\\Policies\\Microsoft\\Windows\\WindowsUpdate\\AU | NoAutoRebootWithLoggedOnUsers -",
     L""},
    {L"updates.noDriverUpdates", L"No driver updates from Windows Update", 't', 0, true, false, nullptr,
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\Windows\\WindowsUpdate | ExcludeWUDriversInQualityUpdate = dword:1\nHKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\DriverSearching | SearchOrderConfig = dword:0",
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\Windows\\WindowsUpdate | ExcludeWUDriversInQualityUpdate -\nHKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\DriverSearching | SearchOrderConfig = dword:1",
     L""},
    {L"updates.noMrt", L"Don't install the Malicious Software Removal Tool", 't', 0, true, false, nullptr,
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\MRT | DontOfferThroughWUAU = dword:1",
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\MRT | DontOfferThroughWUAU -",
     L""},
    {L"updates.noMaintenance", L"Turn off automatic maintenance", 't', 0, true, false, nullptr,
     L"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Schedule\\Maintenance | MaintenanceDisabled = dword:1",
     L"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Schedule\\Maintenance | MaintenanceDisabled -",
     L""},
    {L"security.uac", L"User Account Control", 'c', 'r', true, false, nullptr,
     L"enabled\036HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System | EnableLUA = dword:1\037disabled\036HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System | EnableLUA = dword:0",
     L"",
     L""},
    {L"security.adminApproval", L"UAC for the built-in Administrator", 't', 'r', true, false, nullptr,
     L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System | FilterAdministratorToken = dword:1",
     L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System | FilterAdministratorToken = dword:0",
     L""},
    {L"security.builtinAdmin", L"Built-in Administrator account", 't', 0, true, false, nullptr,
     L"admin net user Administrator /active:yes",
     L"admin net user Administrator /active:no",
     L""},
    {L"security.noDefender", L"Turn off Microsoft Defender Antivirus", 't', 'r', true, false, nullptr,
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\Windows Defender | DisableAntiSpyware = dword:1",
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\Windows Defender | DisableAntiSpyware -",
     L""},
    {L"security.noDownloadBlocking", L"Don't mark downloaded files", 't', 0, false, false, nullptr,
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Attachments | SaveZoneInformation = dword:1",
     L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Attachments | SaveZoneInformation -",
     L""},
    {L"security.usbWriteProtect", L"Write-protect USB drives", 't', 'r', true, false, nullptr,
     L"HKLM\\SYSTEM\\CurrentControlSet\\Control\\StorageDevicePolicies | WriteProtect = dword:1",
     L"HKLM\\SYSTEM\\CurrentControlSet\\Control\\StorageDevicePolicies | WriteProtect -",
     L""},
    {L"security.msiSafeMode", L"Windows Installer works in Safe Mode", 't', 0, true, false, nullptr,
     L"HKLM\\SYSTEM\\CurrentControlSet\\Control\\SafeBoot\\Minimal\\MSIServer | @ = sz:Service\nHKLM\\SYSTEM\\CurrentControlSet\\Control\\SafeBoot\\Network\\MSIServer | @ = sz:Service",
     L"HKLM\\SYSTEM\\CurrentControlSet\\Control\\SafeBoot\\Minimal\\MSIServer | -\nHKLM\\SYSTEM\\CurrentControlSet\\Control\\SafeBoot\\Network\\MSIServer | -",
     L""},
    {L"security.adminShares", L"Administrative shares (C$, ADMIN$)", 't', 'r', true, false, nullptr,
     L"HKLM\\SYSTEM\\CurrentControlSet\\Services\\LanmanServer\\Parameters | AutoShareWks -",
     L"HKLM\\SYSTEM\\CurrentControlSet\\Services\\LanmanServer\\Parameters | AutoShareWks = dword:0",
     L""},
    {L"security.linkedConnections", L"Mapped drives in programs run as administrator", 't', 'r', true, false, nullptr,
     L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System | EnableLinkedConnections = dword:1",
     L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System | EnableLinkedConnections -",
     L""},
    {L"security.rdpPort", L"Remote Desktop port", 'n', 'r', true, false, L"3389",
     L"HKLM\\SYSTEM\\CurrentControlSet\\Control\\Terminal Server\\WinStations\\RDP-Tcp | PortNumber = dword:{}",
     L"",
     L""},
    {L"security.ipRouting", L"IP routing (forwarding)", 't', 'r', true, false, nullptr,
     L"HKLM\\SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters | IPEnableRouter = dword:1",
     L"HKLM\\SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters | IPEnableRouter = dword:0",
     L""},
    {L"power.hibernation", L"Hibernation", 'c', 0, true, false, nullptr,
     L"disabled\036admin powercfg.exe /hibernate off\037enabled\036admin powercfg.exe /hibernate on\nadmin powercfg.exe /hibernate /type full\037reduced\036admin powercfg.exe /hibernate on\nadmin powercfg.exe /hibernate /type reduced",
     L"",
     L""},
    {L"power.hiberFileSize", L"Hibernation file size (% of RAM)", 'n', 0, true, true, L"0",
     L"admin powercfg.exe /hibernate /size {}",
     L"",
     L""},
    {L"power.noPowerThrottling", L"Turn off power throttling", 't', 'r', true, false, nullptr,
     L"HKLM\\SYSTEM\\CurrentControlSet\\Control\\Power\\PowerThrottling | PowerThrottlingOff = dword:1",
     L"HKLM\\SYSTEM\\CurrentControlSet\\Control\\Power\\PowerThrottling | PowerThrottlingOff -",
     L""},
    {L"power.wakePassword", L"Show \"Require a password on wakeup\" in Power Options", 't', 0, true, false, nullptr,
     L"HKLM\\SYSTEM\\CurrentControlSet\\Control\\Power\\PowerSettings\\0e796bdb-100d-47d6-a2d5-f7d2daa51f51 | Attributes = dword:2",
     L"HKLM\\SYSTEM\\CurrentControlSet\\Control\\Power\\PowerSettings\\0e796bdb-100d-47d6-a2d5-f7d2daa51f51 | Attributes = dword:1",
     L""},
    {L"power.screenSaverGrace", L"Screen saver lock grace period (s)", 'n', 0, true, false, L"5",
     L"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon | ScreenSaverGracePeriod = sz:{}",
     L"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon | ScreenSaverGracePeriod -",
     L""},
    {L"power.svchostSplitKB", L"Svchost split threshold (KB)", 'n', 0, true, false, L"3670016",
     L"HKLM\\SYSTEM\\CurrentControlSet\\Control | SvcHostSplitThresholdInKB = dword:{}",
     L"",
     L""},
    {L"apps.photoViewer", L"Windows Photo Viewer in \"Open with\"", 't', 0, true, false, nullptr,
     L"HKLM\\SOFTWARE\\Microsoft\\Windows Photo Viewer\\Capabilities\\FileAssociations | .jpg = sz:PhotoViewer.FileAssoc.Tiff\nHKLM\\SOFTWARE\\Microsoft\\Windows Photo Viewer\\Capabilities\\FileAssociations | .jpeg = sz:PhotoViewer.FileAssoc.Tiff\nHKLM\\SOFTWARE\\Microsoft\\Windows Photo Viewer\\Capabilities\\FileAssociations | .png = sz:PhotoViewer.FileAssoc.Tiff\nHKLM\\SOFTWARE\\Microsoft\\Windows Photo Viewer\\Capabilities\\FileAssociations | .bmp = sz:PhotoViewer.FileAssoc.Tiff\nHKLM\\SOFTWARE\\Microsoft\\Windows Photo Viewer\\Capabilities\\FileAssociations | .gif = sz:PhotoViewer.FileAssoc.Tiff\nHKLM\\SOFTWARE\\Microsoft\\Windows Photo Viewer\\Capabilities\\FileAssociations | .ico = sz:PhotoViewer.FileAssoc.Tiff",
     L"HKLM\\SOFTWARE\\Microsoft\\Windows Photo Viewer\\Capabilities\\FileAssociations | .jpg -\nHKLM\\SOFTWARE\\Microsoft\\Windows Photo Viewer\\Capabilities\\FileAssociations | .jpeg -\nHKLM\\SOFTWARE\\Microsoft\\Windows Photo Viewer\\Capabilities\\FileAssociations | .png -\nHKLM\\SOFTWARE\\Microsoft\\Windows Photo Viewer\\Capabilities\\FileAssociations | .bmp -\nHKLM\\SOFTWARE\\Microsoft\\Windows Photo Viewer\\Capabilities\\FileAssociations | .gif -\nHKLM\\SOFTWARE\\Microsoft\\Windows Photo Viewer\\Capabilities\\FileAssociations | .ico -",
     L""},
    {L"apps.noInk", L"Turn off Windows Ink Workspace", 't', 0, true, false, nullptr,
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\WindowsInkWorkspace | AllowWindowsInkWorkspace = dword:0",
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\WindowsInkWorkspace | AllowWindowsInkWorkspace -",
     L""},
    {L"apps.noOneDriveBackup", L"Don't move Desktop, Documents and Pictures to OneDrive", 't', 0, true, false, nullptr,
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\OneDrive | KFMBlockOptIn = dword:1",
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\OneDrive | KFMBlockOptIn -",
     L""},
    {L"panels.netplwiz", L"Control Panel: User Accounts (classic)", 't', 'e', true, false, nullptr,
     L"HKLM\\SOFTWARE\\Classes\\CLSID\\{5A1E7B1C-3E2F-4D0A-9C1B-7E0F2A6D4B01} | @ = sz:User Accounts (classic)\nHKLM\\SOFTWARE\\Classes\\CLSID\\{5A1E7B1C-3E2F-4D0A-9C1B-7E0F2A6D4B01} | InfoTip = sz:User Accounts (classic)\nHKLM\\SOFTWARE\\Classes\\CLSID\\{5A1E7B1C-3E2F-4D0A-9C1B-7E0F2A6D4B01} | System.ControlPanel.Category = sz:1\nHKLM\\SOFTWARE\\Classes\\CLSID\\{5A1E7B1C-3E2F-4D0A-9C1B-7E0F2A6D4B01}\\DefaultIcon | @ = expand:%SystemRoot%\\System32\\netplwiz.exe\nHKLM\\SOFTWARE\\Classes\\CLSID\\{5A1E7B1C-3E2F-4D0A-9C1B-7E0F2A6D4B01}\\Shell\\Open\\Command | @ = expand:%SystemRoot%\\System32\\netplwiz.exe\nHKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ControlPanel\\NameSpace\\{5A1E7B1C-3E2F-4D0A-9C1B-7E0F2A6D4B01} | @ = sz:User Accounts (classic)",
     L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ControlPanel\\NameSpace\\{5A1E7B1C-3E2F-4D0A-9C1B-7E0F2A6D4B01} | -\nHKLM\\SOFTWARE\\Classes\\CLSID\\{5A1E7B1C-3E2F-4D0A-9C1B-7E0F2A6D4B01} | -",
     L""},
    {L"panels.classicBackground", L"Control Panel: Desktop Background", 't', 'e', true, false, nullptr,
     L"HKLM\\SOFTWARE\\Classes\\CLSID\\{5A1E7B1C-3E2F-4D0A-9C1B-7E0F2A6D4B02} | @ = sz:Desktop Background\nHKLM\\SOFTWARE\\Classes\\CLSID\\{5A1E7B1C-3E2F-4D0A-9C1B-7E0F2A6D4B02} | InfoTip = sz:Desktop Background\nHKLM\\SOFTWARE\\Classes\\CLSID\\{5A1E7B1C-3E2F-4D0A-9C1B-7E0F2A6D4B02} | System.ControlPanel.Category = sz:1\nHKLM\\SOFTWARE\\Classes\\CLSID\\{5A1E7B1C-3E2F-4D0A-9C1B-7E0F2A6D4B02}\\DefaultIcon | @ = expand:%SystemRoot%\\System32\\imageres.dll,-110\nHKLM\\SOFTWARE\\Classes\\CLSID\\{5A1E7B1C-3E2F-4D0A-9C1B-7E0F2A6D4B02}\\Shell\\Open\\Command | @ = expand:%SystemRoot%\\explorer.exe shell:::{ED834ED6-4B5A-4bfe-8F11-A626DCB6A921} -Microsoft.Personalization\\pageWallpaper\nHKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ControlPanel\\NameSpace\\{5A1E7B1C-3E2F-4D0A-9C1B-7E0F2A6D4B02} | @ = sz:Desktop Background",
     L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ControlPanel\\NameSpace\\{5A1E7B1C-3E2F-4D0A-9C1B-7E0F2A6D4B02} | -\nHKLM\\SOFTWARE\\Classes\\CLSID\\{5A1E7B1C-3E2F-4D0A-9C1B-7E0F2A6D4B02} | -",
     L""},
    {L"panels.classicColor", L"Control Panel: Color and Appearance", 't', 'e', true, false, nullptr,
     L"HKLM\\SOFTWARE\\Classes\\CLSID\\{5A1E7B1C-3E2F-4D0A-9C1B-7E0F2A6D4B03} | @ = sz:Color and Appearance\nHKLM\\SOFTWARE\\Classes\\CLSID\\{5A1E7B1C-3E2F-4D0A-9C1B-7E0F2A6D4B03} | InfoTip = sz:Color and Appearance\nHKLM\\SOFTWARE\\Classes\\CLSID\\{5A1E7B1C-3E2F-4D0A-9C1B-7E0F2A6D4B03} | System.ControlPanel.Category = sz:1\nHKLM\\SOFTWARE\\Classes\\CLSID\\{5A1E7B1C-3E2F-4D0A-9C1B-7E0F2A6D4B03}\\DefaultIcon | @ = expand:%SystemRoot%\\System32\\themecpl.dll,-2\nHKLM\\SOFTWARE\\Classes\\CLSID\\{5A1E7B1C-3E2F-4D0A-9C1B-7E0F2A6D4B03}\\Shell\\Open\\Command | @ = expand:%SystemRoot%\\explorer.exe shell:::{ED834ED6-4B5A-4bfe-8F11-A626DCB6A921} -Microsoft.Personalization\\pageColorization\nHKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ControlPanel\\NameSpace\\{5A1E7B1C-3E2F-4D0A-9C1B-7E0F2A6D4B03} | @ = sz:Color and Appearance",
     L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ControlPanel\\NameSpace\\{5A1E7B1C-3E2F-4D0A-9C1B-7E0F2A6D4B03} | -\nHKLM\\SOFTWARE\\Classes\\CLSID\\{5A1E7B1C-3E2F-4D0A-9C1B-7E0F2A6D4B03} | -",
     L""},
    {L"panels.classicPersonalization", L"Control Panel: Personalization (classic)", 't', 'e', true, false, nullptr,
     L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ControlPanel\\NameSpace\\{ED834ED6-4B5A-4bfe-8F11-A626DCB6A921} | @ = sz:Personalization",
     L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ControlPanel\\NameSpace\\{ED834ED6-4B5A-4bfe-8F11-A626DCB6A921} | -",
     L""},
    {L"panels.modernPersonalization", L"Control Panel: Personalization (Settings)", 't', 'e', true, false, nullptr,
     L"HKLM\\SOFTWARE\\Classes\\CLSID\\{5A1E7B1C-3E2F-4D0A-9C1B-7E0F2A6D4B04} | @ = sz:Personalization\nHKLM\\SOFTWARE\\Classes\\CLSID\\{5A1E7B1C-3E2F-4D0A-9C1B-7E0F2A6D4B04} | InfoTip = sz:Personalization\nHKLM\\SOFTWARE\\Classes\\CLSID\\{5A1E7B1C-3E2F-4D0A-9C1B-7E0F2A6D4B04} | System.ControlPanel.Category = sz:1\nHKLM\\SOFTWARE\\Classes\\CLSID\\{5A1E7B1C-3E2F-4D0A-9C1B-7E0F2A6D4B04}\\DefaultIcon | @ = expand:%SystemRoot%\\System32\\themecpl.dll,-1\nHKLM\\SOFTWARE\\Classes\\CLSID\\{5A1E7B1C-3E2F-4D0A-9C1B-7E0F2A6D4B04}\\Shell\\Open\\Command | @ = expand:%SystemRoot%\\explorer.exe ms-settings:personalization\nHKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ControlPanel\\NameSpace\\{5A1E7B1C-3E2F-4D0A-9C1B-7E0F2A6D4B04} | @ = sz:Personalization",
     L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ControlPanel\\NameSpace\\{5A1E7B1C-3E2F-4D0A-9C1B-7E0F2A6D4B04} | -\nHKLM\\SOFTWARE\\Classes\\CLSID\\{5A1E7B1C-3E2F-4D0A-9C1B-7E0F2A6D4B04} | -",
     L""},
    {L"panels.windowsUpdate", L"Control Panel: Windows Update", 't', 'e', true, false, nullptr,
     L"HKLM\\SOFTWARE\\Classes\\CLSID\\{5A1E7B1C-3E2F-4D0A-9C1B-7E0F2A6D4B05} | @ = sz:Windows Update\nHKLM\\SOFTWARE\\Classes\\CLSID\\{5A1E7B1C-3E2F-4D0A-9C1B-7E0F2A6D4B05} | InfoTip = sz:Windows Update\nHKLM\\SOFTWARE\\Classes\\CLSID\\{5A1E7B1C-3E2F-4D0A-9C1B-7E0F2A6D4B05} | System.ControlPanel.Category = sz:1\nHKLM\\SOFTWARE\\Classes\\CLSID\\{5A1E7B1C-3E2F-4D0A-9C1B-7E0F2A6D4B05}\\DefaultIcon | @ = expand:%SystemRoot%\\System32\\shell32.dll,-47\nHKLM\\SOFTWARE\\Classes\\CLSID\\{5A1E7B1C-3E2F-4D0A-9C1B-7E0F2A6D4B05}\\Shell\\Open\\Command | @ = expand:%SystemRoot%\\explorer.exe ms-settings:windowsupdate\nHKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ControlPanel\\NameSpace\\{5A1E7B1C-3E2F-4D0A-9C1B-7E0F2A6D4B05} | @ = sz:Windows Update",
     L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ControlPanel\\NameSpace\\{5A1E7B1C-3E2F-4D0A-9C1B-7E0F2A6D4B05} | -\nHKLM\\SOFTWARE\\Classes\\CLSID\\{5A1E7B1C-3E2F-4D0A-9C1B-7E0F2A6D4B05} | -",
     L""},
    {L"panels.noOnlineTips", L"No online tips and videos in Settings", 't', 0, true, false, nullptr,
     L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer | AllowOnlineTips = dword:0",
     L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer | AllowOnlineTips -",
     L""},
    {L"panels.settingsPages", L"Settings pages to hide or show", 's', 0, true, false, nullptr,
     L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer | SettingsPageVisibility = sz:{}",
     L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer | SettingsPageVisibility -",
     L""},
    {L"panels.hideInsider", L"Hide the Windows Insider page", 't', 0, true, false, nullptr,
     L"HKLM\\SOFTWARE\\Microsoft\\WindowsSelfHost\\UI\\Visibility | HideInsiderPage = dword:1",
     L"HKLM\\SOFTWARE\\Microsoft\\WindowsSelfHost\\UI\\Visibility | HideInsiderPage -",
     L""},
    {L"edge.firstRun", L"No first run experience", 't', 0, true, false, nullptr,
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\Edge | HideFirstRunExperience = dword:1",
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\Edge | HideFirstRunExperience -",
     L""},
    {L"edge.autoImport", L"No import from other browsers on first run", 't', 0, true, false, nullptr,
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\Edge | AutoImportAtFirstRun = dword:4",
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\Edge | AutoImportAtFirstRun -",
     L""},
    {L"edge.defaultPrompts", L"No \"make Edge your default browser\" prompts", 't', 0, true, false, nullptr,
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\Edge | DefaultBrowserSettingsCampaignEnabled = dword:0\nHKLM\\SOFTWARE\\Policies\\Microsoft\\Edge | ShowPDFDefaultRecommendationsEnabled = dword:0",
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\Edge | DefaultBrowserSettingsCampaignEnabled -\nHKLM\\SOFTWARE\\Policies\\Microsoft\\Edge | ShowPDFDefaultRecommendationsEnabled -",
     L""},
    {L"edge.implicitSignIn", L"No automatic sign-in with the Windows account", 't', 0, true, false, nullptr,
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\Edge | ImplicitSignInEnabled = dword:0",
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\Edge | ImplicitSignInEnabled -",
     L""},
    {L"edge.personalization", L"No personalisation of ads, news and the browser", 't', 0, true, false, nullptr,
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\Edge | PersonalizationReportingEnabled = dword:0",
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\Edge | PersonalizationReportingEnabled -",
     L""},
    {L"edge.recommendations", L"No recommendations and desktop notifications", 't', 0, true, false, nullptr,
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\Edge | ShowRecommendationsEnabled = dword:0",
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\Edge | ShowRecommendationsEnabled -",
     L""},
    {L"edge.follow", L"No Follow creators", 't', 0, true, false, nullptr,
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\Edge | EdgeFollowEnabled = dword:0",
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\Edge | EdgeFollowEnabled -",
     L""},
    {L"edge.standaloneSidebar", L"No standalone sidebar", 't', 0, true, false, nullptr,
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\Edge | StandaloneHubsSidebarEnabled = dword:0",
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\Edge | StandaloneHubsSidebarEnabled -",
     L""},
    {L"edge.collections", L"No Collections", 't', 0, true, false, nullptr,
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\Edge | EdgeCollectionsEnabled = dword:0",
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\Edge | EdgeCollectionsEnabled -",
     L""},
    {L"edge.splitScreen", L"No split screen", 't', 0, true, false, nullptr,
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\Edge | SplitScreenEnabled = dword:0",
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\Edge | SplitScreenEnabled -",
     L""},
    {L"edge.feedback", L"No feedback prompts", 't', 0, true, false, nullptr,
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\Edge | UserFeedbackAllowed = dword:0",
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\Edge | UserFeedbackAllowed -",
     L""},
    {L"edge.searchBar", L"No floating Bing search bar", 't', 0, true, false, nullptr,
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\Edge | SearchbarAllowed = dword:0",
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\Edge | SearchbarAllowed -",
     L""},
    {L"edge.restoreDialog", L"No \"Restore pages\" prompt after a crash", 't', 0, true, false, nullptr,
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\Edge | HideRestoreDialogEnabled = dword:1",
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\Edge | HideRestoreDialogEnabled -",
     L""},
    {L"edge.ntpTopSites", L"New tab: hide Microsoft's pinned sites", 't', 0, true, false, nullptr,
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\Edge | NewTabPageHideDefaultTopSites = dword:1",
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\Edge | NewTabPageHideDefaultTopSites -",
     L""},
    {L"edge.desktopShortcut", L"No desktop shortcut after Edge updates", 't', 0, true, false, nullptr,
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\EdgeUpdate | CreateDesktopShortcutDefault = dword:0",
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\EdgeUpdate | CreateDesktopShortcutDefault -",
     L""},
    {L"edge.noUpdates", L"No Edge updates", 't', 0, true, false, nullptr,
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\EdgeUpdate | UpdateDefault = dword:0\nHKLM\\SOFTWARE\\Policies\\Microsoft\\EdgeUpdate | AutoUpdateCheckPeriodMinutes = dword:0",
     L"HKLM\\SOFTWARE\\Policies\\Microsoft\\EdgeUpdate | UpdateDefault -\nHKLM\\SOFTWARE\\Policies\\Microsoft\\EdgeUpdate | AutoUpdateCheckPeriodMinutes -",
     L""},
    {L"system.oemManufacturer", L"Manufacturer", 's', 0, true, false, nullptr,
     L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\OEMInformation | Manufacturer = sz:{}",
     L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\OEMInformation | Manufacturer -",
     L""},
    {L"system.oemModel", L"Model", 's', 0, true, false, nullptr,
     L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\OEMInformation | Model = sz:{}",
     L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\OEMInformation | Model -",
     L""},
    {L"system.oemSupportHours", L"Support hours", 's', 0, true, false, nullptr,
     L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\OEMInformation | SupportHours = sz:{}",
     L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\OEMInformation | SupportHours -",
     L""},
    {L"system.oemSupportPhone", L"Support phone", 's', 0, true, false, nullptr,
     L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\OEMInformation | SupportPhone = sz:{}",
     L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\OEMInformation | SupportPhone -",
     L""},
    {L"system.oemSupportUrl", L"Support website", 's', 0, true, false, nullptr,
     L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\OEMInformation | SupportURL = sz:{}",
     L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\OEMInformation | SupportURL -",
     L""},
    {L"system.oemLogo", L"Logo (.bmp, 120 x 120)", 's', 0, true, false, nullptr,
     L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\OEMInformation | Logo = sz:{}",
     L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\OEMInformation | Logo -",
     L""},
    {L"system.registeredOwner", L"Registered owner", 's', 0, true, true, nullptr,
     L"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion | RegisteredOwner = sz:{}",
     L"",
     L""},
    {L"system.registeredOrganization", L"Registered organisation", 's', 0, true, true, nullptr,
     L"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion | RegisteredOrganization = sz:{}",
     L"",
     L""},
};

////////////////////////////////////////////////////////////////////////////////
// Settings

const Tweak* FindTweak(std::wstring_view id) {
    for (const Tweak& t : kTweaks) {
        if (id == t.id) {
            return &t;
        }
    }
    return nullptr;
}

std::wstring SettingValue(const Tweak& t) {
    if (t.kind == 's' || t.kind == 'c') {
        PCWSTR value = Wh_GetStringSetting(L"%s", t.id);
        std::wstring result = value ? value : L"";
        Wh_FreeStringSetting(value);
        return result;
    }
    return std::to_wstring(Wh_GetIntSetting(L"%s", t.id));
}

std::vector<std::wstring> Words(PCWSTR text) {
    std::vector<std::wstring> words;
    std::wstring word;
    for (PCWSTR p = text ? text : L"";; p++) {
        if (!*p || *p == L' ') {
            if (!word.empty()) {
                words.push_back(word);
            }
            word.clear();
            if (!*p) {
                break;
            }
        } else {
            word += *p;
        }
    }
    return words;
}

// What the stored fingerprint covers: the value and every setting it uses.
std::wstring Signature(const Tweak& t) {
    std::wstring sig = SettingValue(t);
    for (const std::wstring& id : Words(t.uses)) {
        const Tweak* used = FindTweak(id);
        sig += L'\x1D';
        sig += used ? SettingValue(*used) : L"";
    }
    return sig;
}

std::wstring Hash(const std::wstring& text) {
    unsigned long long h = 14695981039346656037ull;
    for (wchar_t c : text) {
        h = (h ^ (unsigned)c) * 1099511628211ull;
    }
    WCHAR buffer[20];
    swprintf_s(buffer, L"%016llx", h);
    return buffer;
}

// Fingerprints of what was last applied live in the mod's storage, one value
// per tweak.
std::wstring StoredHash(const Tweak& t) {
    WCHAR name[160], value[64] = L"";
    swprintf_s(name, L"tweaks.%ls", t.id);
    Wh_GetStringValue(name, value, ARRAYSIZE(value));
    return value;
}

void StoreHash(const Tweak& t, const std::wstring& hash) {
    WCHAR name[160];
    swprintf_s(name, L"tweaks.%ls", t.id);
    Wh_SetStringValue(name, hash.c_str());
}

// Each group has a "Use these tweaks" switch, off to begin with; General has
// none (its settings change nothing in Windows).
bool GroupOn(const Tweak& t) {
    std::wstring_view id = t.id;
    std::wstring group(id.substr(0, id.find(L'.')));
    if (group == L"general") {
        return true;
    }
    return Wh_GetIntSetting(L"%s.use", group.c_str()) != 0;
}

////////////////////////////////////////////////////////////////////////////////
// Text helpers

std::wstring Trim(std::wstring_view s) {
    size_t a = s.find_first_not_of(L" \t"), b = s.find_last_not_of(L" \t");
    return a == std::wstring_view::npos ? L"" : std::wstring(s.substr(a, b - a + 1));
}

void ReplaceAll(std::wstring& s, std::wstring_view from, std::wstring_view to) {
    for (size_t at = 0; (at = s.find(from, at)) != std::wstring::npos; at += to.size()) {
        s.replace(at, from.size(), to);
    }
}

std::vector<std::wstring> Split(std::wstring_view s, wchar_t sep) {
    std::vector<std::wstring> parts;
    size_t start = 0;
    for (size_t at; (at = s.find(sep, start)) != std::wstring_view::npos; start = at + 1) {
        parts.push_back(std::wstring(s.substr(start, at - start)));
    }
    parts.push_back(std::wstring(s.substr(start)));
    return parts;
}

std::wstring Expand(const std::wstring& s) {
    WCHAR buffer[2048];
    DWORD n = ExpandEnvironmentStringsW(s.c_str(), buffer, ARRAYSIZE(buffer));
    return n && n <= ARRAYSIZE(buffer) ? buffer : s;
}

// "#RRGGBB" to the ABGR DWORD Windows stores colours in.
std::wstring ToAbgr(const std::wstring& color) {
    std::wstring hex = Trim(color);
    if (!hex.empty() && hex[0] == L'#') {
        hex.erase(0, 1);
    }
    unsigned rgb = wcstoul(hex.c_str(), nullptr, 16);
    unsigned abgr = 0xFF000000u | ((rgb & 0xFF) << 16) | (rgb & 0xFF00) | ((rgb >> 16) & 0xFF);
    return std::to_wstring(abgr);
}

// "F,H,I" to the NoDrives bit mask.
std::wstring ToDriveMask(const std::wstring& letters) {
    unsigned mask = 0;
    for (wchar_t c : letters) {
        wchar_t up = (wchar_t)towupper(c);
        if (up >= L'A' && up <= L'Z') {
            mask |= 1u << (up - L'A');
        }
    }
    return std::to_wstring(mask);
}

std::wstring Substitute(std::wstring op, const std::wstring& value) {
    if (op.find(L'{') == std::wstring::npos) {
        return op;
    }
    size_t at = 0;
    while ((at = op.find(L"{@", at)) != std::wstring::npos) {
        size_t end = op.find(L'}', at);
        if (end == std::wstring::npos) {
            break;
        }
        const Tweak* used = FindTweak(std::wstring_view(op).substr(at + 2, end - at - 2));
        std::wstring text = used ? SettingValue(*used) : L"";
        op.replace(at, end - at + 1, text);
        at += text.size();
    }
    int number = _wtoi(value.c_str());
    ReplaceAll(op, L"{twips}", std::to_wstring(-15 * number));
    ReplaceAll(op, L"{ms}", std::to_wstring(number * 1000));
    ReplaceAll(op, L"{drives}", ToDriveMask(value));
    ReplaceAll(op, L"{abgr}", ToAbgr(value));
    ReplaceAll(op, L"{cmd}", _wcsicmp(value.c_str(), L"none") == 0 ? L"" : value);
    ReplaceAll(op, L"{}", value);
    return op;
}

////////////////////////////////////////////////////////////////////////////////
// Registry

struct RegOp {
    HKEY root = nullptr;  // HKCU, HKLM or HKU
    std::wstring rootName, path, name, type, data;
    char action = 0;      // 's' set, 'v' delete value, 'k' delete key, 'c' create
};

bool ParseRegOp(const std::wstring& line, RegOp& op) {
    size_t bar = line.find(L" | ");
    if (bar == std::wstring::npos) {
        return false;
    }
    std::wstring key = Trim(std::wstring_view(line).substr(0, bar));
    std::wstring spec = Trim(std::wstring_view(line).substr(bar + 3));
    struct {
        PCWSTR prefix;
        HKEY root;
        PCWSTR name;
    } roots[] = {
        {L"HKCU\\", HKEY_CURRENT_USER, L"HKEY_CURRENT_USER"},
        {L"HKLM\\", HKEY_LOCAL_MACHINE, L"HKEY_LOCAL_MACHINE"},
        {L"HKU\\", HKEY_USERS, L"HKEY_USERS"},
    };
    for (auto& r : roots) {
        if (key.rfind(r.prefix, 0) == 0) {
            op.root = r.root;
            op.rootName = r.name;
            op.path = key.substr(wcslen(r.prefix));
        }
    }
    if (!op.root) {
        return false;
    }
    if (spec == L"-") {
        op.action = 'k';
        return true;
    }
    if (spec == L"+") {
        op.action = 'c';
        return true;
    }
    size_t eq = spec.find(L" = ");
    if (eq != std::wstring::npos) {
        op.action = 's';
        op.name = spec.substr(0, eq);
        std::wstring value = spec.substr(eq + 3);
        size_t colon = value.find(L':');
        if (colon == std::wstring::npos) {
            return false;
        }
        op.type = value.substr(0, colon);
        op.data = value.substr(colon + 1);
        return true;
    }
    if (spec.size() > 2 && spec.compare(spec.size() - 2, 2, L" -") == 0) {
        op.action = 'v';
        op.name = spec.substr(0, spec.size() - 2);
        return true;
    }
    return false;
}

PCWSTR ValueName(const RegOp& op) {
    return op.name == L"@" ? nullptr : op.name.c_str();
}

std::vector<BYTE> ValueBytes(const RegOp& op, DWORD& type) {
    std::vector<BYTE> bytes;
    auto putString = [&](const std::wstring& s) {
        const BYTE* p = (const BYTE*)s.c_str();
        bytes.assign(p, p + (s.size() + 1) * sizeof(WCHAR));
    };
    if (op.type == L"dword") {
        type = REG_DWORD;
        DWORD v = (DWORD)wcstoul(op.data.c_str(), nullptr, 0);
        bytes.assign((BYTE*)&v, (BYTE*)&v + 4);
    } else if (op.type == L"qword") {
        type = REG_QWORD;
        unsigned long long v = wcstoull(op.data.c_str(), nullptr, 0);
        bytes.assign((BYTE*)&v, (BYTE*)&v + 8);
    } else if (op.type == L"expand") {
        type = REG_EXPAND_SZ;
        putString(op.data);
    } else if (op.type == L"multi") {
        type = REG_MULTI_SZ;
        std::wstring multi;
        for (const std::wstring& part : Split(op.data, L';')) {
            multi += part;
            multi += L'\0';
        }
        multi += L'\0';
        const BYTE* p = (const BYTE*)multi.data();
        bytes.assign(p, p + multi.size() * sizeof(WCHAR));
    } else {
        type = REG_SZ;
        putString(op.data);
    }
    return bytes;
}

bool ApplyUserRegOp(const RegOp& op) {
    HKEY key;
    switch (op.action) {
        case 'k':
            RegDeleteTreeW(op.root, op.path.c_str());
            RegDeleteKeyExW(op.root, op.path.c_str(), KEY_WOW64_64KEY, 0);
            return true;
        case 'c':
        case 's':
            if (RegCreateKeyExW(op.root, op.path.c_str(), 0, nullptr, 0,
                                KEY_SET_VALUE | KEY_WOW64_64KEY, nullptr, &key,
                                nullptr) != ERROR_SUCCESS) {
                return false;
            }
            if (op.action == 's') {
                DWORD type;
                std::vector<BYTE> bytes = ValueBytes(op, type);
                RegSetValueExW(key, ValueName(op), 0, type, bytes.data(), (DWORD)bytes.size());
            }
            RegCloseKey(key);
            return true;
        case 'v':
            if (RegOpenKeyExW(op.root, op.path.c_str(), 0, KEY_SET_VALUE | KEY_WOW64_64KEY,
                              &key) == ERROR_SUCCESS) {
                RegDeleteValueW(key, ValueName(op));
                RegCloseKey(key);
            }
            return true;
    }
    return false;
}

std::wstring RegQuote(const std::wstring& s) {
    std::wstring out = L"\"";
    for (wchar_t c : s) {
        if (c == L'\\' || c == L'"') {
            out += L'\\';
        }
        out += c;
    }
    return out + L"\"";
}

// One op in .reg syntax, for the elevated step.
std::wstring RegFileLines(const RegOp& op) {
    std::wstring key = op.rootName + L"\\" + op.path;
    if (op.action == 'k') {
        return L"[-" + key + L"]\r\n";
    }
    std::wstring out = L"[" + key + L"]\r\n";
    if (op.action == 'c') {
        return out;
    }
    std::wstring name = op.name == L"@" ? L"@" : RegQuote(op.name);
    if (op.action == 'v') {
        return out + name + L"=-\r\n";
    }
    if (op.type == L"sz") {
        return out + name + L"=" + RegQuote(op.data) + L"\r\n";
    }
    DWORD type;
    std::vector<BYTE> bytes = ValueBytes(op, type);
    if (type == REG_DWORD) {
        WCHAR hex[16];
        swprintf_s(hex, L"%08x", *(DWORD*)bytes.data());
        return out + name + L"=dword:" + hex + L"\r\n";
    }
    std::wstring line = name + (type == REG_EXPAND_SZ   ? L"=hex(2):"
                                : type == REG_MULTI_SZ  ? L"=hex(7):"
                                : type == REG_QWORD     ? L"=hex(b):"
                                                        : L"=hex:");
    for (size_t i = 0; i < bytes.size(); i++) {
        WCHAR hex[4];
        swprintf_s(hex, i ? L",%02x" : L"%02x", bytes[i]);
        line += hex;
    }
    return out + line + L"\r\n";
}

std::vector<std::wstring> SubKeys(HKEY root, const std::wstring& path) {
    std::vector<std::wstring> names;
    HKEY key;
    if (RegOpenKeyExW(root, path.c_str(), 0, KEY_READ | KEY_WOW64_64KEY, &key) != ERROR_SUCCESS) {
        return names;
    }
    WCHAR name[256];
    for (DWORD i = 0;; i++) {
        DWORD len = ARRAYSIZE(name);
        if (RegEnumKeyExW(key, i, name, &len, nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS) {
            break;
        }
        names.push_back(name);
    }
    RegCloseKey(key);
    return names;
}

std::wstring ReadString(HKEY root, const std::wstring& path, PCWSTR name) {
    WCHAR buffer[1024] = L"";
    DWORD size = sizeof(buffer) - sizeof(WCHAR);
    RegGetValueW(root, path.c_str(), name, RRF_RT_REG_SZ | RRF_NOEXPAND,
                 nullptr, buffer, &size);
    return buffer;
}

////////////////////////////////////////////////////////////////////////////////
// One apply pass

struct Pass {
    // Machine-wide changes, per tweak, for the elevated step.
    std::map<size_t, std::wstring> regFiles;
    std::map<size_t, std::vector<std::wstring>> adminCommands;
    std::vector<std::wstring> notify;
    std::vector<std::wstring> afterRuns;
    std::map<std::wstring, std::wstring> metrics;
    bool shellChanged = false;
    bool iconsChanged = false;
    size_t current = 0;
};

void RunHidden(const std::wstring& command, DWORD waitMs) {
    std::wstring line = Expand(command);
    STARTUPINFOW si{sizeof(si)};
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    PROCESS_INFORMATION pi;
    if (CreateProcessW(nullptr, line.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr,
                       nullptr, &si, &pi)) {
        WaitForSingleObject(pi.hProcess, waitMs);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    } else {
        Wh_Log(L"Couldn't run %s", line.c_str());
    }
}

void DoRegOp(Pass& pass, const RegOp& op) {
    if (op.root == HKEY_CURRENT_USER) {
        ApplyUserRegOp(op);
    } else {
        pass.regFiles[pass.current] += RegFileLines(op);
    }
    if (op.path.find(L"Classes") != std::wstring::npos ||
        op.path.find(L"Explorer") != std::wstring::npos) {
        pass.shellChanged = true;
    }
}

void DoOp(Pass& pass, const std::wstring& line);

void DoOps(Pass& pass, std::initializer_list<std::wstring> lines) {
    for (const std::wstring& line : lines) {
        DoOp(pass, line);
    }
}

// A blank icon for "none" overlays, kept in the mod's storage folder.
std::wstring BlankIconPath() {
    WCHAR folder[MAX_PATH];
    if (!Wh_GetModStoragePath(folder, ARRAYSIZE(folder))) {
        return L"";
    }
    std::wstring path = std::wstring(folder) + L"\\blank.ico";
    if (GetFileAttributesW(path.c_str()) == INVALID_FILE_ATTRIBUTES) {
        // A 16x16 32-bit icon, every pixel transparent.
        std::vector<BYTE> ico(6 + 16 + 40 + 16 * 16 * 4 + 16 * 4, 0);
        BYTE* p = ico.data();
        p[2] = 1;
        p[4] = 1;
        p[6] = 16;
        p[7] = 16;
        p[10] = 1;
        p[12] = 32;
        DWORD size = (DWORD)ico.size() - 22, offset = 22;
        memcpy(p + 14, &size, 4);
        memcpy(p + 18, &offset, 4);
        BITMAPINFOHEADER bih{sizeof(bih), 16, 32, 1, 32};
        memcpy(p + 22, &bih, sizeof(bih));
        HANDLE file = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, 0, nullptr);
        if (file != INVALID_HANDLE_VALUE) {
            DWORD written;
            WriteFile(file, ico.data(), (DWORD)ico.size(), &written, nullptr);
            CloseHandle(file);
        }
    }
    return path;
}

// Deterministic GUID for a folder the user listed, so the same row keeps the
// same identity across saves.
std::wstring FolderGuid(const std::wstring& parent, const std::wstring& name) {
    std::wstring h1 = Hash(L"tourne-folder:" + parent + L":" + name);
    std::wstring h2 = Hash(L"tourne-folder-2:" + name + L":" + parent);
    WCHAR guid[40];
    swprintf_s(guid, L"{%.8ls-%.4ls-4%.3ls-8%.3ls-%.12ls}", h1.c_str(), h1.c_str() + 8,
               h1.c_str() + 12, h2.c_str(), h2.c_str() + 3);
    return guid;
}

// Folders added to This PC or the navigation pane: the rows are the whole
// list, so entries added before (by the suite or by Winaero) are removed
// first, then the rows are written.
void ApplyFolders(Pass& pass, const std::wstring& parent, const std::wstring& list) {
    const std::wstring ns = L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\" + parent +
                            L"\\NameSpace";
    for (const std::wstring& guid : SubKeys(HKEY_CURRENT_USER, ns)) {
        std::wstring by = ReadString(HKEY_CURRENT_USER, ns + L"\\" + guid, L"CreatedBy");
        std::wstring classBy = ReadString(HKEY_CURRENT_USER, L"Software\\Classes\\CLSID\\" + guid, L"CreatedBy");
        if (by == L"Winaero Tweaker" || by == L"Tourne'Suite" || classBy == L"Winaero Tweaker" ||
            classBy == L"Tourne'Suite") {
            DoOps(pass, {L"HKCU\\" + ns + L"\\" + guid + L" | -",
                         L"HKCU\\Software\\Classes\\CLSID\\" + guid + L" | -",
                         L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\HideDesktopIcons\\NewStartPanel | " + guid + L" -"});
        }
    }
    for (const std::wstring& row : Split(list, L'|')) {
        std::vector<std::wstring> fields = Split(row, L'>');
        std::wstring name = Trim(fields[0]);
        std::wstring path = fields.size() > 1 ? Trim(fields[1]) : L"";
        std::wstring icon = fields.size() > 2 ? Trim(fields[2]) : L"";
        if (name.empty() || path.empty()) {
            continue;
        }
        if (icon.empty()) {
            icon = L"%SystemRoot%\\System32\\imageres.dll,-3";
        }
        std::wstring guid = FolderGuid(parent, name);
        std::wstring cls = L"HKCU\\Software\\Classes\\CLSID\\" + guid;
        DoOps(pass, {
            cls + L" | @ = sz:" + name,
            cls + L" | CreatedBy = sz:Tourne'Suite",
            cls + L" | InfoTip = sz:" + path,
            cls + L" | DescriptionID = dword:3",
            cls + L" | System.IsPinnedToNameSpaceTree = dword:1",
            cls + L"\\DefaultIcon | @ = expand:" + icon,
            cls + L"\\InProcServer32 | @ = expand:%SystemRoot%\\System32\\shell32.dll",
            cls + L"\\InProcServer32 | ThreadingModel = sz:Both",
            cls + L"\\Instance | CLSID = sz:{0E5AAE11-A475-4c5b-AB00-C66DE400274E}",
            cls + L"\\Instance\\InitPropertyBag | Attributes = dword:0x11",
            cls + L"\\Instance\\InitPropertyBag | TargetFolderPath = expand:" + path,
            cls + L"\\ShellFolder | Attributes = dword:0xF080004D",
            cls + L"\\ShellFolder | FolderValueFlags = dword:0x28",
            L"HKCU\\" + ns + L"\\" + guid + L" | @ = sz:" + name,
            L"HKCU\\" + ns + L"\\" + guid + L" | CreatedBy = sz:Tourne'Suite",
        });
        if (parent == L"Desktop") {
            DoOp(pass, L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\HideDesktopIcons\\NewStartPanel | " + guid + L" = dword:1");
        }
    }
}

void DoOp(Pass& pass, const std::wstring& line) {
    std::wstring op = Trim(line);
    if (op.empty()) {
        return;
    }
    size_t space = op.find(L' ');
    std::wstring word = op.substr(0, space);
    std::wstring rest = space == std::wstring::npos ? L"" : Trim(std::wstring_view(op).substr(space + 1));

    if (word == L"run") {
        pass.afterRuns.push_back(rest);
    } else if (word == L"admin") {
        pass.adminCommands[pass.current].push_back(rest);
    } else if (word == L"notify") {
        pass.notify.push_back(rest);
    } else if (word == L"spi") {
        // spi ui|pv CODE VALUE: saved to the profile and broadcast.
        std::vector<std::wstring> parts = Words(rest.c_str());
        if (parts.size() == 3) {
            UINT code = wcstoul(parts[1].c_str(), nullptr, 0);
            UINT value = wcstoul(parts[2].c_str(), nullptr, 0);
            BOOL ok = parts[0] == L"ui"
                          ? SystemParametersInfoW(code, value, nullptr, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE)
                          : SystemParametersInfoW(code, 0, (PVOID)(UINT_PTR)value, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
            if (!ok) {
                Wh_Log(L"SystemParametersInfo 0x%x failed: %u", code, GetLastError());
            }
        }
    } else if (word == L"metric" || word == L"font") {
        size_t at = rest.find(L' ');
        std::wstring field = rest.substr(0, at);
        pass.metrics[word + L"." + field] = at == std::wstring::npos ? L"" : Trim(std::wstring_view(rest).substr(at + 1));
    } else if (word == L"style") {
        using SetSystemVisualStyle_t = HRESULT(WINAPI*)(PCWSTR, PCWSTR, PCWSTR, UINT);
        HMODULE uxtheme = LoadLibraryW(L"uxtheme.dll");
        auto setStyle = uxtheme ? (SetSystemVisualStyle_t)GetProcAddress(uxtheme, MAKEINTRESOURCEA(65)) : nullptr;
        if (setStyle) {
            HRESULT hr = setStyle(Expand(rest).c_str(), L"NormalColor", L"NormalSize", 0);
            Wh_Log(L"Visual style %s: 0x%08x", rest.c_str(), hr);
        }
    } else if (word == L"accents") {
        // Up to eight colours for Settings > Colors.
        std::vector<std::wstring> colors = Split(rest, L',');
        for (int i = 0; i < 8; i++) {
            std::wstring key = L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes\\Accents\\0\\Theme" + std::to_wstring(i);
            std::wstring color = i < (int)colors.size() ? Trim(colors[i]) : L"";
            DoOp(pass, color.empty() ? key + L" | -" : key + L" | Color = dword:" + ToAbgr(color));
        }
    } else if (word == L"pwsh") {
        // PowerShell 7 entries for the "Run or modify" menu.
        if (rest == L"1") {
            std::wstring base = L"HKCU\\Software\\Classes\\SystemFileAssociations\\.ps1\\Shell\\TourneRunModify\\shell\\";
            DoOps(pass, {
                base + L"1bpwsh | MUIVerb = sz:Run with PowerShell 7",
                base + L"1bpwsh\\command | @ = sz:pwsh.exe -NoProfile -ExecutionPolicy Bypass -NoExit -File \"%1\"",
                base + L"2bpwshadmin | MUIVerb = sz:Run with PowerShell 7 as administrator",
                base + L"2bpwshadmin | HasLUAShield = sz:",
                base + L"2bpwshadmin\\command | @ = sz:powershell.exe -NoProfile -WindowStyle Hidden -Command \"Start-Process pwsh.exe -Verb RunAs -ArgumentList '-NoProfile','-ExecutionPolicy','Bypass','-NoExit','-File','\\\"%1\\\"'\"",
            });
        }
    } else if (word == L"terminal") {
        // terminal CLASS MODE: a Windows Terminal submenu with the standard profiles.
        std::vector<std::wstring> parts = Words(rest.c_str());
        if (parts.size() == 2) {
            std::wstring key = L"HKCU\\Software\\Classes\\" + parts[0] + L"\\shell\\TourneTerminal";
            DoOps(pass, {
                key + L" | MUIVerb = sz:Open in Terminal",
                key + L" | SubCommands = sz:",
                key + L"\\shell\\1default | MUIVerb = sz:Default profile",
                key + L"\\shell\\1default\\command | @ = sz:wt.exe -d \"%V\"",
                key + L"\\shell\\2cmd | MUIVerb = sz:Command Prompt",
                key + L"\\shell\\2cmd\\command | @ = sz:wt.exe -d \"%V\" -p \"Command Prompt\"",
                key + L"\\shell\\3ps | MUIVerb = sz:Windows PowerShell",
                key + L"\\shell\\3ps\\command | @ = sz:wt.exe -d \"%V\" -p \"Windows PowerShell\"",
                key + L"\\shell\\4pwsh | MUIVerb = sz:PowerShell",
                key + L"\\shell\\4pwsh\\command | @ = sz:wt.exe -d \"%V\" -p \"PowerShell\"",
            });
            if (parts[1] == L"shift") {
                DoOp(pass, key + L" | Extended = sz:");
            }
        }
    } else if (word == L"shellicon") {
        // shellicon N [VALUE]: an Explorer overlay or stock icon.
        size_t at = rest.find(L' ');
        std::wstring index = rest.substr(0, at);
        std::wstring value = at == std::wstring::npos ? L"" : Trim(std::wstring_view(rest).substr(at + 1));
        std::wstring key = L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Icons | " + index;
        if (value.empty()) {
            DoOp(pass, key + L" -");
        } else if (_wcsicmp(value.c_str(), L"none") == 0) {
            DoOp(pass, key + L" = sz:" + BlankIconPath());
        } else if (_wcsicmp(value.c_str(), L"classic") == 0) {
            DoOp(pass, key + L" = expand:%SystemRoot%\\System32\\shell32.dll,-30");
        } else {
            DoOp(pass, key + L" = sz:" + value);
        }
        pass.iconsChanged = true;
    } else if (word == L"driveicons") {
        std::map<wchar_t, std::wstring> icons;
        for (const std::wstring& pair : Split(rest, L'|')) {
            size_t eq = pair.find(L'=');
            std::wstring letter = Trim(std::wstring_view(pair).substr(0, eq == std::wstring::npos ? 0 : eq));
            if (eq != std::wstring::npos && letter.size() == 1) {
                icons[(wchar_t)towupper(letter[0])] = Trim(std::wstring_view(pair).substr(eq + 1));
            }
        }
        for (wchar_t d = L'A'; d <= L'Z'; d++) {
            std::wstring key = L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\DriveIcons\\" + std::wstring(1, d);
            auto it = icons.find(d);
            DoOp(pass, it == icons.end() ? key + L" | -" : key + L"\\DefaultIcon | @ = sz:" + it->second);
        }
        pass.iconsChanged = true;
    } else if (word == L"folders") {
        size_t at = rest.find(L' ');
        ApplyFolders(pass, rest.substr(0, at), at == std::wstring::npos ? L"" : rest.substr(at + 1));
    } else {
        RegOp reg;
        if (ParseRegOp(op, reg)) {
            DoRegOp(pass, reg);
        } else {
            Wh_Log(L"Can't read op: %s", op.c_str());
        }
    }
}

// The ops for a tweak's current value.
std::wstring OpsFor(const Tweak& t, const std::wstring& value) {
    switch (t.kind) {
        case 't':
            return value != L"0" ? t.a : t.b;
        case 'n':
            // keepEmpty on a number: 0 means off, leave the system alone.
            if (t.keepEmpty && value == L"0") {
                return L"";
            }
            return t.b && *t.b && t.windowsDefault && value == t.windowsDefault ? t.b : t.a;
        case 's':
            if (value.empty()) {
                return t.keepEmpty ? L"" : (t.b ? t.b : L"");
            }
            return t.a;
        case 'c':
            // Options are "key\036ops", separated by \037.
            for (const std::wstring& option : Split(t.a, L'\037')) {
                size_t sep = option.find(L'\036');
                if (sep != std::wstring::npos && option.compare(0, sep, value) == 0) {
                    return option.substr(sep + 1);
                }
            }
            return L"";
    }
    return L"";
}

////////////////////////////////////////////////////////////////////////////////
// Window metrics and fonts, applied live in one call each

void ParseFont(const std::wstring& spec, LOGFONTW& lf) {
    std::vector<std::wstring> parts = Split(spec, L',');
    std::wstring face = Trim(parts[0]);
    if (!face.empty()) {
        wcsncpy_s(lf.lfFaceName, face.c_str(), _TRUNCATE);
    }
    if (parts.size() > 1) {
        int size = _wtoi(Trim(parts[1]).c_str());
        if (size > 0) {
            lf.lfHeight = -size;
            lf.lfWidth = 0;
        }
    }
    lf.lfWeight = FW_NORMAL;
    lf.lfItalic = FALSE;
    for (size_t i = 2; i < parts.size(); i++) {
        std::wstring flag = Trim(parts[i]);
        if (_wcsicmp(flag.c_str(), L"bold") == 0) {
            lf.lfWeight = FW_BOLD;
        } else if (_wcsicmp(flag.c_str(), L"italic") == 0) {
            lf.lfItalic = TRUE;
        }
    }
}

void ApplyMetrics(const std::map<std::wstring, std::wstring>& changes) {
    if (changes.empty()) {
        return;
    }
    // At 96 DPI, which is what the settings are in.
    DPI_AWARENESS_CONTEXT old = SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_UNAWARE);
    NONCLIENTMETRICSW ncm{sizeof(ncm)};
    ICONMETRICSW im{sizeof(im)};
    SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
    SystemParametersInfoW(SPI_GETICONMETRICS, sizeof(im), &im, 0);
    bool nonClient = false, icons = false;
    for (const auto& [field, value] : changes) {
        int n = _wtoi(value.c_str());
        if (field == L"metric.iconSpacingH") {
            im.iHorzSpacing = n;
            icons = true;
        } else if (field == L"metric.iconSpacingV") {
            im.iVertSpacing = n;
            icons = true;
        } else if (field == L"font.icon") {
            ParseFont(value, im.lfFont);
            icons = true;
        } else if (field == L"metric.menuHeight") {
            ncm.iMenuHeight = ncm.iMenuWidth = n;
            nonClient = true;
        } else if (field == L"metric.scrollWidth") {
            ncm.iScrollWidth = n;
            nonClient = true;
        } else if (field == L"metric.scrollHeight") {
            ncm.iScrollHeight = n;
            nonClient = true;
        } else if (field == L"metric.captionHeight") {
            ncm.iCaptionHeight = ncm.iCaptionWidth = n;
            nonClient = true;
        } else if (field == L"metric.borderWidth") {
            ncm.iBorderWidth = n;
            nonClient = true;
        } else if (field == L"metric.borderPadding") {
            ncm.iPaddedBorderWidth = n;
            nonClient = true;
        } else if (field == L"font.caption") {
            ParseFont(value, ncm.lfCaptionFont);
            nonClient = true;
        } else if (field == L"font.menu") {
            ParseFont(value, ncm.lfMenuFont);
            nonClient = true;
        } else if (field == L"font.message") {
            ParseFont(value, ncm.lfMessageFont);
            nonClient = true;
        } else if (field == L"font.status") {
            ParseFont(value, ncm.lfStatusFont);
            nonClient = true;
        }
    }
    if (icons && !SystemParametersInfoW(SPI_SETICONMETRICS, sizeof(im), &im,
                                        SPIF_UPDATEINIFILE | SPIF_SENDCHANGE)) {
        Wh_Log(L"SPI_SETICONMETRICS failed: %u", GetLastError());
    }
    if (nonClient && !SystemParametersInfoW(SPI_SETNONCLIENTMETRICS, sizeof(ncm), &ncm,
                                            SPIF_UPDATEINIFILE | SPIF_SENDCHANGE)) {
        Wh_Log(L"SPI_SETNONCLIENTMETRICS failed: %u", GetLastError());
    }
    SetThreadDpiAwarenessContext(old);
}

////////////////////////////////////////////////////////////////////////////////
// The elevated step

bool WriteFileText(const std::wstring& path, const std::string& bytes) {
    HANDLE file = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, 0, nullptr);
    if (file == INVALID_HANDLE_VALUE) {
        return false;
    }
    DWORD written;
    bool ok = WriteFile(file, bytes.data(), (DWORD)bytes.size(), &written, nullptr);
    CloseHandle(file);
    return ok;
}

std::string Utf8(const std::wstring& s) {
    int n = WideCharToMultiByte(CP_UTF8, 0, s.c_str(), (int)s.size(), nullptr, 0, nullptr, nullptr);
    std::string out(n, '\0');
    WideCharToMultiByte(CP_UTF8, 0, s.c_str(), (int)s.size(), out.data(), n, nullptr, nullptr);
    return out;
}

std::atomic<bool> g_stopping;

// Runs every machine-wide change in one elevated cmd.exe, one UAC prompt.
// Returns the tweaks whose part failed; all of them if the prompt was
// declined.
std::vector<size_t> RunElevated(const Pass& pass) {
    std::vector<size_t> ids;
    for (const auto& [index, text] : pass.regFiles) {
        ids.push_back(index);
    }
    for (const auto& [index, commands] : pass.adminCommands) {
        if (std::find(ids.begin(), ids.end(), index) == ids.end()) {
            ids.push_back(index);
        }
    }
    if (ids.empty()) {
        return {};
    }

    WCHAR temp[MAX_PATH];
    GetTempPathW(ARRAYSIZE(temp), temp);
    WCHAR stamp[32];
    swprintf_s(stamp, L"TourneTweaks-%llu", GetTickCount64());
    std::wstring dir = std::wstring(temp) + stamp;
    CreateDirectoryW(dir.c_str(), nullptr);
    std::wstring log = dir + L"\\log.txt";

    std::wstring script = L"@echo off\r\nchcp 65001 >nul\r\n";
    for (size_t index : ids) {
        script += L"echo [" + std::to_wstring(index) + L"]>>\"" + log + L"\"\r\n";
        auto reg = pass.regFiles.find(index);
        if (reg != pass.regFiles.end()) {
            std::wstring file = dir + L"\\t" + std::to_wstring(index) + L".reg";
            std::wstring content = L"\xFEFFWindows Registry Editor Version 5.00\r\n\r\n" + reg->second;
            WriteFileText(file, std::string((const char*)content.data(), content.size() * sizeof(WCHAR)));
            script += L"reg.exe import \"" + file + L"\" >>\"" + log + L"\" 2>&1 || echo FAILED>>\"" + log + L"\"\r\n";
        }
        auto commands = pass.adminCommands.find(index);
        if (commands != pass.adminCommands.end()) {
            for (const std::wstring& command : commands->second) {
                script += command + L" >>\"" + log + L"\" 2>&1 || echo FAILED>>\"" + log + L"\"\r\n";
            }
        }
    }
    std::wstring scriptPath = dir + L"\\apply.cmd";
    WriteFileText(scriptPath, Utf8(script));

    std::wstring params = L"/d /c \"\"" + scriptPath + L"\"\"";
    SHELLEXECUTEINFOW sei{sizeof(sei)};
    sei.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_NOASYNC;
    sei.lpVerb = L"runas";
    sei.lpFile = L"cmd.exe";
    sei.lpParameters = params.c_str();
    sei.nShow = SW_HIDE;
    if (!ShellExecuteExW(&sei) || !sei.hProcess) {
        Wh_Log(L"The admin step didn't run (%u); its changes apply when their settings change again",
               GetLastError());
        return ids;
    }
    // Polled, so unloading the suite never waits on a slow admin step.
    while (WaitForSingleObject(sei.hProcess, 200) == WAIT_TIMEOUT) {
        if (g_stopping) {
            CloseHandle(sei.hProcess);
            return ids;
        }
    }
    CloseHandle(sei.hProcess);

    // Which sections logged a failure.
    std::vector<size_t> failed;
    HANDLE file = CreateFileW(log.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    if (file != INVALID_HANDLE_VALUE) {
        std::string text(GetFileSize(file, nullptr), '\0');
        DWORD read = 0;
        ReadFile(file, text.data(), (DWORD)text.size(), &read, nullptr);
        CloseHandle(file);
        long long section = -1;
        size_t start = 0;
        while (start < text.size()) {
            size_t end = text.find('\n', start);
            std::string line = text.substr(start, end == std::string::npos ? std::string::npos : end - start);
            start = end == std::string::npos ? text.size() : end + 1;
            if (!line.empty() && line[0] == '[') {
                section = atoll(line.c_str() + 1);
                if (section < 0 || section >= (long long)ARRAYSIZE(kTweaks)) {
                    section = -1;
                }
            } else if (line.rfind("FAILED", 0) == 0 && section >= 0) {
                failed.push_back((size_t)section);
                Wh_Log(L"%s: the admin step reported a failure", kTweaks[section].id);
            } else if (!line.empty() && line != "\r" && section >= 0) {
                Wh_Log(L"%s: %hs", kTweaks[section].id, line.c_str());
            }
        }
    }
    return failed;
}

////////////////////////////////////////////////////////////////////////////////
// Explorer

void RestartExplorer() {
    HWND tray = FindWindowW(L"Shell_TrayWnd", nullptr);
    DWORD pid = 0;
    if (!tray || !GetWindowThreadProcessId(tray, &pid)) {
        return;
    }
    HANDLE process = OpenProcess(PROCESS_TERMINATE | SYNCHRONIZE, FALSE, pid);
    if (!process) {
        return;
    }
    TerminateProcess(process, 1);
    WaitForSingleObject(process, 5000);
    CloseHandle(process);
    // Windows restarts the shell by itself; start it if it hasn't.
    for (int i = 0; i < 30 && !FindWindowW(L"Shell_TrayWnd", nullptr); i++) {
        Sleep(100);
    }
    if (!FindWindowW(L"Shell_TrayWnd", nullptr)) {
        RunHidden(L"%SystemRoot%\\explorer.exe", 0);
    }
}

////////////////////////////////////////////////////////////////////////////////
// Applying

// Asks before applying: always at start (nothing the user just did), and on
// a Save that changes more than a few tweaks at once. Yes applies them; No
// leaves Windows as it is and takes the settings as they are, so they're
// only applied when changed again; Cancel (or the suite stopping meanwhile)
// leaves them pending.
constexpr size_t kAskAbove = 5;
bool g_askAgain;  // the last box was answered Cancel (only the apply thread uses it)

BOOL CALLBACK CancelWindow(HWND wnd, LPARAM) {
    PostMessageW(wnd, WM_COMMAND, IDCANCEL, 0);
    return TRUE;
}

// Runs inside the message box's loop: closes it when the suite stops.
VOID CALLBACK CloseAskOnStop(HWND, UINT, UINT_PTR, DWORD) {
    if (g_stopping) {
        EnumThreadWindows(GetCurrentThreadId(), CancelWindow, 0);
    }
}

int AskToApply(const std::vector<std::wstring>& lines, bool atStart) {
    std::wstring text = atStart ? L"These Tweaks settings were changed while the suite wasn't running, and aren't applied yet:\n\n"
                                : L"This Save changes " + std::to_wstring(lines.size()) + L" Tweaks settings:\n\n";
    for (size_t i = 0; i < lines.size(); i++) {
        if (i == 15) {
            text += L"and " + std::to_wstring(lines.size() - i) + L" more\n";
            break;
        }
        text += L"  \x2022 " + lines[i] + L"\n";
    }
    text += L"\nApply them to Windows now?\n\nNo leaves Windows as it is: these settings are then only applied once you change them again. Cancel decides later: they come up again at the next start or Save.";
    UINT_PTR timer = SetTimer(nullptr, 0, 200, CloseAskOnStop);
    int answer = MessageBoxW(nullptr, text.c_str(), L"Tourne'Suite Tweaks",
                             MB_YESNOCANCEL | MB_DEFBUTTON3 | MB_ICONQUESTION | MB_TOPMOST | MB_SETFOREGROUND);
    KillTimer(nullptr, timer);
    return g_stopping ? IDCANCEL : answer;
}

// Applies every tweak whose setting changed since it was last applied, in
// the groups switched on. Nothing is applied the first time a tweak is seen
// (its setting may not match Windows yet), nor when its group is switched
// on: both only note how it's set. A group switched off forgets its tweaks,
// so switching it back on starts afresh. At start, tweaks that need admin
// wait for a Save, so the admin prompt only ever follows something the user
// did; whatever its outcome, nothing is retried until changed again.
void ApplyChanges(bool atStart) {
    Pass pass;
    struct Pending {
        size_t index;
        std::wstring hash;
        std::wstring value;
    };
    std::vector<Pending> pending;
    bool explorer = false;

    for (size_t i = 0; i < ARRAYSIZE(kTweaks); i++) {
        const Tweak& t = kTweaks[i];
        std::wstring stored = StoredHash(t);
        if (!GroupOn(t)) {
            if (!stored.empty()) {
                StoreHash(t, L"");
            }
            continue;
        }
        std::wstring hash = Hash(Signature(t));
        if (stored.empty()) {
            StoreHash(t, hash);
            continue;
        }
        if (stored == hash) {
            continue;
        }
        if (atStart && t.admin) {
            Wh_Log(L"%s changed while the suite wasn't running; it needs admin, so it waits for a Save", t.id);
            continue;
        }
        pending.push_back({i, hash, SettingValue(t)});
    }
    if (pending.empty()) {
        return;
    }

    // Tweaks put off with Cancel are asked about again, however few.
    if (atStart || g_askAgain || pending.size() > kAskAbove) {
        std::vector<std::wstring> lines;
        for (const Pending& p : pending) {
            const Tweak& t = kTweaks[p.index];
            std::wstring value = t.kind == 't' ? (p.value == L"0" ? L"off" : L"on")
                                 : p.value.empty() ? L"(empty)" : p.value;
            lines.push_back(std::wstring(t.title) + L": " + value);
        }
        int answer = AskToApply(lines, atStart);
        if (answer == IDCANCEL) {
            Wh_Log(L"Applying %zu tweaks put off until the next Save", pending.size());
            g_askAgain = true;
            return;
        }
        g_askAgain = false;
        if (answer != IDYES) {
            Wh_Log(L"Not applying %zu tweaks; taken as they are", pending.size());
            for (const Pending& p : pending) {
                StoreHash(kTweaks[p.index], p.hash);
            }
            return;
        }
    }

    for (const Pending& p : pending) {
        const Tweak& t = kTweaks[p.index];
        Wh_Log(L"Applying %s = %s", t.id, p.value.c_str());
        pass.current = p.index;
        for (const std::wstring& line : Split(OpsFor(t, p.value), L'\n')) {
            DoOp(pass, Substitute(line, p.value));
        }
        explorer |= t.after == 'e';
    }

    ApplyMetrics(pass.metrics);
    // A declined prompt or a failed admin step is logged, not retried: the
    // tweak is taken as applied until its setting changes again.
    std::vector<size_t> failed = RunElevated(pass);
    for (const Pending& p : pending) {
        // Cut short by the suite stopping: whether the admin part ran isn't
        // known, so those tweaks stay pending.
        if (g_stopping && kTweaks[p.index].admin) {
            continue;
        }
        if (std::find(failed.begin(), failed.end(), p.index) != failed.end()) {
            Wh_Log(L"%s: the admin part didn't apply; change the setting again to retry", kTweaks[p.index].id);
        }
        StoreHash(kTweaks[p.index], p.hash);
    }

    for (const std::wstring& command : pass.afterRuns) {
        RunHidden(command, 30000);
    }
    if (pass.iconsChanged) {
        RunHidden(L"%SystemRoot%\\System32\\ie4uinit.exe -show", 30000);
    }
    std::sort(pass.notify.begin(), pass.notify.end());
    pass.notify.erase(std::unique(pass.notify.begin(), pass.notify.end()), pass.notify.end());
    for (const std::wstring& area : pass.notify) {
        DWORD_PTR result;
        SendMessageTimeoutW(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM)area.c_str(),
                            SMTO_ABORTIFHUNG, 2000, &result);
    }
    if (pass.shellChanged || pass.iconsChanged) {
        SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);
    }
    if (explorer && Wh_GetIntSetting(L"general.restartExplorer")) {
        RestartExplorer();
    }
}

// Applying runs on its own thread: the admin prompt can wait on the user.
HANDLE g_applyThread;
std::atomic<bool> g_applyAgain;
std::atomic<bool> g_applyRunning;
std::atomic<bool> g_applyAtStart;

DWORD WINAPI ApplyThread(LPVOID) {
    for (;;) {
        do {
            g_applyAgain = false;
            ApplyChanges(g_applyAtStart.exchange(false));  // (a pass asked for meanwhile is a Save's)
        } while (g_applyAgain && !g_stopping);
        g_applyRunning = false;
        // A Save that came in just as this pass ended: run it, unless Apply
        // has already started a thread for it.
        if (!g_applyAgain || g_stopping || g_applyRunning.exchange(true)) {
            return 0;
        }
    }
}

// Called in the launcher at start (atStart) and after each settings save.
void Apply(bool atStart) {
    g_applyAgain = true;  // (before the check, so a pass that is just ending sees it)
    if (g_applyRunning.exchange(true)) {
        return;
    }
    g_applyAtStart = atStart;
    if (g_applyThread) {
        CloseHandle(g_applyThread);
    }
    g_applyThread = CreateThread(nullptr, 0, ApplyThread, nullptr, 0, nullptr);
    if (!g_applyThread) {
        g_applyRunning = false;
    }
}

// The thread can be inside the UAC prompt, which only the user ends; give
// them a while before Windhawk unloads the code under it.
void Stop() {
    g_stopping = true;
    if (g_applyThread) {
        WaitForSingleObject(g_applyThread, 120000);
        CloseHandle(g_applyThread);
        g_applyThread = nullptr;
    }
}

////////////////////////////////////////////////////////////////////////////////
// Live system font: swaps Segoe UI for the chosen font in programs that
// create fonts through GDI, so the change shows before signing out.

WCHAR g_liveFont[LF_FACESIZE];

using CreateFontIndirectExW_t = HFONT(WINAPI*)(const ENUMLOGFONTEXDVW*);
CreateFontIndirectExW_t CreateFontIndirectExW_Original;

bool IsSegoeUi(PCWSTR face) {
    return _wcsicmp(face, L"Segoe UI") == 0 || _wcsnicmp(face, L"Segoe UI Variable", 17) == 0;
}

HFONT WINAPI CreateFontIndirectExW_Hook(const ENUMLOGFONTEXDVW* font) {
    if (font && IsSegoeUi(font->elfEnumLogfontEx.elfLogFont.lfFaceName)) {
        ENUMLOGFONTEXDVW copy = *font;
        wcsncpy_s(copy.elfEnumLogfontEx.elfLogFont.lfFaceName, g_liveFont, _TRUNCATE);
        return CreateFontIndirectExW_Original(&copy);
    }
    return CreateFontIndirectExW_Original(font);
}

std::wstring LiveFontSetting() {
    if (!Wh_GetIntSetting(L"general.liveFontSwap")) {
        return L"";
    }
    PCWSTR font = Wh_GetStringSetting(L"metrics.systemFont");
    std::wstring face = font ? Trim(font) : L"";
    Wh_FreeStringSetting(font);
    return face;
}

// Everywhere but the launcher, the part only exists for the live font.
bool ShouldLoadInThisProcess() {
    return !LiveFontSetting().empty();
}

bool NeedsReload() {
    return LiveFontSetting() != g_liveFont;
}

BOOL Wh_ModInit() {
    std::wstring face = LiveFontSetting();
    if (face.empty()) {
        return TRUE;
    }
    wcsncpy_s(g_liveFont, face.c_str(), _TRUNCATE);
    HMODULE gdi = GetModuleHandleW(L"gdi32full.dll");
    if (!gdi) {
        gdi = LoadLibraryW(L"gdi32.dll");
    }
    void* target = gdi ? (void*)GetProcAddress(gdi, "CreateFontIndirectExW") : nullptr;
    if (!target) {
        return FALSE;
    }
    return Wh_SetFunctionHook(target, (void*)CreateFontIndirectExW_Hook,
                              (void**)&CreateFontIndirectExW_Original);
}

void Wh_ModUninit() {}

void Wh_ModSettingsChanged() {}
