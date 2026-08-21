/**
 * js/main.js — Orchestrator
 *
 * Wires modules together, injects CTAs, handles navigation.
 * No animations, no GSAP.
 */
(function () {
  'use strict';

  document.addEventListener('DOMContentLoaded', function () {

    // Init Demo Panel
    if (typeof DemoPanel !== 'undefined') {
      DemoPanel.init();
    }

    // Detect OS and inject download CTAs
    var platform = OsDetect.detect();
    renderCTAs(platform);
    bindCurlButtons();

    // Wire theme buttons
    var themeBtns = document.querySelectorAll('[data-theme-btn]');
    for (var i = 0; i < themeBtns.length; i++) {
      themeBtns[i].addEventListener('click', function (e) {
        ThemeSwitcher.apply(e.currentTarget.getAttribute('data-theme-btn'));
      });
    }

    // Navbar — no scroll effect needed, solid background

    // Smooth-scroll nav links
    var allLinks = document.querySelectorAll('a[href^="#"]');
    for (var j = 0; j < allLinks.length; j++) {
      allLinks[j].addEventListener('click', function (e) {
        var href = e.currentTarget.getAttribute('href');
        if (!href || href === '#') return;
        var target = document.querySelector(href);
        if (target) {
          e.preventDefault();
          target.scrollIntoView({ behavior: 'smooth' });
          // Close mobile menu if open
          var mobileNav = document.getElementById('nav-links-mobile');
          var toggleBtn = document.getElementById('nav-menu-toggle');
          if (mobileNav && !mobileNav.hidden) {
            mobileNav.hidden = true;
            if (toggleBtn) toggleBtn.setAttribute('aria-expanded', 'false');
          }
        }
      });
    }

    // Mobile hamburger toggle
    var menuToggle = document.getElementById('nav-menu-toggle');
    var mobileNav = document.getElementById('nav-links-mobile');
    if (menuToggle && mobileNav) {
      menuToggle.addEventListener('click', function () {
        var expanded = menuToggle.getAttribute('aria-expanded') === 'true';
        menuToggle.setAttribute('aria-expanded', String(!expanded));
        mobileNav.hidden = expanded;
      });
    }

  });

  // ── CTA rendering ──────────────────────────────────────────────

  function renderCTAs(detectedPlatform) {
    var others = OsDetect.PLATFORMS.filter(function (p) { return p !== detectedPlatform; });

    var heroContainer = document.getElementById('primary-cta-container');
    if (heroContainer) {
      heroContainer.innerHTML = ctaButtonHTML(detectedPlatform, 'primary');
    }

    var container = document.getElementById('download-ctas');
    if (container) {
      var html = ctaButtonHTML(detectedPlatform, 'primary');
      for (var i = 0; i < others.length; i++) {
        html += ctaButtonHTML(others[i], 'secondary');
      }
      container.innerHTML = html;
    }
  }

  function ctaButtonHTML(platform, variant) {
    var info = OsDetect.installerFor(platform);
    if (info.type === 'download') {
      return '<a href="' + info.url + '" download class="cta-btn cta-' + variant + '">' +
               '<i class="iconoir-download"></i>&nbsp;Download for ' + platform +
             '</a>';
    }
    return '<button class="cta-btn cta-' + variant + '" data-curl="' + escapeAttr(info.command) + '">' +
             '<i class="iconoir-download"></i>&nbsp;Install for ' + platform +
           '</button>';
  }

  function escapeAttr(str) {
    return str.replace(/&/g, '&amp;').replace(/"/g, '&quot;').replace(/</g, '&lt;').replace(/>/g, '&gt;');
  }

  function bindCurlButtons() {
    var btns = document.querySelectorAll('[data-curl]');
    for (var i = 0; i < btns.length; i++) {
      btns[i].addEventListener('click', function (e) {
        var cmd = e.currentTarget.getAttribute('data-curl');
        showCurlPanel(cmd);
      });
    }
  }

  function showCurlPanel(command) {
    var existing = document.getElementById('curl-overlay');
    if (existing) existing.remove();

    var overlay = document.createElement('div');
    overlay.id = 'curl-overlay';
    overlay.innerHTML =
      '<div class="curl-panel">' +
        '<div class="curl-panel-header">' +
          '<span class="curl-panel-title">Run this in your terminal</span>' +
          '<button class="curl-close-btn" aria-label="Close"><i class="iconoir-xmark"></i></button>' +
        '</div>' +
        '<div class="curl-command">' +
          '<code>' + escapeHTML(command) + '</code>' +
          '<button class="curl-copy-btn"><i class="iconoir-copy"></i> Copy</button>' +
        '</div>' +
      '</div>';

    document.body.appendChild(overlay);

    overlay.querySelector('.curl-close-btn').addEventListener('click', function () {
      overlay.remove();
    });
    overlay.addEventListener('click', function (e) {
      if (e.target === overlay) overlay.remove();
    });
    overlay.querySelector('.curl-copy-btn').addEventListener('click', function () {
      navigator.clipboard.writeText(command).then(function () {
        var btn = overlay.querySelector('.curl-copy-btn');
        btn.innerHTML = '<i class="iconoir-check"></i> Copied!';
        setTimeout(function () { btn.innerHTML = '<i class="iconoir-copy"></i> Copy'; }, 1500);
      });
    });
  }

  function escapeHTML(str) {
    return str.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;');
  }

}());
