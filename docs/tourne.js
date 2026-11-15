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

// Tourne's mark (the Bonsai living wallpaper's TOURNE logo, its letters' outline) woven into the scanlines: across a
// tile made once, the scanlines turn green where they cross the mark, a row thicker, and the tile repeats at an
// angle with staggered rows over the whole page (content included; clicks pass through). Nothing runs after.
const BONSAI_MARK = { w: 119, rows: ['7fffc07fe0380383ffc01c00707fff','7fffe07fe0380387ffc01c0070dfff','7fffe1fff8380387fff01f0070ffff','3fffe1fffc380387fff01f0070fffc','00f001c03c38038700701f0070f000','00f001c01c38038700701fe070f000','00f001c01c38038700701fe070f000','00f001c01c38038700701de070f000','00f001c01c38038700701de070ffe0','00f001c01c38038700701de070ffe0','00f001c01c380387fff01df870ffe0','00f001c01c380387ffe01df870ff80','00f001c01c380387ffc01c3870f000','00f001c01c380387fff01c3870f000','00f001c01c38038700701c3870f000','00f001c01c38038700701c3e70f000','00f001c01c38038700701c3f70f000','00f001c01c38038700701c0f70f000','00f001c01c38038700701c0f70f000','00f001c01c38038700701c0f70f000','00f001c03c38038700701c0ff0f000','00f000fffc1fff8700701c0ff0ffff','00f0007ff00ffc0700701c03f0ffff','00f0007ff007fc0700701c03f0fffe'] };
function weaveMark() {
  if (document.querySelector('.weave')) return;
  const TW = 432, TH = 252, S = 2, P = 3;  // tile, mark pixel size, scanline pitch
  const mark = document.createElement('canvas');
  mark.width = BONSAI_MARK.w * S; mark.height = BONSAI_MARK.rows.length * S;
  const mc = mark.getContext('2d'); mc.fillStyle = '#000';
  BONSAI_MARK.rows.forEach((hex, y) => {
    const bits = BigInt('0x' + hex).toString(2).padStart(BONSAI_MARK.w, '0');
    for (let x = 0; x < BONSAI_MARK.w; x++) if (bits[x] === '1') mc.fillRect(x * S, y * S, S, S);
  });
  const shape = document.createElement('canvas'); shape.width = TW; shape.height = TH;
  const sc = shape.getContext('2d');
  const put = (cx, cy) => { for (const dx of [-TW, 0, TW]) for (const dy of [-TH, 0, TH]) {
    sc.save(); sc.translate(cx + dx, cy + dy); sc.rotate(-14 * Math.PI / 180); sc.drawImage(mark, -mark.width / 2, -mark.height / 2); sc.restore(); } };
  put(TW * 0.27, TH * 0.28); put(TW * 0.77, TH * 0.78);  // (staggered rows)
  const inside = sc.getImageData(0, 0, TW, TH).data;
  const tile = document.createElement('canvas'); tile.width = TW; tile.height = TH;
  const tc = tile.getContext('2d'), img = tc.createImageData(TW, TH), d = img.data;
  for (let y = 0; y < TH; y++) {
    const line = y % P === 0, thick = y % P === 1;
    if (!line && !thick) continue;
    for (let x = 0; x < TW; x++) {
      const i = (y * TW + x) * 4, on = inside[i + 3] > 127;
      if (on) { d[i] = 21; d[i + 1] = 255; d[i + 2] = 162; d[i + 3] = line ? 15 : 8; }      // the mark: green, a row thicker
      else if (line) { d[i] = 0; d[i + 1] = 0; d[i + 2] = 0; d[i + 3] = 18; }               // the scanline itself
    }
  }
  tc.putImageData(img, 0, 0);
  const layer = document.createElement('div'); layer.className = 'weave'; layer.setAttribute('aria-hidden', 'true');
  layer.style.backgroundImage = `url(${tile.toDataURL('image/png')})`;
  document.body.appendChild(layer);
}
if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', weaveMark); else weaveMark();
