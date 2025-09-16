#!/bin/bash
# Post-installation customization script for Download Sorter
# This script is called by generic-desktop-install.sh after the main installation
#
# Arguments:
# $1 - INSTALL_PREFIX (e.g., /usr/local)
# $2 - PACKAGE_ID (e.g., download-sorter)  
# $3 - APP_NAME (e.g., "Download Sorter")

set -e

INSTALL_PREFIX="$1"
PACKAGE_ID="$2"
APP_NAME="$3"

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

log_info() { echo -e "${BLUE}[POST-INSTALL]${NC} $1"; }
log_success() { echo -e "${GREEN}[POST-INSTALL]${NC} $1"; }

# Example: Create default configuration directory
log_info "Creating default configuration directory..."
mkdir -p "$HOME/.config/$PACKAGE_ID"

# You could add other Download Sorter specific tasks here:
# - Create default sorting rules
# - Set up monitoring directories
# - Configure default download locations
# etc.

log_success "Download Sorter post-install customization completed"
