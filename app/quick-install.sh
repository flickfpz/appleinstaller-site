#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────────────
# Quick Installer — one-command install for Rigset
#
# Usage (Linux / macOS):
#   curl -fsSL https://flickfpz.github.io/appleinstaller-site/install.sh | bash
#
# What it does:
#   1. Clones the Rigset repo (or pulls if already cloned)
#   2. Runs the full install.sh script
#   3. Cleans up the clone afterwards (optional)
# ─────────────────────────────────────────────────────────────────────────────

set -euo pipefail

REPO_URL="https://github.com/flickfpz/appleinstaller.git"
CLONE_DIR="${HOME}/.rigset-build"

RED='\033[0;31m'; GREEN='\033[0;32m'; YELLOW='\033[1;33m'
CYAN='\033[0;36m'; BOLD='\033[1m'; RESET='\033[0m'

info()    { echo -e "${CYAN}${BOLD}[INFO]${RESET}  $*"; }
success() { echo -e "${GREEN}${BOLD}[ OK ]${RESET}  $*"; }
warn()    { echo -e "${YELLOW}${BOLD}[WARN]${RESET}  $*"; }
die()     { echo -e "${RED}${BOLD}[FAIL]${RESET}  $*" >&2; exit 1; }

echo -e "${BOLD}"
echo "  ╔══════════════════════════════════════════════╗"
echo "  ║   Rigset — Quick One-Command Install         ║"
echo "  ╚══════════════════════════════════════════════╝"
echo -e "${RESET}"

# ── Check dependencies ───────────────────────────────────────────────────────
for cmd in git; do
    command -v "$cmd" &>/dev/null || die "'$cmd' is required but not installed."
done

# ── Clone or update ──────────────────────────────────────────────────────────
if [[ -d "$CLONE_DIR/.git" ]]; then
    info "Updating existing clone at $CLONE_DIR..."
    git -C "$CLONE_DIR" pull --ff-only --quiet || {
        warn "Pull failed — removing and re-cloning..."
        rm -rf "$CLONE_DIR"
        git clone --quiet "$REPO_URL" "$CLONE_DIR"
    }
else
    info "Cloning Rigset to $CLONE_DIR..."
    rm -rf "$CLONE_DIR"
    git clone --quiet "$REPO_URL" "$CLONE_DIR"
fi
success "Source ready."

# ── Run the full installer ───────────────────────────────────────────────────
info "Starting full installer..."
echo ""
bash "$CLONE_DIR/install.sh"

# ── Optional cleanup ─────────────────────────────────────────────────────────
echo ""
read -rp "Remove build files at $CLONE_DIR? [y/N]: " CLEAN_ANSWER
if [[ "${CLEAN_ANSWER,,}" == "y" || "${CLEAN_ANSWER,,}" == "yes" ]]; then
    rm -rf "$CLONE_DIR"
    success "Cleaned up build files."
else
    info "Build files kept at $CLONE_DIR"
fi
