<#
Dumps a .msstyles with msstyleEditor's own library (libmsstyle, inside msstyleEditor.exe 2.1.2):
every class, part, state and property, with its type and value, into <Out>\<name>.tsv, and every
image the style uses into <Out>\<name>\img\<class>\p<part>s<state>_<property>.png.
Runs under Windows PowerShell 5.1 (msstyleEditor is a .NET Framework program):
  powershell.exe -NoProfile -File Dump-Msstyles.ps1 -Style '...\x.msstyles' -Out '...\out'
Read-only: the style is opened as a data file and never saved.
#>
param(
    [Parameter(Mandatory)][string]$Style,
    [Parameter(Mandatory)][string]$Out,
    [string]$Editor = '%USERPROFILE%\Downloads\.theme files\Claude Files to View for reference\msstyleEditor.exe',
    [switch]$NoImages
)
$ErrorActionPreference = 'Stop'
$asm = [Reflection.Assembly]::UnsafeLoadFrom($Editor)
$vsType = $asm.GetType('libmsstyle.VisualStyle')
$propInfo = $asm.GetType('libmsstyle.VisualStyleProperties').GetField('PROPERTY_INFO_MAP').GetValue($null)
$name = [IO.Path]::GetFileNameWithoutExtension($Style)
$safe = $name -replace '[^\w\-]+', '_'
New-Item -ItemType Directory -Force $Out | Out-Null
$vs = [Activator]::CreateInstance($vsType)
$vs.Load($Style)

$typeNames = @{ 2='DIBDATA'; 8='GLYPHDIBDATA'; 200='ENUM'; 201='STRING'; 202='INT'; 203='BOOL'; 204='COLOR'; 205='MARGINS';
    206='FILENAME'; 207='SIZE'; 208='POSITION'; 209='RECT'; 210='FONT'; 211='INTLIST'; 212='HBITMAP'; 213='DISKSTREAM';
    214='STREAM'; 215='BITMAPREF'; 216='FLOAT'; 217='FLOATLIST'; 240='COLORLIST'; 241='HIGHCONTRASTCOLORTYPE'; 242='BITMAPIMAGETYPE'; 243='FILENAME_LITE' }

function PngSize([byte[]]$d) {
    if ($d -and $d.Length -gt 24 -and $d[0] -eq 0x89 -and $d[1] -eq 0x50) {
        $w = ($d[16] -shl 24) -bor ($d[17] -shl 16) -bor ($d[18] -shl 8) -bor $d[19]
        $h = ($d[20] -shl 24) -bor ($d[21] -shl 16) -bor ($d[22] -shl 8) -bor $d[23]
        return "${w}x${h}"
    }
    return ''
}

$rows = New-Object System.Collections.Generic.List[string]
$rows.Add("class`tclassId`tpart`tpartName`tstate`tstateName`tproperty`tnameId`ttype`tvalue`timage")
$imgDir = Join-Path $Out "$safe\img"
foreach ($c in ($vs.Classes.Values | Sort-Object ClassId)) {
    foreach ($p in ($c.Parts.Values | Sort-Object PartId)) {
        foreach ($s in ($p.States.Values | Sort-Object StateId)) {
            foreach ($prop in $s.Properties) {
                $h = $prop.Header
                $pname = if ($propInfo.ContainsKey($h.nameID)) { $propInfo[$h.nameID].Name } else { "#$($h.nameID)" }
                $tname = if ($typeNames.ContainsKey($h.typeID)) { $typeNames[$h.typeID] } else { "#$($h.typeID)" }
                $value = ''
                try { $value = $prop.GetValueAsString() } catch { $value = '?' }
                if ($null -eq $value) { $value = '' }
                $value = ($value -replace "[`t`r`n]+", ' ')
                if ($value.Length -gt 300) { $value = $value.Substring(0, 300) + '...' }
                $image = ''
                if ($prop.IsImageProperty()) {
                    try {
                        $res = $vs.GetResourceFromProperty($prop)
                        if ($res -and $res.Data) {
                            $image = "res$($res.ResourceId) $($res.Type) $(PngSize $res.Data)"
                            if (-not $NoImages) {
                                $cdir = Join-Path $imgDir ($c.ClassName -replace '[^\w\-]+', '_')
                                New-Item -ItemType Directory -Force $cdir | Out-Null
                                [IO.File]::WriteAllBytes((Join-Path $cdir ("p{0}s{1}_{2}.png" -f $p.PartId, $s.StateId, $pname)), $res.Data)
                            }
                        } else { $image = 'missing' }
                    } catch { $image = 'error' }
                }
                $rows.Add(("{0}`t{1}`t{2}`t{3}`t{4}`t{5}`t{6}`t{7}`t{8}`t{9}`t{10}" -f $c.ClassName, $c.ClassId, $p.PartId, $p.PartName,
                    $s.StateId, $s.StateName, $pname, $h.nameID, $tname, $value, $image))
            }
        }
    }
}
[IO.File]::WriteAllLines((Join-Path $Out "$safe.tsv"), $rows, (New-Object Text.UTF8Encoding $false))
"{0}: platform {1}, {2} classes, {3} properties" -f $name, $vs.Platform, $vs.Classes.Count, ($rows.Count - 1)
$vs.Dispose()
