// Records a style's live preview from the styles page (docs/index.html) as a GIF, for the Discord post.
// The page draws each preview from a time value, so the frames are stepped by hand at a fixed rate instead of
// filmed in real time: the GIF comes out the same on any machine.
// Usage: node community/tools/preview_gif.mjs <port> <out dir> <file.ttstyle>...
// (the repo root must be served on <port>; writes <out dir>/<file>.gif for each style it finds)
import { chromium } from 'playwright';
import { execFileSync } from 'node:child_process';
import { mkdirSync, rmSync, writeFileSync } from 'node:fs';
import { join } from 'node:path';

const [port, outDir, ...files] = process.argv.slice(2);
const FPS = 25, SECONDS = 4;

const browser = await chromium.launch();
// Reduced motion keeps the page's own animation loop still, so only the frames drawn here land on the canvas.
const page = await browser.newPage({ viewport: { width: 560, height: 900 }, deviceScaleFactor: 1, reducedMotion: 'reduce' });
await page.goto(`http://127.0.0.1:${port}/docs/index.html?local`);
await page.waitForFunction(() => document.getElementById('state').textContent !== '[ LOADING ]', null, { timeout: 30000 });

for (const file of files) {
  const index = await page.evaluate(f => cards.findIndex(c => c.el.querySelector('.btn[download]')?.download === f), file);
  if (index < 0) { console.log(`${file}: not on the page, skipped`); continue; }
  const dir = join(outDir, file + '.frames');
  rmSync(dir, { recursive: true, force: true }); mkdirSync(dir, { recursive: true });
  await page.evaluate(i => { cards[i].el.scrollIntoView(); cards[i].sim = null; }, index);
  for (let k = 0; k < FPS * SECONDS; k++) {
    const png = await page.evaluate(([i, t]) => {
      // The card's dark background is CSS, not drawn, so the frame goes onto a copy of it.
      const cv = cards[i].cv; draw(cards[i], t);
      const out = document.createElement('canvas'); out.width = cv.width; out.height = cv.height;
      const ctx = out.getContext('2d'); ctx.fillStyle = getComputedStyle(cv).backgroundColor; ctx.fillRect(0, 0, out.width, out.height);
      ctx.drawImage(cv, 0, 0); return out.toDataURL('image/png');
    }, [index, 1 + k / FPS]);
    writeFileSync(join(dir, String(k).padStart(4, '0') + '.png'), Buffer.from(png.split(',')[1], 'base64'));
  }
  const gif = join(outDir, file + '.gif');
  execFileSync('ffmpeg', ['-loglevel', 'error', '-y', '-framerate', String(FPS), '-i', join(dir, '%04d.png'),
    '-filter_complex', 'split[a][b];[a]palettegen=reserve_transparent=0:stats_mode=diff[p];[b][p]paletteuse=dither=bayer:bayer_scale=4',
    '-loop', '0', gif]);
  rmSync(dir, { recursive: true, force: true });
  console.log(`${file}: ${gif}`);
}
await browser.close();
