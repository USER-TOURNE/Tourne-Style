using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.Runtime.InteropServices;

public static class Grab {
    [DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
    [DllImport("user32.dll")] public static extern IntPtr FindWindow(string c, string t);
    [DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr h, int c);
    [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out RECT r);
    [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr h);
    [DllImport("kernel32.dll")] public static extern void Sleep(uint ms);
    public struct RECT { public int l, t, r, b; }

    // StartAllBack's own taskbar window, shown for a moment and photographed.
    [DllImport("user32.dll")] static extern bool EnumWindows(EnumProc p, IntPtr l);
    [DllImport("user32.dll", CharSet = CharSet.Unicode)] static extern int GetClassName(IntPtr h, System.Text.StringBuilder s, int n);
    [DllImport("user32.dll")] static extern bool SetWindowPos(IntPtr h, IntPtr a, int x, int y, int cx, int cy, uint f);
    delegate bool EnumProc(IntPtr h, IntPtr l);

    // Our own bar (the suite's, if running) sits over StartAllBack's, so it's put away for the photograph.
    static IntPtr[] OurBars() {
        var list = new IntPtr[16]; int n = 0;
        EnumWindows((h, l) => {
            var s = new System.Text.StringBuilder(128);
            GetClassName(h, s, 128);
            if (s.ToString().StartsWith("TourneTaskbarTest_") && IsWindowVisible(h) && n < 16) list[n++] = h;
            return true;
        }, IntPtr.Zero);
        var found = new IntPtr[n]; for (int i = 0; i < n; i++) found[i] = list[i]; return found;
    }

    // StartAllBack's own taskbar window, shown for a moment and photographed.
    [DllImport("user32.dll")] static extern bool SetCursorPos(int x, int y);
    [DllImport("user32.dll")] static extern bool EnumChildWindows(IntPtr p, EnumProc c, IntPtr l);
    [DllImport("user32.dll")] static extern IntPtr SendMessage(IntPtr h, uint m, IntPtr w, IntPtr l);

    // Is Explorer's taskbar up and populated: "task=<0/1> tray=<icons> vis=<0/1>".
    public static string Probe() {
        IntPtr tray = FindWindow("Shell_TrayWnd", null);
        if (tray == IntPtr.Zero) return "task=0 tray=0 vis=0";
        int task = 0, icons = 0;
        EnumChildWindows(tray, (h, l) => {
            var s = new System.Text.StringBuilder(128);
            GetClassName(h, s, 128);
            string c = s.ToString();
            if (c == "MSTaskListWClass" && IsWindowVisible(h)) task = 1;
            if (c == "ToolbarWindow32") { int n = (int)SendMessage(h, 0x0418, IntPtr.Zero, IntPtr.Zero); if (n > icons) icons = n; }
            return true;
        }, IntPtr.Zero);
        return "task=" + task + " tray=" + icons + " vis=" + (IsWindowVisible(tray) ? 1 : 0);
    }

    public static string Tray(string path) { return Tray(path, -1); }

    // hoverX >= 0: the pointer rests on the bar there (bar-local x) before the photograph.
    public static string Tray(string path, int hoverX) {
        SetProcessDPIAware();
        IntPtr h = FindWindow("Shell_TrayWnd", null);
        if (h == IntPtr.Zero) return "no tray";
        bool was = IsWindowVisible(h);
        var ours = OurBars();
        foreach (var o in ours) ShowWindow(o, 0);
        ShowWindow(h, 8); // SW_SHOWNA
        SetWindowPos(h, (IntPtr)(-1), 0, 0, 0, 0, 0x0001 | 0x0002 | 0x0010); // topmost, no move/size/activate
        RECT r; GetWindowRect(h, out r);
        if (hoverX >= 0) {
            SetCursorPos(r.l + hoverX + 40, r.t - 120);   // arrive from above, so it is a real move onto the bar
            Sleep(150);
            SetCursorPos(r.l + hoverX, (r.t + r.b) / 2);
            Sleep(900);
        } else {
            SetCursorPos(r.l + 960, r.t - 500);
            Sleep(700);
        }
        int w = r.r - r.l, ht = r.b - r.t;
        using (var bmp = new Bitmap(w, ht, PixelFormat.Format32bppArgb)) {
            using (var g = Graphics.FromImage(bmp)) g.CopyFromScreen(r.l, r.t, 0, 0, new Size(w, ht));
            bmp.Save(path, ImageFormat.Png);
        }
        SetCursorPos(r.l + 960, r.t - 500);
        if (!was) ShowWindow(h, 0);
        foreach (var o in ours) ShowWindow(o, 8);
        return "rect " + r.l + "," + r.t + "," + r.r + "," + r.b + " wasVisible=" + was + " ourBarsHidden=" + ours.Length;
    }

    // a over b: a stacked picture (a, b, difference) at 2x, and the numbers.
    public static string Diff(string a, string b, string outStack, string outDiff) {
        using (var A = new Bitmap(a)) using (var B = new Bitmap(b)) {
            int w = Math.Min(A.Width, B.Width), h = Math.Min(A.Height, B.Height);
            using (var D = new Bitmap(w, h, PixelFormat.Format32bppArgb)) {
                long sum = 0; int bad = 0;
                for (int y = 0; y < h; y++) for (int x = 0; x < w; x++) {
                    Color p = A.GetPixel(x, y), q = B.GetPixel(x, y);
                    int d = Math.Abs(p.R - q.R) + Math.Abs(p.G - q.G) + Math.Abs(p.B - q.B);
                    sum += d; if (d > 48) bad++;
                    int v = Math.Min(255, d * 3);
                    D.SetPixel(x, y, Color.FromArgb(255, v, v, v));
                }
                D.Save(outDiff, ImageFormat.Png);
                int s = 2;
                using (var S = new Bitmap(w * s, h * s * 3 + 8, PixelFormat.Format32bppArgb))
                using (var g = Graphics.FromImage(S)) {
                    g.Clear(Color.Magenta);
                    g.InterpolationMode = System.Drawing.Drawing2D.InterpolationMode.NearestNeighbor;
                    g.PixelOffsetMode = System.Drawing.Drawing2D.PixelOffsetMode.Half;
                    g.DrawImage(A, new Rectangle(0, 0, w * s, h * s), 0, 0, w, h, GraphicsUnit.Pixel);
                    g.DrawImage(B, new Rectangle(0, h * s + 4, w * s, h * s), 0, 0, w, h, GraphicsUnit.Pixel);
                    g.DrawImage(D, new Rectangle(0, h * s * 2 + 8, w * s, h * s), 0, 0, w, h, GraphicsUnit.Pixel);
                    S.Save(outStack, ImageFormat.Png);
                }
                return string.Format("size {0}x{1} (sab {2}x{3}, ours {4}x{5}); mean diff {6:F2} of 765; {7:P1} of pixels differ", w, h, A.Width, A.Height, B.Width, B.Height, (double)sum / (w * h), (double)bad / (w * h));
            }
        }
    }
}
