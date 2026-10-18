# Inspiration: melting liquid glass video

*October 5, 2026.*

Same with this one, it's gorgeous. It's a 107 second compositor shader demo (a terminal in it shows a branch named feat/custom-blur-shader). The author and licence aren't shown, so I'm only taking ideas from it.

Frame: screens/inspiration-melting-glass.png.

## What's in it
- **Thick glass windows** over an animated ocean wallpaper. Each window refracts what's behind it, most strongly at its rounded edges, with a bright rim. The bending is clearer than the blur.
- **Free glass shapes** (pills, spheres, rounded boxes, a melting blob) float over the desktop. Where two touch they melt together like metaballs.
- **A 3D audio visualizer**: red spikes across the bottom of the screen pulse with the music, and the glass refracts them too.
- Everything refracts live content (a video playing behind glass), not just the wallpaper.

## Where it lands in Tourne
- My glass engine (desktop/host/glass-engine.cpp) already does edge bend (refraction), colour fringe, edge glow, highlight, bevel and lens on signed-distance rounded shapes.
- New:
  - shapes that melt together: a smooth union of distance fields, which is cheap;
  - spheres and free shapes as desktop decorations;
  - glass over live windows rather than only the wallpaper (this was already an open question);
  - an audio visualizer layer.
- Where it goes in the plan: section 5 (effects and shaders), with the visualizer as an extra or an extension.
