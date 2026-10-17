# Installs Python 3.12 for all users (winget, on PATH, with the py launcher) and OpenVINO into it.
# Run as administrator. Output goes to the log given.
#
#   .\tools\setup\Install-PythonOpenVINO.ps1 -Log <file>
param([string]$Log = '')
$ErrorActionPreference = 'Stop'
if ($Log) { Start-Transcript -Path $Log -Force | Out-Null }
try {
    winget install --exact --id Python.Python.3.12 --scope machine --silent --accept-package-agreements --accept-source-agreements `
        --override '/quiet InstallAllUsers=1 PrependPath=1 Include_launcher=1 Include_test=0'
    $python = Join-Path $env:ProgramFiles 'Python312\python.exe'
    if (-not (Test-Path $python)) { throw "Python didn't land at $python" }
    & $python --version
    & $python -m pip install --upgrade pip
    & $python -m pip install --upgrade openvino
    & $python -c "import openvino as ov; print('OpenVINO', ov.__version__); print('devices', ov.Core().available_devices)"
} finally {
    if ($Log) { Stop-Transcript | Out-Null }
}
