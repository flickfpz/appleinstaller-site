#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────────────
# Rigset — self-installer for Linux & macOS
#
# Usage:
#   chmod +x install.sh && ./install.sh
#
# What it does:
#   1. Detects OS (macOS / Arch / Debian / Fedora / Unknown)
#   2. Installs build dependencies (Qt6, CMake, GCC/Clang)
#   3. Clones or uses the local source tree
#   4. Builds with CMake
#   5. Installs the binary + .desktop entry (Linux) or .app bundle (macOS)
# ─────────────────────────────────────────────────────────────────────────────

set -euo pipefail

# ── Colours ───────────────────────────────────────────────────────────────────
RED='\033[0;31m'; GREEN='\033[0;32m'; YELLOW='\033[1;33m'
CYAN='\033[0;36m'; BOLD='\033[1m'; RESET='\033[0m'

info()    { echo -e "${CYAN}${BOLD}[INFO]${RESET}  $*"; }
success() { echo -e "${GREEN}${BOLD}[ OK ]${RESET}  $*"; }
warn()    { echo -e "${YELLOW}${BOLD}[WARN]${RESET}  $*"; }
die()     { echo -e "${RED}${BOLD}[FAIL]${RESET}  $*" >&2; exit 1; }

# ── Banner ────────────────────────────────────────────────────────────────────
echo -e "${BOLD}"
echo "  ╔══════════════════════════════════════════╗"
echo "  ║           Rigset — Self Installer        ║"
echo "  ╚══════════════════════════════════════════╝"
echo -e "${RESET}"

# ── Detect OS ─────────────────────────────────────────────────────────────────
DETECTED_OS="unknown"

if [[ "$OSTYPE" == darwin* ]]; then
    DETECTED_OS="macos"
elif [[ -f /etc/os-release ]]; then
    source /etc/os-release
    ID_LOWER="${ID,,}"
    ID_LIKE_LOWER="${ID_LIKE,,}"

    if [[ "$ID_LOWER" == "arch"       || "$ID_LIKE_LOWER" == *"arch"*    \
       || "$ID_LOWER" == "manjaro"    || "$ID_LOWER" == "endeavouros"    \
       || "$ID_LOWER" == "garuda" ]]; then
        DETECTED_OS="arch"
    elif [[ "$ID_LOWER" == "fedora"   || "$ID_LIKE_LOWER" == *"fedora"*  \
         || "$ID_LOWER" == "rhel"     || "$ID_LOWER" == "centos"         \
         || "$ID_LOWER" == "almalinux"|| "$ID_LOWER" == "rocky" ]]; then
        DETECTED_OS="fedora"
    elif [[ "$ID_LOWER" == "debian"   || "$ID_LIKE_LOWER" == *"debian"*  \
         || "$ID_LOWER" == "ubuntu"   || "$ID_LIKE_LOWER" == *"ubuntu"*  \
         || "$ID_LOWER" == "linuxmint"|| "$ID_LOWER" == "pop"            \
         || "$ID_LOWER" == "elementary"|| "$ID_LOWER" == "kali" ]]; then
        DETECTED_OS="debian"
    fi
fi

info "Detected OS: ${BOLD}${DETECTED_OS}${RESET}"
[[ "$DETECTED_OS" == "unknown" ]] && die "Unsupported OS. Only macOS, Arch, Debian/Ubuntu, Fedora are supported."

# ── Install build dependencies ────────────────────────────────────────────────
info "Installing build dependencies…"

case "$DETECTED_OS" in

  macos)
    # Ensure Homebrew is present
    if ! command -v brew &>/dev/null; then
        warn "Homebrew not found — installing it now…"
        /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
        # Add brew to PATH for Apple Silicon
        if [[ -f /opt/homebrew/bin/brew ]]; then
            eval "$(/opt/homebrew/bin/brew shellenv)"
        fi
    fi
    brew update --quiet
    brew install --quiet cmake qt@6 ninja
    export PATH="/opt/homebrew/opt/qt@6/bin:/opt/homebrew/bin:/usr/local/opt/qt@6/bin:$PATH"
    ;;

  arch)
    sudo pacman -Sy --noconfirm --needed cmake qt6-base qt6-tools gcc ninja
    ;;

  debian)
    sudo apt-get update -qq
    sudo apt-get install -y --no-install-recommends \
        build-essential cmake ninja-build \
        qt6-base-dev qt6-tools-dev qt6-tools-dev-tools \
        libqt6widgets6 libqt6network6 libqt6concurrent6 \
        qt6-base-dev-tools libgl-dev
    ;;

  fedora)
    sudo dnf install -y \
        cmake ninja-build gcc-c++ \
        qt6-qtbase-devel qt6-qttools-devel \
        mesa-libGL-devel
    ;;

esac
success "Build dependencies installed."

# ── Locate source root ────────────────────────────────────────────────────────
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SRC_DIR="$SCRIPT_DIR"

# If CMakeLists.txt isn't here, try a few likely locations
if [[ ! -f "$SRC_DIR/CMakeLists.txt" ]]; then
    for candidate in "$PWD" "$HOME/appleinstaller" "/opt/appleinstaller"; do
        if [[ -f "$candidate/CMakeLists.txt" ]]; then
            SRC_DIR="$candidate"; break
        fi
    done
fi

[[ -f "$SRC_DIR/CMakeLists.txt" ]] || die "Could not find CMakeLists.txt. Run this script from the source directory."
info "Source directory: $SRC_DIR"

# ── Build ──────────────────────────────────────────────────────────────────────
BUILD_DIR="$SRC_DIR/build"
mkdir -p "$BUILD_DIR"

info "Configuring with CMake…"
cmake -S "$SRC_DIR" -B "$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE=Release \
    -G Ninja \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    2>&1 | tail -5

info "Building ($(nproc 2>/dev/null || sysctl -n hw.logicalcpu) cores)…"
cmake --build "$BUILD_DIR" --parallel 2>&1 | grep -E "^\[|error:|warning:" || true

BINARY=""
if [[ "$DETECTED_OS" == "macos" ]]; then
    BINARY="$BUILD_DIR/Rigset.app/Contents/MacOS/Rigset"
    [[ -f "$BINARY" ]] || BINARY="$BUILD_DIR/Rigset"
else
    BINARY="$BUILD_DIR/Rigset"
fi

[[ -x "$BINARY" ]] || die "Build failed — binary not found at $BINARY"
success "Build complete: $BINARY"

# ── Install ────────────────────────────────────────────────────────────────────
INSTALL_DIR="/usr/local/bin"
DESKTOP_DIR="/usr/share/applications"
ICON_DIR="/usr/share/icons/hicolor/256x256/apps"

case "$DETECTED_OS" in

  macos)
    APP_BUNDLE="$BUILD_DIR/Rigset.app"
    DEST="/Applications/Rigset.app"
    if [[ -d "$APP_BUNDLE" ]]; then
        info "Installing app bundle to $DEST…"
        rm -rf "$DEST"
        cp -r "$APP_BUNDLE" "$DEST"
        success "Installed to $DEST"
    else
        info "Installing binary to $INSTALL_DIR…"
        sudo cp "$BINARY" "$INSTALL_DIR/rigset"
        sudo chmod 755 "$INSTALL_DIR/rigset"
        success "Installed to $INSTALL_DIR/rigset"
    fi
    ;;

  arch|debian|fedora)
    info "Installing binary to $INSTALL_DIR…"
    sudo install -Dm755 "$BINARY" "$INSTALL_DIR/rigset"

    # .desktop entry
    info "Creating .desktop entry…"
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

    # Install icon if it exists alongside the script
    if [[ -f "$SCRIPT_DIR/resources/icons/rigset.png" ]]; then
        sudo mkdir -p "$ICON_DIR"
        sudo install -Dm644 "$SCRIPT_DIR/resources/icons/rigset.png" \
            "$ICON_DIR/rigset.png"
        sudo gtk-update-icon-cache /usr/share/icons/hicolor &>/dev/null || true
    fi

    sudo update-desktop-database "$DESKTOP_DIR" &>/dev/null || true
    success "Installed to $INSTALL_DIR/rigset"
    ;;

esac

# ── Done ──────────────────────────────────────────────────────────────────────
echo ""
echo -e "${GREEN}${BOLD}════════════════════════════════════════════${RESET}"
echo -e "${GREEN}${BOLD}  Rigset installed successfully!${RESET}"
echo -e "${GREEN}${BOLD}  Run: rigset${RESET}"
echo -e "${GREEN}${BOLD}════════════════════════════════════════════${RESET}"
echo ""

# ── Reboot prompt (Linux only) ────────────────────────────────────────────────
if [[ "$DETECTED_OS" != "macos" ]]; then
    echo -e "${CYAN}${BOLD}A reboot is recommended to ensure all changes take effect.${RESET}"
    read -rp "Reboot now? [y/N]: " REBOOT_ANSWER
    if [[ "${REBOOT_ANSWER,,}" == "y" || "${REBOOT_ANSWER,,}" == "yes" ]]; then
        info "Rebooting in 5 seconds… (press Ctrl+C to cancel)"
        sleep 5
        sudo reboot
    else
        info "Reboot skipped. Please reboot when convenient."
    fi
fi
