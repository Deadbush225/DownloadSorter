# Read version from manifest.json
$manifest = Get-Content -Raw -Path "./manifest.json" | ConvertFrom-Json
$version = "$($manifest.version)".Trim()
Write-Host "Building installer version $version"

$installDir = Resolve-Path "./install"
$mainExe = Join-Path $installDir "Download Sorter.exe"
$updaterExe = Join-Path $installDir "Updater.exe"

# If Qt DLLs not present (e.g., deploy skipped), try windeployqt as fallback
$qtDllPresent = Test-Path (Join-Path $installDir "Qt6Core.dll")
if (-not $qtDllPresent) {
  Write-Host "Qt runtime not found in install/. Attempting windeployqt fallback..."
  $qtDir = $env:Qt6_DIR
  if (-not $qtDir) { throw "Qt6_DIR environment variable not set" }
  $windeploy = Join-Path $qtDir "bin/windeployqt.exe"
  if (-not (Test-Path $windeploy)) { throw "windeployqt not found at $windeploy" }
  & $windeploy --release --dir "$installDir" "$mainExe"
  if (Test-Path $updaterExe) { & $windeploy --release --dir "$installDir" "$updaterExe" }
}

# Build the Windows installer with Inno Setup, injecting version
Start-Process "ISCC.exe" -ArgumentList "/DMyAppVersion=$version ./installer.iss" -NoNewWindow -Wait

# List the produced installer(s)
Get-ChildItem -Path "./windows-installer" -Filter "*.exe" | ForEach-Object { Write-Host "Built installer: $($_.FullName)" }
