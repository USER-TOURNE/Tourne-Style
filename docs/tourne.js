// Pixel wordmarks: 5x7 letters drawn like the blocks in Tourne'Table's header (13 px blocks on a 16 px pitch).
const PIXEL_FONT = {
  A: ['01110','10001','10001','11111','10001','10001','10001'], C: ['01111','10000','10000','10000','10000','10000','01111'],
  E: ['11111','10000','10000','11110','10000','10000','11111'], I: ['11111','00100','00100','00100','00100','00100','11111'],
  K: ['10001','10010','10100','11000','10100','10010','10001'], L: ['10000','10000','10000','10000','10000','10000','11111'],
  N: ['10001','11001','10101','10011','10001','10001','10001'], O: ['01110','10001','10001','10001','10001','10001','01110'],
  P: ['11110','10001','10001','11110','10000','10000','10000'], R: ['11110','10001','10001','11110','10100','10010','10001'],
  S: ['01111','10000','10000','01110','00001','00001','11110'],
  T: ['11111','00100','00100','00100','00100','00100','00100'], U: ['10001','10001','10001','10001','10001','10001','01110'], Y: ['10001','10001','01010','00100','00100','00100','00100'],
};
function pixelWord(svg, word) {
  const P = 16, Q = 13, W = word.length * 6 * P - P;
  let r = '';
  [...word].forEach((ch, n) => (PIXEL_FONT[ch] || []).forEach((row, y) => [...row].forEach((bit, x) => {
    if (bit === '1') r += `<rect x="${n * 6 * P + x * P}" y="${y * P}" width="${Q}" height="${Q}" fill="#15ffa2"/>`;
  })));
  svg.setAttribute('viewBox', `0 0 ${W} ${7 * P + 6}`);
  svg.innerHTML = r + `<rect x="0" y="${7 * P + 2}" width="${W - (P - Q)}" height="4" fill="#a95d5d"/>`;
}

// Tourne's mark (the Bonsai living wallpaper's TOURNE logo, its letters' outline), once, in the header's top right
// corner beside the menu (no preview, button or box is ever there): drawn by the scanlines themselves, green where
// they cross the letters and a row thicker. Made once; nothing runs after.
const BONSAI_MARK = { w: 119, rows: ['7fffc07fe0380383ffc01c00707fff','7fffe07fe0380387ffc01c0070dfff','7fffe1fff8380387fff01f0070ffff','3fffe1fffc380387fff01f0070fffc','00f001c03c38038700701f0070f000','00f001c01c38038700701fe070f000','00f001c01c38038700701fe070f000','00f001c01c38038700701de070f000','00f001c01c38038700701de070ffe0','00f001c01c38038700701de070ffe0','00f001c01c380387fff01df870ffe0','00f001c01c380387ffe01df870ff80','00f001c01c380387ffc01c3870f000','00f001c01c380387fff01c3870f000','00f001c01c38038700701c3870f000','00f001c01c38038700701c3e70f000','00f001c01c38038700701c3f70f000','00f001c01c38038700701c0f70f000','00f001c01c38038700701c0f70f000','00f001c01c38038700701c0f70f000','00f001c03c38038700701c0ff0f000','00f000fffc1fff8700701c0ff0ffff','00f0007ff00ffc0700701c03f0ffff','00f0007ff007fc0700701c03f0fffe'] };
function weaveMark() {
  if (document.querySelector('.weave')) return;
  const header = document.querySelector('header');
  if (!header) return;
  // Whole pixels: 2 to a mark pixel where there's room right of the menu, 1 where there's less; with none (a very
  // narrow window), the footer's bottom right instead.
  const nav = header.querySelector('nav'), hr = header.getBoundingClientRect();
  const navRight = nav ? Math.max(...[...nav.children].map(a => a.getBoundingClientRect().right)) - hr.left : 0;
  const free = header.clientWidth - 16 - navRight - 24;
  const S = free >= BONSAI_MARK.w * 2 ? 2 : free >= BONSAI_MARK.w ? 1 : 0, P = 3;
  const host = S ? header : document.querySelector('footer');
  if (!host) return;
  const s = S || 1;
  const W = BONSAI_MARK.w * s, H = BONSAI_MARK.rows.length * s;
  const c = document.createElement('canvas'); c.width = W; c.height = H;
  const x = c.getContext('2d'), img = x.createImageData(W, H), d = img.data;
  const bits = BONSAI_MARK.rows.map(hex => BigInt('0x' + hex).toString(2).padStart(BONSAI_MARK.w, '0'));
  for (let y = 0; y < H; y++) {
    const line = y % P === 0, thick = y % P === 1;  // (the scanlines: green where they cross the letters, a row thicker)
    if (!line && !thick) continue;
    const row = bits[Math.min(bits.length - 1, Math.floor(y / s))];
    for (let px = 0; px < W; px++) {
      if (row[Math.floor(px / s)] !== '1') continue;
      const i = (y * W + px) * 4;
      d[i] = 21; d[i + 1] = 255; d[i + 2] = 162; d[i + 3] = line ? 120 : 55;
    }
  }
  x.putImageData(img, 0, 0);
  c.className = 'weave'; c.setAttribute('aria-hidden', 'true'); c.style.width = W + 'px'; c.style.height = H + 'px';
  if (!S) c.classList.add('foot');
  host.appendChild(c);
}
// (after the type has loaded: the menu's width decides the mark's size and place)
const weaveWhenReady = () => (document.fonts ? document.fonts.ready : Promise.resolve()).then(weaveMark);
if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', weaveWhenReady); else weaveWhenReady();
