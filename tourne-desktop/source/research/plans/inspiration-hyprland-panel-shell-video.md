# Inspiration: Hyprland panel shell video (shared by me, 2026-10-05)

My words: "This is also gorgeous". Decision 2026-10-05: build it after the plan (with the extras), not before views. An 85 second screen recording of a Hyprland rice (a "Submit to r/unixporn" card is in its bar). The author and licence aren't shown, so it is ideas only, like CKFlip3D.

Frames: screens/inspiration-panel-shell-light.png and screens/inspiration-panel-shell-dark.png.

## What's in it

- **One look across everything:** monospace type, chunky rounded cards with a thin outline, flat fills. A cream light side and a near-black dark side, switched live.
- **Bar as islands:** separate pills for launcher, workspaces (pac-man ghost icons), media, clock, weather, CPU/RAM, and system icons. It moves between top, bottom, left and right live.
- **Control panel from the bar**, in three columns:
  - power column: Application, Settings, Sleep, Lock, Logout, Restart, Shutdown;
  - settings sections: General, Appearance, Network, Audio, Performance, Shortcuts, System;
  - the page itself, with an icon tab row along its top.
- **Appearance:**
  - light/dark plus preset themes (Auto from wallpaper, Macchiato, Gruvbox, Tokyo Night, Nord);
  - bar layout picker drawn as small bar previews;
  - workspace count + / -;
  - panel position Top/Bottom/Left/Right as picture buttons;
  - an "enable border" switch;
  - wallpaper grid with Apply on each;
  - workspace icon style grid;
  - CPU and RAM widget styles (Style 1 to 8).
- **General:** language grid with flags; country flag picker.
- **System:** hardware info page.
- **Pop-outs from bar islands:** RAM manager (RAM/swap bars and a process list), Wi-Fi, Bluetooth, music player.
- **Dashboard:** profile, date, weather, uptime, quick buttons, folders, socials.
- **Desktop clock:** big two-line clock (03 / 06, Tuesday, 18 August 2026), bottom right, on the wallpaper.
- **Lock screen:** blurred wallpaper with soft bubbles and a centred clock.
- **Motion:** panels slide and fade from the bar, and theme changes cross-fade.

## Where it lands in Tourne

| In the video | Tourne today | Gap |
|---|---|---|
| Island bar, live position | CSS bar (YASB-style), bar edge setting | island preset, live move |
| Theme presets + Auto from wallpaper | Presets page (828), Tourne theme from wallpaper | none, look only |
| Bar layout picker with previews | Presets tiles | bar-layout tiles |
| Wallpaper grid | wallpaper manager proposal | build it |
| Wi-Fi / Bluetooth / volume pop-outs | System Icons flyouts | restyle to match |
| RAM / process manager, music, weather | partly (CSS bar widgets) | new pop-outs |
| Control panel from the bar | Studio is a separate window | a bar-docked panel |
| Desktop clock, dashboard | none | desktop widgets |
| Lock screen | Windows' own | ideas only (Windows owns the lock screen) |
