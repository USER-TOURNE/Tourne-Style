// bonsai.exe: the same trees in a terminal (Windows Terminal, PowerShell, cmd), with cbonsai's options.
// It uses the screensaver's saved colours and settings; the options below change them for this run only.
// For Scientifica here too, install scientifica.ttf and pick it as the terminal's font.

using System;
using System.Collections.Generic;
using System.Drawing;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;

namespace Bonsai
{
    static class Tui
    {
        const string Usage =
@"Usage: bonsai [OPTION]...

bonsai is a beautifully random bonsai tree generator (after cbonsai).
With no options it grows trees forever (bonsai -l -i); any key stops it.

Options:
  -l, --live             live mode: show each step of growth
  -t, --time=TIME        in live mode, wait TIME secs between steps (default 0.03)
  -i, --infinite         infinite mode: keep growing trees
  -w, --wait=TIME        in infinite mode, wait TIME secs between trees (default 4)
  -S, --style=STYLE      detailed or classic (default: as the screensaver is set)
  -L, --life=INT         life; higher -> more growth (default: as set, 56)
  -M, --multiplier=INT   branch multiplier; higher -> more branching (default 5)
  -c, --leaf=LIST        list of comma-delimited strings for leaves (classic)
  -m, --message=STR      attach message next to the tree
  -b, --base=INT         ascii-art plant base: 0 none, 1 large, 2 small
  -p, --print            print the tree to the terminal when finished
  -s, --seed=INT         seed random number generator
  -n, --no-background    keep the terminal's own background colour
  -h, --help             show help";

        [DllImport("kernel32.dll")] static extern IntPtr GetStdHandle(int handle);
        [DllImport("kernel32.dll")] static extern bool GetConsoleMode(IntPtr handle, out uint mode);
        [DllImport("kernel32.dll")] static extern bool SetConsoleMode(IntPtr handle, uint mode);

        static Settings s;
        static bool live, infinite, print, noBackground;
        static double stepTime = 0.03, wait = 4;
        static int? seed;

        static int Main(string[] args)
        {
            s = Settings.Load();
            s.Life = Math.Min(s.Life, 56);
            if (args.Length == 0) args = new[] { "-l", "-i" };  // just "bonsai": grow trees forever
            try { if (!Parse(args)) return 0; }
            catch (Exception e) { Console.Error.WriteLine("bonsai: " + e.Message); Console.Error.WriteLine("Try 'bonsai -h'."); return 1; }

            // Colours as escape codes (Windows 10 and up understand them once asked).
            try
            {
                IntPtr output = GetStdHandle(-11);
                uint mode;
                if (GetConsoleMode(output, out mode)) SetConsoleMode(output, mode | 0x0004 /* ENABLE_VIRTUAL_TERMINAL_PROCESSING */);
            }
            catch (Exception) { }  // (not a Windows console: the codes go out as they are)
            Console.OutputEncoding = new UTF8Encoding(false);  // (no byte-order mark in front of the tree)

            if (print && !live && !infinite)
            {
                PrintOnce();
                return 0;
            }
            Console.Write("\x1b[?1049h\x1b[?25l");  // the alternate screen, cursor hidden
            try
            {
                var rnd = seed.HasValue ? new Random(seed.Value) : new Random();
                do
                {
                    if (Grow(rnd.Next())) break;
                    if (!infinite) { WaitKey(-1); break; }
                    if (WaitKey(wait)) break;
                } while (true);
            }
            finally
            {
                Console.Write("\x1b[0m\x1b[?25h\x1b[?1049l");
            }
            if (print) PrintOnce();
            return 0;
        }

        static bool Parse(string[] args)
        {
            for (int i = 0; i < args.Length; i++)
            {
                string a = args[i], value = null;
                int eq = a.IndexOf('=');
                if (a.StartsWith("--") && eq > 0) { value = a.Substring(eq + 1); a = a.Substring(0, eq); }
                Func<string> next = () =>
                {
                    if (value != null) return value;
                    if (i + 1 >= args.Length) throw new ArgumentException("option " + a + " needs a value");
                    return args[++i];
                };
                switch (a)
                {
                    case "-l": case "--live": live = true; break;
                    case "-t": case "--time": stepTime = Double(next()); break;
                    case "-i": case "--infinite": infinite = true; live = true; break;
                    case "-w": case "--wait": wait = Double(next()); break;
                    case "-S": case "--style":
                        string st = next().ToLowerInvariant();
                        if (st.StartsWith("c")) s.Style = Style.Classic;
                        else if (st.StartsWith("d")) s.Style = Style.Detailed;
                        else throw new ArgumentException("style is detailed or classic");
                        break;
                    case "-L": case "--life": s.Life = Settings.Clamp(int.Parse(next()), 1, 200); break;
                    case "-M": case "--multiplier": s.Multiplier = Settings.Clamp(int.Parse(next()), 1, 20); break;
                    case "-c": case "--leaf": s.Leaves = next(); break;
                    case "-m": case "--message": s.Message = next(); break;
                    case "-b": case "--base": s.Pot = Settings.Clamp(int.Parse(next()), 0, 2); break;
                    case "-p": case "--print": print = true; break;
                    case "-s": case "--seed": seed = int.Parse(next()); break;
                    case "-n": case "--no-background": noBackground = true; break;
                    case "-h": case "--help": Console.WriteLine(Usage); return false;
                    default: throw new ArgumentException("unknown option " + a);
                }
            }
            return true;
        }

        static double Double(string text)
        {
            return double.Parse(text, System.Globalization.CultureInfo.InvariantCulture);
        }

        // A tree that fits the window, grown from smaller lives until one does.
        static Tree Fit(int seedValue)
        {
            int cols = Math.Max(10, Console.WindowWidth - 1), rows = Math.Max(5, Console.WindowHeight - 1);
            var pick = new Random(seedValue);
            int life = s.Life;
            Tree t = null;
            for (int attempt = 0; attempt < 40; attempt++)
            {
                var copy = s.Clone();
                copy.Life = life;
                t = Grower.GrowToFit(copy, pick.Next(), cols, rows, 4);
                if (t.Width <= cols && t.Height <= rows) return t;
                if (attempt % 4 == 3) life = Math.Max(8, life * 4 / 5);
            }
            return t;
        }

        // Grows one tree; true if a key was pressed to quit.
        static bool Grow(int seedValue)
        {
            Tree t = Fit(seedValue);
            int cols = Console.WindowWidth, rows = Console.WindowHeight;
            int ox = (cols - t.Width) / 2 - t.MinX, oy = rows - 1 - t.MaxY;  // standing on the bottom row
            var sb = new StringBuilder();
            sb.Append(Background());
            sb.Append("\x1b[2J\x1b[H");
            Console.Write(sb.ToString());
            foreach (var step in t.Steps)
            {
                sb.Clear();
                foreach (var c in step)
                {
                    int x = c.X + ox, y = c.Y + oy;
                    if (x < 0 || y < 0 || x >= cols || y >= rows) continue;
                    sb.Append("\x1b[").Append(y + 1).Append(';').Append(x + 1).Append('H');
                    sb.Append(Foreground(s[c.Role])).Append(c.Ch);
                }
                Console.Write(sb.ToString());
                if (live)
                {
                    if (WaitKey(stepTime)) return true;
                }
            }
            Console.Write("\x1b[" + rows + ";1H");
            last = t;
            return false;
        }

        static Tree last;

        // The tree as plain lines with colours, left in the terminal (cbonsai's -p).
        static void PrintOnce()
        {
            Tree t = last ?? Fit(seed ?? Environment.TickCount);
            var grid = new Dictionary<long, Cell>();
            foreach (var step in t.Steps) foreach (var c in step) grid[((long)c.Y << 32) ^ (uint)c.X] = c;
            var sb = new StringBuilder();
            for (int y = t.MinY; y <= t.MaxY; y++)
            {
                sb.Append(Background());
                for (int x = t.MinX; x <= t.MaxX; x++)
                {
                    Cell c;
                    if (grid.TryGetValue(((long)y << 32) ^ (uint)x, out c)) sb.Append(Foreground(s[c.Role])).Append(c.Ch);
                    else sb.Append(' ');
                }
                sb.Append("\x1b[0m").AppendLine();
            }
            Console.Write(sb.ToString());
        }

        static string Foreground(Color c) { return "\x1b[38;2;" + c.R + ";" + c.G + ";" + c.B + "m"; }

        static string Background()
        {
            if (noBackground) return "\x1b[49m";
            Color c = s[Role.Background];
            return "\x1b[48;2;" + c.R + ";" + c.G + ";" + c.B + "m";
        }

        // Waits (seconds; below 0, until a key); true if a key was pressed.
        static bool WaitKey(double seconds)
        {
            DateTime end = DateTime.UtcNow.AddSeconds(seconds < 0 ? 1e6 : seconds);
            do
            {
                try
                {
                    if (Console.KeyAvailable) { Console.ReadKey(true); return true; }
                }
                catch (InvalidOperationException) { }  // (input redirected: no keys to read)
                Thread.Sleep(10);
            } while (DateTime.UtcNow < end);
            return false;
        }
    }
}
