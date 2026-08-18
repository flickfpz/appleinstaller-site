/**
 * js/os-detect.js
 *
 * OS detection module. No DOM reads inside detect(); accepts an optional
 * user-agent string so the function is fully testable in isolation.
 *
 * Exposes: window.OsDetect = { detect, installerFor, PLATFORMS }
 */
window.OsDetect = (function () {

  /** All five supported platforms in canonical order. */
  var PLATFORMS = ['Windows', 'macOS', 'Arch Linux', 'Debian/Ubuntu', 'Fedora'];

  /**
   * Detect the OS from a user-agent string.
   *
   * Priority rules (first match wins):
   *   1. Windows      — ua contains /Windows/i
   *   2. macOS        — ua contains /Mac OS X|macOS/i
   *   3. Fedora       — ua contains /Fedora|CentOS|RHEL/i
   *   4. Arch Linux   — ua contains /Arch|Manjaro/i
   *   5. Debian/Ubuntu — fallback for everything else
   *
   * @param {string} [ua] - Optional user-agent string. When omitted the
   *   browser's navigator.userAgent + ' ' + navigator.platform is used.
   * @returns {string} One of the five platform names.
   */
  function detect(ua) {
    if (ua === undefined || ua === null) {
      ua = navigator.userAgent + ' ' + navigator.platform;
    }
    if (/Windows/i.test(ua))              return 'Windows';
    if (/Mac OS X|macOS/i.test(ua))       return 'macOS';
    if (/Fedora|CentOS|RHEL/i.test(ua))   return 'Fedora';
    if (/Arch|Manjaro/i.test(ua))         return 'Arch Linux';
    return 'Debian/Ubuntu';
  }

  /**
   * Return the download URL for a given platform.
   *
   * @param {string} platform - One of the five supported platform names.
   * @returns {string} URL to download the installer.
   */
  function installerFor(platform) {
    if (platform === 'Windows') {
      return 'https://github.com/flickfpz/appleinstaller/releases';
    }
    return 'install.sh';
  }

  return { detect: detect, installerFor: installerFor, PLATFORMS: PLATFORMS };

})();
