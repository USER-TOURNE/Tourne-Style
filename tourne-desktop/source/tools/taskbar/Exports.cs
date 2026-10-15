using System;
using System.IO;
using System.Text;

// Named exports of a PE file, with ordinals, read from the file itself.
public static class Exports {
    public static string List(string path, string filter) {
        byte[] b = File.ReadAllBytes(path);
        int pe = BitConverter.ToInt32(b, 0x3C);
        int sections = BitConverter.ToUInt16(b, pe + 6);
        int optSize = BitConverter.ToUInt16(b, pe + 20);
        int opt = pe + 24;
        bool pe64 = BitConverter.ToUInt16(b, opt) == 0x20B;
        uint expRva = BitConverter.ToUInt32(b, opt + (pe64 ? 112 : 96));
        Func<uint, int> off = rva => {
            for (int i = 0; i < sections; i++) {
                int s = opt + optSize + 40 * i;
                uint va = BitConverter.ToUInt32(b, s + 12), vs = Math.Max(BitConverter.ToUInt32(b, s + 8), BitConverter.ToUInt32(b, s + 16));
                if (rva >= va && rva < va + vs) return (int)(rva - va + BitConverter.ToUInt32(b, s + 20));
            }
            return -1;
        };
        int e = off(expRva);
        uint baseOrd = BitConverter.ToUInt32(b, e + 16);
        int funcs = BitConverter.ToInt32(b, e + 20), names = BitConverter.ToInt32(b, e + 24);
        int nameTable = off(BitConverter.ToUInt32(b, e + 32)), ordTable = off(BitConverter.ToUInt32(b, e + 36));
        var sb = new StringBuilder();
        var named = new bool[funcs];
        for (int i = 0; i < names; i++) {
            int n = off(BitConverter.ToUInt32(b, nameTable + 4 * i));
            int end = n;
            while (b[end] != 0) end++;
            string name = Encoding.ASCII.GetString(b, n, end - n);
            int ord = BitConverter.ToUInt16(b, ordTable + 2 * i);
            named[ord] = true;
            if (filter == null || name.IndexOf(filter, StringComparison.OrdinalIgnoreCase) >= 0)
                sb.AppendFormat("{0,4} {1}\n", ord + baseOrd, name);
        }
        var unnamed = new StringBuilder();
        for (int i = 0; i < funcs; i++) if (!named[i]) unnamed.Append(i + baseOrd).Append(' ');
        sb.Append("ordinal-only: ").Append(unnamed).Append('\n');
        return sb.ToString();
    }
}
