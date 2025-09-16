#!/bin/bash
# Cross-platform deployment script for Download Sorter
# Usage: ./deploy.sh [all|windows|linux|appimage|deb|rpm|arch]
#
# This script now uses the generic deployment system.
# Configuration is loaded from ../deploy.conf

set -e

# Get to project root and source the generic deploy script
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$PROJECT_ROOT"

# Source the generic deployment script
source "./scripts/generic-deploy.sh"

# Run main function with all arguments
main "$@"
