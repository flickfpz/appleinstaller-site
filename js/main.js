/**
 * js/main.js — Orchestrator
 *
 * Wires all modules together, injects dynamic HTML (CTAs), builds GSAP
 * animations, and handles navigation behaviour.
 *
 * Loaded with `defer` so the DOM is ready when this runs.
 */
(function () {
  'use strict';

  document.addEventListener('DOMContentLoaded', function () {

    // 1. Init Demo Panel
    if (typeof DemoPanel !== 'undefined') {
      DemoPanel.init();
    }

    // 2. Detect OS and inject download CTAs
    var platform = OsDetect.detect();
    renderCTAs(platform);

    // 3. Wire theme buttons
    var themeBtns = document.querySelectorAll('[data-theme-btn]');
    for (var i = 0; i < themeBtns.length; i++) {
      themeBtns[i].addEventListener('click', function (e) {
        ThemeSwitcher.apply(e.currentTarget.getAttribute('data-theme-btn'));
      });
    }

    // 4. Navbar scroll effect
    var navbar = document.getElementById('navbar');
    window.addEventListener('scroll', function () {
      if (window.scrollY > 80) {
        navbar.classList.add('scrolled');
      } else {
        navbar.classList.remove('scrolled');
      }
    }, { passive: true });

    // 5. Smooth-scroll nav links (desktop + mobile)
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

    // 6. Mobile hamburger toggle
    var menuToggle = document.getElementById('nav-menu-toggle');
    var mobileNav = document.getElementById('nav-links-mobile');
    if (menuToggle && mobileNav) {
      menuToggle.addEventListener('click', function () {
        var expanded = menuToggle.getAttribute('aria-expanded') === 'true';
        menuToggle.setAttribute('aria-expanded', String(!expanded));
        mobileNav.hidden = expanded;
      });
    }

    // 7. GSAP Hero timeline (≤ 1.2s total)
    if (typeof gsap !== 'undefined') {
      var tl = gsap.timeline({ defaults: { ease: 'power3.out' } });
      tl.from('.hero-badge',      { y: 20, opacity: 0, duration: 0.4 })
        .from('.hero-title',      { y: 40, opacity: 0, duration: 0.6 }, '-=0.15')
        .from('.hero-tagline',    { y: 30, opacity: 0, duration: 0.5 }, '-=0.2')
        .from('#primary-cta-container', { y: 20, opacity: 0, duration: 0.4 }, '-=0.2')
        .from('.hero-note',       { y: 15, opacity: 0, duration: 0.35 }, '-=0.15');

      // 8. GSAP ScrollTrigger — feature cards (stagger ≤ 100ms)
      if (typeof ScrollTrigger !== 'undefined') {
        gsap.registerPlugin(ScrollTrigger);
        gsap.from('.feature-card', {
          scrollTrigger: { trigger: '#features', start: 'top 80%' },
          y: 40, opacity: 0,
          duration: 0.55,
          stagger: 0.09
        });

        // Platform entries
        gsap.from('.platform-entry', {
          scrollTrigger: { trigger: '#platforms', start: 'top 80%' },
          y: 30, opacity: 0,
          duration: 0.5,
          stagger: 0.08
        });
      }
    }

  });

  // ── CTA rendering ────────────────────────────────────────────────

  function renderCTAs(detectedPlatform) {
    var others = OsDetect.PLATFORMS.filter(function (p) { return p !== detectedPlatform; });

    // Primary CTA in hero
    var heroContainer = document.getElementById('primary-cta-container');
    if (heroContainer) {
      heroContainer.innerHTML = ctaButtonHTML(detectedPlatform, 'primary');
    }

    // All CTAs in download section
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
    var file = OsDetect.installerFor(platform);
    return '<a href="' + file + '" download class="cta-btn cta-' + variant + '" aria-label="Download for ' + platform + '">' +
             'Download for ' + platform +
           '</a>';
  }

}());
