/**
 * js/os-detect.js
 *
 * OS detection module. No DOM reads inside detect(); accepts an optional
 * user-agent string so the function is fully testable in isolation.
 *
 * Exposes: window.OsDetect = { detect, installerFor, PLATFORMS }
 */
window.OsDetect = (function () {

  var PLATFORMS = ['Windows', 'macOS', 'Arch Linux', 'Debian/Ubuntu', 'Fedora', 'Void Linux'];

  var IA_BASE = 'https://flickfpz.github.io/appleinstaller-site';

  var INSTALLERS = {
    'Windows':        { type: 'download', url: IA_BASE + '/AppInstaller-Setup.exe' },
    'macOS':          { type: 'curl',     command: 'curl -fsSL ' + IA_BASE + '/AppInstaller-macos-arm64.tar.gz | tar -xz' },
    'Arch Linux':     { type: 'curl',     command: 'curl -fsSL ' + IA_BASE + '/AppInstaller-linux-x86_64.tar.gz | tar -xz' },
    'Debian/Ubuntu':  { type: 'curl',     command: 'curl -fsSL ' + IA_BASE + '/AppInstaller-linux-x86_64.tar.gz | tar -xz' },
    'Fedora':         { type: 'curl',     command: 'curl -fsSL ' + IA_BASE + '/AppInstaller-linux-x86_64.tar.gz | tar -xz' },
    'CachyOS':        { type: 'curl',     command: 'curl -fsSL ' + IA_BASE + '/AppInstaller-linux-x86_64.tar.gz | tar -xz' },
    'Manjaro':        { type: 'curl',     command: 'curl -fsSL ' + IA_BASE + '/AppInstaller-linux-x86_64.tar.gz | tar -xz' },
    'Linux Mint':     { type: 'curl',     command: 'curl -fsSL ' + IA_BASE + '/AppInstaller-linux-x86_64.tar.gz | tar -xz' },
    'Void Linux':     { type: 'curl',     command: 'curl -fsSL ' + IA_BASE + '/AppInstaller-linux-x86_64.tar.gz | tar -xz' }
  };

  function detect(ua) {
    if (ua === undefined || ua === null) {
      ua = navigator.userAgent + ' ' + navigator.platform;
    }
    if (/Windows/i.test(ua))              return 'Windows';
    if (/Mac OS X|macOS/i.test(ua))       return 'macOS';
    if (/Fedora|CentOS|RHEL/i.test(ua))   return 'Fedora';
    if (/CachyOS/i.test(ua))              return 'CachyOS';
    if (/Manjaro/i.test(ua))              return 'Manjaro';
    if (/Linux Mint/i.test(ua))           return 'Linux Mint';
    if (/Void/i.test(ua))                 return 'Void Linux';
    if (/Arch/i.test(ua))                 return 'Arch Linux';
    return 'Debian/Ubuntu';
  }

  function installerFor(platform) {
    return INSTALLERS[platform];
  }

  return { detect: detect, installerFor: installerFor, PLATFORMS: PLATFORMS };

})();
