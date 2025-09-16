param(
    [switch]$SkipDependencies,
    [switch]$SkipBuild,
    [switch]$SkipPack,
    [switch]$SkipInstallQt,
    [switch]$SkipInstallBoost,
    [string]$Configuration = "Release",
    [string]$QtPath,
    [string]$BoostPath,
    [string]$CMakeGenerator = "Ninja"
)

$ErrorActionPreference = "Stop"
$scriptStartTime = Get-Date
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$projectRoot = (Get-Item $scriptDir).Parent.FullName

function Show-Banner {
    Write-Host "========================================="
    Write-Host "=== Download Sorter Deployment Tool ==="
    Write-Host "========================================="
    Write-Host ""
    Write-Host "Configuration:"
    Write-Host "  Skip Dependencies: $SkipDependencies"
    Write-Host "  Skip Build: $SkipBuild"
    Write-Host "  Skip Pack: $SkipPack"
    Write-Host "  Skip Install Qt: $SkipInstallQt"
    Write-Host "  Skip Install Boost: $SkipInstallBoost"
    Write-Host "  Build Configuration: $Configuration"
    Write-Host "  Qt Path: $QtPath"
    Write-Host "  Boost Path: $BoostPath"
    Write-Host "  CMake Generator: $CMakeGenerator"
    Write-Host ""
}

# Install Qt and Boost DLLs if needed
function Install-QtAndBoost {
    if ($SkipInstallQt -and $SkipInstallBoost) {
        Write-Host "⏩ Skipping Qt and Boost installation as requested." -ForegroundColor Yellow
        return
    }
    
    Write-Host "Checking for Qt and Boost DLLs..."

    # Install Qt
    if (-not $SkipInstallQt) {
        if ([string]::IsNullOrEmpty($QtPath)) {
            Write-Host "Installing Qt (requires Chocolatey)..."
            if (-not (Get-Command choco -ErrorAction SilentlyContinue)) {
                Write-Host "Chocolatey not found. Installing Chocolatey..." -ForegroundColor Yellow
                Set-ExecutionPolicy Bypass -Scope Process -Force
                [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072
                Invoke-Expression ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))
            }
            choco install qt5 -y
        } else {
            Write-Host "Using Qt from specified path: $QtPath"
        }
    } else {
        Write-Host "⏩ Skipping Qt installation as requested." -ForegroundColor Yellow
    }

    # Install Boost
    if (-not $SkipInstallBoost) {
        if ([string]::IsNullOrEmpty($BoostPath)) {
            Write-Host "Installing Boost (requires Chocolatey)..."
            if (-not (Get-Command choco -ErrorAction SilentlyContinue)) {
                Write-Host "Chocolatey not found. Installing Chocolatey..." -ForegroundColor Yellow
                Set-ExecutionPolicy Bypass -Scope Process -Force
                [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072
                Invoke-Expression ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))
            }
            choco install boost-msvc -y
        } else {
            Write-Host "Using Boost from specified path: $BoostPath"
        }
    } else {
        Write-Host "⏩ Skipping Boost installation as requested." -ForegroundColor Yellow
    }

    Write-Host "✅ Qt and Boost installation completed." -ForegroundColor Green
}

function Build-Project {
    if ($SkipBuild) {
        Write-Host "⏩ Skipping build as requested." -ForegroundColor Yellow
        return
    }
    
    Write-Host "Building project with configuration: $Configuration"
    try {
        # Create build directory if it doesn't exist
        $buildDir = Join-Path $projectRoot "build-windows"
        if (-not (Test-Path $buildDir)) {
            New-Item -ItemType Directory -Path $buildDir -Force | Out-Null
        }
        
        Push-Location $buildDir
        
        # Configure with CMake
        $cmakeArgs = "-DCMAKE_BUILD_TYPE=$Configuration"
        
        # Add Qt path if specified
        if (-not [string]::IsNullOrEmpty($QtPath)) {
            $cmakeArgs += " -DCMAKE_PREFIX_PATH=`"$QtPath`""
        }
        
        # Add Boost path if specified
        if (-not [string]::IsNullOrEmpty($BoostPath)) {
            $cmakeArgs += " -DBOOST_ROOT=`"$BoostPath`""
        }
        
        # Run CMake
        Write-Host "Configuring with CMake..."
        $cmakeCmd = "cmake -S ../src -G `"$CMakeGenerator`" $cmakeArgs"
        Write-Host "Running: $cmakeCmd"
        Invoke-Expression $cmakeCmd
        
        if ($LASTEXITCODE -ne 0) {
            throw "CMake configuration failed with exit code $LASTEXITCODE"
        }
        
        # Build and Install using install_local target
        Write-Host "Building and installing with CMake..."
        cmake --build . --target install_local
        
        if ($LASTEXITCODE -ne 0) {
            throw "Build and install failed with exit code $LASTEXITCODE"
        }
        
        Pop-Location
        Write-Host "✅ Build completed successfully" -ForegroundColor Green
    }
    catch {
        Write-Host "❌ Build failed: $_" -ForegroundColor Red
        if (Test-Path "Variable:buildDir") {
            Pop-Location
        }
        exit 1
    }
}

function Install-Project {
    if ($SkipBuild) {
        Write-Host "⏩ Skipping installation as build was skipped." -ForegroundColor Yellow
        return
    }
    
    Write-Host "Installing project..."
    try {
        Push-Location (Join-Path $projectRoot "build-windows")
        
        # Run CMake install with absolute path
        $installPath = Join-Path $projectRoot "install"
        Write-Host "Installing with CMake..."
        Write-Host "Install path: $installPath"
        cmake --install . --config $Configuration --prefix $installPath
        
        if ($LASTEXITCODE -ne 0) {
            throw "Installation failed with exit code $LASTEXITCODE"
        }
        
        Pop-Location
        Write-Host "✅ Installation completed successfully" -ForegroundColor Green
    }
    catch {
        Write-Host "❌ Installation failed: $_" -ForegroundColor Red
        if (Get-Location -ErrorAction SilentlyContinue | Where-Object { $_.Path -like "*build-windows" }) {
            Pop-Location
        }
        exit 1
    }
}

function Deploy-QtDependencies {
    if ($SkipBuild) {
        Write-Host "⏩ Skipping Qt deployment as build was skipped." -ForegroundColor Yellow
        return
    }
    
    Write-Host "Deploying Qt dependencies..."
    try {
        $installDir = Join-Path $projectRoot "install"
        $binDir = Join-Path $installDir "bin"
        
        if (-not (Test-Path $binDir)) {
            throw "Binary directory not found: $binDir. Make sure the installation step completed successfully."
        }
        
        # Find windeployqt.exe
        $windeployqt = $null
        if (-not [string]::IsNullOrEmpty($QtPath)) {
            $qtBinDir = Join-Path $QtPath "bin"
            $windeployqt = Join-Path $qtBinDir "windeployqt.exe"
        } else {
            # Try to find windeployqt in PATH
            $windeployqt = Get-Command windeployqt.exe -ErrorAction SilentlyContinue | Select-Object -ExpandProperty Source
        }
        
        if (-not $windeployqt -or -not (Test-Path $windeployqt)) {
            Write-Host "⚠️ windeployqt.exe not found. Qt dependencies may not be properly deployed." -ForegroundColor Yellow
            return
        }
        
        # Find the main executable
        $exeFiles = Get-ChildItem -Path $binDir -Filter "*.exe"
        if ($exeFiles.Count -eq 0) {
            Write-Host "⚠️ No executable files found in $binDir. Skipping Qt deployment." -ForegroundColor Yellow
            return
        }
        
        # Deploy Qt dependencies for each executable
        foreach ($exeFile in $exeFiles) {
            $exePath = $exeFile.FullName
            Write-Host "Running windeployqt for: $exePath"
            & $windeployqt --no-compiler-runtime --no-translations $exePath
            
            if ($LASTEXITCODE -ne 0) {
                throw "windeployqt failed with exit code $LASTEXITCODE for $exePath"
            }
        }
        
        Write-Host "✅ Qt dependencies deployed successfully" -ForegroundColor Green
    }
    catch {
        Write-Host "❌ Qt deployment failed: $_" -ForegroundColor Red
        exit 1
    }
}

function Deploy-BoostDependencies {
    if ($SkipBuild) {
        Write-Host "⏩ Skipping Boost deployment as build was skipped." -ForegroundColor Yellow
        return
    }
    
    Write-Host "Deploying Boost dependencies..."
    try {
        $installDir = Join-Path $projectRoot "install"
        $binDir = Join-Path $installDir "bin"
        
        if (-not (Test-Path $binDir)) {
            throw "Binary directory not found: $binDir. Make sure the installation step completed successfully."
        }
        
        # Determine Boost DLLs to copy
        $boostDllDir = $null
        if (-not [string]::IsNullOrEmpty($BoostPath)) {
            $boostDllDir = Join-Path $BoostPath "lib"
            if (-not (Test-Path $boostDllDir)) {
                $boostDllDir = Join-Path $BoostPath "stage\lib"
                if (-not (Test-Path $boostDllDir)) {
                    Write-Host "⚠️ Boost library directory not found in $BoostPath. Skipping Boost deployment." -ForegroundColor Yellow
                    return
                }
            }
        } else {
            # Try to find Boost in Program Files
            $potentialBoostDirs = @(
                "${env:ProgramFiles}\boost",
                "${env:ProgramFiles(x86)}\boost",
                "${env:ProgramFiles}\boost_*",
                "${env:ProgramFiles(x86)}\boost_*"
            )
            
            foreach ($dir in $potentialBoostDirs) {
                $matchedDirs = Get-Item $dir -ErrorAction SilentlyContinue
                if ($matchedDirs) {
                    foreach ($matchedDir in $matchedDirs) {
                        $potentialLibDir = Join-Path $matchedDir.FullName "lib"
                        if (Test-Path $potentialLibDir) {
                            $boostDllDir = $potentialLibDir
                            break
                        }
                        $potentialLibDir = Join-Path $matchedDir.FullName "stage\lib"
                        if (Test-Path $potentialLibDir) {
                            $boostDllDir = $potentialLibDir
                            break
                        }
                    }
                }
                if ($boostDllDir) {
                    break
                }
            }
            
            if (-not $boostDllDir) {
                Write-Host "⚠️ Boost library directory not found. Skipping Boost deployment." -ForegroundColor Yellow
                return
            }
        }
        
        # Copy required Boost DLLs
        # This is a list of commonly used Boost DLLs - adjust based on your project's needs
        $boostLibs = @(
            "boost_filesystem",
            "boost_system",
            "boost_thread",
            "boost_regex",
            "boost_date_time",
            "boost_chrono",
            "boost_atomic"
        )
        
        $dllsCopied = 0
        foreach ($lib in $boostLibs) {
            $dllPattern = "$lib*.dll"
            $boostDlls = Get-ChildItem -Path $boostDllDir -Filter $dllPattern -ErrorAction SilentlyContinue
            
            foreach ($dll in $boostDlls) {
                Write-Host "Copying: $($dll.Name)"
                Copy-Item -Path $dll.FullName -Destination $binDir -Force
                $dllsCopied++
            }
        }
        
        if ($dllsCopied -eq 0) {
            Write-Host "⚠️ No Boost DLLs were found to copy. Your application may not work correctly." -ForegroundColor Yellow
        } else {
            Write-Host "✅ Copied $dllsCopied Boost DLLs successfully" -ForegroundColor Green
        }
    }
    catch {
        Write-Host "❌ Boost deployment failed: $_" -ForegroundColor Red
        exit 1
    }
}

function Pack-WithInnoSetup {
    if ($SkipPack) {
        Write-Host "⏩ Skipping packaging as requested." -ForegroundColor Yellow
        return
    }
    
    Write-Host "Packing with Inno Setup..."
    try {
        # Check if Inno Setup is installed
        $innoSetupPaths = @(
            "${env:ProgramFiles(x86)}\Inno Setup 6\ISCC.exe",
            "${env:ProgramFiles}\Inno Setup 6\ISCC.exe"
        )
        
        $innoExe = $null
        foreach ($path in $innoSetupPaths) {
            if (Test-Path $path) {
                $innoExe = $path
                break
            }
        }
        
        if (-not $innoExe) {
            if ($SkipDependencies) {
                throw "Inno Setup not found. Please install it or use -SkipDependencies:$false to auto-install it."
            } else {
                Write-Host "Inno Setup not found. Installing it..."
                choco install innosetup -y
                
                # Recheck paths
                foreach ($path in $innoSetupPaths) {
                    if (Test-Path $path) {
                        $innoExe = $path
                        break
                    }
                }
                
                if (-not $innoExe) {
                    throw "Failed to find Inno Setup even after installation attempt."
                }
            }
        }
        
        # Locate the .iss file
        $issFiles = Get-ChildItem -Path $projectRoot -Filter "installer.iss" -Recurse | Select-Object -First 1
        
        if (-not $issFiles) {
            throw "No Inno Setup script (.iss) file found in the project."
        }
        
        $issFile = $issFiles.FullName
        
        # Define output directory for installer - root/release
        $installerOutput = Join-Path $projectRoot "release"
        if (-not (Test-Path $installerOutput)) {
            New-Item -ItemType Directory -Path $installerOutput -Force | Out-Null
        }
        
        # Run Inno Setup compiler
        Write-Host "Compiling installer with Inno Setup: $issFile"
        & $innoExe "/O$installerOutput" "/F$Configuration" "$issFile"
        
        if ($LASTEXITCODE -ne 0) {
            throw "Inno Setup compilation failed with exit code $LASTEXITCODE"
        }
        
        Write-Host "✅ Packaging completed successfully" -ForegroundColor Green
        Write-Host "Installer saved to: $installerOutput" -ForegroundColor Cyan
    }
    catch {
        Write-Host "❌ Packaging failed: $_" -ForegroundColor Red
        exit 1
    }
}

function Install-Dependencies {
    if ($SkipDependencies) {
        Write-Host "⏩ Skipping dependency installation as requested." -ForegroundColor Yellow
        return
    }
    
    Write-Host "Installing dependencies..."
    try {
        # Check if Chocolatey is installed
        if (-not (Get-Command choco -ErrorAction SilentlyContinue)) {
            Write-Host "Chocolatey not found. Installing Chocolatey..." -ForegroundColor Yellow
            Set-ExecutionPolicy Bypass -Scope Process -Force
            [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072
            Invoke-Expression ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))
        }
        
        # Install Inno Setup
        Write-Host "Installing Inno Setup..."
        choco install innosetup -y
        
        # Install CMake if not present
        if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
            Write-Host "Installing CMake..."
            choco install cmake -y
        }
        
        # Install Ninja if CMake generator is Ninja and it's not installed
        if ($CMakeGenerator -eq "Ninja" -and -not (Get-Command ninja -ErrorAction SilentlyContinue)) {
            Write-Host "Installing Ninja build system..."
            choco install ninja -y
        }
        
        Write-Host "✅ Dependencies installed successfully" -ForegroundColor Green
    }
    catch {
        Write-Host "❌ Failed to install dependencies: $_" -ForegroundColor Red
        exit 1
    }
}

function Show-Summary {
    $scriptEndTime = Get-Date
    $duration = $scriptEndTime - $scriptStartTime

    Write-Host ""
    Write-Host "=================== SUMMARY ==================="
    Write-Host "Build:              $(if ($SkipBuild) { '⏩ Skipped' } else { '✅ Completed' })"
    Write-Host "Qt & Boost Install: $(if ($SkipInstallQt -and $SkipInstallBoost) { '⏩ Skipped' } else { '✅ Completed' })"
    Write-Host "Project Install:    $(if ($SkipBuild) { '⏩ Skipped' } else { '✅ Completed' })"
    Write-Host "Qt Deploy:          $(if ($SkipBuild) { '⏩ Skipped' } else { '✅ Completed' })"
    Write-Host "Boost Deploy:       $(if ($SkipBuild) { '⏩ Skipped' } else { '✅ Completed' })"
    Write-Host "Packaging:          $(if ($SkipPack) { '⏩ Skipped' } else { '✅ Completed' })"
    Write-Host "Total time:         $($duration.Minutes)m $($duration.Seconds)s"
    Write-Host "================================================"

    if (-not $SkipPack) {
        $installerPath = Join-Path $projectRoot "release"
        Write-Host "Installer is ready at: $installerPath" -ForegroundColor Green
    }
}

# Main execution
try {
    Show-Banner
    
    if (-not $SkipDependencies) {

        # Step 1: Install Qt and Boost DLLs (optional)
        Install-QtAndBoost
        
        # Step 2: Install other dependencies
        Install-Dependencies
    }
    
    # Step 3: Build and install the project using install_local target
    Build-Project
    
    # Step 4: Install-Project - Skipped (handled by install_local target)
    # Install-Project
    
    # Step 5: Deploy Qt dependencies - Skipped (handled by install_local target)
    # Deploy-QtDependencies
    
    # Step 6: Deploy Boost DLLs - Skipped (handled by install_local target)
    # Deploy-BoostDependencies
    
    # Step 7: Package the application with Inno Setup
    Pack-WithInnoSetup
    
    # Show summary
    Show-Summary

    Write-Host "✅ Deployment completed successfully" -ForegroundColor Green
    exit 0
}
catch {
    Write-Host "❌ Deployment failed: $_" -ForegroundColor Red
    exit 1
}
