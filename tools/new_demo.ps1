param([Parameter(Mandatory=$true)][string]$Name)
$root = Split-Path -Parent $PSScriptRoot
if (Test-Path "$root/demos/$Name") { Write-Error "demos/$Name already exists"; exit 1 }
Copy-Item -Recurse "$root/demos/_template" "$root/demos/$Name"
Get-ChildItem -Recurse "$root/demos/$Name" -File | ForEach-Object {
    $c = Get-Content $_.FullName -Raw
    Set-Content -Path $_.FullName -Value $c.Replace('_template', $Name) -NoNewline
}
New-Item -ItemType Directory -Force "$root/assets/$Name" | Out-Null
New-Item -ItemType File -Force "$root/assets/$Name/.gitkeep" | Out-Null
$cm = "$root/CMakeLists.txt"
if (-not (Select-String -Path $cm -Pattern "add_demo\($Name\)" -Quiet)) {
    Add-Content -Path $cm -Value "add_demo($Name)"
}
Write-Host "demo '$Name' created. Build with: cmake --build build/debug --target $Name"
# 用法：pwsh tools/new_demo.ps1 demo2 一键得到目录、改名后的骨架、资产目录、CMake 注册行。
