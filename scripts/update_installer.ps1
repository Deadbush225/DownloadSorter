# Read version from top-level VERSION file
$version = (Get-Content -Raw -Path "./VERSION").Trim()
Write-Host "Building installer version $version"

# Build the Windows installer with Inno Setup, injecting version
Start-Process "ISCC.exe" -ArgumentList "/DMyAppVersion=$version ./installer.iss" -NoNewWindow -Wait

# List the produced installer(s)
Get-ChildItem -Path "./windows-installer" -Filter "*.exe" | ForEach-Object { Write-Host "Built installer: $($_.FullName)" }
