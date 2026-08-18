#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────────────
# build-and-publish.sh — Build Rigset for the current platform, package it,
#                        and push binaries to the site repo.
#
# Usage:
#   ./build-and-publish.sh [version]
#
# If no version is given, reads from CMakeLists.txt.
# ─────────────────────────────────────────────────────────────────────────────

set -euo pipefail

RED='\033[0;31m'; GREEN='\033[0;32m'; YELLOW='\033[1;33m'
CYAN='\033[0;36m'; BOLD='\033[1m'; RESET='\033[0m'

info()    { echo -e "${CYAN}${BOLD}[INFO]${RESET}  $*"; }
success() { echo -e "${GREEN}${BOLD}[ OK ]${RESET}  $*"; }
warn()    { echo -e "${YELLOW}${BOLD}[WARN]${RESET}  $*"; }
die()     { echo -e "${RED}${BOLD}[FAIL]${RESET}  $*" >&2; exit 1; }

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SITE_REPO="/tmp/appleinstaller-site"
SITE_URL="https://flickfpz.github.io/appleinstaller-site"

# ── Detect OS ─────────────────────────────────────────────────────────────────
OS="$(uname -s)"
ARCH="$(uname -m)"

case "$OS" in
    Linux*)  PLATFORM="linux"  ;;
    Darwin*) PLATFORM="macos"  ;;
    MINGW*|MSYS*|CYGWIN*) die "Use install.bat on Windows." ;;
    *)       die "Unsupported OS: $OS" ;;
esac

info "Platform: ${BOLD}${PLATFORM}${RESET} / ${BOLD}${ARCH}${RESET}"

# ── Get version ───────────────────────────────────────────────────────────────
if [[ -n "${1:-}" ]]; then
    VERSION="$1"
else
    VERSION=$(grep -oP 'project\(Rigset VERSION \K[0-9.]+' "$SCRIPT_DIR/CMakeLists.txt" 2>/dev/null || echo "1.0.0")
fi
info "Version: ${BOLD}${VERSION}${RESET}"

# ── Build ──────────────────────────────────────────────────────────────────────
BUILD_DIR="$SCRIPT_DIR/build"
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

info "Configuring with CMake..."
cmake -S "$SCRIPT_DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release -G Ninja 2>&1 | tail -3

info "Building ($(nproc 2>/dev/null || sysctl -n hw.logicalcpu) cores)..."
cmake --build "$BUILD_DIR" --parallel 2>&1 | tail -5

# ── Package ────────────────────────────────────────────────────────────────────
OUTPUT_DIR="$SCRIPT_DIR/dist"
rm -rf "$OUTPUT_DIR"
mkdir -p "$OUTPUT_DIR"

if [[ "$PLATFORM" == "linux" ]]; then
    BINARY=""
    for b in "$BUILD_DIR/Rigset" "$BUILD_DIR/AppleInstaller"; do
        [[ -f "$b" ]] && BINARY="$b" && break
    done
    [[ -n "$BINARY" ]] || die "Linux binary not found in $BUILD_DIR"
    chmod +x "$BINARY"
    cp "$BINARY" "$OUTPUT_DIR/Rigset"
    tar -czf "$OUTPUT_DIR/Rigset-linux-x86_64.tar.gz" -C "$OUTPUT_DIR" Rigset
    ASSET="$OUTPUT_DIR/Rigset-linux-x86_64.tar.gz"
    success "Packaged: Rigset-linux-x86_64.tar.gz"

elif [[ "$PLATFORM" == "macos" ]]; then
    BINARY=""
    for b in "$BUILD_DIR/Rigset.app" "$BUILD_DIR/AppleInstaller.app" "$BUILD_DIR/Rigset" "$BUILD_DIR/AppleInstaller"; do
        [[ -e "$b" ]] && BINARY="$b" && break
    done
    [[ -n "$BINARY" ]] || die "macOS binary not found in $BUILD_DIR"
    if [[ -d "$BINARY" ]]; then
        tar -czf "$OUTPUT_DIR/Rigset-macos-arm64.tar.gz" -C "$BUILD_DIR" "$(basename "$BINARY")"
    else
        cp "$BINARY" "$OUTPUT_DIR/Rigset"
        tar -czf "$OUTPUT_DIR/Rigset-macos-arm64.tar.gz" -C "$OUTPUT_DIR" Rigset
    fi
    ASSET="$OUTPUT_DIR/Rigset-macos-arm64.tar.gz"
    success "Packaged: Rigset-macos-arm64.tar.gz"
fi

# ── Push to site repo ─────────────────────────────────────────────────────────
if [[ ! -d "$SITE_REPO/.git" ]]; then
    info "Cloning site repo..."
    git clone https://github.com/flickfpz/appleinstaller-site.git "$SITE_REPO"
else
    info "Updating site repo..."
    git -C "$SITE_REPO" pull --ff-only --quiet || true
fi

cp "$ASSET" "$SITE_REPO/"

# Update version.json
if command -v python3 &>/dev/null; then
    python3 -c "
import json, sys
with open('$SITE_REPO/version.json') as f:
    d = json.load(f)
d['version'] = '$VERSION'
with open('$SITE_REPO/version.json', 'w') as f:
    json.dump(d, f, indent=2)
print(f'Updated version.json to {\"$VERSION\"}')
"
fi

cd "$SITE_REPO"
git add -A
if git diff --cached --quiet; then
    info "No changes to commit."
else
    git commit -m "Update binaries to v${VERSION} (${PLATFORM})"
    git push origin main
    success "Pushed to site repo!"
fi

# ── Install locally (optional) ────────────────────────────────────────────────
echo ""
read -rp "Install locally now? [Y/n]: " INSTALL_LOCAL
if [[ "${INSTALL_LOCAL,,}" != "n" && "${INSTALL_LOCAL,,}" != "no" ]]; then
    INSTALL_DIR="/usr/local/bin"
    sudo install -Dm755 "$OUTPUT_DIR/Rigset" "$INSTALL_DIR/rigset"
    success "Installed to $INSTALL_DIR/rigset"
fi

echo ""
success "Done! Site: ${SITE_URL}"
