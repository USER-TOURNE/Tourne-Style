using System;
using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;

// A .res file: a run of RESOURCEHEADER records, each followed by its data.
// Only numeric types and names are kept (icons use numbers).
public static class ResFile {
    public static Dictionary<long, byte[]> Load(string path) {
        var map = new Dictionary<long, byte[]>();
        using (var fs = File.OpenRead(path))
        using (var r = new BinaryReader(fs)) {
            while (fs.Position + 32 <= fs.Length) {
                long start = fs.Position;
                uint dataSize = r.ReadUInt32(), headerSize = r.ReadUInt32();
                int type = ReadId(r), name = ReadId(r);
                fs.Position = start + headerSize;
                byte[] data = r.ReadBytes((int)dataSize);
                if (type > 0 && name > 0) map[((long)type << 32) | (uint)name] = data;
                fs.Position = (fs.Position + 3) & ~3L;
            }
        }
        return map;
    }

    // 0xFFFF then an id, or a zero-terminated string (skipped, returns -1).
    static int ReadId(BinaryReader r) {
        ushort first = r.ReadUInt16();
        if (first == 0xFFFF) return r.ReadUInt16();
        ushort c = first;
        while (c != 0) c = r.ReadUInt16();
        return -1;
    }

    public static byte[] Get(Dictionary<long, byte[]> map, int type, int id) {
        byte[] d;
        return map.TryGetValue(((long)type << 32) | (uint)id, out d) ? d : null;
    }

    // Alpha of a frame, PNG or 32-bit DIB: "ok", "opaque", or a format note.
    public static string Alpha(byte[] f, int width) {
        if (f[0] == 0x89 && f[1] == 0x50) {
            using (var ms = new MemoryStream(f))
            using (var bmp = new Bitmap(ms)) {
                var d = bmp.LockBits(new Rectangle(0, 0, bmp.Width, bmp.Height), ImageLockMode.ReadOnly, PixelFormat.Format32bppArgb);
                byte[] px = new byte[d.Stride * d.Height];
                Marshal.Copy(d.Scan0, px, 0, px.Length);
                bmp.UnlockBits(d);
                for (int i = 3; i < px.Length; i += 4) if (px[i] != 255) return "PNG ok";
                return "PNG OPAQUE";
            }
        }
        int hdr = BitConverter.ToInt32(f, 0), bpp = BitConverter.ToUInt16(f, 14);
        if (bpp != 32) return "BMP " + bpp + "bpp";
        for (int p = 0; p < width * width; p++) {
            byte a = f[hdr + p * 4 + 3];
            if (a != 0 && a != 255) return "BMP ok";
        }
        return "BMP no soft alpha";
    }

    public static string Report(string path, int[] groups) {
        var map = Load(path);
        var sb = new StringBuilder();
        int icons = 0, groupCount = 0;
        foreach (var k in map.Keys) { if ((k >> 32) == 3) icons++; if ((k >> 32) == 14) groupCount++; }
        sb.AppendFormat("{0}: {1} groups, {2} icon frames\n", Path.GetFileName(path), groupCount, icons);
        foreach (int g in groups) {
            byte[] grp = Get(map, 14, g);
            if (grp == null) { sb.AppendFormat("  group {0}: missing\n", g); continue; }
            int n = BitConverter.ToUInt16(grp, 4);
            for (int i = 0; i < n; i++) {
                int o = 6 + 14 * i;
                int w = grp[o] == 0 ? 256 : grp[o];
                int id = BitConverter.ToUInt16(grp, o + 12);
                byte[] f = Get(map, 3, id);
                sb.AppendFormat("  group {0} {1}px id {2}: {3}\n", g, w, id, f == null ? "missing" : f.Length + " bytes, " + Alpha(f, w));
            }
        }
        return sb.ToString();
    }
}
