# eUpdater

A generic, reusable Qt6-based updater utility that can be used by any Qt application to check for updates and download/install new versions.

## Features

- **CLI-configurable**: No hardcoded URLs - all update sources passed via command-line arguments
- **Flexible update sources**: Supports both JSON manifest files and GitHub Releases API
- **Version comparison**: Uses semantic versioning for reliable update detection
- **Cross-platform**: Works on Windows, macOS, and Linux
- **Dark theme**: Matches modern application aesthetics
- **Minimal dependencies**: Only requires Qt6 Core, Widgets, and Network

## Usage

### Basic Usage

```bash
# Using manifest URL (recommended)
./eUpdater --manifest-url https://example.com/manifest.json \
           --installer-template "https://example.com/releases/%1/installer-%1.exe"

# Using GitHub Releases API
./eUpdater --release-api-url https://api.github.com/repos/owner/repo/releases/latest \
           --installer-template "https://github.com/owner/repo/releases/download/%1/installer-%1.exe"
```

### Command Line Options

- `-m, --manifest-url <url>`: Remote manifest JSON URL containing version info
- `-a, --release-api-url <url>`: GitHub releases API URL for latest release
- `-t, --installer-template <template>`: URL template for installer download (use %1 as version placeholder)
- `-h, --help`: Show help information
- `-v, --version`: Show version information

### Expected Manifest Format

```json
{
	"version": "1.2.3",
	"description": "Latest release with bug fixes"
}
```

## Integration

### From Your Qt Application

````cpp
#include <QtCore/QProcess>

void MyApp::checkForUpdates() {
    QString updaterPath = QCoreApplication::applicationDirPath() + "/eUpdater";
#ifdef Q_OS_WIN
    updaterPath += ".exe";
#endif

    QStringList args;
    args << "--manifest-url" << "https://myapp.com/manifest.json"
         << "--installer-template" << "https://myapp.com/releases/%1/installer-%1.exe";

    QProcess::startDetached(updaterPath, args);
}
```### As CMake Package

```cmake
find_package(eUpdater REQUIRED)
# The eUpdater executable will be available after installation
````

## Building

### Requirements

- Qt6 (Core, Widgets, Network)
- CMake 3.16+
- C++17 compatible compiler

### Build Instructions

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Installation

```bash
sudo make install
# Or for custom prefix:
# cmake -DCMAKE_INSTALL_PREFIX=/usr/local ..
```

## Distribution Strategies

### Option 1: Standalone Repository

- Create separate `qt-generic-updater` repository
- Tag releases for versioning
- Other projects consume as git submodule or prebuilt binary

### Option 2: Git Submodule

```bash
git submodule add https://github.com/user/qt-generic-updater external/updater
```

In your CMakeLists.txt:

```cmake
add_subdirectory(external/updater)
# Binary will be built and available for packaging
```

### Option 3: Package Manager

- Publish to vcpkg, Conan, or similar
- Projects install as dependency

## Maintenance

### Version Updates

- Update version in `CMakeLists.txt`
- Tag release: `git tag v1.0.1`
- Update consuming projects to new version

### Adding Features

- Maintain backward compatibility in CLI interface
- Add new optional arguments without breaking existing usage
- Update documentation

### Security Considerations

- Validate SSL certificates for HTTPS downloads
- Verify installer signatures before execution
- Consider adding checksum verification for downloads

## License

[Specify your license here]
