/**
 * config.example.js — Admin configuration template
 *
 * 1. Copy this file to config.js
 * 2. Fill in your values (see .env for reference)
 * 3. config.js is gitignored and never committed
 *
 * To regenerate the PBKDF2 hash after changing the password, run this in the
 * browser console on the admin page:
 *
 *   var enc = new TextEncoder();
 *   var key = await crypto.subtle.importKey('raw', enc.encode('YOUR_PASSWORD'), {name:'PBKDF2'}, false, ['deriveBits']);
 *   var salt = Uint8Array.from(atob('cmlnc2V0LWFkbWluLXYx'), c => c.charCodeAt(0));
 *   var bits = await crypto.subtle.deriveBits({name:'PBKDF2', salt:salt, iterations:100000, hash:'SHA-256'}, key, 256);
 *   console.log(btoa(String.fromCharCode(...new Uint8Array(bits))));
 */
window.RIGSET_CONFIG = {
  AUTH_SALT: 'cmlnc2V0LWFkbWluLXYx',
  AUTH_HASH: 'REPLACE_WITH_PBKDF2_HASH',
  AUTH_ITER: 100000,
  GH_PAT: ''
};
