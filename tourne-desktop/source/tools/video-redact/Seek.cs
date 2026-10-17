using System;
using System.Diagnostics;
using System.IO;

// Looks for one piece of rendered text (a template cut from a frame) in every
// frame of a video, by normalised cross-correlation on brightness. Used to
// prove "%USERPROFILE%" never shows in the redacted video: run it on the
// original first (it must find it), then on the result (it must not).
public static class Seek {
    const int W = 944, H = 1080;

    static float[] Gray(byte[] f) {
        var g = new float[W * H];
        for (int i = 0; i < W * H; i++) g[i] = (f[i * 3] + f[i * 3 + 1] + f[i * 3 + 2]) / 3f;
        return g;
    }

    static bool Read(Stream s, byte[] buf) {
        int got = 0;
        while (got < buf.Length) {
            int n = s.Read(buf, got, buf.Length - got);
            if (n <= 0) return false;
            got += n;
        }
        return true;
    }

    static Process Decode(string ffmpeg, string video, string filter) {
        var psi = new ProcessStartInfo(ffmpeg, "-v error -i \"" + video + "\"" + (filter != null ? " -vf \"" + filter + "\"" : "") +
                                       " -an -f rawvideo -pix_fmt bgr24 -") {
            UseShellExecute = false, CreateNoWindow = true, RedirectStandardOutput = true
        };
        return Process.Start(psi);
    }

    // The template: tw x th at (tx, ty) of frame number n.
    public static float[] Template(string ffmpeg, string video, string filter, int n, int tx, int ty, int tw, int th) {
        var p = Decode(ffmpeg, video, filter);
        var buf = new byte[W * H * 3];
        for (int i = 0; i <= n; i++) Read(p.StandardOutput.BaseStream, buf);
        try { p.Kill(); } catch { }
        var g = Gray(buf);
        var t = new float[tw * th];
        for (int y = 0; y < th; y++)
            for (int x = 0; x < tw; x++) t[y * tw + x] = g[(ty + y) * W + tx + x];
        return t;
    }

    public static int TextStart(string ffmpeg, string video, string filter, int n, int y0, int rows) {
        var p = Decode(ffmpeg, video, filter);
        var buf = new byte[W * H * 3];
        for (int i = 0; i <= n; i++) Read(p.StandardOutput.BaseStream, buf);
        try { p.Kill(); } catch { }
        var g = Gray(buf);
        int best = int.MaxValue;
        for (int y = y0; y < y0 + rows; y++)
            for (int x = 80; x < 240; x++) if (g[y * W + x] > 90) { best = Math.Min(best, x); break; }
        return best;
    }

    // Best match per frame; reports frames scoring over the threshold.
    public static string Scan(string ffmpeg, string video, string filter, float[] t, int tw, int th, double threshold) {
        double tm = 0; foreach (var v in t) tm += v; tm /= t.Length;
        double ts = 0; foreach (var v in t) ts += (v - tm) * (v - tm); ts = Math.Sqrt(ts);
        var p = Decode(ffmpeg, video, filter);
        var buf = new byte[W * H * 3];
        int frame = 0, hits = 0;
        double best = 0; int bestFrame = -1;
        var sb = new System.Text.StringBuilder();
        while (Read(p.StandardOutput.BaseStream, buf)) {
            var g = Gray(buf);
            double frameBest = 0; int fx = 0, fy = 0;
            // Where each row's text starts (first bright pixel from x 80).
            var first = new int[H];
            for (int y = 0; y < H; y++) {
                first[y] = -1;
                for (int x = 80; x < 240; x++) if (g[y * W + x] > 90) { first[y] = x; break; }
            }
            for (int y = 0; y + th <= H; y++) {
                int start = -1;
                for (int yy = 0; yy < th && start < 0; yy++) if (first[y + yy] >= 0) start = first[y + yy];
                if (start < 0) continue;
                for (int x = start - 4; x <= start + 12; x++) {
                    if (x < 0 || x + tw > W) continue;
                    double m = 0;
                    for (int yy = 0; yy < th; yy++)
                        for (int xx = 0; xx < tw; xx++) m += g[(y + yy) * W + x + xx];
                    m /= t.Length;
                    double num = 0, ss = 0;
                    for (int yy = 0; yy < th; yy++)
                        for (int xx = 0; xx < tw; xx++) {
                            double a = g[(y + yy) * W + x + xx] - m;
                            num += a * (t[yy * tw + xx] - tm);
                            ss += a * a;
                        }
                    if (ss < 1) continue;
                    double ncc = num / (Math.Sqrt(ss) * ts);
                    if (ncc > frameBest) { frameBest = ncc; fx = x; fy = y; }
                }
            }            if (frameBest > best) { best = frameBest; bestFrame = frame; }
            if (frameBest >= threshold) {
                hits++;
                if (hits <= 15) sb.AppendFormat("  frame {0} ({1:F1}s) score {2:F2} at {3},{4}\n", frame, frame / 30.0, frameBest, fx, fy);
            }
            frame++;
        }
        p.WaitForExit();
        return string.Format("{0} frames, {1} with a match >= {2:F2}; best {3:F2} at frame {4}\n{5}",
                             frame, hits, threshold, best, bestFrame, sb);
    }
}
