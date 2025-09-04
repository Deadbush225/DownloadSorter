# Read the version from a JSON file
$jsonFilePath = "./manifest.json"
$jsonContent = Get-Content -Path $jsonFilePath -Raw | ConvertFrom-Json
$currentVersion = $jsonContent.version

# Define a dictionary of files and their version replacement patterns
$versionUpdates = @{
    "./installer.iss"       = @(
        @{ Pattern      = '#define\s+MyAppVersion\s+"[^"]+"';
            Replacement = { "#define MyAppVersion `"$currentVersion`"" } 
        }
    )
    # "./Updater/updater.cpp" = @(
    #     @{ Pattern      = 'appVersion = ".*"';
    #         Replacement = { "appVersion = `"$currentVersion`"" } 
    #     }
    # )
}

# Iterate through each file and apply the replacements
foreach ($filePath in $versionUpdates.Keys) {
    if (Test-Path $filePath) {
        $fileContent = Get-Content -Path $filePath -Raw
        foreach ($rule in $versionUpdates[$filePath]) {
            $fileContent = $fileContent -replace $rule.Pattern, ($rule.Replacement.Invoke())
        }
        Set-Content -Path $filePath -Value $fileContent
    }
}


Write-Host "Version updated to $currentVersion"