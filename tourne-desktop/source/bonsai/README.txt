Bonsai: a cbonsai screensaver and terminal tree for Windows
===========================================================

Building (nothing to install; Windows' own C# compiler does it)
  Double-click build.bat. It makes:
    BonsaiSaver.scr   the screensaver, with the Scientifica font built in
    bonsai.exe        the terminal version
  If it fails, build.log says why.

The screensaver
  Right-click BonsaiSaver.scr: Test to watch it, Install to make it yours, Configure for its settings.
  Keep it in this folder: Windows runs a screensaver from wherever it's installed.
  Settings: Detailed or Classic cbonsai style, life, multiplier, branch density, canopy size, pot,
  leaf characters, a message, seconds to grow and to stay, fill the screen (and how much) or a fixed
  zoom, every screen or the main one only, and every colour (swatch or hex, with Reset colours).
  Logo: a picture in the top right corner while it plays (a video opener, say). Put it beside the
  .scr as logo.png, or pick any file with "..." under Logo. Width is a share of the screen's (0 hides
  it). "White see-through" turns a white background clear, soft shadows and glows included.
  Default colours: trunk #a95d5d lit #ff8f8f, leaves #64aa89 lit #15ffa2 shadow #6ba6a0,
  pot #6ba6a0, background #121c21.

The terminal version
  build.bat also copies bonsai.exe to your WindowsApps folder (already on your PATH), so it runs
  from any terminal by name:
  bonsai               grow trees forever (any key stops it)
  bonsai -l -m "hi"    one tree with a message
  bonsai -p            print a tree and exit
  bonsai -S classic    cbonsai's own look
  bonsai -h            everything else
  It uses the screensaver's colours and settings. For Scientifica in the terminal too, double-click
  scientifica.ttf, Install, then pick "scientifica" as your terminal profile's font.

Files
  BonsaiCore.cs     the tree: cbonsai's growth (Classic) and the Detailed style, settings, colours
  BonsaiSaver.cs    the screensaver and its settings window
  BonsaiTui.cs      the terminal version
  scientifica.ttf   Scientifica by Akshay Oppiliappan, SIL Open Font License (scientifica-LICENSE.txt)
