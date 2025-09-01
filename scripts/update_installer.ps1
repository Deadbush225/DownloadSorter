# Start-Process -FilePath binarycreator.exe -ArgumentList ( "-n -c config/config.xml -p packages .\FolderCustomizerSetup-x64.exe") -NoNewWindow -Wait

# Build the Windows installer with Inno Setup
Start-Process "ISCC.exe" -ArgumentList "./installer.iss" -NoNewWindow -Wait

# Optionally print the produced installer path for logs
Get-ChildItem -Path "./windows-installer" -Filter "*.exe" | ForEach-Object { Write-Host "Built installer: $($_.FullName)" }
