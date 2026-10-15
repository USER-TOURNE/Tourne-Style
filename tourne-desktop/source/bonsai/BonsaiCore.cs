// Bonsai: the tree, shared by the screensaver (BonsaiSaver.scr) and the terminal version (bonsai.exe).
//
// "Classic" is cbonsai's own growth (John Allbritten's, gitlab.com/jallbrit/cbonsai), ported step for
// step: the same dice, the same branch strings, the same pot. "Detailed" grows the same way with more side
// branches, a trunk that tapers from a root flare, and shaded leaf clouds (pads along each branch's outer
// half, a full cloud at its end) sized to the tree, as on a trained bonsai.
//
// A tree is grown once, off screen, into a list of steps (each a few cells written); playing it back step
// by step is the growth animation. Coordinates: the root is at (0, 0), y goes down, so the tree is at y < 0.
//
// Written for C# 5, which the compiler built into Windows (.NET Framework's csc) understands.

using System;
using System.Collections.Generic;
using System.Drawing;
using Microsoft.Win32;

namespace Bonsai
{
    // What a cell is part of; each has its own colour.
    public enum Role { Trunk, TrunkLit, Leaf, LeafLit, LeafShade, Pot, Background }

    public struct Cell
    {
        public int X, Y;
        public char Ch;
        public Role Role;
        public Cell(int x, int y, char ch, Role role) { X = x; Y = y; Ch = ch; Role = role; }
    }

    public enum Style { Detailed, Classic }

    // The settings, kept in HKCU\Software\Tourne\Bonsai (both programs read them).
    public class Settings
    {
        public const string Key = @"Software\Tourne\Bonsai";
        public static readonly string[] RoleNames = { "Trunk", "Trunk highlight", "Leaves", "Leaf highlight", "Leaf shadow", "Pot", "Background" };
        public static readonly string[] RoleKeys = { "ColorTrunk", "ColorTrunkLit", "ColorLeaf", "ColorLeafLit", "ColorLeafShade", "ColorPot", "ColorBackground" };

        // The palette: Pale Red, Red, Pale Green, Green, Alt Pale Green, Alt Pale Green, Dark Teal.
        public static readonly Color[] DefaultColors = {
            Color.FromArgb(0xa9, 0x5d, 0x5d), Color.FromArgb(0xff, 0x8f, 0x8f),
            Color.FromArgb(0x64, 0xaa, 0x89), Color.FromArgb(0x15, 0xff, 0xa2), Color.FromArgb(0x6b, 0xa6, 0xa0),
            Color.FromArgb(0x6b, 0xa6, 0xa0), Color.FromArgb(0x12, 0x1c, 0x21) };
        // cbonsai's own (terminal yellow, bright yellow, green, bright green, green, grey, black):
        // { #808000, #ffff00, #008000, #00ff00, #008000, #808080, #000000 }

        public Style Style = Style.Detailed;
        public int Life = 56;            // how big it grows (cbonsai's -L; its default is 32)
        public int Multiplier = 5;       // how bushy (cbonsai's -M)
        public int BranchDensity = 8;    // 1 to 10: how often side branches sprout (Detailed)
        public int CanopySize = 5;       // 0 to 10: the leaf clouds' size (Detailed)
        public int GrowSeconds = 30;     // how long a tree takes to grow
        public int HoldSeconds = 8;      // how long a grown tree stays before the next
        public bool FillScreen = true;   // zoomed to fill the screen, else a fixed zoom
        public int FillPercent = 90;     // how much of the screen it fills
        public int Zoom = 3;             // the fixed zoom: whole pixels per font pixel
        public int Pot = 1;              // 0 none, 1 large, 2 small
        public string Leaves = "&";      // Classic's leaf characters, comma separated (cbonsai's -c)
        public string Message = "";      // shown in a box beside the tree (cbonsai's -m)
        public bool AllMonitors = true;  // a tree on every screen, else the main one only
        public string Logo = "logo.png"; // a picture in the top right corner (beside the .scr unless a full path); "" none
        public int LogoSize = 22;        // its width, in % of the screen's (0: not shown)
        public bool LogoKeyWhite = true; // its white background made see-through (a logo on white)
        public Color[] Colors = (Color[])DefaultColors.Clone();

        public Color this[Role role] { get { return Colors[(int)role]; } }

        public Settings Clone()
        {
            var c = (Settings)MemberwiseClone();
            c.Colors = (Color[])Colors.Clone();
            return c;
        }

        public static Settings Load()
        {
            var s = new Settings();
            try
            {
                using (var k = Registry.CurrentUser.OpenSubKey(Key))
                {
                    if (k == null) return s;
                    s.Style = Int(k, "Style", 0) == 1 ? Style.Classic : Style.Detailed;
                    s.Life = Clamp(Int(k, "Life", s.Life), 8, 200);
                    s.Multiplier = Clamp(Int(k, "Multiplier", s.Multiplier), 1, 20);
                    s.BranchDensity = Clamp(Int(k, "BranchDensity", s.BranchDensity), 1, 10);
                    s.CanopySize = Clamp(Int(k, "CanopySize", s.CanopySize), 0, 10);
                    s.GrowSeconds = Clamp(Int(k, "GrowSeconds", s.GrowSeconds), 1, 600);
                    s.HoldSeconds = Clamp(Int(k, "HoldSeconds", s.HoldSeconds), 0, 600);
                    s.FillScreen = Int(k, "FillScreen", 1) != 0;
                    s.FillPercent = Clamp(Int(k, "FillPercent", s.FillPercent), 30, 100);
                    s.Zoom = Clamp(Int(k, "Zoom", s.Zoom), 1, 12);
                    s.Pot = Clamp(Int(k, "Pot", s.Pot), 0, 2);
                    s.Leaves = Str(k, "Leaves", s.Leaves);
                    s.Message = Str(k, "Message", s.Message);
                    s.AllMonitors = Int(k, "AllMonitors", 1) != 0;
                    s.Logo = Str(k, "Logo", s.Logo);
                    s.LogoSize = Clamp(Int(k, "LogoSize", s.LogoSize), 0, 100);
                    s.LogoKeyWhite = Int(k, "LogoKeyWhite", 1) != 0;
                    for (int i = 0; i < RoleKeys.Length; i++)
                    {
                        Color c;
                        if (TryParseHex(Str(k, RoleKeys[i], ""), out c)) s.Colors[i] = c;
                    }
                }
            }
            catch (Exception) { }
            return s;
        }

        public void Save()
        {
            using (var k = Registry.CurrentUser.CreateSubKey(Key))
            {
                k.SetValue("Style", Style == Style.Classic ? 1 : 0);
                k.SetValue("Life", Life);
                k.SetValue("Multiplier", Multiplier);
                k.SetValue("BranchDensity", BranchDensity);
                k.SetValue("CanopySize", CanopySize);
                k.SetValue("GrowSeconds", GrowSeconds);
                k.SetValue("HoldSeconds", HoldSeconds);
                k.SetValue("FillScreen", FillScreen ? 1 : 0);
                k.SetValue("FillPercent", FillPercent);
                k.SetValue("Zoom", Zoom);
                k.SetValue("Pot", Pot);
                k.SetValue("Leaves", Leaves ?? "");
                k.SetValue("Message", Message ?? "");
                k.SetValue("AllMonitors", AllMonitors ? 1 : 0);
                k.SetValue("Logo", Logo ?? "");
                k.SetValue("LogoSize", LogoSize);
                k.SetValue("LogoKeyWhite", LogoKeyWhite ? 1 : 0);
                for (int i = 0; i < RoleKeys.Length; i++) k.SetValue(RoleKeys[i], ToHex(Colors[i]));
            }
        }

        public string[] LeafStrings()
        {
            var list = new List<string>();
            foreach (var part in (Leaves ?? "").Split(','))
                if (part.Trim().Length > 0) list.Add(part.Trim());
            if (list.Count == 0) list.Add("&");
            return list.ToArray();
        }

        public static string ToHex(Color c) { return string.Format("#{0:x2}{1:x2}{2:x2}", c.R, c.G, c.B); }

        public static bool TryParseHex(string text, out Color color)
        {
            color = Color.Black;
            text = (text ?? "").Trim().TrimStart('#');
            int v;
            if (text.Length != 6 || !int.TryParse(text, System.Globalization.NumberStyles.HexNumber, null, out v)) return false;
            color = Color.FromArgb((v >> 16) & 255, (v >> 8) & 255, v & 255);
            return true;
        }

        static int Int(RegistryKey k, string name, int fallback)
        {
            object v = k.GetValue(name);
            return v is int ? (int)v : fallback;
        }

        static string Str(RegistryKey k, string name, string fallback)
        {
            object v = k.GetValue(name);
            return v is string ? (string)v : fallback;
        }

        public static int Clamp(int v, int lo, int hi) { return v < lo ? lo : v > hi ? hi : v; }
    }

    // A grown tree: its steps in order, and the box everything drawn fits in (pot and message included).
    public class Tree
    {
        public readonly List<Cell[]> Steps = new List<Cell[]>();
        public int MinX, MaxX, MinY, MaxY;
        public int Width { get { return MaxX - MinX + 1; } }
        public int Height { get { return MaxY - MinY + 1; } }

        public void Add(List<Cell> cells)
        {
            if (cells.Count == 0) return;
            Steps.Add(cells.ToArray());
            cells.Clear();
        }

        public void Measure()
        {
            MinX = MinY = int.MaxValue;
            MaxX = MaxY = int.MinValue;
            foreach (var step in Steps)
                foreach (var c in step)
                {
                    if (c.X < MinX) MinX = c.X;
                    if (c.X > MaxX) MaxX = c.X;
                    if (c.Y < MinY) MinY = c.Y;
                    if (c.Y > MaxY) MaxY = c.Y;
                }
            if (MinX > MaxX) MinX = MaxX = MinY = MaxY = 0;
        }
    }

    public class Grower
    {
        enum Kind { Trunk, ShootLeft, ShootRight, Dying, Dead }

        readonly Settings s;
        readonly Random rnd;
        readonly string[] leaves;
        readonly Tree tree = new Tree();
        readonly List<Cell> pending = new List<Cell>();
        int shootCounter;
        int calls;

        public Grower(Settings settings, int seed)
        {
            s = settings;
            rnd = new Random(seed);
            leaves = s.LeafStrings();
            shootCounter = rnd.Next();
        }

        // The whole tree: pot first (as cbonsai draws it), then the growth, then the message.
        public static Tree Grow(Settings s, int seed)
        {
            var g = new Grower(s, seed);
            g.Pot();
            g.Branch(0, -1, Kind.Trunk, s.Life, s.Life);
            g.Message();
            g.tree.Measure();
            return g.tree;
        }

        // Several trees, kept the one whose shape best fills a box of this aspect (width / height in cells).
        public static Tree GrowToFit(Settings s, int seed, double cellsWide, double cellsHigh, int tries)
        {
            Tree best = null;
            double bestScore = -1;
            var pick = new Random(seed);
            for (int i = 0; i < Math.Max(1, tries); i++)
            {
                Tree t = Grow(s, pick.Next());
                double z = Math.Min(cellsWide / t.Width, cellsHigh / t.Height);
                double score = (t.Width * z) * (t.Height * z) / (cellsWide * cellsHigh);
                if (score > bestScore) { bestScore = score; best = t; }
            }
            return best;
        }

        int Roll(int n) { return rnd.Next(n); }

        void Put(int x, int y, string text, Role role)
        {
            for (int i = 0; i < text.Length; i++) pending.Add(new Cell(x + i, y, text[i], role));
        }

        // ---------------------------------------------------------------- the pot (cbonsai's drawBase)

        void Pot()
        {
            if (s.Pot == 1)
            {
                int x = -15, y = 0;
                Put(x, y, ":", Role.Pot);
                Put(x + 1, y, "___________", Role.Leaf);
                Put(x + 12, y, "./~~~\\.", Role.Trunk);
                Put(x + 19, y, "___________", Role.Leaf);
                Put(x + 30, y, ":", Role.Pot);
                Put(x, y + 1, " \\                           / ", Role.Pot);
                Put(x, y + 2, "  \\_________________________/ ", Role.Pot);
                Put(x, y + 3, "  (_)                     (_)", Role.Pot);
            }
            else if (s.Pot == 2)
            {
                int x = -7, y = 0;
                Put(x, y, "(", Role.Pot);
                Put(x + 1, y, "---", Role.Leaf);
                Put(x + 4, y, "./~~~\\.", Role.Trunk);
                Put(x + 11, y, "---", Role.Leaf);
                Put(x + 14, y, ")", Role.Pot);
                Put(x, y + 1, " (           ) ", Role.Pot);
                Put(x, y + 2, "  (_________)  ", Role.Pot);
            }
            pending.RemoveAll(c => c.Ch == ' ');
            tree.Add(pending);
        }

        // ---------------------------------------------------------------- growth (cbonsai's branch)

        void Branch(int x, int y, Kind kind, int life, int lifeStart)
        {
            if (++calls > 20000) return;  // (a safety net; real trees stay far below it)
            bool detailed = s.Style == Style.Detailed;
            int m = s.Multiplier;
            int dx = 0, dy = 0, age = 0;
            int shootCooldown = m;
            int start = life, padIn = PadGap();
            while (life > 0)
            {
                life--;
                age = lifeStart - life;
                Deltas(kind, life, age, out dx, out dy);
                if (dy > 0 && y > -2) dy--;  // too close to the ground
                if (detailed && kind == Kind.Trunk && age <= 2) dy = Math.Min(dy, 0);

                if (life < 3)
                {
                    if (!detailed) Branch(x, y, Kind.Dead, life, s.Life);
                    else if (Roll(3) == 0) Branch(x, y, Kind.Dead, life, s.Life);
                }
                else if ((kind == Kind.Trunk || kind == Kind.ShootLeft || kind == Kind.ShootRight) && life < m + 2)
                {
                    if (!detailed) Branch(x, y, Kind.Dying, life, s.Life);
                    // (Detailed: no more shoots; the branch ends in a cloud below)
                }
                else if (kind == Kind.Trunk && (Roll(3) == 0 || life % m == 0))
                {
                    bool fork = detailed ? (Roll(20) == 0 && life > 7 && age > m * 2) : (Roll(8) == 0 && life > 7);
                    if (fork)
                    {
                        shootCooldown = m * 2;
                        Branch(x, y, Kind.Trunk, life + Roll(5) - 2, s.Life);
                    }
                    else if (shootCooldown <= 0 && !(detailed && age < lifeStart / 4))  // (Detailed: a bare lower trunk)
                    {
                        // Detailed: more often the denser it's set (8 is about twice cbonsai's).
                        shootCooldown = detailed ? Math.Max(1, (int)Math.Round(m * 2 * (11 - s.BranchDensity) / 6.0)) : m * 2;
                        int shootLife = life + m + (detailed ? s.CanopySize / 3 : 0);
                        shootCounter++;
                        Branch(x, y, shootCounter % 2 == 0 ? Kind.ShootLeft : Kind.ShootRight, shootLife, s.Life);
                    }
                }
                shootCooldown--;

                x += dx;
                y += dy;
                Draw(kind, x, y, life, age, lifeStart, dx, dy);
                tree.Add(pending);

                // Detailed: foliage pads along a branch's outer half (shoots, and the trunk's top third),
                // layered like a trained bonsai's.
                if (detailed && (kind == Kind.ShootLeft || kind == Kind.ShootRight ? life < start / 2 : kind == Kind.Trunk && life < start / 4)
                    && life > 2 && --padIn <= 0)
                {
                    // (on the trunk, out to one side or the other: a broad crown, not a column)
                    int side = kind == Kind.Trunk ? (Roll(2) == 0 ? -1 : 1) * (int)(CloudWidth() * (0.4 + rnd.NextDouble() * 0.4)) : 0;
                    Cloud(x + side, y, 0.5 + rnd.NextDouble() * 0.2);
                    padIn = PadGap() + (kind == Kind.Trunk ? 2 : 0);
                }
            }
            // Detailed: every branch ends in a full cloud (the trunk's top in the biggest).
            if (detailed && (kind == Kind.Trunk || kind == Kind.ShootLeft || kind == Kind.ShootRight))
                Cloud(x, y, kind == Kind.Trunk ? 1.15 : 1.0);
        }

        // Steps between foliage pads: closer the bigger the canopy is set.
        int PadGap() { return Math.Max(2, 7 - s.CanopySize / 2) + Roll(3); }

        void Deltas(Kind kind, int life, int age, out int dx, out int dy)
        {
            int m = s.Multiplier;
            bool detailed = s.Style == Style.Detailed;
            int dice;
            dx = dy = 0;
            switch (kind)
            {
                case Kind.Trunk:
                    if (age <= 2 || life < 4) { dy = 0; dx = Roll(3) - 1; }
                    else if (age < m * 3)
                    {
                        int every = Math.Max(1, (int)(m * 0.5));
                        dy = age % every == 0 ? -1 : 0;
                        if (detailed) dy = Roll(10) < 7 ? -1 : 0;  // (rising more often, so its girth shows)
                        dice = Roll(10);
                        dx = dice == 0 ? -2 : dice <= 3 ? -1 : dice <= 5 ? 0 : dice <= 8 ? 1 : 2;
                        if (detailed) dx = Math.Max(-1, Math.Min(1, dx));  // (a thick trunk sways, it doesn't lurch)
                    }
                    else
                    {
                        dy = Roll(10) > 2 ? -1 : 0;
                        dx = Roll(3) - 1;
                    }
                    break;
                case Kind.ShootLeft:
                    dice = Roll(10);
                    dy = dice <= (detailed ? 2 : 1) ? -1 : dice <= 7 ? 0 : 1;  // (Detailed: reaching up a little more)
                    dice = Roll(10);
                    dx = dice <= 1 ? -2 : dice <= 5 ? -1 : dice <= 8 ? 0 : 1;
                    break;
                case Kind.ShootRight:
                    dice = Roll(10);
                    dy = dice <= (detailed ? 2 : 1) ? -1 : dice <= 7 ? 0 : 1;
                    dice = Roll(10);
                    dx = dice <= 1 ? 2 : dice <= 5 ? 1 : dice <= 8 ? 0 : -1;
                    break;
                case Kind.Dying:
                    dice = Roll(10);
                    dy = dice <= 1 ? -1 : dice <= 8 ? 0 : 1;
                    dice = Roll(15);
                    dx = dice == 0 ? -3 : dice <= 2 ? -2 : dice <= 5 ? -1 : dice <= 8 ? 0 : dice <= 11 ? 1 : dice <= 13 ? 2 : 3;
                    break;
                case Kind.Dead:
                    dice = Roll(10);
                    dy = dice <= 2 ? -1 : dice <= 6 ? 0 : 1;
                    dx = Roll(3) - 1;
                    break;
            }
        }

        // cbonsai's chooseString and chooseColor.
        string ClassicString(Kind kind, int life, int dx, int dy)
        {
            if (life < 4) kind = Kind.Dying;
            switch (kind)
            {
                case Kind.Trunk:
                    if (dy == 0) return "/~";
                    if (dx < 0) return "\\|";
                    if (dx == 0) return "/|\\";
                    return "|/";
                case Kind.ShootLeft:
                    if (dy > 0) return "\\";
                    if (dy == 0) return "\\_";
                    if (dx < 0) return "\\|";
                    if (dx == 0) return "/|";
                    return "/";
                case Kind.ShootRight:
                    if (dy > 0) return "/";
                    if (dy == 0) return "_/";
                    if (dx < 0) return "\\|";
                    if (dx == 0) return "/|";
                    return "/";
                default:
                    return leaves[Roll(leaves.Length)];
            }
        }

        Role ClassicRole(Kind kind, int life)
        {
            if (life < 4 && kind != Kind.Dead) kind = Kind.Dying;
            switch (kind)
            {
                case Kind.Dying: return Roll(10) == 0 ? Role.LeafLit : Role.Leaf;
                case Kind.Dead: return Roll(3) == 0 ? Role.LeafLit : Role.LeafShade;
                default: return Roll(2) == 0 ? Role.TrunkLit : Role.Trunk;
            }
        }

        void Draw(Kind kind, int x, int y, int life, int age, int lifeStart, int dx, int dy)
        {
            if (s.Style == Style.Classic || kind == Kind.Dying || kind == Kind.Dead)
            {
                Role role = ClassicRole(kind, life);
                string text = ClassicString(kind, life, dx, dy);
                if (s.Style == Style.Detailed && (kind == Kind.Dying || kind == Kind.Dead)) text = LeafChar(0.8 + rnd.NextDouble() * 0.2).ToString();
                Put(x, y, text, role);
                return;
            }
            if (kind == Kind.Trunk) DetailedTrunk(x, y, life, age, lifeStart, dx, dy);
            else DetailedShoot(kind, x, y, life, dx, dy);
        }

        // ---------------------------------------------------------------- the detailed style

        // The trunk, widest at its root and tapering up: a lit left edge, bark between, a shaded right edge.
        void DetailedTrunk(int x, int y, int life, int age, int lifeStart, int dx, int dy)
        {
            if (life < 4) { Put(x, y, LeafChar(0.85).ToString(), Role.Leaf); return; }
            double left = (double)life / Math.Max(1, lifeStart);
            int half = left > 0.75 ? 2 : left > 0.45 ? 1 : 0;
            if (age <= 2)
            {
                // Root flare, spreading over the soil.
                Put(x - 4, y, "_/", Role.Trunk);
                Put(x - 2, y, "(", Role.TrunkLit);
                Put(x - 1, y, "|||", Role.Trunk);
                Put(x + 2, y, ")", Role.Trunk);
                Put(x + 3, y, "\\_", Role.Trunk);
                return;
            }
            char edge = dy == 0 ? '~' : dx < 0 ? '\\' : dx > 0 ? '/' : '|';
            if (half == 0 || dy == 0)  // (thick only where it rises: a sideways step stays a thin join)
            {
                Put(x, y, dy == 0 ? "/~" : dx < 0 ? "\\|" : dx > 0 ? "|/" : "|", Roll(3) == 0 ? Role.TrunkLit : Role.Trunk);
                return;
            }
            Put(x - half, y, edge.ToString(), Role.TrunkLit);
            for (int i = -half + 1; i < half; i++)
                Put(x + i, y, dy == 0 ? "~" : (Roll(5) == 0 ? ":" : "|"), Role.Trunk);
            Put(x + half, y, edge.ToString(), Role.Trunk);
        }

        void DetailedShoot(Kind kind, int x, int y, int life, int dx, int dy)
        {
            string text;
            if (dy > 0) text = kind == Kind.ShootLeft ? "\\" : "/";
            else if (dy == 0) text = kind == Kind.ShootLeft ? "\\_" : "_/";
            else if (dx < 0) text = "\\";
            else if (dx > 0) text = "/";
            else text = "|";
            Put(x, y, text, Roll(3) == 0 ? Role.TrunkLit : Role.Trunk);
        }

        // Dense in the middle of a cloud, light at its edges (d: 0 the middle, 1 the edge).
        char LeafChar(double d)
        {
            if (d < 0.35) return Roll(3) == 0 ? '%' : '@';
            if (d < 0.65) return Roll(4) == 0 ? '%' : '&';
            if (d < 0.85) return Roll(3) == 0 ? '*' : '&';
            return Roll(3) == 0 ? '\u00b7' : '*';
        }

        // A leaf cloud around a branch's end, lit from above: bright on top, shaded underneath. It opens
        // from the middle out, a ring at a time. Its size grows with the canopy setting and with the
        // tree (a big tree's clouds are big, as on a real bonsai); scale is its share of a full cloud.
        double CloudWidth()  // a full cloud's half width, in cells
        {
            double treeScale = Math.Max(0.7, Math.Min(2.5, Math.Sqrt(s.Life / 56.0)));
            return (4 + s.CanopySize * 1.6) * treeScale;
        }

        void Cloud(int cx, int cy, double scale)
        {
            double rx = (CloudWidth() + rnd.NextDouble() * 3) * scale;
            double ry = Math.Max(1.5, rx * 0.38);  // (cells are about twice as tall as wide: a flat-topped pad)
            cy -= (int)Math.Round(ry * 0.35);     // (mostly above the branch's end)
            var rings = new List<KeyValuePair<double, Cell>>();
            int bx = (int)Math.Ceiling(rx), by = (int)Math.Ceiling(ry);
            double wobble = rnd.NextDouble() * Math.PI * 2;
            for (int oy = -by; oy <= by; oy++)
                for (int ox = -bx; ox <= bx; ox++)
                {
                    double nx = ox / rx, ny = oy / ry;
                    double angle = Math.Atan2(ny, nx);
                    double edge = 1 + 0.18 * Math.Sin(angle * 3 + wobble) + 0.1 * Math.Sin(angle * 5 - wobble);
                    double d = Math.Sqrt(nx * nx + ny * ny) / edge;
                    if (d > 1) continue;
                    if (d > 0.8 && Roll(3) == 0) continue;  // a ragged edge
                    double lightY = ny + (rnd.NextDouble() - 0.5) * 0.45;  // (dappled, not a straight band)
                    Role role = lightY < -0.4 ? Role.LeafLit : lightY > 0.3 ? Role.LeafShade : Role.Leaf;
                    if (Roll(9) == 0) role = Roll(2) == 0 ? Role.Leaf : role == Role.LeafLit ? Role.Leaf : Role.LeafShade;
                    rings.Add(new KeyValuePair<double, Cell>(d, new Cell(cx + ox, cy + oy, LeafChar(d), role)));
                }
            rings.Sort((a, b) => a.Key.CompareTo(b.Key));
            int perStep = Math.Max(3, rings.Count / 6);
            foreach (var r in rings)
            {
                pending.Add(r.Value);
                if (pending.Count >= perStep) tree.Add(pending);
            }
            tree.Add(pending);
        }

        // ---------------------------------------------------------------- the message (cbonsai's -m)

        void Message()
        {
            string text = (s.Message ?? "").Trim();
            if (text.Length == 0) return;
            var lines = Wrap(text, 32);
            int w = 0;
            foreach (var l in lines) w = Math.Max(w, l.Length);
            // Beside the tree, at mid height, right of whatever grew there.
            int maxX = 0, minY = 0;
            foreach (var step in tree.Steps)
                foreach (var c in step) { if (c.X > maxX) maxX = c.X; if (c.Y < minY) minY = c.Y; }
            int x = maxX + 4, y = minY / 2 - lines.Count / 2 - 1;
            Put(x, y, "+" + new string('-', w + 2) + "+", Role.Pot);
            for (int i = 0; i < lines.Count; i++)
            {
                Put(x, y + 1 + i, "|", Role.Pot);
                Put(x + 2, y + 1 + i, lines[i], Role.LeafLit);
                Put(x + w + 3, y + 1 + i, "|", Role.Pot);
            }
            Put(x, y + 1 + lines.Count, "+" + new string('-', w + 2) + "+", Role.Pot);
            pending.RemoveAll(c => c.Ch == ' ');
            tree.Add(pending);
        }

        static List<string> Wrap(string text, int width)
        {
            var lines = new List<string>();
            foreach (var para in text.Replace("\r", "").Split('\n'))
            {
                string line = "";
                foreach (var word in para.Split(' '))
                {
                    if (line.Length > 0 && line.Length + 1 + word.Length > width) { lines.Add(line); line = ""; }
                    line = line.Length == 0 ? word : line + " " + word;
                }
                lines.Add(line);
            }
            return lines;
        }
    }
}
