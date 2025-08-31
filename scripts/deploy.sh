#!/bin/bash
# Cross-platform deployment script for Download Sorter
# Usage: ./deploy.sh [all|windows|linux|appimage|deb|rpm|arch]

set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="$PROJECT_ROOT/build"
INSTALL_DIR="$PROJECT_ROOT/install"
SCRIPTS_DIR="$PROJECT_ROOT/scripts"
VERSION=$(grep "VERSION" "$PROJECT_ROOT/src/CMakeLists.txt" | head -1 | sed 's/.*VERSION \([0-9.]*\).*/\1/')

echo "=== Download Sorter Deployment Script ==="
echo "Version: $VERSION"
echo "Project root: $PROJECT_ROOT"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if build exists
check_build() {
    if [ ! -f "$INSTALL_DIR/Download Sorter" ]; then
        log_error "Build not found. Please build the project first:"
        echo "  cd $PROJECT_ROOT/src"
        echo "  cmake -B build -DCMAKE_BUILD_TYPE=Release"
        echo "  cmake --build build"
        echo "  cmake --build build --target install_local"
        exit 1
    fi
}

# Create AppImage (Universal Linux)
create_appimage() {
    log_info "Creating AppImage..."
    
    local appdir="$PROJECT_ROOT/dist/DownloadSorter.AppDir"
    rm -rf "$appdir"
    mkdir -p "$appdir/usr/bin"
    mkdir -p "$appdir/usr/lib"
    
    # Copy application
    cp "$INSTALL_DIR/Download Sorter" "$appdir/usr/bin/"
    
    # Copy libraries
    cp -r "$INSTALL_DIR"/*.so* "$appdir/usr/lib/" 2>/dev/null || true
    
    # Create desktop file
    cat > "$appdir/DownloadSorter.desktop" << EOF
[Desktop Entry]
Type=Application
Name=Download Sorter
Comment=Organize your downloads automatically
Exec=Download Sorter
Icon=download-sorter
Categories=Utility;FileManager;
EOF
    
    # Copy icon (you'll need to have this)
    if [ -f "$PROJECT_ROOT/src/icons/Download Sorter.png" ]; then
        cp "$PROJECT_ROOT/src/icons/Download Sorter.png" "$appdir/download-sorter.png"
    else
        log_warning "Icon not found, AppImage will have default icon"
        # Create a simple placeholder icon
        echo "iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mNkYPhfDwAChwGA60e6kgAAAABJRU5ErkJggg==" | base64 -d > "$appdir/download-sorter.png" 2>/dev/null || true
    fi
    
    # Create AppRun script
    cat > "$appdir/AppRun" << 'EOF'
#!/bin/bash
HERE="$(dirname "$(readlink -f "${0}")")"
export LD_LIBRARY_PATH="${HERE}/usr/lib:${LD_LIBRARY_PATH}"
exec "${HERE}/usr/bin/Download Sorter" "$@"
EOF
    chmod +x "$appdir/AppRun"
    
    # Download and use appimagetool
    if [ ! -f "$SCRIPTS_DIR/appimagetool" ]; then
        log_info "Downloading appimagetool..."
        mkdir -p "$SCRIPTS_DIR"
        wget -O "$SCRIPTS_DIR/appimagetool" "https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage" || {
            log_error "Failed to download appimagetool. Creating simple tar.gz instead."
            cd "$PROJECT_ROOT/dist"
            tar czf "DownloadSorter-${VERSION}-x86_64.tar.gz" -C "$appdir" .
            log_success "Archive created: dist/DownloadSorter-${VERSION}-x86_64.tar.gz"
            return
        }
        chmod +x "$SCRIPTS_DIR/appimagetool"
    fi
    
    cd "$PROJECT_ROOT/dist"
    "$SCRIPTS_DIR/appimagetool" "$appdir" "DownloadSorter-${VERSION}-x86_64.AppImage" || {
        log_warning "AppImage creation failed, creating tar.gz instead"
        tar czf "DownloadSorter-${VERSION}-x86_64.tar.gz" -C "$appdir" .
        log_success "Archive created: dist/DownloadSorter-${VERSION}-x86_64.tar.gz"
        return
    }
    
    log_success "AppImage created: dist/DownloadSorter-${VERSION}-x86_64.AppImage"
}

# Create DEB package (Ubuntu/Debian)
create_deb() {
    log_info "Creating DEB package..."
    
    local debdir="$PROJECT_ROOT/dist/deb"
    rm -rf "$debdir"
    mkdir -p "$debdir/DEBIAN"
    mkdir -p "$debdir/usr/bin"
    mkdir -p "$debdir/usr/lib/download-sorter"
    mkdir -p "$debdir/usr/share/applications"
    mkdir -p "$debdir/usr/share/icons/hicolor/256x256/apps"
    
    # Copy application and libraries
    cp "$INSTALL_DIR/Download Sorter" "$debdir/usr/lib/download-sorter/"
    cp -r "$INSTALL_DIR"/*.so* "$debdir/usr/lib/download-sorter/" 2>/dev/null || true
    
    # Create wrapper script
    cat > "$debdir/usr/bin/download-sorter" << EOF
#!/bin/bash
export LD_LIBRARY_PATH="/usr/lib/download-sorter:\$LD_LIBRARY_PATH"
exec "/usr/lib/download-sorter/Download Sorter" "\$@"
EOF
    chmod +x "$debdir/usr/bin/download-sorter"
    
    # Create desktop file
    cat > "$debdir/usr/share/applications/download-sorter.desktop" << EOF
[Desktop Entry]
Name=Download Sorter
Comment=Organize your downloads automatically
Exec=download-sorter
Icon=download-sorter
Type=Application
Categories=Utility;FileManager;
EOF
    
    # Copy icon if available
    if [ -f "$PROJECT_ROOT/src/icons/Download Sorter.png" ]; then
        cp "$PROJECT_ROOT/src/icons/Download Sorter.png" "$debdir/usr/share/icons/hicolor/256x256/apps/download-sorter.png"
    fi
    
    # Create control file
    cat > "$debdir/DEBIAN/control" << EOF
Package: download-sorter
Version: $VERSION
Section: utils
Priority: optional
Architecture: amd64
Maintainer: Deadbush225 <your-email@example.com>
Description: Download Sorter - Organize your downloads automatically
 A Qt-based application that helps you automatically organize
 your downloaded files into appropriate folders.
Depends: libc6, libqt6core6, libqt6gui6, libqt6widgets6
EOF
    
    # Build package
    if command -v dpkg-deb &> /dev/null; then
        dpkg-deb --build "$debdir" "$PROJECT_ROOT/dist/download-sorter_${VERSION}_amd64.deb"
        log_success "DEB package created: dist/download-sorter_${VERSION}_amd64.deb"
    else
        log_warning "dpkg-deb not found, creating tar.gz instead"
        cd "$PROJECT_ROOT/dist"
        tar czf "download-sorter_${VERSION}_amd64.tar.gz" -C "$debdir" .
        log_success "Archive created: dist/download-sorter_${VERSION}_amd64.tar.gz"
    fi
}

# Create RPM package (Fedora/RHEL)
create_rpm() {
    log_info "Creating RPM package..."
    
    if ! command -v rpmbuild &> /dev/null; then
        log_warning "rpmbuild not found, creating tar.gz instead"
        local rpmdir="$PROJECT_ROOT/dist/rpm-content"
        rm -rf "$rpmdir"
        mkdir -p "$rpmdir/usr/bin"
        mkdir -p "$rpmdir/usr/lib/download-sorter"
        
        cp "$INSTALL_DIR/Download Sorter" "$rpmdir/usr/lib/download-sorter/"
        cp -r "$INSTALL_DIR"/*.so* "$rpmdir/usr/lib/download-sorter/" 2>/dev/null || true
        
        cat > "$rpmdir/usr/bin/download-sorter" << 'EOF'
#!/bin/bash
export LD_LIBRARY_PATH="/usr/lib/download-sorter:$LD_LIBRARY_PATH"
exec "/usr/lib/download-sorter/Download Sorter" "$@"
EOF
        chmod +x "$rpmdir/usr/bin/download-sorter"
        
        cd "$PROJECT_ROOT/dist"
        tar czf "download-sorter-${VERSION}-1.x86_64.tar.gz" -C "$rpmdir" .
        log_success "Archive created: dist/download-sorter-${VERSION}-1.x86_64.tar.gz"
        return
    fi
    
    local rpmdir="$PROJECT_ROOT/dist/rpm"
    rm -rf "$rpmdir"
    mkdir -p "$rpmdir"/{BUILD,BUILDROOT,RPMS,SOURCES,SPECS,SRPMS}
    
    # Create spec file
    cat > "$rpmdir/SPECS/download-sorter.spec" << EOF
Name:           download-sorter
Version:        $VERSION
Release:        1%{?dist}
Summary:        Organize your downloads automatically
License:        MIT
URL:            https://github.com/Deadbush225/DownloadSorter
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  qt6-qtbase-devel
Requires:       qt6-qtbase qt6-qtbase-gui

%description
A Qt-based application that helps you automatically organize
your downloaded files into appropriate folders.

%prep
%setup -q

%build
# Files are pre-built

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/usr/bin
mkdir -p %{buildroot}/usr/lib/%{name}
mkdir -p %{buildroot}/usr/share/applications

# Install files
cp -r * %{buildroot}/usr/lib/%{name}/

# Create wrapper script
cat > %{buildroot}/usr/bin/%{name} << 'EOFSCRIPT'
#!/bin/bash
export LD_LIBRARY_PATH="/usr/lib/download-sorter:\$LD_LIBRARY_PATH"
exec "/usr/lib/download-sorter/Download Sorter" "\$@"
EOFSCRIPT
chmod +x %{buildroot}/usr/bin/%{name}

%files
/usr/bin/%{name}
/usr/lib/%{name}/
%if 0%{?fedora} || 0%{?rhel} >= 8
/usr/share/applications/%{name}.desktop
%endif

%changelog
* $(date +'%a %b %d %Y') Deadbush225 <your-email@example.com> - $VERSION-1
- Initial RPM package
EOF
    
    # Create source tarball
    cd "$INSTALL_DIR"
    tar czf "$rpmdir/SOURCES/download-sorter-${VERSION}.tar.gz" *
    
    # Build RPM
    rpmbuild --define "_topdir $rpmdir" -bb "$rpmdir/SPECS/download-sorter.spec"
    
    # Copy result
    cp "$rpmdir/RPMS/x86_64/download-sorter-${VERSION}-1."*.rpm "$PROJECT_ROOT/dist/" 2>/dev/null || {
        log_warning "RPM build failed, creating tar.gz"
        cd "$PROJECT_ROOT/dist"
        tar czf "download-sorter-${VERSION}-1.x86_64.tar.gz" -C "$INSTALL_DIR" .
    }
    
    log_success "RPM package created in dist/"
}

# Create Arch Linux package (for Manjaro)
create_arch() {
    log_info "Creating Arch Linux package..."
    
    local archdir="$PROJECT_ROOT/dist/arch"
    rm -rf "$archdir"
    mkdir -p "$archdir"
    
    if ! command -v makepkg &> /dev/null; then
        log_warning "makepkg not found, creating tar.gz instead"
        cd "$PROJECT_ROOT/dist"
        tar czf "download-sorter-${VERSION}-1-x86_64.pkg.tar.gz" -C "$INSTALL_DIR" .
        log_success "Archive created: dist/download-sorter-${VERSION}-1-x86_64.pkg.tar.gz"
        return
    fi
    
    # Create PKGBUILD
    cat > "$archdir/PKGBUILD" << EOF
# Maintainer: Deadbush225 <your-email@example.com>
pkgname=download-sorter
pkgver=$VERSION
pkgrel=1
pkgdesc="Organize your downloads automatically"
arch=('x86_64')
url="https://github.com/Deadbush225/DownloadSorter"
license=('MIT')
depends=('qt6-base')
source=()
md5sums=()

package() {
    # Install binary and libraries
    install -dm755 "\$pkgdir/usr/lib/\$pkgname"
    cp -r "$INSTALL_DIR"/* "\$pkgdir/usr/lib/\$pkgname/"
    
    # Create wrapper script
    install -dm755 "\$pkgdir/usr/bin"
    cat > "\$pkgdir/usr/bin/\$pkgname" << 'EOFSCRIPT'
#!/bin/bash
export LD_LIBRARY_PATH="/usr/lib/download-sorter:\$LD_LIBRARY_PATH"
exec "/usr/lib/download-sorter/Download Sorter" "\$@"
EOFSCRIPT
    chmod +x "\$pkgdir/usr/bin/\$pkgname"
    
    # Desktop file
    install -dm755 "\$pkgdir/usr/share/applications"
    cat > "\$pkgdir/usr/share/applications/\$pkgname.desktop" << 'EOFDESKTOP'
[Desktop Entry]
Name=Download Sorter
Comment=Organize your downloads automatically
Exec=download-sorter
Icon=download-sorter
Type=Application
Categories=Utility;FileManager;
EOFDESKTOP
}
EOF
    
    cd "$archdir"
    makepkg -f || {
        log_warning "makepkg failed, creating tar.gz"
        cd "$PROJECT_ROOT/dist"
        tar czf "download-sorter-${VERSION}-1-x86_64.pkg.tar.gz" -C "$INSTALL_DIR" .
        log_success "Archive created: dist/download-sorter-${VERSION}-1-x86_64.pkg.tar.gz"
        return
    }
    
    # Copy result
    cp *.pkg.tar.* "$PROJECT_ROOT/dist/" 2>/dev/null || true
    
    log_success "Arch package created in dist/"
}

# Create all Linux packages
create_linux() {
    log_info "Creating all Linux packages..."
    mkdir -p "$PROJECT_ROOT/dist"
    
    create_appimage
    create_deb
    create_rpm
    create_arch
}

# Create Windows installer
create_windows() {
    log_info "Creating Windows installer..."
    
    if command -v powershell &> /dev/null; then
        cd "$PROJECT_ROOT"
        powershell -ExecutionPolicy Bypass -File "./scripts/update_installer.ps1"
        log_success "Windows installer created"
    else
        log_warning "PowerShell not found. Run update_installer.ps1 on Windows."
    fi
}

# Main deployment function
deploy_all() {
    log_info "Creating all deployment packages..."
    create_linux
    create_windows
    log_success "All packages created in dist/ directory"
}

# Main script logic
case "${1:-all}" in
    "all")
        check_build
        deploy_all
        ;;
    "windows")
        create_windows
        ;;
    "linux")
        check_build
        create_linux
        ;;
    "appimage")
        check_build
        create_appimage
        ;;
    "deb")
        check_build
        create_deb
        ;;
    "rpm")
        check_build
        create_rpm
        ;;
    "arch")
        check_build
        create_arch
        ;;
    *)
        echo "Usage: $0 [all|windows|linux|appimage|deb|rpm|arch]"
        exit 1
        ;;
esac

log_success "Deployment completed!"
