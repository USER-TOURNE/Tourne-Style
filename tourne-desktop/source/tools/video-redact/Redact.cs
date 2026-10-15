using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;

// Pixelates long values (file paths) in the Windhawk settings text boxes of
// the settings-tour video. ffmpeg decodes to raw BGR, this edits the frames,
// and a second ffmpeg encodes them.
//
// Only the settings' text boxes have a 1px grey (67) border, so a row inside
// a box has a grey pixel near its left and near its right, with the inside
// not grey. Found row by row, so half-scrolled boxes count too. Boxes whose
// text is long get pixelated; each frame also takes the boxes found in the
// frames either side, so one missed frame mid-scroll can't show anything.
// The floating "Create a new mod" pill is put back on top afterwards.
public static class Redact {
    public const int W = 944, H = 1080;
    const int LongText = 120;                       // px of text that counts as a path
    const int WindowEdge = 930;                     // the window's own border column starts here
    // The floating button: a pill, in video coordinates.
    const int PillLeft = 733, PillRight = 903, PillTop = 987, PillBottom = 1024;

    struct Box { public int L, T, R, B; }

    static bool Border(byte[] f, int x, int y) {
        int i = (y * W + x) * 3;
        int b = f[i], g = f[i + 1], r = f[i + 2];
        return r >= 50 && r <= 88 && Math.Abs(r - g) <= 10 && Math.Abs(g - b) <= 10;
    }

    static bool Lit(byte[] f, int x, int y) {
        int i = (y * W + x) * 3;
        return f[i] + f[i + 1] + f[i + 2] > 3 * 70;
    }

    static bool InPill(int x, int y) {
        if (y < PillTop || y > PillBottom || x < PillLeft || x > PillRight) return false;
        double r = (PillBottom - PillTop) / 2.0, cy = PillTop + r;
        if (x < PillLeft + r) { double dx = PillLeft + r - x, dy = y - cy; return dx * dx + dy * dy <= r * r; }
        if (x > PillRight - r) { double dx = x - (PillRight - r), dy = y - cy; return dx * dx + dy * dy <= r * r; }
        return true;
    }

    // Where the settings' text boxes have their side borders (video x),
    // measured from the video: the usual boxes, nested ones, boxes beside a
    // file button, and the Theme paths list's wider boxes. Header buttons
    // have borders too, at other places, so only these count.
    static readonly int[] Lefts = {82, 107, 114, 126, 128};
    static readonly int[] Rights = {851, 870};

    static int EdgeAt(byte[] f, int y, int[] edges) {
        int found = -1;
        foreach (int e in edges)
            for (int x = e - 2; x <= e + 2; x++)
                if (x > found && Border(f, x, y) && !InPill(x, y)) found = x;
        return found;
    }

    // Left and right border of the box this row is inside.
    static bool Row(byte[] f, int y, out int left, out int right) {
        left = EdgeAt(f, y, Lefts);
        right = -1;
        if (left < 0) return false;
        right = EdgeAt(f, y, Rights);
        bool underPill = y >= PillTop - 4 && y <= PillBottom + 4;
        if (right < 0 && underPill) right = 850;   // the pill hides the usual right border
        if (right < 0) return false;
        // A horizontal border row is grey all along; the inside of a box isn't.
        int grey = 0, samples = 0;
        for (int x = left + 4; x < right - 4; x += 7) {
            if (InPill(x, y)) continue;
            samples++;
            if (Border(f, x, y)) grey++;
        }
        return samples > 0 && grey < samples / 2;
    }

    // A horizontal box border on row y: grey from a known right edge back to
    // a known left edge (gaps of a few pixels allowed for compression). Beside
    // the floating pill, the line is followed from the pill's left instead.
    static bool Line(byte[] f, int y, out int left, out int right) {
        left = right = -1;
        bool band = y >= PillTop - 2 && y <= PillBottom + 2;
        foreach (int r0 in Rights) {
            int start = -1;
            for (int x = r0 + 2; x >= r0 - 2; x--) if (Border(f, x, y)) { start = x; break; }
            if (start < 0 && band) start = PillLeft - 3;
            if (start < 0) continue;
            // Compression can break the line up, so gaps of up to 12 px are
            // allowed as long as at least half of it is grey.
            int x2 = start, lastGrey = start, misses = 0, grey = 0;
            while (x2 > 40 && misses <= 12) {
                if (Border(f, x2, y)) { misses = 0; lastGrey = x2; grey++; } else misses++;
                x2--;
            }
            if (start - lastGrey < 150 || grey * 2 < start - lastGrey) continue;
            foreach (int l0 in Lefts)
                if (Math.Abs(lastGrey - l0) <= 3) { left = l0; right = r0; return true; }
        }
        return false;
    }

    // How far the box's side border runs from row y in direction dy.
    static int Extend(byte[] f, int y, int dy, int l, int r) {
        int last = y;
        for (int k = 1; k <= 30; k++) {
            int yy = y + dy * k;
            if (yy < 0 || yy >= H) break;
            bool side = false;
            for (int x = l - 2; x <= l + 2 && !side; x++) side = Border(f, x, yy);
            for (int x = r - 2; x <= r + 2 && !side; x++) side = !InPill(x, yy) && Border(f, x, yy);
            if (!side) break;
            last = yy;
        }
        return last;
    }

    // Does the text in this box have a backslash? At this size it's a thin
    // stroke from the top of the capitals to below the baseline (about 12
    // rows), drifting 3 to 8 px right. Found as the longest chain of lit
    // pixels going down one row at a time, each step straight down or one to
    // the right. X and N are too short, "/" leans the other way, and l, | and
    // [ don't drift.
    static bool HasBackslash(byte[] f, Box b) {
        int x0 = b.L + 1, x1 = b.R - 1, y0 = b.T, y1 = b.B;
        int w = x1 - x0, h = y1 - y0;
        if (w <= 0 || h < 11) return false;
        var len = new int[h, w];
        var drift = new int[h, w];
        for (int yy = h - 1; yy >= 0; yy--)
            for (int xx = 0; xx < w; xx++) {
                int x = x0 + xx, y = y0 + yy;
                if (InPill(x, y) || !Lit(f, x, y)) continue;
                int bestLen = 1, bestDrift = 0;
                if (yy + 1 < h) {
                    if (len[yy + 1, xx] + 1 > bestLen) { bestLen = len[yy + 1, xx] + 1; bestDrift = drift[yy + 1, xx]; }
                    if (xx + 1 < w && (len[yy + 1, xx + 1] + 1 > bestLen ||
                                       (len[yy + 1, xx + 1] + 1 == bestLen && drift[yy + 1, xx + 1] + 1 > bestDrift))) {
                        bestLen = len[yy + 1, xx + 1] + 1; bestDrift = drift[yy + 1, xx + 1] + 1;
                    }
                }
                len[yy, xx] = bestLen; drift[yy, xx] = bestDrift;
                if (bestLen >= 11 && bestDrift >= 3 && bestDrift <= 8) return true;
            }
        return false;
    }
    static List<Box> Find(byte[] f) { return Find(f, true); }

    static List<Box> Find(byte[] f, bool longOnly) {
        // Border lines (one per line; a line can be 2 rows thick after scaling).
        var lines = new List<int[]>();
        for (int y = 0; y < H; y++) {
            int l, r;
            if (!Line(f, y, out l, out r)) continue;
            if (lines.Count > 0) {
                var p = lines[lines.Count - 1];
                if (p[1] == l && p[2] == r && y - p[3] <= 1) { p[3] = y; continue; }
            }
            lines.Add(new[] { y, l, r, y });
        }
        var boxes = new List<Box>();
        var used = new bool[lines.Count];
        for (int i = 0; i < lines.Count; i++) {
            if (used[i]) continue;
            int top = -1, bottom = -1, l = lines[i][1], r = lines[i][2];
            bool clipped = false;
            for (int j = i + 1; j < lines.Count; j++) {
                int gap = lines[j][0] - lines[i][3];
                if (gap > 32) break;
                if (lines[j][1] == l && lines[j][2] == r && gap >= 14) {
                    top = lines[i][3]; bottom = lines[j][0]; used[j] = true; break;
                }
            }
            if (top < 0) {
                // Only one border in view: the box runs on while its side border does.
                int down = Extend(f, lines[i][3], 1, l, r), up = Extend(f, lines[i][0], -1, l, r);
                if (down - lines[i][3] >= lines[i][0] - up && down > lines[i][3] + 2) { top = lines[i][3]; bottom = down + 1; }
                else if (lines[i][0] - up > 2) { top = up - 1; bottom = lines[i][0]; }
                else continue;
                clipped = true;
            }
            used[i] = true;
            var b = new Box { L = l + 2, T = Math.Max(0, top + 1), R = r - 1, B = Math.Min(H, bottom) };
            if (b.B <= b.T) continue;
            if (longOnly) {
                int minX = int.MaxValue, maxX = -1;
                for (int yy = b.T; yy < b.B; yy++)
                    for (int x = b.L + 1; x < b.R - 1; x++)
                        if (!InPill(x, yy) && Lit(f, x, yy)) { if (x < minX) minX = x; if (x > maxX) maxX = x; }
                if (maxX - minX < LongText) continue;
                // Half a line can't be judged, so a box cut off by the edge of
                // the scroll area is hidden whenever its text is long.
                if (b.B - b.T < 18) clipped = true;
                if (!clipped && !HasBackslash(f, b)) continue;
            }
            boxes.Add(b);
        }
        return boxes;
    }
    static void Pixelate(byte[] f, Box b) {
        const int S = 12;
        for (int by = b.T; by < b.B; by += S)
            for (int bx = b.L; bx < b.R; bx += S) {
                int ex = Math.Min(bx + S, b.R), ey = Math.Min(by + S, b.B);
                long sb = 0, sg = 0, sr = 0; int n = 0;
                for (int yy = by; yy < ey; yy++)
                    for (int xx = bx; xx < ex; xx++) {
                        int i = (yy * W + xx) * 3;
                        sb += f[i]; sg += f[i + 1]; sr += f[i + 2]; n++;
                    }
                if (n == 0) continue;
                byte ab = (byte)(sb / n), ag = (byte)(sg / n), ar = (byte)(sr / n);
                for (int yy = by; yy < ey; yy++)
                    for (int xx = bx; xx < ex; xx++) {
                        int i = (yy * W + xx) * 3;
                        f[i] = ab; f[i + 1] = ag; f[i + 2] = ar;
                    }
            }
    }

    static void Apply(byte[] frame, byte[] original, IEnumerable<List<Box>> sets) {
        foreach (var set in sets)
            foreach (var b in set) Pixelate(frame, b);
        // The pill sits on top of everything, as on screen.
        for (int y = PillTop; y <= PillBottom; y++)
            for (int x = PillLeft; x <= PillRight; x++)
                if (InPill(x, y)) {
                    int i = (y * W + x) * 3;
                    frame[i] = original[i]; frame[i + 1] = original[i + 1]; frame[i + 2] = original[i + 2];
                }
    }

    static bool ReadFrame(Stream s, byte[] buf) {
        int got = 0;
        while (got < buf.Length) {
            int n = s.Read(buf, got, buf.Length - got);
            if (n <= 0) return false;
            got += n;
        }
        return true;
    }

    public static string Run(string ffmpeg, string input, string output, string crop) {
        var dec = Start(ffmpeg, "-v error -i \"" + input + "\" -vf \"" + crop + "\" -an -f rawvideo -pix_fmt bgr24 -", false);
        var enc = Start(ffmpeg, "-v error -y -f rawvideo -pix_fmt bgr24 -s " + W + "x" + H +
                        " -r 30 -i - -c:v libx264 -preset slow -crf 22 -pix_fmt yuv420p -movflags +faststart \"" + output + "\"", true);
        Stream src = dec.StandardOutput.BaseStream, dst = enc.StandardInput.BaseStream;

        // A window of three frames: previous, current, next.
        var frames = new List<byte[]>();
        var found = new List<List<Box>>();
        int written = 0, hidden = 0;
        bool more = true;
        while (more || frames.Count > 0) {
            if (more) {
                var buf = new byte[W * H * 3];
                if (ReadFrame(src, buf)) { frames.Add(buf); found.Add(Find(buf)); }
                else more = false;
            }
            // Write the middle frame once its neighbour is known (or at the end).
            int cur = written == 0 ? 0 : 1;
            if (frames.Count > cur + 1 || (!more && frames.Count > cur)) {
                var sets = new List<List<Box>>();
                for (int k = Math.Max(0, cur - 1); k <= Math.Min(frames.Count - 1, cur + 1); k++) sets.Add(found[k]);
                byte[] outFrame = (byte[])frames[cur].Clone();
                Apply(outFrame, frames[cur], sets);
                if (found[cur].Count > 0) hidden++;
                dst.Write(outFrame, 0, outFrame.Length);
                written++;
                if (cur == 1) { frames.RemoveAt(0); found.RemoveAt(0); }
                if (!more && frames.Count == 1 && written > 1) { frames.Clear(); found.Clear(); }
            } else if (!more) {
                break;
            }
        }
        dst.Close();
        dec.WaitForExit();
        enc.WaitForExit();
        return string.Format("{0} frames written, {1} with paths found", written, hidden);
    }

    // Leak check on a finished video: boxes whose inside still has sharp text
    // edges (a pixelated box only changes colour every 12 px, softly).
    public static string Check(string ffmpeg, string video, string dumpDir) {
        var dec = Start(ffmpeg, "-v error -i \"" + video + "\" -f rawvideo -pix_fmt bgr24 -", false);
        Stream src = dec.StandardOutput.BaseStream;
        var buf = new byte[W * H * 3];
        int frame = 0, bad = 0, dumped = 0;
        var list = new List<string>();
        while (ReadFrame(src, buf)) {
            foreach (var b in Find(buf, true)) {
                int sharp = 0;
                for (int y = b.T; y < b.B; y++)
                    for (int x = b.L + 1; x < b.R; x++) {
                        if (InPill(x, y) || InPill(x - 1, y)) continue;
                        int i = (y * W + x) * 3, j = i - 3;
                        int d = Math.Abs(buf[i] - buf[j]) + Math.Abs(buf[i + 1] - buf[j + 1]) + Math.Abs(buf[i + 2] - buf[j + 2]);
                        if (d > 150) sharp++;
                    }
                if (sharp > 60) {
                    bad++;
                    if (list.Count < 40) list.Add(string.Format("frame {0} ({1:F1}s) box y{2}-{3} sharp={4}", frame, frame / 30.0, b.T, b.B, sharp));
                    if (dumped < 12) { Dump(buf, b, Path.Combine(dumpDir, "leak_" + frame + "_" + b.T + ".bmp")); dumped++; }
                }
            }
            frame++;
        }
        dec.WaitForExit();
        return string.Format("{0} frames checked, {1} boxes with sharp text\n{2}", frame, bad, string.Join("\n", list));
    }

    public static string Sample(string ffmpeg, string input, string crop, string dir) {
        var dec = Start(ffmpeg, "-v error -i \"" + input + "\" -vf \"" + crop + "\" -an -f rawvideo -pix_fmt bgr24 -", false);
        Stream src = dec.StandardOutput.BaseStream;
        var buf = new byte[W * H * 3];
        int frame = 0, paths = 0, kept = 0;
        while (ReadFrame(src, buf)) {
            if (frame % 90 == 0) {
                foreach (var b in FindLong(buf)) {
                    bool p = HasBackslash(buf, b);
                    if (p) paths++; else kept++;
                    Dump(buf, b, Path.Combine(dir, (p ? "P_" : "K_") + frame + "_" + b.T + ".bmp"));
                }
            }
            frame++;
        }
        dec.WaitForExit();
        return paths + " path boxes, " + kept + " kept sharp";
    }

    // Long-text boxes, path or not.
    static List<Box> FindLong(byte[] f) {
        var all = new List<Box>();
        foreach (var b in Find(f, false)) {
            int minX = int.MaxValue, maxX = -1;
            for (int yy = b.T; yy < b.B; yy++)
                for (int x = b.L + 1; x < b.R - 1; x++)
                    if (!InPill(x, yy) && Lit(f, x, yy)) { if (x < minX) minX = x; if (x > maxX) maxX = x; }
            if (maxX - minX >= LongText) all.Add(b);
        }
        return all;
    }
    public static string DebugFrame(string ffmpeg, string input, string crop, int n, int y0, int y1) {
        var dec = Start(ffmpeg, "-v error -i \"" + input + "\" -vf \"" + crop + "\" -an -f rawvideo -pix_fmt bgr24 -", false);
        var buf = new byte[W * H * 3];
        for (int i = 0; i <= n; i++) ReadFrame(dec.StandardOutput.BaseStream, buf);
        try { dec.Kill(); } catch { }
        var sb = new System.Text.StringBuilder();
        for (int y = y0; y <= y1; y++) {
            int l, r;
            bool line = Line(buf, y, out l, out r);
            var greys = new List<int>();
            for (int x = 0; x < W; x++) if (Border(buf, x, y)) greys.Add(x);
            string g = greys.Count > 10 ? greys[0] + ".." + greys[greys.Count - 1] + " (" + greys.Count + ")" : string.Join(" ", greys);
            sb.AppendFormat("{0}: line={1} {2}-{3}  grey: {4}\n", y, line, l, r, g);
        }
        foreach (var b in Find(buf, false)) {
            bool bs = HasBackslash(buf, b);
            sb.AppendFormat("box L{0} T{1} R{2} B{3} backslash={4}\n", b.L, b.T, b.R, b.B, bs);
        }
        return sb.ToString();
    }
    static void Dump(byte[] f, Box b, string path) {
        int w = b.R - b.L, h = b.B - b.T;
        int stride = (w * 3 + 3) & ~3;
        using (var fs = File.Create(path))
        using (var bw = new BinaryWriter(fs)) {
            bw.Write((ushort)0x4D42); bw.Write(54 + stride * h); bw.Write(0); bw.Write(54);
            bw.Write(40); bw.Write(w); bw.Write(h); bw.Write((ushort)1); bw.Write((ushort)24); bw.Write(0);
            bw.Write(stride * h); bw.Write(0); bw.Write(0); bw.Write(0); bw.Write(0);
            var row = new byte[stride];
            for (int y = b.B - 1; y >= b.T; y--) {
                Array.Copy(f, (y * W + b.L) * 3, row, 0, w * 3);
                bw.Write(row);
            }
        }
    }

    static Process Start(string exe, string args, bool input) {
        var psi = new ProcessStartInfo(exe, args) {
            UseShellExecute = false, CreateNoWindow = true,
            RedirectStandardOutput = !input, RedirectStandardInput = input
        };
        return Process.Start(psi);
    }
}
