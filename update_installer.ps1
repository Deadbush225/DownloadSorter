Start-Process -FilePath binarycreator.exe -ArgumentList ( "-n -c config/config.xml -p packages .\DownloadSorterSetup-x64.exe") -Wait

Get-FileHash "./DownloadSorterSetup-x64.exe" | Format-List
