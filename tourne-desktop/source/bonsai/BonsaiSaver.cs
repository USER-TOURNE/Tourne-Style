// BonsaiSaver.scr: a cbonsai screensaver. Windows starts it with /s to run, /p <window> for the little
// preview in Screen Saver Settings, and /c (or nothing) for its settings.
//
// Each tree is drawn in Scientifica (built into the .scr) at the font's own pixel size, 5 x 11 a
// character, into a small picture, and that picture is enlarged by whole pixels to fill the screen, so
// every edge stays sharp. Twelve trees are grown off screen and the one that fills the screen best is
// the one shown. It grows a step at a time, stays for a while, then the next one grows. A logo (any
// picture, its white background made see-through if asked) can sit in the top right corner.

using System;
using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Drawing.Imaging;
using System.Drawing.Text;
using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Windows.Forms;

namespace Bonsai
{
    static class Program
    {
        [STAThread]
        static void Main(string[] args)
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            string mode = args.Length > 0 ? args[0].Trim().ToLowerInvariant() : "/c";
            string arg = args.Length > 1 ? args[1] : null;
            if (mode.Length > 2 && mode[2] == ':') { arg = mode.Substring(3); mode = mode.Substring(0, 2); }  // "/p:1234"
            mode = mode.Replace('-', '/');

            var settings = Settings.Load();
            // The tree in real pixels (Windows stretching it afterwards would blur the font); the settings
            // window is left to Windows' own scaling.
            if (mode == "/s" || mode == "/p") Native.SetProcessDPIAware();
            if (mode == "/s")
            {
                Cursor.Hide();
                var forms = new List<Form>();
                foreach (Screen screen in Screen.AllScreens)
                {
                    bool tree = settings.AllMonitors || screen.Primary;
                    var f = new SaverForm(settings, screen.Bounds, tree, IntPtr.Zero);
                    forms.Add(f);
                    f.Show();
                }
                Application.Run(new MultiFormContext(forms));
            }
            else if (mode == "/p")
            {
                long handle;
                if (arg == null || !long.TryParse(arg, out handle)) return;
                var parent = new IntPtr(handle);
                Native.RECT rc;
                if (!Native.GetClientRect(parent, out rc)) return;
                var f = new SaverForm(settings, new Rectangle(0, 0, rc.Right - rc.Left, rc.Bottom - rc.Top), true, parent);
                Application.Run(f);
            }
            else
            {
                Application.Run(new SettingsForm(settings));
            }
        }
    }

    // Ends when every screen's window has closed (any one closing closes the rest).
    class MultiFormContext : ApplicationContext
    {
        readonly List<Form> forms;
        bool closing;

        public MultiFormContext(List<Form> forms)
        {
            this.forms = forms;
            foreach (var f in forms) f.FormClosed += OnClosed;
        }

        void OnClosed(object sender, FormClosedEventArgs e)
        {
            if (!closing)
            {
                closing = true;
                foreach (var f in forms.ToArray()) if (!f.IsDisposed && f != sender) f.Close();
            }
            ExitThread();
        }
    }

    static class Native
    {
        [StructLayout(LayoutKind.Sequential)]
        public struct RECT { public int Left, Top, Right, Bottom; }

        [DllImport("user32.dll")] public static extern bool GetClientRect(IntPtr hWnd, out RECT rect);
        [DllImport("user32.dll")] public static extern IntPtr SetParent(IntPtr child, IntPtr parent);
        [DllImport("user32.dll")] public static extern int SetWindowLong(IntPtr hWnd, int index, int value);
        [DllImport("user32.dll")] public static extern int GetWindowLong(IntPtr hWnd, int index);
        [DllImport("user32.dll")] public static extern bool IsWindow(IntPtr hWnd);
        [DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
        public const int GWL_STYLE = -16, WS_CHILD = 0x40000000;
    }

    // Scientifica, from inside the .scr (else beside it, else Consolas), and its character cell.
    static class CellFont
    {
        public static Font Font;
        public static int Width = 5, Height = 11;
        static PrivateFontCollection fonts;
        static IntPtr memory;

        public static void Load()
        {
            if (Font != null) return;
            try
            {
                byte[] data = null;
                using (Stream s = Assembly.GetExecutingAssembly().GetManifestResourceStream("scientifica.ttf"))
                {
                    if (s != null)
                    {
                        data = new byte[s.Length];
                        s.Read(data, 0, data.Length);
                    }
                }
                if (data == null)
                {
                    string beside = Path.Combine(Path.GetDirectoryName(Application.ExecutablePath), "scientifica.ttf");
                    if (File.Exists(beside)) data = File.ReadAllBytes(beside);
                }
                if (data != null)
                {
                    fonts = new PrivateFontCollection();
                    memory = Marshal.AllocCoTaskMem(data.Length);  // (kept for as long as the font is used)
                    Marshal.Copy(data, 0, memory, data.Length);
                    fonts.AddMemoryFont(memory, data.Length);
                    // 1100 units to the em, 500 a character: at 11 px, a 5 x 11 cell.
                    Font = new Font(fonts.Families[0], 11f, FontStyle.Regular, GraphicsUnit.Pixel);
                    Width = 5;
                    Height = 11;
                    return;
                }
            }
            catch (Exception) { }
            Font = new Font("Consolas", 16f, FontStyle.Regular, GraphicsUnit.Pixel);
            using (var b = new Bitmap(1, 1))
            using (var g = Graphics.FromImage(b))
            {
                SizeF size = g.MeasureString("MMMMMMMMMM", Font, PointF.Empty, StringFormat.GenericTypographic);
                Width = Math.Max(1, (int)Math.Round(size.Width / 10));
                Height = Font.Height;
            }
        }
    }

    class SaverForm : Form
    {
        readonly Settings settings;
        readonly bool withTree;
        readonly bool preview;
        readonly Random seeds = new Random();
        readonly Timer timer = new Timer();
        readonly Dictionary<Role, SolidBrush> brushes = new Dictionary<Role, SolidBrush>();
        readonly StringFormat format;

        Tree tree;
        Bitmap logo;          // the top right picture, if any
        Bitmap canvas;        // the tree at the font's own size
        Rectangle dest;       // where it's shown, enlarged
        int shown;            // steps drawn so far
        DateTime started, grown;
        Point? firstMouse;

        public SaverForm(Settings settings, Rectangle bounds, bool withTree, IntPtr parent)
        {
            this.settings = settings;
            this.withTree = withTree;
            preview = parent != IntPtr.Zero;
            FormBorderStyle = FormBorderStyle.None;
            StartPosition = FormStartPosition.Manual;
            BackColor = settings[Role.Background];
            ShowInTaskbar = false;
            DoubleBuffered = true;
            Bounds = bounds;
            if (preview)
            {
                Native.SetWindowLong(Handle, Native.GWL_STYLE, Native.GetWindowLong(Handle, Native.GWL_STYLE) | Native.WS_CHILD);
                Native.SetParent(Handle, parent);
                Location = Point.Empty;
                Size = bounds.Size;
            }
            else
            {
                TopMost = true;
            }
            foreach (Role r in Enum.GetValues(typeof(Role))) brushes[r] = new SolidBrush(settings[r]);
            format = (StringFormat)StringFormat.GenericTypographic.Clone();
            format.FormatFlags |= StringFormatFlags.NoClip | StringFormatFlags.MeasureTrailingSpaces;
            CellFont.Load();
            if (withTree) logo = Logo.Load(settings);
            timer.Interval = 15;
            timer.Tick += Tick;
            parentWindow = parent;
        }

        readonly IntPtr parentWindow;

        protected override void OnShown(EventArgs e)
        {
            base.OnShown(e);
            if (withTree) NextTree();
            timer.Start();
        }

        // A new tree, chosen to fill this screen, on a clear canvas.
        void NextTree()
        {
            int cw = CellFont.Width, ch = CellFont.Height;
            double fill = settings.FillScreen ? settings.FillPercent / 100.0 : 1.0;
            double cellsWide = ClientSize.Width * fill / cw, cellsHigh = ClientSize.Height * fill / ch;
            tree = Grower.GrowToFit(settings, seeds.Next(), Math.Max(1, cellsWide), Math.Max(1, cellsHigh), settings.FillScreen ? 12 : 1);

            if (canvas != null) canvas.Dispose();
            canvas = new Bitmap(tree.Width * cw, tree.Height * ch, PixelFormat.Format32bppRgb);
            using (var g = Graphics.FromImage(canvas)) g.Clear(settings[Role.Background]);

            int pw = canvas.Width, ph = canvas.Height;
            if (preview)
            {
                // The small preview: scaled to fit, whatever that comes to.
                double z = Math.Min(ClientSize.Width * 0.9 / pw, ClientSize.Height * 0.9 / ph);
                int w = Math.Max(1, (int)(pw * z)), h = Math.Max(1, (int)(ph * z));
                dest = new Rectangle((ClientSize.Width - w) / 2, (ClientSize.Height - h) / 2, w, h);
            }
            else
            {
                int zoom = settings.FillScreen
                    ? Math.Max(1, (int)Math.Floor(Math.Min(ClientSize.Width * fill / pw, ClientSize.Height * fill / ph)))
                    : settings.Zoom;
                int w = pw * zoom, h = ph * zoom;
                dest = new Rectangle((ClientSize.Width - w) / 2, (ClientSize.Height - h) / 2, w, h);
            }
            shown = 0;
            started = DateTime.UtcNow;
            grown = DateTime.MinValue;
            Invalidate();
        }

        void Tick(object sender, EventArgs e)
        {
            if (preview && !Native.IsWindow(parentWindow)) { Close(); return; }
            if (!withTree || tree == null) return;
            if (shown < tree.Steps.Count)
            {
                double perStep = settings.GrowSeconds * 1000.0 / Math.Max(1, tree.Steps.Count);
                int due = (int)Math.Min(tree.Steps.Count, (DateTime.UtcNow - started).TotalMilliseconds / perStep + 1);
                if (due <= shown) return;
                Rectangle dirty = Rectangle.Empty;
                using (var g = Graphics.FromImage(canvas))
                {
                    g.TextRenderingHint = TextRenderingHint.SingleBitPerPixelGridFit;
                    for (; shown < due; shown++)
                        foreach (Cell c in tree.Steps[shown])
                        {
                            Rectangle cell = DrawCell(g, c);
                            dirty = dirty.IsEmpty ? cell : Rectangle.Union(dirty, cell);
                        }
                }
                if (!dirty.IsEmpty) Invalidate(ToScreen(dirty));
                if (shown >= tree.Steps.Count) grown = DateTime.UtcNow;
            }
            else if ((DateTime.UtcNow - grown).TotalSeconds >= settings.HoldSeconds)
            {
                NextTree();
            }
        }

        Rectangle DrawCell(Graphics g, Cell c)
        {
            int cw = CellFont.Width, ch = CellFont.Height;
            var r = new Rectangle((c.X - tree.MinX) * cw, (c.Y - tree.MinY) * ch, cw, ch);
            g.FillRectangle(brushes[Role.Background], r);
            g.DrawString(c.Ch.ToString(), CellFont.Font, brushes[c.Role], r.X, r.Y, format);
            return r;
        }

        // A part of the canvas, where it is on the screen (a pixel spare all round).
        Rectangle ToScreen(Rectangle r)
        {
            double sx = (double)dest.Width / canvas.Width, sy = (double)dest.Height / canvas.Height;
            int x = dest.X + (int)Math.Floor(r.X * sx) - 1, y = dest.Y + (int)Math.Floor(r.Y * sy) - 1;
            return new Rectangle(x, y, (int)Math.Ceiling(r.Width * sx) + 3, (int)Math.Ceiling(r.Height * sy) + 3);
        }

        protected override void OnPaint(PaintEventArgs e)
        {
            e.Graphics.Clear(settings[Role.Background]);
            if (canvas == null) return;
            e.Graphics.InterpolationMode = InterpolationMode.NearestNeighbor;
            e.Graphics.PixelOffsetMode = PixelOffsetMode.Half;
            e.Graphics.DrawImage(canvas, dest);
            if (logo != null)
            {
                e.Graphics.InterpolationMode = InterpolationMode.HighQualityBicubic;
                e.Graphics.PixelOffsetMode = PixelOffsetMode.HighQuality;
                e.Graphics.DrawImage(logo, LogoPlace());
            }
        }

        // The logo's place: the top right corner, its width a share of the screen's (and at most a
        // quarter of its height), with a margin.
        Rectangle LogoPlace()
        {
            int margin = (int)(Math.Min(ClientSize.Width, ClientSize.Height) * 0.03);
            double w = ClientSize.Width * settings.LogoSize / 100.0, h = w * logo.Height / logo.Width;
            double most = ClientSize.Height * 0.25;
            if (h > most) { w *= most / h; h = most; }
            return new Rectangle(ClientSize.Width - margin - (int)w, margin, Math.Max(1, (int)w), Math.Max(1, (int)h));
        }

        protected override void OnPaintBackground(PaintEventArgs e) { }

        // Any key, click or real movement of the mouse ends it (not in the preview).
        protected override void OnKeyDown(KeyEventArgs e) { if (!preview) Close(); }
        protected override void OnMouseDown(MouseEventArgs e) { if (!preview) Close(); }

        protected override void OnMouseMove(MouseEventArgs e)
        {
            if (preview) return;
            Point p = Cursor.Position;
            if (firstMouse == null) { firstMouse = p; return; }
            if (Math.Abs(p.X - firstMouse.Value.X) > 8 || Math.Abs(p.Y - firstMouse.Value.Y) > 8) Close();
        }

        protected override void OnFormClosed(FormClosedEventArgs e)
        {
            timer.Stop();
            if (canvas != null) canvas.Dispose();
            if (logo != null) logo.Dispose();
            foreach (var b in brushes.Values) b.Dispose();
            base.OnFormClosed(e);
        }
    }

    // The logo picture: read once (the file isn't kept open), its white made see-through if asked.
    static class Logo
    {
        public static string FullPath(string path)
        {
            if (string.IsNullOrEmpty(path)) return null;
            path = Environment.ExpandEnvironmentVariables(path.Trim().Trim('"'));
            if (!Path.IsPathRooted(path))
                path = Path.Combine(Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location), path);
            return path;
        }

        public static Bitmap Load(Settings s)
        {
            string path = FullPath(s.Logo);
            if (s.LogoSize <= 0 || path == null || !File.Exists(path)) return null;
            try
            {
                Bitmap copy;
                using (var stream = new MemoryStream(File.ReadAllBytes(path)))
                using (var image = Image.FromStream(stream))
                {
                    copy = new Bitmap(image.Width, image.Height, PixelFormat.Format32bppArgb);
                    using (var g = Graphics.FromImage(copy)) g.DrawImage(image, 0, 0, image.Width, image.Height);
                }
                if (s.LogoKeyWhite) KeyWhite(copy);
                return copy;
            }
            catch (Exception) { return null; }
        }

        // White to see-through, as GIMP's "Color to Alpha": each pixel keeps only as much opacity as it
        // differs from white, and its colour is what, laid over white, gives the original. Soft shadows
        // and glows stay soft.
        static void KeyWhite(Bitmap b)
        {
            var rect = new Rectangle(0, 0, b.Width, b.Height);
            BitmapData data = b.LockBits(rect, ImageLockMode.ReadWrite, PixelFormat.Format32bppArgb);
            var px = new int[b.Width * b.Height];
            for (int y = 0; y < b.Height; y++) Marshal.Copy(data.Scan0 + y * data.Stride, px, y * b.Width, b.Width);
            for (int i = 0; i < px.Length; i++)
            {
                int a0 = (px[i] >> 24) & 255, r = (px[i] >> 16) & 255, g = (px[i] >> 8) & 255, bl = px[i] & 255;
                int away = Math.Max(255 - r, Math.Max(255 - g, 255 - bl));  // how far from white
                if (away == 0) { px[i] = 0; continue; }
                double a = away / 255.0;
                int nr = Clamp255(255 - (255 - r) / a), ng = Clamp255(255 - (255 - g) / a), nb = Clamp255(255 - (255 - bl) / a);
                int na = (int)Math.Round(a * a0);
                px[i] = (na << 24) | (nr << 16) | (ng << 8) | nb;
            }
            for (int y = 0; y < b.Height; y++) Marshal.Copy(px, y * b.Width, data.Scan0 + y * data.Stride, b.Width);
            b.UnlockBits(data);
        }

        static int Clamp255(double v) { return v < 0 ? 0 : v > 255 ? 255 : (int)Math.Round(v); }
    }

    // The settings, from Screen Saver Settings' "Settings" button (or the .scr's "Configure").
    class SettingsForm : Form
    {
        readonly Settings s;
        readonly ComboBox style = new ComboBox(), size = new ComboBox(), pot = new ComboBox(), monitors = new ComboBox();
        readonly NumericUpDown life = Num(8, 200), multiplier = Num(1, 20), grow = Num(1, 600), hold = Num(0, 600),
                               fill = Num(30, 100), zoom = Num(1, 12);
        readonly TrackBar density = Track(1, 10), canopy = Track(0, 10);
        readonly TextBox leaves = new TextBox(), message = new TextBox(), logo = new TextBox();
        readonly NumericUpDown logoSize = Num(0, 100);
        readonly CheckBox logoKeyWhite = new CheckBox { Text = "", AutoSize = true };
        readonly Button[] swatches = new Button[Settings.RoleKeys.Length];
        readonly TextBox[] hexes = new TextBox[Settings.RoleKeys.Length];
        readonly Color[] colors;

        public SettingsForm(Settings settings)
        {
            s = settings;
            colors = (Color[])s.Colors.Clone();
            Text = "Bonsai screensaver settings";
            FormBorderStyle = FormBorderStyle.FixedDialog;
            MaximizeBox = MinimizeBox = false;
            StartPosition = FormStartPosition.CenterScreen;
            Font = new Font("Segoe UI", 9f);
            ClientSize = new Size(640, 560);

            var tree = Group("Tree", 10, 10, 305, 300);
            int y = 22;
            Row(tree, "Style", Combo(style, "Detailed", "Classic cbonsai"), ref y);
            Row(tree, "Life (how big)", life, ref y);
            Row(tree, "Multiplier (how bushy)", multiplier, ref y);
            Row(tree, "Branch density", density, ref y);
            Row(tree, "Canopy size", canopy, ref y);
            Row(tree, "Pot", Combo(pot, "None", "Large", "Small"), ref y);
            Row(tree, "Leaf characters (Classic)", leaves, ref y);
            Row(tree, "Message", message, ref y);

            var show = Group("Showing", 10, 315, 305, 145);
            y = 22;
            Row(show, "Seconds to grow", grow, ref y);
            Row(show, "Seconds to stay", hold, ref y);
            Row(show, "Size", Combo(size, "Fill the screen", "Fixed zoom"), ref y);
            Row(show, "Screen fill %  /  zoom", Pair(fill, zoom), ref y);

            var look = Group("Colours", 325, 10, 305, 300);
            y = 22;
            for (int i = 0; i < swatches.Length; i++)
            {
                int index = i;
                var swatch = new Button { Width = 28, Height = 22, FlatStyle = FlatStyle.Flat };
                swatch.Click += delegate { PickColor(index); };
                var hex = new TextBox { Width = 70 };
                hex.TextChanged += delegate { Color c; if (Settings.TryParseHex(hexes[index].Text, out c)) { colors[index] = c; swatches[index].BackColor = c; } };
                swatches[i] = swatch;
                hexes[i] = hex;
                var panel = new FlowLayoutPanel { AutoSize = true, AutoSizeMode = AutoSizeMode.GrowAndShrink, Margin = Padding.Empty, WrapContents = false };
                panel.Controls.Add(swatch);
                panel.Controls.Add(hex);
                Row(look, Settings.RoleNames[i], panel, ref y);
            }
            var reset = new Button { Text = "Reset colours", Left = 12, Top = y + 4, Width = 120 };
            reset.Click += delegate { for (int i = 0; i < colors.Length; i++) SetColor(i, Settings.DefaultColors[i]); };
            look.Controls.Add(reset);

            var other = Group("Screens", 325, 315, 305, 70);
            y = 22;
            Row(other, "Monitors", Combo(monitors, "A tree on every screen", "Main screen only"), ref y);

            var mark = Group("Logo (top right)", 325, 390, 305, 125);
            y = 22;
            var browse = new Button { Text = "...", Width = 32, Height = 23 };
            browse.Click += delegate { PickLogo(); };
            logo.Width = 102;
            var pick = new FlowLayoutPanel { AutoSize = true, AutoSizeMode = AutoSizeMode.GrowAndShrink, Margin = Padding.Empty, WrapContents = false };
            pick.Controls.Add(logo);
            pick.Controls.Add(browse);
            Row(mark, "Picture", pick, ref y);
            Row(mark, "Width % (0 hides it)", logoSize, ref y);
            Row(mark, "White see-through", logoKeyWhite, ref y);

            var ok = new Button { Text = "OK", Left = 450, Top = 525, Width = 85, DialogResult = DialogResult.OK };
            var cancel = new Button { Text = "Cancel", Left = 545, Top = 525, Width = 85, DialogResult = DialogResult.Cancel };
            ok.Click += delegate { Save(); Close(); };
            cancel.Click += delegate { Close(); };
            Controls.Add(ok);
            Controls.Add(cancel);
            AcceptButton = ok;
            CancelButton = cancel;

            style.SelectedIndex = s.Style == Style.Classic ? 1 : 0;
            life.Value = s.Life;
            multiplier.Value = s.Multiplier;
            density.Value = s.BranchDensity;
            canopy.Value = s.CanopySize;
            pot.SelectedIndex = s.Pot;
            leaves.Text = s.Leaves;
            message.Text = s.Message;
            grow.Value = s.GrowSeconds;
            hold.Value = s.HoldSeconds;
            size.SelectedIndex = s.FillScreen ? 0 : 1;
            fill.Value = s.FillPercent;
            zoom.Value = s.Zoom;
            monitors.SelectedIndex = s.AllMonitors ? 0 : 1;
            logo.Text = s.Logo;
            logoSize.Value = s.LogoSize;
            logoKeyWhite.Checked = s.LogoKeyWhite;
            for (int i = 0; i < colors.Length; i++) SetColor(i, colors[i]);
        }

        void SetColor(int i, Color c)
        {
            colors[i] = c;
            swatches[i].BackColor = c;
            hexes[i].Text = Settings.ToHex(c);
        }

        void PickLogo()
        {
            using (var dialog = new OpenFileDialog { Title = "Logo picture", Filter = "Pictures|*.png;*.jpg;*.jpeg;*.bmp;*.gif|All files|*.*" })
            {
                string now = Logo.FullPath(logo.Text);
                if (now != null && File.Exists(now)) dialog.FileName = now;
                if (dialog.ShowDialog(this) == DialogResult.OK) logo.Text = dialog.FileName;
            }
        }

        void PickColor(int i)
        {
            using (var dialog = new ColorDialog { Color = colors[i], FullOpen = true })
                if (dialog.ShowDialog(this) == DialogResult.OK) SetColor(i, dialog.Color);
        }

        void Save()
        {
            s.Style = style.SelectedIndex == 1 ? Style.Classic : Style.Detailed;
            s.Life = (int)life.Value;
            s.Multiplier = (int)multiplier.Value;
            s.BranchDensity = density.Value;
            s.CanopySize = canopy.Value;
            s.Pot = pot.SelectedIndex;
            s.Leaves = leaves.Text;
            s.Message = message.Text;
            s.GrowSeconds = (int)grow.Value;
            s.HoldSeconds = (int)hold.Value;
            s.FillScreen = size.SelectedIndex == 0;
            s.FillPercent = (int)fill.Value;
            s.Zoom = (int)zoom.Value;
            s.AllMonitors = monitors.SelectedIndex == 0;
            s.Logo = logo.Text.Trim();
            s.LogoSize = (int)logoSize.Value;
            s.LogoKeyWhite = logoKeyWhite.Checked;
            s.Colors = (Color[])colors.Clone();
            try { s.Save(); }
            catch (Exception ex) { MessageBox.Show(this, "The settings couldn't be saved: " + ex.Message, Text); }
        }

        static GroupBox Group(string text, int x, int y, int w, int h)
        {
            return new GroupBox { Text = text, Left = x, Top = y, Width = w, Height = h };
        }

        void Row(GroupBox box, string label, Control control, ref int y)
        {
            if (!Controls.Contains(box)) Controls.Add(box);
            box.Controls.Add(new Label { Text = label, Left = 12, Top = y + 4, Width = 140, AutoEllipsis = true });
            control.Left = 155;
            control.Top = y;
            if (control is TextBox || control is ComboBox) control.Width = 138;
            if (control is TrackBar) { control.Width = 140; control.Height = 30; }
            box.Controls.Add(control);
            y += control is TrackBar ? 34 : 30;
        }

        static ComboBox Combo(ComboBox box, params string[] items)
        {
            box.DropDownStyle = ComboBoxStyle.DropDownList;
            box.Items.AddRange(items);
            return box;
        }

        static NumericUpDown Num(int min, int max)
        {
            return new NumericUpDown { Minimum = min, Maximum = max, Width = 70 };
        }

        static TrackBar Track(int min, int max)
        {
            return new TrackBar { Minimum = min, Maximum = max, TickFrequency = 1, AutoSize = false };
        }

        // (GrowAndShrink: a FlowLayoutPanel starts 200 by 100 and by default only grows, so each
        // row's panel covered the rows under it; only the first colour showed.)
        static Control Pair(Control a, Control b)
        {
            var panel = new FlowLayoutPanel { AutoSize = true, AutoSizeMode = AutoSizeMode.GrowAndShrink, Margin = Padding.Empty, WrapContents = false };
            a.Width = 60;
            b.Width = 60;
            panel.Controls.Add(a);
            panel.Controls.Add(b);
            return panel;
        }
    }
}
