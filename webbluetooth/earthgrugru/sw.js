/* sw.js */
importScripts('serviceworker-cache-polyfill.js');
var version="1.1";
var CACHE_NAME = 'earth-gru-gru-'+version;
var urlsToCache = [
    './favicon.png',
    './index.html',
    './manifest.json',
    './register_sw.js',
    './serviceworker-cache-polyfill.js',
    './styles.css',
    './sw.js',

    './js/three.js',
    
    './images/earth.jpg',
    './images/earthguruguru_144x144.png',
    
    './md/family_roboto_mono.css',
    './md/icon_family_material_icons.css',
    './md/material.green-light_green.min.css',
    './md/material.min.js',
    './md/fonts/2fcrYFNaTjcS6g4U3t-Y5StnKWgpfO2iSkLzTz-AABg.ttf',
    './md/fonts/2fcrYFNaTjcS6g4U3t-Y5UEw0lE80llgEseQY3FEmqw.woff2',
    './md/fonts/hMqPNLsu_dywMa4C_DEpY14sYYdJg5dU2qzJEVSuta0.woff2',
    './md/fonts/hMqPNLsu_dywMa4C_DEpY1BW26QxpSj-_ZKm_xT4hWw.woff2',
    './md/fonts/hMqPNLsu_dywMa4C_DEpY4gp9Q8gbYrhqGlRav_IXfk.woff2',
    './md/fonts/hMqPNLsu_dywMa4C_DEpY6E8kM4xWR1_1bYURRojRGc.woff2',
    './md/fonts/hMqPNLsu_dywMa4C_DEpY9DiNsR5a-9Oe_Ivpu8XWlY.woff2',
    './md/fonts/hMqPNLsu_dywMa4C_DEpY_ZraR2Tg8w2lzm7kLNL0-w.woff2',
    './md/fonts/hMqPNLsu_dywMa4C_DEpYwt_Rm691LTebKfY2ZkKSmI.woff2'

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

