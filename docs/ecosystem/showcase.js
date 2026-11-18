// Tourne ecosystem showcase. Every preview here is a small working copy of the real piece; nothing is fetched,
// nothing is sent anywhere, and each preview stops its work while it's off screen.
(function () {
'use strict';
const $ = id => document.getElementById(id);
const el = (tag, cls, text) => { const e = document.createElement(tag); if (cls) e.className = cls; if (text != null) e.textContent = text; return e; };
const css = name => getComputedStyle(document.documentElement).getPropertyValue(name).trim();
const reduce = matchMedia('(prefers-reduced-motion: reduce)').matches;

// Pixel wordmark: 5x7 letters drawn like the blocks in Tourne'Table's header.
const PF = {
  T: ['11111','00100','00100','00100','00100','00100','00100'], O: ['01110','10001','10001','10001','10001','10001','01110'],
  U: ['10001','10001','10001','10001','10001','10001','01110'], R: ['11110','10001','10001','11110','10100','10010','10001'],
  N: ['10001','11001','10101','10011','10001','10001','10001'], E: ['11111','10000','10000','11110','10000','10000','11111'],
};
function pixelWord(svg, word) {
  const P = 16, Q = 13, W = word.length * 6 * P - P;
  let r = '';
  [...word].forEach((ch, n) => (PF[ch] || []).forEach((row, y) => [...row].forEach((bit, x) => {
    if (bit === '1') r += `<rect class="px" style="animation-delay:${(n * 7 + y) * 18}ms" x="${n * 6 * P + x * P}" y="${y * P}" width="${Q}" height="${Q}" fill="var(--neon)"/>`;
  })));
  svg.setAttribute('viewBox', `0 0 ${W} ${7 * P + 6}`);
  svg.innerHTML = `<style>.px{animation:pxin .5s steps(4) both}@keyframes pxin{from{opacity:0;transform:translateY(-8px)}}@media (prefers-reduced-motion:reduce){.px{animation:none}}</style>` + r + `<rect x="0" y="${7 * P + 2}" width="${W - (P - Q)}" height="4" fill="var(--brick)"/>`;
}
pixelWord($('word'), 'TOURNE');

// Run a frame loop only while its stage is on screen (idle means no work).
function whileVisible(node, start, stop) {
  if (!('IntersectionObserver' in window)) { start(); return; }
  new IntersectionObserver(es => es.forEach(e => e.isIntersecting ? start() : stop()), { threshold: 0.05 }).observe(node);
}

/* ---------------- Tourne Theme ---------------- */
const PALETTES = [
  { name: 'Tourne', ground: '#121c21', panel: '#1e2e36', neon: '#15ffa2', text: '#64aa89', dim: '#5d8d6f', red: '#ff8f8f', brick: '#a95d5d' },
  { name: 'Rose', ground: '#1d1418', panel: '#2b1d24', neon: '#ff8fb8', text: '#c98aa0', dim: '#8e6475', red: '#ffd08f', brick: '#6e8f5d' },
  { name: 'Sage', ground: '#151b16', panel: '#212b22', neon: '#b9ef7e', text: '#8db07a', dim: '#68825c', red: '#efa88a', brick: '#7a5640' },
  { name: 'Night', ground: '#15161f', panel: '#1f2335', neon: '#7aa2f7', text: '#8f9bc8', dim: '#5d6690', red: '#f7768e', brick: '#9d7cd8' },
  { name: 'Amber', ground: '#1b1a17', panel: '#2a2722', neon: '#fabd2f', text: '#c2a873', dim: '#8c7c5c', red: '#fb4934', brick: '#8f3f71' },
];
const hex = h => [1, 3, 5].map(i => parseInt(h.slice(i, i + 2), 16));
const mix = (a, b, t) => '#' + hex(a).map((v, i) => Math.round(v + (hex(b)[i] - v) * t).toString(16).padStart(2, '0')).join('');
function applyPalette(p) {
  const s = document.documentElement.style;
  s.setProperty('--ground', p.ground); s.setProperty('--panel', p.panel); s.setProperty('--neon', p.neon);
  s.setProperty('--text', p.text); s.setProperty('--dim', p.dim); s.setProperty('--red', p.red); s.setProperty('--brick', p.brick);
  s.setProperty('--raised', mix(p.ground, p.panel, 0.6)); s.setProperty('--edge', mix(p.panel, p.text, 0.18));
  s.setProperty('--lines', mix(p.ground, '#000000', 0.18)); s.setProperty('--hover', mix(p.ground, p.neon, 0.12));
  s.setProperty('--line', mix(p.ground, p.text, 0.3));
  colours = null;
}
const palWrap = $('palettes');
PALETTES.forEach((p, i) => {
  const b = el('button', 'swatch'); b.type = 'button'; b.setAttribute('role', 'radio'); b.setAttribute('aria-checked', i === 0);
  [p.ground, p.panel, p.text, p.neon, p.brick].forEach(c => { const sw = el('i'); sw.style.background = c; b.appendChild(sw); });
  b.appendChild(document.createTextNode(p.name));
  b.dataset.tip = p.name + '|Paints this palette on the whole page, the way Tourne Theme paints my desktop.';
  b.onclick = () => { applyPalette(p); palWrap.querySelectorAll('.swatch').forEach(x => x.setAttribute('aria-checked', x === b)); play('toggle-on'); };
  palWrap.appendChild(b);
});

/* ---------------- Sound engine ---------------- */
const FEELS = [
  ['minimal', 'Minimal', 'Dry, precise, almost invisible.'], ['soft', 'Soft', 'Rounded felt, warm and reassuring.'],
  ['glass', 'Glass', 'Bright, crystalline, and premium.'], ['arcade', 'Arcade', 'Chunky pixels and cheerful voltage.'],
  ['mechanical', 'Mechanical', 'Switches, relays, and firm detents.'], ['organic', 'Organic', 'Wood, water, breath, and small stones.'],
  ['dreamy', 'Dreamy', 'Airy blooms, soft light, and slow sparkle.'], ['scifi', 'Sci-fi', 'Clean holographic pings with a restrained digital shimmer.'],
  ['rubber', 'Rubber', 'Tactile elastic taps with a quick, friendly rebound.'], ['cinematic', 'Cinematic', 'Deep impacts, polished tails, and quiet scale.'],
  ['studio', 'Studio', 'Tactile editing precision with warm cinematic restraint.'], ['zen', 'Zen', 'Pure tones, dry wood, and brief washi detail.'],
];
const CUES = [
  ['hover', 'Hover', 'menu hover'], ['press', 'Press', 'a button'], ['open', 'Open', 'a menu opens'], ['close', 'Close', 'a window closes'],
  ['toggle-on', 'Toggle on', 'a switch'], ['snap', 'Snap', 'a window tiles'], ['swipe', 'Swipe', 'Alt+Tab moves'],
  ['notification', 'Notification', 'a pop-up lands'], ['success', 'Success', 'a capture saved'], ['error', 'Error', 'something failed'],
];
let feel = 'glass', lastCue = 'press';
const cache = {};
function play(cue, f) {
  f = f || feel;
  const key = f + '/' + cue;
  try {
    const a = (cache[key] = cache[key] || new Audio(`sounds/${key}.mp3`));
    a.volume = +$('vol').value; a.currentTime = 0; a.play().catch(() => {});
  } catch (e) { /* no audio here */ }
}
const feelWrap = $('feels');
FEELS.forEach(([id, label]) => {
  const b = el('button', null, label); b.type = 'button'; b.dataset.feel = id; b.setAttribute('role', 'radio');
  b.dataset.tip = label + '|' + FEELS.find(f => f[0] === id)[2] + ' 78 cues in this feel.';
  b.onclick = () => { setFeel(id); play(lastCue); };
  feelWrap.appendChild(b);
});
function setFeel(id) {
  feel = id;
  feelWrap.querySelectorAll('button').forEach(b => b.setAttribute('aria-checked', b.dataset.feel === id));
  $('feelAbout').textContent = FEELS.find(f => f[0] === id)[2];
}
setFeel(feel);
const pads = $('pads');
CUES.forEach(([id, label, use]) => {
  const b = el('button'); b.type = 'button'; b.dataset.cue = id;
  b.append(el('span', null, label), el('small', null, use));
  b.dataset.tip = label + '|Plays when ' + use + '. Any sound row in Studio can take this cue from any feel.';
  b.onclick = () => { lastCue = id; play(id); flash(b); };
  pads.appendChild(b);
});
function flash(b) { b.classList.remove('hit'); void b.offsetWidth; b.classList.add('hit'); }
$('compare').onclick = () => {
  const keep = feel;
  FEELS.forEach(([id], i) => setTimeout(() => { setFeel(id); play(lastCue, id); flash(pads.querySelector(`[data-cue="${lastCue}"]`)); if (i === FEELS.length - 1) setTimeout(() => setFeel(keep), 600); }, i * 520));
};

/* ---------------- Motion engine ---------------- */
function springPoints(n) {
  const pts = [];
  for (let i = 0; i <= n; i++) { const t = i / n; pts.push(1 - Math.exp(-6 * t) * Math.cos(t * Math.PI * 3.2)); }
  return pts;
}
const linearOk = window.CSS && CSS.supports && CSS.supports('transition-timing-function', 'linear(0, 1)');
const CURVES = {
  linear: { css: 'linear', f: t => t },
  easeOutSine: { css: 'cubic-bezier(.61,1,.88,1)', f: t => Math.sin(t * Math.PI / 2) },
  easeOutQuad: { css: 'cubic-bezier(.5,1,.89,1)', f: t => 1 - (1 - t) * (1 - t) },
  easeOutCubic: { css: 'cubic-bezier(.33,1,.68,1)', f: t => 1 - Math.pow(1 - t, 3) },
  easeInOutSine: { css: 'cubic-bezier(.37,0,.63,1)', f: t => -(Math.cos(Math.PI * t) - 1) / 2 },
  easeInOutCubic: { css: 'cubic-bezier(.65,0,.35,1)', f: t => t < .5 ? 4 * t * t * t : 1 - Math.pow(-2 * t + 2, 3) / 2 },
  easeOutExpo: { css: 'cubic-bezier(.16,1,.3,1)', f: t => t === 1 ? 1 : 1 - Math.pow(2, -10 * t) },
  overshoot: { css: 'cubic-bezier(.34,1.56,.64,1)', f: t => { const c = 1.70158, c3 = c + 1; return 1 + c3 * Math.pow(t - 1, 3) + c * Math.pow(t - 1, 2); } },
  spring: { css: linearOk ? 'linear(' + springPoints(40).map(v => v.toFixed(3)).join(',') + ')' : 'cubic-bezier(.34,1.56,.64,1)', f: t => 1 - Math.exp(-6 * t) * Math.cos(t * Math.PI * 3.2) },
};
const curveSel = $('mCurve');
Object.keys(CURVES).forEach(k => curveSel.add(new Option(k, k)));
curveSel.value = 'easeOutCubic';
function motionChanged() {
  const c = CURVES[curveSel.value], d = +$('mDur').value;
  $('mDurOut').textContent = d + ' ms';
  document.documentElement.style.setProperty('--ease', c.css);
  document.documentElement.style.setProperty('--dur', d + 'ms');
  drawGraph();
}
function drawGraph() {
  const cv = $('mGraph'), x = cv.getContext('2d'), W = cv.width, H = cv.height, f = CURVES[curveSel.value].f;
  x.clearRect(0, 0, W, H);
  const top = 26, bot = H - 14, sy = v => bot - v * (bot - top);
  x.fillStyle = css('--edge') || '#24393f';
  x.fillRect(0, sy(0), W, 1); x.fillRect(0, sy(1), W, 1);
  x.fillStyle = css('--neon') || '#15ffa2';
  for (let i = 0; i < W; i += 2) { const v = f(i / (W - 1)); x.fillRect(i, Math.round(sy(v)), 2, 2); }
}
['mCurve', 'mDur'].forEach(id => $(id).addEventListener('input', motionChanged));
const WHAT = {
  window: { k: [{ opacity: 0, transform: 'translate(-50%,-50%) scale(.82)' }, { opacity: 1, transform: 'translate(-50%,-50%) scale(1)' }] },
  menu: { k: [{ opacity: 0, transform: 'translate(-50%,-90%) scaleY(.2)' }, { opacity: 1, transform: 'translate(-50%,-50%) scaleY(1)' }] },
  flyout: { k: [{ opacity: 0, transform: 'translate(-50%,40%)' }, { opacity: 1, transform: 'translate(-50%,-50%)' }] },
  popup: { k: [{ opacity: 0, transform: 'translate(40%,-50%)' }, { opacity: 1, transform: 'translate(-50%,-50%)' }] },
};
$('mPlay').onclick = () => {
  const o = $('mObj'), w = WHAT[$('mWhat').value];
  o.style.transformOrigin = $('mWhat').value === 'menu' ? '50% 0' : '50% 50%';
  if (o.animate) o.animate(w.k, { duration: +$('mDur').value, easing: CURVES[curveSel.value].css, fill: 'both' });
  play('open');
};
motionChanged();

/* ---------------- Close effects ---------------- */
const EFFECTS = ['dust', 'shatter', 'melt', 'derez', 'wake', 'motion'];
let effect = 'dust', fxRun = 0;
const fxWrap = $('cEffects');
EFFECTS.forEach(id => {
  const b = el('button', null, id[0].toUpperCase() + id.slice(1)); b.type = 'button'; b.setAttribute('role', 'radio'); b.setAttribute('aria-checked', id === effect);
  b.dataset.tip = ({ dust: 'Dust|The window crumbles away from one side.', shatter: 'Shatter|Breaks into pieces that fly and fall.', melt: 'Melt|Columns drip down the screen.', derez: 'Derez|Folds to a line, then a dot, like an old screen turning off.', wake: 'Wake|Settles back and fades with a ripple.', motion: 'Motion|Uses the window-out motion line and its curve.' })[id];
  b.onclick = () => { effect = id; fxWrap.querySelectorAll('button').forEach(x => x.setAttribute('aria-checked', x === b)); playEffect(); };
  fxWrap.appendChild(b);
});
const cv = $('cCanvas'), cx = cv.getContext('2d');
const WX = 190, WY = 60, WW = 260, WH = 160;
function paintWindow(g, ox, oy) {
  g.fillStyle = css('--panel'); g.fillRect(ox, oy, WW, WH);
  g.fillStyle = css('--neon'); g.fillRect(ox, oy, WW, 2); g.fillRect(ox, oy, 2, WH); g.fillRect(ox + WW - 2, oy, 2, WH); g.fillRect(ox, oy + WH - 2, WW, 2);
  g.fillStyle = css('--red'); g.fillRect(ox + 10, oy + 9, 8, 8);
  g.fillStyle = css('--edge'); g.fillRect(ox, oy + 24, WW, 2);
  g.fillStyle = css('--neon'); g.fillRect(ox + 16, oy + 42, 150, 8);
  g.fillStyle = css('--text'); g.fillRect(ox + 16, oy + 62, 200, 6); g.fillRect(ox + 16, oy + 78, 120, 6);
  g.fillStyle = css('--dim'); g.fillRect(ox + 16, oy + 94, 170, 6); g.fillRect(ox + 16, oy + 110, 90, 6);
  g.fillStyle = css('--brick'); g.fillRect(ox + 16, oy + 130, 60, 12);
}
function drawDesk(g) {
  g.fillStyle = css('--ground'); g.fillRect(0, 0, cv.width, cv.height);
  g.fillStyle = css('--panel'); g.fillRect(0, cv.height - 24, cv.width, 24);
  g.fillStyle = css('--neon'); g.fillRect(16, cv.height - 17, 10, 10);
  g.fillStyle = css('--dim'); g.fillRect(34, cv.height - 17, 10, 10); g.fillRect(52, cv.height - 17, 10, 10);
}
function blocks(size) {
  const off = document.createElement('canvas'); off.width = WW; off.height = WH;
  const g = off.getContext('2d'); paintWindow(g, 0, 0);
  const data = g.getImageData(0, 0, WW, WH).data, out = [];
  for (let y = 0; y < WH; y += size) for (let x = 0; x < WW; x += size) {
    const i = ((y + (size >> 1)) * WW + x + (size >> 1)) * 4;
    out.push({ x, y, c: `rgb(${data[i]},${data[i + 1]},${data[i + 2]})`, r1: Math.random(), r2: Math.random(), r3: Math.random() });
  }
  return { off, out };
}
function playEffect() {
  const run = ++fxRun, on = $('cOn').value, size = effect === 'shatter' ? 16 : 5;
  const { off, out } = blocks(size), T = 1100, t0 = performance.now(), mini = on === 'Minimize';
  play(on === 'Restore' ? 'open' : 'close');
  const frame = now => {
    if (run !== fxRun) return;
    let t = Math.min(1, (now - t0) / T);
    const p = on === 'Restore' ? 1 - t : t;
    drawDesk(cx);
    const ease = CURVES[curveSel.value].f;
    if (effect === 'derez') {
      const a = Math.min(1, p / .55), b = Math.max(0, (p - .55) / .3), h = Math.max(2, WH * (1 - ease(a))), w = Math.max(2, WW * (1 - Math.min(1, b)));
      cx.globalAlpha = p > .9 ? 1 - (p - .9) * 10 : 1;
      cx.drawImage(off, WX + (WW - w) / 2, WY + (WH - h) / 2, w, h);
      if (a >= 1) { cx.fillStyle = css('--neon'); cx.fillRect(WX + (WW - w) / 2, WY + WH / 2 - 1, w, 2); }
      cx.globalAlpha = 1;
    } else if (effect === 'wake' || effect === 'motion') {
      const s = effect === 'wake' ? 1 - .12 * ease(p) : 1 - .25 * ease(p), w = WW * s, h = WH * s, dy = mini ? 90 * ease(p) : 0;
      cx.globalAlpha = 1 - ease(p);
      cx.drawImage(off, WX + (WW - w) / 2, WY + (WH - h) / 2 + dy, w, h);
      cx.globalAlpha = 1;
      if (effect === 'wake' && p > 0 && p < 1) {
        const r = 1 + p * .5; cx.strokeStyle = css('--neon'); cx.globalAlpha = 1 - p; cx.lineWidth = 2;
        cx.strokeRect(WX + WW / 2 - WW * r / 2, WY + WH / 2 - WH * r / 2 + dy, WW * r, WH * r); cx.globalAlpha = 1;
      }
    } else {
      for (const k of out) {
        let x = k.x, y = k.y, a = 1;
        if (effect === 'dust') {
          const d = (k.x / WW) * .45 + k.r1 * .15, q = Math.max(0, (p - d) / (1 - d));
          x += q * (40 + k.r2 * 90); y -= q * (20 + k.r3 * 70) - (mini ? q * q * 160 : 0); a = 1 - q;
        } else if (effect === 'shatter') {
          const dx = k.x + 8 - WW / 2, dy = k.y + 8 - WH / 2, len = Math.hypot(dx, dy) || 1, sp = 160 + k.r1 * 260, q = p * 1.1;
          x += dx / len * sp * q; y += dy / len * sp * q + 420 * q * q * (mini ? 1.6 : 1); a = 1 - Math.max(0, (p - .5) * 2);
        } else if (effect === 'melt') {
          const col = (k.x / size) | 0, d = ((col * 7919) % 97) / 97 * .35, q = Math.max(0, (p - d) / (1 - d));
          y += q * q * (260 + k.y * .8); a = 1 - Math.max(0, (q - .6) * 2.5);
        }
        if (a <= 0) continue;
        cx.globalAlpha = a; cx.fillStyle = k.c; cx.fillRect(Math.round(WX + x), Math.round(WY + y), size, size);
      }
      cx.globalAlpha = 1;
    }
    if (t < 1) requestAnimationFrame(frame);
    else setTimeout(() => { if (run === fxRun && on !== 'Restore') { drawDesk(cx); setTimeout(() => { if (run === fxRun) { drawDesk(cx); cx.drawImage(off, WX, WY); } }, 500); } }, 250);
  };
  requestAnimationFrame(frame);
}
$('cPlay').onclick = playEffect;
drawDesk(cx); { const { off } = blocks(5); cx.drawImage(off, WX, WY); }

/* ---------------- Tiling (the same maths as tiling-layouts.cpp) ---------------- */
const LAYOUTS = ['columns', 'rows', 'verticalStack', 'rightMainVerticalStack', 'horizontalStack', 'ultrawideVerticalStack', 'grid', 'bsp', 'spiral', 'threeColumn', 'slice', 'screenSplit', 'monocle', 'layers', 'script'];
const NAMES = ['terminal', 'notes', 'browser', 'studio', 'music', 'files', 'editor', 'chat', 'viewer', 'capture', 'mail', 'calc'];
const R = (l, t, r, b) => ({ l, t, r, b });
function cut(start, length, count, gap) {
  const out = [], room = Math.max(0, length - gap * (count - 1)), each = Math.floor(room / count), extra = room % count;
  for (let i = 0, at = start; i < count; i++) { const s = each + (i < extra ? 1 : 0); out.push([at, at + s]); at += s + gap; }
  return out;
}
function cutShare(start, length, pct, gap) {
  if (length < 2) return [[start, start + length], [start + length, start + length]];
  gap = Math.max(0, Math.min(gap, length - 2));
  const room = length - gap, first = Math.max(1, Math.min(room - 1, Math.floor(room * pct / 100)));
  return [[start, start + first], [start + first + gap, start + length]];
}
function bsp(a, n, across, pct, gap, out) {
  if (n === 1) { out.push(a); return; }
  const A = { ...a }, B = { ...a };
  if (across) { const [l, r] = cutShare(a.l, a.r - a.l, pct, gap); A.l = l[0]; A.r = l[1]; B.l = r[0]; B.r = r[1]; }
  else { const [t, u] = cutShare(a.t, a.b - a.t, pct, gap); A.t = t[0]; A.b = t[1]; B.t = u[0]; B.b = u[1]; }
  out.push(A); bsp(B, n - 1, !across, 50, gap, out);
}
function spiral(a, n, turn, pct, gap, out) {
  if (n <= 1) { out.push(a); return; }
  const across = turn % 2 === 0, front = turn < 2, M = { ...a }, Rr = { ...a };
  if (across) { const [l, r] = cutShare(a.l, a.r - a.l, front ? pct : 100 - pct, gap); const [take, leave] = front ? [l, r] : [r, l]; M.l = take[0]; M.r = take[1]; Rr.l = leave[0]; Rr.r = leave[1]; }
  else { const [t, b] = cutShare(a.t, a.b - a.t, front ? pct : 100 - pct, gap); const [take, leave] = front ? [t, b] : [b, t]; M.t = take[0]; M.b = take[1]; Rr.t = leave[0]; Rr.b = leave[1]; }
  out.push(M); spiral(Rr, n - 1, (turn + 1) % 4, 50, gap, out);
}
function layoutRects(layout, a, n, gap, pct) {
  const rects = [], w = a.r - a.l, h = a.b - a.t;
  if (n <= 0) return rects;
  const column = (l, r, c) => cut(a.t, h, c, gap).forEach(([t, b]) => rects.push(R(l, t, r, b)));
  const row = (t, b, c) => cut(a.l, w, c, gap).forEach(([l, r]) => rects.push(R(l, t, r, b)));
  if (n === 1 || layout === 'monocle') { for (let i = 0; i < n; i++) rects.push({ ...a }); return rects; }
  switch (layout) {
    case 'rows': column(a.l, a.r, n); break;
    case 'verticalStack': { const [m, s] = cutShare(a.l, w, pct, gap); rects.push(R(m[0], a.t, m[1], a.b)); column(s[0], s[1], n - 1); break; }
    case 'rightMainVerticalStack': { const [s, m] = cutShare(a.l, w, 100 - pct, gap); rects.push(R(m[0], a.t, m[1], a.b)); column(s[0], s[1], n - 1); break; }
    case 'horizontalStack': { const [m, s] = cutShare(a.t, h, pct, gap); rects.push(R(a.l, m[0], a.r, m[1])); row(s[0], s[1], n - 1); break; }
    case 'ultrawideVerticalStack': case 'threeColumn': {
      if (n === 2) return layoutRects('verticalStack', a, n, gap, pct);
      const mainW = Math.floor(Math.max(0, w - 2 * gap) * pct / 100), side = Math.floor(Math.max(0, w - 2 * gap - mainW) / 2), ml = a.l + side + gap;
      rects.push(R(ml, a.t, ml + mainW, a.b));
      if (layout === 'ultrawideVerticalStack') { rects.push(R(a.l, a.t, a.l + side, a.b)); column(ml + mainW + gap, a.r, n - 2); }
      else { const left = Math.floor((n - 1) / 2); column(a.l, a.l + side, left); column(ml + mainW + gap, a.r, n - 1 - left); }
      break;
    }
    case 'grid': case 'screenSplit': {
      const cols = Math.ceil(Math.sqrt(n)), spans = cut(a.l, w, cols, gap);
      for (let c = 0; c < cols; c++) { const cnt = Math.floor(n / cols) + (c < n % cols ? 1 : 0); if (cnt) column(spans[c][0], spans[c][1], cnt); }
      break;
    }
    case 'bsp': bsp(a, n, true, pct, gap, rects); break;
    case 'spiral': spiral(a, n, 0, pct, gap, rects); break;
    case 'slice': { const [s, r] = cutShare(a.l, w, 30, gap); rects.push(R(s[0], a.t, s[1], a.b)); layoutRects('columns', R(r[0], a.t, r[1], a.b), n - 1, gap, pct).forEach(x => rects.push(x)); break; }
    case 'layers': for (let at = 0; at < n; at += 3) { const cnt = at === 0 ? Math.min(3, n) : 3, lay = layoutRects('verticalStack', a, cnt, gap, pct); for (let i = 0; i < cnt && at + i < n; i++) rects.push(lay[i]); } break;
    case 'script': {
      // A layout of my own from a script: a golden main window, the rest along the bottom of the other side.
      const [m, s] = cutShare(a.l, w, 62, gap); rects.push(R(m[0], a.t, m[1], a.b));
      if (n === 2) { rects.push(R(s[0], a.t, s[1], a.b)); break; }
      const [top, bottom] = cutShare(a.t, h, 55, gap); rects.push(R(s[0], top[0], s[1], top[1]));
      cut(s[0], s[1] - s[0], n - 2, gap).forEach(([l, r]) => rects.push(R(l, bottom[0], r, bottom[1])));
      break;
    }
    default: row(a.t, a.b, n);
  }
  return rects;
}
const tStage = $('tStage'), tSel = $('tLayout');
LAYOUTS.forEach(l => tSel.add(new Option(l, l)));
const badge = el('span', 'tile-badge'); tStage.appendChild(badge);
let wins = [], focus = 0, nextId = 0, layout = 'bsp', gap = 8;
tSel.value = layout;
function autoLayout(n) { return n <= 1 ? 'monocle' : n === 2 ? 'columns' : n === 3 ? 'verticalStack' : n <= 4 ? 'grid' : 'bsp'; }
function addWin(name) {
  const w = { id: nextId++, name: name || NAMES[nextId % NAMES.length], float: false, node: el('div', 'tw gone') };
  w.node.innerHTML = `<div class="tb"></div><div class="body"><span></span><span style="width:80%"></span><span style="width:45%"></span></div>`;
  w.node.querySelector('.tb').textContent = w.name;
  const W = tStage.clientWidth, H = tStage.clientHeight;
  Object.assign(w.node.style, { left: W / 2 - 30 + 'px', top: H / 2 - 20 + 'px', width: '60px', height: '40px' });
  w.node.dataset.tip = w.name + '|Click to focus it.';
  w.node.onclick = () => { focus = wins.indexOf(w); render(); };
  tStage.appendChild(w.node); wins.push(w); focus = wins.length - 1;
  requestAnimationFrame(() => requestAnimationFrame(() => { w.node.classList.remove('gone'); render(); }));
  if ($('tSound').checked) play('snap');
}
function killWin() {
  if (!wins.length) return;
  const w = wins.splice(focus, 1)[0];
  w.node.classList.add('gone');
  setTimeout(() => w.node.remove(), parseInt(css('--dur')) || 320);
  focus = Math.max(0, Math.min(focus, wins.length - 1));
  if ($('tSound').checked) play('close');
  render();
}
function render() {
  const W = tStage.clientWidth, H = tStage.clientHeight;
  if ($('tAuto').checked) { layout = autoLayout(wins.filter(w => !w.float).length); tSel.value = layout; }
  const tiled = wins.filter(w => !w.float), area = R(gap, gap, W - gap, H - gap);
  const rects = layoutRects(layout, area, tiled.length, gap, 55);
  tiled.forEach((w, i) => { const r = rects[i]; Object.assign(w.node.style, { left: r.l + 'px', top: r.t + 'px', width: (r.r - r.l) + 'px', height: (r.b - r.t) + 'px', zIndex: layout === 'monocle' ? (wins[focus] === w ? 2 : 1) : 1 }); });
  wins.forEach((w, i) => {
    w.node.classList.toggle('focus', i === focus); w.node.classList.toggle('bordered', $('tBorder').checked); w.node.classList.toggle('float', w.float);
    if (w.float) { const fw = Math.min(260, W * .5), fh = Math.min(170, H * .55); Object.assign(w.node.style, { left: (W - fw) / 2 + 'px', top: (H - fh) / 2 + 'px', width: fw + 'px', height: fh + 'px', zIndex: 3 }); }
    w.node.style.opacity = layout === 'monocle' && !w.float && i !== focus ? '0.35' : '';
  });
  badge.textContent = `${layout} . ${wins.length} window${wins.length === 1 ? '' : 's'} . gaps ${gap}`;
}
function setLayout(l) { layout = l; tSel.value = l; $('tAuto').checked = false; if ($('tSound').checked) play('snap'); render(); }
tSel.onchange = () => setLayout(tSel.value);
$('tAdd').onclick = () => addWin();
$('tKill').onclick = killWin;
$('tGap').oninput = () => { gap = +$('tGap').value; $('tGapOut').textContent = gap + ' px'; render(); };
['tAuto', 'tBorder'].forEach(id => $(id).onchange = render);
addEventListener('resize', render);
['terminal', 'browser', 'music', 'notes'].forEach(n => addWin(n));

/* ---------------- Key binds (the Hyprland way, as binds.cpp reads them) ---------------- */
const DEFAULT_BINDS = `# My key binds, one a line, written the way Hyprland writes them.
# bind = MODS, key, what it does, its argument
$mod = SHIFT

bind  = $mod, N, exec, terminal        # open a window
bind  = $mod, Q, killactive            # close the focused one
bind  = $mod, L, layout, next          # the next layout
bind  = $mod, B, layout, bsp
bind  = $mod, S, layout, spiral
bind  = , left, movefocus, l
bind  = , right, movefocus, r
binde = $mod, up, gaps, +2             # e: repeats while held
binde = $mod, down, gaps, -2

# One key, three jobs (my Win+Space):
bind  = , space, togglefloating        # tap: tiled or floating
bindk = , space, monocle               # k: double tap, focused one full
bindo = , space, tileall               # o: hold, tile every window

bind  = , 1, playsound, success
bind  = , 2, playsound, notification
bind  = $mod, F, feel, next            # the next sound feel`;
const bindsBox = $('binds'), log = $('bLog');
bindsBox.value = DEFAULT_BINDS;
let binds = [];
const MODS = ['SHIFT', 'CTRL', 'ALT', 'SUPER'];
function parseBinds() {
  const vars = {}; binds = [];
  bindsBox.value.split('\n').forEach(raw => {
    const line = raw.replace(/#.*/, '').trim();
    if (!line) return;
    let m = line.match(/^\$(\w+)\s*=\s*(.*)$/);
    if (m) { vars[m[1]] = m[2].trim(); return; }
    m = line.match(/^bind([a-z]*)\s*=\s*(.*)$/i);
    if (!m) return;
    const parts = m[2].replace(/\$(\w+)/g, (_, v) => vars[v] || '').split(',').map(s => s.trim());
    if (parts.length < 3) return;
    const mods = parts[0].toUpperCase().split(/[\s+]+/).filter(Boolean).map(x => x === 'WIN' ? 'SUPER' : x === 'CONTROL' ? 'CTRL' : x).sort().join('+');
    binds.push({ flags: m[1].toLowerCase(), mods, key: parts[1].toLowerCase(), cmd: parts[2].toLowerCase(), arg: parts.slice(3).join(',').trim(), raw: raw.trim() });
  });
  drawKeycaps();
}
function comboLabel(mods, key) { return (mods ? mods.split('+').map(m => m[0] + m.slice(1).toLowerCase()).join('+') + '+' : '') + (key.length === 1 ? key.toUpperCase() : key[0].toUpperCase() + key.slice(1)); }
function say(html, miss) { const p = el('p'); if (miss) p.className = 'miss'; p.innerHTML = html; log.prepend(p); while (log.children.length > 40) log.lastChild.remove(); }
const esc = s => s.replace(/[&<>"]/g, c => ({ '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;' }[c]));
function run(b, how) {
  const ok = dispatch(b.cmd, b.arg);
  say(`<b>${esc(comboLabel(b.mods, b.key))}</b>${how ? ' ' + how : ''}: ${esc(b.cmd)}${b.arg ? ' ' + esc(b.arg) : ''}${ok ? '' : ' (not in this preview)'}`, !ok);
}
function dispatch(cmd, arg) {
  switch (cmd) {
    case 'exec': addWin(arg || undefined); return true;
    case 'killactive': killWin(); return true;
    case 'layout': {
      const i = LAYOUTS.indexOf(layout);
      setLayout(arg === 'next' ? LAYOUTS[(i + 1) % LAYOUTS.length] : arg === 'prev' ? LAYOUTS[(i + LAYOUTS.length - 1) % LAYOUTS.length] : LAYOUTS.includes(arg) ? arg : layout);
      return LAYOUTS.includes(arg) || arg === 'next' || arg === 'prev';
    }
    case 'movefocus': if (!wins.length) return true; focus = (focus + (arg === 'l' || arg === 'u' ? wins.length - 1 : 1)) % wins.length; render(); if ($('tSound').checked) play('hover'); return true;
    case 'gaps': gap = Math.max(0, Math.min(24, gap + (parseInt(arg) || 0))); $('tGap').value = gap; $('tGapOut').textContent = gap + ' px'; render(); return true;
    case 'togglefloating': if (wins[focus]) { wins[focus].float = !wins[focus].float; render(); if ($('tSound').checked) play('snap'); } return true;
    case 'monocle': wins.forEach(w => w.float = false); setLayout(layout === 'monocle' ? 'bsp' : 'monocle'); return true;
    case 'tileall': wins.forEach(w => w.float = false); setLayout(layout === 'monocle' ? 'bsp' : layout); return true;
    case 'playsound': play(arg || 'press'); return CUES.some(c => c[0] === arg);
    case 'feel': { const i = FEELS.findIndex(f => f[0] === feel); setFeel(FEELS[(i + 1) % FEELS.length][0]); play('press'); say(`feel is now <b>${feel}</b>`); return true; }
  }
  return false;
}
// Tap, double tap (k) and hold (o) on one key, with the real defaults: 300 ms for a double, 500 ms for a hold.
const DOUBLE_MS = 300, HOLD_MS = 500;
const keyState = {};
function pressDown(mods, key, repeat) {
  const mine = binds.filter(b => b.mods === mods && b.key === key);
  if (!mine.length) { if (!repeat) say(`${esc(comboLabel(mods, key))}: nothing bound`, true); return false; }
  const special = mine.some(b => b.flags.includes('k') || b.flags.includes('o'));
  const id = mods + '|' + key, st = keyState[id] = keyState[id] || {};
  if (repeat) { mine.filter(b => b.flags.includes('e')).forEach(b => run(b, '(held)')); return true; }
  if (!special) { mine.filter(b => !b.flags.includes('r')).forEach(b => run(b)); return true; }
  st.held = false;
  clearTimeout(st.hold);
  const hold = mine.find(b => b.flags.includes('o'));
  if (hold) st.hold = setTimeout(() => { st.held = true; clearTimeout(st.tap); st.tapPending = false; run(hold, '(held)'); }, HOLD_MS);
  return true;
}
function pressUp(mods, key) {
  const mine = binds.filter(b => b.mods === mods && b.key === key);
  mine.filter(b => b.flags.includes('r')).forEach(b => run(b, '(let go)'));
  const id = mods + '|' + key, st = keyState[id];
  if (!st || !mine.some(b => b.flags.includes('k') || b.flags.includes('o'))) return;
  clearTimeout(st.hold);
  if (st.held) { st.held = false; return; }
  const dbl = mine.find(b => b.flags.includes('k')), tap = mine.find(b => !/[ko]/.test(b.flags));
  if (dbl && st.tapPending) { clearTimeout(st.tap); st.tapPending = false; run(dbl, '(double)'); return; }
  if (dbl) { st.tapPending = true; st.tap = setTimeout(() => { st.tapPending = false; if (tap) run(tap, '(tap)'); }, DOUBLE_MS); }
  else if (tap) run(tap, '(tap)');
}
const CODE = c => c.startsWith('Key') ? c.slice(3).toLowerCase() : c.startsWith('Digit') ? c.slice(5) : c.startsWith('Arrow') ? c.slice(5).toLowerCase() : c === 'Space' ? 'space' : c === 'Enter' ? 'return' : c === 'Escape' ? 'escape' : c === 'Tab' ? 'tab' : c.toLowerCase();
const modsOf = e => [e.shiftKey && 'SHIFT', e.ctrlKey && 'CTRL', e.altKey && 'ALT', e.metaKey && 'SUPER'].filter(Boolean).sort().join('+');
const downKeys = {};
tStage.addEventListener('keydown', e => {
  if (/^(Shift|Control|Alt|Meta)/.test(e.code)) return;
  const key = CODE(e.code), mods = modsOf(e);
  if (pressDown(mods, key, e.repeat)) e.preventDefault();
  downKeys[e.code] = mods;
});
tStage.addEventListener('keyup', e => {
  if (!(e.code in downKeys)) return;
  pressUp(downKeys[e.code], CODE(e.code)); delete downKeys[e.code];
});
tStage.addEventListener('pointerdown', () => tStage.focus({ preventScroll: true }));
function drawKeycaps() {
  const k = $('keys'); k.innerHTML = '';
  const seen = new Set();
  binds.forEach(b => {
    const id = b.mods + '|' + b.key; if (seen.has(id)) return; seen.add(id);
    const btn = el('button', null, comboLabel(b.mods, b.key)); btn.type = 'button';
    btn.dataset.tip = comboLabel(b.mods, b.key) + '|' + binds.filter(x => x.mods === b.mods && x.key === b.key).map(x => x.raw.replace(/\s*#.*/, '')).join('\n');
    let rep;
    btn.onpointerdown = e => { e.preventDefault(); btn.classList.add('down'); pressDown(b.mods, b.key, false); if (binds.some(x => x.mods === b.mods && x.key === b.key && x.flags.includes('e'))) rep = setInterval(() => pressDown(b.mods, b.key, true), 120); };
    const up = () => { if (!btn.classList.contains('down')) return; btn.classList.remove('down'); clearInterval(rep); pressUp(b.mods, b.key); };
    btn.onpointerup = up; btn.onpointerleave = up;
    btn.onkeydown = e => { if (e.key === 'Enter' || e.key === ' ') { e.preventDefault(); if (!e.repeat) { pressDown(b.mods, b.key, false); setTimeout(() => pressUp(b.mods, b.key), 60); } } };
    k.appendChild(btn);
  });
}
bindsBox.addEventListener('input', parseBinds);
$('bReset').onclick = () => { bindsBox.value = DEFAULT_BINDS; parseBinds(); say('the file is back as it was'); };
parseBinds();
say('click the screen above, then press a bound key, or press a keycap');

/* ---------------- Alt+Tab views ---------------- */
const VIEWS = ['List', 'Flip 3D', 'Cover Flow', 'Ring', 'Deck', 'Canvas'];
let view = 'Flip 3D', cur = 0;
const sRow = $('sRow'), sStage = $('sStage'), SW = ['terminal', 'browser', 'music', 'studio', 'notes', 'files', 'viewer'];
const cards = SW.map((n, i) => {
  const c = el('div', 'sw-card'); c.innerHTML = '<div class="tb"></div><div class="pic"></div>';
  c.querySelector('.tb').textContent = n;
  c.querySelector('.pic').style.background = `linear-gradient(${i * 50}deg, var(--edge), color-mix(in srgb, var(--neon) ${15 + i * 6}%, var(--panel)))`;
  c.onclick = () => { cur = i; layoutSwitcher(); };
  sRow.appendChild(c); return c;
});
const list = el('div', 'sw-list'); sStage.appendChild(list);
const vWrap = $('sViews');
VIEWS.forEach(v => {
  const b = el('button', null, v); b.type = 'button'; b.setAttribute('role', 'radio'); b.setAttribute('aria-checked', v === view);
  b.dataset.tip = v + '|One of the switcher views. Press Next to move through the windows.';
  b.onclick = () => { view = v; vWrap.querySelectorAll('button').forEach(x => x.setAttribute('aria-checked', x === b)); layoutSwitcher(); };
  vWrap.appendChild(b);
});
function layoutSwitcher() {
  const n = cards.length;
  list.style.display = view === 'List' ? '' : 'none';
  if (view === 'List') {
    list.innerHTML = SW.map((s, i) => `<div style="padding:2px 8px;border:1px solid ${i === cur ? 'var(--neon)' : 'transparent'};color:${i === cur ? 'var(--neon)' : 'var(--text)'}">${s}</div>`).join('');
  }
  cards.forEach((c, i) => {
    const off = ((i - cur) % n + n) % n, sgn = (() => { let d = i - cur; if (d > n / 2) d -= n; if (d < -n / 2) d += n; return d; })();
    let t = '', o = 1, z = 0;
    switch (view) {
      case 'List': o = 0; break;
      case 'Flip 3D': t = `translate3d(${off * -34}px, ${off * -16}px, ${-off * 90}px) rotateY(-38deg)`; z = n - off; o = off > 5 ? 0 : 1; break;
      case 'Cover Flow': t = sgn === 0 ? 'translateZ(80px)' : `translateX(${Math.sign(sgn) * (110 + Math.abs(sgn) * 46)}px) rotateY(${-Math.sign(sgn) * 62}deg)`; z = n - Math.abs(sgn); o = Math.abs(sgn) > 3 ? 0 : 1; break;
      case 'Ring': { const a = sgn / n * Math.PI * 2, Rr = 230; t = `translate3d(${Math.sin(a) * Rr}px, 0, ${Math.cos(a) * Rr - Rr}px)`; z = Math.round(Math.cos(a) * 10) + 10; break; }
      case 'Deck': t = `translate3d(${off * 18}px, ${off * 10}px, ${-off * 40}px)`; z = n - off; o = Math.max(0, 1 - off * .14); break;
      case 'Canvas': { const col = i % 4, row = (i / 4) | 0; t = `translate(${(col - 1.5) * 170}px, ${(row - .5) * 118}px) scale(.92)`; break; }
    }
    c.style.transform = t; c.style.opacity = o; c.style.zIndex = z; c.classList.toggle('cur', i === cur);
  });
  $('sName').textContent = SW[cur];
}
$('sNext').onclick = () => { cur = (cur + 1) % cards.length; layoutSwitcher(); if ($('tSound').checked) play('swipe'); };
$('sPrev').onclick = () => { cur = (cur + cards.length - 1) % cards.length; layoutSwitcher(); if ($('tSound').checked) play('swipe'); };
$('sDim').onchange = () => sStage.classList.toggle('dim', $('sDim').checked);
sStage.classList.toggle('dim', true);
layoutSwitcher();

/* ---------------- Icon animation engine ---------------- */
const ANIMS = ['bounce', 'wiggle', 'pulse', 'jelly', 'spin', 'float', 'glitch', 'flip'];
const iSel = $('iAnim'); ANIMS.forEach(a => iSel.add(new Option(a, a)));
const ICONS = ['audio-high', 'wifi-4', 'bluetooth-connected', 'battery-full', 'bell', 'mic', 'brightness', 'night-light', 'security-ok', 'vpn-on', 'folder', 'settings', 'power', 'camera-in-use'];
const ibar = $('ibar');
const imgs = ICONS.map(n => {
  const b = el('button'); b.type = 'button'; b.setAttribute('aria-label', n.replace(/-/g, ' ')); b.dataset.tip = n.replace(/-/g, ' ') + '|One of my pixel tray icons.';
  const im = el('img'); im.src = `icons/tray-${n}.png`; im.alt = ''; b.appendChild(im);
  b.onmouseenter = () => $('iWhen').value === 'hover' && animate(im);
  b.onclick = () => { if ($('iWhen').value === 'press') { animate(im); play('press'); } };
  ibar.appendChild(b); return im;
});
function animate(im, delay) {
  const spd = +$('iSpd').value, a = iSel.value, d = a === 'spin' ? 700 : a === 'float' ? 1400 : 600;
  im.style.animation = 'none'; void im.offsetWidth;
  im.style.animation = `a-${a} ${d / spd}ms ${a === 'glitch' ? 'steps(5)' : 'steps(8)'} ${delay || 0}ms ${$('iWhen').value === 'attention' ? 'infinite' : '1'}`;
}
function iconsChanged() {
  $('iSpdOut').textContent = (+$('iSpd').value).toFixed(1) + 'x';
  imgs.forEach((im, i) => { im.style.animation = 'none'; if ($('iWhen').value === 'attention') animate(im, i * 90); });
}
['iAnim', 'iWhen', 'iSpd'].forEach(id => $(id).addEventListener('input', iconsChanged));
iconsChanged();

/* ---------------- Tourne'Table: a made-up 120 BPM test beat ---------------- */
const vc = $('vCanvas'), vx = vc.getContext('2d');
let colours = null;
const getColours = () => colours || (colours = { neon: css('--neon'), text: css('--text'), dim: css('--dim'), red: css('--red'), brick: css('--brick'), ground: css('--ground'), panel: css('--panel'), edge: css('--edge') });
let spec = new Float32Array(96), peak = new Float32Array(96), raf = 0, t0 = performance.now(), lastBeat = -1;
const BEAT = 500;
const LINES = ['my desktop answers only me', 'every window lands on the beat', 'nothing here just pops in', 'glass along every edge tonight'];
const lyric = $('lyric');
let lineIdx = -1, spans = [];
function setLine(i) {
  lineIdx = i; lyric.innerHTML = '';
  const mode = $('lMode').value, text = LINES[i % LINES.length];
  const pieces = mode === 'letter' ? [...text] : text.split(/(\s+)/);
  spans = pieces.map(p => { const s = el('span', mode === 'line' ? '' : 'wait', p); lyric.appendChild(s); return s; });
}
function beatEnv(ms) { const ph = (ms % BEAT) / BEAT; return Math.exp(-ph * 7); }
function synth(now) {
  const ms = now - t0, n = +$('vCount').value, beatN = Math.floor(ms / BEAT), kick = beatEnv(ms), snare = beatN % 2 ? kick : 0, hat = Math.exp(-((ms + BEAT / 2) % (BEAT / 2)) / (BEAT / 2) * 9);
  for (let i = 0; i < n; i++) {
    const f = i / n;
    let v = (1 - f) * 0.25 + 0.08 * Math.sin(ms / 900 + i * .5) + 0.06 * Math.sin(ms / 370 + i * 1.7);
    v += kick * Math.max(0, 1 - f * 5) * .9 + snare * Math.exp(-Math.pow((f - .35) * 6, 2)) * .55 + hat * Math.max(0, f - .6) * .7;
    v += 0.22 * Math.exp(-Math.pow((f - (.4 + .25 * Math.sin(ms / 2600))) * 12, 2));
    v = Math.max(0, Math.min(1, v));
    spec[i] = v > spec[i] ? spec[i] + (v - spec[i]) * .6 : spec[i] * .86 + v * .14;
    peak[i] = Math.max(peak[i] - .008, spec[i]);
  }
  if (beatN !== lastBeat) {
    lastBeat = beatN;
    if (beatN % 4 === 0) setLine(lineIdx + 1);
    if ($('iWhen').value === 'beat') imgs.forEach((im, i) => animate(im, i * 12));
  }
  // sung colour: paced across the line's four beats
  const into = ((ms % (BEAT * 4)) / (BEAT * 4)) * spans.length * 1.15;
  spans.forEach((s, i) => { s.classList.toggle('sung', i < into); if (i < into + 2) s.classList.remove('wait'); });
  return { n, kick, ms };
}
let spectroRow = 0;
function drawVis(now) {
  const { n, kick, ms } = synth(now), c = getColours(), W = vc.width, H = vc.height, shape = $('vShape').value;
  if (shape !== 'spectro') { vx.fillStyle = c.ground; vx.fillRect(0, 0, W, H); }
  const gapX = 2, bw = (W - 40) / n - gapX, base = shape === 'floor' ? H * .62 : H - 20;
  vx.fillStyle = c.neon;
  if (shape === 'bars' || shape === 'floor') {
    for (let i = 0; i < n; i++) {
      const h = Math.round(spec[i] * (base - 70)), x = 20 + i * (bw + gapX);
      vx.fillStyle = c.neon; vx.fillRect(x, base - h, bw, h);
      vx.fillStyle = c.red; vx.fillRect(x, base - Math.round(peak[i] * (base - 70)) - 3, bw, 2);
      if (shape === 'floor') { vx.globalAlpha = .25; vx.fillStyle = c.neon; vx.fillRect(x, base + 3, bw, h * .5); vx.globalAlpha = 1; }
    }
  } else if (shape === 'led') {
    const seg = 7;
    for (let i = 0; i < n; i++) {
      const x = 20 + i * (bw + gapX), lit = Math.round(spec[i] * (base - 70) / seg);
      for (let s = 0; s < lit; s++) { vx.fillStyle = s > 22 ? c.red : s > 15 ? c.brick : c.neon; vx.fillRect(x, base - (s + 1) * seg, bw, seg - 2); }
    }
  } else if (shape === 'bass') {
    vx.fillStyle = c.edge; vx.fillRect(20, H / 2 - 1, W - 40, 2);
    const lane = Math.round(kick * 120);
    vx.fillStyle = c.neon; vx.fillRect(W / 2 - 60, H / 2 - lane / 2, 120, lane);
    vx.fillStyle = c.red; const sn = (Math.floor(ms / BEAT) % 2) ? lane * .7 : 0; vx.fillRect(W / 2 + 80, H / 2 - sn / 2, 40, sn);
    vx.fillStyle = c.dim; vx.font = '12px monospace'; vx.fillText('kick', W / 2 - 14, H / 2 + 80); vx.fillText('snare', W / 2 + 82, H / 2 + 80);
  } else if (shape === 'scope') {
    vx.fillStyle = c.neon;
    for (let x = 0; x < W; x += 2) { const y = H / 2 + Math.sin(x / 18 + ms / 90) * 40 * (0.3 + kick) + Math.sin(x / 5.3 + ms / 40) * 10 * spec[(x / W * n) | 0]; vx.fillRect(x, Math.round(y), 2, 2); }
  } else if (shape === 'ring') {
    const cx0 = W / 2, cy0 = H / 2 + 10, r0 = 60 + kick * 10;
    for (let i = 0; i < n; i++) { const a = i / n * Math.PI * 2 - Math.PI / 2, len = spec[i] * 70; for (let s = 0; s < len; s += 4) vx.fillRect(Math.round(cx0 + Math.cos(a) * (r0 + s)), Math.round(cy0 + Math.sin(a) * (r0 + s)), 3, 3); }
  } else if (shape === 'spectro') {
    vx.drawImage(vc, -2, 0);
    for (let y = 0; y < H; y += 3) { const v = spec[Math.floor((1 - y / H) * (n - 1))]; vx.globalAlpha = Math.min(1, v * 1.3); vx.fillStyle = v > .7 ? c.red : c.neon; vx.fillRect(W - 2, y, 2, 3); }
    vx.globalAlpha = 1;
  }
}
function loop(now) { drawVis(now); raf = requestAnimationFrame(loop); }
const startVis = () => { if (!raf && $('vRun').checked && !reduce) raf = requestAnimationFrame(loop); };
const stopVis = () => { cancelAnimationFrame(raf); raf = 0; };
whileVisible($('vCanvas'), startVis, stopVis);
$('vRun').onchange = () => $('vRun').checked ? startVis() : stopVis();
$('vCount').oninput = () => { $('vCountOut').textContent = $('vCount').value; spec.fill(0); peak.fill(0); };
$('vShape').onchange = () => { const c = getColours(); vx.fillStyle = c.ground; vx.fillRect(0, 0, vc.width, vc.height); };
$('lMode').onchange = () => setLine(lineIdx);
setLine(0);
if (reduce) { $('vRun').checked = false; synth(performance.now()); drawVis(performance.now()); }

/* ---------------- Every part ---------------- */
const SECTIONS = [
  ['Tourne Studio', 'The control panel, and the switches that reach everything.', ['Tourne Studio', 'Presets', 'Identity', 'Look Groups', 'Extensions', 'Plugins Page', 'Menu Tips', 'Style Apps (Inject)', "Windows' Taskbar Off", 'NVIDIA Displays']],
  ['The bar, Start and the shell', 'Built from nothing, standing in for Explorer.', ['Taskbar', 'Tray Clock', 'Tray Icons', 'Icon Animations', 'Icon Animation Engine', 'Tourne Menu', 'Tourne Menu Settings', 'Classic Start Menu', 'Launcher', 'Shell Mode', 'Awake', 'Desktops Pager', 'Selection Box', "Desktop Icons' Look", 'Scrollbar Dots']],
  ['Flyouts and system icons', 'One click from the tray, and nothing from Windows opens instead.', ['Flyouts', 'Notifications', 'Device Batteries', 'iPhone Link', 'Tourne Calls', 'NVIDIA In The Brightness Flyout', 'Own Brightness, Contrast, Gamma', 'RTX Super Resolution', 'Super Resolution Indicator']],
  ['Windows: switching, tiling, keys', 'How windows move, land, leave and answer the keyboard.', ['Alt+Tab Views', 'Tiling', 'Window Borders', 'Close Effect', 'Key Binds']],
  ['The look, all over', 'One palette, one motion, one glass, on everything.', ['Tourne Theme', 'Motion', 'Glass', 'Screen Shader', 'Living Wallpapers', 'Lock Screen', 'Cursors', 'App Styler', 'Keystrokes']],
  ['Sound', 'A voice for every click that matters.', ['Sounds', 'Sound Engine', 'Typing Sounds']],
  ['Capture, Viewer and Mark Up', 'Grab it, open it, draw on it.', ['Capture', 'Viewer', 'Mark Up']],
  ["Tourne'Table", 'My music, taking over the desktop.', ["Tourne'Table", 'Depth', 'Lyrics', 'Lyric Animation Engine', 'Style Editor', 'Tune By Ear', 'Queue Pop-Ups', 'Media Strip', 'Lyrics By Listening', 'Models', 'Listening Ahead', 'Glass Frame', 'Glass Frame Reflections']],
  ['Little extras', 'Plugins and side pieces that ride along.', ['Notes', 'GMK87', 'Private Local Model']],
  ['Menus and icons', 'Inside Explorer and Windows itself.', ['Tourne Menus', "Tourne'Icons", 'Icon Redirect']],
];
const parts = (window.PARTS || []).slice();
const placed = new Set(SECTIONS.flatMap(s => s[2]));
const extra = parts.filter(p => !placed.has(p.n)).map(p => p.n);
if (extra.length) SECTIONS.push(['More', '', extra]);
$('partCount').textContent = parts.length;
const FROMS = ['All', 'Tourne Studio', 'Tourne Desktop', "Tourne'Table", 'Tourne Menus', "Tourne'Icons", "Tourne'Suite"];
let from = 'All';
const fWrap = $('fFrom');
FROMS.forEach(f => {
  if (f !== 'All' && !parts.some(p => p.f === f)) return;
  const b = el('button', null, f === 'All' ? 'All' : f); b.type = 'button'; b.setAttribute('role', 'radio'); b.setAttribute('aria-checked', f === from);
  b.onclick = () => { from = f; fWrap.querySelectorAll('button').forEach(x => x.setAttribute('aria-checked', x === b)); renderParts(); };
  fWrap.appendChild(b);
});
function splitLine(t) {
  const i = t.indexOf(' [');
  if (i < 0 || !t.trimEnd().endsWith(']')) return { head: t, facts: null };
  let inner = t.slice(i + 2).trim(); inner = inner.slice(0, -1).trim();
  return { head: t.slice(0, i), facts: inner.split(' | ') };
}
function card(p, q) {
  const c = el('article', 'card');
  const top = el('div', 'card-top'); const h4 = el('h4', null, p.n); h4.dataset.tip = p.n + '|Lives in ' + p.f + (p.wip ? '. Work in progress.' : '.'); if (p.wip) h4.append(' ', el('span', 'wip', 'WIP')); top.append(h4, el('span', 'from', p.f)); c.appendChild(top);
  const { head, facts } = splitLine(p.t);
  c.appendChild(el('p', 'line', head));
  if (facts) {
    const ul = el('ul', 'facts');
    ul.appendChild(el('li', 'br', '['));
    facts.forEach(f => ul.appendChild(el('li', null, f)));
    ul.appendChild(el('li', 'br', ']'));
    c.appendChild(ul);
  }
  if (p.o) { const a = el('p', 'also'); a.append(el('b', null, 'also'), document.createTextNode(p.o)); c.appendChild(a); }
  if (q) c.querySelectorAll('h4, .line, .facts li, .also').forEach(n => mark(n, q));
  return c;
}
function mark(node, q) {
  node.childNodes.forEach(ch => {
    if (ch.nodeType !== 3) return;
    const s = ch.textContent, i = s.toLowerCase().indexOf(q);
    if (i < 0) return;
    const m = el('mark', null, s.slice(i, i + q.length)), frag = document.createDocumentFragment();
    frag.append(s.slice(0, i), m, s.slice(i + q.length));
    ch.replaceWith(frag);
  });
}
function renderParts() {
  const q = $('q').value.trim().toLowerCase(), wrap = $('catalog');
  wrap.innerHTML = '';
  let shown = 0;
  SECTIONS.forEach(([title, sub, names]) => {
    const list = names.map(n => parts.find(p => p.n === n)).filter(Boolean)
      .filter(p => from === 'All' || p.f === from)
      .filter(p => !q || (p.n + ' ' + p.t + ' ' + p.o + ' ' + p.f).toLowerCase().includes(q));
    if (!list.length) return;
    shown += list.length;
    const h = el('h3', 'group-h'); h.append(el('span', null, title), el('small', null, sub)); wrap.appendChild(h);
    const g = el('div', 'cards'); list.forEach(p => g.appendChild(card(p, q))); wrap.appendChild(g);
  });
  $('shown').textContent = shown === parts.length ? `${parts.length} parts` : `${shown} of ${parts.length} parts`;
}
$('q').addEventListener('input', renderParts);
renderParts();


/* ---------------- Being added right now (WIP) ---------------- */
const WIP = [
  ['Sounds Everywhere', 'Tourne Desktop', 'Every event its own row, any row any sound', ['Menus, buttons, switches, recording, screenshots', 'Windows opening, closing, moving, lock, sleep, devices', 'Any program\'s menus and buttons too', 'Rapid sounds smoothed and ducked'], 'all rows start on None; flyout sliders tick as they move, the volume tick at the new level'],
  ['Typed Values', 'Tourne Studio', 'Double-click any number and type it', ['Enter keeps, Esc puts it back', 'A wrong value turns red', 'Shift and Ctrl step finely'], ''],
  ['Key Ownership', 'Tourne Desktop', 'My binds can take a key away from Windows', ['Block a key everywhere, or only in named programs', '31 ready-made Windows shortcuts to claim', 'A way out: both Shifts and Esc held'], 'rides the binds engine already running, no new keyboard hook'],
  ['Menu Frame Borders', 'Tourne Desktop', 'Window borders drawn like my menus\' frame', ['Painted on the graphics card', 'Their own thread, nothing runs when off', 'Tiling makes room for them'], ''],
  ['Smooth Capture', 'Tourne Desktop', 'Screen recording past 60 fps', ['143.6 fps measured on a 144 Hz screen', 'The pointer in screenshots'], ''],
  ['Viewer Upgrades', 'Tourne Desktop', 'The Viewer learning more every day', ['Video zoom on the graphics card', 'A file info line and editable tags', 'Remembers audio and subtitle picks', 'SVG and many more picture formats'], ''],
  ['Search By Date', 'Tourne Studio', 'Type "added" and see every setting by the day it arrived', ['"added today", "added nov 17 to nov 18"', 'Matches page and section names too'], ''],
  ['Chained Icon Animations', 'Tourne Desktop', 'Animations chained one after another', ['One pick per icon or per group', 'Viewer and Mark Up buttons move too'], ''],
  ['Glass Frame Reflections', "Tourne'Table", 'More for the glass to mirror', ['Wallpaper, bars\' light, glint, ripple', 'Windows and their colours', 'Full-screen windows: tiled, all or none'], 'with the music paused the bars draw no frames at all'],
  ['Window Previews In Order', 'Tourne Desktop', 'Previews ordered the way windows stand on screen', ['Screen by screen, left to right', 'Minimised last'], ''],
];
const wc = $('wipCards');
WIP.forEach(([n, f, head, facts, also]) => wc.appendChild(card({ n, f, t: head + ' [ ' + facts.join(' | ') + ' ]', o: also, wip: true })));

/* ---------------- Windhawk mods ---------------- */
const REPO = 'https://github.com/USER-TOURNE/Tourne-Style/tree/main/windhawk-mods/';
const MODS_LIST = [
  ["Tourne'Suite", 'tourne-suite', 'Use together', 'Every Tourne part that hooks Windows, in one mod, each part a switch: tray icons and folders, system icons, text colour, menus without icons or doubles, the clock, Icon Redirect, the switcher, Explorer style and colours, theme gaps, dark mode fixes, classic menus, square corners, Tweaks, and a classic taskbar (WIP).'],
  ['Tourne Menus', 'tourne-menus', 'Use together', "Explorer's, the file dialogs' and Windows' menus in Tourne Desktop's look."],
  ["Tourne'Icons", 'tourne-icons', 'Use together', 'Change any icon everywhere from a picker in the tray; applies icon packs. With Tourne\'Icons Lasting and the Kit.'],
  ["Tourne'Table", 'tourne-table', 'Use together', 'The desktop audio visualizer as a mod of its own, with community styles.'],
  ["Tourne'Tray [System Icons]", 'tourne-tray-system-icons', 'Also a suite part', 'Pixel-art tray icons with live state, each with a Tourne flyout.'],
  ["Tourne'Text [System Text Color]", 'tourne-system-text-color', 'Also a suite part', "Recolours white system text, Chrome's menus included."],
  ["Tourne'Tray [Change Tray Icons]", 'tourne-change-tray-icons', 'Also a suite part', "Swap any app's tray icon for your own."],
  ["Tourne'Menu [No Icons]", 'tourne-menu-no-icons', 'Also a suite part', 'Context menus without icons.'],
  ["Tourne'Menu [No Duplicates]", 'tourne-menu-no-duplicates', 'Also a suite part', 'Removes doubled context menu entries.'],
  ["Tourne'Tray [Classic Tray Folders]", 'tourne-classic-tray-folders', 'Also a suite part', "Group tray icons into folders in StartAllBack's classic tray."],
  ["Tourne'Tray [Clock]", 'tourne-tray-clock', 'Also a suite part', "Restyles StartAllBack's classic clock."],
  ["Tourne'Icons [Icon Redirect]", 'tourne-icon-redirect', 'Also a suite part', 'Icon themes by redirecting resources, my own rewrite.'],
  ["Tourne'Suite Tweaks", 'tourne-tweaks', 'Also a suite part', "Winaero Tweaker's options as switches."],
  ['Tourne Drop Install', 'tourne-drop-install', 'On its own', 'A small window: drop .wh.cpp files on it to install them.'],
  ['Tourne Mod Tray', 'tourne-mod-tray', 'On its own', 'A tray icon with a right-click list of your mods to switch on and off.'],
  ['Hide the app icon in taskbar thumbnail previews', 'taskbar-thumbnail-hide-icon', 'On its own', 'Removes the small app icon from taskbar previews.'],
  ['Tourne Lock Screen', '', 'Not public yet', "The lock screen's clock and media player in Tourne's look, with glass, set from Tourne Studio."],
  ['Tourne Lock Screen Motion', '', 'Not public yet', 'My living wallpaper, looped, behind the lock screen and the sign-in screen, recorded by Tourne Studio.'],
  ["Tourne'Icons Lasting", '', 'Not public yet', "Program, folder and file type icons from one picker, written into Windows' own settings so they stay with the mod off."],
  ["Tourne'Task Manager graphs", '', 'Not public yet', "Task Manager's performance graphs in Tourne's look: no grid, the activity filled as Tourne's hover."],
  ["Tourne'Switcher", '', 'Also a suite part', 'An Alt+Tab switcher in the Tourne style, with tiling and Hyprland-style binds.'],
  ['Tourne Square Corners', '', 'Also a suite part', "Windows 11's rounded corners made square again, in every program, except the ones I leave out."],
  ['Tourne Classic Menus', '', 'Also a suite part', "Windows' own menus drawn as classic menus everywhere, instead of the immersive ones."],
  ['Tourne Dark Magic', '', 'Also a suite part', "Dark mode for Windows' classic programs and dialogs."],
  ['Tourne Explorer Colours', '', 'Also a suite part', "Explorer's drive bars, selections and preview pane in my own colours, keeping the visual style's shapes."],
  ['Tourne Explorer Style', '', 'Also a suite part', "Explorer's windows in a full visual style of my choosing, without patching Windows."],
  ['Tourne Theme Gaps', '', 'Also a suite part', "The parts of Windows a visual style doesn't reach, filled in from the style's own colours."],
  ["Tourne'Table two-way sync", '', 'Not public yet', 'The Windhawk Table beta that keeps its setups in step with the Table built into Tourne Desktop.'],
  ["Tourne'Taskbar", '', 'WIP', 'A classic taskbar drawn from StartAllBack themes, as a second bar beside the real one.'],
];
const mg = $('modGrid');
MODS_LIST.forEach(([n, dir, kind, what]) => {
  const a = el('a', 'mod'); a.href = REPO + dir; a.dataset.tip = n + '|' + kind + (dir ? '. Opens its folder with the .wh.cpp and README.' : '. Running in my workspace; its folder comes when it is published.');
  a.append(el('span', 'kind', kind.toUpperCase()), el('b', null, n), el('small', null, what));
  mg.appendChild(a);
});
const SUITE = [
  ['System Icons', 'Pixel-art tray icons with live state, and a flyout for each', 'Its own process'],
  ['Explorer relay', 'The jobs that have to happen inside Explorer', 'Explorer, while System Icons is on'],
  ['Classic Tray Folders, Clock', 'Tray folders; the classic clock restyled', 'Explorer'],
  ['Change Tray Icons', "Any app's tray icon swapped for a themed one", 'Apps that have a row in its list'],
  ['System Text Color, Menu No Icons, Menu No Duplicates', 'Text colour; menus without icons or doubles', 'Their Processes lists'],
  ['Icon Redirect', 'Icon themes and single-app icon swaps', 'Every program, while it has a theme or a row'],
  ['Tweaks', "Winaero Tweaker's options, 312 of them", "Windhawk's own process; every program for the live font swap"],
  ['Square Corners', 'Every window and menu at 0 corner radius, except programs you exclude', 'Every program'],
  ['Explorer Style and Colors, Theme Gaps', "Explorer and Windows' own theme parts in my palette", 'Explorer and themed programs'],
];
const sr = $('suiteRows');
SUITE.forEach(r => { const tr = el('tr'); r.forEach(c => tr.appendChild(el('td', null, c))); sr.appendChild(tr); });

// Repaint the canvases when the palette changes.
new MutationObserver(() => { colours = null; drawGraph(); if (!raf) { drawDesk(cx); const { off } = blocks(5); cx.drawImage(off, WX, WY); } }).observe(document.documentElement, { attributes: true, attributeFilter: ['style'] });
/* ---------------- Tooltips: the Menu Tips look, on every piece of this page ---------------- */
const STATIC_TIPS = {
  '.nav a[href="#map"]': 'Ecosystem|How the pieces fit together.',
  '.nav a[href="#engines"]': 'Engines|Working copies of the real engines.',
  '.nav a[href="#parts"]': 'Every part|All 73 parts in my own words.',
  '.nav a[href="#mods"]': 'Windhawk mods|The mods that reach inside Windows.',
  '.nav a[href="#get"]': 'Get it|Downloads and source.',
  '#mCurve': 'Curve|How a move speeds up and settles. Each motion line picks its own.',
  '#mDur': 'Length|How long the move takes, in ms. Typed or dragged.',
  '#mWhat': 'What moves|The same curve on different parts of the desktop.',
  '#mPlay': 'Play|Runs the move with this curve and length.',
  '#mGraph': 'The curve|Time runs left to right; the lines are start and end.',
  '#cOn': 'On|Close, minimise or restore. Restore plays the effect backwards.',
  '#cPlay': 'Close the window|Plays the close effect you picked.',
  '#compare': 'Compare|Plays the last cue in all 12 feels, one after another.',
  '#vol': 'Volume|For this page only.',
  '#tLayout': 'Layout|15 layouts, the same maths as Tourne Desktop.',
  '#tAdd': 'Open a window|It tiles itself into the layout.',
  '#tKill': 'Close the focused one|The rest fill the space.',
  '#tGap': 'Gaps|Space between windows and round the screen.',
  '#tAuto': 'By window count|1 monocle, 2 columns, 3 a stack, 4 a grid, more BSP.',
  '#tBorder': 'Focus border|A glowing frame on the window in front.',
  '#tSound': 'Snap sound|Plays the Snap cue from the feel picked above.',
  '#tStage': 'The screen|Click it, then press a bound key.',
  '#binds': 'Binds file|Edit any line; it applies as you type.',
  '#bReset': 'Put the file back|Restores the sample binds.',
  '#bLog': 'What ran|Each key press and the command it ran.',
  '#sPrev': 'Back|The previous window.', '#sNext': 'Next|The next window.',
  '#sDim': 'Dim behind|Blurs and dims the screen while switching.',
  '#iAnim': 'Animation|How the icons move.', '#iWhen': 'When|Hover, press, the beat, or all the time.',
  '#iSpd': 'Speed|Faster or slower.',
  '#vShape': 'Shape|7 of the Table\'s 18 shapes.', '#vCount': 'Bars|How many bars across.',
  '#lMode': 'Lyric|How each line comes in, sung colour paced to the beat.',
  '#vRun': 'Test beat|A made-up 120 BPM beat. Off means no work at all.',
  '#q': 'Search|Any part, word or number.',
  '#repoBtn': 'Source|The public Tourne-Style repo.',
  '#tipStyles': 'Tooltip look|Pick how these tips look, as Menu Tips does per part.',
};
function staticTips() {
  Object.entries(STATIC_TIPS).forEach(([sel, t]) => document.querySelectorAll(sel).forEach(n => { if (!n.dataset.tip) n.dataset.tip = t; }));
  document.querySelectorAll('.nums summary').forEach(s => s.dataset.tip = s.querySelector('.k').textContent.replace('> ', '') + '|Open for what\'s inside, custom values, and import and export.');
  document.querySelectorAll('#fFrom button').forEach(b => b.dataset.tip = b.textContent + '|Shows only the parts that live here.');
  document.querySelectorAll('.node').forEach(n => n.dataset.tip = n.querySelector('h3').textContent + '|' + n.dataset.label.toLowerCase());
}
staticTips();
const tip = el('div'); tip.id = 'tip'; tip.setAttribute('role', 'tooltip'); document.body.appendChild(tip);
let tipFor = null, tipX = 0, tipY = 0;
function showTip(node, x, y) {
  const [title, ...rest] = node.dataset.tip.split('|');
  tip.innerHTML = '';
  const b = el('b', null, title); tip.appendChild(b);
  if (rest.length) tip.appendChild(el('span', null, rest.join('|')));
  tipFor = node; node.setAttribute('aria-describedby', 'tip');
  place(x, y); tip.classList.add('on');
}
function place(x, y) {
  const r = tip.getBoundingClientRect(), W = innerWidth, H = innerHeight;
  let left = x + 14, top = y + 18;
  if (left + r.width > W - 8) left = Math.max(8, W - r.width - 8);
  if (top + r.height > H - 8) top = Math.max(8, y - r.height - 10);
  tip.style.left = left + 'px'; tip.style.top = top + 'px';
}
function hideTip() { if (tipFor) tipFor.removeAttribute('aria-describedby'); tipFor = null; tip.classList.remove('on'); }
document.addEventListener('pointerover', e => {
  if (e.pointerType === 'touch') return;
  const n = e.target.closest && e.target.closest('[data-tip]');
  if (!n) { hideTip(); return; }
  if (n !== tipFor) showTip(n, e.clientX, e.clientY);
});
document.addEventListener('pointermove', e => { if (tipFor && e.pointerType !== 'touch') place(e.clientX, e.clientY); }, { passive: true });
document.addEventListener('pointerleave', hideTip);
document.addEventListener('focusin', e => { const n = e.target.closest && e.target.closest('[data-tip]'); if (n && n.matches(':focus-visible')) { const r = n.getBoundingClientRect(); showTip(n, r.left, r.bottom - 10); } });
document.addEventListener('focusout', hideTip);
addEventListener('scroll', hideTip, { passive: true });
document.addEventListener('keydown', e => { if (e.key === 'Escape') hideTip(); });
const tsWrap = $('tipStyles');
[['glass', 'Glass'], ['frame', 'Frame'], ['terminal', 'Terminal']].forEach(([id, label], i) => {
  const b = el('button', null, label); b.type = 'button'; b.setAttribute('role', 'radio'); b.setAttribute('aria-checked', i === 0);
  b.dataset.tip = label + '|' + ({ glass: 'Frosted glass in my menus\' frame. The default.', frame: 'A solid panel with a hard shadow.', terminal: 'Plain ground with a prompt mark.' })[id];
  b.onclick = () => { tip.dataset.look = id; tsWrap.querySelectorAll('button').forEach(x => x.setAttribute('aria-checked', x === b)); showTip(b, b.getBoundingClientRect().left, b.getBoundingClientRect().bottom - 10); };
  tsWrap.appendChild(b);
});
})();
