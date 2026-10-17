using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.Runtime.InteropServices;

// Input and capture helpers for the style comparison (no computer-use tools).
public static class Cap {
    [DllImport("user32.dll")] static extern bool SetProcessDPIAware();
    [DllImport("user32.dll")] static extern bool SetCursorPos(int x, int y);
    [DllImport("user32.dll")] static extern void keybd_event(byte vk, byte scan, uint flags, UIntPtr extra);
    [DllImport("user32.dll")] static extern void mouse_event(uint flags, uint dx, uint dy, uint data, UIntPtr extra);
    [DllImport("user32.dll", CharSet = CharSet.Unicode)] static extern IntPtr SendMessageTimeout(IntPtr h, uint msg, IntPtr w, string l, uint flags, uint timeout, out IntPtr result);
    [DllImport("kernel32.dll")] static extern void Sleep(uint ms);

    public static void Init() { SetProcessDPIAware(); }
    public static void Wait(int ms) { Sleep((uint)ms); }
    public static void Move(int x, int y) { SetCursorPos(x, y); }
    // A pointer move that arrives from above, so windows see a real hover.
    public static void HoverAt(int x, int y) { SetCursorPos(x + 30, y - 100); Sleep(120); SetCursorPos(x, y); }
    public static void Key(byte vk) { keybd_event(vk, 0, 0, UIntPtr.Zero); Sleep(40); keybd_event(vk, 0, 2, UIntPtr.Zero); }
    public static void Chord(byte mod, byte vk) {
        keybd_event(mod, 0, 0, UIntPtr.Zero); Sleep(40);
        keybd_event(vk, 0, 0, UIntPtr.Zero); Sleep(40);
        keybd_event(vk, 0, 2, UIntPtr.Zero); Sleep(40);
        keybd_event(mod, 0, 2, UIntPtr.Zero);
    }
    public static void Click(int x, int y) {
        SetCursorPos(x, y); Sleep(80);
        mouse_event(2, 0, 0, 0, UIntPtr.Zero); Sleep(50); mouse_event(4, 0, 0, 0, UIntPtr.Zero);
    }
    // Tells running programs that colours or the theme changed (the suite's bars reload their style on it).
    public static void ThemeChanged() {
        IntPtr r;
        SendMessageTimeout((IntPtr)0xFFFF, 0x001A, IntPtr.Zero, "ImmersiveColorSet", 2, 2000, out r);
    }
    public static void Full(string path, int w, int h) {
        using (var bmp = new Bitmap(w, h, PixelFormat.Format32bppArgb)) {
            using (var g = Graphics.FromImage(bmp)) g.CopyFromScreen(0, 0, 0, 0, new Size(w, h));
            bmp.Save(path, ImageFormat.Png);
        }
    }
}
