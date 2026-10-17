using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;

// Lists a module's resources: every type with its names and sizes, plus the
// class map (CMAP), which in a visual style names each class it styles.
public static class ResList {
    [DllImport("kernel32", CharSet = CharSet.Unicode, SetLastError = true)]
    static extern IntPtr LoadLibraryExW(string path, IntPtr file, uint flags);
    [DllImport("kernel32")] static extern bool FreeLibrary(IntPtr m);
    delegate bool EnumTypeProc(IntPtr m, IntPtr type, IntPtr param);
    delegate bool EnumNameProc(IntPtr m, IntPtr type, IntPtr name, IntPtr param);
    [DllImport("kernel32", CharSet = CharSet.Unicode)] static extern bool EnumResourceTypesW(IntPtr m, EnumTypeProc proc, IntPtr param);
    [DllImport("kernel32", CharSet = CharSet.Unicode)] static extern bool EnumResourceNamesW(IntPtr m, IntPtr type, EnumNameProc proc, IntPtr param);
    [DllImport("kernel32", CharSet = CharSet.Unicode)] static extern IntPtr FindResourceW(IntPtr m, IntPtr name, IntPtr type);
    [DllImport("kernel32", CharSet = CharSet.Unicode)] static extern IntPtr FindResourceW(IntPtr m, string name, string type);
    [DllImport("kernel32")] static extern IntPtr LoadResource(IntPtr m, IntPtr r);
    [DllImport("kernel32")] static extern IntPtr LockResource(IntPtr h);
    [DllImport("kernel32")] static extern uint SizeofResource(IntPtr m, IntPtr r);

    static string Id(IntPtr p) {
        long v = p.ToInt64();
        return v < 0x10000 ? "#" + v : Marshal.PtrToStringUni(p);
    }

    public static string List(string path, int maxNames) {
        IntPtr m = LoadLibraryExW(path, IntPtr.Zero, 0x2 | 0x20);
        if (m == IntPtr.Zero) return "cannot load " + path;
        var sb = new StringBuilder();
        var types = new List<string>();
        EnumResourceTypesW(m, (mod, t, p) => { types.Add(Id(t)); return true; }, IntPtr.Zero);
        foreach (string type in types) {
            var names = new List<string>();
            IntPtr typePtr = type.StartsWith("#") ? (IntPtr)int.Parse(type.Substring(1)) : Marshal.StringToHGlobalUni(type);
            long total = 0;
            EnumResourceNamesW(m, typePtr, (mod, t, n, p) => {
                string name = Id(n);
                IntPtr r = FindResourceW(mod, n, t);
                uint size = SizeofResource(mod, r);
                total += size;
                names.Add(name + "(" + size + ")");
                return true;
            }, IntPtr.Zero);
            sb.AppendFormat("{0}: {1} items, {2} bytes: {3}{4}\n", type, names.Count, total,
                            string.Join(" ", names.GetRange(0, Math.Min(maxNames, names.Count))),
                            names.Count > maxNames ? " ..." : "");
        }
        // The class map: zero-separated UTF-16 class names.
        IntPtr cmap = FindResourceW(m, "CMAP", "CMAP");
        if (cmap != IntPtr.Zero) {
            uint size = SizeofResource(m, cmap);
            IntPtr data = LockResource(LoadResource(m, cmap));
            string all = Marshal.PtrToStringUni(data, (int)size / 2);
            sb.Append("CMAP classes: ").Append(string.Join(", ", all.Split(new[] { '\0' }, StringSplitOptions.RemoveEmptyEntries))).Append('\n');
        }
        FreeLibrary(m);
        return sb.ToString();
    }
}
