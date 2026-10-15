using System;
using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;

// Repairs a theme DLL whose 256px PNG frames lost their alpha: each such
// frame is rebuilt from the same group's 256px frame in the theme's .res
// (a 32-bit DIB with its alpha intact), saved as PNG. Groups whose 48px
// frame differs between the two files are skipped, so only the same art
// is ever swapped in.
public static class Fix {
    [DllImport("kernel32", CharSet = CharSet.Unicode, SetLastError = true)]
    static extern IntPtr BeginUpdateResourceW(string file, bool deleteExisting);
    [DllImport("kernel32", SetLastError = true)]
    static extern bool UpdateResourceW(IntPtr h, IntPtr type, IntPtr name, ushort lang, byte[] data, uint size);
    [DllImport("kernel32", SetLastError = true)]
    static extern bool EndUpdateResourceW(IntPtr h, bool discard);

    static byte[] DibToPng(byte[] dib) {
        int hdr = BitConverter.ToInt32(dib, 0);
        int w = BitConverter.ToInt32(dib, 4);
        int h = BitConverter.ToInt32(dib, 8) / 2;  // colour half; the AND mask follows
        var bmp = new Bitmap(w, h, PixelFormat.Format32bppArgb);
        var d = bmp.LockBits(new Rectangle(0, 0, w, h), ImageLockMode.WriteOnly, PixelFormat.Format32bppArgb);
        byte[] row = new byte[w * 4];
        for (int y = 0; y < h; y++) {
            Array.Copy(dib, hdr + (h - 1 - y) * w * 4, row, 0, row.Length);  // bottom-up
            Marshal.Copy(row, 0, d.Scan0 + y * d.Stride, row.Length);
        }
        bmp.UnlockBits(d);
        using (var ms = new MemoryStream()) {
            bmp.Save(ms, ImageFormat.Png);
            bmp.Dispose();
            return ms.ToArray();
        }
    }

    static bool Same(byte[] a, byte[] b) {
        if (a == null || b == null || a.Length != b.Length) return false;
        for (int i = 0; i < a.Length; i++) if (a[i] != b[i]) return false;
        return true;
    }

    public static string Run(string dllIn, string resIn, string dllOut) {
        File.Copy(dllIn, dllOut, true);
        var pe = Scan.Open(dllIn);
        var res = ResFile.Load(resIn);
        IntPtr h = BeginUpdateResourceW(dllOut, false);
        if (h == IntPtr.Zero) return "BeginUpdateResource failed " + Marshal.GetLastWin32Error();
        int fixedCount = 0;
        var skipped = new StringBuilder();
        foreach (int g in Scan.Groups(pe)) {
            byte[] grp = Scan.Data(pe, 14, g);
            byte[] resGrp = ResFile.Get(res, 14, g);
            if (grp == null) continue;
            int n = BitConverter.ToUInt16(grp, 4);
            bool changed = false;
            for (int i = 0; i < n; i++) {
                int o = 6 + 14 * i;
                if (grp[o] != 0) continue;                     // 256px only
                int frameId = BitConverter.ToUInt16(grp, o + 12);
                byte[] frame = Scan.Data(pe, 3, frameId);
                if (frame == null || frame[0] != 0x89 || !Scan.PngOpaque(frame)) continue;
                if (resGrp == null) { skipped.Append(g + "(no .res group) "); continue; }
                byte[] good256 = null, res48 = null, dll48 = null;
                int rn = BitConverter.ToUInt16(resGrp, 4);
                for (int j = 0; j < rn; j++) {
                    int ro = 6 + 14 * j;
                    int rid = BitConverter.ToUInt16(resGrp, ro + 12);
                    if (resGrp[ro] == 0) good256 = ResFile.Get(res, 3, rid);
                    if (resGrp[ro] == 48) res48 = ResFile.Get(res, 3, rid);
                }
                for (int j = 0; j < n; j++) {
                    int jo = 6 + 14 * j;
                    if (grp[jo] == 48) dll48 = Scan.Data(pe, 3, BitConverter.ToUInt16(grp, jo + 12));
                }
                if (good256 == null || good256[0] == 0x89 || BitConverter.ToUInt16(good256, 14) != 32) {
                    skipped.Append(g + "(no 32-bit 256 in .res) "); continue;
                }
                if (!Same(res48, dll48)) { skipped.Append(g + "(different art) "); continue; }
                byte[] png = DibToPng(good256);
                ushort lang = (ushort)Scan.Lang(pe, 3, frameId);
                if (!UpdateResourceW(h, (IntPtr)3, (IntPtr)frameId, lang, png, (uint)png.Length)) {
                    EndUpdateResourceW(h, true);
                    return "UpdateResource failed " + Marshal.GetLastWin32Error();
                }
                BitConverter.GetBytes((uint)png.Length).CopyTo(grp, o + 8);   // dwBytesInRes
                changed = true;
                fixedCount++;
            }
            if (changed) {
                ushort glang = (ushort)Scan.Lang(pe, 14, g);
                UpdateResourceW(h, (IntPtr)14, (IntPtr)g, glang, grp, (uint)grp.Length);
            }
        }
        if (!EndUpdateResourceW(h, false)) return "EndUpdateResource failed " + Marshal.GetLastWin32Error();
        return string.Format("{0}: fixed {1} frames. Skipped: {2}", Path.GetFileName(dllOut), fixedCount,
                             skipped.Length > 0 ? skipped.ToString() : "none");
    }
}
