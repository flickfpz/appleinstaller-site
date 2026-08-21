#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────────────
# Rigset — one-line install
#
# Usage:
#   curl -fsSL https://flickfpz.github.io/rigset/install.sh | bash
#
# What it does:
#   1. Detects OS (Linux / macOS) and architecture
#   2. Downloads the latest pre-built binary from the site
#   3. Installs it to /usr/local/bin (Linux) or /Applications (macOS)
# ─────────────────────────────────────────────────────────────────────────────

set -euo pipefail

SITE_BASE="https://flickfpz.github.io/rigset"
INSTALL_DIR="/usr/local/bin"
APP_NAME="rigset"

RED='\033[0;31m'; GREEN='\033[0;32m'; YELLOW='\033[1;33m'
CYAN='\033[0;36m'; BOLD='\033[1m'; RESET='\033[0m'

info()    { echo -e "${CYAN}${BOLD}[INFO]${RESET}  $*"; }
success() { echo -e "${GREEN}${BOLD}[ OK ]${RESET}  $*"; }
warn()    { echo -e "${YELLOW}${BOLD}[WARN]${RESET}  $*"; }
die()     { echo -e "${RED}${BOLD}[FAIL]${RESET}  $*" >&2; exit 1; }

echo -e "${BOLD}"
echo "  ╔══════════════════════════════════════════╗"
echo "  ║       Rigset — One-Line Install          ║"
echo "  ╚══════════════════════════════════════════╝"
echo -e "${RESET}"

# ── Detect OS ─────────────────────────────────────────────────────────────────
OS="unknown"
ARCH="unknown"

case "$(uname -s)" in
    Linux*)  OS="linux"  ;;
    Darwin*) OS="macos"  ;;
    MINGW*|MSYS*|CYGWIN*) OS="windows" ;;
esac

case "$(uname -m)" in
    x86_64|amd64)  ARCH="x86_64"   ;;
    aarch64|arm64) ARCH="aarch64"  ;;
    armv7l)        ARCH="armv7"     ;;
esac

info "Detected: ${BOLD}${OS}${RESET} / ${BOLD}${ARCH}${RESET}"

[[ "$OS" == "unknown" ]] && die "Unsupported OS. Only Linux and macOS are supported."
[[ "$ARCH" == "unknown" ]] && die "Unsupported architecture: $(uname -m)"

# ── Determine download asset name ────────────────────────────────────────────
if [[ "$OS" == "linux" ]]; then
    ASSET="Rigset-linux-x86_64.tar.gz"
elif [[ "$OS" == "macos" ]]; then
    ASSET="Rigset-macos-arm64.tar.gz"
fi

DOWNLOAD_URL="${SITE_BASE}/${ASSET}"
info "Download URL: ${BOLD}${DOWNLOAD_URL}${RESET}"

# ── Download ──────────────────────────────────────────────────────────────────
TMP_DIR="$(mktemp -d)"
trap 'rm -rf "$TMP_DIR"' EXIT

info "Downloading latest release..."
if command -v curl &>/dev/null; then
    curl -fSL --progress-bar "$DOWNLOAD_URL" -o "$TMP_DIR/$ASSET"
elif command -v wget &>/dev/null; then
    wget -q --show-progress "$DOWNLOAD_URL" -O "$TMP_DIR/$ASSET"
else
    die "Neither curl nor wget found. Please install one."
fi
success "Download complete."

# ── Extract ───────────────────────────────────────────────────────────────────
info "Extracting..."
tar -xzf "$TMP_DIR/$ASSET" -C "$TMP_DIR"

# Find the binary (could be named Rigset, rigset, or AppleInstaller)
BINARY="$(find "$TMP_DIR" -maxdepth 1 -type f -executable | head -1)"
if [[ -z "$BINARY" ]]; then
    BINARY="$(find "$TMP_DIR" -maxdepth 1 -type f \( -name 'Rigset' -o -name 'rigset' -o -name 'Rigset.app' \) | head -1)"
fi
[[ -n "$BINARY" ]] || die "Could not find the binary in the archive."

# ── Install ───────────────────────────────────────────────────────────────────
if [[ "$OS" == "macos" ]]; then
    DEST="/Applications/Rigset.app"
    if [[ -d "$BINARY" ]]; then
        info "Installing app bundle to $DEST..."
        rm -rf "$DEST"
        cp -r "$BINARY" "$DEST"
        success "Installed to $DEST"
    else
        info "Installing binary to $INSTALL_DIR..."
        sudo install -Dm755 "$BINARY" "$INSTALL_DIR/$APP_NAME"
        success "Installed to $INSTALL_DIR/$APP_NAME"
    fi
else
    info "Installing to $INSTALL_DIR..."
    sudo install -Dm755 "$BINARY" "$INSTALL_DIR/$APP_NAME"

    # .desktop entry
    DESKTOP_DIR="/usr/share/applications"
    info "Creating .desktop entry..."
    sudo mkdir -p "$DESKTOP_DIR"
    sudo tee "$DESKTOP_DIR/rigset.desktop" > /dev/null <<DESKTOP
[Desktop Entry]
Version=1.0
Type=Application
Name=Rigset
GenericName=Software Installer
Comment=Install your favourite apps in one click
Exec=rigset
Icon=rigset
Categories=System;PackageManager;
Keywords=install;apps;software;
StartupNotify=true
DESKTOP
    sudo update-desktop-database "$DESKTOP_DIR" &>/dev/null || true
    success "Installed to $INSTALL_DIR/$APP_NAME"
fi

echo ""
echo -e "${GREEN}${BOLD}════════════════════════════════════════════${RESET}"
echo -e "${GREEN}${BOLD}  Rigset installed successfully!${RESET}"
echo -e "${GREEN}${BOLD}  Run: rigset${RESET}"
echo -e "${GREEN}${BOLD}════════════════════════════════════════════${RESET}"
echo ""
