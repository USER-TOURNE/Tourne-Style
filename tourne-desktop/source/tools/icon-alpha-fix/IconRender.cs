using System;
using System.Runtime.InteropServices;

public static class IconRender {
    [DllImport("user32", CharSet = CharSet.Unicode)]
    static extern uint PrivateExtractIconsW(string file, int index, int cx, int cy, IntPtr[] icons, uint[] ids, uint count, uint flags);
    [DllImport("user32")] static extern bool DestroyIcon(IntPtr h);
    [DllImport("user32")] static extern bool DrawIconEx(IntPtr dc, int x, int y, IntPtr icon, int w, int h, uint step, IntPtr brush, uint flags);
    [DllImport("user32")] static extern bool GetIconInfo(IntPtr icon, out ICONINFO info);
    [StructLayout(LayoutKind.Sequential)] struct ICONINFO { public bool fIcon; public int x, y; public IntPtr mask, color; }
    [StructLayout(LayoutKind.Sequential)] struct BITMAP { public int type, width, height, widthBytes; public short planes, bpp; public IntPtr bits; }
    [DllImport("gdi32")] static extern int GetObject(IntPtr h, int size, ref BITMAP bm);
    [DllImport("gdi32")] static extern IntPtr CreateCompatibleDC(IntPtr dc);
    [DllImport("gdi32")] static extern bool DeleteDC(IntPtr dc);
    [DllImport("gdi32")] static extern IntPtr SelectObject(IntPtr dc, IntPtr obj);
    [DllImport("gdi32")] static extern bool DeleteObject(IntPtr h);
    [DllImport("gdi32")] static extern IntPtr CreateDIBSection(IntPtr dc, byte[] info, uint usage, out IntPtr bits, IntPtr section, uint offset);

    // Extracts one icon at a size, reports its colour bitmap's depth, then
    // draws it onto a clear 32-bit surface and counts what the alpha became.
    public static string Test(string file, int index, int size) {
        IntPtr[] icons = new IntPtr[1];
        uint[] ids = new uint[1];
        uint got = PrivateExtractIconsW(file, index, size, size, icons, ids, 1, 0);
        if (got == 0 || got == 0xFFFFFFFF || icons[0] == IntPtr.Zero) return "no icon (" + got + ")";
        ICONINFO ii;
        GetIconInfo(icons[0], out ii);
        BITMAP bm = new BITMAP();
        string depth = "no colour bitmap";
        if (ii.color != IntPtr.Zero) { GetObject(ii.color, Marshal.SizeOf(bm), ref bm); depth = bm.width + "px " + bm.bpp + "bpp"; }
        if (ii.color != IntPtr.Zero) DeleteObject(ii.color);
        if (ii.mask != IntPtr.Zero) DeleteObject(ii.mask);

        byte[] info = new byte[40];
        BitConverter.GetBytes(40).CopyTo(info, 0);
        BitConverter.GetBytes(size).CopyTo(info, 4);
        BitConverter.GetBytes(-size).CopyTo(info, 8);
        BitConverter.GetBytes((short)1).CopyTo(info, 12);
        BitConverter.GetBytes((short)32).CopyTo(info, 14);
        IntPtr bits;
        IntPtr dib = CreateDIBSection(IntPtr.Zero, info, 0, out bits, IntPtr.Zero, 0);
        IntPtr dc = CreateCompatibleDC(IntPtr.Zero);
        IntPtr old = SelectObject(dc, dib);
        DrawIconEx(dc, 0, 0, icons[0], size, size, 0, IntPtr.Zero, 3);
        byte[] px = new byte[size * size * 4];
        Marshal.Copy(bits, px, 0, px.Length);
        SelectObject(dc, old);
        DeleteDC(dc);
        DeleteObject(dib);
        DestroyIcon(icons[0]);
        int clear = 0, solid = 0;
        for (int i = 0; i < size * size; i++) {
            if (px[i * 4 + 3] == 0) clear++; else if (px[i * 4 + 3] == 255) solid++;
        }
        return string.Format("{0}; drawn alpha {1}% clear {2}% solid{3}", depth, clear * 100 / (size * size),
                             solid * 100 / (size * size), clear == 0 ? "  <-- LOST" : "");
    }
}
