using System;
using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;

// Reads a PE file's icon resources straight from the file (no Windows
// resource APIs, so no hooks), and reports which groups have a 256px PNG
// frame whose alpha channel is fully opaque.
public static class Scan {
    public class Pe {
        public byte[] B;
        public int Root;
        public uint SecRva, SecRaw;
        public int Off(uint rva) { return (int)(rva - SecRva + SecRaw); }
    }

    public static Pe Open(string path) {
        var pe = new Pe { B = File.ReadAllBytes(path) };
        byte[] b = pe.B;
        int p = BitConverter.ToInt32(b, 0x3C);
        int sections = BitConverter.ToUInt16(b, p + 6);
        int optSize = BitConverter.ToUInt16(b, p + 20);
        int opt = p + 24;
        bool pe64 = BitConverter.ToUInt16(b, opt) == 0x20B;
        uint rsrcRva = BitConverter.ToUInt32(b, opt + (pe64 ? 112 : 96) + 16);
        for (int i = 0; i < sections; i++) {
            int s = opt + optSize + 40 * i;
            uint va = BitConverter.ToUInt32(b, s + 12), vs = Math.Max(BitConverter.ToUInt32(b, s + 8), BitConverter.ToUInt32(b, s + 16));
            if (rsrcRva >= va && rsrcRva < va + vs) { pe.SecRva = va; pe.SecRaw = BitConverter.ToUInt32(b, s + 20); }
        }
        pe.Root = pe.Off(rsrcRva);
        return pe;
    }

    static List<KeyValuePair<int, int>> Entries(Pe pe, int dir) {
        var list = new List<KeyValuePair<int, int>>();
        byte[] b = pe.B;
        int named = BitConverter.ToUInt16(b, dir + 12), ids = BitConverter.ToUInt16(b, dir + 14);
        for (int i = named; i < named + ids; i++) {
            int e = dir + 16 + 8 * i;
            list.Add(new KeyValuePair<int, int>((int)BitConverter.ToUInt32(b, e), (int)BitConverter.ToUInt32(b, e + 4)));
        }
        return list;
    }

    static int Sub(Pe pe, int dir, int id) {
        foreach (var kv in Entries(pe, dir)) if (kv.Key == id && kv.Value < 0) return pe.Root + (kv.Value & 0x7FFFFFFF);
        return -1;
    }

    public static byte[] Data(Pe pe, int type, int id) {
        int t = Sub(pe, pe.Root, type);
        if (t < 0) return null;
        int n = Sub(pe, t, id);
        if (n < 0) return null;
        var langs = Entries(pe, n);
        if (langs.Count == 0) return null;
        int de = pe.Root + langs[0].Value;
        uint rva = BitConverter.ToUInt32(pe.B, de), size = BitConverter.ToUInt32(pe.B, de + 4);
        byte[] outBytes = new byte[size];
        Array.Copy(pe.B, pe.Off(rva), outBytes, 0, size);
        return outBytes;
    }

    public static int Lang(Pe pe, int type, int id) {
        int t = Sub(pe, pe.Root, type);
        if (t < 0) return 0;
        int n = Sub(pe, t, id);
        if (n < 0) return 0;
        var langs = Entries(pe, n);
        return langs.Count == 0 ? 0 : langs[0].Key;
    }

    public static List<int> Groups(Pe pe) {
        var list = new List<int>();
        int t = Sub(pe, pe.Root, 14);
        if (t < 0) return list;
        foreach (var kv in Entries(pe, t)) list.Add(kv.Key);
        return list;
    }

    // Frames of a group: (width, RT_ICON id).
    public static List<int[]> Frames(Pe pe, int group) {
        var list = new List<int[]>();
        byte[] g = Data(pe, 14, group);
        if (g == null) return list;
        int n = BitConverter.ToUInt16(g, 4);
        for (int i = 0; i < n; i++) {
            int o = 6 + 14 * i;
            list.Add(new[] { g[o] == 0 ? 256 : g[o], BitConverter.ToUInt16(g, o + 12), BitConverter.ToUInt16(g, o + 6) });
        }
        return list;
    }

    public static bool PngOpaque(byte[] png) {
        using (var ms = new MemoryStream(png))
        using (var bmp = new Bitmap(ms)) {
            var r = new Rectangle(0, 0, bmp.Width, bmp.Height);
            var d = bmp.LockBits(r, ImageLockMode.ReadOnly, PixelFormat.Format32bppArgb);
            byte[] px = new byte[d.Stride * d.Height];
            Marshal.Copy(d.Scan0, px, 0, px.Length);
            bmp.UnlockBits(d);
            for (int i = 3; i < px.Length; i += 4) if (px[i] != 255) return false;
            return true;
        }
    }

    public static string Report(string path) {
        var pe = Open(path);
        var groups = Groups(pe);
        int withBig = 0, broken = 0;
        var sb = new StringBuilder();
        foreach (int g in groups) {
            foreach (var f in Frames(pe, g)) {
                if (f[0] != 256) continue;
                byte[] data = Data(pe, 3, f[1]);
                if (data == null || data[0] != 0x89) continue;
                withBig++;
                if (PngOpaque(data)) { broken++; sb.Append(g).Append(' '); }
            }
        }
        return string.Format("{0}: {1} groups, {2} with a 256px PNG, {3} of them opaque: {4}", Path.GetFileName(path),
                             groups.Count, withBig, broken, sb);
    }
}
