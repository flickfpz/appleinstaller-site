/**
 * js/apps.js — App Registry
 *
 * Defines window.APP_REGISTRY: the single source of truth for all apps
 * shown in the Demo Panel and any app listing on the page.
 *
 * Each entry shape:
 *   {
 *     id:          string  — unique kebab-case identifier
 *     name:        string  — human-readable display name
 *     category:    string  — one of the defined categories
 *     description: string  — plain-text summary, max 80 characters
 *     icon:        string  — emoji for display in the Demo Panel
 *     platform:    string  — 'all' | 'windows' | 'linux' | 'mac'
 *     commands:    object  — per-platform install commands
 *   }
 */

(function () {
  'use strict';

  window.APP_REGISTRY = [

    // ── Browsers ──────────────────────────────────────────────────────────────
    {
      id: 'firefox', name: 'Firefox', category: 'Browsers',
      description: 'Privacy-focused open-source browser by Mozilla',
      icon: '🦊', platform: 'all',
      commands: {
        windows: 'winget install --id Mozilla.Firefox --silent --accept-package-agreements --accept-source-agreements',
        mac: 'brew install --cask firefox',
        linux: 'sudo pacman -S --noconfirm --needed firefox'
      }
    },
    {
      id: 'chromium', name: 'Chromium', category: 'Browsers',
      description: 'Open-source browser project behind Google Chrome',
      icon: '🌐', platform: 'all',
      commands: {
        windows: 'winget install --id Google.Chrome --silent --accept-package-agreements --accept-source-agreements',
        mac: 'brew install --cask chromium',
        linux: 'sudo pacman -S --noconfirm --needed chromium'
      }
    },
    {
      id: 'brave', name: 'Brave', category: 'Browsers',
      description: 'Privacy-first browser with built-in ad and tracker blocking',
      icon: '🦁', platform: 'all',
      commands: {
        windows: 'winget install --id Brave.Brave --silent --accept-package-agreements --accept-source-agreements',
        mac: 'brew install --cask brave-browser',
        linux: 'sudo pacman -S --noconfirm --needed brave-bin'
      }
    },
    {
      id: 'tor-browser', name: 'Tor Browser', category: 'Browsers',
      description: 'Anonymous browsing via the Tor network',
      icon: '🧅', platform: 'all',
      commands: {
        windows: 'winget install --id TorProject.TorBrowser --silent --accept-package-agreements --accept-source-agreements',
        mac: 'brew install --cask tor-browser',
        linux: 'sudo pacman -S --noconfirm --needed torbrowser-launcher'
      }
    },
    {
      id: 'zen-browser', name: 'Zen Browser', category: 'Browsers',
      description: 'Privacy-focused Firefox-based browser with a minimal UI',
      icon: '🧘', platform: 'all',
      commands: {
        windows: 'winget install --id ZenZenZen.ZenBrowser --silent --accept-package-agreements --accept-source-agreements',
        mac: 'brew install --cask zen-browser',
        linux: 'yay -S --noconfirm zen-browser-bin'
      }
    },

    // ── Media ─────────────────────────────────────────────────────────────────
    {
      id: 'vlc', name: 'VLC', category: 'Media',
      description: 'Versatile open-source media player for all formats',
      icon: '🎥', platform: 'all',
      commands: {
        windows: 'winget install --id VideoLAN.VLC --silent --accept-package-agreements --accept-source-agreements',
        mac: 'brew install --cask vlc',
        linux: 'sudo pacman -S --noconfirm --needed vlc'
      }
    },
    {
      id: 'mpv', name: 'MPV', category: 'Media',
      description: 'Lightweight, scriptable command-line media player',
      icon: '▶️', platform: 'all',
      commands: {
        windows: 'winget install --id mpv.mpv --silent --accept-package-agreements --accept-source-agreements',
        mac: 'brew install mpv',
        linux: 'sudo pacman -S --noconfirm --needed mpv'
      }
    },
    {
      id: 'spotify', name: 'Spotify', category: 'Media',
      description: 'Stream music, podcasts, and playlists from Spotify',
      icon: '🎵', platform: 'all',
      commands: {
        windows: 'winget install --id Spotify.Spotify --silent --accept-package-agreements --accept-source-agreements',
        mac: 'brew install --cask spotify',
        linux: 'sudo pacman -S --noconfirm --needed spotify'
      }
    },
    {
      id: 'obs-studio', name: 'OBS Studio', category: 'Media',
      description: 'Free open-source software for video recording and streaming',
      icon: '🎙️', platform: 'all',
      commands: {
        windows: 'winget install --id OBSProject.OBSStudio --silent --accept-package-agreements --accept-source-agreements',
        mac: 'brew install --cask obs',
        linux: 'sudo pacman -S --noconfirm --needed obs-studio'
      }
    },
    {
      id: 'handbrake', name: 'HandBrake', category: 'Media',
      description: 'Open-source video transcoder for converting any format',
      icon: '🎞️', platform: 'all',
      commands: {
        windows: 'winget install --id HandBrake.HandBrake --silent --accept-package-agreements --accept-source-agreements',
        mac: 'brew install --cask handbrake',
        linux: 'sudo pacman -S --noconfirm --needed handbrake'
      }
    },
    {
      id: 'roblox', name: 'Roblox', category: 'Media',
      description: 'Online game platform — uses Sober on Linux',
      icon: '🎮', platform: 'all',
      commands: {
        windows: 'winget install --id Roblox.Roblox --silent --accept-package-agreements --accept-source-agreements',
        mac: 'brew install --cask roblox',
        linux: 'flatpak install -y com.github.hexhex.sober'
      }
    },
    {
      id: 'steam', name: 'Steam', category: 'Media',
      description: 'Valve\'s digital game distribution platform',
      icon: '🎲', platform: 'all',
      commands: {
        windows: 'winget install --id Valve.Steam --silent --accept-package-agreements --accept-source-agreements',
        mac: 'brew install --cask steam',
        linux: 'sudo pacman -S --noconfirm --needed steam'
      }
    },
    {
      id: 'lively-wallpapers', name: 'Lively Wallpapers', category: 'Media',
      description: 'Free and open-source animated desktop wallpapers',
      icon: '🖼️', platform: 'windows',
      commands: {
        windows: 'winget install --id rocksdanister.LivelyWallpaper --silent --accept-package-agreements --accept-source-agreements'
      }
    },
    {
      id: 'lutris', name: 'Lutris', category: 'Media',
      description: 'Open-source gaming platform for Linux game management',
      icon: '🕹️', platform: 'linux',
      commands: {
        linux: 'sudo pacman -S --noconfirm --needed lutris'
      }
    },

    // ── Dev Tools ─────────────────────────────────────────────────────────────
    {
      id: 'vscode', name: 'VS Code', category: 'Dev Tools',
      description: 'Lightweight but powerful source code editor by Microsoft',
      icon: '💻', platform: 'all',
      commands: {
        windows: 'winget install --id Microsoft.VisualStudioCode --silent --accept-package-agreements --accept-source-agreements',
        mac: 'brew install --cask visual-studio-code',
        linux: 'sudo pacman -S --noconfirm --needed visual-studio-code-bin'
      }
    },
    {
      id: 'git', name: 'Git', category: 'Dev Tools',
      description: 'Distributed version control system for tracking code changes',
      icon: '🌿', platform: 'all',
      commands: {
        windows: 'winget install --id Git.Git --silent --accept-package-agreements --accept-source-agreements',
        mac: 'brew install git',
        linux: 'sudo pacman -S --noconfirm --needed git'
      }
    },
    {
      id: 'docker', name: 'Docker', category: 'Dev Tools',
      description: 'Platform for building and running containerised applications',
      icon: '🐳', platform: 'all',
      commands: {
        windows: 'winget install --id Docker.DockerDesktop --silent --accept-package-agreements --accept-source-agreements',
        mac: 'brew install --cask docker',
        linux: 'sudo pacman -S --noconfirm --needed docker docker-compose'
      }
    },
    {
      id: 'nodejs', name: 'Node.js', category: 'Dev Tools',
      description: 'JavaScript runtime built on Chrome\'s V8 engine',
      icon: '🟩', platform: 'all',
      commands: {
        windows: 'winget install --id OpenJS.NodeJS.LTS --silent --accept-package-agreements --accept-source-agreements',
        mac: 'brew install node',
        linux: 'sudo pacman -S --noconfirm --needed nodejs npm'
      }
    },
    {
      id: 'python', name: 'Python', category: 'Dev Tools',
      description: 'General-purpose programming language, batteries included',
      icon: '🐍', platform: 'all',
      commands: {
        windows: 'winget install --id Python.Python.3.12 --silent --accept-package-agreements --accept-source-agreements',
        mac: 'brew install python',
        linux: 'sudo pacman -S --noconfirm --needed python python-pip'
      }
    },
    {
      id: 'neovim', name: 'Neovim', category: 'Dev Tools',
      description: 'Hyperextensible Vim-based text editor',
      icon: '✏️', platform: 'all',
      commands: {
        windows: 'winget install --id Neovim.Neovim --silent --accept-package-agreements --accept-source-agreements',
        mac: 'brew install neovim',
        linux: 'sudo pacman -S --noconfirm --needed neovim'
      }
    },
    {
      id: 'kiro-ide', name: 'Kiro IDE', category: 'Dev Tools',
      description: 'AI-powered IDE with spec-driven development workflows',
      icon: '⚡', platform: 'all',
      commands: {
        windows: 'winget install --id Amazon.Kiro --silent --accept-package-agreements --accept-source-agreements',
        mac: 'brew install --cask kiro',
        linux: 'yay -S --noconfirm kiro'
      }
    },

    // ── Productivity ──────────────────────────────────────────────────────────
    {
      id: 'libreoffice', name: 'LibreOffice', category: 'Productivity',
      description: 'Free and open-source office suite compatible with MS Office',
      icon: '📄', platform: 'all',
      commands: {
        windows: 'winget install --id TheDocumentFoundation.LibreOffice --silent --accept-package-agreements --accept-source-agreements',
        mac: 'brew install --cask libreoffice',
        linux: 'sudo pacman -S --noconfirm --needed libreoffice-still'
      }
    },
    {
      id: 'obsidian', name: 'Obsidian', category: 'Productivity',
      description: 'Markdown-based personal knowledge base and note-taking app',
      icon: '💎', platform: 'all',
      commands: {
        windows: 'winget install --id Obsidian.Obsidian --silent --accept-package-agreements --accept-source-agreements',
        mac: 'brew install --cask obsidian',
        linux: 'sudo pacman -S --noconfirm --needed obsidian'
      }
    },
    {
      id: 'notion', name: 'Notion', category: 'Productivity',
      description: 'All-in-one workspace for notes, tasks, wikis, and databases',
      icon: '📝', platform: 'all',
      commands: {
        windows: 'winget install --id Notion.Notion --silent --accept-package-agreements --accept-source-agreements',
        mac: 'brew install --cask notion',
        linux: 'yay -S --noconfirm notion-app'
      }
    },
    {
      id: 'thunderbird', name: 'Thunderbird', category: 'Productivity',
      description: 'Free open-source email client by Mozilla',
      icon: '📨', platform: 'all',
      commands: {
        windows: 'winget install --id Mozilla.Thunderbird --silent --accept-package-agreements --accept-source-agreements',
        mac: 'brew install --cask thunderbird',
        linux: 'sudo pacman -S --noconfirm --needed thunderbird'
      }
    },
    {
      id: 'revo-uninstaller', name: 'Revo Uninstaller', category: 'Productivity',
      description: 'Thorough software uninstaller with leftover file cleanup',
      icon: '🗑️', platform: 'windows',
      commands: {
        windows: 'winget install --id VS.RevoUninstaller --silent --accept-package-agreements --accept-source-agreements'
      }
    },
    {
      id: 'translucenttb', name: 'TranslucentTB', category: 'Productivity',
      description: 'Lightweight app for a translucent taskbar on Windows',
      icon: '🪟', platform: 'windows',
      commands: {
        windows: 'winget install --id TranslucentTB.TranslucentTB --silent --accept-package-agreements --accept-source-agreements'
      }
    },
    {
      id: 'rainmeter', name: 'Rainmeter', category: 'Productivity',
      description: 'Desktop customisation tool with skins and widgets',
      icon: '🌧️', platform: 'windows',
      commands: {
        windows: 'winget install --id RainbowMetric.Rainmeter --silent --accept-package-agreements --accept-source-agreements'
      }
    },
    {
      id: 'windhawk', name: 'Windhawk', category: 'Productivity',
      description: 'Customisation platform for Windows Explorer and apps',
      icon: '🌬️', platform: 'windows',
      commands: {
        windows: 'winget install --id Windhawk.Windhawk --silent --accept-package-agreements --accept-source-agreements'
      }
    },

    // ── Communication ─────────────────────────────────────────────────────────
    {
      id: 'discord', name: 'Discord', category: 'Communication',
      description: 'Voice, video, and text chat for communities and friends',
      icon: '💬', platform: 'all',
      commands: {
        windows: 'winget install --id Discord.Discord --silent --accept-package-agreements --accept-source-agreements',
        mac: 'brew install --cask discord',
        linux: 'sudo pacman -S --noconfirm --needed discord'
      }
    },
    {
      id: 'equibop', name: 'Equibop', category: 'Communication',
      description: 'Lightweight Discord client built for Linux',
      icon: '🫧', platform: 'linux',
      commands: {
        linux: 'yay -S --noconfirm equibop-bin'
      }
    },
    {
      id: 'telegram', name: 'Telegram', category: 'Communication',
      description: 'Cloud-based instant messaging with strong privacy features',
      icon: '✈️', platform: 'all',
      commands: {
        windows: 'winget install --id Telegram.TelegramDesktop --silent --accept-package-agreements --accept-source-agreements',
        mac: 'brew install --cask telegram',
        linux: 'sudo pacman -S --noconfirm --needed telegram-desktop'
      }
    },
    {
      id: 'signal', name: 'Signal', category: 'Communication',
      description: 'End-to-end encrypted messaging app focused on privacy',
      icon: '🔒', platform: 'all',
      commands: {
        windows: 'winget install --id Signal.SignalsDesktop --silent --accept-package-agreements --accept-source-agreements',
        mac: 'brew install --cask signal',
        linux: 'sudo pacman -S --noconfirm --needed signal-desktop'
      }
    },
    {
      id: 'slack', name: 'Slack', category: 'Communication',
      description: 'Team messaging and collaboration platform for workplaces',
      icon: '💼', platform: 'all',
      commands: {
        windows: 'winget install --id SlackTechnologies.Slack --silent --accept-package-agreements --accept-source-agreements',
        mac: 'brew install --cask slack',
        linux: 'sudo pacman -S --noconfirm --needed slack-desktop'
      }
    },

    // ── Security ──────────────────────────────────────────────────────────────
    {
      id: 'bitwarden', name: 'Bitwarden', category: 'Security',
      description: 'Open-source password manager with cloud sync',
      icon: '🛡️', platform: 'all',
      commands: {
        windows: 'winget install --id Bitwarden.Bitwarden --silent --accept-package-agreements --accept-source-agreements',
        mac: 'brew install --cask bitwarden',
        linux: 'sudo pacman -S --noconfirm --needed bitwarden'
      }
    },
    {
      id: 'keepassxc', name: 'KeePassXC', category: 'Security',
      description: 'Offline cross-platform password manager, no cloud required',
      icon: '🔑', platform: 'all',
      commands: {
        windows: 'winget install --id KeePassXCTeam.KeePassXC --silent --accept-package-agreements --accept-source-agreements',
        mac: 'brew install --cask keepassxc',
        linux: 'sudo pacman -S --noconfirm --needed keepassxc'
      }
    },
    {
      id: 'mullvad-vpn', name: 'Mullvad VPN', category: 'Security',
      description: 'Privacy-first VPN with anonymous accounts, no logs',
      icon: '🕵️', platform: 'all',
      commands: {
        windows: 'winget install --id MullvadVPN.MullvadVPN --silent --accept-package-agreements --accept-source-agreements',
        mac: 'brew install --cask mullvad-vpn',
        linux: 'sudo pacman -S --noconfirm --needed mullvad-vpn'
      }
    },
    {
      id: 'wireguard', name: 'WireGuard', category: 'Security',
      description: 'Fast, modern, and secure VPN tunnel protocol',
      icon: '🔐', platform: 'all',
      commands: {
        windows: 'winget install --id WireGuard.WireGuard --silent --accept-package-agreements --accept-source-agreements',
        mac: 'brew install --cask wireguard',
        linux: 'sudo pacman -S --noconfirm --needed wireguard-tools'
      }
    },

  ];

}());
