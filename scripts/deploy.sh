#!/bin/bash
# Cross-platform deployment script for Download Sorter
# Usage: ./deploy.sh [all|windows|linux|appimage|deb|rpm|arch]

set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="$PROJECT_ROOT/build"
INSTALL_DIR="$PROJECT_ROOT/install"
SCRIPTS_DIR="$PROJECT_ROOT/scripts"
# Read version from manifest.json
VERSION=$(grep -o '"version"[^"]*"[0-9.]*"' "$PROJECT_ROOT/manifest.json" | sed 's/.*"\([0-9.]*\)"/\1/')

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
    if [ -f "$INSTALL_DIR/DownloadSorter" ]; then
        BIN_NAME="DownloadSorter"
    elif [ -f "$INSTALL_DIR/Download Sorter" ]; then
        BIN_NAME="Download Sorter"
    else
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
        cp "$INSTALL_DIR/$BIN_NAME" "$appdir/usr/bin/DownloadSorter"
    # Copy updater if built
    if [ -f "$INSTALL_DIR/Updater" ]; then
        cp "$INSTALL_DIR/Updater" "$appdir/usr/bin/"
    fi
    
    # Copy libraries
    cp -r "$INSTALL_DIR"/*.so* "$appdir/usr/lib/" 2>/dev/null || true
    
    # Create desktop file (no leading spaces per spec)
    cat > "$appdir/DownloadSorter.desktop" << EOF
[Desktop Entry]
Type=Application
Name=Download Sorter
Comment=Organize your downloads automatically
Exec=DownloadSorter
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
exec "${HERE}/usr/bin/DownloadSorter" "$@"
EOF
    chmod +x "$appdir/AppRun"
    
    # Download appimagetool (as AppImage)
    if [ ! -f "$SCRIPTS_DIR/appimagetool" ]; then
        log_info "Downloading appimagetool..."
        mkdir -p "$SCRIPTS_DIR"
        wget -O "$SCRIPTS_DIR/appimagetool" "https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage" || true
        chmod +x "$SCRIPTS_DIR/appimagetool" 2>/dev/null || true
    fi

    mkdir -p "$PROJECT_ROOT/dist"
    cd "$PROJECT_ROOT/dist"

    # Try to run appimagetool normally (requires FUSE)
    if "$SCRIPTS_DIR/appimagetool" "$appdir" "DownloadSorter-${VERSION}-x86_64.AppImage" 2>appimage.err; then
        log_success "AppImage created: dist/DownloadSorter-${VERSION}-x86_64.AppImage"
        return
    fi

    # If FUSE is missing, extract and run embedded appimagetool without FUSE
    if grep -qi "libfuse" appimage.err; then
        log_warning "FUSE not available, trying extracted appimagetool"
        (
            cd "$SCRIPTS_DIR" && \
            ./appimagetool --appimage-extract >/dev/null 2>&1 && \
            chmod +x "$SCRIPTS_DIR/squashfs-root/AppRun" && \
            "$SCRIPTS_DIR/squashfs-root/AppRun" "$appdir" "$PROJECT_ROOT/dist/DownloadSorter-${VERSION}-x86_64.AppImage"
        ) && {
            log_success "AppImage created (no FUSE): dist/DownloadSorter-${VERSION}-x86_64.AppImage"
            return
        }
    fi

    tar czf "DownloadSorter-${VERSION}-x86_64.tar.gz" -C "$(dirname "$appdir")" "$(basename "$appdir")"
    log_success "Archive created: dist/DownloadSorter-${VERSION}-x86_64.tar.gz"
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
        cp "$INSTALL_DIR/$BIN_NAME" "$debdir/usr/lib/download-sorter/DownloadSorter"
    # Copy updater if built
    if [ -f "$INSTALL_DIR/Updater" ]; then
        cp "$INSTALL_DIR/Updater" "$debdir/usr/lib/download-sorter/"
    fi
    cp -r "$INSTALL_DIR"/*.so* "$debdir/usr/lib/download-sorter/" 2>/dev/null || true
    
    # Create wrapper script
    cat > "$debdir/usr/bin/download-sorter" << EOF
#!/bin/bash
export LD_LIBRARY_PATH="/usr/lib/download-sorter:\$LD_LIBRARY_PATH"
exec "/usr/lib/download-sorter/DownloadSorter" "\$@"
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
Depends: libc6, libqt6core6, libqt6gui6, libqt6widgets6, libqt6network6
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

    # Force tarball on non-RPM distros or when requested
    if [ "${FORCE_RPM_TARBALL:-}" = "1" ]; then
        log_warning "FORCE_RPM_TARBALL=1 set; creating tar.gz instead of RPM"
        cd "$PROJECT_ROOT/dist"
        tar czf "download-sorter-${VERSION}-1.x86_64.tar.gz" -C "$INSTALL_DIR" .
        log_success "Archive created: dist/download-sorter-${VERSION}-1.x86_64.tar.gz"
        return
    fi

    # If rpmbuild not available OR distro is not Fedora/RHEL/SUSE, fallback
    if ! command -v rpmbuild &> /dev/null; then
        log_warning "rpmbuild not found, creating tar.gz instead"
        cd "$PROJECT_ROOT/dist"
        tar czf "download-sorter-${VERSION}-1.x86_64.tar.gz" -C "$INSTALL_DIR" .
        log_success "Archive created: dist/download-sorter-${VERSION}-1.x86_64.tar.gz"
        return
    fi

    if [ -r /etc/os-release ]; then
        . /etc/os-release
        case "${ID_LIKE}${ID}" in
            *fedora*|*rhel*|*centos*|*suse*) : ;;
            *)
                log_warning "Non-RPM-based distro detected (${ID:-unknown}); creating tar.gz instead"
                cd "$PROJECT_ROOT/dist"
                tar czf "download-sorter-${VERSION}-1.x86_64.tar.gz" -C "$INSTALL_DIR" .
                log_success "Archive created: dist/download-sorter-${VERSION}-1.x86_64.tar.gz"
                return
                ;;
        esac
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
export LD_LIBRARY_PATH="/usr/lib/download-sorter:$LD_LIBRARY_PATH"
exec "/usr/lib/download-sorter/DownloadSorter" "$@"
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
    if ! rpmbuild --define "_topdir $rpmdir" -bb "$rpmdir/SPECS/download-sorter.spec"; then
        log_warning "RPM build failed, creating tar.gz"
        cd "$PROJECT_ROOT/dist"
        tar czf "download-sorter-${VERSION}-1.x86_64.tar.gz" -C "$INSTALL_DIR" .
        log_success "Archive created: dist/download-sorter-${VERSION}-1.x86_64.tar.gz"
        return
    fi
    
    # Copy result
    cp "$rpmdir/RPMS/x86_64/download-sorter-${VERSION}-1."*.rpm "$PROJECT_ROOT/dist/" 2>/dev/null || true
    log_success "RPM package created in dist/"
}

# Create Arch Linux package (for Manjaro)
create_arch() {
    log_info "Creating Arch Linux package..."
    
    local archdir="$PROJECT_ROOT/dist/arch"
    rm -rf "$archdir"
    mkdir -p "$archdir"
    
    # Always create tar.gz instead of pkg.tar.gz for better compatibility
    log_info "Creating tar.gz for Arch (better compatibility than pkg.tar.gz)"
    cd "$PROJECT_ROOT/dist"
    tar czf "download-sorter-${VERSION}-1-x86_64.tar.gz" -C "$INSTALL_DIR" .
    log_success "Archive created: dist/download-sorter-${VERSION}-1-x86_64.tar.gz"
    return

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
