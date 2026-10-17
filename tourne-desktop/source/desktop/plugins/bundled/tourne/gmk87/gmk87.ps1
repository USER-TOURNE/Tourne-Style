# Tourne Desktop's GMK87 helper: talks to a Zuoya GMK87 (wired 320F:5055, or its 2.4 GHz dongle 320F:5088)
# over the keyboard's own configuration channel (the HID collection on usage page 0xFF1C), as worked out by
# gmk87-node (codedgar, MIT) and its reference.py (Jochen Eisinger, BSD) from the official app's traffic.
#
# Every report is 64 bytes: [0] report id 4, [1-2] checksum (sum of bytes 3..63, little-endian), [3] command,
# [4] data length, [5-7] position (24-bit little-endian), [8..] up to 56 bytes of data. The keyboard answers each
# with a report whose byte 3 is the same command.
#   0x01 open, 0x02 close (after 100 ms), 0x03 part of reading the settings, 0x05 read 4 bytes of the settings,
#   0x06 write the settings, 0x23 start a picture upload, 0x21 picture data.
# The settings are 48 bytes: 1 underglow effect (0-18), 2 brightness (0-9), 3 speed (0-9), 4 orientation, 5 rainbow,
# 6-8 underglow colour, 21 Win lock, 28 key LED mode (0-4), 29 saturation (0-9), 31 key LED rainbow, 32 key LED
# colour (0-8), 33 screen (0 clock, 1 picture 1, 2 picture 2), 34 picture 1 frames, 35-41 the clock (BCD but the
# weekday), 43-44 frame time in ms, 46 picture 2 frames.
#
#   gmk87.ps1 -Op info                       the settings, as JSON
#   gmk87.ps1 -Op set -Values k=v,k=v       change named settings (and set the clock)
#   gmk87.ps1 -Op time                       set the clock to now
#   gmk87.ps1 -Op picture -Slot1 a.gif -Slot2 b.png -Delay 100 -Show 1
# One JSON line is printed: {ok, ...} or {ok:false, error}.

param(
  [string]$Op = 'info',
  [string]$Values = '',
  [string]$Slot1 = '',
  [string]$Slot2 = '',
  [int]$Delay = 100,
  [int]$Show = -1
)
$ErrorActionPreference = 'Stop'

Add-Type -TypeDefinition @'
using System;
using System.IO;
using System.Runtime.InteropServices;
using System.Threading;
using Microsoft.Win32.SafeHandles;
public static class Gmk {
  [DllImport("hid.dll")] static extern void HidD_GetHidGuid(out Guid g);
  [DllImport("hid.dll")] static extern bool HidD_GetPreparsedData(SafeFileHandle h, out IntPtr p);
  [DllImport("hid.dll")] static extern bool HidD_FreePreparsedData(IntPtr p);
  [DllImport("hid.dll")] static extern int HidP_GetCaps(IntPtr p, out CAPS c);
  [StructLayout(LayoutKind.Sequential)] struct CAPS { public ushort Usage, UsagePage, InLen, OutLen, FeatLen; [MarshalAs(UnmanagedType.ByValArray, SizeConst=17)] public ushort[] r; public ushort a,b,c,d,e,f,g,h,i,j; }
  [DllImport("setupapi.dll", CharSet=CharSet.Unicode)] static extern IntPtr SetupDiGetClassDevs(ref Guid g, IntPtr e, IntPtr w, int f);
  [StructLayout(LayoutKind.Sequential)] struct IFD { public int cb; public Guid g; public int f; public IntPtr r; }
  [DllImport("setupapi.dll", CharSet=CharSet.Unicode)] static extern bool SetupDiEnumDeviceInterfaces(IntPtr s, IntPtr d, ref Guid g, int i, ref IFD ifd);
  [DllImport("setupapi.dll", CharSet=CharSet.Unicode)] static extern bool SetupDiGetDeviceInterfaceDetail(IntPtr s, ref IFD ifd, IntPtr buf, int size, out int need, IntPtr dev);
  [DllImport("setupapi.dll")] static extern bool SetupDiDestroyDeviceInfoList(IntPtr s);
  [DllImport("kernel32.dll", CharSet=CharSet.Unicode)] static extern SafeFileHandle CreateFile(string n, uint a, uint s, IntPtr sa, uint c, uint f, IntPtr t);

  static FileStream dev;
  public static string Product = "";

  public static bool Open() {
    Guid g; HidD_GetHidGuid(out g);
    IntPtr set = SetupDiGetClassDevs(ref g, IntPtr.Zero, IntPtr.Zero, 0x12);
    try {
      for (int i = 0; ; i++) {
        IFD ifd = new IFD(); ifd.cb = Marshal.SizeOf(ifd);
        if (!SetupDiEnumDeviceInterfaces(set, IntPtr.Zero, ref g, i, ref ifd)) break;
        int need; SetupDiGetDeviceInterfaceDetail(set, ref ifd, IntPtr.Zero, 0, out need, IntPtr.Zero);
        IntPtr buf = Marshal.AllocHGlobal(need); Marshal.WriteInt32(buf, IntPtr.Size == 8 ? 8 : 6);
        string path = null;
        if (SetupDiGetDeviceInterfaceDetail(set, ref ifd, buf, need, out need, IntPtr.Zero)) path = Marshal.PtrToStringUni(buf + 4);
        Marshal.FreeHGlobal(buf);
        if (path == null) continue;
        string low = path.ToLowerInvariant();
        // (the cable only: the 2.4 GHz dongle has this collection but doesn't pass the protocol on)
        if (!low.Contains("vid_320f") || !low.Contains("pid_5055")) continue;
        var h = CreateFile(path, 0xC0000000, 3, IntPtr.Zero, 3, 0x40000000, IntPtr.Zero);
        if (h.IsInvalid) continue;
        IntPtr pp; CAPS c = new CAPS();
        bool ok = HidD_GetPreparsedData(h, out pp);
        if (ok) { HidP_GetCaps(pp, out c); HidD_FreePreparsedData(pp); }
        if (ok && c.UsagePage == 0xFF1C && c.OutLen == 64) {
          dev = new FileStream(h, FileAccess.ReadWrite, 64, true);
          Product = low.Contains("pid_5088") ? "dongle" : "wired";
          return true;
        }
        h.Close();
      }
    } finally { SetupDiDestroyDeviceInfoList(set); }
    return false;
  }

  public static void Close() { if (dev != null) dev.Dispose(); dev = null; if (via != null) via.Dispose(); via = null; }

  // VIA (the raw HID collection on usage page 0xFF60, also through the dongle): 32-byte requests, answered in kind.
  static FileStream via;
  public static bool OpenVia() {
    Guid g; HidD_GetHidGuid(out g);
    IntPtr set = SetupDiGetClassDevs(ref g, IntPtr.Zero, IntPtr.Zero, 0x12);
    try {
      for (int i = 0; ; i++) {
        IFD ifd = new IFD(); ifd.cb = Marshal.SizeOf(ifd);
        if (!SetupDiEnumDeviceInterfaces(set, IntPtr.Zero, ref g, i, ref ifd)) break;
        int need; SetupDiGetDeviceInterfaceDetail(set, ref ifd, IntPtr.Zero, 0, out need, IntPtr.Zero);
        IntPtr buf = Marshal.AllocHGlobal(need); Marshal.WriteInt32(buf, IntPtr.Size == 8 ? 8 : 6);
        string path = null;
        if (SetupDiGetDeviceInterfaceDetail(set, ref ifd, buf, need, out need, IntPtr.Zero)) path = Marshal.PtrToStringUni(buf + 4);
        Marshal.FreeHGlobal(buf);
        if (path == null) continue;
        string low = path.ToLowerInvariant();
        if (!low.Contains("vid_320f") || !(low.Contains("pid_5055") || low.Contains("pid_5088"))) continue;
        var h = CreateFile(path, 0xC0000000, 3, IntPtr.Zero, 3, 0x40000000, IntPtr.Zero);
        if (h.IsInvalid) continue;
        IntPtr pp; CAPS c = new CAPS();
        bool ok = HidD_GetPreparsedData(h, out pp);
        if (ok) { HidP_GetCaps(pp, out c); HidD_FreePreparsedData(pp); }
        if (ok && c.UsagePage == 0xFF60 && c.Usage == 0x61) {
          via = new FileStream(h, FileAccess.ReadWrite, 33, true);
          if (Product == "") Product = low.Contains("pid_5088") ? "dongle" : "wired";
          return true;
        }
        h.Close();
      }
    } finally { SetupDiDestroyDeviceInfoList(set); }
    return false;
  }

  public static byte[] Via(params byte[] ask) {
    var b = new byte[33];
    Array.Copy(ask, 0, b, 1, ask.Length);
    via.Write(b, 0, 33);
    var r = new byte[33];
    var deadline = DateTime.Now.AddSeconds(2);
    while (DateTime.Now < deadline) {
      var t = via.ReadAsync(r, 0, 33);
      var left = deadline - DateTime.Now;
      if (left.TotalMilliseconds <= 0 || !t.Wait(left)) break;
      if (r[1] == ask[0]) { var o = new byte[32]; Array.Copy(r, 1, o, 0, 32); return o; }
    }
    throw new TimeoutException("the keyboard didn't answer (VIA 0x" + ask[0].ToString("x2") + ")");
  }

  // One report: the reply's data (from byte 4), or throws when the keyboard doesn't answer in time.
  public static byte[] Send(byte cmd, byte[] data, int pos) {
    var b = new byte[64];
    b[0] = 4; b[3] = cmd;
    int n = data == null ? 0 : Math.Min(56, data.Length);
    b[4] = (byte)n;
    b[5] = (byte)(pos & 0xff); b[6] = (byte)((pos >> 8) & 0xff); b[7] = (byte)((pos >> 16) & 0xff);
    if (n > 0) Array.Copy(data, 0, b, 8, n);
    int sum = 0;
    for (int i = 3; i < 64; i++) sum += b[i];
    b[1] = (byte)(sum & 0xff); b[2] = (byte)((sum >> 8) & 0xff);
    if (cmd == 0x02) Thread.Sleep(100);
    dev.Write(b, 0, 64);
    var deadline = DateTime.Now.AddSeconds(5);
    var r = new byte[64];
    while (DateTime.Now < deadline) {
      var t = dev.ReadAsync(r, 0, 64);
      var left = deadline - DateTime.Now;
      if (left.TotalMilliseconds <= 0 || !t.Wait(left)) break;
      if (t.Result >= 5 && r[3] == cmd) {
        var o = new byte[60];
        Array.Copy(r, 4, o, 0, 60);
        return o;
      }
    }
    throw new TimeoutException("the keyboard didn't answer command 0x" + cmd.ToString("x2"));
  }

  public static byte[] ReadConfig() {
    Send(0x01, null, 0);
    for (int p = 0; p <= 32; p += 4) Send(0x03, new byte[4], p);
    Send(0x03, new byte[1], 36);
    Send(0x02, null, 0);
    var cfg = new byte[48];
    for (int p = 0; p < 48; p += 4) {
      var r = Send(0x05, new byte[4], p);  // (data starts at reply byte 8: r[4..7])
      Array.Copy(r, 4, cfg, p, 4);
    }
    return cfg;
  }

  public static void WriteConfig(byte[] cfg) {
    Send(0x01, null, 0);
    Send(0x06, cfg, 0);
    Send(0x02, null, 0);
  }

  public static void Upload(byte[] data) {
    Send(0x23, null, 0);
    Send(0x01, null, 0);
    var chunk = new byte[56];
    for (int pos = 0; pos < data.Length; pos += 56) {
      int n = Math.Min(56, data.Length - pos);
      var c = n == 56 ? chunk : new byte[n];
      Array.Copy(data, pos, c, 0, n);
      Send(0x21, c, pos);
    }
    Send(0x02, null, 0);
  }
}
'@

function Out-Json($o) { Write-Output ($o | ConvertTo-Json -Compress -Depth 4) }
function Bcd([int]$v) { [byte](([math]::Floor($v / 10) * 16) + ($v % 10)) }

$fields = [ordered]@{
  effect = 1; brightness = 2; speed = 3; orientation = 4; rainbow = 5; red = 6; green = 7; blue = 8
  winlock = 21; ledMode = 28; saturation = 29; ledRainbow = 31; ledColor = 32; screen = 33
}

function Set-Clock([byte[]]$cfg) {
  $now = Get-Date
  $cfg[35] = Bcd $now.Second; $cfg[36] = Bcd $now.Minute; $cfg[37] = Bcd $now.Hour
  $cfg[38] = [byte]([int]$now.DayOfWeek); if ($cfg[38] -eq 0) { $cfg[38] = 7 }
  $cfg[39] = Bcd $now.Day; $cfg[40] = Bcd $now.Month; $cfg[41] = Bcd ($now.Year - 2000)
}

function Describe([byte[]]$cfg) {
  $o = [ordered]@{ ok = $true; device = [Gmk]::Product }
  foreach ($k in $fields.Keys) { $o[$k] = [int]$cfg[$fields[$k]] }
  $o.frames1 = [int]$cfg[34]; $o.frames2 = [int]$cfg[46]
  $o.delay = [int]$cfg[43] + 256 * [int]$cfg[44]
  return $o
}

# A picture (or GIF) as RGB565 frames, 240 x 135, each zero-padded to 64 KiB.
function Read-Frames([string]$path) {
  Add-Type -AssemblyName System.Drawing
  $out = New-Object System.Collections.Generic.List[byte[]]
  if (-not $path) { return $out }
  $img = [System.Drawing.Image]::FromFile($path)
  try {
    $dim = New-Object System.Drawing.Imaging.FrameDimension $img.FrameDimensionsList[0]
    $count = [math]::Min($img.GetFrameCount($dim), 36)
    for ($f = 0; $f -lt $count; $f++) {
      [void]$img.SelectActiveFrame($dim, $f)
      $bmp = New-Object System.Drawing.Bitmap 240, 135, ([System.Drawing.Imaging.PixelFormat]::Format24bppRgb)
      $g = [System.Drawing.Graphics]::FromImage($bmp)
      $g.InterpolationMode = 'HighQualityBicubic'
      # Cover: filled edge to edge, the middle kept.
      $scale = [math]::Max(240 / $img.Width, 135 / $img.Height)
      $w = $img.Width * $scale; $h = $img.Height * $scale
      $g.DrawImage($img, [single]((240 - $w) / 2), [single]((135 - $h) / 2), [single]$w, [single]$h)
      $g.Dispose()
      $rect = New-Object System.Drawing.Rectangle 0, 0, 240, 135
      $bd = $bmp.LockBits($rect, 'ReadOnly', [System.Drawing.Imaging.PixelFormat]::Format24bppRgb)
      $raw = New-Object byte[] ($bd.Stride * 135)
      [Runtime.InteropServices.Marshal]::Copy($bd.Scan0, $raw, 0, $raw.Length)
      $bmp.UnlockBits($bd); $bmp.Dispose()
      $frame = New-Object byte[] 65536
      $i = 0
      for ($y = 0; $y -lt 135; $y++) {
        $row = $y * $bd.Stride
        for ($x = 0; $x -lt 240; $x++) {
          $b = $raw[$row + $x * 3]; $gg = $raw[$row + $x * 3 + 1]; $r = $raw[$row + $x * 3 + 2]
          $v = (($r -shr 3) -shl 11) -bor (($gg -shr 2) -shl 5) -bor ($b -shr 3)
          $frame[$i] = [byte](($v -shr 8) -band 0xff); $frame[$i + 1] = [byte]($v -band 0xff)
          $i += 2
        }
      }
      $out.Add($frame)
    }
  } finally { $img.Dispose() }
  return $out
}

try {
  if ($Op -eq 'lighting' -or $Op -eq 'light') {
    # VIA's backlight (channel 3): 1 brightness 0-9, 2 effect 0-18, 3 speed 0-4, 4 colour (hue, saturation).
    if (-not [Gmk]::OpenVia()) { Out-Json ([ordered]@{ ok = $false; none = $true; error = 'No GMK87 found (wired or its 2.4 GHz dongle).' }); exit 0 }
    if ($Op -eq 'light') {
      foreach ($pair in ($Values -split ',')) {
        if ($pair -notmatch '^\s*(\w+)\s*=\s*(\d+)(?:/(\d+))?\s*$') { continue }
        $k = $Matches[1]; $v = [byte]([math]::Min(255, [int]$Matches[2]))
        switch ($k) {
          'brightness' { [void][Gmk]::Via(0x07, 3, 1, [byte][math]::Min(9, $v)) }
          'effect' { [void][Gmk]::Via(0x07, 3, 2, [byte][math]::Min(18, $v)) }
          'speed' { [void][Gmk]::Via(0x07, 3, 3, [byte][math]::Min(4, $v)) }
          'color' { $sat = if ($Matches[3]) { [byte]([math]::Min(255, [int]$Matches[3])) } else { [byte]255 }; [void][Gmk]::Via(0x07, 3, 4, $v, $sat) }
        }
      }
      [void][Gmk]::Via(0x09, 3)  # (kept when the keyboard turns off)
    }
    $b = [Gmk]::Via(0x08, 3, 1); $e = [Gmk]::Via(0x08, 3, 2); $s = [Gmk]::Via(0x08, 3, 3); $c = [Gmk]::Via(0x08, 3, 4)
    Out-Json ([ordered]@{ ok = $true; device = [Gmk]::Product; brightness = [int]$b[3]; effect = [int]$e[3]; speed = [int]$s[3]; hue = [int]$c[3]; sat = [int]$c[4] })
    exit 0
  }
  if (-not [Gmk]::Open()) {
    $wireless = [Gmk]::OpenVia()
    Out-Json ([ordered]@{ ok = $false; none = -not $wireless; wireless = $wireless
      error = $(if ($wireless) { 'Pictures, the clock and the other settings need the GMK87 plugged in by its cable.' } else { 'No GMK87 found (wired or its 2.4 GHz dongle).' }) })
    exit 0
  }
  switch ($Op) {
    'info' {
      Out-Json (Describe ([Gmk]::ReadConfig()))
    }
    'set' {
      $cfg = [Gmk]::ReadConfig()
      foreach ($pair in ($Values -split ',')) {
        if ($pair -notmatch '^\s*(\w+)\s*=\s*(\d+)\s*$') { continue }
        $k = $Matches[1]; $v = [int]$Matches[2]
        if ($fields.Contains($k)) { $cfg[$fields[$k]] = [byte]([math]::Min(255, [math]::Max(0, $v))) }
        elseif ($k -eq 'delay') { $d = [math]::Min(65535, [math]::Max(60, $v)); $cfg[43] = [byte]($d -band 0xff); $cfg[44] = [byte]($d -shr 8) }
      }
      Set-Clock $cfg
      [Gmk]::WriteConfig($cfg)
      Out-Json (Describe $cfg)
    }
    'time' {
      $cfg = [Gmk]::ReadConfig()
      Set-Clock $cfg
      [Gmk]::WriteConfig($cfg)
      Out-Json (Describe $cfg)
    }
    'picture' {
      $one = Read-Frames $Slot1
      $two = Read-Frames $Slot2
      if ($one.Count + $two.Count -gt 36) { throw 'At most 36 frames across both pictures.' }
      if ($one.Count + $two.Count -eq 0) { throw 'No picture given.' }
      $cfg = [Gmk]::ReadConfig()
      $cfg[34] = [byte]$one.Count; $cfg[46] = [byte]$two.Count
      $d = [math]::Min(65535, [math]::Max(60, $Delay)); $cfg[43] = [byte]($d -band 0xff); $cfg[44] = [byte]($d -shr 8)
      if ($Show -ge 0) { $cfg[33] = [byte]$Show } elseif ($one.Count) { $cfg[33] = 1 } else { $cfg[33] = 2 }
      Set-Clock $cfg
      [Gmk]::WriteConfig($cfg)
      $all = New-Object System.IO.MemoryStream
      foreach ($f in $one) { $all.Write($f, 0, $f.Length) }
      foreach ($f in $two) { $all.Write($f, 0, $f.Length) }
      [Gmk]::Upload($all.ToArray())
      $o = Describe $cfg; $o.uploaded = $one.Count + $two.Count
      Out-Json $o
    }
    default { throw "Unknown operation '$Op'." }
  }
} catch {
  Out-Json ([ordered]@{ ok = $false; error = "$($_.Exception.Message)".Trim() })
} finally {
  [Gmk]::Close()
}
