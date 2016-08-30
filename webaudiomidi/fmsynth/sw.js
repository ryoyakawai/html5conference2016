/* sw.js */
importScripts('serviceworker-cache-polyfill.js');
var version="1.0";
var CACHE_NAME = 'fm_taketech-'+version;
var urlsToCache = [
    './register_sw.js',
    './sw.js',
    './manifest.json',
    './index.html',

    './css/index.css',
    './css/DsLzC9scoPnrGiwYYMQXpkpeNX8RPf6i6WQfJWyCWEs.ttf',
    './css/family_Archivo_Narrow.css',

    './AlgView.js',
    './EG.js',
    './FMToneGenerator.js',
    './PresetVoiceBank.js',
    './VoiceParameters.js',
    './jquery.min.js',
    './parseuri.js',
    './sprintf.js',
    './webmidilink.js',
    './Application.js',
    './EGView.js',
    './FaderView.js',
    './SpectrumView.js',
    './images',
    
    './images/dxjs-144x144.png',
    './images/LED_No0.png',
    './images/LED_No1.png',
    './images/LED_No2.png',
    './images/LED_No3.png',
    './images/LED_No4.png',
    './images/LED_No5.png',
    './images/LED_No6.png',
    './images/LED_No7.png',
    './images/LED_No8.png',
    './images/LED_No9.png',
    './images/background.png',
    './images/edit.png',
    './images/effect.png',
    './images/fader_knob.png',
    './images/led_on.png',
    './images/recorder.png',
    './images/sequencer.png',
    './images/waveselect.png'
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

