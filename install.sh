#!/bin/bash
# filepath: /media/deadbush225/LocalDisk/System/Coding/Projects/download-sorter/install.sh

# Download Sorter Linux Installation Script
# This script installs Download Sorter system-wide on Linux

set -e

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

# Prompt helper (yes/no)
confirm() {
    read -r -p "${1:-Are you sure?} [y/N] " response
    case "$response" in
        [yY][eE][sS]|[yY]) return 0 ;;
        *) return 1 ;;
    esac
}

# Check if running as root
if [ "$EUID" -eq 0 ]; then
    INSTALL_PREFIX="/usr"
    INSTALL_USER="system-wide"
else
    INSTALL_PREFIX="$HOME/.local"
    INSTALL_USER="user-specific"
fi

log_info "Download Sorter Installation Script"
log_info "Installing for: $INSTALL_USER"
log_info "Install prefix: $INSTALL_PREFIX"

# Parse args
ACTION="install"
AUTO_YES=0
for arg in "$@"; do
    case "$arg" in
        uninstall|--uninstall)
            ACTION="uninstall" ;;
        -y|--yes)
            AUTO_YES=1 ;;
        -h|--help)
            echo "Usage: $0 [--uninstall] [-y]";
            echo "  (no args)   Install Download Sorter";
            echo "  --uninstall Uninstall Download Sorter";
            echo "  -y, --yes   Skip confirmation prompts";
            exit 0 ;;
    esac
done

# Uninstall routine
do_uninstall() {
    log_info "Uninstalling Download Sorter from $INSTALL_PREFIX ..."

    if [ "$AUTO_YES" -ne 1 ]; then
        if ! confirm "Remove Download Sorter from $INSTALL_PREFIX?"; then
            log_warning "Uninstall cancelled"
            exit 0
        fi
    fi

    rm -f "$INSTALL_PREFIX/bin/download-sorter" || true
    rm -rf "$INSTALL_PREFIX/lib/download-sorter" || true
    rm -f "$INSTALL_PREFIX/share/applications/download-sorter.desktop" || true
    rm -f "$INSTALL_PREFIX/share/icons/hicolor/256x256/apps/download-sorter.png" || true
    
    # Also remove system-wide eUpdater if it was installed by this script
    rm -f "$INSTALL_PREFIX/bin/eUpdater" || true

    if command -v update-desktop-database &> /dev/null; then
        log_info "Updating desktop database..."
        update-desktop-database "$INSTALL_PREFIX/share/applications" 2>/dev/null || true
    fi

    if command -v gtk-update-icon-cache &> /dev/null; then
        log_info "Updating icon cache..."
        gtk-update-icon-cache -t "$INSTALL_PREFIX/share/icons/hicolor" 2>/dev/null || true
    fi

    log_success "Download Sorter uninstalled from $INSTALL_PREFIX"
    exit 0
}

if [ "$ACTION" = "uninstall" ]; then
    do_uninstall
fi

do_install() {
    # Locate package/build directory relative to this script
    SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    INSTALL_DIR="$SCRIPT_DIR/install"

    # Support repo root (with install/) or packaged release in current dir
    if [ -f "$SCRIPT_DIR/install/DownloadSorter" ] || [ -f "$SCRIPT_DIR/install/Download\ Sorter" ]; then
        INSTALL_DIR="$SCRIPT_DIR/install"
    elif [ -f "$SCRIPT_DIR/DownloadSorter" ] || [ -f "$SCRIPT_DIR/Download\ Sorter" ]; then
        INSTALL_DIR="$SCRIPT_DIR"
    else
        INSTALL_DIR="$SCRIPT_DIR"
    fi

    # Resolve binary name in package/build
    if [ -f "$INSTALL_DIR/DownloadSorter" ]; then
        BIN_SRC="DownloadSorter"
    elif [ -f "$INSTALL_DIR/Download Sorter" ]; then
        BIN_SRC="Download Sorter"
    else
        log_error "Package not found."
        echo "Run this script from the extracted package directory (containing 'DownloadSorter' and 'manifest.json'),"
        echo "or build the project and run it from the repo root after 'install_local' to use ./install/."
        exit 1
    fi

    # Read version from manifest.json if available
    if [ -f "$INSTALL_DIR/manifest.json" ]; then
        VERSION=$(grep -o '"version"[[:space:]]*:[[:space:]]*"[^"]*"' "$INSTALL_DIR/manifest.json" | sed 's/.*: *"\([^"]*\)".*/\1/')
        log_info "Installing version $VERSION"
    else
        log_info "Installing version (unknown)"
    fi

    # Create directories
    log_info "Creating installation directories..."
    mkdir -p "$INSTALL_PREFIX/bin"
    mkdir -p "$INSTALL_PREFIX/lib/download-sorter"
    mkdir -p "$INSTALL_PREFIX/share/applications"
    mkdir -p "$INSTALL_PREFIX/share/icons/hicolor/256x256/apps"

    # Install application and libraries
    log_info "Installing Download Sorter..."
    cp "$INSTALL_DIR/$BIN_SRC" "$INSTALL_PREFIX/lib/download-sorter/"
    cp "$INSTALL_DIR/manifest.json" "$INSTALL_PREFIX/lib/download-sorter/"

    # Install updater if available
    if [ -f "$INSTALL_DIR/eUpdater" ]; then
        log_info "Installing eUpdater..."
        cp "$INSTALL_DIR/eUpdater" "$INSTALL_PREFIX/lib/download-sorter/"
        # Also install eUpdater system-wide so other applications can use it
        cp "$INSTALL_DIR/eUpdater" "$INSTALL_PREFIX/bin/"
        chmod +x "$INSTALL_PREFIX/bin/eUpdater"
        log_info "eUpdater installed system-wide and available for other applications"
    elif [ -f "$INSTALL_DIR/Updater" ]; then
        log_info "Installing Updater (legacy)..."
        cp "$INSTALL_DIR/Updater" "$INSTALL_PREFIX/lib/download-sorter/"
    fi

    # Copy Qt libraries
    if ls "$INSTALL_DIR"/*.so* 1> /dev/null 2>&1; then
        log_info "Installing Qt libraries..."
        cp "$INSTALL_DIR"/*.so* "$INSTALL_PREFIX/lib/download-sorter/" 2>/dev/null || true
    fi

    # Create wrapper script
    log_info "Creating launcher script..."
    cat > "$INSTALL_PREFIX/bin/download-sorter" << EOF
#!/bin/bash
export LD_LIBRARY_PATH="$INSTALL_PREFIX/lib/download-sorter:\$LD_LIBRARY_PATH"
exec "$INSTALL_PREFIX/lib/download-sorter/$BIN_SRC" "\$@"
EOF
    chmod +x "$INSTALL_PREFIX/bin/download-sorter"

    # Create desktop file
    log_info "Creating desktop entry..."
    cat > "$INSTALL_PREFIX/share/applications/download-sorter.desktop" << EOF
[Desktop Entry]
Name=Download Sorter
Comment=Organize your downloads automatically
Exec=download-sorter
Icon=download-sorter
Type=Application
Categories=Utility;FileManager;Qt;
StartupNotify=true
EOF

    # Install icon if available
    if [ -f "$INSTALL_DIR/DownloadSorter.png" ]; then
        log_info "Installing application icon..."
        cp "$INSTALL_DIR/DownloadSorter.png" "$INSTALL_PREFIX/share/icons/hicolor/256x256/apps/download-sorter.png"
    # Fallback: original project icon path inside repo
    elif [ -f "$SCRIPT_DIR/src/icons/Download Sorter.png" ]; then
        log_info "Installing application icon (from src/icons)..."
        cp "$SCRIPT_DIR/src/icons/Download Sorter.png" "$INSTALL_PREFIX/share/icons/hicolor/256x256/apps/download-sorter.png"
    else
        log_warning "No icon found, application will use default icon"
    fi

    # Update desktop database if available
    if command -v update-desktop-database &> /dev/null; then
        log_info "Updating desktop database..."
        update-desktop-database "$INSTALL_PREFIX/share/applications" 2>/dev/null || true
    fi

    # Update icon cache if available
    if command -v gtk-update-icon-cache &> /dev/null; then
        log_info "Updating icon cache..."
        gtk-update-icon-cache -t "$INSTALL_PREFIX/share/icons/hicolor" 2>/dev/null || true
    fi

    log_success "Download Sorter installed successfully!"
    log_info "You can now run 'download-sorter' from the command line"
    log_info "Or find it in your application menu under Utilities"

    # Offer to add to PATH for user installs
    if [ "$EUID" -ne 0 ] && [[ ":$PATH:" != *":$HOME/.local/bin:"* ]]; then
        log_warning "~/.local/bin is not in your PATH"
        if [ -n "$ZSH_VERSION" ]; then
            echo "Add this line to your ~/.zprofile or ~/.zshrc:"
        else
            echo "Add this line to your ~/.bashrc or ~/.profile:"
        fi
        echo "export PATH=\"$HOME/.local/bin:\$PATH\""
    fi

    log_info "Installation complete!"
}

# Run install when requested
if [ "$ACTION" = "install" ]; then
    do_install
fi