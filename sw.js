/* sw.js */
importScripts('serviceworker-cache-polyfill.js');
var version="1.3";
var CACHE_NAME = 'html5conf2016-index-'+version;
var urlsToCache = [
    './index.html',
    './register_sw.js',
    './serviceworker-cache-polyfill.js',
    './sw.js',

    './js/material.min.js',
    './webbluetooth/candymagic/md/icon_family_material_icons.css',
    './webbluetooth/candymagic/md/material.green-light_green.min.css',
    './webbluetooth/candymagic/md/material.min.js',
    './webbluetooth/candymagic/md/fonts/2fcrYFNaTjcS6g4U3t-Y5StnKWgpfO2iSkLzTz-AABg.ttf',
    './webbluetooth/candymagic/md/fonts/2fcrYFNaTjcS6g4U3t-Y5UEw0lE80llgEseQY3FEmqw.woff2',
    './webbluetooth/candymagic/md/fonts/hMqPNLsu_dywMa4C_DEpY14sYYdJg5dU2qzJEVSuta0.woff2',
    './webbluetooth/candymagic/md/fonts/hMqPNLsu_dywMa4C_DEpY1BW26QxpSj-_ZKm_xT4hWw.woff2',
    './webbluetooth/candymagic/md/fonts/hMqPNLsu_dywMa4C_DEpY4gp9Q8gbYrhqGlRav_IXfk.woff2',
    './webbluetooth/candymagic/md/fonts/hMqPNLsu_dywMa4C_DEpY6E8kM4xWR1_1bYURRojRGc.woff2',
    './webbluetooth/candymagic/md/fonts/hMqPNLsu_dywMa4C_DEpY9DiNsR5a-9Oe_Ivpu8XWlY.woff2',
    './webbluetooth/candymagic/md/fonts/hMqPNLsu_dywMa4C_DEpY_ZraR2Tg8w2lzm7kLNL0-w.woff2',
    './webbluetooth/candymagic/md/fonts/hMqPNLsu_dywMa4C_DEpYwt_Rm691LTebKfY2ZkKSmI.woff2'

];

self.addEventListener('install', function(event) {
  event.waitUntil(
      caches.open(CACHE_NAME)
          .then(function(cache) {
              //console.log('Opened cache');
              return cache.addAll(urlsToCache);
          })
  );
});

self.addEventListener('fetch', function(event) {
  event.respondWith(
      caches.match(event.request)
          .then(function(response) {
              if (response) {
                  //console.log("[return cache] ", (response.url).split("/").pop());
                  return response;
              }
              var fetchRequest = event.request.clone();

              return fetch(fetchRequest)
                  .then(function(response) {
                      if (!response || response.status !== 200 || response.type !== 'basic') {
                          return response;
                      }
                      
                      var responseToCache = response.clone();

                      caches.open(CACHE_NAME)
                          .then(function(cache) {
                              cache.put(event.request, responseToCache);
                          });
                      
                      return response;
                  });
          })
  );
});

