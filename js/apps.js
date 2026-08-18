/**
 * js/apps.js — App Registry
 *
 * Defines window.APP_REGISTRY: the single source of truth for all apps
 * shown in the Demo Panel and any app listing on the page.
 *
 * No imports, no dependencies. Safe to load via file:// without CORS issues.
 *
 * Each entry shape:
 *   {
 *     id:          string  — unique kebab-case identifier
 *     name:        string  — human-readable display name
 *     category:    string  — one of the six defined categories
 *     description: string  — plain-text summary, max 80 characters
 *     icon:        string  — emoji for display in the Demo Panel
 *     platform:    string  — 'all' | 'windows' | 'linux' | 'mac'
 *   }
 *
 * Valid categories: Browsers | Media | Dev Tools | Productivity | Communication | Security
 * Valid platforms:  all | windows | linux | mac
 */

(function () {
  'use strict';

  window.APP_REGISTRY = [

    // ── Browsers ──────────────────────────────────────────────────────────────
    {
      id:          'firefox',
      name:        'Firefox',
      category:    'Browsers',
      description: 'Privacy-focused open-source browser by Mozilla',
      icon:        '🦊',
      platform:    'all',
    },
    {
      id:          'chromium',
      name:        'Chromium',
      category:    'Browsers',
      description: 'Open-source browser project behind Google Chrome',
      icon:        '🌐',
      platform:    'all',
    },
    {
      id:          'brave',
      name:        'Brave',
      category:    'Browsers',
      description: 'Privacy-first browser with built-in ad and tracker blocking',
      icon:        '🦁',
      platform:    'all',
    },
    {
      id:          'tor-browser',
      name:        'Tor Browser',
      category:    'Browsers',
      description: 'Anonymous browsing via the Tor network',
      icon:        '🧅',
      platform:    'all',
    },
    {
      id:          'zen-browser',
      name:        'Zen Browser',
      category:    'Browsers',
      description: 'Privacy-focused Firefox-based browser with a minimal UI',
      icon:        '🧘',
      platform:    'all',
    },

    // ── Media ─────────────────────────────────────────────────────────────────
    {
      id:          'vlc',
      name:        'VLC',
      category:    'Media',
      description: 'Versatile open-source media player for all formats',
      icon:        '🎥',
      platform:    'all',
    },
    {
      id:          'mpv',
      name:        'MPV',
      category:    'Media',
      description: 'Lightweight, scriptable command-line media player',
      icon:        '▶️',
      platform:    'all',
    },
    {
      id:          'spotify',
      name:        'Spotify',
      category:    'Media',
      description: 'Stream music, podcasts, and playlists from Spotify',
      icon:        '🎵',
      platform:    'all',
    },
    {
      id:          'obs-studio',
      name:        'OBS Studio',
      category:    'Media',
      description: 'Free open-source software for video recording and streaming',
      icon:        '🎙️',
      platform:    'all',
    },
    {
      id:          'handbrake',
      name:        'HandBrake',
      category:    'Media',
      description: 'Open-source video transcoder for converting any format',
      icon:        '🎞️',
      platform:    'all',
    },
    {
      id:          'roblox',
      name:        'Roblox',
      category:    'Media',
      description: 'Online game platform — uses Sober on Linux',
      icon:        '🎮',
      platform:    'all',
    },
    {
      id:          'steam',
      name:        'Steam',
      category:    'Media',
      description: 'Valve\'s digital game distribution platform',
      icon:        '🎲',
      platform:    'all',
    },
    {
      id:          'lively-wallpapers',
      name:        'Lively Wallpapers',
      category:    'Media',
      description: 'Free and open-source animated desktop wallpapers',
      icon:        '🖼️',
      platform:    'windows',
    },
    {
      id:          'lutris',
      name:        'Lutris',
      category:    'Media',
      description: 'Open-source gaming platform for Linux game management',
      icon:        '🕹️',
      platform:    'linux',
    },

    // ── Dev Tools ─────────────────────────────────────────────────────────────
    {
      id:          'vscode',
      name:        'VS Code',
      category:    'Dev Tools',
      description: 'Lightweight but powerful source code editor by Microsoft',
      icon:        '💻',
      platform:    'all',
    },
    {
      id:          'git',
      name:        'Git',
      category:    'Dev Tools',
      description: 'Distributed version control system for tracking code changes',
      icon:        '🌿',
      platform:    'all',
    },
    {
      id:          'docker',
      name:        'Docker',
      category:    'Dev Tools',
      description: 'Platform for building and running containerised applications',
      icon:        '🐳',
      platform:    'all',
    },
    {
      id:          'nodejs',
      name:        'Node.js',
      category:    'Dev Tools',
      description: 'JavaScript runtime built on Chrome\'s V8 engine',
      icon:        '🟩',
      platform:    'all',
    },
    {
      id:          'python',
      name:        'Python',
      category:    'Dev Tools',
      description: 'General-purpose programming language, batteries included',
      icon:        '🐍',
      platform:    'all',
    },
    {
      id:          'neovim',
      name:        'Neovim',
      category:    'Dev Tools',
      description: 'Hyperextensible Vim-based text editor',
      icon:        '✏️',
      platform:    'all',
    },
    {
      id:          'kiro-ide',
      name:        'Kiro IDE',
      category:    'Dev Tools',
      description: 'AI-powered IDE with spec-driven development workflows',
      icon:        '⚡',
      platform:    'all',
    },

    // ── Productivity ──────────────────────────────────────────────────────────
    {
      id:          'libreoffice',
      name:        'LibreOffice',
      category:    'Productivity',
      description: 'Free and open-source office suite compatible with MS Office',
      icon:        '📄',
      platform:    'all',
    },
    {
      id:          'obsidian',
      name:        'Obsidian',
      category:    'Productivity',
      description: 'Markdown-based personal knowledge base and note-taking app',
      icon:        '💎',
      platform:    'all',
    },
    {
      id:          'notion',
      name:        'Notion',
      category:    'Productivity',
      description: 'All-in-one workspace for notes, tasks, wikis, and databases',
      icon:        '📝',
      platform:    'all',
    },
    {
      id:          'thunderbird',
      name:        'Thunderbird',
      category:    'Productivity',
      description: 'Free open-source email client by Mozilla',
      icon:        '📨',
      platform:    'all',
    },
    {
      id:          'revo-uninstaller',
      name:        'Revo Uninstaller',
      category:    'Productivity',
      description: 'Thorough software uninstaller with leftover file cleanup',
      icon:        '🗑️',
      platform:    'windows',
    },
    {
      id:          'translucenttb',
      name:        'TranslucentTB',
      category:    'Productivity',
      description: 'Lightweight app for a translucent taskbar on Windows',
      icon:        '🪟',
      platform:    'windows',
    },
    {
      id:          'rainmeter',
      name:        'Rainmeter',
      category:    'Productivity',
      description: 'Desktop customisation tool with skins and widgets',
      icon:        '🌧️',
      platform:    'windows',
    },
    {
      id:          'windhawk',
      name:        'Windhawk',
      category:    'Productivity',
      description: 'Customisation platform for Windows Explorer and apps',
      icon:        '🌬️',
      platform:    'windows',
    },

    // ── Communication ─────────────────────────────────────────────────────────
    {
      id:          'discord',
      name:        'Discord',
      category:    'Communication',
      description: 'Voice, video, and text chat for communities and friends',
      icon:        '💬',
      platform:    'all',
    },
    {
      id:          'equibop',
      name:        'Equibop',
      category:    'Communication',
      description: 'Lightweight Discord client built for Linux',
      icon:        '🫧',
      platform:    'linux',
    },
    {
      id:          'telegram',
      name:        'Telegram',
      category:    'Communication',
      description: 'Cloud-based instant messaging with strong privacy features',
      icon:        '✈️',
      platform:    'all',
    },
    {
      id:          'signal',
      name:        'Signal',
      category:    'Communication',
      description: 'End-to-end encrypted messaging app focused on privacy',
      icon:        '🔒',
      platform:    'all',
    },
    {
      id:          'slack',
      name:        'Slack',
      category:    'Communication',
      description: 'Team messaging and collaboration platform for workplaces',
      icon:        '💼',
      platform:    'all',
    },

    // ── Security ──────────────────────────────────────────────────────────────
    {
      id:          'bitwarden',
      name:        'Bitwarden',
      category:    'Security',
      description: 'Open-source password manager with cloud sync',
      icon:        '🛡️',
      platform:    'all',
    },
    {
      id:          'keepassxc',
      name:        'KeePassXC',
      category:    'Security',
      description: 'Offline cross-platform password manager, no cloud required',
      icon:        '🔑',
      platform:    'all',
    },
    {
      id:          'mullvad-vpn',
      name:        'Mullvad VPN',
      category:    'Security',
      description: 'Privacy-first VPN with anonymous accounts, no logs',
      icon:        '🕵️',
      platform:    'all',
    },
    {
      id:          'wireguard',
      name:        'WireGuard',
      category:    'Security',
      description: 'Fast, modern, and secure VPN tunnel protocol',
      icon:        '🔐',
      platform:    'all',
    },

  ];

}());
