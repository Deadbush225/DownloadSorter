# Read version from manifest.json
$manifest = Get-Content -Raw -Path "./manifest.json" | ConvertFrom-Json
$version = "$($manifest.version)".Trim()
Write-Host "Building installer version $version"

$installDir = Resolve-Path "./install"
$mainExe = Join-Path $installDir "Download Sorter.exe"
$updaterExe = Join-Path $installDir "Updater.exe"

# If Qt DLLs not present (e.g., deploy skipped), try windeployqt as fallback
$qtDllPresent = Test-Path (Join-Path $installDir "Qt6Core.dll")
Write-Host "Qt6Core.dll present: $qtDllPresent"

# Disable windeployqt completely - CMake qt_deploy_runtime_dependencies should handle this
# Qt deployment is now handled by CMake deployment script using windeployqt
# No additional deployment needed here

# Build the Windows installer with Inno Setup, injecting version
Start-Process "ISCC.exe" -ArgumentList "/DMyAppVersion=$version ./installer.iss" -NoNewWindow -Wait

# List the produced installer(s)
Get-ChildItem -Path "./windows-installer" -Filter "*.exe" | ForEach-Object { Write-Host "Built installer: $($_.FullName)" }
