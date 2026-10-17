<#
Compares visual style dumps (from Dump-Msstyles.ps1) against a base, normally stock aero.
For every class: the part of Windows it paints (Get-Surface), and how many of its properties are the same as the
base, changed, missing (the base has them, this style doesn't) or added; for images, whether each is the base's own
picture, a repainted one, or blank on purpose (every pixel transparent).
  pwsh -File Compare-Msstyles.ps1 -Dumps <folder> -Base aero -Styles 'Bouquet_MAC_-_Night','Everforest_Night'
Writes <folder>\compare\<style>-vs-<base>.classes.tsv (one row per class) and <style>-vs-<base>.props.tsv (one row per property that
differs), and prints a summary per surface.
#>
param(
    [Parameter(Mandatory)][string]$Dumps,
    [string]$Base = 'aero',
    [Parameter(Mandatory)][string[]]$Styles
)
$ErrorActionPreference = 'Stop'
. "$PSScriptRoot\Msstyles-Surfaces.ps1"

Add-Type -AssemblyName System.Drawing
$null = [System.Drawing.Bitmap]
$refs = @([AppDomain]::CurrentDomain.GetAssemblies() | Where-Object { $_.Location -match 'System\.Drawing|GdiPlus|System\.Private\.Windows|System\.Runtime|System\.Security\.Cryptography|System\.Memory|netstandard|System\.Private\.CoreLib|System\.ComponentModel' } | ForEach-Object Location)
Add-Type -ReferencedAssemblies $refs -TypeDefinition @'
using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Security.Cryptography;
public static class ImgInfo {
    // "hash|w|h|kind|avgRGB" where kind is blank (all transparent), opaque, or alpha.
    public static string Of(string path) {
        byte[] data = File.ReadAllBytes(path);
        string hash = Convert.ToHexString(SHA1.HashData(data)).Substring(0, 16);
        try {
            using (var bmp = new Bitmap(new MemoryStream(data))) {
                int w = bmp.Width, h = bmp.Height;
                var r = bmp.LockBits(new Rectangle(0, 0, w, h), ImageLockMode.ReadOnly, PixelFormat.Format32bppArgb);
                int n = w * h; var px = new int[n];
                System.Runtime.InteropServices.Marshal.Copy(r.Scan0, px, 0, n);
                bmp.UnlockBits(r);
                // The picture, not the file: a PNG saved again has other bytes but the same pixels.
                var raw = new byte[n * 4 + 8];
                Buffer.BlockCopy(px, 0, raw, 8, n * 4);
                BitConverter.GetBytes(w).CopyTo(raw, 0); BitConverter.GetBytes(h).CopyTo(raw, 4);
                hash = Convert.ToHexString(SHA1.HashData(raw)).Substring(0, 16);
                long sa = 0, sr = 0, sg = 0, sb = 0; int opaque = 0, clear = 0;
                foreach (int p in px) {
                    int a = (p >> 24) & 255; if (a == 255) opaque++; if (a == 0) clear++;
                    sa += a; sr += ((p >> 16) & 255) * a; sg += ((p >> 8) & 255) * a; sb += (p & 255) * a;
                }
                string kind = clear == n ? "blank" : opaque == n ? "opaque" : "alpha";
                string avg = sa == 0 ? "" : string.Format("{0:X2}{1:X2}{2:X2}", sr / sa, sg / sa, sb / sa);
                return hash + "|" + w + "|" + h + "|" + kind + "|" + avg;
            }
        } catch { return hash + "|0|0|unreadable|"; }
    }
}
'@

function Load-Dump([string]$name) {
    $rows = Import-Csv (Join-Path $Dumps "$name.tsv") -Delimiter "`t"
    $map = @{}
    foreach ($r in $rows) {
        $key = "$($r.class)|$($r.part)|$($r.state)|$($r.property)"
        $img = $null
        if ($r.image -like 'res*') {
            $file = Join-Path $Dumps ("$name\img\{0}\p{1}s{2}_{3}.png" -f ($r.class -replace '[^\w\-]+', '_'), $r.part, $r.state, $r.property)
            if (Test-Path -LiteralPath $file) { $img = [ImgInfo]::Of($file) }
        }
        $map[$key] = [pscustomobject]@{ Row = $r; Img = $img }
    }
    $map
}

$out = Join-Path $Dumps 'compare'
New-Item -ItemType Directory -Force $out | Out-Null
$baseMap = Load-Dump $Base
foreach ($style in $Styles) {
    $map = Load-Dump $style
    $classes = @{}
    $diffs = New-Object System.Collections.Generic.List[string]
    $diffs.Add("class`tsurface`tpart`tstate`tproperty`ttype`tstatus`tbase`tstyle")
    $touch = {
        param($cls)
        if (-not $classes.ContainsKey($cls)) {
            $classes[$cls] = [ordered]@{ class = $cls; surface = (Get-Surface $cls); baseProps = 0; styleProps = 0; same = 0; changed = 0; missing = 0; added = 0
                                         imgBase = 0; imgRepainted = 0; imgBlank = 0; imgMissing = 0; imgAdded = 0 }
        }
        $classes[$cls]
    }
    foreach ($k in $baseMap.Keys) {
        $b = $baseMap[$k]; $c = & $touch $b.Row.class; $c.baseProps++
        if ($map.ContainsKey($k)) {
            $s = $map[$k]
            $isImg = $b.Img -or $s.Img
            if ($isImg) {
                $bh = if ($b.Img) { ($b.Img -split '\|')[0] } else { '' }
                $sh = if ($s.Img) { ($s.Img -split '\|')[0] } else { '' }
                $kind = if ($s.Img) { ($s.Img -split '\|')[3] } else { '' }
                if ($bh -eq $sh) { $c.imgBase++; $status = 'image-stock' }
                elseif ($kind -eq 'blank') { $c.imgBlank++; $status = 'image-blank' }
                else { $c.imgRepainted++; $status = 'image-repainted' }
                if ($status -ne 'image-repainted') { $diffs.Add("$($b.Row.class)`t$($c.surface)`t$($b.Row.part)`t$($b.Row.state)`t$($b.Row.property)`t$($b.Row.type)`t$status`t$($b.Img)`t$($s.Img)") }
            } elseif ($b.Row.value -eq $s.Row.value) { $c.same++ }
            else {
                $c.changed++
                $diffs.Add("$($b.Row.class)`t$($c.surface)`t$($b.Row.part)`t$($b.Row.state)`t$($b.Row.property)`t$($b.Row.type)`tchanged`t$($b.Row.value)`t$($s.Row.value)")
            }
        } else {
            $c.missing++
            if ($b.Img) { $c.imgMissing++ }
            $diffs.Add("$($b.Row.class)`t$($c.surface)`t$($b.Row.part)`t$($b.Row.state)`t$($b.Row.property)`t$($b.Row.type)`tmissing`t$(if ($b.Img) { $b.Img } else { $b.Row.value })`t")
        }
    }
    foreach ($k in $map.Keys) {
        $s = $map[$k]; $c = & $touch $s.Row.class; $c.styleProps++
        if (-not $baseMap.ContainsKey($k)) {
            $c.added++
            if ($s.Img) { $c.imgAdded++ }
            $diffs.Add("$($s.Row.class)`t$($c.surface)`t$($s.Row.part)`t$($s.Row.state)`t$($s.Row.property)`t$($s.Row.type)`tadded`t`t$(if ($s.Img) { $s.Img } else { $s.Row.value })")
        }
    }
    $rows = $classes.Values | ForEach-Object { [pscustomobject]$_ } | Sort-Object surface, class
    $rows | Export-Csv (Join-Path $out "$style-vs-$Base.classes.tsv") -Delimiter "`t" -NoTypeInformation -UseQuotes Never
    [IO.File]::WriteAllLines((Join-Path $out "$style-vs-$Base.props.tsv"), $diffs)
    "== $style against $Base"
    $rows | Group-Object surface | ForEach-Object {
        $g = $_.Group
        $present = @($g | Where-Object { $_.styleProps -gt 0 }).Count
        $absent = @($g | Where-Object { $_.styleProps -eq 0 }).Count
        '{0,-34} classes {1,3} (+{2} absent)  same {3,5}  changed {4,5}  missing {5,5}  added {6,4}  img stock {7,4} repainted {8,4} blank {9,3}' -f $_.Name, $present, $absent,
            ($g | Measure-Object same -Sum).Sum, ($g | Measure-Object changed -Sum).Sum, ($g | Measure-Object missing -Sum).Sum, ($g | Measure-Object added -Sum).Sum,
            ($g | Measure-Object imgBase -Sum).Sum, ($g | Measure-Object imgRepainted -Sum).Sum, ($g | Measure-Object imgBlank -Sum).Sum
    }
}
