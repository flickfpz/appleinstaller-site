#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────────────
# build-and-publish.sh — Build Rigset for the current platform, package it,
#                        and commit binaries to this repo.
#
# Usage (run from repo root):
#   ./app/build-and-publish.sh [version]
#
# If no version is given, reads from app/CMakeLists.txt.
# ─────────────────────────────────────────────────────────────────────────────

set -euo pipefail

RED='\033[0;31m'; GREEN='\033[0;32m'; YELLOW='\033[1;33m'
CYAN='\033[0;36m'; BOLD='\033[1m'; RESET='\033[0m'

info()    { echo -e "${CYAN}${BOLD}[INFO]${RESET}  $*"; }
success() { echo -e "${GREEN}${BOLD}[ OK ]${RESET}  $*"; }
warn()    { echo -e "${YELLOW}${BOLD}[WARN]${RESET}  $*"; }
die()     { echo -e "${RED}${BOLD}[FAIL]${RESET}  $*" >&2; exit 1; }

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
SRC_DIR="$SCRIPT_DIR"
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
    VERSION=$(grep -oP 'project\(Rigset VERSION \K[0-9.]+' "$SRC_DIR/CMakeLists.txt" 2>/dev/null || echo "1.0.0")
fi
info "Version: ${BOLD}${VERSION}${RESET}"

# ── Build ──────────────────────────────────────────────────────────────────────
BUILD_DIR="$REPO_ROOT/build"
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

info "Configuring with CMake..."
cmake -S "$SRC_DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release -G Ninja 2>&1 | tail -3

info "Building ($(nproc 2>/dev/null || sysctl -n hw.logicalcpu) cores)..."
cmake --build "$BUILD_DIR" --parallel 2>&1 | tail -5

# ── Package ────────────────────────────────────────────────────────────────────
OUTPUT_DIR="$REPO_ROOT"
ASSET=""

if [[ "$PLATFORM" == "linux" ]]; then
    BINARY=""
    for b in "$BUILD_DIR/Rigset" "$BUILD_DIR/AppleInstaller"; do
        [[ -f "$b" ]] && BINARY="$b" && break
    done
    [[ -n "$BINARY" ]] || die "Linux binary not found in $BUILD_DIR"
    chmod +x "$BINARY"
    tar -czf "$OUTPUT_DIR/Rigset-linux-x86_64.tar.gz" -C "$BUILD_DIR" Rigset
    ASSET="Rigset-linux-x86_64.tar.gz"
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
        rm -f "$OUTPUT_DIR/Rigset"
    fi
    ASSET="Rigset-macos-arm64.tar.gz"
    success "Packaged: Rigset-macos-arm64.tar.gz"
fi

# ── Update version.json ────────────────────────────────────────────────────────
if command -v python3 &>/dev/null; then
    python3 -c "
import json
with open('$REPO_ROOT/version.json') as f:
    d = json.load(f)
d['version'] = '$VERSION'
with open('$REPO_ROOT/version.json', 'w') as f:
    json.dump(d, f, indent=2)
"
    info "Updated version.json to ${VERSION}"
fi

# ── Commit & push ──────────────────────────────────────────────────────────────
cd "$REPO_ROOT"
git add "$ASSET" version.json
if git diff --cached --quiet; then
    info "No changes to commit."
else
    git commit -m "Update binaries to v${VERSION} (${PLATFORM})"
    git push origin main
    success "Pushed to repo!"
fi

# ── Install locally (optional) ────────────────────────────────────────────────
echo ""
read -rp "Install locally now? [Y/n]: " INSTALL_LOCAL
if [[ "${INSTALL_LOCAL,,}" != "n" && "${INSTALL_LOCAL,,}" != "no" ]]; then
    INSTALL_DIR="/usr/local/bin"
    sudo install -Dm755 "$BUILD_DIR/Rigset" "$INSTALL_DIR/rigset"
    success "Installed to $INSTALL_DIR/rigset"
fi

echo ""
success "Done! Site: ${SITE_URL}"
