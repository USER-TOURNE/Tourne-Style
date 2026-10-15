# Wallpaper Depth

Wallpaper Depth generates a foreground mask for each image wallpaper, allowing
Tourne Desktop's desktop widgets to pass behind nearby scenery. Ported from
Noctalia's Wallpaper Depth plugin (MIT).

Depth estimation runs locally with
[Depth Anything V2 Small](https://huggingface.co/onnx-community/depth-anything-v2-small).
Wallpapers are never uploaded.

## Plugin

| Field | Value |
| --- | --- |
| ID | `tourne/wallpaper-depth` |
| Entries | Service: `service`; panel: `manager`; bar widget: `bar` |

## Requirements

Install Python 3.11 to 3.14 for Windows (python.org, the Microsoft Store, or
`winget install Python.Python.3.13`). The plugin uses the `py` launcher when
there is one, else `python` on the PATH; the advanced **Python** setting takes
a full path to `python.exe` instead. Initial setup needs network access.

From the plugin panel, setup creates an isolated Python environment in the
plugin data folder (`runtime\.venv`) and downloads the 99 MB Apache-2.0 Depth
Anything V2 Small ONNX model from Hugging Face. The environment contains pinned
versions of NumPy, ONNX Runtime, and Pillow (all have Windows wheels, x64 and
ARM64); it does not modify the system Python.

## Usage

Add the **Wallpaper Depth** bar widget: left click opens the panel, right click
opens Studio's plugin settings.

1. Open the **Wallpaper Depth** panel.
2. Select **Install model** and wait for setup to finish. Setup stops if Tourne
   Desktop or the plugin restarts; select it again to carry on.
3. Set an image wallpaper.
4. Select **Generate masks**, or leave **Generate automatically** on.
5. Adjust **Foreground threshold** and **Edge feather** in the plugin settings
   when the default mask does not match the scene.

The panel reports generation state for every screen. **Clear cache** removes
saved depth maps and masks; the plugin regenerates them when needed.

## Settings

| Setting | Type | Default | Description |
| --- | --- | --- | --- |
| `auto_generate` | `bool` | `true` | Regenerates masks when a wallpaper or mask parameter changes. |
| `threshold` | `int` | `30` | Normalized depth cutoff from 0 to 100. Lower values place more of the scene in front of desktop widgets. |
| `feather` | `int` | `8` | Soft-transition width around the depth cutoff, from 0 to 50. |
| `python` | `string` | `""` | `python.exe`'s full path; empty uses `py` or `python` from the PATH. |

## How it works

The service processes each screen independently. It preserves the wallpaper's
aspect ratio during inference, normalizes the relative depth prediction, and
refines it against the source image before applying the configured threshold
and feather. The resulting mask is restored to the wallpaper's original
resolution and registered with Tourne Desktop for that screen.

Depth predictions are cached separately from masks, so changing threshold or
feather can reuse the expensive model result. Cache entries are keyed by the
wallpaper contents, model revision, and processing version, and old entries are
pruned automatically.

## Licensing and privacy

The plugin is MIT-licensed. Depth Anything V2 Small is downloaded at setup time
under the Apache-2.0 license. Model inference, depth caches, and generated masks
remain in the local plugin data folder; only setup contacts the internet (PyPI
for the packages, Hugging Face for the model).
