# Read values from manifest.json
$manifest = Get-Content -Raw -Path "./manifest.json" | ConvertFrom-Json
$version = "$($manifest.version)".Trim()
$desktopName = "$($manifest.desktop.desktop_name)".Trim()
$packageId = "$($manifest.desktop.package_id)".Trim()
Write-Host "Building installer version $version for $desktopName (package: $packageId)"

$installDir = Resolve-Path "./install"
$mainExe = Join-Path $installDir "bin/DownloadSorter.exe"
$updaterExe = Join-Path $installDir "bin/eUpdater.exe"

# If Qt DLLs not present (e.g., deploy skipped), try windeployqt as fallback
$qtDllPresent = (Test-Path (Join-Path $installDir "Qt6Core.dll")) -or (Test-Path (Join-Path $installDir "bin/Qt6Core.dll"))
Write-Host "Qt6Core.dll present: $qtDllPresent"

# Disable windeployqt completely - CMake qt_deploy_runtime_dependencies should handle this
# Qt deployment is now handled by CMake deployment script using windeployqt
# No additional deployment needed here

# Build the Windows installer with Inno Setup, passing values as defines
Start-Process "ISCC.exe" -ArgumentList @("/DMyAppVersion=$version", "/DMyAppName=`"$desktopName`"", "/DMyPackageId=$packageId", "./installer.iss") -NoNewWindow -Wait

# List the produced installer(s)
Get-ChildItem -Path "./windows-installer" -Filter "*.exe" | ForEach-Object { Write-Host "Built installer: $($_.FullName)" }
