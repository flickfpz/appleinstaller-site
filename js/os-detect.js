/**
 * js/os-detect.js
 *
 * OS detection module. No DOM reads inside detect(); accepts an optional
 * user-agent string so the function is fully testable in isolation.
 *
 * Exposes: window.OsDetect = { detect, installerFor, PLATFORMS }
 */
window.OsDetect = (function () {

  var PLATFORMS = ['Windows', 'macOS', 'Linux'];

  var IA_BASE = 'https://flickfpz.github.io/appleinstaller-site';
  var LINUX_CMD = 'curl -fsSL ' + IA_BASE + '/rigset-linux-x86_64.tar.gz | tar -xz';

  var INSTALLERS = {
    'Windows':        { type: 'download', url: 'https://github.com/flickfpz/appleinstaller-site/releases/latest/download/rigset-windows-x64.zip' },
    'macOS':          { type: 'curl',     command: 'curl -fsSL ' + IA_BASE + '/rigset-macos-arm64.tar.gz | tar -xz' },
    'Linux':          { type: 'curl',     command: LINUX_CMD }
  };

  function detect(ua) {
    if (ua === undefined || ua === null) {
      ua = navigator.userAgent + ' ' + navigator.platform;
    }
    if (/Windows/i.test(ua))              return 'Windows';
    if (/Mac OS X|macOS/i.test(ua))       return 'macOS';
    return 'Linux';
  }

  function installerFor(platform) {
    return INSTALLERS[platform] || INSTALLERS['Linux'];
  }

  return { detect: detect, installerFor: installerFor, PLATFORMS: PLATFORMS };

})();
