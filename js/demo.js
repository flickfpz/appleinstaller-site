/**
 * js/demo.js — Demo Panel
 *
 * Interactive app-selection widget. Reads window.APP_REGISTRY and
 * window.OsDetect, filters apps by detected OS, manages selection state,
 * and renders into #demo-panel.
 *
 * Exposes: window.DemoPanel = { init, setCategory, toggleApp }
 */
window.DemoPanel = (function () {
  'use strict';

  var detectedPlatform = null;

  var PLATFORM_LABELS = {
    windows: 'Windows',
    linux:   'Linux',
    mac:     'macOS',
    all:     null
  };

  var state = {
    activeCategory: 'All',
    selectedIds:    new Set(),
    commandVisible: false
  };

  function platformMatches(appPlat) {
    if (!detectedPlatform) return true;
    if (appPlat === 'all') return true;
    return appPlat === detectedPlatform;
  }

  function categories() {
    var seen = {};
    var cats = ['All'];
    for (var i = 0; i < APP_REGISTRY.length; i++) {
      var app = APP_REGISTRY[i];
      if (!platformMatches(app.platform)) continue;
      var c = app.category;
      if (!seen[c]) {
        seen[c] = true;
        cats.push(c);
      }
    }
    return cats;
  }

  function filteredApps() {
    var apps = APP_REGISTRY.filter(function (a) {
      return platformMatches(a.platform);
    });
    if (state.activeCategory === 'All') return apps;
    return apps.filter(function (a) {
      return a.category === state.activeCategory;
    });
  }

  function setCategory(cat) {
    state.activeCategory = cat;
    render();
  }

  function toggleApp(id) {
    if (state.selectedIds.has(id)) {
      state.selectedIds.delete(id);
    } else {
      state.selectedIds.add(id);
    }
    if (state.selectedIds.size === 0) state.commandVisible = false;
    render();
  }

  function generateCommand() {
    var lines = [];
    state.selectedIds.forEach(function (id) {
      var app = APP_REGISTRY.find(function (a) { return a.id === id; });
      if (!app || !app.commands) return;
      var cmd = app.commands[detectedPlatform] || app.commands.linux || '';
      if (cmd) lines.push(cmd);
    });
    if (lines.length === 0) return '# No commands available';
    return lines.join(' && \\\n');
  }

  function escapeHtml(str) {
    return str.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;').replace(/"/g, '&quot;');
  }

  function buildHTML() {
    var cats = categories();
    var apps = filteredApps();

    // Category tabs
    var tabsHtml = '<div class="demo-categories">';
    for (var i = 0; i < cats.length; i++) {
      var active = cats[i] === state.activeCategory ? ' active' : '';
      tabsHtml += '<button class="demo-cat-btn' + active + '" data-demo-cat="' + escapeHtml(cats[i]) + '">' + escapeHtml(cats[i]) + '</button>';
    }
    tabsHtml += '</div>';

    // Header
    var count = state.selectedIds.size;
    var headerHtml = '<div class="demo-panel-header">';
    headerHtml += tabsHtml;
    headerHtml += '<span class="demo-selected-count">' + (count > 0 ? '<strong>' + count + '</strong> selected' : 'No apps selected') + '</span>';
    headerHtml += '</div>';

    // App cards
    if (apps.length === 0) {
      var gridHtml = '<div class="demo-empty">No apps found in this category.</div>';
    } else {
      var gridHtml = '<div class="demo-app-grid">';
      for (var j = 0; j < apps.length; j++) {
        var app = apps[j];
        var sel = state.selectedIds.has(app.id) ? ' selected' : '';
        var pressed = state.selectedIds.has(app.id) ? 'true' : 'false';
        var platBadge = '';
        if (app.platform !== 'all') {
          platBadge = '<span class="app-card-platform">' + escapeHtml(PLATFORM_LABELS[app.platform] || app.platform) + '</span>';
        }
        gridHtml += '<div class="app-card' + sel + '" data-demo-app="' + app.id + '" tabindex="0" role="button" aria-pressed="' + pressed + '">';
        gridHtml += '<span class="app-card-icon" aria-hidden="true">' + app.icon + '</span>';
        gridHtml += '<span class="app-card-name">' + escapeHtml(app.name) + '</span>';
        gridHtml += '<span class="app-card-desc">' + escapeHtml(app.description) + '</span>';
        gridHtml += platBadge;
        gridHtml += '<span class="app-card-check" aria-hidden="true">✓ Selected</span>';
        gridHtml += '</div>';
      }
      gridHtml += '</div>';
    }

    // Generate button
    var genHtml = '';
    if (count > 0) {
      genHtml = '<button class="demo-generate-btn" data-demo-generate>\u26A1 Get Installer Script</button>';
    }

    // Command output
    var cmdHtml = '';
    if (state.commandVisible && count > 0) {
      var cmd = generateCommand();
      cmdHtml = '<div class="demo-command"><code>' + escapeHtml(cmd) + '</code><button class="demo-copy-btn" data-demo-copy>Copy</button></div>';
    }

    return headerHtml + gridHtml + genHtml + cmdHtml;
  }

  function attachListeners() {
    var panel = document.getElementById('demo-panel');
    if (!panel) return;

    // Category tabs
    var catBtns = panel.querySelectorAll('[data-demo-cat]');
    for (var i = 0; i < catBtns.length; i++) {
      catBtns[i].addEventListener('click', function (e) {
        setCategory(e.currentTarget.getAttribute('data-demo-cat'));
      });
    }

    // App cards — click
    var cards = panel.querySelectorAll('[data-demo-app]');
    for (var j = 0; j < cards.length; j++) {
      cards[j].addEventListener('click', function (e) {
        var card = e.currentTarget;
        toggleApp(card.getAttribute('data-demo-app'));
      });
      cards[j].addEventListener('keydown', function (e) {
        if (e.key === 'Enter' || e.key === ' ') {
          e.preventDefault();
          toggleApp(e.currentTarget.getAttribute('data-demo-app'));
        }
      });
    }

    // Generate button
    var genBtn = panel.querySelector('[data-demo-generate]');
    if (genBtn) {
      genBtn.addEventListener('click', function () {
        state.commandVisible = !state.commandVisible;
        render();
      });
    }

    // Copy button
    var copyBtn = panel.querySelector('[data-demo-copy]');
    if (copyBtn) {
      copyBtn.addEventListener('click', function () {
        var cmd = generateCommand();
        if (navigator.clipboard && navigator.clipboard.writeText) {
          navigator.clipboard.writeText(cmd).then(function () {
            copyBtn.textContent = 'Copied!';
            setTimeout(function () { copyBtn.textContent = 'Copy'; }, 1500);
          });
        } else {
          var ta = document.createElement('textarea');
          ta.value = cmd;
          ta.style.position = 'fixed';
          ta.style.opacity = '0';
          document.body.appendChild(ta);
          ta.select();
          document.execCommand('copy');
          document.body.removeChild(ta);
          copyBtn.textContent = 'Copied!';
          setTimeout(function () { copyBtn.textContent = 'Copy'; }, 1500);
        }
      });
    }
  }

  function render() {
    var panel = document.getElementById('demo-panel');
    if (!panel) return;
    panel.innerHTML = buildHTML();
    attachListeners();
  }

  function init() {
    if (typeof OsDetect !== 'undefined') {
      detectedPlatform = OsDetect.detect();
      // Map OsDetect names to our platform keys
      if (detectedPlatform === 'Windows')          detectedPlatform = 'windows';
      else if (detectedPlatform === 'macOS')       detectedPlatform = 'mac';
      else if (detectedPlatform === 'Arch Linux'
            || detectedPlatform === 'Debian/Ubuntu'
            || detectedPlatform === 'Fedora'
            || detectedPlatform === 'Void Linux'
            || detectedPlatform === 'CachyOS'
            || detectedPlatform === 'Manjaro'
            || detectedPlatform === 'Linux Mint')  detectedPlatform = 'linux';
      else                                         detectedPlatform = 'linux';
    }
    render();
  }

  return { init: init, setCategory: setCategory, toggleApp: toggleApp };
}());
