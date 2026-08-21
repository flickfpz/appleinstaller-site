/**
 * theme.js — ThemeSwitcher
 *
 * Exposes window.ThemeSwitcher = { apply, init, THEMES }
 *
 * This script is loaded without `defer` so that ThemeSwitcher.init() runs at
 * parse time and applies the persisted (or default) theme before the first
 * paint, preventing a flash-of-wrong-theme.
 *
 * Requirements: 6.1, 6.2, 6.3, 6.4, 6.5, 6.6, 6.7
 */
window.ThemeSwitcher = (function () {
  'use strict';

  /** Full CSS custom-property palettes for all three themes. */
  var THEMES = {
    midnight: {
      '--bg':             '#0A0818',
      '--surface':        '#161230',
      '--surface-alt':    '#100D24',
      '--border':         '#37375F',
      '--accent':         '#8A50FF',
      '--accent-hover':   '#A069FF',
      '--accent-text':    '#FFFFFF',
      '--text-primary':   '#EBE4FF',
      '--text-secondary': '#9B91C8',
      '--glass-bg':       'rgba(20,16,44,0.85)',
      '--glass-border':   'rgba(90,70,140,0.7)',
      '--hero-grad-a':    '#080818',
      '--hero-grad-b':    '#14163C',
      '--hero-grad-c':    '#0E0626'
    },
    dark: {
      '--bg':             '#111111',
      '--surface':        '#1C1C1E',
      '--surface-alt':    '#242426',
      '--border':         '#3A3A3C',
      '--accent':         '#8A50FF',
      '--accent-hover':   '#A069FF',
      '--accent-text':    '#FFFFFF',
      '--text-primary':   '#F2F2F7',
      '--text-secondary': '#AEAEB2',
      '--glass-bg':       'rgba(28,28,30,0.85)',
      '--glass-border':   'rgba(72,72,74,0.9)',
      '--hero-grad-a':    '#111111',
      '--hero-grad-b':    '#1A1A1F',
      '--hero-grad-c':    '#111111'
    },
    light: {
      '--bg':             '#FFFFFF',
      '--surface':        '#F2F2F7',
      '--surface-alt':    '#E9E9EE',
      '--border':         '#D1D1D6',
      '--accent':         '#8A50FF',
      '--accent-hover':   '#7040EE',
      '--accent-text':    '#FFFFFF',
      '--text-primary':   '#1D1D1F',
      '--text-secondary': '#6E6E73',
      '--glass-bg':       'rgba(255,255,255,0.85)',
      '--glass-border':   'rgba(209,209,214,1)',
      '--hero-grad-a':    '#F2F2F7',
      '--hero-grad-b':    '#EAE4FF',
      '--hero-grad-c':    '#F8F5FF'
    },
    'true-black': {
      '--bg':             '#000000',
      '--surface':        '#0A0A0A',
      '--surface-alt':    '#111111',
      '--border':         '#1A1A1A',
      '--accent':         '#8A50FF',
      '--accent-hover':   '#A069FF',
      '--accent-text':    '#FFFFFF',
      '--text-primary':   '#F0F0F0',
      '--text-secondary': '#808080',
      '--glass-bg':       'rgba(0,0,0,0.9)',
      '--glass-border':   'rgba(40,40,40,0.9)',
      '--hero-grad-a':    '#000000',
      '--hero-grad-b':    '#050510',
      '--hero-grad-c':    '#000000'
    }
  };

  var STORAGE_KEY = 'ai-theme';
  var VALID_THEMES = Object.keys(THEMES);

  /**
   * apply(themeName)
   *
   * 1. Sets data-theme attribute on <html>.
   * 2. Writes every CSS custom property from the palette via style.setProperty.
   * 3. Persists the choice to localStorage (wrapped in try/catch for private-
   *    browsing or storage-quota errors).
   * 4. Toggles the `active` class on [data-theme-btn] buttons.
   *
   * @param {string} themeName - One of 'midnight', 'dark', or 'light'.
   */
  function apply(themeName) {
    if (!THEMES[themeName]) {
      themeName = 'midnight';
    }

    var palette = THEMES[themeName];
    var html = document.documentElement;

    // 1. Set data-theme attribute
    html.setAttribute('data-theme', themeName);

    // 2. Write CSS custom properties
    var props = Object.keys(palette);
    for (var i = 0; i < props.length; i++) {
      html.style.setProperty(props[i], palette[props[i]]);
    }

    // 3. Persist to localStorage
    try {
      localStorage.setItem(STORAGE_KEY, themeName);
    } catch (e) {
      // Private browsing or quota exceeded — silently ignore
    }

    // 4. Toggle active class on theme buttons
    var buttons = document.querySelectorAll('[data-theme-btn]');
    for (var j = 0; j < buttons.length; j++) {
      var btn = buttons[j];
      if (btn.getAttribute('data-theme-btn') === themeName) {
        btn.classList.add('active');
      } else {
        btn.classList.remove('active');
      }
    }
  }

  /**
   * init()
   *
   * Reads localStorage['ai-theme'] and calls apply() with the stored value,
   * or falls back to 'midnight' if the stored value is absent or invalid.
   */
  function init() {
    var stored = null;
    try {
      stored = localStorage.getItem(STORAGE_KEY);
    } catch (e) {
      // localStorage unavailable — proceed with default
    }

    var theme = (stored && THEMES[stored]) ? stored : 'midnight';
    apply(theme);
  }

  // Expose public API
  var ThemeSwitcher = { apply: apply, init: init, THEMES: THEMES };

  // Call init() immediately at parse time (no defer) to prevent FOUC.
  ThemeSwitcher.init();

  return ThemeSwitcher;
}());
