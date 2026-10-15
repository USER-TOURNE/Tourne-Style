# Screen Toolkit

Color picker, OCR, QR/barcode scanning, palette extraction, Google Lens search,
annotation, pixel measuring, screen recording and image sharing, for Tourne
Desktop. Built into Tourne Desktop: install it from the Studio's Extensions page
like any plugin.

Ported from alexander's Screen Toolkit for Noctalia v5 (`alexander/screen-toolkit`,
MIT), itself a remake of the legacy Noctalia v4 screen-toolkit plugin. The panels,
tools, settings and translations are the original's; what changed is how each tool
reaches Windows.

## Programs it uses

Install the ones for the tools you want. A missing one is named when its tool starts.

| Tool | Program | Install |
| --- | --- | --- |
| OCR | `tesseract` and `magick` | `winget install UB-Mannheim.TesseractOCR` (found in `C:\Program Files\Tesseract-OCR` even off the PATH), `winget install ImageMagick.ImageMagick` |
| QR / barcode | `zbarimg` | ZBar's Windows build (found in `C:\Program Files (x86)\ZBar\bin` even off the PATH) |
| Palette | `magick` | as above |
| Lens, Share | `curl` | built into Windows |
| Recording | `ffmpeg`, `ffprobe` | `winget install Gyan.FFmpeg` (Desktop Duplication needs ffmpeg 6 or newer) |
| Markup | Paint (`mspaint`) | built into Windows; the Markup Editor setting picks another |
| Translate | `trans` (optional) | without it, the text opens in Google Translate |

Captures, the window picker and the color picker use Tourne's own screen picker:
no program needed.

## How each tool works on Windows

- **Color**: the picker's magnifier; click a pixel. The hex code is copied.
- **OCR / QR / Palette / Lens**: drag a region. The picture is kept only in the
  plugin's data folder, so these tools leave no screenshot behind.
- **Measure**: drag a region; its size in pixels is copied.
- **Markup / Markup FS / Window**: the capture is saved in the Screenshot Path
  folder (`~/Pictures/Screenshots`) as `Screenshot 2026-10-13 141502.png` and
  opens in Paint (or the Markup Editor setting's program). Save in the editor to
  keep your drawing. Window dims everything but the window under the pointer;
  click to take it.
- **Recording**: ffmpeg records the screen with the focused window (fullscreen)
  or a dragged rectangle (region), through Desktop Duplication (ddagrab) or GDI
  (gdigrab), with the PC's sound (Record System Audio) and a microphone (Record
  Microphone plus the Microphone setting, a DirectShow device name; empty takes
  the first one ffmpeg lists). Stop it from the bar widget's panel. The result
  panel then offers Save (MP4 or MOV, or GIF for GIF recordings) and Discard.
  Recording Codec and Encoder choose the encoder: CPU (libx264, libx265,
  libsvtav1), NVIDIA (NVENC), AMD (AMF) or Intel (Quick Sync).
- **Share**: uploads to uguu.se (anonymous, 3 hours, 128 MB), or to up.x02.me
  with an API key, through curl; the link is copied.

## Commands

The original's IPC commands work through Tourne Desktop, for a hotkey tool or a
shortcut:

```powershell
& "$env:LOCALAPPDATA\Tourne\Desktop\bin\TourneDesktop.exe" --msg plugin tourne/screen-toolkit:service all toggle
& "$env:LOCALAPPDATA\Tourne\Desktop\bin\TourneDesktop.exe" --msg plugin tourne/screen-toolkit:service all ocr
& "$env:LOCALAPPDATA\Tourne\Desktop\bin\TourneDesktop.exe" --msg plugin tourne/screen-toolkit:service all share C:\path\to\image.png
```

(The same words as Linux's `noctalia msg plugin alexander/screen-toolkit:service all ...`.)

The full list (colorPicker, qr, palette, lens, measure, annotate,
annotateFullscreen, annotateWindow, record, recordMp4, recordFullscreen,
recordFullscreenMp4, recordStop, recordSave, recordCopy, recordDiscard,
ocrSearch, ocrTranslate, clearResult, clearHistory, setCursorHidden) is the
original's.

## Not the same as on Linux

- The control-center shortcut tile doesn't run in Tourne Desktop yet.
- The recording preview plays in the default video player instead of mpv.
