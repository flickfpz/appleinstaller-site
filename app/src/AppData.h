#pragma once
#include <QString>
#include <QMap>
#include <QStringList>
#include <QVector>
#include "OsDetect.h"

struct OsCmd {
    QString     program;
    QStringList args;
    bool        available = true;
};

struct AppData {
    QString id;
    QString name;
    QString description;
    QString category;
    QString iconName;
    QString iconPath;
    QString version;

    QMap<OsDetect::OS, OsCmd> cmds;

    OsCmd cmdForCurrentOs() const {
        auto it = cmds.find(OsDetect::current());
        if (it != cmds.end()) return it.value();
        OsCmd empty; empty.available = false; return empty;
    }

    bool availableOnCurrentOs() const {
        auto it = cmds.find(OsDetect::current());
        return it != cmds.end() && it.value().available;
    }
};

// Builder helpers — MUST be defined BEFORE defaultCatalogue()
namespace Detail {

inline AppData makeApp(
    const QString &id, const QString &name, const QString &desc,
    const QString &category, const QString &ver,
    const QString &win, const QString &mac,
    const QString &arch, const QString &deb, const QString &fed)
{
    AppData a;
    a.id = id; a.name = name; a.description = desc;
    a.category = category; a.version = ver;
    a.iconPath = QStringLiteral(":/icons/%1.svg").arg(id);

    if (!win.isEmpty()) {
        OsCmd c; c.program = "winget";
        c.args = { "install", "--silent",
                   "--accept-package-agreements",
                   "--accept-source-agreements", win };
        c.available = true;
        a.cmds[OsDetect::OS::Windows] = c;
    }
    if (!mac.isEmpty()) {
        OsCmd c; c.program = "brew";
        c.args = { "install", "--cask", mac };
        c.available = true;
        a.cmds[OsDetect::OS::macOS] = c;
    }
    if (!arch.isEmpty()) {
        OsCmd c; c.program = "sudo";
        c.args = { "pacman", "-S", "--noconfirm", "--needed", arch };
        c.available = true;
        a.cmds[OsDetect::OS::Arch] = c;
    }
    if (!deb.isEmpty()) {
        OsCmd c; c.program = "sudo";
        c.args = { "apt-get", "install", "-y", deb };
        c.available = true;
        a.cmds[OsDetect::OS::Debian] = c;
    }
    if (!fed.isEmpty()) {
        OsCmd c; c.program = "sudo";
        c.args = { "dnf", "install", "-y", fed };
        c.available = true;
        a.cmds[OsDetect::OS::Fedora] = c;
    }
    return a;
}

// makeFormula: CLI tool — brew formula (no --cask flag)
inline AppData makeFormula(
    const QString &id, const QString &name, const QString &desc,
    const QString &category, const QString &ver,
    const QString &win, const QString &mac,
    const QString &arch, const QString &deb, const QString &fed)
{
    AppData a = makeApp(id, name, desc, category, ver,
                        win, "", arch, deb, fed);
    if (!mac.isEmpty()) {
        OsCmd c; c.program = "brew";
        c.args = { "install", mac };
        c.available = true;
        a.cmds[OsDetect::OS::macOS] = c;
    }
    return a;
}

} // namespace Detail

// Default catalogue
inline QVector<AppData> defaultCatalogue()
{
    namespace D = Detail;
    QVector<AppData> cat;
    cat.reserve(27);

    // Browsers
    cat << D::makeApp("chrome","Google Chrome",
        "Fast, secure web browser","Browsers","latest",
        "Google.Chrome","google-chrome",
        "google-chrome","google-chrome-stable","google-chrome-stable");

    cat << D::makeApp("firefox","Firefox",
        "Privacy-focused open-source browser","Browsers","latest",
        "Mozilla.Firefox","firefox",
        "firefox","firefox","firefox");

    cat << D::makeApp("brave","Brave",
        "Ad-blocking browser built for speed","Browsers","latest",
        "Brave.Brave","brave-browser",
        "brave-bin","brave-browser","brave-browser");

    cat << D::makeApp("edge","Microsoft Edge",
        "Chromium browser by Microsoft","Browsers","latest",
        "Microsoft.Edge","microsoft-edge",
        "microsoft-edge-stable-bin","microsoft-edge-stable","microsoft-edge-stable");

    // Media
    cat << D::makeApp("vlc","VLC",
        "Play any video or audio file","Media","3.0.x",
        "VideoLAN.VLC","vlc",
        "vlc","vlc","vlc");

    cat << D::makeApp("spotify","Spotify",
        "Music streaming & podcasts","Media","latest",
        "Spotify.Spotify","spotify",
        "spotify","spotify-client","lpf-spotify-client");

    cat << D::makeApp("handbrake","HandBrake",
        "Open-source video transcoder","Media","1.7.x",
        "HandBrake.HandBrake","handbrake",
        "handbrake","handbrake","HandBrake-gui");

    cat << D::makeFormula("mpv","mpv",
        "Lightweight, scriptable media player","Media","latest",
        "mpv.mpv","mpv",
        "mpv","mpv","mpv");

    // Dev Tools
    cat << D::makeApp("vscode","VS Code",
        "Lightweight, powerful code editor","Dev Tools","latest",
        "Microsoft.VisualStudioCode","visual-studio-code",
        "visual-studio-code-bin","code","code");

    // AI Coding Tools
    {
        AppData a; a.id = "claude-code"; a.name = "Claude Code";
        a.description = "AI pair programmer by Anthropic"; a.category = "AI Tools"; a.version = "latest";
        a.iconPath = ":/icons/claude.svg";
        { OsCmd c; c.program = "winget"; c.args = {"install","--silent","--accept-package-agreements","--accept-source-agreements","Anthropic.ClaudeCode"}; c.available = true; a.cmds[OsDetect::OS::Windows] = c; }
        { OsCmd c; c.program = "brew"; c.args = {"install","--cask","claude-code"}; c.available = true; a.cmds[OsDetect::OS::macOS] = c; }
        { OsCmd c; c.program = "npm"; c.args = {"install","-g","@anthropic-ai/claude-code"}; c.available = true; a.cmds[OsDetect::OS::Arch] = c; }
        a.cmds[OsDetect::OS::Debian] = a.cmds[OsDetect::OS::Arch];
        a.cmds[OsDetect::OS::Fedora] = a.cmds[OsDetect::OS::Arch];
        cat << a;
    }
    {
        AppData a; a.id = "codex"; a.name = "Codex";
        a.description = "AI coding agent by OpenAI"; a.category = "AI Tools"; a.version = "latest";
        a.iconPath = ":/icons/openai.svg";
        { OsCmd c; c.program = "winget"; c.args = {"install","--silent","--accept-package-agreements","--accept-source-agreements","OpenAI.Codex"}; c.available = true; a.cmds[OsDetect::OS::Windows] = c; }
        { OsCmd c; c.program = "brew"; c.args = {"install","--cask","codex"}; c.available = true; a.cmds[OsDetect::OS::macOS] = c; }
        { OsCmd c; c.program = "npm"; c.args = {"install","-g","@openai/codex"}; c.available = true; a.cmds[OsDetect::OS::Arch] = c; }
        a.cmds[OsDetect::OS::Debian] = a.cmds[OsDetect::OS::Arch];
        a.cmds[OsDetect::OS::Fedora] = a.cmds[OsDetect::OS::Arch];
        cat << a;
    }
    {
        AppData a; a.id = "opencode"; a.name = "OpenCode";
        a.description = "Open-source AI coding assistant"; a.category = "AI Tools"; a.version = "latest";
        a.iconPath = ":/icons/opencode.svg";
        { OsCmd c; c.program = "brew"; c.args = {"install","anomalyco/tap/opencode"}; c.available = true; a.cmds[OsDetect::OS::macOS] = c; }
        { OsCmd c; c.program = "npm"; c.args = {"install","-g","opencode-ai"}; c.available = true; a.cmds[OsDetect::OS::Arch] = c; }
        a.cmds[OsDetect::OS::Debian] = a.cmds[OsDetect::OS::Arch];
        a.cmds[OsDetect::OS::Fedora] = a.cmds[OsDetect::OS::Arch];
        cat << a;
    }

    cat << D::makeFormula("git","Git",
        "Distributed version control system","Dev Tools","latest",
        "Git.Git","git",
        "git","git","git");

    cat << D::makeFormula("nodejs","Node.js LTS",
        "JavaScript runtime built on V8","Dev Tools","20.x",
        "OpenJS.NodeJS.LTS","node",
        "nodejs","nodejs","nodejs");

    cat << D::makeFormula("python","Python 3",
        "General-purpose scripting language","Dev Tools","3.12.x",
        "Python.Python.3","python",
        "python","python3","python3");

    cat << D::makeApp("wt","Windows Terminal",
        "Modern terminal for Windows","Dev Tools","latest",
        "Microsoft.WindowsTerminal","",
        "","","");

    cat << D::makeFormula("cmake","CMake",
        "Cross-platform build system","Dev Tools","latest",
        "Kitware.CMake","cmake",
        "cmake","cmake","cmake");

    cat << D::makeFormula("neovim","Neovim",
        "Hyperextensible Vim-based text editor","Dev Tools","latest",
        "Neovim.Neovim","neovim",
        "neovim","neovim","neovim");

    cat << D::makeFormula("gh","GitHub CLI",
        "GitHub from the command line","Dev Tools","latest",
        "GitHub.cli","gh",
        "github-cli","gh","gh");

    // Productivity
    cat << D::makeApp("7zip","7-Zip",
        "Open-source file archiver","Productivity","23.x",
        "7zip.7zip","sevenzip",
        "p7zip","p7zip-full","p7zip");

    cat << D::makeApp("obsidian","Obsidian",
        "Markdown knowledge base","Productivity","latest",
        "Obsidian.Obsidian","obsidian",
        "obsidian","obsidian","obsidian");

    cat << D::makeApp("notion","Notion",
        "All-in-one workspace","Productivity","latest",
        "Notion.Notion","notion",
        "notion-app","","");

    cat << D::makeApp("powertoys","PowerToys",
        "Window snapping & utilities","Productivity","latest",
        "Microsoft.PowerToys","",
        "","","");

    // Communication
    cat << D::makeApp("discord","Discord",
        "Voice, video & text chat","Communication","latest",
        "Discord.Discord","discord",
        "discord","discord","discord");

    cat << D::makeApp("slack","Slack",
        "Team messaging platform","Communication","latest",
        "SlackTechnologies.Slack","slack",
        "slack-desktop","slack","slack");

    cat << D::makeApp("zoom","Zoom",
        "Video conferencing","Communication","latest",
        "Zoom.Zoom","zoom",
        "zoom","zoom","zoom");

    cat << D::makeApp("telegram","Telegram",
        "Fast, secure messenger","Communication","latest",
        "Telegram.TelegramDesktop","telegram",
        "telegram-desktop","telegram-desktop","telegram-desktop");

    // Security
    cat << D::makeApp("bitwarden","Bitwarden",
        "Open-source password manager","Security","latest",
        "Bitwarden.Bitwarden","bitwarden",
        "bitwarden","bitwarden","bitwarden");

    cat << D::makeFormula("gnupg","GnuPG",
        "Encrypt & sign your data","Security","latest",
        "GnuPG.GnuPG","gnupg",
        "gnupg","gnupg","gnupg2");

    cat << D::makeFormula("wireguard","WireGuard",
        "Fast, modern, secure VPN tunnel","Security","latest",
        "WireGuard.wireguard","wireguard-tools",
        "wireguard-tools","wireguard","wireguard-tools");

    return cat;
}
